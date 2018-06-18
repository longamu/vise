add_library( dataset dataset.cpp )
target_link_libraries( dataset sqlite3 )

add_library( document_map document_map.cpp )
target_link_libraries( document_map dataset ${Boost_LIBRARIES} )
