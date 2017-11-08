#ifndef VISE_IMAGE_METADATA
#define VISE_IMAGE_METADATA

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <stdio.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

class ImageRegionMetadata {
  public:
  ImageRegionMetadata();
  void PrintRegionMetadata();
  void GetMetadataString(std::string& s);

  double IOU(double rx0, double ry0, double rx1, double ry1);

  double region_[4]; // x0 y0 x1 y1
  std::map<std::string, std::string> region_json_map_;
  std::map<std::string, std::string> region_metadata_;
};

// Singleton design pattern
class ImageMetadata {
  public:
  static ImageMetadata *Instance();

  void LoadMetadata( boost::filesystem::path metadata_csv_fn );
  void LoadPreprocessData( boost::filesystem::path preprocess_log_fn );
  void GetImageMetadata(std::string image_fn,
                        double rx0, double ry0, double rx1, double ry1,
                        double overlap_threshold,
                        std::string &metadata_str,
                        std::string &metadata_region_str);
  void PrintMetadata();

  // utils
  void JsonParse(std::string& json, std::map<std::string, std::string>& data);
  void StringSplit( const std::string s, char sep, std::vector<std::string> &tokens );
  bool StringStartsWith( const std::string &s, const std::string &prefix );
  void ReplaceAll(std::string& str, const std::string& old_str, const std::string& new_str);

  private:
  ImageMetadata() { };
  ImageMetadata( ImageMetadata const & ) { };
  ImageMetadata *operator=(ImageMetadata const & ) {
    return 0;
  };

  static ImageMetadata* image_metadata_;

  void ParseCsvLine(std::string csvline, std::vector<std::string>& tokens);
  boost::filesystem::path preprocess_log_fn_;
  boost::filesystem::path metadata_fn_;
  std::map< std::string, std::vector<ImageRegionMetadata> > metadata_;
  std::map< std::string, std::vector<double> > img_scale_;
};

#endif
