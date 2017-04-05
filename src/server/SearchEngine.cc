#include "SearchEngine.h"

SearchEngine::SearchEngine() {
  state_ = SearchEngine::UNKNOWN;
}

void SearchEngine::Init(std::string name, boost::filesystem::path basedir) {
  basedir_ = boost::filesystem::path(basedir);

  if ( state_ == SearchEngine::UNKNOWN ) {
    if ( EngineExists(name) ) {
      LoadEngine( name );
    } else {
      CreateEngine( name );
    }
  }
  else if ( state_ == SearchEngine::INIT ) {
    std::cout << "\nSearch engine already initialized" << std::flush;
  }
}

void SearchEngine::CreateEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;

  // create directory
  if ( ! create_directory( enginedir_ ) ) {
    std::cerr << "\nSearchEngine::CreateEngine() : "
              << "failed to create search engine directory "
              << "[" << enginedir_ << "]" << std::flush;
  }

  state_ = SearchEngine::INIT;
  std::cout << "\n" << name << " search engine created" << std::flush;
}

void SearchEngine::LoadEngine( std::string name ) {
  boost::filesystem::path engine_name( name );
  enginedir_ = basedir_ / engine_name;
  if ( is_directory( enginedir_ ) ) {
    std::cout << "\n" << name << " search engine loaded" << std::flush;
    state_ = SearchEngine::INIT;
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

void SearchEngine::MoveToNextState(std::string &result) {
  switch ( state_ ) {
  case SearchEngine::INIT :
    // Move to configuration state
    // serve config.html
    if ( config_html_.length() == 0 ) {
      // @todo
      LoadFile("/home/tlm/dev/vise/src/server/html_templates/config.html",
               config_html_);
    }
    result = config_html_;
    break;

  default:
    std::cerr << "Do not know how to move to next state!" << std::flush;
  }
}

void SearchEngine::ConfigureEngine(std::string config_json) {

}

void SearchEngine::JsonToMap( std::string json, std::map<std::string, std::string> ) {

}

void SearchEngine::LoadFile(std::string filename, std::string &file_contents) {
  std::ifstream f;
  f.open(filename.c_str());
  f.seekg(0, std::ios::end);
  file_contents.reserve( f.tellg() );
  f.seekg(0, std::ios::beg);

  file_contents.assign( std::istreambuf_iterator<char>(f),
                        std::istreambuf_iterator<char>() );
  f.close();
}
