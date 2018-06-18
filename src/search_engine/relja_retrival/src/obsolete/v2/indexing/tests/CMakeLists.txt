add_executable( eval_v2_as_v1 eval_v2_as_v1.cpp )
target_link_libraries( eval_v2_as_v1 document_map evaluator fidx_v2_as_v1 iidx_v2_as_v1 par_queue proto_db_file spatial_verif tfidf )
