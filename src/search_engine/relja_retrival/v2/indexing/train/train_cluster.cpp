//
// Compute visual vocabulary from SIFT descriptors extracted from
// a set of images (based on Bag of Visual Words concept described in:
//
// Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
// Date:   16 July 2019
//

#include "train_cluster.h"
#include "mpi_queue.h"

#include <vl/generic.h>
#include <vl/kdtree.h>

#include <algorithm>
#include <chrono>

//#define _OPENMP // to activate OpenMP in vlfeat

namespace buildIndex {
  void isPrime( uint32_t x, bool &result ){
    if (x==0 || x==1){
      result= false;
      return;
    }
    uint32_t lim= sqrt(x)+2;
    if (lim>=x) lim= x-1;
    for (uint32_t i=2; i <= lim; ++i)
      if (x%i==0){
        result= false;
        return;
      }
    result= true;
  }

  class train_cluster_worker : public queueWorker<bool> {
  public:
    inline void operator() ( uint32_t jobID, bool &result ) const {
      isPrime(jobID, result);
    }
  };

  class train_cluster_manager : public queueManager<bool> {
  public:
    train_cluster_manager(uint32_t nJobs) : rec_(nJobs,false), count_(0), numc_(0) {}
    inline void operator() ( uint32_t jobID, bool &result ){
      ++numc_;
      count_+= result;
      rec_[jobID]= result;
    }
    std::vector<bool> rec_;
    uint32_t count_, numc_;
  };

  void compute_train_cluster(std::string const train_desc_fn,
                             bool const use_root_sift,
                             std::string const cluster_fn) {
    std::size_t cluster_count = 10000; // number of clusters

    std::cout << "buildIndex::compute_train_cluster()" << std::endl;
    std::cout << "train_desc_fn = " << train_desc_fn << std::endl;
    std::cout << "use_root_sift = " << use_root_sift << std::endl;
    std::cout << "cluster_fn = " << cluster_fn << std::endl;
    std::cout << "cluster_count = " << cluster_count << std::endl;

    FILE *f = fopen(train_desc_fn.c_str(), "rb");
    ASSERT( f != NULL );

    // file structure
    // SIFT-dimension (uint32_t, 4 bytes)
    // data-type-code (uint8_t , 1 byte )
    // continuous-stream-of-data ...

    // size_t fwrite( const void *buffer, size_t size, size_t count, FILE *stream );
    // size_t fread( void *buffer, size_t size, size_t count, FILE *stream );
    const std::size_t HEADER_BYTES = 4 + 1;
    const std::vector<std::string> DATA_TYPE_STR = {"uint8", "uint16", "uint32", "uint64", "float32", "float64"};
    const std::size_t DATA_TYPE_SIZE[] = {1, 2, 4, 8, 4, 8};

    uint32_t descriptor_dimension;
    uint8_t data_type_code;

    std::size_t read_count;
    read_count = fread(&descriptor_dimension, sizeof(descriptor_dimension), 1, f);

    if ( read_count != 1 ) {
      std::cerr << "Error reading value of descriptor_dimension! "
                << "stored in train descs file: " << train_desc_fn
                << std::endl;
      return;
    }

    read_count = fread(&data_type_code, sizeof(data_type_code), 1, f);
    if ( read_count != 1 ) {
      std::cerr << "Error reading value of data_type_code stored in train descs file: " << train_desc_fn << std::endl;
      return;
    }
    uint32_t element_size = DATA_TYPE_SIZE[data_type_code];

    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);

    uint32_t descriptor_data_length = (file_size - HEADER_BYTES) / (element_size);
    uint32_t descriptor_count = descriptor_data_length / descriptor_dimension;

    std::cout << "descriptor_dimension = " << descriptor_dimension << std::endl;
    std::cout << "data_type_code = " << (int) data_type_code << std::endl;
    std::cout << "file_size = " << file_size << std::endl;
    std::cout << "element_size = " << element_size << std::endl;
    std::cout << "descriptor_data_length = " << descriptor_data_length << std::endl;
    std::cout << "descriptor_count = " << descriptor_count << std::endl;

    std::vector<float> cluster_centers( cluster_count * descriptor_dimension );
    std::vector<uint8_t> descriptors_sift( descriptor_count * descriptor_dimension );
    std::vector<float> descriptors_rootsift( descriptor_count * descriptor_dimension );

