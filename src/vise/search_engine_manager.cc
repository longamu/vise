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
    boost::filesystem::path image_filename = get_image_filename(search_engine_name, search_engine_version, uri_param);
    bool ok = add_image_from_http_payload(image_filename, request_body);
    if ( ok ) {
      std::ostringstream s;
      s << "{filename:\"" << image_filename.string() << "\"}";
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload(s.str());
    } else {
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload("{error:\"failed to add image\"}");
    }
  }

  if ( search_engine_command == "index_start" ) {
    try {
      search_engine_index_thread_ = new boost::thread( boost::bind( &search_engine_manager::index_start, this, search_engine_name, search_engine_version ) );
      response.set_status(200);
      response.set_field("Content-Type", "application/json");
      response.set_payload("{ok:\"indexing started\"}");
    } catch( std::exception &e ) {
      std::ostringstream s;
      s << "{error:\"" << e.what() << "\"}";
      response.set_status(400);
      response.set_field("Content-Type", "application/json");
      response.set_payload( s.str() );
    }
  }
}

bool search_engine_manager::index_start(const std::string search_engine_name,
                                        const std::string search_engine_version) {
  run_shell_command( "pwd", "pwd" );
  std::string index_exec   = "/home/tlm/dev/vise/bin/compute_index_v2";
  std::string cluster_exec = "/home/tlm/dev/vise/src/search_engine/relja_retrival/src/v2/indexing/compute_clusters.py";
  boost::filesystem::path config_fn = get_config_filename(search_engine_name, search_engine_version);

  std::ostringstream s;

  // 1. create image filename list
  boost::filesystem::path image_list_fn = get_image_list_filename(search_engine_name, search_engine_version);
  boost::filesystem::path image_data_dir = get_image_data_dir(search_engine_name, search_engine_version);
  s << "cd " << image_data_dir.string() << " && find *.jpg -print -type f > " << image_list_fn.string();
  std::system( s.str().c_str() );

  // 2. extract descriptors
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " trainDescs " << search_engine_name << " " << config_fn.string();
  run_shell_command( "trainDescs", s.str() );

  // 3. cluster descriptors
  s.str("");
  s.clear();
  s << "mpirun -np 8 python " << cluster_exec
    << " " << search_engine_name << " " << config_fn.string() << " 4";
  run_shell_command( "cluster", s.str() );

  // 4. trainAssign
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " trainAssign " << search_engine_name << " " << config_fn.string();
  run_shell_command( "trainAssign", s.str() );

  // 4. trainHamm
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " trainHamm " << search_engine_name << " " << config_fn.string();
  run_shell_command( "trainHamm", s.str() );

  // 5. index
  s.str("");
  s.clear();
  s << "mpirun -np 8 " << index_exec
    << " index " << search_engine_name << " " << config_fn.string();
  run_shell_command( "index", s.str() );
}

bool search_engine_manager::run_shell_command(std::string name, std::string cmd) {
  boost::process::ipstream pipe;
  BOOST_LOG_TRIVIAL(debug) << "running command: {" << name << "} [" << cmd << "]";
  boost::process::child p(cmd, boost::process::std_out > pipe);
  std::string line;
  while ( pipe && std::getline(pipe, line) && !line.empty() ) {
    BOOST_LOG_TRIVIAL(debug) << name << ":" << line;
  }
  p.wait();
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
                                                 const std::string search_engine_version) {
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

  create_default_config(search_engine_name, search_engine_version);

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
                                                  const std::string search_engine_version) {
  boost::filesystem::path config_fn = get_config_filename(search_engine_name, search_engine_version);
  try {
    boost::property_tree::ptree pt;
    //std::string prefix = search_engine_name + "." + search_engine_version;
    std::string prefix = search_engine_name;
    std::string vise_data_dir = get_vise_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;
    std::string image_data_dir = get_image_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;
    std::string temp_data_dir  = get_temp_data_dir(search_engine_name, search_engine_version).string() + boost::filesystem::path::preferred_separator;

    pt.put( prefix + ".titlePrefix", "Search engine created using VGG Image Search Engine (VISE)");
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
