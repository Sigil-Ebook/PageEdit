# Build examples/clean_text to ensure c++ code can
# be linked to newly built gumbo shared library.

cmake_minimum_required(VERSION 3.18) 

# project(examples)


if( APPLE )
     set(CMAKE_MACOSX_RPATH 1)
endif()

if( UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()

set(CXX_APPS clean_text find_links prettyprint positions_of_class well_formed serialize )

set(C_APPS get_title )

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC ${GUMBO_INCLUDE_DIRS})

foreach( app_name ${CXX_APPS} )
   set( app_source_file "${app_name}.cc" )
   add_executable( ${app_name} ${app_source_file} )
   target_link_libraries( ${app_name} ${GUMBO_LIBRARIES} )
endforeach( app_name ${CXX_APPS} )
 
foreach( app_name ${C_APPS} )
   set( app_source_file "${app_name}.c" )
   add_executable( ${app_name} ${app_source_file} )
   target_link_libraries( ${app_name} ${GUMBO_LIBRARIES} )
endforeach( app_name ${C_APPS} )