    // read descriptors
    std::cout << "Reading SIFT descriptors ... ";
    fseek(f, HEADER_BYTES, SEEK_SET); // move file pointer to start of descriptors
    read_count = fread(descriptors_sift.data(), element_size, descriptor_data_length, f);
    std::cout << read_count << " elements read" << std::endl;
    fclose(f);

    std::size_t di = 0;
    std::size_t sum = 0;

    std::cout << "[" << di << "] ";
    for ( std::size_t i = 0; i < descriptor_data_length; i++ ) {
      //std::cout << (int) descriptors_sift[i] << ",";
      sum = sum + (int) descriptors_sift[i];
      if ( i % 128 == 0 ) {
        di = di + 1;
        if ( sum == 0 ) {
          std::cout << sum << std::endl << "[" << di << "] ";
        }
        sum = 0;
      }
    }

    // convert SIFT descriptors to RootSIFT
    std::cout << "Converting descriptors to RootSIFT ... " << std::endl;
    uint32_t descriptor_index = 0;
    std::vector<uint32_t> descriptors_sum( descriptor_count, 0 );
    for ( std::size_t i = 0; i < descriptor_data_length; i++ ) {
      descriptor_index = (uint32_t) (i / descriptor_dimension);
      descriptors_sum[descriptor_index] += descriptors_sift[i];
    }

    descriptor_index = 0;
    for ( std::size_t i = 0; i < descriptor_data_length; i++ ) {
      descriptor_index = (uint32_t) (i / descriptor_dimension);
      descriptors_rootsift[i] = sqrt( ((float) descriptors_sift[i]) / ((float) descriptors_sum[descriptor_index]) );
    }

    /*
    for ( std::size_t i = 0; i < descriptor_count; ++i ) {
      std::cout << "[" << i << "] ";
      std::size_t offset = i*descriptor_dimension;
      for ( std::size_t j = 0; j < 7; ++j ) {
        std::cout << (int) descriptors_sift[offset + j] << "/" << descriptors_rootsift[offset + j] << ", ";
      }
      std::cout << std::endl;
    }
    return;
    */

    // assign cluster centers to randomly choosen descripts
    std::cout << "Assigning initial clusters to random descriptors ... " << std::endl;
    std::vector<uint32_t> descriptors_index_list( descriptor_count );
    for ( std::size_t i = 0; i < descriptor_count; ++i ) {
      descriptors_index_list[i] = i;
    }
    std::random_shuffle( descriptors_index_list.begin(), descriptors_index_list.end() );
    for ( std::size_t i = 0; i < cluster_count; ++i ) {
      // cluster_centers[i*descriptor_dimension : i*descriptor_dimension + descriptor_dimension]
      // di = descriptors_index_list[i]
      // descriptors[di*descriptor_dimension : di*descriptor_dimension + descriptor_dimension]
      std::size_t descriptors_offset = descriptors_index_list[i] * descriptor_dimension;
      std::size_t cluster_centers_offset = i * descriptor_dimension;
      for ( std::size_t j = 0; j < descriptor_dimension; ++j ) {
        cluster_centers[ cluster_centers_offset + j ] = descriptors_rootsift[descriptors_offset + j];
      }
    }

    // create kd tree of the descriptors
    // see vlfeat-0.9.21/doc/api/kdtree_8c.html#a52564e86ef0d9294a9bc9b13c5d44427
    std::size_t num_trees = 8;
    std::size_t max_num_checks = 512;
    std::cout.precision(7);

