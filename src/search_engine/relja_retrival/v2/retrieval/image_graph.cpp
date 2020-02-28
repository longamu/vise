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

#include "image_graph.h"

#include <iostream>
#include <fstream>

#include <boost/format.hpp>

#ifdef RR_MPI
#include <boost/mpi/collectives.hpp>
#include <boost/serialization/utility.hpp> // for std::pair
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#endif

// include headers that implement a archive in simple text format
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include "mpi_queue.h"
#include "par_queue.h"
#include "proto_db_file.h"
#include "proto_index.h"
#include "timing.h"



void
imageGraph::computeSingle(
                          std::string filename,
                          uint32_t numDocs,
                          retriever const &retrieverObj,
                          uint32_t maxNeighs,
                          double scoreThr ) {

  graph_.clear();

  // prepare files

  protoDbFileBuilder dbBuilder(filename, "image graph");
  indexBuilder idxBuilder(dbBuilder, false, false, false); // no "doDiff" compression as I'm not sorting returned docIDs, but one could

  // do it all

  double score;
  uint32_t numNeighsTotal= 0, docIDres;

  std::vector<indScorePair> queryRes;

  timing::progressPrint graphBuildProgress(numDocs, "imageGraph");

  for (uint32_t docID=0; docID < numDocs; ++docID){

    // print debug info

    graphBuildProgress.inc( docID==0 ?
                            "" :
                            (boost::format("neigh/image= %.2f") % (static_cast<double>(numNeighsTotal)/docID)).str()
                            );

    // query with docID

    queryRes.clear();
    retrieverObj.internalQuery( docID, queryRes, maxNeighs );
    ASSERT( maxNeighs==0 || queryRes.size()<=maxNeighs );

    // save results into the graph

    std::vector<indScorePair> neighs;
    rr::indexEntry entry; // format for saving

    for (std::vector<indScorePair>::const_iterator itRes= queryRes.begin();
         itRes!=queryRes.end();
         ++itRes){

      docIDres= itRes->first;
      score= itRes->second;

      // skip self
      if (docIDres == docID)
        continue;

      // if it passes the score threshold, save it
      if (score >= scoreThr){

        entry.add_id(docIDres);
        entry.add_weight(score);

        neighs.push_back( *itRes );

      } else
        // otherwise stop execution as no future scores will be large enough since they are sorted in non-increasing order
        break;

    }

    if (neighs.size() > 0){

      // insert into the graph
      graph_[docID]= neighs;

      // save to file
      idxBuilder.addEntry(docID, entry);
    }

    numNeighsTotal+= neighs.size();

  }

  // close the file explicitly, though its destructor would do it anyway
  idxBuilder.close();

}



// ------- imageGraph::computeParallel and helper functions

class search_result {
public:
  uint32_t d_fid;
  double d_score;
  std::array<double, 9> d_H;


  friend class boost::serialization::access;
  // When the class Archive corresponds to an output archive, the
  // & operator is defined similar to <<.  Likewise, when the class Archive
  // is a type of input archive the & operator is defined similar to >>.
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
    ar & d_fid;
    ar & d_score;
    ar & d_H;
  }

  search_result() {};
  search_result(uint32_t fid, double score, std::array<double, 9> H) :
    d_fid(fid), d_score(score), d_H(H)
  { }
};

typedef std::vector<search_result> search_result_list;

class imageGraphManager : public managerWithTiming<search_result_list> {
public:

  imageGraphManager(std::string filename,
                    uint32_t numDocs,
                    double scoreThr)
    : managerWithTiming<search_result_list>(numDocs, "imageGraph"),
      dbBuilder_(filename, "image graph"),
      idxBuilder_(dbBuilder_, false, false, false),
      scoreThr_(scoreThr),
      currentDocID(0)
  {}

  void
  compute(uint32_t docID, search_result_list &result_list);

  void
  finalize();

private:
  protoDbFileBuilder dbBuilder_;
  indexBuilder idxBuilder_;
  double const scoreThr_;
  std::map<uint32_t, rr::indexEntry> buffer_;
  uint32_t currentDocID;
};



void
imageGraphManager::compute(uint32_t docID, search_result_list &result_list){

  // save results returned by the worker into the graph

  rr::indexEntry entry; // format for saving

  // if this part takes long (it doesn't here) then it should be handled by the worker, e.g. the worker would create neighs and entry and send it as the result

  double self_match_score = 0.0; // added on 12 feb. 2020
  for(uint32_t i=0; i<result_list.size(); ++i) {
    if(result_list[i].d_fid == docID) {
      self_match_score = result_list[i].d_score;
      break;
    }
  }

  for(uint32_t i=0; i<result_list.size(); ++i) {
    /* Do not skip as we want to find out exact duplicates as well (Abhishek Dutta, 23 Jan. 2020)
    // skip self
    if (docIDres == docID)
    continue;
    */

    // if it passes the score threshold, save it
    if (result_list[i].d_score >= scoreThr_){
      entry.add_id(result_list[i].d_fid);
      entry.add_x(result_list[i].d_H[0]);
      entry.add_y(result_list[i].d_H[1]);
      entry.add_a(result_list[i].d_H[2]);
      entry.add_b(result_list[i].d_H[3]);
      entry.add_c(result_list[i].d_H[4]);
      entry.add_weight(result_list[i].d_H[5]);

      if( fabs(self_match_score - result_list[i].d_score) < 0.00001 ) {
        entry.add_keep(true); // indicates exact match
      } else {
        entry.add_keep(false);
      }
      entry.add_count( ((uint32_t) result_list[i].d_score) );
    } else
      // otherwise stop execution as no future scores will be large enough since they are sorted in non-increasing order
      break;
  }

  // add to buffer
  buffer_[docID]= entry;

  // check if we can save something (i.e. we have all results from before this docID
  for (; buffer_.count(currentDocID)!=0; ++currentDocID){
    entry= buffer_[currentDocID];
    if (entry.id_size()>0)
      idxBuilder_.addEntry(currentDocID, entry);
    buffer_.erase(currentDocID);
  }
}



