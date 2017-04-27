#include "SearchEngine.h"

SearchEngine::SearchEngine() {
  engine_name_ = "";
}

void SearchEngine::Init(std::string name, boost::filesystem::path basedir) {
  basedir_ = boost::filesystem::path(basedir);
  engine_name_ = name;

  boost::filesystem::path engine_name( engine_name_ );
  enginedir_ = basedir_ / engine_name;

  engine_config_fn_    = enginedir_ / "user_settings.txt";
  transformed_imgdir_  = enginedir_ / "img";
  training_datadir_    = enginedir_ / "training_data";
  tmp_datadir_         = enginedir_ / "tmp";

  imglist_fn_ = training_datadir_ / "imlist.txt";
  engine_config_fn_ = training_datadir_ / "vise_config.cfg";

  transformed_imgdir_ += boost::filesystem::path::preferred_separator;
  training_datadir_ += boost::filesystem::path::preferred_separator;
  tmp_datadir_ += boost::filesystem::path::preferred_separator;

  // create directory
  if ( ! boost::filesystem::exists( enginedir_ ) ) {
    boost::filesystem::create_directory( enginedir_ );
    boost::filesystem::create_directory( transformed_imgdir_ );
    boost::filesystem::create_directory( training_datadir_ );
    std::cout << "\n" << engine_name_ << " search engine created" << std::flush;
  } else {
    if ( ! boost::filesystem::exists( transformed_imgdir_ ) ) {
      boost::filesystem::create_directory( transformed_imgdir_ );
    }
    if ( ! boost::filesystem::exists( training_datadir_ ) ) {
      boost::filesystem::create_directory( training_datadir_ );
    }
    imglist_fn_ = training_datadir_ / "imlist.txt";

    if ( ! boost::filesystem::exists( tmp_datadir_ ) ) {
      boost::filesystem::create_directory( tmp_datadir_ );
    }

    std::cout << "\n" << engine_name_ << " search engine loaded" << std::flush;
  }
}

//
// Workers for each state
//
void SearchEngine::Preprocess() {
  SendStatus("Preprocess", "\nPreprocessing started ...");

  // scale and copy image to transformed_imgdir_
  SendStatus("Preprocess",
             "\nSaving transformed images to [" + transformed_imgdir_.string() + "] ");

  for ( unsigned int i=0; i<imglist_.size(); i++ ) {
    boost::filesystem::path img_rel_path = imglist_.at(i);
    boost::filesystem::path src_fn  = original_imgdir_ / img_rel_path;
    boost::filesystem::path dest_fn = transformed_imgdir_ / img_rel_path;

    if ( !boost::filesystem::exists( dest_fn ) ) {
      try {
        Magick::Image im;
        im.read( src_fn.string() );
        Magick::Geometry size = im.size();

        double aspect_ratio =  ((double) size.height()) / ((double) size.width());
        unsigned int new_width = 400;
        unsigned int new_height = (unsigned int) (new_width * aspect_ratio);

        Magick::Geometry resize = Magick::Geometry(new_width, new_height);
        resize.greater(true); // Resize if image is greater than size (>)

        std::ostringstream info;
        info << std::string(size) << " -> " << std::string(resize);
        im.zoom( resize );

        // check if image path exists
        if ( ! boost::filesystem::is_directory( dest_fn.parent_path() ) ) {
          boost::filesystem::create_directories( dest_fn.parent_path() );
        }
        im.write( dest_fn.string() );
        SendStatus("Preprocess", ".");
      } catch (std::exception &error) {
        SendStatus("Preprocess",
                   "\n" + src_fn.string() + " : Error [" + error.what() + "]" );
      }
    }
  }
  SendStatus("Preprocess", "[Done]");

  //if ( ! boost::filesystem::exists( imglist_fn_ ) ) {
  WriteImageListToFile( imglist_fn_.string(), imglist_ );
  SendStatus("Preprocess",
             "\nWritten image list to : [" + imglist_fn_.string() + "]" );

  // save full configuration file to training_datadir_
  SetEngineConfigParam("trainDatabasePath", transformed_imgdir_.string());
  SetEngineConfigParam("databasePath", transformed_imgdir_.string());
  SetEngineConfigParam("trainImagelistFn",  imglist_fn_.string());
  SetEngineConfigParam("imagelistFn",  imglist_fn_.string());
  boost::filesystem::path train_file_prefix = training_datadir_ / "train_";
  SetEngineConfigParam("trainFilesPrefix", train_file_prefix.string() );
  SetEngineConfigParam("pathManHide", transformed_imgdir_.string() );
  SetEngineConfigParam("descFn", train_file_prefix.string() + "descs.e3bin" );
  SetEngineConfigParam("assignFn", train_file_prefix.string() + "assign.bin" );
  SetEngineConfigParam("hammFn", train_file_prefix.string() + "hamm.v2bin" );
  SetEngineConfigParam("dsetFn", train_file_prefix.string() + "dset.v2bin" );
  SetEngineConfigParam("clstFn", train_file_prefix.string() + "clst.e3bin" );
  SetEngineConfigParam("iidxFn", train_file_prefix.string() + "iidx.e3bin" );
  SetEngineConfigParam("fidxFn", train_file_prefix.string() + "fidx.e3bin" );
  SetEngineConfigParam("wghtFn", train_file_prefix.string() + "wght.e3bin" );
  SetEngineConfigParam("tmpDir", tmp_datadir_.string());

  WriteConfigToFile();
  SendStatus("Preprocess",
             "\nWritten search engine configuration to : [" + engine_config_fn_.string() + "]" );
}

