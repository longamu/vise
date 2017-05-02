#include "SearchEngine.h"

SearchEngine::SearchEngine() {
  engine_name_ = "";
  engine_config_.clear();

  acceptable_img_ext_.insert( ".jpg" );
  acceptable_img_ext_.insert( ".jpeg" );
  acceptable_img_ext_.insert( ".png" );
  acceptable_img_ext_.insert( ".pgm" );
  acceptable_img_ext_.insert( ".pnm" );
  acceptable_img_ext_.insert( ".ppm" );
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
    boost::filesystem::create_directory( tmp_datadir_ );
    std::cout << "\n" << engine_name_ << " search engine created" << std::flush;
  } else {
    if ( ! boost::filesystem::exists( transformed_imgdir_ ) ) {
      boost::filesystem::create_directory( transformed_imgdir_ );
    }
    if ( ! boost::filesystem::exists( training_datadir_ ) ) {
      boost::filesystem::create_directory( training_datadir_ );
    }
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
  if ( imglist_.empty() ) {
    boost::filesystem::path imagePath(GetEngineConfigParam("imagePath"));
    CreateFileList( imagePath );
  }

  if ( boost::filesystem::exists( imglist_fn_ ) ) {
    SendLog("Preprocess", "\nLoaded");
  } else {
    SendLog("Preprocess", "\nPreprocessing started ...");
    std::string transformed_img_width = GetEngineConfigParam("transformed_img_width");
    if (transformed_img_width != "original") {
      // scale and copy image to transformed_imgdir_
      SendLog("Preprocess", "\nSaving transformed images to [" + transformed_imgdir_.string() + "] ");
    } else {
      SendLog("Preprocess", "\nCopying original images to [" + transformed_imgdir_.string() + "] ");
    }
    for ( unsigned int i=0; i<imglist_.size(); i++ ) {
      boost::filesystem::path img_rel_path = imglist_.at(i);
      boost::filesystem::path src_fn  = original_imgdir_ / img_rel_path;
      boost::filesystem::path dest_fn = transformed_imgdir_ / img_rel_path;

      if ( !boost::filesystem::exists( dest_fn ) ) {
        try {
          // check if image path exists
          if ( ! boost::filesystem::is_directory( dest_fn.parent_path() ) ) {
            boost::filesystem::create_directories( dest_fn.parent_path() );
          }

          if (transformed_img_width != "original") {
            Magick::Image im;
            im.read( src_fn.string() );
            Magick::Geometry size = im.size();

            double aspect_ratio =  ((double) size.height()) / ((double) size.width());

            std::stringstream s;
            s << transformed_img_width;
            unsigned int new_width;
            s >> new_width;
            unsigned int new_height = (unsigned int) (new_width * aspect_ratio);

            Magick::Geometry resize = Magick::Geometry(new_width, new_height);
            im.zoom( resize );

            im.write( dest_fn.string() );
            imglist_fn_transformed_size_.at(i) = boost::filesystem::file_size(dest_fn.string().c_str());
          } else {
            // just copy the files
            boost::filesystem::copy_file( src_fn, dest_fn );
            imglist_fn_transformed_size_.at(i) = imglist_fn_original_size_.at(i);
          }
          if ( (i % 50) == 0 ) {
            SendLog("Preprocess", ".");
          }
        } catch (std::exception &error) {
          SendLog("Preprocess", "\n" + src_fn.string() + " : Error [" + error.what() + "]" );
        }
      }
    }
    SendLog("Preprocess", "[Done]");

    //if ( ! boost::filesystem::exists( imglist_fn_ ) ) {
    WriteImageListToFile( imglist_fn_.string(), imglist_ );
    SendLog("Preprocess",
            "\nWritten image list to : [" + imglist_fn_.string() + "]" );
  }
}

void SearchEngine::Descriptor() {
  std::string const trainDescsFn  = GetEngineConfigParam("descFn");
  boost::filesystem::path train_desc_fn( trainDescsFn );

  if ( boost::filesystem::exists( train_desc_fn ) ) {
    // delete file
    //boost::filesystem::remove( train_desc_fn );
    SendLog("Descriptor", "\nLoaded");
  } else {
    SendLog("Descriptor", "\nComputing training descriptors ...");
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
}

// ensure that you install the dkmeans_relja as follows
// $ cd src/external/dkmeans_relja/
// $ python setup.py build
// $ sudo python setup.py install
void SearchEngine::Cluster() {
  if ( ! ClstFnExists() ) {
    SendLog("Cluster", "\nStarting clustering of descriptors ...");

    //boost::thread t( boost::bind( &SearchEngine::RunClusterCommand, this ) );
    RunClusterCommand();
  } else {
    SendLog("Cluster", "\nLoaded");
  }
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
      SendLog("Cluster", status_txt);
    }
    pclose( pipe );
  } else {
    SendLog("Cluster",
               "\nFailed to execute python script for clustering: \n\t $" + cmd);
  }
}

