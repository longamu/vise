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
                                        const std::map<std::string, std::string> uri_param,
                                        const std::string request_body,
                                        http_response& response) {
  BOOST_LOG_TRIVIAL(debug) << "processing search engine command: " << search_engine_name << "," << search_engine_version << "," << search_engine_command << ": payload=" << request_body.size() << " bytes";

  if ( search_engine_command == "add_image" ) {
    if ( ! search_engine_exists(search_engine_name, search_engine_version) ) {
      create_search_engine(search_engine_name, search_engine_version);
    }
    std::string filename;
    boost::filesystem::path image_filename = get_image_data_dir(search_engine_name, search_engine_version);
    if ( uri_param.count("filename") ) {
      filename = uri_param.at("filename");
      // ensure that image_filename has .jpg extension
      std::size_t dotindex = filename.rfind(".");
      std::string ext = filename.substr(dotindex + 1);
      if ( ext != "jpg" ) {
        filename.replace(dotindex, filename.size() - dotindex, ".jpg");
      }
    } else{
      filename = get_unique_filename(".jpg");
    }
    image_filename = image_filename / boost::filesystem::path(filename).filename();

    if ( boost::filesystem::exists(image_filename) ) {
      image_filename = convert_to_unique_filename(image_filename);
    }

    bool ok = add_image_from_http_payload(image_filename, request_body);
    if ( ok ) {
      std::ostringstream s;
      s << "{filename:\"" << image_filename << "\"}";
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload(s.str());
    } else {
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload("{error:\"failed to add image\"}");
    }
  }
}

boost::filesystem::path search_engine_manager::convert_to_unique_filename(boost::filesystem::path filename) {
  boost::filesystem::path newfn = filename.parent_path() / ( filename.stem().string() +
                                                             boost::filesystem::unique_path("_%%%%%").string() +
                                                             filename.extension().string() );
  return newfn;
}

std::string search_engine_manager::get_unique_filename(std::string extension) {
  return boost::filesystem::unique_path("%%%%%%%%%%").string() + extension;
}

bool search_engine_manager::search_engine_exists(const std::string search_engine_name,
                                                 const std::string search_engine_version) {
  boost::filesystem::path search_engine_dir = data_dir_ / search_engine_name;
  search_engine_dir = search_engine_dir / search_engine_version;

  if ( boost::filesystem::exists(search_engine_dir) ) {
    return true;
  } else {
    return false;
  }
}

bool search_engine_manager::create_search_engine(const std::string search_engine_name,
                                                 const std::string search_engine_version) {
  boost::filesystem::path search_engine_dir = get_search_engine_dir(search_engine_name, search_engine_version);
  if ( boost::filesystem::exists(search_engine_dir) ) {
    boost::filesystem::remove_all(search_engine_dir);
    BOOST_LOG_TRIVIAL(debug) << "removing existing search engine directory [" << search_engine_dir.string() << "]";
  }
  boost::filesystem::create_directories( search_engine_dir );
  boost::filesystem::create_directories( get_image_data_dir(search_engine_name, search_engine_version)    );
  boost::filesystem::create_directories( get_vise_data_dir(search_engine_name, search_engine_version)     );
  boost::filesystem::create_directories( get_log_data_dir(search_engine_name, search_engine_version)      );
  BOOST_LOG_TRIVIAL(debug) << "created search engine directory [" << search_engine_dir.string() << "]";
}

boost::filesystem::path search_engine_manager::get_search_engine_dir(const std::string search_engine_name,
                                                                     const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  return dir / search_engine_version;
}

boost::filesystem::path search_engine_manager::get_image_data_dir(const std::string search_engine_name,
                                                                  const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  dir = dir / search_engine_version;
  return dir / "images";;
}

boost::filesystem::path search_engine_manager::get_vise_data_dir(const std::string search_engine_name,
                                                                 const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  dir = dir / search_engine_version;
  return dir / "vise_data";
}

boost::filesystem::path search_engine_manager::get_log_data_dir(const std::string search_engine_name,
                                                                const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  dir = dir / search_engine_version;
  return dir / "log_data";
}

//
// search engine image i/o
//
bool search_engine_manager::add_image_from_http_payload(const boost::filesystem::path filename,
                                                        const std::string& request_body) {
  try {
    Magick::Blob blob(request_body.c_str(), request_body.size());

    if ( blob.length() ) {
      Magick::Image im(blob);
      im.magick("JPEG");
      im.colorSpace(Magick::sRGBColorspace);
      im.write(filename.string());
      BOOST_LOG_TRIVIAL(debug) << "written file [" << filename.string() << "]";
      return true;
    } else {
      BOOST_LOG_TRIVIAL(debug) << "failed to write file [" << filename.string() << "]";
      return false;
    }
  } catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while writing file [" << filename.string() << "] : " << e.what();
    return false;
  }
}
