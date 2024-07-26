########################################################
#
#  This is a CMake configuration file.
#  To use it you need CMake which can be
#  downloaded from here:
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0.5 )

#############################################################################

# We use the lower case name
# on UNIX systems other than Mac OS X
if ( WIN32 OR APPLE )
    project( PageEdit )
else()
    project( pageedit )
endif()

#############################################################################

# Set Inno minimum Windows version 
if ( NOT DEFINED BUILD_FOR_WIN7 )
    # Windows 10 (1809)
    set( BUILD_FOR_WIN7 0 )
    set ( WIN_MIN_VERSION 10.0.17763 )
else()
    # Windows 7sp1
    set ( WIN_MIN_VERSION 6.1.7601 )
endif()

set( CMAKE_DEBUG_POSTFIX "d" )
# Avoid the whole Release/Debug folder creation with Visual Studio
if (MSVC_IDE)
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/bin")
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/bin")
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_BINARY_DIR}/lib")
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_BINARY_DIR}/lib")
endif()
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )

set( PAGEEDIT_MAJOR_VERSION 2 )
set( PAGEEDIT_MINOR_VERSION 3 )
set( PAGEEDIT_REVISION_VERSION 0 )
set( PAGEEDIT_FULL_VERSION ${PAGEEDIT_MAJOR_VERSION}.${PAGEEDIT_MINOR_VERSION}.${PAGEEDIT_REVISION_VERSION} )

if( UNIX AND NOT APPLE )
    include(GNUInstallDirs)
    #Use -DCMAKE_SKIP_RPATH=1 to cancel all the below
    set( CMAKE_SKIP_BUILD_RPATH  FALSE )
    set( CMAKE_BUILD_WITH_INSTALL_RPATH FALSE )
    set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE )
endif()

# Make sure gumbo builds as static
if(NOT DEFINED GUMBO_STATIC_LIB)
  set(GUMBO_STATIC_LIB 1)
endif()

# Prevent some testing/copying that we don't want or use
if(NOT DEFINED GUMBO_IS_SUBTREE)
  set(GUMBO_IS_SUBTREE 1)
endif()

add_subdirectory( gumbo_subtree/src )

include(CMakeLists6.txt)

