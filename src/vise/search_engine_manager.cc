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

  search_engine_index_thread_running_ = false;
  now_search_engine_index_state_.clear();

  search_engine_index_state_name_list.clear();
  search_engine_index_state_name_list.push_back("relja_retrival:trainDescs");
  search_engine_index_state_name_list.push_back("relja_retrival:cluster");
  search_engine_index_state_name_list.push_back("relja_retrival:trainAssign");
  search_engine_index_state_name_list.push_back("relja_retrival:trainHamm");
  search_engine_index_state_name_list.push_back("relja_retrival:index");
  search_engine_index_state_desc_list.clear();
  search_engine_index_state_desc_list.push_back("Computing image descriptors");
  search_engine_index_state_desc_list.push_back("Clustering descriptors");
  search_engine_index_state_desc_list.push_back("Assigning descriptors");
  search_engine_index_state_desc_list.push_back("Computing embeddings");
  search_engine_index_state_desc_list.push_back("Indexing");
}

void search_engine_manager::process_cmd(const std::string search_engine_name,
                                        const std::string search_engine_version,
                                        const std::string search_engine_command,
                                        const std::map<std::string, std::string> uri_param,
                                        const std::string request_body,
                                        http_response& response) {
  if ( search_engine_command != "index_status" ) {
    BOOST_LOG_TRIVIAL(debug) << "processing search engine command: " << search_engine_name << "," << search_engine_version << "," << search_engine_command << ": payload=" << request_body.size() << " bytes";
  }

  if ( search_engine_command == "init" ) {
    if ( search_engine_exists(search_engine_name, search_engine_version) ) {
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload( "{\"error\":\"search engine already exists\"}" );
    } else {
      if ( request_body.length() ) {
        create_search_engine(search_engine_name, search_engine_version, request_body);
      } else {
        create_search_engine(search_engine_name, search_engine_version, "");
      }
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      std::ostringstream s;
      s << "{\"ok\":\"search engine created\", "
        << "\"search_engine_name\":\"" << search_engine_name << "\","
        << "\"search_engine_version\":\"" << search_engine_version << "\"}";

      response.set_payload( s.str() );
    }
    return;
  }

  if ( search_engine_command == "add_file" ) {
    boost::filesystem::path image_filename = get_image_filename(search_engine_name, search_engine_version, uri_param);
    bool ok = add_image_from_http_payload(image_filename, request_body);
    if ( ok ) {
      std::ostringstream s;
      s << "{\"filename\":\"" << image_filename.filename().string() << "\"}";
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload(s.str());
    } else {
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload("{\"error\":\"failed to add image\"}");
    }
    return;
  }

  if ( search_engine_command == "index_start" ) {
    if ( search_engine_index_thread_running_ ) {
      std::ostringstream s;
      s << "{\"error\":\"indexing ongoing for " << now_search_engine_name
        << ":" << now_search_engine_version << "\"}";
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload( s.str() );
      return;
    }

    try {
      search_engine_index_thread_ = new boost::thread( boost::bind( &search_engine_manager::index_start, this, search_engine_name, search_engine_version ) );
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload("{\"ok\":\"indexing started\"}");
    } catch( std::exception &e ) {
      std::ostringstream s;
      s << "{\"error\":\"" << e.what() << "\"}";
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload( s.str() );
}
    return;
  }

  if ( search_engine_command == "index_status" ) {
    if ( now_search_engine_index_state_.size() ) {
      std::ostringstream s;
      std::string name, desc;
      s << "[";
      for ( std::size_t i = 0; i < search_engine_index_state_name_list.size(); ++i ) {
        name = search_engine_index_state_name_list.at(i);
        desc = search_engine_index_state_desc_list.at(i);
        if ( i != 0 ) {
          s << ",";
        }

        s << "{\"name\":\"" << name << "\","
          << "\"description\":\"" << desc << "\","
          << "\"state\":\"" << now_search_engine_index_state_[ name ] << "\"";
        if ( now_search_engine_index_state_[ name ] == "progress" ) {
          s << ",\"steps_done\":" << now_search_engine_index_steps_done_[ name ] << ","
            << "\"steps_count\":" << now_search_engine_index_steps_count_[ name ] << "";
        }
        s << "}";
      }
      s << "]";
      std::string state;
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload( s.str() );
    } else {
      std::string state;
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload( "{\"error\":\"indexing not started yet!\"}" );
    }
    return;
  }

  if ( search_engine_command == "rename" ) {
    if ( search_engine_exists(search_engine_name, search_engine_version) ) {
      if ( uri_param.count("new_search_engine_name") ) {
        std::string new_search_engine_name = uri_param.at("new_search_engine_name");
        boost::filesystem::path old_name = get_search_engine_dir(search_engine_name, search_engine_version);
        boost::filesystem::path new_name = get_search_engine_dir(new_search_engine_name, search_engine_version);
        if ( !search_engine_exists(new_search_engine_name, search_engine_version) ) {
          boost::filesystem::rename(old_name, new_name);
          std::ostringstream s;
          s << "{\"ok\":\"search engine renamed\","
            << "\"new_search_engine_name\":\"" << new_search_engine_name << "\","
            << "\"new_search_engine_version\":\"" << search_engine_version << "\"}";
          response.set_status(200);
          response.set_field("Content-Type", "application/json");
          response.set_payload( s.str() );
          return;
        }
      }
    }

    std::ostringstream s;
    s << "{\"error\":\"could not rename search engine\"}";
    response.set_status(400);
    response.set_field("Content-Type", "application/json");
    response.set_payload( s.str() );
    return;
  }

  if ( search_engine_command == "engine_exists" ) {
    response.set_status(200);
    response.set_field("Content-Type", "application/json");
    if ( search_engine_exists(search_engine_name, search_engine_version) ) {
      response.set_payload( "{\"yes\":\"search engine exists\"}" );
    } else {
      response.set_payload( "{\"no\":\"search engine does not exist\"}" );
    }
    return;
  }

  if ( search_engine_command == "set_config" ) {
    if ( search_engine_exists(search_engine_name, search_engine_version) ) {
      if ( uri_param.count("config_name") && uri_param.count("config_value") ) {
        set_search_engine_config(search_engine_name,
                                 search_engine_version,
                                 uri_param.at("config_name"),
                                 uri_param.at("config_value")
                                 );
        response.set_status(200);
        response.set_field("Content-Type", "application/json");
        response.set_payload( "{\"ok\":\"config updated\"}" );
        return;
      }
    }
    response.set_status(400);
    response.set_field("Content-Type", "application/json");
    response.set_payload( "{\"error\":\"failed to update config\"}" );
    return;
  }

  // unhandled cases
  response.set_status(400);
}

