
add_library( jpe3_compress
    minilzo.c
)
# z is zlib
target_link_libraries( jpe3_compress z )

