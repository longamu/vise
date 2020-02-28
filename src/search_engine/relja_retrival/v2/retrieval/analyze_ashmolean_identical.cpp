/*
Export identical image pairs in the Ashmolean image archive

http://www.robots.ox.ac.uk/~vgg/software/via

Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 6 Feb. 2020
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

// for filesystem i/o
#include <boost/filesystem.hpp>

// for memory mapped file
#include <boost/iostreams/device/mapped_file.hpp>

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

#include "export_html.h"

bool is_file_identical(std::string fn1_str, std::string fn2_str) {
  const boost::filesystem::path original_img_basedir = "/data/seebibyte_tap/ashmolean/original_images/";
  boost::filesystem::path fn1 = original_img_basedir / fn1_str;
  boost::filesystem::path fn2 = original_img_basedir / fn2_str;

  if( boost::filesystem::file_size(fn1) == boost::filesystem::file_size(fn2) ) {
    return true;
  } else {
    return false;
  }

  /*
  boost::iostreams::mapped_file_source f1(fn1.string());
  boost::iostreams::mapped_file_source f2(fn2.string());
  if( f1.size() == f2.size() ) {
    if( std::equal(f1.data(), f1.data() + f1.size(), f2.data()) ) {
      return true;
    } else {
      return false;
    }
    } else {
    return false;
    }
  */
}

void load_fnlist(std::string fn, std::vector<std::string> &fnlist) {
  std::cout << "loading file " << fn << std::endl;
  std::string line;
  try {
    std::ifstream f(fn);
    fnlist.clear();
    while(f.good()) {
      std::getline(f, line);
      fnlist.push_back(line);
    }
    f.close();
  } catch(std::exception &ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}

void load_identical_list(std::string fn, std::unordered_map<uint32_t, std::vector<uint32_t> > &identical_list) {
  std::cout << "loading file " << fn << std::endl;
  std::string line;
  try {
    std::ifstream f(fn);
    identical_list.clear();
    while(f.good()) {
      std::getline(f, line);
      std::istringstream ss(line);
      std::string file_id_i_str;
      uint32_t file_id_i;
      bool first_entry = true;
      uint32_t file_id;
      std::vector<uint32_t> nbd_list;
      while( std::getline(ss, file_id_i_str, ',') ) {
        file_id_i = std::atoi(file_id_i_str.c_str());
        if(first_entry) {
          file_id = file_id_i;
          first_entry = false;
        } else {
          nbd_list.push_back(file_id_i);
        }
      }
      if(identical_list.count(file_id) == 0) {
        identical_list.insert( std::pair<uint32_t, std::vector<uint32_t> >(file_id, nbd_list) );
      } else {
        std::cout << "file_id=" << file_id << " already exists in identical_list" << std::endl;
      }
    }
    f.close();
  } catch(std::exception &ex) {
    std::cerr << "exception: " << ex.what() << std::endl;
  }
}

int main(int argc, char* argv[]){
  if ( argc != 5 ) {
    std::cout << "  Usage: " << argv[0]
              << " data_dir nbd score_threshold out_dir"
              << std::endl;
    return 0;
  }

  // file names
  boost::filesystem::path data_dir( argv[1] );
  boost::filesystem::path dset_filename = data_dir / "dset.v2bin";
  std::ostringstream ss;
  ss << "imgraph_nbd" << argv[2] << "_th" << argv[3] << ".v2bin";
  boost::filesystem::path image_graph_filename = data_dir / ss.str();
  boost::filesystem::path outdir( argv[4] );

  std::cout << "Loading graph data form file [" << image_graph_filename << "]" << std::endl;
  protoDbFile graphdata(image_graph_filename.string());
  protoIndex graphdata_index(graphdata, false);

  //// load dataset (needed to convert file id into filename)
  // methods: getNumDoc(), getInternalFn(file_id), getDocID(filename)
  std::cout << "Loading dataset from file [" << dset_filename << "]" << std::endl;
  datasetV2 *dset = new datasetV2( dset_filename.string(), "" );

  // load map from jpg to original TIF filename
  std::unordered_map<std::string, std::string> tx_img_to_original_map;
  std::vector<std::string> imlist;
  std::vector<std::string> original_imlist;
  boost::filesystem::path imlist_fn = data_dir / "imlist.txt";
  boost::filesystem::path original_imlist_fn = data_dir / "original_imlist.txt";
  load_fnlist(imlist_fn.string(), imlist);
  load_fnlist(original_imlist_fn.string(), original_imlist);
  std::cout << "imlist.size=" << imlist.size()
            << ", original_imlist.size=" << original_imlist.size()
            << std::endl;
  if(imlist.size() != original_imlist.size()) {
    return EXIT_FAILURE;
  }
  for(uint32_t i=0; i<imlist.size(); ++i) {
    tx_img_to_original_map[imlist.at(i)] = original_imlist.at(i);
  }

  std::vector<bool> fid_visited_flag(dset->getNumDoc(), false);
  std::unordered_map<uint32_t, std::vector<uint32_t> > identical;
  std::unordered_map<uint32_t, std::vector<uint32_t> > identical_size_stat;
  std::unordered_map<uint32_t, std::vector<uint32_t> >::const_iterator itr;

  boost::filesystem::path identical_fn = outdir / "identical.txt";
  load_identical_list(identical_fn.string(), identical);
  for(itr = identical.begin(); itr != identical.end(); ++itr) {
    uint32_t query_file_id = itr->first;
    uint32_t identical_size = itr->second.size() + 1;
    if(identical_size_stat.count(identical_size) == 0) {
      identical_size_stat.insert( std::pair<uint32_t, std::vector<uint32_t> >(identical_size, std::vector<uint32_t>()) );
    }
    identical_size_stat[identical_size].push_back(query_file_id);
  }

  std::cout << "Showing files with same file size but different byte-wise:" << std::endl;
  for ( uint32_t query_fid = 0; query_fid < graphdata_index.numIDs(); ++query_fid ) {
    //for ( uint32_t query_fid = 0; query_fid < 30000; ++query_fid ) {
    if( fid_visited_flag.at(query_fid) ) {
      continue; // skip as this fid has already been visited
    } else {
      fid_visited_flag.at(query_fid) = true;
    }

    std::vector<rr::indexEntry> entries;
    graphdata_index.getEntries(query_fid, entries);

    if ( entries.size() == 1 ) {
      rr::indexEntry const &entry = entries[0];
      uint32_t nbd_count = entry.id_size();
      std::string query_fn = imlist.at(query_fid);
      std::string query_original_fn = original_imlist.at(query_fid);
      std::vector<uint32_t> nbd;

      for ( uint32_t nbd_i = 0; nbd_i < nbd_count; ++nbd_i ) {
        uint32_t match_fid = entry.id(nbd_i);
        if(entry.keep(nbd_i) && match_fid != query_fid) {
          // an exact matching score
          std::string match_fn = imlist.at(match_fid);
          std::string match_original_fn = original_imlist.at(match_fid);
          bool is_identical = is_file_identical(query_original_fn, match_original_fn);
          if(is_identical) {
            // check if this query,match pair exists in our dataset, if not, print it
            if(identical.count(query_fid) == 1) {
              bool found_match = false;
              for(uint32_t k=0; k < identical[query_fid].size(); ++k) {
                if(identical[query_fid].at(k) == match_fid) {
                  found_match = true;
                  break;
                }
              }
              if(!found_match) {
                std::cout << query_original_fn << " : " << match_original_fn << std::endl;
              }
            }
          }
        }
      }
    }
  }

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
