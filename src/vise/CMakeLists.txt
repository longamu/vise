add_library( vise_request_handler vise_request_handler.cc)
target_link_libraries( vise_request_handler http_request http_response ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )
