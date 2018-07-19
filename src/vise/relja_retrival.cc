#include "vise/relja_retrival.h"

vise::relja_retrival::relja_retrival(const std::string search_engine_id,
                                     const boost::filesystem::path data_dir,
                                     const boost::filesystem::path asset_dir,
                                     const boost::filesystem::path temp_dir) {
  is_search_engine_loaded_ = false;
  search_engine_id_ = search_engine_id;
  data_dir_  = data_dir;
  asset_dir_ = asset_dir;
  temp_dir_  = temp_dir;

  image_dir_     = asset_dir_ / "image";
  thumbnail_dir_ = asset_dir_ / "thumbnail";
  original_dir_  = asset_dir_ / "original";

  dset_fn_   = data_dir_ / "dset.v2bin";
  clst_fn_   = data_dir_ / "clst.e3bin";
  iidx_fn_   = data_dir_ / "iidx.v2bin";
  fidx_fn_   = data_dir_ / "fidx.v2bin";
  wght_fn_   = data_dir_ / "wght.v2bin";
  hamm_fn_   = data_dir_ / "hamm.v2bin";
  assign_fn_ = data_dir_ / "assign.bin";

  imlist_fn_ = data_dir_ / "imlist.txt";
  config_fn_ = data_dir_ / "config.txt";
}

std::string vise::relja_retrival::id() {
  return search_engine_id_;
}

bool vise::relja_retrival::init() {
  load_config();
}

bool vise::relja_retrival::is_load_possible() { }

