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

  if ( EngineExists(name) ) {
    LoadEngine( name );

    if ( EngineConfigExists() ) {
      state_ = SearchEngine::SETTINGS;
    }
  } else {
    CreateEngine( name );
  }
}

void SearchEngine::CreateEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;

  boost::filesystem::path config_fn( "user_settings.txt" );
  engine_config_fn_ = enginedir_ / config_fn;

  // create directory
  if ( ! create_directory( enginedir_ ) ) {
    std::cerr << "\nSearchEngine::CreateEngine() : "
              << "failed to create search engine directory "
              << "[" << enginedir_ << "]" << std::flush;
  }

  state_ = SearchEngine::INITIALIZE;
  engine_name_ = name;
  std::cout << "\n" << name << " search engine created" << std::flush;
}

void SearchEngine::LoadEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;
  if ( is_directory( enginedir_ ) ) {
    std::cout << "\n" << name << " search engine loaded" << std::flush;
    state_ = SearchEngine::INITIALIZE;
    engine_name_ = name;

    boost::filesystem::path config_fn( "user_settings.txt" );
    engine_config_fn_ = enginedir_ / config_fn;
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
    std::string rel_path = fn_dir.replace(0, basedir.length(), "./");
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
  CreateFileList( imagePath, image_file_list_ );

  engine_overview_ << "<h3>Overview of Search Engine</h3>";
  engine_overview_ << "<table id=\"engine_overview\">";
  engine_overview_ << "<tr><td># of images</td><td>" << image_file_list_.size() << "</td></tr>";
  engine_overview_ << "<tr><td>Training time*</td><td>4 hours</td></tr>";
  engine_overview_ << "<tr><td>Memory required*</td><td>3 GB</td></tr>";
  engine_overview_ << "<tr><td>Disk space required*</td><td>10 GB</td></tr>";
  engine_overview_ << "<tr><td>&nbsp;</td><td>&nbsp;</td></tr>";
  engine_overview_ << "<tr><td><input type=\"button\" onclick=\"_vise_send_post_request('back')\" value=\"Back\"></td><td><input type=\"button\" onclick=\"_vise_send_post_request('proceed')\" value=\"Proceed\"></td></tr>";
  engine_overview_ << "</table>";
  engine_overview_ << "<p>&nbsp;</p><p>* : todo</p>";
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
