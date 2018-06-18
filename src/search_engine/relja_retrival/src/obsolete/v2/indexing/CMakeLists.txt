add_subdirectory( tests )

add_library( fidx_v2_as_v1 fidx_v2_as_v1.cpp )
target_link_libraries( fidx_v2_as_v1 index_entry_util proto_db proto_index )

add_library( iidx_v2_as_v1 iidx_v2_as_v1.cpp )
target_link_libraries( iidx_v2_as_v1 proto_db proto_index )

add_executable( convert_jp_iidx convert_jp_iidx.cpp )
target_link_libraries( convert_jp_iidx iidx_in_ram index_entry.pb proto_db proto_db_file proto_index )

add_executable( convert_jp_fidx convert_jp_fidx.cpp )
target_link_libraries( convert_jp_fidx document_map fidx_in_ram fidx_wrapper_jp_db5 index_entry.pb proto_db proto_db_file proto_index )