bool vise::relja_retrival::load() {
  // critical section using a mutex
  // ensures exclusive access to a single thread
  load_mutex_.lock();

  if ( is_search_engine_loaded_ ) {
    load_mutex_.unlock();
    return is_search_engine_loaded_;
  }

  try {
    // construct dataset
    BOOST_LOG_TRIVIAL(debug) << "loading dataset from " << dset_fn_;
    dataset_ = new datasetV2( dset_fn_.string(), image_dir_.string() + "/" );
    BOOST_LOG_TRIVIAL(debug) << "done loading dataset";

    // needed to setup forward and inverted index
    BOOST_LOG_TRIVIAL(debug) << "setting up forward and inverted index ";
    cons_queue_ = new sequentialConstructions();
    // setup forward index
    dbFidx_file_ = new protoDbFile( fidx_fn_.string() );
    boost::function<protoDb*()> fidxInRamConstructor= boost::lambda::bind(boost::lambda::new_ptr<protoDbInRam>(),
                                                                          boost::cref(*dbFidx_file_) );
    dbFidx_ = new protoDbInRamStartDisk( *dbFidx_file_, fidxInRamConstructor, true, cons_queue_ );
    fidx_ = new protoIndex(*dbFidx_, false);
    BOOST_LOG_TRIVIAL(debug) << "forward index done";

    // setup inverted index
    dbIidx_file_ = new protoDbFile( iidx_fn_.string() );
    boost::function<protoDb*()> iidxInRamConstructor= boost::lambda::bind(boost::lambda::new_ptr<protoDbInRam>(),
                                                                          boost::cref(*dbIidx_file_) );
    dbIidx_ = new protoDbInRamStartDisk( *dbIidx_file_, iidxInRamConstructor, true, cons_queue_ );
    iidx_ = new protoIndex(*dbIidx_, false);
    BOOST_LOG_TRIVIAL(debug) << "inverted index done";
    cons_queue_->start(); // start the construction of in-RAM stuff

    // feature getter and assigner
    BOOST_LOG_TRIVIAL(debug) << "loading feature getter ";
    bool SIFTscale3  = false;
    if ( config_.get<std::string>(search_engine_id_ + ".SIFTscale3") == "true" ) {
      SIFTscale3 = true;
    }

    bool useRootSIFT = false;
    if ( config_.get<std::string>(search_engine_id_ + ".RootSIFT") == "true" ) {
      useRootSIFT = true;
    }

    feat_getter_ = new featGetter_standard( (
                                             std::string("hesaff-") +
                                             std::string((useRootSIFT ? "rootsift" : "sift")) +
                                             std::string(SIFTscale3 ? "-scale3" : "")
                                             ).c_str() );

    // clusters
    BOOST_LOG_TRIVIAL(debug) << "loading clusters";
    clst_centres_ = new clstCentres( clst_fn_.c_str(), true );

    nn_ = fastann::nn_obj_build_kdtree(clst_centres_->clstC_flat,
                                       clst_centres_->numClst,
                                       clst_centres_->numDims, 8, 1024);

    // soft assigner
    BOOST_LOG_TRIVIAL(debug) << "loading assigner";
    use_hamm_ = false;
    uint32_t hamm_emb_bits;
    std::string hamm_emb_bits_str = config_.get<std::string>(search_engine_id_ + ".hammEmbBits");
    std::istringstream s(hamm_emb_bits_str);
    s >> hamm_emb_bits;
    if ( hamm_emb_bits_str != "" ) {
      use_hamm_ = true;
    }

    if ( ! use_hamm_ ) {
      if ( useRootSIFT ) {
        soft_assigner_ = new SA_exp( 0.02 );
      } else {
        soft_assigner_ = new SA_exp( 6250 );
      }
    }
    if ( use_hamm_ ) {
      emb_factory_ = new hammingEmbedderFactory(hamm_fn_.string(), hamm_emb_bits);
    }
    else {
      emb_factory_ = new noEmbedderFactory;
    }

    // create retriever
    BOOST_LOG_TRIVIAL(debug) << "loading retriever";
    tfidf_ = new tfidfV2(iidx_,
                         fidx_,
                         wght_fn_.string(),
                         feat_getter_,
                         nn_,
                         soft_assigner_);

    if ( use_hamm_ ) {
      hamming_emb_ = new hamming(*tfidf_,
                                 iidx_,
                                 *dynamic_cast<hammingEmbedderFactory const *>(emb_factory_),
                                 fidx_,
                                 feat_getter_, nn_, clst_centres_);
      base_retriever_ = hamming_emb_;
    } else {
      base_retriever_ = tfidf_;
    }

    // spatial verifier
    BOOST_LOG_TRIVIAL(debug) << "loading spatial verifier";
    spatial_verif_v2_ = new spatialVerifV2(*base_retriever_, iidx_, fidx_, true, feat_getter_, nn_, clst_centres_);
    spatial_retriever_ = spatial_verif_v2_;

    // multiple queries
    BOOST_LOG_TRIVIAL(debug) << "loading multiple queries";
    multi_query_max_ = new multiQueryMax( *spatial_verif_v2_ );
    if (hamming_emb_ != NULL){
      multi_query_= new mqFilterOutliers(*multi_query_max_,
                                         *spatial_verif_v2_,
                                         *dynamic_cast<hammingEmbedderFactory const *>(emb_factory_) );
    } else {
      multi_query_ = multi_query_max_;
    }
    BOOST_LOG_TRIVIAL(debug) << "finished loading search engine [" << search_engine_id_ << "]";
    is_search_engine_loaded_ = true;
  } catch( std::exception& e ) {
    is_search_engine_loaded_ = false;
    BOOST_LOG_TRIVIAL(debug) << "failed to load search engine [" << search_engine_id_ << "]";
    BOOST_LOG_TRIVIAL(debug) << e.what();
  }
  load_mutex_.unlock();
  return is_search_engine_loaded_;
}

bool vise::relja_retrival::unload() {
  BOOST_LOG_TRIVIAL(debug) << "vise::relja_retrival::unload()";

  delete cons_queue_;

  if ( hamming_emb_ != NULL ) {
    delete hamming_emb_;
    delete multi_query_max_;
  }
  if ( multi_query_ != NULL ) {
    delete multi_query_;
  }
  delete emb_factory_;

  if ( ! use_hamm_ ) {
    delete soft_assigner_;
  }

  delete nn_;
  delete clst_centres_;
  delete feat_getter_;

  delete fidx_;
  delete iidx_;

  delete tfidf_;

  delete dataset_;

  delete dbFidx_;
  delete dbIidx_;
}

