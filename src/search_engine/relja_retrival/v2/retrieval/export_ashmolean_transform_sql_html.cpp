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

class transform_entry {
public:
  uint32_t query_fid;
  uint32_t match_fid;
  char transform_type;
  float score;

  float theta;
  float phi;
  float lambda1;
  float lambda2;
  float tx;
  float ty;

  transform_entry() { }
  transform_entry(std::string d) {
    char comma;
    std::istringstream ss(d);
    ss >> query_fid >> comma >> match_fid >> comma
       >> score >> comma >> transform_type >> comma
       >> theta >> comma >> phi >> comma
       >> lambda1 >> comma >> lambda2 >> comma
       >> tx >> comma >> ty >> comma;
  }

  friend std::ostream& operator<<(std::ostream &os, const transform_entry &te) {
    os << "[" << te.transform_type << " : "
       << "query_fid=" << te.query_fid << "/match_fid=" << te.match_fid << "/score=" << te.score
       << "/theta=" << te.theta << "/phi=" << te.phi << "/"
       << "lambda=" << te.lambda1 << "/" << te.lambda2 << "/tx=" << te.tx << "/ty=" << te.ty << "]";
    return os;
  }
};

class transform_summary {
public:
  uint32_t query_fid;
  uint32_t match_fid;
  float value;

  transform_summary(uint32_t qfid, uint32_t mfid, float val) : query_fid(qfid), match_fid(mfid), value(val) {
  }

  friend std::ostream& operator<<(std::ostream &os, const transform_summary &ts) {
    os << "[" << ts.query_fid << "/" << ts.match_fid << "/" << ts.value << "]";
    return os;
  }
  std::string to_sql_str() const {
    std::ostringstream ss;
    ss << "(" << query_fid << "," << match_fid << "," << value << ")";
    return ss.str();
  }
};

