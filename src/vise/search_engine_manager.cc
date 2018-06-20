#include "vise/search_engine_manager.h"

search_engine_manager *search_engine_manager::search_engine_manager_ = NULL;

search_engine_manager* search_engine_manager::instance() {
  if ( !search_engine_manager_ ) {
    search_engine_manager_ = new search_engine_manager;
  }
  return search_engine_manager_;
}

void search_engine_manager::init(const boost::filesystem::path data_dir) {
  data_dir_ = data_dir;
  BOOST_LOG_TRIVIAL(debug) << "search_engine_manager::init() : data_dir=" << data_dir_.string() << flush;
}

void search_engine_manager::process_cmd(const std::string search_engine_name,
                                        const std::string search_engine_version,
                                        const std::string search_engine_command,
                                        const std::string payload,
                                        http_response& response) {
  BOOST_LOG_TRIVIAL(debug) << "processing search engine command: " << search_engine_name << "," << search_engine_version << "," << search_engine_command;
}