bool vise::relja_retrival::is_loaded() {
  return is_search_engine_loaded_;
}

bool vise::relja_retrival::query_using_upload_region() { }
bool vise::relja_retrival::query_using_file_region(uint32_t file_id,
                                                   unsigned int x, unsigned int y, unsigned int w, unsigned int h,
                                                   uint32_t from, uint32_t to, double score_threshold,
                                                   std::vector<unsigned int> &result_file_id,
                                                   std::vector<std::string> &result_filename,
                                                   std::vector<std::string> &result_metadata,
                                                   std::vector<float> &result_score,
                                                   std::vector< std::array<double, 9> > &result_H) {
  query qobj(file_id, true, "", x, x + w, y, y + h);
  std::vector<indScorePair> result;
  std::map<uint32_t, homography> H;
  spatial_retriever_->spatialQuery(qobj, result, H);

  if ( to > result.size() ) {
    to = result.size();
  }

  BOOST_LOG_TRIVIAL(debug) << "query(): total files = " << dataset_->getNumDoc();
  BOOST_LOG_TRIVIAL(debug) << "query(): search result = " << result.size();
  if ( from > result.size() || to > result.size() ) {
    BOOST_LOG_TRIVIAL(debug) << "query(): 0 <= {from,to} <= " << result.size();
  }

  std::cout << "\nquery(): Showing results from " << from << " to " << to;

  unsigned int n = to - from;
  result_file_id.clear();
  result_file_id.reserve(n);
  result_score.clear();
  result_score.reserve(n);
  result_H.clear();
  result_H.reserve(n);

  for ( uint32_t i = from; i < to; ++i ) {
    uint32_t file_id = result[i].first;
    double score = result[i].second;
    std::string filename = get_filename(file_id);
    std::string metadata = ""; // @todo: fill with file metadata

    if ( score > score_threshold ) {
      result_file_id.push_back(file_id);
      result_score.push_back(score);
      result_filename.push_back(filename);
      result_metadata.push_back(metadata);
      std::array<double, 9> hi;
      hi[0] = H[i].H[0];
      hi[1] = H[i].H[1];
      hi[2] = H[i].H[2];
      hi[3] = H[i].H[3];
      hi[4] = H[i].H[4];
      hi[5] = H[i].H[5];
      hi[6] = H[i].H[6];
      hi[7] = H[i].H[7];
      hi[8] = H[i].H[8];
      result_H.push_back(hi);

      std::cout << "\n  [" << i << "], file_id=" << file_id
                << ", score=" << result[i].second
                << ", file=" << dataset_->getFn( file_id )
                << ", H = " << H[i].H[0] << "," << H[i].H[1] << "," << H[i].H[2] << ","
                << H[i].H[3] << "," << H[i].H[4] << "," << H[i].H[5] << ","
                << H[i].H[6] << "," << H[i].H[7] << "," << H[i].H[8]
                << std::flush;
    }
  }
  std::cout << "\nquery(): results end";
}

void vise::relja_retrival::get_filelist(const unsigned int from, const unsigned int to,
                                        std::vector<uint32_t> &file_id_list,
                                        std::vector<std::string> &filename_list) {
  // validation of from and to
  if ( from < 0 || from > dataset_->getNumDoc() ) {
    BOOST_LOG_TRIVIAL(debug) << "get_filelist(): from=" << from
                             << " : 0 <= from < " << dataset_->getNumDoc();
    return;
  }

  if ( to < 0 || to > dataset_->getNumDoc() ) {
    BOOST_LOG_TRIVIAL(debug) << "get_filelist(): to=" << to
                             << " : 0 <= to < " << dataset_->getNumDoc();
    return;
  }

  if ( to < from ) {
    BOOST_LOG_TRIVIAL(debug) << "get_filelist(): to=" << to
                             << " > from=" << from;
    return;
  }

  BOOST_LOG_TRIVIAL(debug) << "get_filelist(): from=" << from << ", to=" << to;

  unsigned int n = to - from;
  file_id_list.clear();
  file_id_list.reserve(n);
  filename_list.clear();
  filename_list.reserve(n);

  for ( unsigned int i = from; i < to; ++i ) {
    file_id_list.push_back(i);
    filename_list.push_back( dataset_->getInternalFn(i) );
  }
}

