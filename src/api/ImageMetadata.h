#ifndef VISE_IMAGE_METADATA
#define VISE_IMAGE_METADATA

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>

#include <boost/filesystem.hpp>

class ImageRegionMetadata {
  public:
  ImageRegionMetadata(std::string& region_json, std::string& metadata_json);
  void JsonParse(std::string& json, std::map<std::string, std::string>& data);
  void StringSplit( const std::string s, char sep, std::vector<std::string> &tokens );
  bool StringStartsWith( const std::string &s, const std::string &prefix );
  void PrintRegionMetadata();
  void GetMetadataString(std::string& s);

  double IOU(double x0y0x1y1[]);

  double x0, y0, x1, y1;
  std::map<std::string, std::string> region_;
  std::map<std::string, std::string> region_metadata_;
};

// Singleton design pattern
class ImageMetadata {
  public:
  static ImageMetadata *Instance();

  void LoadMetadata( boost::filesystem::path metadata_csv_fn );
  void GetImageMetadata(std::string image_fn, double x0y0x1y1[], double overlap_threshold, std::string &metadata);
  void PrintMetadata();

  private:
  ImageMetadata() { };
  ImageMetadata( ImageMetadata const & ) { };
  ImageMetadata *operator=(ImageMetadata const & ) {
    return 0;
  };

  static ImageMetadata* image_metadata_;

  void ParseCsvLine(std::string csvline, std::vector<std::string>& tokens);
  boost::filesystem::path metadata_fn_;
  std::map< std::string, std::vector<ImageRegionMetadata> > metadata_;
};

#endif