void SearchEngine::Descriptor() {
  std::string const trainDescsFn  = GetEngineConfigParam("descFn");
  boost::filesystem::path train_desc_fn( trainDescsFn );

  if ( boost::filesystem::exists( train_desc_fn ) ) {
    // delete file
    boost::filesystem::remove( train_desc_fn );
    SendStatus("Descriptor", "\nDeleted old training descriptors ...");
  }

  SendStatus("Descriptor", "\nComputing training descriptors ...");
  std::string const trainImagelistFn = GetEngineConfigParam("trainImagelistFn");
  std::string const trainDatabasePath = GetEngineConfigParam("trainDatabasePath");

  int32_t trainNumDescs;
  std::stringstream s;
  s << GetEngineConfigParam("trainNumDescs");
  s >> trainNumDescs;

  bool SIFTscale3 = false;
  if ( GetEngineConfigParam("SIFTscale3") == "on" ) {
    SIFTscale3 = true;
  }

  // source: src/v2/indexing/compute_index_v2.cpp
  featGetter_standard const featGetter_obj( (
                                             std::string("hesaff-") +
                                             "sift" +
                                             std::string(SIFTscale3 ? "-scale3" : "")
                                             ).c_str() );

  buildIndex::computeTrainDescs(trainImagelistFn, trainDatabasePath,
                                trainDescsFn,
                                trainNumDescs,
                                featGetter_obj);
}

// ensure that you install the dkmeans_relja as follows
// $ cd src/external/dkmeans_relja/
// $ python setup.py build
// $ sudo python setup.py install
void SearchEngine::Cluster() {
  SendStatus("Cluster", "\nStarting clustering of descriptors ...");

  boost::thread t( boost::bind( &SearchEngine::RunClusterCommand, this ) );
}

void SearchEngine::RunClusterCommand() {
  // @todo: avoid relative path and discover the file "compute_clusters.py" automatically
  std::string cmd = "python ../src/v2/indexing/compute_clusters.py";
  cmd += " " + engine_name_;
  cmd += " " + engine_config_fn_.string();

  FILE *pipe = popen( cmd.c_str(), "r");

  if ( pipe != NULL ) {
    SendPacket("Cluster", "status", "\nCommand executed: $" + cmd);

    char line[128];
    while ( fgets(line, 64, pipe) ) {
      std::string status_txt(line);
      SendStatus("Cluster", status_txt);
    }
    pclose( pipe );
  } else {
    SendStatus("Cluster",
               "\nFailed to execute python script for clustering: \n\t $" + cmd);
  }
}

void SearchEngine::Assignment() {
  SendStatus("Assign", "\nStarting assignment ...");
  bool useRootSIFT = false;
  if ( GetEngineConfigParam("RootSIFT") == "on" ) {
    useRootSIFT = true;
  }

  buildIndex::computeTrainAssigns( GetEngineConfigParam("clstFn"),
                                   useRootSIFT,
                                   GetEngineConfigParam("descFn"),
                                   GetEngineConfigParam("assignFn"));

}

