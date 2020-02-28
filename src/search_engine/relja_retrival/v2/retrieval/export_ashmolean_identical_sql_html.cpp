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
#include <map>
#include <unordered_map>
#include <cmath>

// for filesystem i/o
#include <boost/filesystem.hpp>

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

void load_fnlist(std::string fn, std::vector<std::string> &fnlist) {
  std::cout << "loading file " << fn << std::endl;
  std::string line;
  try {
    std::ifstream f(fn);
    fnlist.clear();
    while(f.good()) {
      std::getline(f, line);
      if(line != "") {
        fnlist.push_back(line);
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
  boost::filesystem::path outdir( argv[4] );


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

  std::unordered_map<uint32_t, std::vector<uint32_t> > identical_list;
  std::map<uint32_t, std::vector<uint32_t> > identical_list_hist;
  boost::filesystem::path identical_fn = outdir / "identical_multithreaded.txt";
  std::ifstream identical_f(identical_fn.string());
  std::string line;
  while(std::getline(identical_f, line)) {
    std::istringstream ss(line);
    std::string file_id_str;
    std::getline(ss, file_id_str, ',');
    uint32_t fid1 = std::atoi(file_id_str.c_str());
    std::vector<uint32_t> fid_list;
    while(std::getline(ss, file_id_str, ',')) {
      fid_list.push_back( std::atoi(file_id_str.c_str()) );
    }
    if(identical_list.count(fid1) == 0) {
      identical_list.insert( std::pair<uint32_t, std::vector<uint32_t> >(fid1, fid_list) );
    } else {
      for(uint32_t i=0; i<fid_list.size(); ++i) {
        identical_list[fid1].push_back( fid_list.at(i) );
      }
    }
    uint32_t identical_size = fid_list.size() + 1;
    if(identical_list_hist.count(identical_size) == 0) {
      identical_list_hist.insert( std::pair<uint32_t, std::vector<uint32_t> >(identical_size, std::vector<uint32_t>()) );
    }
    identical_list_hist[identical_size].push_back(fid1);
  }
  identical_f.close();

  std::cout << "showing identical_list_hist" << std::endl;
  for(std::map<uint32_t, std::vector<uint32_t> >::const_iterator itr = identical_list_hist.begin(); itr != identical_list_hist.end(); ++itr) {
    std::cout << itr->first << " : " << itr->second.size() << std::endl;
  }

  boost::filesystem::path sqlfn = outdir / "identical.sql";
  if(!boost::filesystem::exists(sqlfn)) {
    std::cout << "overwriting " << sqlfn << std::endl;
  }
  std::ofstream sqlf(sqlfn.string());
  sqlf << "BEGIN TRANSACTION;"
       << "\nCREATE TABLE IF NOT EXISTS `fileid_to_filename_map` (`file_id` INTEGER UNIQUE,`filename` TEXT NOT NULL, PRIMARY KEY(`file_id`));"
       << "\nCREATE TABLE IF NOT EXISTS `identical_list` (`identical_set_id` INTEGER, `file_id` INTEGER NOT NULL);"
       << "\nCREATE TABLE IF NOT EXISTS `identical_list_stat` (`size_of_identical_set` INTEGER, `count_of_such_identical_sets` INTEGER NOT NULL);";

  // write fid->original_filename map
  sqlf << "\nINSERT INTO `fileid_to_filename_map` VALUES \n"
       << "(0,\"" << original_imlist.at(0) << "\")";
  for(uint32_t i=1; i<original_imlist.size(); ++i) {
    std::size_t quote = original_imlist.at(i).find("\"");
    if( quote == std::string::npos) {
      sqlf << ",\n(" << i << ",\"" << original_imlist.at(i) << "\")";
    } else {
      sqlf << ",\n(" << i << ",\"" << original_imlist.at(i).replace(quote, 1, "\\\"") << "\")";
    }
  }
  sqlf << ";\n";

  // write identical_list
  uint32_t identical_id = 0;
  std::unordered_map<uint32_t, std::vector<uint32_t> >::const_iterator itr = identical_list.begin();
  sqlf << "\nINSERT INTO `identical_list` VALUES \n"
       << "(" << identical_id << "," << itr->first << ")";
  for(uint32_t i=0; i<itr->second.size(); ++i) {
    sqlf << ",\n(" << identical_id << "," << itr->second.at(i) << ")";
  }

  ++itr;
  ++identical_id;
  for(; itr != identical_list.end(); ++itr) {
    sqlf << ",\n(" << identical_id << "," << itr->first << ")";
    for(uint32_t i=0; i<itr->second.size(); ++i) {
      sqlf << ",\n(" << identical_id << "," << itr->second.at(i) << ")";
    }
    ++identical_id;
  }
  sqlf << ";\n";

  // write stat. of identical set
  std::map<uint32_t, std::vector<uint32_t> >::const_iterator itr_hist = identical_list_hist.begin();
  sqlf << "\nINSERT INTO `identical_list_stat` VALUES \n"
       << "(" << itr_hist->first << "," << itr_hist->second.size() << ")";
  ++itr_hist;
  for(; itr_hist != identical_list_hist.end(); ++itr_hist) {
    sqlf << ",\n(" << itr_hist->first << "," << itr_hist->second.size() << ")";
  }
  sqlf << ";\n";

  sqlf << "COMMIT;";
  sqlf.close();

  boost::filesystem::path htmlfn = outdir / "identical.html";
  if(!boost::filesystem::exists(htmlfn)) {
    std::cout << "overwriting " << htmlfn << std::endl;
  }
  std::ofstream htmlf(htmlfn.string());

  // Write matches to HTML
  std::cout << "Writing " << identical_list.size() << " entries to html file: " << htmlfn << std::endl;
  htmlf << VISE_HTML_EXPORT_HEAD_STR;
  htmlf << "<script>\n"
        << "var target_dir='image/';\n";

  // write filename list
  htmlf << "var flist={'0':'" << imlist.at(0) << "'";
  for(uint32_t i=1; i<imlist.size(); ++i) {
    std::string fname = imlist.at(i);
    std::size_t loc = fname.find('\'');
    if(loc == std::string::npos) {
      htmlf << ",'" << i << "':'" << fname << "'";
    } else {
      htmlf << ",'" << i << "':'" << fname.replace(loc, 1, "\\'") << "'";
    }
  }
  htmlf << "};\n";

  // write original_filename list
  htmlf << "var flist_original={'0':'" << original_imlist.at(0) << "'";
  for(uint32_t i=1; i<original_imlist.size(); ++i) {
    std::string fname = original_imlist.at(i);
    std::size_t loc = fname.find('\'');
    if(loc == std::string::npos) {
      htmlf << ",'" << i << "':'" << fname << "'";
    } else {
      htmlf << ",'" << i << "':'" << fname.replace(loc, 1, "\\'") << "'";
    }
  }
  htmlf << "};\n";

  // match fid
  std::unordered_map<uint32_t, std::vector<uint32_t> >::const_iterator it = identical_list.begin();
  htmlf << "var match_list={'" << it->first << "':[" << it->second.at(0);
  for(uint32_t i=1; i<it->second.size(); ++i) {
    htmlf << "," << it->second.at(i);
  }
  htmlf << "]";
  ++it;
  for(; it != identical_list.end(); ++it) {
    htmlf << ",'" << it->first << "':[" << it->second.at(0);
    for(uint32_t i=1; i<it->second.size(); ++i) {
      htmlf << "," << it->second.at(i);
    }
    htmlf << "]";
  }
  htmlf << "};\n";

  // match stat.
  std::map<uint32_t, std::vector<uint32_t> >::const_iterator it_hist = identical_list_hist.begin();
  htmlf << "var match_stat={'" << it_hist->first << "':[" << it_hist->second.at(0);
  for(uint32_t i=1; i<it_hist->second.size(); ++i) {
    htmlf << "," << it_hist->second.at(i);
  }
  htmlf << "]";
  ++it_hist;
  for(; it_hist != identical_list_hist.end(); ++it_hist) {
    htmlf << ",'" << it_hist->first << "':[" << it_hist->second.at(0);
    for(uint32_t i=1; i<it_hist->second.size(); ++i) {
      htmlf << "," << it_hist->second.at(i);
    }
    htmlf << "]";
  }
  htmlf << "};\n";
  htmlf << "//console.log(flist); console.log(target_dir); console.log(match);console.log(match_stat)\n"
        << "</script>\n";
  htmlf << VISE_HTML_EXPORT_MATCH_JS_STR << std::endl;
  htmlf << "<body onload=\"vise_init_html_ui()\">"
        << "<h2>Identical Set</h2>"
        << "<div id=\"toolbar\"></div><div id=\"content\"></div>"
        << "</body>\n";
  htmlf << VISE_HTML_EXPORT_TAIL_STR << std::endl;
  htmlf.close();
  std::cout << "written to HTML file: " << htmlfn << std::endl;

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
