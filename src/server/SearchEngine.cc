#include "SearchEngine.h"

SearchEngine::SearchEngine() {
  state_ = SearchEngine::UNKNOWN;
}

void SearchEngine::Init(std::string name, boost::filesystem::path basedir) {
  basedir_ = boost::filesystem::path(basedir);

  if ( EngineExists(name) ) {
    LoadEngine( name );

    if ( EngineConfigExists() ) {
      state_ = SearchEngine::CONFIG;
    }
  } else {
    CreateEngine( name );
  }
}

void SearchEngine::CreateEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;

  boost::filesystem::path config_fn( "settings.txt" );
  engine_config_fn_ = enginedir_ / config_fn;

  // create directory
  if ( ! create_directory( enginedir_ ) ) {
    std::cerr << "\nSearchEngine::CreateEngine() : "
              << "failed to create search engine directory "
              << "[" << enginedir_ << "]" << std::flush;
  }

  state_ = SearchEngine::INIT;
  engine_name_ = name;
  std::cout << "\n" << name << " search engine created" << std::flush;
}

void SearchEngine::LoadEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;
  if ( is_directory( enginedir_ ) ) {
    std::cout << "\n" << name << " search engine loaded" << std::flush;
    state_ = SearchEngine::INIT;
    engine_name_ = name;

    boost::filesystem::path config_fn( "settings.txt" );
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

std::string SearchEngine::MoveToNextState() {
  std::string redirect_uri;
  switch ( state_ ) {
  case SearchEngine::INIT :
    if ( EngineConfigExists() ) {
      state_ = SearchEngine::CONFIG;
      redirect_uri = GetResourceUri("training");
    } else {
      redirect_uri = GetResourceUri("settings");
    }
    break;

  case SearchEngine::CONFIG :
    redirect_uri = GetResourceUri("training");
    break;

  case SearchEngine::TRAIN :
    redirect_uri = GetResourceUri("query");
    break;

  default:
    redirect_uri = GetResourceUri("unknown");
    std::cerr << "Do not know how to move to next state!" << std::flush;
  }
  return redirect_uri;
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
    resource_uri = resource_uri + "/unknown";
  }
  return resource_uri;
}
