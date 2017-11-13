add_library( Connection STATIC Connection.cc )
target_link_libraries( Connection register_images ${ImageMagick_LIBRARIES} ${Boost_LIBRARIES} )

add_library( ImcompServer STATIC ImcompServer.cc Resources.cc)
target_link_libraries( ImcompServer Connection ${Boost_LIBRARIES} )
