
add_library( util util.cc)
target_link_libraries( util ${Boost_LIBRARIES} )

add_library( relja_retrival search_engine.cc relja_retrival.cc )
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/util")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/embedding")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/evaluation")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/indexing")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/retrieval")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/retrieval")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/represent")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/matching/det_ransac")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/indexing")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/preprocessing")
target_include_directories(relja_retrival PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/compression")
target_link_libraries(  relja_retrival 
                        dataset_v2 
                        proto_db
                        proto_db_file
                        proto_index
                        slow_construction
                        feat_standard
                        clst_centres
                        hamming
                        hamming_embedder
                        spatial_api
                        spatial_verif_v2
                        tfidf_v2
                        mq_filter_outliers
                        ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} ${fastann_LIBRARIES})

add_library( search_engine_manager search_engine_manager.cc)
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/util")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/embedding")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/evaluation")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/indexing")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/retrieval")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/retrieval")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/represent")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/matching/det_ransac")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/indexing")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/preprocessing")
target_include_directories(search_engine_manager PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/compression")
target_link_libraries( search_engine_manager relja_retrival http_request http_response util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

add_library( vise_request_handler vise_request_handler.cc)
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/util")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/dataset")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/embedding")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/evaluation")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/indexing")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/v2/retrieval")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/retrieval")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/represent")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/matching/det_ransac")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/indexing")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/preprocessing")
target_include_directories(vise_request_handler PRIVATE "${CMAKE_SOURCE_DIR}/src/search_engine/relja_retrival/compression")
target_link_libraries( vise_request_handler http_request http_response util search_engine_manager ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

IF(CMAKE_BUILD_TYPE MATCHES Debug)
  # compile unit tests
#  add_executable( test_search_engine_manager test_search_engine_manager.cc )
#  target_link_libraries( test_search_engine_manager search_engine_manager http_request http_response util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} -lm -lpthread)

  add_executable( test_search_engine_manager test_search_engine_manager.cc )
  target_link_libraries( test_search_engine_manager search_engine_manager http_request http_response util ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} -lm -lpthread)
endif()
