#include "SearchEngine.h"

SearchEngine::SearchEngine() {
  state_ = SearchEngine::INITIALIZE;

  state_name_list_.push_back("Initialize");
  state_name_list_.push_back("Settings");
  state_name_list_.push_back("Overview");
  state_name_list_.push_back("Preprocessing");
  state_name_list_.push_back("Descriptor");
  state_name_list_.push_back("Cluster");
  state_name_list_.push_back("Assignment");
  state_name_list_.push_back("Hamm");
  state_name_list_.push_back("Index");
  state_name_list_.push_back("Query");

  state_info_list_.push_back("");
  state_info_list_.push_back("");
  state_info_list_.push_back("");
  state_info_list_.push_back("(2 min.)");
  state_info_list_.push_back("(6 min.)");
  state_info_list_.push_back("(49 min.)");
  state_info_list_.push_back("(1 min.)");
  state_info_list_.push_back("(1 min.)");
  state_info_list_.push_back("(3 hours)");
  state_info_list_.push_back("");
}

void SearchEngine::Init(std::string name, boost::filesystem::path basedir) {
  basedir_ = boost::filesystem::path(basedir);

  engine_name_ = name;
  InitEngineResources();

  state_ = SearchEngine::INITIALIZE;
}

void SearchEngine::InitEngineResources() {
  boost::filesystem::path engine_name( engine_name_ );
  enginedir_ = basedir_ / engine_name;

  engine_config_fn_    = enginedir_ / "user_settings.txt";
  transformed_imgdir_  = enginedir_ / "img";
  training_datadir_    = enginedir_ / "training_data";
  tmp_datadir_         = enginedir_ / "tmp";

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
    if ( ! boost::filesystem::exists( tmp_datadir_ ) ) {
      boost::filesystem::create_directory( tmp_datadir_ );
    }

    std::cout << "\n" << engine_name_ << " search engine loaded" << std::flush;
  }
}

bool SearchEngine::EngineExists( std::string name ) {
  boost::filesystem::path engine_name( name );
  boost::filesystem::path engine_dir = basedir_ / engine_name;

  if ( is_directory( engine_dir ) ) {
    return true;
  } else {
    return false;
  }
}

void SearchEngine::MoveToNextState() {
  std::string redirect_uri;
  switch ( state_ ) {
  case SearchEngine::INITIALIZE :
    state_ = SearchEngine::SETTINGS;
    break;

  case SearchEngine::SETTINGS :
    state_ = SearchEngine::OVERVIEW;
    break;

  case SearchEngine::OVERVIEW :
    state_ = SearchEngine::PREPROCESSING;
    break;

  case SearchEngine::PREPROCESSING :
    state_ = SearchEngine::COMPUTE_DESCRIPTORS;
    break;

  case SearchEngine::COMPUTE_DESCRIPTORS :
    state_ = SearchEngine::CLUSTER_DESCRIPTORS;
    break;

  default:
    std::cerr << "Do not know how to move to next state!" << std::flush;
  }
}

void SearchEngine::MoveToPrevState() {
  std::string redirect_uri;
  switch ( state_ ) {
  case SearchEngine::SETTINGS :
    state_ = SearchEngine::INITIALIZE;
    break;

  case SearchEngine::OVERVIEW :
    state_ = SearchEngine::SETTINGS;
    break;

  default:
    std::cerr << "Do not know how to move to previous state!" << std::flush;
  }
}

bool SearchEngine::EngineConfigExists() {
  if ( is_regular_file(engine_config_fn_) ) {
    return true;
  } else {
    return false;
  }
}