void load_transform_list(std::string fn,
                         std::vector<transform_entry> &scale_list,
                         std::vector<transform_entry> &transform_list,
                         std::vector<transform_entry> &identity_list) {
  std::cout << "loading file " << fn << std::endl;

  std::string line;
  try {
    std::ifstream f(fn);
    scale_list.clear();
    transform_list.clear();
    identity_list.clear();

    std::getline(f, line); // to discard header
    while(f.good()) {
      std::getline(f, line);
      if(line != "") {
        transform_entry te(line);
        switch(te.transform_type) {
        case 'S':
          scale_list.push_back(te);
          break;
        case 'T':
          transform_list.push_back(te);
          break;
        case 'I':
          identity_list.push_back(te);
          break;
        default:
          std::cout << "unknown transform type: " << te.transform_type << std::endl;
        }
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

  std::vector<transform_entry> scale_list;
  std::vector<transform_entry> transform_list;
  std::vector<transform_entry> identity_list;
  boost::filesystem::path transform_fn = outdir / "resize_and_transform.txt";
  load_transform_list(transform_fn.string(), scale_list, transform_list, identity_list);

  std::cout << "showing transform_list_hist" << std::endl;
  std::cout << "Scale : " << scale_list.size() << std::endl;
  std::cout << "Transform : " << transform_list.size() << std::endl;
  std::cout << "Identity : " << identity_list.size() << std::endl;

  std::unordered_map<std::string, std::vector<transform_summary> > all_transform_list;
  std::unordered_map<std::string, std::vector<transform_summary> >::const_iterator itr;
  std::vector<transform_summary> txlist;

  // resize
  txlist.clear();
  for(uint32_t i=0; i < scale_list.size(); ++i) {
    if(scale_list.at(i).score > 0.4) {
      float avg_scale_factor = (scale_list.at(i).lambda1 + scale_list.at(i).lambda2) / (2.0);
      transform_summary tsi(scale_list.at(i).query_fid,
                            scale_list.at(i).match_fid,
                            avg_scale_factor);
      txlist.push_back(tsi);
    }
    //std::cout << tsi << std::endl;
  }
  all_transform_list["transform_resize"] = txlist;

  // rotate by a factor of 90 deg.
  std::cout << "showing rotated by multiple of 90 deg." << std::endl;
  txlist.clear();
  for(uint32_t i=0; i < transform_list.size(); ++i) {
    if( (fabs(transform_list.at(i).lambda1 - 1.0) < 0.1) &&
        (fabs(transform_list.at(i).lambda2 - 1.0) < 0.1) &&
        (transform_list.at(i).score > 0.3 ) &&
        ( (fabs(transform_list.at(i).theta - 90.0) < 15) ||
          (fabs(transform_list.at(i).theta - 180.0) < 15) ||
          (fabs(transform_list.at(i).theta - 270.0) < 15) )
        ) {
      transform_summary tsi(transform_list.at(i).query_fid,
                            transform_list.at(i).match_fid,
                            transform_list.at(i).theta);
      txlist.push_back(tsi);
      //std::cout << transform_list.at(i) << std::endl;
      //std::cout << tsi << std::endl;
    }
  }
  all_transform_list["transform_rotate90i"] = txlist;

  // deformations
  std::cout << "showing other transformations" << std::endl;
  txlist.clear();
  for(uint32_t i=0; i < transform_list.size(); ++i) {
    if( (fabs(transform_list.at(i).lambda1 - 1.0) > 0.1) &&
        (fabs(transform_list.at(i).lambda2 - 1.0) > 0.1) &&
        (transform_list.at(i).score > 0.3 ) &&
        (fabs(transform_list.at(i).theta - 0.0) < 15)
        ) {
      transform_summary tsi(transform_list.at(i).query_fid,
                            transform_list.at(i).match_fid,
                            transform_list.at(i).lambda1);
      txlist.push_back(tsi);
      //std::cout << transform_list.at(i) << std::endl;
      //std::cout << tsi << std::endl;
    }
  }
  all_transform_list["transform_other"] = txlist;

  // show stat
  std::cout << "Showing stat." << std::endl;
  for(itr = all_transform_list.begin(); itr != all_transform_list.end(); ++itr) {
    std::cout << itr->first << " : " << itr->second.size() << std::endl;
  }

  boost::filesystem::path sqlfn = outdir / "transform.sql";
  if(!boost::filesystem::exists(sqlfn)) {
    std::cout << "overwriting " << sqlfn << std::endl;
  }
  std::ofstream sqlf(sqlfn.string());
  sqlf << "BEGIN TRANSACTION;";
  for(itr = all_transform_list.begin(); itr != all_transform_list.end(); ++itr) {
    sqlf << "\nCREATE TABLE IF NOT EXISTS `" << itr->first
         << "` (`file1_id` INTEGER NOT NULL, `file2_id` INTEGER NOT NULL, `transform_parameter` REAL);";
  }
  sqlf << "\nCREATE TABLE IF NOT EXISTS `transform_list_stat` (`name` TEXT NOT NULL, `count` INTEGER NOT NULL);";

  // write all data
  for(itr = all_transform_list.begin(); itr != all_transform_list.end(); ++itr) {
    sqlf << "\nINSERT INTO `" << itr->first << "` VALUES \n"
         << itr->second.at(0).to_sql_str();
    for(uint32_t i=1; i<itr->second.size(); ++i) {
      sqlf << ",\n" << itr->second.at(i).to_sql_str();
    }
    sqlf << ";\n";
    sqlf << "\nINSERT INTO `transform_list_stat` VALUES (\"" << itr->first << "\"," << itr->second.size() << ");";
    sqlf << ";\n";
  }
  sqlf << "COMMIT;";
  sqlf.close();

  // write html visualizer
  std::unordered_map<std::string, std::string> txname_map = {{"transform_rotate90i", "Rotation by a factor of 90&deg;"},
                                                             {"transform_resize", "Resized"},
                                                             {"transform_other", "Other transformations"}
  };

  for(itr = all_transform_list.begin(); itr != all_transform_list.end(); ++itr) {
    boost::filesystem::path htmlfn = outdir / (itr->first + ".html");
    if(!boost::filesystem::exists(htmlfn)) {
      std::cout << "overwriting " << htmlfn << std::endl;
    }
    std::ofstream htmlf(htmlfn.string());

    // Write matches to HTML
    std::cout << "Writing " << itr->second.size() << " entries to html file: " << htmlfn << std::endl;
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
    htmlf << "var match_list={'" << itr->second.at(0).query_fid << "':[" << itr->second.at(0).match_fid << "]";
    for(uint32_t i=1; i < itr->second.size(); ++i) {
      htmlf << ",'" << itr->second.at(i).query_fid << "':[" << itr->second.at(i).match_fid << "]";
    }
    htmlf << "};\n";

    // match stat.
    htmlf << "var match_stat={'2':[" << itr->second.at(0).query_fid;
    for(uint32_t i=1; i<itr->second.size(); ++i) {
      htmlf << "," << itr->second.at(i).query_fid;
    }
    htmlf << "]};\n";
    htmlf << "//console.log(flist); console.log(target_dir); console.log(match);console.log(match_stat)\n"
          << "</script>\n";
    htmlf << VISE_HTML_EXPORT_MATCH_JS_STR << std::endl;
    htmlf << "<body onload=\"vise_init_html_ui()\">"
          << "<h2>Transform Set: " << txname_map.at(itr->first) << "</h2>"
          << "<div id=\"toolbar\"></div><div id=\"content\"></div>"
          << "</body>\n";
    htmlf << VISE_HTML_EXPORT_TAIL_STR << std::endl;
    htmlf.close();
    std::cout << "written to HTML file: " << htmlfn << std::endl;
  }

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
