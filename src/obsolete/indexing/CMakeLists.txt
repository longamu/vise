add_library( fidx_wrapper_jp_db5 fidx_wrapper_jp_db5.cpp )
target_link_libraries( fidx_wrapper_jp_db5 ellipse feat_getter jpe3_jp_db5 ${Boost_LIBRARIES} )

add_library( fidx_aug fidx_aug.cpp )
target_link_libraries( fidx_aug ellipse image_graph )

add_library( iidx_in_ram iidx_in_ram.cpp )
target_link_libraries( iidx_in_ram slow_construction ${Boost_LIBRARIES} )

add_library( fidx_in_ram fidx_in_ram.cpp )
target_link_libraries( fidx_in_ram thread_queue slow_construction ${Boost_LIBRARIES} )

add_library( iidx_with_data_builder iidx_with_data_builder.cpp )
target_link_libraries( iidx_with_data_builder index_with_data_file char_streams )