std::string SearchEngine::GetResourceUri(std::string resource_name) {
  std::string resource_uri = "/" + engine_name_;
  if ( resource_name == "settings" ) {
    resource_uri = resource_uri + "/settings";
  } else if ( resource_name == "training" ) {
    resource_uri = resource_uri + "/training";
  } else {
    resource_uri = resource_uri + "/";
  }
  return resource_uri;
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
    return "Not Found";
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

std::string SearchEngine::GetEngineStateList() {
  std::ostringstream json;
  json << "{ \"id\" : \"search_engine_state\",";
  json << "\"state_id_list\" : [ 0";
  for ( unsigned int i=1 ; i<SearchEngine::STATE_COUNT; i++ ) {
    json << "," << i;
  }
  json << "], \"state_name_list\" : [ \"" << state_name_list_.at(0) << "\"";
  for ( unsigned int i=1 ; i<SearchEngine::STATE_COUNT; i++ ) {
    json << ", \"" << state_name_list_.at(i) << "\"";
  }
  json << "], \"state_info_list\" : [ \"" << state_info_list_.at(0) << "\"";
  for ( unsigned int i=1 ; i<SearchEngine::STATE_COUNT; i++ ) {
    json << ", \"" << state_info_list_.at(i) << "\"";
  }
  json << "], \"current_state_id\" : " << state_ << "  }";
  return json.str();
}

int SearchEngine::GetEngineState(std::string state_name) {
  std::vector<std::string>::iterator found;
  found = std::find( state_name_list_.begin(), state_name_list_.end(), state_name );
  if ( found != state_name_list_.end() ) {
    return ( std::distance(state_name_list_.begin(), found) );
  } else {
    return -1;
  }
}

SearchEngine::STATE SearchEngine::GetEngineState() {
  return state_;
}
std::string SearchEngine::GetEngineStateName() {
  return state_name_list_.at(state_);
}
std::string SearchEngine::GetEngineStateName( unsigned int state_id ) {
  return state_name_list_.at(state_id);
}
std::string SearchEngine::GetEngineStateInfo() {
  return state_info_list_.at(state_);
}
std::string SearchEngine::Name() {
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

void SearchEngine::Preprocess() {
  SendStatus("Preprocessing started ...");

  // scale and copy image to transformed_imgdir_
  SendStatus("\nSaved transformed images to [" + transformed_imgdir_.string() + "] ");
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
        SendStatus( "." );
      } catch (std::exception &error) {
        SendStatus( "\n" + src_fn.string() + " : Error [" + error.what() + "]" );
      }
    }
  }
  SendStatus("[DONE]");

  //if ( ! boost::filesystem::exists( imglist_fn_ ) ) {
  imglist_fn_ = training_datadir_ / "imlist.txt";
  WriteImageListToFile( imglist_fn_.string(), imglist_ );
  SendStatus( "\nWritten image list to : [" + imglist_fn_.string() + "]" );

  // save full configuration file to training_datadir_
  SetEngineConfigParam("trainDatabasePath", transformed_imgdir_.string());
  SetEngineConfigParam("databasePath", transformed_imgdir_.string());
  SetEngineConfigParam("trainImagelistFn",  imglist_fn_.string());
  SetEngineConfigParam("imagelistFn",  imglist_fn_.string());
  boost::filesystem::path train_file_prefix = training_datadir_ / "train_";
  SetEngineConfigParam("trainFilesPrefix", train_file_prefix.string() );
  SetEngineConfigParam("pathManHide", transformed_imgdir_.string() );
  SetEngineConfigParam("dsetFn", train_file_prefix.string() + "dset.v2bin" );
  SetEngineConfigParam("clstFn", train_file_prefix.string() + "clst.e3bin" );
  SetEngineConfigParam("iidxFn", train_file_prefix.string() + "iidx.e3bin" );
  SetEngineConfigParam("fidxFn", train_file_prefix.string() + "fidx.e3bin" );
  SetEngineConfigParam("wdhtFn", train_file_prefix.string() + "wght.e3bin" );
  SetEngineConfigParam("tmpDir", tmp_datadir_.string());

  engine_config_fn_ = training_datadir_ / "vise_config.cfg";
  WriteConfigToFile();
  SendStatus( "\nWritten search engine configuration to : [" + engine_config_fn_.string() + "]" );

  SendControl("<div id=\"Preprocessing_button_proceed\" class=\"action_button\" onclick=\"_vise_send_post_request('proceed')\">Proceed</div>");
}

void SearchEngine::Descriptor() {
  SendStatus("Computing training descriptors ...");
  boost::this_thread::sleep( boost::posix_time::seconds(4) );

  SendControl("<div id=\"Descriptor_button_proceed\" class=\"action_button\" onclick=\"_vise_send_post_request('proceed')\">Proceed</div>");
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

void SearchEngine::SendMessage(std::string message) {
  SendPacket(GetEngineStateName(), "message", message);
}

void SearchEngine::SendStatus(std::string status) {
  SendPacket(GetEngineStateName(), "status", status);
}

void SearchEngine::SendControl(std::string control) {
  SendPacket(GetEngineStateName(), "control", control);
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
