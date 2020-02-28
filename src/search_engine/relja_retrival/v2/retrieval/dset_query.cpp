/*
Show image filenames for given docID
Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 29 Jan. 2020
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
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
  if ( argc < 3 ) {
    std::cout << "  Usage: " << argv[0]
              << " dset_filename query_fid fid0 fid1 ..."
              << std::endl;
    return 0;
  }

  // file names
  std::string dset_filename(argv[1]);

  //// load dataset (needed to convert file id into filename)
  // dataset_->getNumDoc();
  // dataset_->getInternalFn(file_id);
  // dataset_->getDocID(filename);
  // construct dataset
  std::cout << "Loading dataset from file [" << dset_filename << "]" << std::endl;
  datasetV2 *dset = new datasetV2( dset_filename, "" );
  std::cout << "dataset has " << dset->getNumDoc() << " files" << std::endl;

  for(int i=2; i < argc; ++i) {
    uint32_t fid = std::atoi(argv[i]);
    if(fid < dset->getNumDoc()) {
      std::cout << fid << " : " << dset->getInternalFn(fid) << std::endl;
    }
  }
  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
