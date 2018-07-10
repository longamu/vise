#include "vise/search_engine.h"

vise::search_engine::search_engine(const std::string search_engine_id,
                                  const boost::filesystem::path dset_fn,
                                  const boost::filesystem::path clst_fn,
                                  const boost::filesystem::path iidx_fn,
                                  const boost::filesystem::path fidx_fn,
                                  const boost::filesystem::path wght_fn,
                                  const boost::filesystem::path image_dir) {
  is_search_engine_loaded_ = false;
  search_engine_id_ = search_engine_id;

  dset_fn_ = dset_fn;
  clst_fn_ = clst_fn;
  iidx_fn_ = iidx_fn;
  fidx_fn_ = fidx_fn;
  wght_fn_ = wght_fn;
  image_dir_ = image_dir_;
}


bool vise::search_engine::load() {
  BOOST_LOG_TRIVIAL(debug) << "loading dataset from " << dset_fn_;
  //dset_ = datasetV2( dset_fn_, image_dir_ );
  BOOST_LOG_TRIVIAL(debug) << "done loading dataset";
}
