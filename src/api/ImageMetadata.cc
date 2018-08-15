#include "ImageMetadata.h"

ImageMetadata* ImageMetadata::image_metadata_ = NULL;

ImageMetadata* ImageMetadata::Instance() {
  if ( !image_metadata_ ) {
    image_metadata_ = new ImageMetadata();
  }
  return image_metadata_;
}

void ImageMetadata::LoadMetadata(boost::filesystem::path metadata_fn) {
  metadata_fn_ = metadata_fn;
  std::cout << "\nOpening file " << metadata_fn_ << std::flush;
  std::ifstream f(metadata_fn_.string().c_str(), std::ifstream::in);

  while( !f.eof() ) {
    std::string csvline;
    std::getline( f, csvline, '\r');
    //std::cout << "\n*** CSVLINE : \n" << csvline << std::flush;
    if ( csvline[0] == '#' ) {
      // ignore header lines
      continue;
    }
    if ( csvline == "" ) {
      continue;
    }

    std::vector<std::string> tokens;
    ParseCsvLine(csvline, tokens);

    std::string filename = tokens.at(0);
    std::map<std::string, std::string> region;
    if ( tokens.at(5) == "\"{}\"" ) {
      // ignore empty fields
      continue;
    }
    JsonParse(tokens.at(5), region);

    // only store rect regions
    if ( region.count("name") == 1) {
      if ( region.find("name")->second == "rect" ) {
        if ( tokens.at(6) == "\"{}\"" ) {
          // ignore empty region attributes
          continue;
        }

        ImageRegionMetadata metadata_i;
        metadata_i.region_json_map_ = region;
        JsonParse(tokens.at(6), metadata_i.region_metadata_);

        // load region coordinates
        std::stringstream s;
        double w, h;
        s << metadata_i.region_json_map_.find("x")->second;
        s >> metadata_i.region_[0]; s.clear();
        s << metadata_i.region_json_map_.find("y")->second;
        s >> metadata_i.region_[1]; s.clear();

        s << metadata_i.region_json_map_.find("width")->second;
        s >> w;
        metadata_i.region_[2] = metadata_i.region_[0] + w; s.clear();

        s << metadata_i.region_json_map_.find("height")->second;
        s >> h;
        metadata_i.region_[3] = metadata_i.region_[1] + h; s.clear();

        //std::cout << "\nfilename=" << filename << " : " << metadata_i.region_[0] << "," << metadata_i.region_[1] << "," << metadata_i.region_[2] << "," << metadata_i.region_[3] << std::flush;
        if ( metadata_.find(filename) == metadata_.end() ) {
          //metadata_.insert( std::make_pair<std::string, std::vector<ImageRegionMetadata> >(filename, std::vector<ImageRegionMetadata>()) );
          metadata_.insert( std::make_pair(filename, std::vector<ImageRegionMetadata>()) );
        }
        metadata_.find(filename)->second.push_back(metadata_i);
      }/* else {
        std::cerr << "\nDiscarding non-rectangular region in metadata : " << tokens.at(5) << std::flush;
      }*/
    } else {
      std::cerr << "\nrect region not found : " << tokens.at(5) << std::flush;
    }
  }
  f.close();
}

void ImageMetadata::LoadPreprocessData( boost::filesystem::path preprocess_log_fn ) {
  preprocess_log_fn = preprocess_log_fn;
  std::ifstream f(preprocess_log_fn.string().c_str(), std::ifstream::in);

  while( f ) {
    //"filename","width_original","height_original","width","height"
    std::string csvline;
    std::getline( f, csvline);

    if ( csvline[0] != '#' && csvline.length() != 0) {
      // ignore header lines
      std::vector<std::string> tokens;
      ParseCsvLine(csvline, tokens);

      std::string filename = tokens.at(0);
      if ( filename[0] == '"' && filename[ filename.size() - 1] == '"' ) {
        // remove double quotes
        filename = filename.substr(1, filename.size() - 2);
      }
      unsigned long w0, h0, wtx, htx;

      std::stringstream s;
      s << tokens.at(1); s >> w0; s.clear();
      s << tokens.at(2); s >> h0; s.clear();
      s << tokens.at(3); s >> wtx; s.clear();
      s << tokens.at(4); s >> htx; s.clear();
      std::vector<double> scale;
      scale.push_back( ((double) w0) / ((double) wtx) ); // scale x
      scale.push_back( ((double) h0) / ((double) htx) ); // scale y
      //img_scale_.insert( std::make_pair<std::string, std::vector<double> >(filename, scale) );
      img_scale_.insert( std::make_pair(filename, scale) );
      //printf("\n%s : w0=%ld, h0=%ld, wtx=%ld, htx=%ld", filename.c_str(), w0, h0, wtx, htx);
      //std::cout << "\n  " << filename << " : sx=" << scale.at(0) << ", sy=" << scale.at(1) << std::flush;
    }
  }
  f.close();
}

