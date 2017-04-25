add_library( SearchEngine SearchEngine.cc)
target_link_libraries( SearchEngine
  ViseMessageQueue
  train_descs
  train_assign
  train_hamming
  feat_standard
  hamming_embedder
  build_index
  ${Boost_LIBRARIES} 
  ${ImageMagick_LIBRARIES})

add_library( ViseMessageQueue ViseMessageQueue.cc)
target_link_libraries( ViseMessageQueue ${Boost_LIBRARIES} )

add_library( ViseServer ViseServer.cc)
target_link_libraries( ViseServer 
  ViseMessageQueue
  SearchEngine
  ${Boost_LIBRARIES} 
  ${ImageMagick_LIBRARIES})

