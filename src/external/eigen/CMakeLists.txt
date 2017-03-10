# Include Eigen, needs RegexUtils
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include( RegexUtils )
test_escape_string_as_regex()
add_subdirectory( Eigen )