void ImageMetadata::GetImageMetadata(std::string image_fn,
                                     double rx0, double ry0, double rx1, double ry1,
                                     double overlap_threshold,
                                     std::string &metadata_str,
                                     std::string &metadata_region_str) {
  //std::cout << "\nImageMetadata::GetImageMetadata() filename = " << image_fn << std::flush;

  std::map< std::string, std::vector<ImageRegionMetadata> >::iterator it;
  it = metadata_.find( image_fn );
  if ( it != metadata_.end() ) {
    //std::cout << "\nFound entry in metadata list" << std::flush;
    std::map< std::string, std::vector<double> >::iterator scale_it = img_scale_.find(image_fn);

    double sx = 1.0;
    double sy = 1.0;
    if( scale_it != img_scale_.end() ) {
      // absence of an entry in preprocess shows that the file has not been modified
      sx = scale_it->second.at(0);
      sy = scale_it->second.at(1);
    }

    rx0 = rx0 * sx;
    ry0 = ry0 * sy;
    rx1 = rx1 * sx;
    ry1 = ry1 * sy;

    double max_iou = -1.0;
    int max_iou_index = -1;

    for ( unsigned int i=0; i < it->second.size(); i++ ) {
      //std::cout << "\n  Region " << (i+1) << std::flush;
      //double* d = it->second.at(i).region_;
      //std::cout << "\n  region_ = " << d[0] << "," << d[1] << "," << d[2] << "," << d[3] << std::flush;
      //it->second.at(i).PrintRegionMetadata();
      double iou = it->second.at(i).IOU( rx0, ry0, rx1, ry1 );
      //std::cout << "\n  IOU = " << iou << std::flush;

      if ( iou >= overlap_threshold ) {
        if ( iou > max_iou ) {
          max_iou = iou;
          max_iou_index = i;
        }
        //std::cout << "(** Matched thresdhold = " << overlap_threshold << ")";
        //break;
      }
    }
    if ( max_iou_index != -1 ) {
      it->second.at(max_iou_index).GetMetadataString( metadata_str );

      // get region coordinates in tx image space
      double mx0 = it->second.at(max_iou_index).region_[0] / sx;
      double my0 = it->second.at(max_iou_index).region_[1] / sy;
      double mx1 = it->second.at(max_iou_index).region_[2] / sx;
      double my1 = it->second.at(max_iou_index).region_[3] / sy;
      std::stringstream s;
      s << (int) mx0 << "," << (int) my0 << "," << (int) mx1 << "," << (int) my1;
      metadata_region_str = s.str();
    }
  } else {
    std::cerr << "\nMetadata not found: " << image_fn << std::flush;
  }
}

void ImageMetadata::PrintMetadata() {
  std::map< std::string, std::vector<ImageRegionMetadata> >::iterator it;
  for ( it = metadata_.begin(); it != metadata_.end(); it++ ) {
    std::string filename = it->first;
    std::cout << "\nFile = " << filename << std::flush;

    for ( unsigned int i=0; i < metadata_.find(filename)->second.size(); i++ ) {
      metadata_.find(filename)->second.at(i).PrintRegionMetadata();
    }
  }
}

// "{""name"":""rect"",""x"":203,""y"":169,""width"":236,""height"":732}"
void ImageMetadata::JsonParse(std::string& json, std::map<std::string, std::string>& data) {
  try {
    json = json.substr(1, json.length() - 2);  // remove json string quotes
    if( json == "" ) {
      return;
    }
    ReplaceAll(json, "\"\"", "\"");          // remove all escaped double quotes

    std::stringstream json_stream(json);
    boost::property_tree::ptree pt;
    boost::property_tree::read_json( json_stream, pt );

    data.clear();
    boost::property_tree::ptree::iterator it;
    for ( it = pt.begin(); it != pt.end(); it++ ) {
      std::string key = it->first;
      std::string val = pt.get<std::string>(key);
      //data.insert( std::make_pair<std::string, std::string>(key, val) );
      data.insert( std::make_pair(key, val) );
    }
  } catch( std::exception& e ) {
    std::cerr << "\nImageMetadata::JsonParse() : error parsing json : " << json << " : " << e.what() << std::flush;
  }
}

void ImageMetadata::ReplaceAll(std::string& str, const std::string& old_str, const std::string& new_str) {
  std::size_t start = 0;
  while ( (start = str.find(old_str, start)) != std::string::npos ) {
    str.replace(start, old_str.length(), new_str);
    start += new_str.length();
  }
}

