add_executable( iidx_test iidx_test.cpp )
target_link_libraries( iidx_test )

add_executable( tfidf_compute_test tfidf_compute_test.cpp )
target_link_libraries( tfidf_compute_test iidx_in_ram tfidf )

add_executable( docmap_test docmap_test.cpp )
target_link_libraries( docmap_test document_map )

add_executable( tfidf_query_test tfidf_query_test.cpp )
target_link_libraries( tfidf_query_test tfidf fidx_wrapper_jp_db5 document_map )

add_executable( eval_tfidfspat_test eval_tfidfspat_test.cpp )
target_link_libraries( eval_tfidfspat_test tfidf spatial_verif iidx_in_ram fidx_in_ram fidx_wrapper_jp_db5 document_map evaluator thread_queue ${Boost_LIBRARIES} )

add_executable( image_graph_test image_graph_test.cpp )
target_link_libraries( image_graph_test tfidf spatial_verif fidx_wrapper_jp_db5 document_map evaluator exclude_docs image_graph tfidf_aug ${Boost_LIBRARIES} )

add_executable( eval_qe eval_qe.cpp )
target_link_libraries( eval_qe tfidf iidx_in_ram spatial_verif query_expand query_expand_svm fidx_wrapper_jp_db5 document_map evaluator ${Boost_LIBRARIES} )

add_executable( compute_index compute_index.cpp )
target_link_libraries( compute_index tfidf fidx_wrapper_jp_db5 document_map evaluator ${Boost_LIBRARIES} )

add_executable( compute_index_aug compute_index_aug.cpp )
target_link_libraries( compute_index_aug tfidf spatial_verif fidx_wrapper_jp_db5 fidx_aug document_map evaluator exclude_docs ${Boost_LIBRARIES} )

add_executable( coarseann_test coarseann_test.cpp )
target_link_libraries( coarseann_test product_quant nn_evaluator coarse_residual index_with_data_file index_with_data_file_fixed1 iidx_with_data_builder )

add_executable( iidx_test2 iidx_test2.cpp )
target_link_libraries( iidx_test2 index_with_data_file iidx_with_data_builder )

if (cREGISTER)
add_executable( register_test register_test.cpp )
target_link_libraries( register_test spatial_verif fidx_wrapper_jp_db5 document_map thread_queue register_images ${Boost_LIBRARIES} )
endif (cREGISTER)
