add_library( evaluator evaluator.cpp )
target_link_libraries( evaluator document_map par_queue ${Boost_LIBRARIES} )