void search_engine_manager::clear_now_search_engine_index_state(void) {
  now_search_engine_index_steps_done_.clear();
  now_search_engine_index_steps_count_.clear();
  now_search_engine_index_state_.clear();

  for ( std::size_t i = 0; i < search_engine_index_state_name_list.size(); ++i ) {
    now_search_engine_index_state_[ search_engine_index_state_name_list.at(i) ] = "not_started";
  }
}

bool search_engine_manager::index_start(const std::string search_engine_name,
                                        const std::string search_engine_version) {
  search_engine_index_thread_running_ = true;
  now_search_engine_name = search_engine_name;
  now_search_engine_version = search_engine_version;
  clear_now_search_engine_index_state();

  // @todo: find a way to automatically located these executables
  std::string index_exec   = "/home/tlm/dev/vise/bin/compute_index_v2";
  std::string cluster_exec = "/home/tlm/dev/vise/src/search_engine/relja_retrival/src/v2/indexing/compute_clusters.py";
  boost::filesystem::path config_fn = get_config_filename(search_engine_name, search_engine_version);

  std::ostringstream s;

  // 1. create image filename list
  boost::filesystem::path image_list_fn = get_image_list_filename(search_engine_name, search_engine_version);
  boost::filesystem::path image_data_dir = get_image_data_dir(search_engine_name, search_engine_version);
  s << "cd " << image_data_dir.string() << " && find *.jpg -print -type f > " << image_list_fn.string();

  // @todo: code is not portable for Windows
  std::system( s.str().c_str() );

  bool ok;

  // 2. extract descriptors
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " trainDescs " << search_engine_name << " " << config_fn.string();
  now_search_engine_index_state_[ "relja_retrival:trainDescs" ] = "started";
  ok = run_shell_command( "relja_retrival:trainDescs", s.str() );
  if ( ! ok ) {
    search_engine_index_thread_running_ = false;
    return false;
  }

  // 3. cluster descriptors
  s.str("");
  s.clear();
  s << "mpirun -np 8 python " << cluster_exec
    << " " << search_engine_name << " " << config_fn.string() << " 8";
  now_search_engine_index_state_[ "relja_retrival:cluster" ] = "started";
  ok = run_shell_command( "relja_retrival:cluster", s.str() );
  if ( ! ok ) {
    search_engine_index_thread_running_ = false;
    return false;
  }

  // 4. trainAssign
  s.str("");
  s.clear();
  //s << "mpirun -np 8 " << index_exec // see docs/known_issues.txt
  s << index_exec
    << " trainAssign " << search_engine_name << " " << config_fn.string();
  now_search_engine_index_state_[ "relja_retrival:trainAssign" ] = "started";
  ok = run_shell_command( "relja_retrival:trainAssign", s.str() );
  if ( ! ok ) {
    search_engine_index_thread_running_ = false;
    return false;
  }

  // 4. trainHamm
  s.str("");
  s.clear();
  //s << "mpirun -np 8 " << index_exec
  s << index_exec
    << " trainHamm " << search_engine_name << " " << config_fn.string();
  now_search_engine_index_state_[ "relja_retrival:trainHamm" ] = "started";
  ok = run_shell_command( "relja_retrival:trainHamm", s.str() );
  if ( ! ok ) {
    search_engine_index_thread_running_ = false;
    return false;
  }

  // 5. index
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " index " << search_engine_name << " " << config_fn.string();
  ok = run_shell_command( "relja_retrival:index", s.str() );
  if ( ! ok ) {
    search_engine_index_thread_running_ = false;
    return false;
  }

  search_engine_index_thread_running_ = false;
  return true;
}

