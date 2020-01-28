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
  if ( argc != 4 ) {
    std::cout << "  Usage: " << argv[0]
              << " dset_filename image_graph_filename out_dir"
              << std::endl;
    return 0;
  }

  // file names
  boost::filesystem::path dset_filename( argv[1] );
  boost::filesystem::path image_graph_filename( argv[2] );
  boost::filesystem::path outdir( argv[3] );

  std::cout << "Loading graph data form file [" << image_graph_filename << "]" << std::endl;
  protoDbFile graphdata(image_graph_filename.string());
  protoIndex graphdata_index(graphdata, false);

  //// load dataset (needed to convert file id into filename)
  // dataset_->getNumDoc();
  // dataset_->getInternalFn(file_id);
  // dataset_->getDocID(filename);
  // construct dataset
  std::cout << "Loading dataset from file [" << dset_filename << "]" << std::endl;
  datasetV2 *dset = new datasetV2( dset_filename.string(), "" );
  std::map<uint32_t, std::string > fid_filename_map;

  std::map<uint32_t, std::vector<uint32_t> > is_same_list;
  std::map<uint32_t, std::vector<uint32_t> > is_copy_list;
  std::map<uint32_t, std::vector<uint32_t> > same_hist_list;
  std::map<uint32_t, std::vector<uint32_t> > copy_hist_list;
  std::map<uint32_t, uint32_t> same_parent_fid_list;
  std::map<uint32_t, uint32_t> copy_parent_fid_list;

  for ( uint32_t fid = 0; fid < graphdata_index.numIDs(); ++fid ) {
    std::vector<rr::indexEntry> entries;
    graphdata_index.getEntries(fid, entries);

    if ( entries.size() == 1 ) {
      rr::indexEntry const &entry = entries[0];
      int nbd_count = entry.id_size();
      std::vector<uint32_t> same_nbd;
      std::vector<uint32_t> copy_nbd;
      double same_weight;
      bool same_weight_found = false;
      for ( int i = 0; i < nbd_count; ++i ) {
	if(entry.id(i) == fid) {
	  same_weight = entry.weight(i);
	  same_weight_found = true;
	  break;
	}
      }
      if(!same_weight_found) {
	std::cerr << "search results does not contain the search query!" << std::endl;
	std::cerr << fid << " != " << entry.id(0) << std::endl;
	google::protobuf::ShutdownProtobufLibrary();
	return 1;
      }

      for ( int i = 0; i < nbd_count; ++i ) {
	if ( fabs(entry.weight(i) - same_weight) < 0.00001 &&
	     fid != entry.id(i) ) {
	  if(same_parent_fid_list.count(entry.id(i)) == 0) {
	    // same pixelwise but a different file, never seen before
	    same_nbd.push_back(entry.id(i)); 
	    same_parent_fid_list[entry.id(i)] = fid;
	  }
	}
      }
      if(same_nbd.size()) {
	if(is_same_list.count(fid) !=0) {
	  std::cout << "is_same_list already contains an entry for fid=" << fid << std::endl;
	  return 1;
	}
	is_same_list.insert( std::pair<uint32_t, std::vector<uint32_t> >(fid, same_nbd) );
	if(same_hist_list.count(same_nbd.size()) == 0) {
	  // create new entry
	  same_hist_list[same_nbd.size()] = std::vector<uint32_t>();
	}
	same_hist_list[same_nbd.size()].push_back(fid);
      }
    }
  }

  /* shows size of duplicates
  std::map<uint32_t, uint32_t> same_hist_filesize;
  std::map<uint32_t, std::vector<uint32_t> >::const_iterator it;
  std::cout << "is-same stat." << std::endl;
  std::vector<std::string> ext_list = { ".tif", ".JPG", ".tif.tif", ".TIF", "..tif", ".pdf", ".png", ".tiff" };
  for(it=same_hist_list.begin(); it!=same_hist_list.end(); ++it) {
    uint32_t fsize = 0;
    uint32_t file_not_found_count = 0;
    for(uint32_t i=0; i< it->second.size(); ++i) {
      const boost::filesystem::path fbasepath("/data/seebibyte_tap/ashmolean/original_images");
      const boost::filesystem::path fpath = fbasepath / dset->getInternalFn(it->second.at(i));
      if(boost::filesystem::exists(fpath)) {
	fsize += boost::filesystem::file_size(fpath);
      } else {
	const boost::filesystem::path fname(dset->getInternalFn(it->second.at(i)));
	const std::string fname_str(fname.string());
	boost::filesystem::path fparent = fbasepath / fname.parent_path();

	std::size_t extpos = fname_str.rfind(".jpg");
	std::size_t extlen = 4;
	// check if it was a multi-page TIF or PDF
	for(uint32_t pgno=0; pgno < 100; ++pgno) {
	  std::ostringstream ss;
	  ss << "-" << pgno << ".jpg";
	  std::size_t is_match = fname_str.rfind(ss.str());
	  if(is_match != std::string::npos) {
	    extpos = is_match;
	    extlen = ss.str().length();
	    break;
	  }
	}

	bool found = false;
	boost::filesystem::path fpathtx;
	for(uint32_t j=0; j < ext_list.size(); ++j) {
	  std::string fname_str_j(fname.string());
	  fpathtx = fbasepath / fname_str_j.replace(extpos, extlen, ext_list.at(j));
	  if(boost::filesystem::exists(fpathtx)) {
	    fsize += boost::filesystem::file_size(fpathtx);
	    found = true;
	    break;
	  }
	}
	if(!found) {
	  file_not_found_count += 1;
	  std::cout << "match not found: " << "fid= " << it->second.at(i) << ", fname= "<< dset->getInternalFn(it->second.at(i)) << std::endl;
	}
      }
    }
    std::cout << "Duplicates=" << (it->first + 1) << " : " << it->second.size() << " : size=" << (fsize/(1024*1024)) << " Mb (file_not_found=" << file_not_found_count << ")" << std::endl;
    same_hist_filesize[it->first] = fsize;
  }
*/

  uint32_t SET_COUNT=3;
  std::ofstream same("/ssd/adutta/_tmp_vise_nginx_redir/vise/_tmp/ashmolean/same.html");
  same << "<html><body>";
  for(uint32_t j=0; j<same_hist_list[SET_COUNT].size(); ++j) {
    uint32_t same_fid = same_hist_list[SET_COUNT].at(j);
    std::string ofn = dset->getInternalFn( same_fid );
    std::cout << "[" << j << "] " << ofn << std::endl;
    same << "<div style=\"display:block; border:1px solid black;\"><a href=\"image/" << ofn << "\" target=\"_blank\"><img src=\"image/" << fid_filename_map[same_fid] << "\" style=\"display:block;\"></a>";
    for(uint32_t i=0; i<is_same_list[same_fid].size(); ++i) {
      std::string fn = dset->getInternalFn( is_same_list[same_fid].at(i) );
      std::cout << "  " << fn << std::endl;
      same << "<a href=\"image/" << fn << "\" target=\"_blank\"><img src=\"image/" << fn << "\" style=\"display:inline;\"></a>";
    }
    same << "</div>";
  }
  same << "</body></html>";
  same.close();


  /*
  for ( uint32_t fid = 0; fid < graphdata_index.numIDs(); ++fid ) {
    std::vector<rr::indexEntry> entries;
    graphdata_index.getEntries(fid, entries);

    if ( entries.size() == 1 ) {
      rr::indexEntry const &entry = entries[0];
      int nbd_count = entry.id_size();
      std::vector<uint32_t> same_nbd;
      std::vector<uint32_t> copy_nbd;
      double same_weight;
      bool same_weight_found = false;
      for ( int i = 0; i < nbd_count; ++i ) {
	if(entry.id(i) == fid) {
	  same_weight = entry.weight(i);
	  same_weight_found = true;
	  break;
	}
      }
      if(!same_weight_found) {
	std::cerr << "search results does not contain the search query!" << std::endl;
	std::cerr << fid << " != " << entry.id(0) << std::endl;
	google::protobuf::ShutdownProtobufLibrary();
	return 1;
      }


      for ( int i = 0; i < nbd_count; ++i ) {
	if ( fabs(entry.weight(i) - same_weight) < 0.00001 ) {
	  if( fid != entry.id(i) ) {
	    if(same_parent_fid_list.count(entry.id(i)) == 0) {
	      // same pixelwise but a different file, never seen before
	      same_nbd.push_back(entry.id(i)); 
	      same_parent_fid_list[entry.id(i)] = fid;
	    } else {
	      // discard, as 1->2, 2->1 is same
	    }
	  }
	} else {
	  if( copy_parent_fid_list.count(fid) == 0 ) {
	    copy_nbd.push_back(entry.id(i));
	    copy_parent_fid_list[entry.id(i)] = fid;
	  } else {
	    uint32_t existing_fid = copy_parent_fid_list[entry.id(i)];
	    // check if existing copy_list contains this entry
	    if( std::find(is_copy_list[existing_fid].begin(),
			  is_copy_list[existing_fid].end(), entry.id(i)) == is_copy_list[existing_fid].end()) {
	      // add this to existing set
	      is_copy_list[existing_fid].push_back(entry.id(i));
	    }
	  }
	}
      }

      if(same_nbd.size()) {
	is_same_list.insert( std::pair<uint32_t, std::vector<uint32_t> >(fid, same_nbd) );
	if(same_hist_list.count(same_nbd.size()) == 0) {
	  // create new entry
	  same_hist_list[same_nbd.size()] = std::vector<uint32_t>();
	}
	same_hist_list[same_nbd.size()].push_back(fid);
      }

      if(copy_nbd.size()) {
	is_copy_list.insert( std::pair<uint32_t, std::vector<uint32_t> >(fid, copy_nbd) );
	if(copy_hist_list.count(copy_nbd.size()) == 0) {
	  // create new entry
	  copy_hist_list[copy_nbd.size()] = std::vector<uint32_t>();
	}
	copy_hist_list[copy_nbd.size()].push_back(fid);
      }
    }
  }

  std::map<uint32_t, std::vector<uint32_t> >::const_iterator it;
  std::cout << "is-copy stat." << std::endl;
  for(it=copy_hist_list.begin(); it!=copy_hist_list.end(); ++it) {
    std::cout << "size=" << it->first << " : " << it->second.size() << std::endl;
  }

  std::cout << "is-same stat." << std::endl;
  for(it=same_hist_list.begin(); it!=same_hist_list.end(); ++it) {
    std::cout << "size=" << it->first << " : " << it->second.size() << std::endl;
  }

  uint32_t SET_COUNT=3;
  std::ofstream same("/ssd/adutta/_tmp_vise_nginx_redir/vise/_tmp/ashmolean/same.html");
  same << "<html><body>";
  for(uint32_t j=0; j<same_hist_list[SET_COUNT].size(); ++j) {
    uint32_t same_fid = same_hist_list[SET_COUNT].at(j);
    std::string ofn = dset->getInternalFn( same_fid );
    std::cout << "[" << j << "] " << ofn << std::endl;
    same << "<div style=\"display:block; border:1px solid black;\"><a href=\"image/" << ofn << "\" target=\"_blank\"><img src=\"image/" << fid_filename_map[same_fid] << "\" style=\"display:block;\"></a>";
    for(uint32_t i=0; i<is_same_list[same_fid].size(); ++i) {
      std::string fn = dset->getInternalFn( is_same_list[same_fid].at(i) );
      std::cout << "  " << fn << std::endl;
      same << "<a href=\"image/" << fn << "\" target=\"_blank\"><img src=\"image/" << fn << "\" style=\"display:inline;\"></a>";
    }
    same << "</div>";
  }
  same << "</body></html>";
  same.close();

  std::ofstream copy("/ssd/adutta/_tmp_vise_nginx_redir/vise/_tmp/ashmolean/copy.html");
  copy << "<html><body>";
  for(uint32_t j=0; j<copy_hist_list[SET_COUNT].size(); ++j) {
    uint32_t copy_fid = copy_hist_list[SET_COUNT].at(j);
    std::string ofn = dset->getInternalFn( copy_fid );
    std::cout << "[" << j << "] " << ofn << std::endl;
    copy << "<div style=\"display:block; border:1px solid black;\"><a href=\"image/" << ofn << "\" target=\"_blank\"><img src=\"image/" << fid_filename_map[copy_fid] << "\" style=\"display:block;\"></a>";
    for(uint32_t i=0; i<is_copy_list[copy_fid].size(); ++i) {
      std::string fn = dset->getInternalFn( is_copy_list[copy_fid].at(i) );
      std::cout << "  " << fn << std::endl;
      copy << "<a href=\"image/" << fn << "\" target=\"_blank\"><img src=\"image/" << fn << "\" style=\"display:inline;\"></a>";
    }
    copy << "</div>";
  }
  copy << "</body></html>";
  copy.close();
  */

  // required clean up for protocol buffers
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