    for ( std::size_t kmeans_iteration = 0; kmeans_iteration < 30; ++kmeans_iteration ) {
      std::chrono::steady_clock::time_point iter_start = std::chrono::steady_clock::now();
      //std::cout << "Iteration [" << kmeans_iteration << "] ..." << std::endl;
      VlKDForest* kd_forest = vl_kdforest_new( VL_TYPE_FLOAT, descriptor_dimension, num_trees, VlDistanceL2 );
      vl_kdforest_set_max_num_comparisons(kd_forest, max_num_checks);

      //std::cout << "Building Kd forest from cluster centers using " << num_trees << " trees ..." << std::endl;
      vl_kdforest_build(kd_forest, cluster_count, cluster_centers.data());

      // find nearest cluster assignment for each descriptor
      // see vlfeat-0.9.21/doc/api/kdtree_8c.html#a2b44b23d4ea1b3296a76c6a020831ac6

      //std::cout << "Querying Kd forest to obtain cluster assignment for each descriptor" << std::endl;
      std::size_t num_comp;
      std::vector<uint32_t> descriptor_cluster_assignment(descriptor_count);
      std::vector<float> descriptor_cluster_distance(descriptor_count);

      num_comp = vl_kdforest_query_with_array(kd_forest,
                                              descriptor_cluster_assignment.data(),
                                              1, // number of nearest neighbour to be found for each data point
                                              descriptor_count,
                                              descriptor_cluster_distance.data(),
                                              descriptors_rootsift.data() );

      std::vector<uint32_t> cluster_descriptor_count(cluster_count, 0);
      double cluster_distance_sum = 0.0;
      for ( std::size_t i = 0; i < descriptor_count; ++i ) {
        cluster_descriptor_count[ descriptor_cluster_assignment[i] ] += 1;
        cluster_distance_sum += descriptor_cluster_distance[i];
      }

      // ensure that no cluster is empty
      for ( std::size_t i = 0; i < cluster_count; ++i ) {
        //std::cout << "Cluster " << i << " = " << cluster_descriptor_count[i];
        if ( cluster_descriptor_count[i] == 0 ) {
          std::cout << " *************** Empty *******************";
          break;
        }
        //std::cout << std::endl;
      }

      //std::cout << "Recomputing cluster centers ..." << std::endl;
      cluster_centers = std::vector<float>( cluster_count * descriptor_dimension, 0.0 );
      uint32_t  cluster_id, descriptor_id, offset;
      for ( std::size_t i = 0; i < descriptor_data_length; ++i ) {
        descriptor_id = (uint32_t) ( i / descriptor_dimension );
        offset = i - (descriptor_id * descriptor_dimension);
        cluster_id = descriptor_cluster_assignment[descriptor_id];
        cluster_centers[cluster_id * descriptor_dimension + offset] += descriptors_rootsift[i];
      }

      for ( std::size_t i = 0; i < cluster_count; ++i ) {
        for ( std::size_t j = 0; j < descriptor_dimension; ++j ) {
          cluster_centers[i*descriptor_dimension + j] = cluster_centers[i*descriptor_dimension + j] / cluster_descriptor_count[i];
        }
      }

      std::chrono::steady_clock::time_point iter_end = std::chrono::steady_clock::now();
      /*
      for ( std::size_t i = 0; i < cluster_count; ++i ) {
        std::cout << "[" << i << "] " << cluster_descriptor_count[i] << " : ";
        for ( std::size_t j = 0; j < 9; ++j ) {
          std::cout << cluster_centers[i * descriptor_dimension + j] << ",";
        }
        std::cout << std::endl;
      }
      */
      std::cout << kmeans_iteration << ", "
                << cluster_distance_sum << ", "
                << std::chrono::duration_cast<std::chrono::seconds>(iter_end - iter_start).count()
                << std::endl;

      // cleanup of kd-tree
      vl_kdforest_delete(kd_forest);
    }

    /*
    for ( std::size_t i = 0; i < cluster_count; ++i ) {
      std::cout << "Cluster " << i << " : " << cluster_descriptor_count[i] << " descriptors" << std::endl;
    }

    for ( std::size_t i = 0; i < descriptor_count; ++i ) {
      std::cout << "descriptor=" << i << ", "
                << "cluster=" << descriptor_cluster_assignment[i] << ", "
                << "distance=" << descriptor_cluster_distance[i]
                << std::endl;
    }
    std::cout << "num_comp=" << num_comp << std::endl;
    */

    std::cout << "\n@todo\n" << std::endl;

    /*
    //
    // debug
    //
    MPI_GLOBAL_ALL;
    std::cout << "rank=" << rank << ", num_proc=" << numProc << std::endl;
    uint32_t nJobs= 100000;
    train_cluster_worker worker_obj;
    train_cluster_manager *manager_obj= (rank==0) ? new train_cluster_manager(nJobs) : NULL;

    mpiQueue<bool>::start(nJobs, worker_obj, manager_obj);

    if ( rank == 0 ) {
      if (manager_obj->count_>0){
        std::cout<<manager_obj->count_<<" primes found in first "<<manager_obj->numc_<<" integers.\n";
      }
    }

    if (rank==0) delete manager_obj;
    */
  }
}; // end of namespace buildIndex