bool search_engine_manager::run_shell_command(std::string cmd_name,
                                              std::string cmd) {
  boost::process::ipstream pipe;
  BOOST_LOG_TRIVIAL(debug) << "running command: {" << cmd_name << "} [" << cmd << "]";
  boost::process::child p(cmd, boost::process::std_out > pipe);
  std::string line;

  while ( pipe && std::getline(pipe, line) && !line.empty() ) {
    BOOST_LOG_TRIVIAL(debug) << "[" << cmd_name << "] " << line;
    if ( vise::util::starts_with(line, "relja_retrival,") ) {
      //trainDescs:relja_retrival,trainDescsManager(images),2018-Jun-25 11:47:52,1,40
      // 0                       , 1                       , 2                  ,3,4
      std::vector<std::string> d = vise::util::split(line, ',');
      now_search_engine_index_steps_done_[ cmd_name ] = d[3];
      now_search_engine_index_steps_count_[cmd_name ] = d[4];
      now_search_engine_index_state_[ cmd_name ] = "progress";
    }
  }
  // @todo check if the process returned non-zero status code
  // @todo process error cases

  p.wait();
  if ( p.exit_code() ) {
    now_search_engine_index_state_[ cmd_name ] = "error";
    return false;
  } else {
    now_search_engine_index_state_[ cmd_name ] = "done";
    return true;
  }
}


boost::filesystem::path search_engine_manager::get_image_filename(const std::string search_engine_name,
                                                                  const std::string search_engine_version,
                                                                  const std::map<std::string, std::string>& uri_param ) {
  boost::filesystem::path image_filename = get_image_data_dir(search_engine_name, search_engine_version);
  std::string filename;
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
  return image_filename;
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
                                                 const std::string search_engine_version,
                                                 const std::string search_engine_description) {
  boost::filesystem::path search_engine_dir = get_search_engine_dir(search_engine_name, search_engine_version);
  if ( boost::filesystem::exists(search_engine_dir) ) {
    boost::filesystem::remove_all(search_engine_dir);
    BOOST_LOG_TRIVIAL(debug) << "removing existing search engine directory [" << search_engine_dir.string() << "]";
  }
  boost::filesystem::create_directories( search_engine_dir );
  boost::filesystem::create_directories( get_image_data_dir(search_engine_name, search_engine_version) );
  boost::filesystem::create_directories( get_vise_data_dir(search_engine_name, search_engine_version)  );
  boost::filesystem::create_directories( get_log_data_dir(search_engine_name, search_engine_version)   );
  boost::filesystem::create_directories( get_temp_data_dir(search_engine_name, search_engine_version)  );

  create_default_config(search_engine_name, search_engine_version, search_engine_description);

  BOOST_LOG_TRIVIAL(debug) << "created search engine directory [" << search_engine_dir.string() << "]";
}

