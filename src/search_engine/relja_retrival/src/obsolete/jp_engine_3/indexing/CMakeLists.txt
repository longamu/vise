add_subdirectory( compression )

add_library( jpe3_jp_db5 jp_db5.cpp )
target_link_libraries( jpe3_jp_db5 jpe3_compress )
set_target_properties(jpe3_jp_db5 PROPERTIES LINKER_LANGUAGE C)