void SearchEngine::Hamm() {
  SendStatus("Hamm", "\nComputing hamm ...");
  uint32_t hammEmbBits;
  std::string hamm_emb_bits = GetEngineConfigParam("hammEmbBits");
  std::istringstream s(hamm_emb_bits);
  s >> hammEmbBits;

  bool useRootSIFT = false;
  if ( GetEngineConfigParam("RootSIFT") == "on" ) {
    useRootSIFT = true;
  }

  buildIndex::computeHamming(GetEngineConfigParam("clstFn"),
                             useRootSIFT,
                             GetEngineConfigParam("descFn"),
                             GetEngineConfigParam("assignFn"),
                             GetEngineConfigParam("hammFn"),
                             hammEmbBits);

}

void SearchEngine::Index() {
  SendStatus("Index", "\nStarting indexing ...");
  bool SIFTscale3 = false;
  if ( GetEngineConfigParam("SIFTscale3") == "on" ) {
    SIFTscale3 = true;
  }

  bool useRootSIFT = false;
  if ( GetEngineConfigParam("RootSIFT") == "on" ) {
    useRootSIFT = true;
  }

  // source: src/v2/indexing/compute_index_v2.cpp
  std::ostringstream feat_getter_config;
  feat_getter_config << "hesaff-";
  if (useRootSIFT) {
    feat_getter_config << "rootsift";
  } else {
    feat_getter_config << "sift";
  }
  if (SIFTscale3) {
    feat_getter_config << "-scale3";
  }
  featGetter_standard const featGetter_obj( feat_getter_config.str().c_str() );

  // embedder
  uint32_t hammEmbBits;
  std::string hamm_emb_bits = GetEngineConfigParam("hammEmbBits");
  std::istringstream s(hamm_emb_bits);
  s >> hammEmbBits;

  embedderFactory *embFactory= NULL;
  if ( EngineConfigParamExists("hammEmbBits") ) {
    embFactory= new hammingEmbedderFactory(GetEngineConfigParam("hammFn"), hammEmbBits);
  } else {
    embFactory= new noEmbedderFactory;
  }

  buildIndex::build(GetEngineConfigParam("imagelistFn"),
                    GetEngineConfigParam("databasePath"),
                    GetEngineConfigParam("dsetFn"),
                    GetEngineConfigParam("iidxFn"),
                    GetEngineConfigParam("fidxFn"),
                    GetEngineConfigParam("tmpDir"),
                    featGetter_obj,
                    GetEngineConfigParam("clstFn"),
                    embFactory);

  delete embFactory;
}

//
// Helper functions
//
bool SearchEngine::EngineConfigExists() {
  if ( is_regular_file(engine_config_fn_) ) {
    return true;
  } else {
    return false;
  }
}

void SearchEngine::CreateFileList(boost::filesystem::path dir,
                                  std::vector< std::string > &filelist) {

  //std::cout << "\nShowing directory contents of : " << dir.string() << std::endl;
  filelist.clear();
  boost::filesystem::recursive_directory_iterator dir_it( dir ), end_it;

  std::string basedir = dir.string();
  while ( dir_it != end_it ) {
    boost::filesystem::path p = dir_it->path();
    std::string fn_dir = p.parent_path().string();
    std::string rel_path = fn_dir.replace(0, basedir.length(), "");
    boost::filesystem::path rel_fn = boost::filesystem::path(rel_path) / p.filename();
    if ( boost::filesystem::is_regular_file(p) ) {
      /*
      // todo : add only image files which can be read by VISE
      std::string fn_ext = p.extension().string();
      if ( acceptable_types.count(fn_ext) == 1 ) {
        std::cout << rel_fn << std::endl;
        filelist << rel_fn.string() << std::endl;
      }
      */
      filelist.push_back( rel_fn.string() );
    }
    ++dir_it;
  }
  original_imgdir_ = dir;
}

// data_str =
// key1=value1
// key2=value2
// ...
void SearchEngine::SetEngineConfig(std::string engine_config) {
  std::istringstream d(engine_config);

  std::string keyval;
  while( std::getline(d, keyval, '\n') ) {
    unsigned int sep_idx = keyval.find('=');
    std::string key = keyval.substr(0, sep_idx);
    std::string val = keyval.substr(sep_idx+1);

    engine_config_.insert( std::make_pair<std::string, std::string>(key, val) );
  }
  update_engine_overview_ = true;
}