boost::filesystem::path search_engine_manager::get_config_filename(const std::string search_engine_name,
                                                                   const std::string search_engine_version) {
  return ( get_vise_data_dir(search_engine_name, search_engine_version) / "config.txt" );
}

boost::filesystem::path search_engine_manager::get_image_list_filename(const std::string search_engine_name,
                                                                       const std::string search_engine_version) {
  return ( get_vise_data_dir(search_engine_name, search_engine_version) / "image_filename_list.txt" );
}

bool search_engine_manager::create_default_config(const std::string search_engine_name,
                                                  const std::string search_engine_version,
                                                  const std::string search_engine_description) {
  boost::filesystem::path config_fn = get_config_filename(search_engine_name, search_engine_version);
  try {
    boost::property_tree::ptree pt;
    //std::string prefix = search_engine_name + "." + search_engine_version;
    std::string prefix = search_engine_name;
    std::string vise_data_dir = get_vise_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;
    std::string image_data_dir = get_image_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;
    std::string temp_data_dir  = get_temp_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;

    pt.put( prefix + ".titlePrefix", search_engine_description);
    pt.put( prefix + ".RootSIFT", "true");
    pt.put( prefix + ".SIFTscale3", "true");
    pt.put( prefix + ".hammEmbBits", "64");
    pt.put( prefix + ".imagelistFn", get_image_list_filename(search_engine_name, search_engine_version).string());
    pt.put( prefix + ".databasePath", image_data_dir );
    pt.put( prefix + ".trainFilesPrefix", vise_data_dir );
    pt.put( prefix + ".dsetFn", (vise_data_dir + "dset.v2bin") );
    pt.put( prefix + ".clstFn", (vise_data_dir + "clst.e3bin") );
    pt.put( prefix + ".iidxFn", (vise_data_dir + "iidx.v2bin") );
    pt.put( prefix + ".fidxFn", (vise_data_dir + "fidx.v2bin") );
    pt.put( prefix + ".wghtFn", (vise_data_dir + "wght.v2bin") );
    pt.put( prefix + ".tmpDir", temp_data_dir);
    pt.put( prefix + ".trainNumDescs", "-1");
    pt.put( prefix + ".vocSize", "1000");
    boost::property_tree::ini_parser::write_ini(config_fn.string(), pt);
    BOOST_LOG_TRIVIAL(debug) << "written search engine config to [" << config_fn.string() << "]";
    return true;
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while writing config file [" << config_fn.string() << "] : " << e.what();
    return false;
  }
}

bool search_engine_manager::set_search_engine_config(const std::string search_engine_name,
                                                     const std::string search_engine_version,
                                                     const std::string config_name,
                                                     const std::string config_value) {
  boost::filesystem::path config_fn = get_config_filename(search_engine_name, search_engine_version);
  try {
    boost::property_tree::ptree pt;
    boost::property_tree::ini_parser::read_ini(config_fn.string(), pt);
    //std::string prefix = search_engine_name + "." + search_engine_version;
    std::string prefix = search_engine_name;
    pt.put( prefix + "." + config_name, config_value);
    boost::property_tree::ini_parser::write_ini(config_fn.string(), pt);
    BOOST_LOG_TRIVIAL(debug) << "updated search engine config " << config_name << "=" << config_value << "in config file [" << config_fn.string() << "]";
    return true;
  }  catch( std::exception &e ) {
    BOOST_LOG_TRIVIAL(debug) << "exception occured while updating config file [" << config_fn.string() << "] : " << e.what();
    return false;
  }
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

boost::filesystem::path search_engine_manager::get_temp_data_dir(const std::string search_engine_name,
                                                                 const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  dir = dir / search_engine_version;
  return dir / "temp";
}

boost::filesystem::path search_engine_manager::get_log_data_dir(const std::string search_engine_name,
                                                                const std::string search_engine_version) {
  boost::filesystem::path dir = data_dir_ / search_engine_name;
  dir = dir / search_engine_version;
  return dir / "log_data";
}

boost::filesystem::path search_engine_manager::get_index_log_filename(const std::string search_engine_name,
                                                                      const std::string search_engine_version) {
  boost::filesystem::path dir = get_log_data_dir(search_engine_name, search_engine_version);
  return dir / "index.log";
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
