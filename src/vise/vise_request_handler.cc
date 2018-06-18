#include "vise/vise_request_handler.h"

vise_request_handler *vise_request_handler::vise_request_handler_ = NULL;

vise_request_handler* vise_request_handler::instance() {
  if ( !vise_request_handler_ ) {
    vise_request_handler_ = new vise_request_handler;
  }
  return vise_request_handler_;
}

void vise_request_handler::init(const boost::filesystem::path upload_dir,
                                  const boost::filesystem::path result_dir,
                                  const boost::filesystem::path asset_dir) {
  upload_dir_ = upload_dir;
  result_dir_ = result_dir;
  asset_dir_ = asset_dir;
  cout << "\nImageMagick Magick++ quantum depth = " << MAGICKCORE_QUANTUM_DEPTH << flush;
  cout << "\nvise_request_handler::init() : initializing http request handler" << flush;
  cout << "\nvise_request_handler::init() : upload_dir=" << upload_dir_.string() << flush;
  cout << "\nvise_request_handler::init() : result_dir=" << result_dir_.string() << flush;
  cout << "\nvise_request_handler::init() : asset_dir=" << asset_dir_.string() << flush;
}

void vise_request_handler::handle_http_request(const http_request& request, http_response& response) {
  cout << "\n" << request.method_ << " [" << request.uri_ << "]" << flush;
  response.set_status(200);
  return;
}

