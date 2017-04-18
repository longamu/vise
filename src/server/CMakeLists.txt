add_library( ViseServer ViseServer.cc SearchEngine.cc ViseMessageQueue.cc)
target_link_libraries( ViseServer ${Boost_LIBRARIES} )

