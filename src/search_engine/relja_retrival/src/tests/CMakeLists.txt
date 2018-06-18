add_executable( mpi_queue_test mpi_queue_test.cpp )
target_link_libraries( mpi_queue_test mpi_queue ${Boost_LIBRARIES} )

add_executable( thread_queue_test thread_queue_test.cpp )
target_link_libraries( thread_queue_test thread_queue ${Boost_LIBRARIES} )

add_executable( feat_extract feat_extract.cpp )
target_link_libraries( feat_extract feat_standard )

add_executable( pq_test pq_test.cpp )
target_link_libraries( pq_test product_quant nn_evaluator nn_compressed char_streams )

add_executable( nn_retriever_test nn_retriever_test.cpp )
target_link_libraries( nn_retriever_test nn_single_retriever product_quant coarse_residual index_with_data_file index_with_data_file_fixed1 )