void
imageGraphManager::finalize(){

  ASSERT(buffer_.size()==0);

  // close the file explicitly, though its destructor would do it anyway
  idxBuilder_.close();
}



class imageGraphWorker : public queueWorker<search_result_list> {

public:

  imageGraphWorker(
                   spatialRetriever const &retrieverObj,
                   uint32_t maxNeighs)
    : retriever_(retrieverObj),
      maxNeighs_(maxNeighs) {}

  void
  operator() ( uint32_t docID, search_result_list &result_list ) const {
    //retriever_.internalQuery(docID, queryRes, maxNeighs_);

    std::vector<indScorePair> queryRes;
    query query_obj(docID, true);
    std::map<uint32_t, homography> Hs;
    retriever_.spatialQuery(query_obj, queryRes, Hs, maxNeighs_);
    ASSERT( maxNeighs_==0 || queryRes.size()<=maxNeighs_ );

    // initialize result
    result_list.clear();
    result_list.reserve(queryRes.size());

    for(uint32_t i=0; i<queryRes.size(); ++i) {
      uint32_t fid = queryRes[i].first;
      search_result ri;
      ri.d_fid = fid;
      ri.d_score = queryRes[i].second;
      memcpy(ri.d_H.data(), Hs[fid].H, 9*sizeof(double));
      /*
      if(queryRes[i].second > 50) {
        //[fid].exportToDoubleArray(ri.d_H.data());
        //std::cout << "query_fid=" << docID << ", fid=" << fid << " : H[1]= " << Hs[fid].H[1] << std::endl;
        std::cout << "query_fid=" << docID << ", fid=" << fid << ", score=" << ri.d_score << " : H= ["
                  << Hs[fid].H[0] << ", " << Hs[fid].H[1] << ", " << Hs[fid].H[2] << ", "
                  << Hs[fid].H[3] << ", " << Hs[fid].H[4] << ", " << Hs[fid].H[5] << ", "
                  << Hs[fid].H[6] << ", " << Hs[fid].H[7] << ", " << Hs[fid].H[8] << "]" << std::endl;
      }
      */
      /*
      ri.tx = Hs[fid].H[2];
      ri.ty = Hs[fid].H[5];
      double a11 = Hs[fid].H[0];
      double a12 = Hs[fid].H[1];
      double a21 = Hs[fid].H[3];
      double a22 = Hs[fid].H[4];
      // H[0] H[1] = A11 A12
      // H[3] H[4]   A21 A22
      // see https://math.stackexchange.com/questions/612006/decomposing-an-affine-transformation
      ri.sx = sqrt(a11*a11 + a21*a21);
      ri.theta = atan2(a21, a11) * 180.0 / 3.14159265;
      double msy = a12*cos(ri.theta) + a22*sin(ri.theta);
      if( fabs(ri.theta - 0.0) < 0.01 ||
          fabs(ri.theta - 180.0) < 0.01) {
        // implies sin(theta) = 0
        ri.sy = (a22 - msy * sin(ri.theta)) / (cos(ri.theta));
      } else {
        ri.sy = (msy * cos(ri.theta) - a12) / (sin(ri.theta));
      }
      ri.shear = msy / ri.sy;
      */
      result_list.push_back(ri);
    }
  }

private:
  spatialRetriever const &retriever_;
  uint32_t const maxNeighs_;
};



void
imageGraph::computeParallel(
                            std::string filename,
                            uint32_t numDocs,
                            spatialRetriever const &retrieverObj,
                            uint32_t maxNeighs,
                            double scoreThr ) {

  MPI_GLOBAL_RANK
    bool useThreads= detectUseThreads();
  uint32_t numWorkerThreads= 128;

  graph_.clear();

  if (rank==0)
    std::cout<<"imageGraph::computeParallel: maxNeighs=" << maxNeighs << ", scoreThr=" << scoreThr << ", numWorkerThreads=" << numWorkerThreads << "\n";

  // make the manager if we are on the master node, otherwise NULL
  imageGraphManager *manager= (rank==0) ?
    new imageGraphManager(filename, numDocs, scoreThr) :
    NULL;

  // make the worker
  imageGraphWorker worker(retrieverObj, maxNeighs);

  parQueue<search_result_list>::startStatic(
                                            numDocs,
                                            worker,
                                            manager,
                                            useThreads,
                                            numWorkerThreads);

  // free memory
  if (rank==0) delete manager;

}



// -------



void
imageGraph::loadFromFile( std::string filename ){

  std::cout<<"imageGraph::loadFromFile\n";

  graph_.clear();

  // open files
  protoDbFile db(filename);
  protoIndex idx(db, false);

  std::vector<rr::indexEntry> entries;

  for (uint32_t docID= 0; docID<idx.numIDs(); ++docID){

    // load
    idx.getEntries(docID, entries);
    ASSERT(entries.size()<=1);

    // if there are edges to this node
    if (entries.size()==1){

      // copy from indexEntry to graph_
      rr::indexEntry const &entry= entries[0];
      std::vector<indScorePair> &neighs= graph_[docID];
      ASSERT( entry.id_size() == entry.weight_size() );
      neighs.reserve( entry.id_size() );

      for (int iEntry= 0; iEntry<entry.id_size(); ++iEntry)
        neighs.push_back(
                         std::make_pair(entry.id(iEntry),
                                        entry.weight(iEntry)) );
    }

  }

  std::cout<<"imageGraph::loadFromFile - DONE\n";

}
