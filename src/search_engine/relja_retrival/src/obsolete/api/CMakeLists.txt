add_executable( api api.cpp )
target_link_libraries( api spatial_api tfidf spatial_verif multi_query iidx_in_ram fidx_in_ram fidx_wrapper_jp_db5 fidx_v2_as_v1 iidx_v2_as_v1 proto_db_file clst_centres feat_standard document_map ${Boost_LIBRARIES} ${fastann_LIBRARIES} ${MPI_LIBRARIES} )
