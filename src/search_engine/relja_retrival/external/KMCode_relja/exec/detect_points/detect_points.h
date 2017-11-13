#include "../../../../matching/det_ransac/ellipse.h"
#include <string>
#include <vector>

namespace KM_detect_points {
  int lib_main(int argc, char **argv);
  void detect_points_hesaff(std::string jpg_filename, std::vector<ellipse> &regions);
}
