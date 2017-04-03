add_library( ViseServer ViseServer.cc SearchEngine.cc)
target_link_libraries( ViseServer ${Boost_LIBRARIES} )