std::string SearchEngine::GetEngineConfigParam(std::string key) {
  std::map<std::string, std::string>::iterator it;
  it = engine_config_.find( key );

  if ( it != engine_config_.end() ) {
    return it->second;
  }
  else {
    return "";
  }
}

bool SearchEngine::EngineConfigParamExists(std::string key) {
  std::map<std::string, std::string>::iterator it;
  it = engine_config_.find( key );

  if ( it != engine_config_.end() ) {
    return true;
  }
  else {
    return false;
  }
}

void SearchEngine::SetEngineConfigParam(std::string key, std::string value) {
  std::map<std::string, std::string>::iterator it;
  it = engine_config_.find( key );

  if ( it != engine_config_.end() ) {
    it->second = value;
  }
  else {
    engine_config_.insert( std::make_pair<std::string, std::string>(key, value) );
  }
}

std::string SearchEngine::GetName() {
  return engine_name_;
}
boost::filesystem::path SearchEngine::GetEngineConfigPath() {
  return engine_config_fn_;
}
std::string SearchEngine::GetSearchEngineBaseUri() {
  return "/" + engine_name_ + "/";
}

std::string SearchEngine::GetEngineOverview() {
  if ( update_engine_overview_ ) {
    UpdateEngineOverview();
  }
  return engine_overview_.str();
}

void SearchEngine::UpdateEngineOverview() {
  engine_overview_.str("");
  engine_overview_.clear();

  boost::filesystem::path imagePath(GetEngineConfigParam("imagePath"));
  CreateFileList( imagePath, imglist_);

  engine_overview_ << "<h3>Overview of Search Engine</h3>";
  engine_overview_ << "<table id=\"engine_overview\">";
  engine_overview_ << "<tr><td># of images</td><td>" << imglist_.size() << "</td></tr>";
  engine_overview_ << "<tr><td>Training time*</td><td>4 hours</td></tr>";
  engine_overview_ << "<tr><td>Memory required*</td><td>3 GB</td></tr>";
  engine_overview_ << "<tr><td>Disk space required*</td><td>10 GB</td></tr>";
  engine_overview_ << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
  engine_overview_ << "<tr><td><input type=\"button\" onclick=\"_vise_send_post_request('back')\" value=\"Back\"></td><td><input type=\"button\" onclick=\"_vise_send_post_request('proceed')\" value=\"Proceed\"></td></tr>";
  engine_overview_ << "</table>";
  engine_overview_ << "<p>&nbsp;</p><p>* : todo</p>";
}

void SearchEngine::WriteImageListToFile(const std::string fn,
                                        const std::vector< std::string > &imlist) {
  try {
    std::ofstream f( fn.c_str() );
    for ( unsigned int i=0; i < imlist.size(); i++) {
      f << imlist.at(i) << "\n";
    }
    f.close();
  } catch( std::exception &e) {
    std::cerr << "\nSearchEngine::WriteImageListToFile() : error writing image filename list to [" << fn << "]" << std::flush;
    std::cerr << e.what() << std::flush;
  }
}

void SearchEngine::WriteConfigToFile() {
  try {
    std::map<std::string, std::string>::iterator it;
    std::ofstream f( engine_config_fn_.c_str() );

    f << "[" << engine_name_ << "]";
    for ( it = engine_config_.begin(); it != engine_config_.end(); ++it) {
      f << "\n" << it->first << "=" << it->second;
    }
    f.close();
  } catch( std::exception &e) {
    std::cerr << "\nSearchEngine::WriteConfigToFile() : error writing configuration to [" << engine_config_fn_ << "]" << std::flush;
    std::cerr << e.what() << std::flush;
  }
}

void SearchEngine::SendStatus(std::string sender, std::string message) {
  SendPacket(sender, "status", message);
}

void SearchEngine::SendPacket(std::string sender, std::string type, std::string message) {
  std::ostringstream s;
  s << sender << " " << type << " " << message;
  vise_message_queue_.Push( s.str() );
}

// for debug
void SearchEngine::PrintEngineConfig() {
  std::cout << "\nShowing configurations for [" << engine_name_ << "] :" << std::endl;
  std::map<std::string, std::string>::iterator it;
  for ( it = engine_config_.begin(); it != engine_config_.end(); ++it) {
    std::cout << it->first << " = " << it->second << std::endl;
  }
  std::cout << std::flush;
}