ImageRegionMetadata::ImageRegionMetadata() {
  region_[0] = 0.0;
  region_[1] = 0.0;
  region_[2] = 0.0;
  region_[3] = 0.0;

  region_json_map_.clear();
  region_metadata_.clear();
}

void ImageMetadata::StringSplit( const std::string s, char sep, std::vector<std::string> &tokens ) {
  if ( s.length() == 0 ) {
    return;
  }

  unsigned int start = 0;
  for ( unsigned int i=0; i<s.length(); ++i) {
    if ( s.at(i) == sep ) {
      tokens.push_back( s.substr(start, (i-start)) );
      start = i + 1;
    }
  }
  tokens.push_back( s.substr(start, s.length()) );
}

bool ImageMetadata::StringStartsWith( const std::string &s, const std::string &prefix ) {
  if ( s.substr(0, prefix.length()) == prefix ) {
    return true;
  } else {
    return false;
  }
}

// assumes that csv line follows the RFC 4180 standard
 void ImageMetadata::ParseCsvLine(std::string csvline, std::vector<std::string>& tokens) {
  bool double_quote_seen = false;
  unsigned int start = 0;
  unsigned int i = 0;
  char field_separator = ',';

  while( i < csvline.length() ) {
    if ( csvline[i] == field_separator ) {
      if ( double_quote_seen ) {
        // field separator inside double quotes is ignored
        i = i + 1;
      } else {
        tokens.push_back( csvline.substr(start, i - start) );
        start = i + 1;
        i = i + 1;
      }
    }
    else {
      if ( csvline[i] == '"' ) {
        if ( double_quote_seen ) {
          if ( csvline[i+1] == '"' ) {
            // ignore escaped double quotes
            i = i + 2;
          } else {
            // closing of double quote
            double_quote_seen = false;
            i = i + 1;
          }
        } else {
          double_quote_seen = true;
          start = i;
          i = i + 1;
        }
      } else {
        i = i + 1;
      }
    }
  }
  // extract the last field (csv rows have no trailing comma)
  tokens.push_back( csvline.substr(start) );
}


void ImageRegionMetadata::PrintRegionMetadata() {
  std::map<std::string, std::string>::iterator it;
  for ( it = region_json_map_.begin(); it != region_json_map_.end(); it++ ) {
    std::cout << "\n  Region : " << it->first << "=" << it->second << std::flush;
  }

  for ( it = region_metadata_.begin(); it != region_metadata_.end(); it++ ) {
    std::cout << "\n  Metadata : " << it->first << "=" << it->second << std::flush;
  }
}

void ImageRegionMetadata::GetMetadataString(std::string& metadata_string) {
  std::ostringstream s;
  /*
  s << "<![CDATA[<table>";
  std::map<std::string, std::string>::iterator it;
  for ( it = region_metadata_.begin(); it != region_metadata_.end(); it++ ) {
    //s << it->first << "=" << it->second << " ; ";
    s << "<tr><td>" << it->first << "</td>"
      << "<td>" << it->second << "</td></tr>";
  }
  s << "</table>]]>";
  */
  std::map<std::string, std::string>::iterator it;
  for ( it = region_metadata_.begin(); it != region_metadata_.end(); it++ ) {
    s << it->first << "__KEYVAL_SEP__" << it->second << "__SEP__";
  }
  metadata_string = s.str();
}

// compute intersection over union
double ImageRegionMetadata::IOU(double rx0, double ry0, double rx1, double ry1) {
  //printf("\n  region = %f,%f,%f,%f", region_[0], region_[1], region_[2], region_[3]);
  //printf("\n  query  = %f,%f,%f,%f", rx0, ry0, rx1, ry1);

  // intersection
  double ix0 = fmax( region_[0], rx0 );
  double iy0 = fmax( region_[1], ry0 );
  double ix1 = fmin( region_[2], rx1 );
  double iy1 = fmin( region_[3], ry1 );
  double x_intersect_area = fmax(0, ix1 - ix0);
  double y_intersect_area = fmax(0, iy1 - iy0);
  double intersection_area = fabs( x_intersect_area * y_intersect_area );

  // union
  double union_area1 = fabs( (region_[2] - region_[0]) * (region_[3] - region_[1]) );
  double union_area2 = fabs( (rx1 - rx0) * (ry1 - ry0) );
  double union_area = union_area1 + union_area2 - intersection_area;
  //std::cout << "\n  intersection = " << intersection_area << ", union = " << union_area << ", IOU=" << (intersection_area / union_area) << std::flush;
  return intersection_area / union_area;
}