uint32_t vise::relja_retrival::get_filelist_size() {
  return dataset_->getNumDoc();
}

std::string vise::relja_retrival::get_filename(unsigned int file_id) {
  return dataset_->getInternalFn(file_id);
}

bool vise::relja_retrival::file_exists(std::string filename) {
  return dataset_->containsFn(filename);
}
bool vise::relja_retrival::file_exists(unsigned int file_id) {
  if ( file_id < 0 || file_id > dataset_->getNumDoc() ) {
    return false;
  } else {
    return false;
  }
}

std::string vise::relja_retrival::get_filename_absolute_path(std::string filename) {
  boost::filesystem::path fn_abs_path = image_dir_ / filename;
  return fn_abs_path.string();
}
std::string vise::relja_retrival::get_filename_absolute_path(unsigned int file_id) {
  boost::filesystem::path fn_abs_path = image_dir_ / get_filename(file_id);
  return fn_abs_path.string();
}

bool vise::relja_retrival::index() { }

// helper functions
void vise::relja_retrival::save_config() {
  try {
    boost::property_tree::ini_parser::write_ini(config_fn_.string(), config_);
    BOOST_LOG_TRIVIAL(debug) << "written search engine config to ["
                             << config_fn_.string() << "]";
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while writing config file ["
                             << config_fn_.string() << "] : " << e.what();
  }
}

void vise::relja_retrival::load_config() {
  try {
    config_.clear();
    boost::property_tree::ini_parser::read_ini(config_fn_.string(), config_);
    BOOST_LOG_TRIVIAL(debug) << "loaded config from file [" << config_fn_.string() << "]";
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while loading config file [" << config_fn_.string() << "] : " << e.what();
  }
}

void vise::relja_retrival::set_default_config() {
  try {
    config_.clear();
    config_.put( search_engine_id_ + ".RootSIFT", "true");
    config_.put( search_engine_id_ + ".SIFTscale3", "true");
    config_.put( search_engine_id_ + ".hammEmbBits", "64");
    config_.put( search_engine_id_ + ".imagelistFn", imlist_fn_);
    config_.put( search_engine_id_ + ".databasePath", image_dir_ );
    config_.put( search_engine_id_ + ".trainFilesPrefix", data_dir_ );
    config_.put( search_engine_id_ + ".dsetFn", dset_fn_ );
    config_.put( search_engine_id_ + ".clstFn", clst_fn_ );
    config_.put( search_engine_id_ + ".iidxFn", iidx_fn_ );
    config_.put( search_engine_id_ + ".fidxFn", fidx_fn_ );
    config_.put( search_engine_id_ + ".wghtFn", wght_fn_ );
    config_.put( search_engine_id_ + ".tmpDir", temp_dir_);
    config_.put( search_engine_id_ + ".trainNumDescs", "-1");
    config_.put( search_engine_id_ + ".vocSize", "-1");
    save_config();
    BOOST_LOG_TRIVIAL(debug) << "written search engine config to ["
                             << config_fn_.string() << "]";
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while writing config file ["
                             << config_fn_.string() << "] : " << e.what();
  }
}

void vise::relja_retrival::set_config(const std::string name,
                                      const std::string value) {
  try {
    config_.put( search_engine_id_ + "." + name, value);
    save_config();
    BOOST_LOG_TRIVIAL(debug) << "updated search engine config " << name << "=" << value << "in config file [" << config_fn_.string() << "]";
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while updating config file [" << config_fn_.string() << "] : " << e.what();
  }
}
