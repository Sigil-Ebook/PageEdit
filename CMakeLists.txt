########################################################
#  
#  This is a CMake configuration file.
#  To use it you need CMake which can be 
#  downloaded from here: 
#    http://www.cmake.org/cmake/resources/software.html
#
#########################################################

cmake_minimum_required( VERSION 3.0.0 )

#############################################################################

# We use the lower case name
# on UNIX systems other than Mac OS X
if ( WIN32 OR APPLE )
    project( PageEdit )
else()
    project( pageedit )
endif()

#############################################################################

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

set( PAGEEDIT_MAJOR_VERSION 0 )
set( PAGEEDIT_MINOR_VERSION 1 )
set( PAGEEDIT_REVISION_VERSION 0 )
set( PAGEEDIT_FULL_VERSION ${PAGEEDIT_MAJOR_VERSION}.${PAGEEDIT_MINOR_VERSION}.${PAGEEDIT_REVISION_VERSION} )

if( UNIX AND NOT APPLE )
   include(GNUInstallDirs)
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

# Qt5 packages minimum version 5.12
find_package(Qt5 5.12 COMPONENTS Core Network Svg WebEngine WebEngineWidgets Widgets Xml XmlPatterns Concurrent PrintSupport LinguistTools)
set(CMAKE_AUTOMOC ON)

set( SOURCE_FILES
    main.cpp
    pageedit_constants.cpp
    MainWindow.cpp
    SelectCharacter.cpp
    WebPageEdit.cpp
    WebViewEdit.cpp
    Inspector.cpp
    Preferences.cpp
    AppearanceWidget.cpp
    LanguageWidget.cpp
    UILanguage.cpp
    SettingsStore.cpp
    Utility.cpp
    GumboInterface.cpp
    )

set( HEADER_FILES
    pageedit_constants.h
    pageedit_exception.h
    MainWindow.h
    Zoomable.h
    Viewer.h
    ElementIndex.h
    SelectCharacter.h
    WebPageEdit.h
    WebViewEdit.h
    Inspector.h
    Preferences.h
    PreferencesWidget.h
    AppearanceWidget.h
    LanguageWidget.h
    UILanguage.h
    SettingsStore.h
    Utility.h
    GumboInterface.h
    )

set( FORM_FILES
    main.ui
    SelectCharacter.ui
    Preferences.ui
    PAppearanceWidget.ui
    PLanguageWidget.ui
    )

set( RESOURCE_FILES
    icons/icons.qrc
    javascript/javascript.qrc
    )

file( GLOB TS_FILES ts/pageedit_*.ts )

set( LINUX_DESKTOP_FILE
    platform/linux/freedesktop/pageedit.desktop
    )

set( LINUX_DESKTOP_ICON_FILE
    icons/app_icon_48.png
   )

#############################################################################

# Runs UIC on specified files
qt5_wrap_ui( UI_FILES_H ${FORM_FILES} )
set_property( SOURCE ${UI_FILES_H} PROPERTY SKIP_AUTOMOC ON )
# Runs RCC on specified files
qt5_add_resources( QRC_FILES_CPP ${RESOURCE_FILES} )
set_property( SOURCE ${QRC_FILES_CPP} PROPERTY SKIP_AUTOMOC ON )
# Runs lrelease on the specified files
qt5_add_translation( QM_FILES ${TS_FILES} )

# Define the PageEdit version string for use in source files
#set_source_files_properties( About.cpp PROPERTIES COMPILE_DEFINITIONS PAGEEDIT_FULL_VERSION="${PAGEEDIT_FULL_VERSION}" )

set( ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES_H} ${QRC_FILES_CPP} ${QM_FILES} )

# Adding resource (RC) files for Windows
# Grab the current year so copyright notice is updated on Windows file properties
string( TIMESTAMP BUILD_YEAR "%Y" )
if ( WIN32 )
    configure_file( version.rc.in ${PROJECT_BINARY_DIR}/version.rc )
    set( WINDOWS_RC_FILES
         ${PROJECT_BINARY_DIR}/version.rc
         )
    list( APPEND ALL_SOURCES ${WINDOWS_RC_FILES} )
endif()

