cmake_minimum_required( VERSION 3.6 )
project( ivy )

# lower case duilt type
# used for directory paths
string( TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE )

# set output directories
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${BUILD_TYPE} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYPE} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib/${BUILD_TYPE} )

# build the boostrap ivy compiler
add_subdirectory( ${PROJECT_SOURCE_DIR}/src/bootstrap )