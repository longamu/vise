#include "../../../../matching/det_ransac/ellipse.h"
#include <string>
#include <vector>

namespace KM_compute_descriptors {
  int lib_main(int argc, char **argv);
  void compute_descriptors_sift(std::string jpg_filename,
                              std::vector<ellipse> &regions,
                              uint32_t & feat_count,
                              float scale_multi,
                              bool upright,
                              float *& descs
                              );
}
