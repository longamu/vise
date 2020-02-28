/*
Export transformed image pairs in the Ashmolean image archive

http://www.robots.ox.ac.uk/~vgg/software/via

Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 12 Feb. 2020
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

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/SVD>

#include <Magick++.h>

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

#define PI 3.14159265

void decompose_affine(const Eigen::MatrixXd &H,
                      double &theta, double &phi,
                      double &lambda1, double &lambda2) {
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(H, Eigen::ComputeFullV | Eigen::ComputeFullU);
  //std::cout << "U = " << std::endl << svd.matrixU() << std::endl;
  //std::cout << "S = " << std::endl << svd.singularValues() << std::endl;
  //std::cout << "V = " << std::endl << svd.matrixV() << std::endl;

  // see Multiview Geometry, by Hartley and Zisserman, Pg 40.
  Eigen::MatrixXd U = svd.matrixU();
  Eigen::MatrixXd V = svd.matrixV();
  Eigen::Vector2d S = svd.singularValues();
  Eigen::MatrixXd Rtheta = U * V.transpose();
  phi = (acos(V(0,0)) * 180.0) / PI;
  theta = (acos(Rtheta(0,0)) * 180.0) / PI;
  lambda1 = S(0);
  lambda2 = S(1);
}

/*
void original_imsize(const std::string fn, uint32_t &width, uint32_t &height) {
  const boost::filesystem::path original_img_basedir = "/data/seebibyte_tap/ashmolean/original_images/";
  boost::filesystem::path fnpath = original_img_basedir / fn;
  Magick::Image im;
  im.quiet(true);

  try {
    im.read(fnpath.string());
    width = im.columns();
    height = im.rows();
  } catch(std::exception &ex) {
    width = 0;
    height = 0;
    //std::cerr << "error loading file: " << fn << std::endl;
    //std::cerr << ex.what() << std::endl;
  }
}
*/

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
  Magick::InitializeMagick(*argv);

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

  // write identical to file
  boost::filesystem::path outfn = outdir / "transform_only_18feb2020.txt";
  std::cout << "writing to file: " << outfn << std::endl;
  std::ofstream outf(outfn.string());
  outf << "query_fid,match_fid,score,flag,theta,phi,lambda1,lambda2,tx,ty" << std::endl;
  std::vector<bool> fid_visited_flag(dset->getNumDoc(), false);
  for ( uint32_t query_fid = 0; query_fid < graphdata_index.numIDs(); ++query_fid ) {
    std::cout << "Processing query_fid=" << query_fid << std::endl;
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
      /*
      uint32_t qw, qh;
      original_imsize(query_original_fn, qw, qh);
      if(qw == 0 && qh == 0) {
        //std::cout << "ignoring invalid query image : " << query_fid << std::endl;
        continue;
      }
      */
      uint32_t self_match_score = 1;
      for ( uint32_t nbd_i = 0; nbd_i < nbd_count; ++nbd_i ) {
        if(entry.keep(nbd_i) && entry.id(nbd_i) == query_fid) {
          self_match_score = entry.count(nbd_i);
        }
      }

      std::vector<uint32_t> nbd;
      for ( uint32_t nbd_i = 0; nbd_i < nbd_count; ++nbd_i ) {
        uint32_t match_fid = entry.id(nbd_i);
        double match_norm_score = ((double) entry.count(nbd_i)) / ((double) self_match_score);
        if(!entry.keep(nbd_i) && match_fid != query_fid && match_norm_score > 0.2) {
          fid_visited_flag.at(match_fid) = true;
          std::string match_fn = imlist.at(match_fid);
          std::string match_original_fn = original_imlist.at(match_fid);

          // NOT an exact match but score is above the threshold
          // compute rotation, scale parameters
          Eigen::MatrixXd H(2,2);
          H(0,0) = entry.x(nbd_i);
          H(0,1) = entry.y(nbd_i);
          H(1,0) = entry.b(nbd_i);
          H(1,1) = entry.c(nbd_i);

          if( !H.isIdentity(0.0001) ) {
            // decompose the affine transformation matrix
            double tx = entry.a(nbd_i);
            double ty = entry.weight(nbd_i);
            double theta, phi, lambda1, lambda2;
            decompose_affine(H, theta, phi, lambda1, lambda2);
            outf << query_fid << "," << match_fid << "," << match_norm_score << ",T," << theta << "," << phi << "," << lambda1 << "," << lambda2 << "," << tx << "," << ty << std::endl;
          }
          /*
          if( H.isIdentity(0.0001) ) {
            // this happens if the query original TIF is 3744 x 5620 and matched original JPG is 1024x1537
            // when these original images are scaled to jpg for VISE, the final dimension of both image
            // is 533x800 as we had requested the max. dimension to be 800x800
            uint32_t mw, mh;
            original_imsize(match_original_fn, mw, mh);
            if(mw == 0 && mh == 0) {
              //std::cout << "ignoring invalid match image : " << query_fid << ":" << match_fid << std::endl;
              continue;
            }

            if( qw == mw &&
                qh == mh ) {
              outf << query_fid << "," << match_fid << "," << match_norm_score << ",I,0,0,1,1,0,0" << std::endl;
            } else {
              double arw = ((double) qw)/ ((double) mw);
              double arh = ((double) qh)/ ((double) mh);
              outf << query_fid << "," << match_fid << "," << match_norm_score << ",S,0,0," << arw << "," << arh << ",0,0" << std::endl;
            }
          } else {
            // decompose the affine transformation matrix
            double tx = entry.a(nbd_i);
            double ty = entry.weight(nbd_i);
            double theta, phi, lambda1, lambda2;
            decompose_affine(H, theta, phi, lambda1, lambda2);
            outf << query_fid << "," << match_fid << "," << match_norm_score << ",T," << theta << "," << phi << "," << lambda1 << "," << lambda2 << "," << tx << "," << ty << std::endl;
          }
          */
        } else {
          continue;
        }
      }
    }

  }
  outf.close();

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
