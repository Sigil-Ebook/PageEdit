########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0 ) 

project(gumbo C)

if ( ${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR} )
    message( FATAL_ERROR "You are trying to run CMake from the <top_folder>/src/ directory, "
                         "instead of just from the <top_folder> directory.\nDO NOT DO THIS.\n"
                         "The correct way looks like this:\n"
                         "  cmake -G '<generator_name>' /path/to/topmost/folder/in/source/package\n"
                         "You will probably now need to first clean your build directory." )
endif()

set(GUMBO_LIBRARIES ${PROJECT_NAME} CACHE INTERNAL "")
set(GUMBO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")

file(GLOB SOURCES *.c)
list(SORT SOURCES)

if( APPLE )
     set(CMAKE_MACOSX_RPATH 1)
endif()

if ( MSVC AND NOT GUMBO_STATIC_LIB )
     set(SOURCES ${SOURCES} ${CMAKE_SOURCE_DIR}/visualc/include/gumbo_windll.def)
endif()

if ( NOT GUMBO_STATIC_LIB  )
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()

if ( NOT GUMBO_IS_SUBTREE )
    # Copy python scripts to build directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cmake -E copy_directory
				${CMAKE_SOURCE_DIR}/python ${TOP_BUILD_LEVEL}/python)
    # Copy testdata to build directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cmake -E copy_directory
				${CMAKE_SOURCE_DIR}/testdata ${TOP_BUILD_LEVEL}/testdata)
    # Copy python module setup.py to build directory
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cmake -E copy 
	# 		${CMAKE_SOURCE_DIR}/setup.py ${TOP_BUILD_LEVEL})
endif()

if ( MSVC AND NOT GUMBO_STATIC_LIB )
    # Copy Windows .dll from bin to .libs so existing python tests can run.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cmake -E copy 
     		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.dll ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif()

# Special compiler and linker flags for MSVC
if(MSVC)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/inc/strings.h)
        FILE(WRITE ${CMAKE_BINARY_DIR}/inc/strings.h
          "/*Dummy file to satisfy source file dependencies on Windows platform*/\n"
        )
    endif()
    set (EXTRA_INC ${CMAKE_BINARY_DIR}/inc/)
    include_directories(SYSTEM ${EXTRA_INC})

    add_definitions(-Dstrcasecmp=_stricmp)
    add_definitions(-Dstrncasecmp=_strnicmp)
    add_definitions(-Dinline=__inline)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /Oi /GL")
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} /LTCG")
    set(CMAKE_STATIC_LINKER_FLAGS_RELEASE "${CMAKE_STATIC_LINKER_FLAGS} /LTCG")
endif()

if( UNIX )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
endif()
