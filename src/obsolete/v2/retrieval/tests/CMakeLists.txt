add_executable( tfidf_v2_compute tfidf_v2_compute.cpp )
target_link_libraries( tfidf_v2_compute proto_db proto_db_file proto_index tfidf tfidf_v2 )
