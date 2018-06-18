add_subdirectory( search_engine/relja_retrival/src )
add_subdirectory( http_server )
add_subdirectory( vise )

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_executable( vise_server vise_server.cc )
  target_link_libraries( vise_server http_server vise_request_handler ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES})
endif()
