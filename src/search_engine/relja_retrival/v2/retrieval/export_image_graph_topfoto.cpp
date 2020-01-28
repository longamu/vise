/*
Image graph is a graph with vertices corresponding to images in the dataset and
edges corresponding to the target vertex being in the search result using source
vertex as query. Such an image graph can be produced using
"src/search_engine/relja_retrival/v2/retrieval/create_image_graph.cpp"

Groups of images containing similar patterns correspond to strongly connected
components of this image graph and is computed by this code. These groups are
exported in a VGG Image Annotator (VIA) project file format for visualization
and further annotation. VIA is an open source manual image annotator and can
be obtained from:
http://www.robots.ox.ac.uk/~vgg/software/via

Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
Date: 5 Dec. 2019
*/

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <stack> // for DFS in graph
#include <sstream>
#include <algorithm>
#include <map>

// for filesystem i/o
#include <boost/filesystem.hpp>

// to compute connected components
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/strong_components.hpp>

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
  if ( argc != 6 ) {
    std::cout << "  Usage: " << argv[0]
              << " dset_filename image_basedir image_graph_filename score_threshold out_dir"
              << std::endl;
    return 0;
  }

  // file names
  boost::filesystem::path dset_filename( argv[1] );
  boost::filesystem::path image_basedir( argv[2] );
  boost::filesystem::path image_graph_filename( argv[3] );
  double SCORE_THRESHOLD;
  std::stringstream ss1;
  ss1 << argv[4];
  ss1 >> SCORE_THRESHOLD;
  boost::filesystem::path outdir( argv[5] );

  boost::filesystem::path im_dir = image_basedir / "image";
  boost::filesystem::path thumb_dir = image_basedir / "thumbnail";

  std::cout << "Loading graph data form file [" << image_graph_filename << "]" << std::endl;
  protoDbFile graphdata(image_graph_filename.string());
  protoIndex graphdata_index(graphdata, false);
  std::size_t nvertex = graphdata_index.numIDs();

  //// load dataset (needed to convert file id into filename)
  // dataset_->getNumDoc();
  // dataset_->getInternalFn(file_id);
  // dataset_->getDocID(filename);
  // construct dataset
  std::cout << "Loading dataset from file [" << dset_filename << "]" << std::endl;
  datasetV2 *dset = new datasetV2( dset_filename.string(), im_dir.string() + "/" );

  std::map<uint32_t, std::vector<uint32_t> > dmap;
  std::map<uint32_t, std::vector<uint32_t>, std::greater<uint32_t> > size_fid_list_map;
  for ( uint32_t fid = 0; fid < graphdata_index.numIDs(); ++fid ) {
  //for ( uint32_t fid = 0; fid < 1000; ++fid ) {
    std::vector<rr::indexEntry> entries;
    graphdata_index.getEntries(fid, entries);

    if ( entries.size() == 1 ) {
      rr::indexEntry const &entry = entries[0];
      int nbd_count = entry.id_size();
      uint32_t nbd_score_threshold_count = 0;
      for ( int i = 0; i < nbd_count; ++i ) {
        if ( entry.weight(i) >= SCORE_THRESHOLD ) {
          nbd_score_threshold_count += 1;
        }
      }

      if(nbd_score_threshold_count) {
	std::vector<uint32_t> nbd;
        for ( int i = 0; i < nbd_count; ++i ) {
          if ( entry.weight(i) >= SCORE_THRESHOLD ) {
	    nbd.push_back( entry.id(i) );
	  }
	}
	dmap.insert( std::pair<uint32_t, std::vector<uint32_t> >(fid, nbd) );
	if(size_fid_list_map.count(nbd.size()) == 0) {
	  std::vector<uint32_t> fid_list;
	  size_fid_list_map.insert( std::pair<uint32_t, std::vector<uint32_t> >(nbd.size(), fid_list));
	}
	size_fid_list_map.at(nbd.size()).push_back(fid);
      }
    }
  }

  std::map<uint32_t, std::vector<uint32_t> >::iterator itr;
  for(itr=size_fid_list_map.begin(); itr!= size_fid_list_map.end(); ++itr) {
    uint32_t nbd_size = (*itr).first;
    std::cout << "nbd_size=" << nbd_size << ", fid_list.size=" << (*itr).second.size() << std::endl;
  }

  /*
  // create sqlite database
  boost::filesystem::path sqlfn = outdir / "topfoto_duplicates_dec2019.sql";
  std::ofstream sqlf(sqlfn.string());
  sqlf << "BEGIN TRANSACTION;"
       << "\nCREATE TABLE IF NOT EXISTS `fid_filename_map` (`fid` INTEGER UNIQUE,`filename` TEXT NOT NULL, PRIMARY KEY(`fid`));"
       << "\nCREATE TABLE IF NOT EXISTS `duplicates` (`fid` INTEGER UNIQUE,`duplicate_fid_list` TEXT NOT NULL, PRIMARY KEY(`fid`));";

  sqlf << "\nINSERT INTO `fid_filename_map` VALUES \n"
       << "(0,'" << dset->getInternalFn(0) << "')";
  for(uint32_t fid=1; fid < dset->getNumDoc(); ++fid) {
    //for(uint32_t fid=1; fid < 4; ++fid) {
    std::string filename = dset->getInternalFn(fid);
    sqlf << ",\n(" << fid << ",'" << filename << "')";
  }
  sqlf << ";\n";
  sqlf << "INSERT INTO `duplicates` VALUES ";
  std::map<uint32_t, std::vector<uint32_t> >::iterator dmap_itr;
  std::map<uint32_t, std::vector<uint32_t> >::iterator dmap_debug(dmap.begin());
  std::advance(dmap_debug, 4);
  for(dmap_itr=dmap.begin(); dmap_itr!=dmap.end(); ++dmap_itr) {
  //for(dmap_itr=dmap.begin(); dmap_itr!=dmap_debug; ++dmap_itr) {
    uint32_t fid = (*dmap_itr).first;
    std::vector<uint32_t> nbd = (*dmap_itr).second;
    if(dmap_itr != dmap.begin()) {
      sqlf << ",";
    }
    sqlf << "\n(" << fid << ",'"
	 << nbd.at(0);
    for(std::size_t i=1; i<nbd.size(); ++i) {
      sqlf << "," << nbd.at(i);
    }
    sqlf << "')";
  }
  sqlf << ";\n";
  sqlf << "COMMIT;";
  sqlf.close();
  
  return 0;
  */

  /*
  // EXPORT HTML FILES
  uint32_t pageno = 1;
  uint32_t img_id = 1;
  const uint32_t MAX_IMG_COUNT = 1000;
  const uint32_t TOTAL_PAGE_COUNT = 341;
  uint32_t page_img_count = 0;
  std::ostringstream html;
  std::map<uint32_t, std::vector<uint32_t> >::iterator dmap_itr;
  for(dmap_itr=dmap.begin(); dmap_itr!=dmap.end(); ++dmap_itr) {
    uint32_t fid = (*dmap_itr).first;
    if(page_img_count==0) {
      html.str("");
      html.clear();
      html << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
	   << "<title>VGG Image Annotator</title><meta name=\"author\" content=\"Abhishek Dutta\">"
	   << "<style>p {font-size:small;} div { border:1px solid #cccccc; display:block; margin:2em 0; } div figure { display:inline-block; margin:0 1em; } figcaption {text-align:center; font-size:small;} div p {font-size:small; margin:0; padding:1em;}</style>"
	   << "</head><body><h1>Duplicates in TopFoto Image Database Containing 437698 Images (Dec. 2019)</h1>"
	   << "<p>Jump to Page: ";
      for(uint32_t pg=1; pg<TOTAL_PAGE_COUNT; ++pg) {
	html << "<a href=\"pg" << pg << ".html\">" << pg << "</a>, ";
      }
      html << "</p><h3>Page&nbsp;" << pageno << "</h3>";
    }

    html << "\n<div>";
    html << "<p>" << img_id << "</p>";
    img_id += 1;
    std::string fid_filename = dset->getInternalFn(fid);
    html << "<a target=\"_blank\" href=\"../image/" << fid_filename << "\"><figure><img src=\"../thumbnail/" << fid_filename << "\"><figcaption>" << fid_filename << "</figcaption></figure></a>";
    page_img_count += 1;

    std::vector<uint32_t> nbd = (*dmap_itr).second;
    for(std::size_t i=0; i<nbd.size(); ++i) {
      std::string nbd_filename = dset->getInternalFn(nbd.at(i));
      html << "<a target=\"_blank\" href=\"../image/" << nbd_filename << "\"><figure><img src=\"../thumbnail/" << nbd_filename << "\"><figcaption>" << nbd_filename << "</figcaption></figure></a>";
      page_img_count += 1;
    }
    html << "</div>";

    if(page_img_count > MAX_IMG_COUNT) {
      html << "</body></html>";

      // write to file
      std::ostringstream pagefn;
      pagefn << "pg" << pageno << ".html";
      boost::filesystem::path htmlfn = outdir / pagefn.str();
      std::ofstream htmlf(htmlfn.string());
      htmlf << html.str();
      htmlf.close();
      std::cout << "Written " << htmlfn.string() << std::endl;
      page_img_count = 0;
      pageno += 1;
    }
  }
  */

  /*
  // debug to find out number of pages
  ////////////////
  uint32_t pageno = 1;
  uint32_t img_id = 1;
  const uint32_t MAX_IMG_COUNT = 1000;
  const uint32_t TOTAL_PAGE_COUNT = 1000;
  uint32_t page_img_count = 0;
  std::ostringstream html;
  std::map<uint32_t, std::vector<uint32_t> >::iterator dmap_itr;
  for(dmap_itr=dmap.begin(); dmap_itr!=dmap.end(); ++dmap_itr) {
    uint32_t fid = (*dmap_itr).first;
    img_id += 1;
    page_img_count += 1;

    std::vector<uint32_t> nbd = (*dmap_itr).second;
    page_img_count += (*dmap_itr).second.size();

    if(page_img_count > MAX_IMG_COUNT) {
      std::cout << "Written " << pageno << std::endl;
      page_img_count = 0;
      pageno += 1;
    }
  }
  */

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
