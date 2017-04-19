add_library( SearchEngine SearchEngine.cc)
target_link_libraries( SearchEngine
  train_descs
  feat_getter
  feat_standard
  ${Boost_LIBRARIES} 
  ${ImageMagick_LIBRARIES})

add_library( ViseServer ViseServer.cc ViseMessageQueue.cc)
target_link_libraries( ViseServer 
  SearchEngine
  ${Boost_LIBRARIES} 
  ${ImageMagick_LIBRARIES})