void SearchEngine::Assign() {
  if ( ! AssignFnExists() ) {
    SendLog("Assign", "\nStarting assignment ...");
    bool useRootSIFT = false;
    if ( GetEngineConfigParam("RootSIFT") == "on" ) {
      useRootSIFT = true;
    }

    buildIndex::computeTrainAssigns( GetEngineConfigParam("clstFn"),
                                     useRootSIFT,
                                     GetEngineConfigParam("descFn"),
                                     GetEngineConfigParam("assignFn"));
  } else {
    SendLog("Assign", "\nLoaded");
  }
}

void SearchEngine::Hamm() {
  if ( ! HammFnExists() ) {
    SendLog("Hamm", "\nComputing hamm ...");
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
  } else {
    SendLog("Hamm", "\nLoaded");
  }
}

void SearchEngine::Index() {
  if ( ! IndexFnExists() ) {
    SendLog("Index", "\nStarting indexing ...");
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
  } else {
    SendLog("Index", "\nLoaded");
  }
}

void SearchEngine::Query() {
  // @todo-query implement image query
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

void SearchEngine::CreateFileList(boost::filesystem::path dir ) {

  //std::cout << "\nShowing directory contents of : " << dir.string() << std::endl;
  imglist_.clear();
  imglist_fn_original_size_.clear();
  imglist_fn_transformed_size_.clear();
  boost::filesystem::recursive_directory_iterator dir_it( dir ), end_it;

  std::string basedir = dir.string();
  std::locale locale;
  while ( dir_it != end_it ) {
    boost::filesystem::path p = dir_it->path();
    std::string fn_dir = p.parent_path().string();
    std::string rel_path = fn_dir.replace(0, basedir.length(), "");
    boost::filesystem::path rel_fn = boost::filesystem::path(rel_path) / p.filename();
    if ( boost::filesystem::is_regular_file(p) ) {
      // add only image files which can be read by VISE
      std::string fn_ext = p.extension().string();

      // convert fn_ext to lower case
      for ( unsigned int i=0; i<fn_ext.length(); i++) {
        std::tolower( fn_ext.at(i), locale );
      }

      if ( acceptable_img_ext_.count( fn_ext ) == 1 ) {
        imglist_.push_back( rel_fn.string() );
        imglist_fn_original_size_.push_back( boost::filesystem::file_size( p ) );
        imglist_fn_transformed_size_.push_back( 0 );
      }
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

boost::filesystem::path SearchEngine::GetEngineConfigFn() {
  return engine_config_fn_;
}
std::string SearchEngine::GetSearchEngineBaseUri() {
  return "/" + engine_name_ + "/";
}

void SearchEngine::UpdateEngineOverview() {
  engine_overview_.str("");
  engine_overview_.clear();

  if ( imglist_.empty() ) {
    boost::filesystem::path imagePath(GetEngineConfigParam("imagePath"));
    CreateFileList( imagePath );
  }
  engine_overview_ << "<h3>Overview of Search Engine</h3>";
  engine_overview_ << "<table id=\"engine_overview\">";
  engine_overview_ << "<tr><td># of images</td><td>" << imglist_.size() << "</td></tr>";
  engine_overview_ << "<tr><td>Training time*</td><td>4 hours</td></tr>";
  engine_overview_ << "<tr><td>Memory required*</td><td>3 GB</td></tr>";
  engine_overview_ << "<tr><td>Disk space required*</td><td>10 GB</td></tr>";
  engine_overview_ << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
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

void SearchEngine::ReadConfigFromFile() {
  try {
    engine_config_.clear();
    std::ifstream f( engine_config_fn_.c_str() );
    char line[1024];
    f.getline(line, 1024, '\n');
    std::string search_engine_name(line);
    assert( engine_name_ == search_engine_name );

    while ( !f.eof() ) {
      f.getline(line, 1024, '\n');
      std::string param( line );
      std::size_t eq_pos = param.find('=');
      if ( eq_pos != std::string::npos ) {
        std::string key   = param.substr(0, eq_pos);
        std::string value = param.substr(eq_pos+1, std::string::npos);
        SetEngineConfigParam(key, value);
      }
    }
    f.close();
  } catch( std::exception &e) {
    std::cerr << "\nSearchEngine::ReadConfigFromFile() : error reading configuration from [" << engine_config_fn_ << "]" << std::flush;
    std::cerr << e.what() << std::flush;
  }
}

void SearchEngine::SendLog(std::string sender, std::string log) {
  SendPacket(sender, "log", log);
}

void SearchEngine::SendPacket(std::string sender, std::string type, std::string message) {
  std::ostringstream s;
  s << sender << " " << type << " " << message;
  vise_message_queue_.Push( s.str() );
}

//
// Helper methods
//
std::string SearchEngine::GetName() {
  return engine_name_;
}
bool SearchEngine::IsEngineConfigEmpty() {
  return engine_config_.empty();
}
bool SearchEngine::IsImglistEmpty() {
  return imglist_.empty();
}
bool SearchEngine::EngineConfigFnExists() {
  return boost::filesystem::exists( engine_config_fn_ );
}
bool SearchEngine::ImglistFnExists() {
  return boost::filesystem::exists( imglist_fn_ );
}
bool SearchEngine::DescFnExists() {
  boost::filesystem::path train_desc_fn( GetEngineConfigParam("descFn") );
  return boost::filesystem::exists( train_desc_fn );
}
bool SearchEngine::ClstFnExists() {
  boost::filesystem::path clst_fn( GetEngineConfigParam("clstFn") );
  return boost::filesystem::exists( clst_fn );
}
bool SearchEngine::AssignFnExists() {
  boost::filesystem::path assign_fn( GetEngineConfigParam("assignFn") );
  return boost::filesystem::exists( assign_fn );
}
bool SearchEngine::HammFnExists() {
  boost::filesystem::path hamm_fn( GetEngineConfigParam("hammFn") );
  return boost::filesystem::exists( hamm_fn );
}
bool SearchEngine::IndexFnExists() {
  boost::filesystem::path dset_fn( GetEngineConfigParam("dsetFn") );
  boost::filesystem::path fidx_fn( GetEngineConfigParam("fidxFn") );
  boost::filesystem::path iidx_fn( GetEngineConfigParam("iidxFn") );
  bool dset_exist = boost::filesystem::exists( dset_fn );
  bool fidx_exist = boost::filesystem::exists( fidx_fn );
  bool iidx_exist = boost::filesystem::exists( iidx_fn );
  return ( dset_exist && fidx_exist && iidx_exist );
}
unsigned long SearchEngine::DescFnSize() {
  boost::filesystem::path train_desc_fn( GetEngineConfigParam("descFn") );
  return boost::filesystem::file_size( train_desc_fn.string() );
}
unsigned long SearchEngine::ClstFnSize() {
  boost::filesystem::path clst_fn( GetEngineConfigParam("clstFn") );
  return boost::filesystem::file_size( clst_fn.string() );
}
unsigned long SearchEngine::AssignFnSize() {
  boost::filesystem::path assign_fn( GetEngineConfigParam("assignFn") );
  return boost::filesystem::file_size( assign_fn.string() );
}
unsigned long SearchEngine::HammFnSize() {
  boost::filesystem::path hamm_fn( GetEngineConfigParam("hammFn") );
  return boost::filesystem::file_size( hamm_fn.string() );
}
unsigned long SearchEngine::IndexFnSize() {
  boost::filesystem::path dset_fn( GetEngineConfigParam("dsetFn") );
  boost::filesystem::path fidx_fn( GetEngineConfigParam("fidxFn") );
  boost::filesystem::path iidx_fn( GetEngineConfigParam("iidxFn") );
  unsigned long dset_size = boost::filesystem::file_size( dset_fn.string() );
  unsigned long fidx_size = boost::filesystem::file_size( fidx_fn.string() );
  unsigned long iidx_size = boost::filesystem::file_size( iidx_fn.string() );
  return (dset_size + fidx_size + iidx_size);
}
unsigned long SearchEngine::GetImglistOriginalSize() {
  unsigned long total_size = 0;
  for ( unsigned int i=0; i<imglist_fn_original_size_.size(); i++) {
    total_size += imglist_fn_original_size_.at(i);
  }
  return total_size;
}
unsigned long SearchEngine::GetImglistTransformedSize() {
  unsigned long total_size = 0;
  for ( unsigned int i=0; i<imglist_fn_transformed_size_.size(); i++) {
    total_size += imglist_fn_transformed_size_.at(i);
  }
  return total_size;
}

std::string SearchEngine::GetEngineOverview() {
  return engine_overview_.str();
}
boost::filesystem::path SearchEngine::GetOriginalImageDir() {
  return original_imgdir_;
}
boost::filesystem::path SearchEngine::GetTransformedImageDir() {
  return transformed_imgdir_;
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
