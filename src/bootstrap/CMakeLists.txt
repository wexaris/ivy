cmake_minimum_required( VERSION 3.6 )
project( bootstrap )

# get the directory of the current file
set( CURR_DIR ${CMAKE_CURRENT_LIST_DIR} )

# set the compiler version
# should be C++17 for [[attributes]] to work
set( CMAKE_CXX_STANDARD 17 )

# set inlude path
include_directories( ${CURR_DIR} )

# all of the source files to compile
set( SRC_FILES
		${CURR_DIR}/driver/driver.cpp
		${CURR_DIR}/parser/parser.cpp
		${CURR_DIR}/lexer/lexer.cpp
		${CURR_DIR}/source/source_map.cpp
		${CURR_DIR}/token/token_translate.cpp
	)

add_executable( ivy.out ${SRC_FILES} )