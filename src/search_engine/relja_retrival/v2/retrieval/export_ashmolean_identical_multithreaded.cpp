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
#include <pthread.h>
#include <omp.h>

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

  /*
  if( boost::filesystem::file_size(fn1) == boost::filesystem::file_size(fn2) ) {
    return true;
  } else {
    return false;
  }

    if( std::equal(f1.data(), f1.data() + f1.size(), f2.data()) ) {
      return true;
    } else {
      return false;
    }
  */

  boost::iostreams::mapped_file_source f1(fn1.string());
  boost::iostreams::mapped_file_source f2(fn2.string());
  if( f1.size() == f2.size() ) {
    bool bytewise_same = true;
#pragma omp parallel for shared(bytewise_same) num_threads(16)
    for(uint32_t i=0; i < f1.size(); ++i) {
      if(!bytewise_same) {
        continue;
      } else {
        if( *(f1.data() + i) != *(f2.data() + i) ) {
          bytewise_same = false;
        }
      }
    }

    if(bytewise_same) {
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
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
          bool identical = is_file_identical(query_original_fn, match_original_fn);
          if(identical) {
            nbd.push_back(match_fid);
          }
        }
      }
      if(nbd.size() > 0) {
        identical.insert( std::pair<uint32_t, std::vector<uint32_t> >(query_fid, nbd) );
        uint32_t identical_size = nbd.size() + 1;
        if(identical_size_stat.count(identical_size) == 0) {
          identical_size_stat.insert( std::pair<uint32_t, std::vector<uint32_t> >(identical_size, std::vector<uint32_t>()) );
        }
        identical_size_stat[identical_size].push_back(query_fid);
      }
    }

    if(query_fid % 1000 == 0) {
      // print identical_size_stat
      std::cout << "\n====[ stat at query_fid=" << query_fid << " ]====" << std::endl;
      for(itr=identical_size_stat.begin(); itr != identical_size_stat.end(); ++itr) {
        std::cout << itr->first << " : " << itr->second.size() << std::endl;
      }

      // write identical to file (temporary stat.)
      boost::filesystem::path outfn = outdir / "identical_multithreaded_tmp.txt";
      std::cout << "writing to file: " << outfn << std::endl;
      std::ofstream outf(outfn.string());
      for(itr = identical.begin(); itr!=identical.end(); ++itr) {
        outf << itr->first;
        for(uint32_t i=0; i < itr->second.size(); ++i) {
          outf << "," << itr->second.at(i);
        }
        outf << std::endl;
      }
      outf.close();
    }
  }

  // print identical_size_stat
  std::cout << "\n====[ final stat ]====" << std::endl;
  for(itr=identical_size_stat.begin(); itr != identical_size_stat.end(); ++itr) {
    std::cout << itr->first << " : " << itr->second.size() << std::endl;
  }

  // write identical to file
  boost::filesystem::path outfn = outdir / "identical_multithreaded.txt";
  std::cout << "writing to file: " << outfn << std::endl;
  std::ofstream outf(outfn.string());
  for(itr = identical.begin(); itr!=identical.end(); ++itr) {
    outf << itr->first;
    for(uint32_t i=0; i < itr->second.size(); ++i) {
      outf << "," << itr->second.at(i);
    }
    outf << std::endl;
  }
  outf.close();

  /*
  sqlfn = outdir / "fid_filename_map.sql";
  if(!boost::filesystem::exists(sqlfn)) {
    std::ofstream sqlf(sqlfn.string());
    sqlf << "BEGIN TRANSACTION;"
         << "\nCREATE TABLE IF NOT EXISTS `fid_filename_map` (`fid` INTEGER UNIQUE,`filename` TEXT NOT NULL, `image_count` INTEGER, PRIMARY KEY(`fid`));"
         << "\nCREATE TABLE IF NOT EXISTS `discarded_filename_list` (`filename` TEXT NOT NULL);";

    std::unordered_map<std::string, uint32_t>::iterator it = original_fid_map.begin();

    sqlf << "\nINSERT INTO `fid_filename_map` VALUES \n"
         << "(" << it->second << ",'" << it->first
         << "'," << original_to_jpg_map[it->first].size() << ")";
    ++it;

    for(; it!=original_fid_map.end(); ++it) {
      sqlf << ",\n(" << it->second << ",'" << it->first << "',"
           << original_to_jpg_map[it->first].size() << ")";
    }
    sqlf << ";\n";

    if(discarded_flist.size()) {
      sqlf << "\nINSERT INTO `discarded_filename_list` VALUES \n";
      sqlf << "('" << discarded_flist.at(0) << "')";
      for(uint32_t i=1; i < discarded_flist.size(); ++i) {
        sqlf << ",\n('" << discarded_flist.at(i) << "')";
      }
      sqlf << ";\n";
    }
    sqlf << "COMMIT;";
    sqlf.close();
  }


  std::vector<bool> fid_visited_flag(dset->getNumDoc(), false);
  std::vector<uint32_t> identical_multipage_files;
  std::unordered_map<uint32_t, std::vector<uint32_t> > identical_files;
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
      std::string query_internal_fn = dset->getInternalFn(query_fid);
      if(jpg_to_original_map.count(query_internal_fn) == 0) {
        continue;
      }
      std::string query_original_fn = jpg_to_original_map[query_internal_fn];
      uint32_t query_original_fid = original_fid_map[query_original_fn];

      uint32_t nbd_count = entry.id_size();
      double query_match_score;
      bool query_found_in_result = false;
      for ( uint32_t j = 0; j < nbd_count; ++j ) { // ignore the first self match
        if( entry.id(j) == query_fid ) {
          query_match_score = entry.weight(j);
          query_found_in_result = true;
          break;
        }
      }
      if(!query_found_in_result) {
        std::cout << "query not found in result" << std::endl;
        std::cout << "query: " << query_fid << " : " << query_internal_fn << std::endl;
        for ( uint32_t k = 0; k < nbd_count; ++k ) { // ignore the first self match
          std::cout << "  " << entry.id(k) << " : " << entry.weight(k) << std::endl;
        }
        return EXIT_FAILURE;
      }

      for ( uint32_t nbd_i = 0; nbd_i < nbd_count; ++nbd_i ) { // ignore the first self match
        uint32_t match_fid = entry.id(nbd_i);
        double match_score = entry.weight(nbd_i);
        std::string match_internal_fn = dset->getInternalFn(match_fid);
        if( query_fid != match_fid &&
            fabs(query_match_score - match_score) < 0.001 ) {
          fid_visited_flag.at(match_fid) = true;
          std::string match_original_fn = jpg_to_original_map[match_internal_fn];
          uint32_t match_original_fid = original_fid_map[match_original_fn];
          if(match_original_fn == query_original_fn) {
            identical_multipage_files.push_back( original_fid_map[query_original_fn] );
            std::cout << "multiple jpg corresponding to same TIF matched" << std::endl;
            std::cout << "query_internal_fn=" << query_internal_fn << std::endl;
            std::cout << "match_internal_fn=" << match_internal_fn << std::endl;
            std::cout << "original_fn=" << match_original_fn << std::endl;
          } else {
            boost::filesystem::path query_original_fn_path = ORIGINAL_IMG_DIR / query_original_fn;
            boost::filesystem::path match_original_fn_path = ORIGINAL_IMG_DIR / match_original_fn;
            bool result = is_file_identical(query_original_fn_path.string(),
                                            match_original_fn_path.string());
            if(result) {
              // files are same in every aspect (size, visual content, byte-wise)
              if(identical_files.count(query_original_fid) == 0) {
                identical_files.insert( std::pair<uint32_t, std::vector<uint32_t> >(query_original_fid, std::vector<uint32_t>()) ) ;
              }
              identical_files[query_original_fid].push_back(match_original_fid);;
            }
          }
        }
      }
      // print debug
      if(query_fid % 500 == 0) {
        std::cout << "[query_fid=" << query_fid << "]: identical_files=" << identical_files.size()
                  << ", identical_multipage_files=" << identical_multipage_files.size() << std::endl;
      }
    }
  }
  std::cout << "identical_files=" << identical_files.size() << std::endl;
  std::cout << "identical_multipage_files=" << identical_multipage_files.size() << std::endl;

  // write identical multipage files
  sqlfn = outdir / "identical-multipage-files.sql";
  if(!boost::filesystem::exists(sqlfn)) {
    std::ofstream sqlf(sqlfn.string());
    sqlf << "BEGIN TRANSACTION;"
         << "\nCREATE TABLE IF NOT EXISTS `identical-multipage-files` (`fid` INTEGER UNIQUE, PRIMARY KEY(`fid`));";
    std::vector<uint32_t>::const_iterator it = identical_multipage_files.begin();
    sqlf << "\nINSERT INTO `identical-multipage-files` VALUES \n"
         << "(" << *it << ")";
    ++it;

    for(; it!=identical_multipage_files.end(); ++it) {
      sqlf << ",\n(" << *it << ")";
    }
    sqlf << ";\n" << "COMMIT;";
    sqlf.close();
  }

  // write identical files
  sqlfn = outdir / "identical-files.sql";
  if(!boost::filesystem::exists(sqlfn)) {
    std::ofstream sqlf(sqlfn.string());
    sqlf << "BEGIN TRANSACTION;"
         << "\nCREATE TABLE IF NOT EXISTS `identical-files` (`id` INTEGER UNIQUE, `identical_fid_list` TEXT NOT NULL, `count` INTEGER,PRIMARY KEY(`fid`));";

    std::unordered_map<uint32_t, std::vector<uint32_t> >::const_iterator it = identical_files.begin();

    sqlf << "\nINSERT INTO `identical-files` VALUES \n"
         << "(" << it->first << ",'" << it->second.at(0);
    for(uint32_t i=1; i<it->second.size(); ++i) {
      sqlf << "," << it->second.at(i);
    }
    sqlf << "'," << it->second.size() << ")";

    ++it;
    for(; it!=identical_files.end(); ++it) {
      sqlf << ",\n(" << it->first << ",'" << it->second.at(0);
      for(uint32_t i=1; i<it->second.size(); ++i) {
        sqlf << "," << it->second.at(i);
      }
      sqlf << "'," << it->second.size() << ")";

    }
    sqlf << ";\n" << "COMMIT;";
    sqlf.close();
  }
  */

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
