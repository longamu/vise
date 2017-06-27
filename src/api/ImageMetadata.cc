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
  std::ifstream f(metadata_fn_.string().c_str(), std::ifstream::in);

  while( !f.eof() ) {
    std::string csvline;
    std::getline( f, csvline);
    if ( csvline[0] != '#' ) {
      // ignore header lines
      //std::cout << "\nCSVLINE : " << csvline << std::flush;
      std::vector<std::string> tokens;
      ParseCsvLine(csvline, tokens);

      std::string filename = tokens.at(0);
      ImageRegionMetadata metadata_i(tokens.at(5), tokens.at(6));
      metadata_.insert( std::make_pair<std::string, std::vector<ImageRegionMetadata> >(filename, std::vector<ImageRegionMetadata>()) );

      metadata_.find(filename)->second.push_back(metadata_i);
    }
  }
  f.close();
}

void ImageMetadata::GetImageMetadata(std::string image_fn, double x0y0x1y1[], double overlap_threshold, std::string &metadata) {
  std::map< std::string, std::vector<ImageRegionMetadata> >::iterator it;
  it = metadata_.find( image_fn );
  if ( it != metadata_.end() ) {
    for ( unsigned int i=0; i < it->second.size(); i++ ) {
      it->second.at(i).PrintRegionMetadata();
      double iou = it->second.at(i).IOU( x0y0x1y1 );

      if ( iou >= overlap_threshold ) {
        it->second.at(i).GetMetadataString( metadata );
        break;
      }
    }

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

ImageRegionMetadata::ImageRegionMetadata(std::string& region_json, std::string& metadata_json) {
  std::map<std::string, std::string> region;
  std::map<std::string, std::string> metadata;

  JsonParse(region_json, region_);
  JsonParse(metadata_json, region_metadata_);

  if ( region_.find("name")->second == "rect" ) {
    std::string xs = region_.find("x")->second;
    std::string ys = region_.find("y")->second;
    std::string ws = region_.find("width")->second;
    std::string hs = region_.find("height")->second;

    double w,h;
    std::stringstream s;
    s << xs; s >> x0;
    s.clear();


    s << ys; s >> y0;
    s.clear();

    s << ws; s >> w;
    s.clear();

    s << hs; s >> h;
    s.clear();

    x1 = x0 + w;
    y1 = y0 + h;

  }
}

// "{""name"":""rect"",""x"":203,""y"":169,""width"":236,""height"":732}"
void ImageRegionMetadata::JsonParse(std::string& json, std::map<std::string, std::string>& data) {
  std::string s = json.substr(2, json.length()-4); // remove "{ and }"
  std::vector<std::string> tokens;
  StringSplit(s, ',', tokens);
  for( unsigned int i=0; i < tokens.size(); i++ ) {
    std::vector<std::string> keyval;
    StringSplit(tokens.at(i), ':', keyval);
    std::string key = keyval.at(0);
    std::string val = keyval.at(1);
    if ( StringStartsWith( key, "\"\"") ) { // remove escaped double quotes
      key = key.substr(2, key.length() - 4);
    }
    if ( StringStartsWith( val, "\"\"") ) {
      val = val.substr(2, val.length() - 4);
    }

    data.insert( std::make_pair<std::string, std::string>(key, val) );
    //std::cout << "\n\t" << key << " = " << val << std::flush;
  }
}

void ImageRegionMetadata::StringSplit( const std::string s, char sep, std::vector<std::string> &tokens ) {
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

bool ImageRegionMetadata::StringStartsWith( const std::string &s, const std::string &prefix ) {
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
  for ( it = region_.begin(); it != region_.end(); it++ ) {
    std::cout << "\n  Region : " << it->first << "=" << it->second << std::flush;
  }

  for ( it = region_metadata_.begin(); it != region_metadata_.end(); it++ ) {
    std::cout << "\n  Metadata : " << it->first << "=" << it->second << std::flush;
  }
}

void ImageRegionMetadata::GetMetadataString(std::string& metadata_string) {
  std::ostringstream s;
  std::map<std::string, std::string>::iterator it;
  for ( it = region_metadata_.begin(); it != region_metadata_.end(); it++ ) {
    s << it->first << "=" << it->second << " ; ";
  }
  metadata_string = s.str();
}

// compute intersection over union
double ImageRegionMetadata::IOU(double qx0y0x1y1[]) {
  // intersection
  double ix0 = fmax( x0, qx0y0x1y1[0] );
  double iy0 = fmax( y0, qx0y0x1y1[1] );
  double ix1 = fmin( x1, qx0y0x1y1[2] );
  double iy1 = fmin( y1, qx0y0x1y1[3] );
  double intersection_area = fabs( (iy1 - iy0) * (ix1 - ix0) );

  // union
  double union_area1 = fabs( (x0 - x1) * (y0 - y1) );
  double union_area2 = fabs( (qx0y0x1y1[0] - qx0y0x1y1[2]) * (qx0y0x1y1[1] - qx0y0x1y1[3]) );
  double union_area = union_area1 + union_area2 - intersection_area;
  return intersection_area / union_area;
}
