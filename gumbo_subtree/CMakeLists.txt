########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0 ) 

project(gumbo) 

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/.libs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/.libs)

set(TOP_BUILD_LEVEL ${PROJECT_BINARY_DIR})

if(NOT DEFINED GUMBO_STATIC_LIB)
  set(GUMBO_STATIC_LIB 0)
endif()

if(NOT DEFINED GUMBO_IS_SUBTREE)
  set(GUMBO_IS_SUBTREE 0)
endif()

add_subdirectory(src/)
add_subdirectory(examples/)

