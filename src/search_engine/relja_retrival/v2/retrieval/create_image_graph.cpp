/*
  ==== Author:

  Relja Arandjelovic (relja@robots.ox.ac.uk)
  Visual Geometry Group,
  Department of Engineering Science
  University of Oxford

  ==== Copyright:

  The library belongs to Relja Arandjelovic and the University of Oxford.
  No usage or redistribution is allowed without explicit permission.
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <cstdlib>

#include "dataset_v2.h"
#include "hamming.h"
#include "hamming_embedder.h"
#include "mpi_queue.h"
#include "image_graph.h"
#include "proto_db.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "spatial_verif_v2.h"
#include "tfidf_v2.h"
#include "util.h"


int main(int argc, char* argv[]){
    
  // MPI initialization
  MPI_INIT_ENV
  MPI_GLOBAL_ALL
    
 if(argc != 4) {
   std::cerr << "Usage: " << argv[0] << " NBD_COUNT SCORE_THRESHOLD BASE_DIR" << std::endl;
   return 0;
 }
  // file name
  //std::string imageGraphFn= util::expandUser("/nvme/seebibyte_tap/ashmolean/vise_data/data/ashmolean/1/imgraph_nbd512_th20.v2bin");
  //std::string imageGraphFn= util::expandUser("/nvme/seebibyte_tap/ashmolean/vise_data/data/ashmolean/1/imgraph_nbd256_th50.v2bin");
  uint32_t nbd_count = std::atoi(argv[1]);
  double score_threshold = std::atoi(argv[2]);

  std::string basedir(argv[3]);
  if(basedir.back() != '/') {
    basedir += "/";
  }
  std::string imageGraphFn = basedir + "imgraph_nbd" + argv[1] + "_th" + argv[2] + ".v2bin";
   
  // file names
  std::string iidxFn= basedir + "iidx.v2bin";
  std::string fidxFn= basedir + "fidx.v2bin";
  std::string wghtFn= basedir + "wght.v2bin";
  std::string hammFn= basedir + "hamm.v2bin";
    
  if ( rank == 0 ) {
    std::cout << "\nBuilding image graph (numProc=" << numProc << ") ..." << std::flush;
    std::cout << "\n  iidx = " << iidxFn;
    std::cout << "\n  fidx = " << fidxFn;
    std::cout << "\n  wght = " << wghtFn;
    std::cout << "\n  hamm = " << hammFn;
    std::cout << "\n  neighbour count = " << nbd_count;
    std::cout << "\n  score threshold = " << score_threshold;
    std::cout << "\n  output file = " << imageGraphFn << std::endl;
  }

  // load fidx
    
  protoDbFile dbFidx_file(fidxFn);
  protoDbInRam dbFidx(dbFidx_file, rank==0);
  protoIndex fidx(dbFidx, false);
    
  // load iidx
    
  protoDbFile dbIidx_file(iidxFn);
  protoDbInRam dbIidx(dbIidx_file, rank==0);
  protoIndex iidx(dbIidx, false);
    
  // create retriever
    
  tfidfV2 tfidfObj(&iidx, &fidx, wghtFn);
  hammingEmbedderFactory embFactory(hammFn, 64);
  hamming hammingObj(tfidfObj, &iidx, embFactory, &fidx);
  spatialVerifV2 spatVerifHamm(hammingObj, &iidx, &fidx, true);
    
  imageGraph imGraph;
  // compute image graph in parallel
  imGraph.computeParallel(imageGraphFn, fidx.numIDs(), spatVerifHamm, nbd_count, score_threshold ); // 27 Jan. 2020

  //imGraph.computeParallel(imageGraphFn, fidx.numIDs(), spatVerifHamm, 256, 50 ); // 23 Jan. 2020

  //imGraph.computeParallel(imageGraphFn, fidx.numIDs(), spatVerifHamm, 999, 3 );
  //imGraph.computeSingle(imageGraphFn, fidx.numIDs(), spatVerifHamm, 9999, 3 );

  // DEBUG
  //imGraph.computeParallel(imageGraphFn, 1, spatVerifHamm, 256, 50 );

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
