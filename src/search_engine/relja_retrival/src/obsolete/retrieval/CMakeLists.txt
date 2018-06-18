add_library( bow_retriever bow_retriever.cpp )
target_link_libraries( bow_retriever retriever represent ${Boost_LIBRARIES} ${fastann_LIBRARIES} )

add_library( weighter weighter.cpp )
target_link_libraries( weighter )

add_library( tfidf tfidf.cpp )
target_link_libraries( tfidf bow_retriever weighter ${Boost_LIBRARIES} )

add_library( spatial_verif spatial_verif.cpp )
target_link_libraries( spatial_verif spatial_retriever homography bow_retriever det_ransac same_random )

add_library( image_graph image_graph.cpp )
target_link_libraries( image_graph spatial_verif ${Boost_LIBRARIES} )

add_library( exclude_docs exclude_docs.cpp )
target_link_libraries( exclude_docs evaluator bow_retriever )

add_library( tfidf_aug tfidf_aug.cpp )
target_link_libraries( tfidf_aug retriever bow_retriever image_graph ${Boost_LIBRARIES} )

add_library( query_expand query_expand.cpp )
target_link_libraries( query_expand bow_retriever represent spatial_verif )

add_library( query_expand_svm query_expand_svm.cpp )
target_link_libraries( query_expand_svm query_expand weighter tfidf libsvm svm_util )

add_library( multi_query_mqbm multi_query_mqbm.cpp )
target_link_libraries( multi_query_mqbm multi_query bow_retriever ${Boost_LIBRARIES} )