# Apple bundle configuration
if( APPLE )
    exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources")
    # exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/polyfills")
    # exec_program("unzip ${MATHJAX_ZIP} -d ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/polyfills")
    exec_program("cp ${PROJECT_SOURCE_DIR}/icons/PageEdit.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources")
    exec_program("cp ${PROJECT_SOURCE_DIR}/icons/xhtml.icns ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources")
    # Create translation directory.
    exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/translations")
    exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/lib")

    # Copy the PLIST file...
    exec_program("cp ${PROJECT_SOURCE_DIR}/platform/mac/MacOSXBundleInfo.plist ${PROJECT_BINARY_DIR}")

    # ...and set the PageEdit version string
    exec_program("sed -i -e 's/PEVERSION/${PAGEDIT_FULL_VERSION}/g' ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist")
endif()

#############################################################################

# We need to pick up the ui*.h files,
# and the headers for the linked-to libraries.
# The directories are in reverse order because we're using before to
# put our include dirs before any system ones.
include_directories( BEFORE
                     ${GUMBO_INCLUDE_DIRS}
                     ${CMAKE_CURRENT_SOURCE_DIR}
                     ${CMAKE_CURRENT_BINARY_DIR} )

#############################################################################

# We make bundles for Mac OS X
if ( APPLE )
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${ALL_SOURCES} )
    set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist )
    set_target_properties(${PROJECT_NAME} PROPERTIES CMAKE_SKIP_BUILD_RPATH  TRUE)
# ...and a normal executable for everything else.
else()
    add_executable( ${PROJECT_NAME} WIN32 ${ALL_SOURCES} )
endif()

# No need to explicity link Qt5::WinMain or to use the qt5_use_modules macro since CMAKE 2.8.11. We require CMAKE 3.0.0
target_link_libraries( ${PROJECT_NAME} ${GUMBO_LIBRARIES} Qt5::Widgets  Qt5::Xml  Qt5::XmlPatterns  Qt5::PrintSupport
                                     Qt5::Svg  Qt5::WebEngine  Qt5::WebEngineWidgets  Qt5::Network  Qt5::Concurrent )

#############################################################################

# "Link time code generation" flags for MSVC
if( MSVC )
    add_definitions( /DUNICODE /D_UNICODE /DHAVE_ROUND )
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Oi /GL" ) 
    set( CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG" )

# "Print all warnings" flag for GCC
elseif( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -Wall )
endif()

if( UNIX AND NOT APPLE )
    if ( NOT SHARE_INSTALL_PREFIX )
        set ( SHARE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} )
    endif()

    set ( PAGEEDIT_SHARE_ROOT "${SHARE_INSTALL_PREFIX}/share/${PROJECT_NAME}" )
    
    # Set some defines that pageedit_constants.cpp can then access
    set_property (
        SOURCE pageedit_constants.cpp
        PROPERTY COMPILE_DEFINITIONS
        PAGEEDIT_SHARE_ROOT="${PAGEEDIT_SHARE_ROOT}"
    )
    # Standard Linux 'make install'
    install( TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} )
    install( FILES ${LINUX_DESKTOP_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/applications/ )
    install( FILES ${LINUX_DESKTOP_ICON_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/pixmaps RENAME pageedit.png )
    install( FILES ${QM_FILES} DESTINATION ${PAGEEDIT_SHARE_ROOT}/translations/ )
endif()

# For Mac, add frameworks and make a DMG
if( APPLE )
    set( WORK_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" )
    set( MAIN_PACKAGE_DIR ${WORK_DIR}/PageEdit.app )

    if ("${CODE_SIGN_ID}" STREQUAL "")
        add_custom_target(  addframeworks
                            COMMAND macdeployqt PageEdit.app
                            WORKING_DIRECTORY ${WORK_DIR}
                            DEPENDS ${PROJECT_NAME} )
    ELSE ()
        add_custom_target(  addframeworks
                            COMMAND macdeployqt  PageEdit.app -codesign="${CODE_SIGN_ID}"
                            WORKING_DIRECTORY ${WORK_DIR}
                            DEPENDS ${PROJECT_NAME} )
    ENDIF ()

    add_custom_target(  makedmg
                        COMMAND macdeployqt PageEdit.app -dmg
                        WORKING_DIRECTORY ${WORK_DIR})

    add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${PROJECT_BINARY_DIR}/*.qm ${WORK_DIR}/PageEdit.app/Contents/translations/ )
    # add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/../lib/*.dylib ${WORK_DIR}/PageEdit.app/Contents/lib/ )
    # add_custom_command( TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -rpath ${CMAKE_BINARY_DIR}/lib "@executable_path/../lib" ${WORK_DIR}/PageEdit.app/Contents/MacOS/PageEdit )
endif()
