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
set( PAGEEDIT_MINOR_VERSION 8 )
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


if( UNIX AND NOT APPLE )
    # Qt5 packages minimum version 5.9 for Linux
    set(QT5_NEEDED 5.9)
else()
    # Qt5 packages minimum version 5.12 for Windows/Mac
    set(QT5_NEEDED 5.12)
endif()
find_package(Qt5 ${QT5_NEEDED} COMPONENTS Core Network WebEngine WebEngineWidgets Widgets Xml Concurrent PrintSupport LinguistTools)
set(CMAKE_AUTOMOC ON)

set( SOURCE_FILES
    main.cpp
    pageedit_constants.cpp
    AppEventFilter.cpp
    MainWindow.cpp
    SelectCharacter.cpp
    WebPageEdit.cpp
    WebViewEdit.cpp
    Inspector.cpp
    Preferences.cpp
    AppearanceWidget.cpp
    GeneralSettings.cpp
    UILanguage.cpp
    UIDictionary.cpp
    SettingsStore.cpp
    FocusSelectLineEdit.cpp
    SearchToolbar.cpp
    Utility.cpp
    GumboInterface.cpp
    )

set( HEADER_FILES
    pageedit_constants.h
    pageedit_exception.h
    AppEventFilter.h
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
    GeneralSettings.h
    UILanguage.h
    UIDictionary.h
    SettingsStore.h
    FocusSelectLineEdit.h
    SearchToolbar.h
    Utility.h
    GumboInterface.h
    )

set( FORM_FILES
    main.ui
    SelectCharacter.ui
    Preferences.ui
    PAppearanceWidget.ui
    PGeneralSettings.ui
    SearchToolbar.ui
    )

set( RESOURCE_FILES
    icons/icons.qrc
    javascript/javascript.qrc
    )

file( GLOB TS_FILES ts/pageedit_*.ts )

set( DICTIONARY_FILES
     de_DE
     en_GB
     en_US
     es
     fr
   )

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
set_source_files_properties( Utility.cpp PROPERTIES COMPILE_DEFINITIONS PAGEEDIT_VERSION="${PAGEEDIT_FULL_VERSION}" )
set_source_files_properties( main.cpp PROPERTIES COMPILE_DEFINITIONS PAGEEDIT_VERSION="${PAGEEDIT_FULL_VERSION}" )

set( ALL_SOURCES ${SOURCE_FILES} ${HEADER_FILES} ${UI_FILES_H} ${QRC_FILES_CPP} ${QM_FILES} )

# Adding resource (RC) files for Windows
# Grab the current year so copyright notice is updated on Windows file properties
string( TIMESTAMP BUILD_YEAR "%Y" )
if ( WIN32 )
    configure_file( version.rc.in ${PROJECT_BINARY_DIR}/version.rc )
    set( WINDOWS_RC_FILES
         icons/icon.rc
         ${PROJECT_BINARY_DIR}/version.rc
         )
    list( APPEND ALL_SOURCES ${WINDOWS_RC_FILES} )
endif()

# Apple bundle configuration
if( APPLE )
    exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources")

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

    # create a directory for holding spellcheck dictionaries to be found by qtwebengine
    # convert them to .bdic files in the correct location
    exec_program("mkdir -p ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources/qtwebengine_dictionaries")
    foreach( DICT ${DICTIONARY_FILES} )
        exec_program("qwebengine_convert_dict ${PROJECT_SOURCE_DIR}/dictionaries/${DICT}.dic ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/PageEdit.app/Contents/Resources/qtwebengine_dictionaries/${DICT}.bdic")
    endforeach( DICT )

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
target_link_libraries( ${PROJECT_NAME} ${GUMBO_LIBRARIES} Qt5::Widgets  Qt5::Xml  Qt5::PrintSupport
                                     Qt5::WebEngine  Qt5::WebEngineWidgets  Qt5::Network  Qt5::Concurrent )

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

get_target_property(QMAKE_EXECUTABLE Qt5::qmake LOCATION)
function(QUERY_QMAKE VAR RESULT)
    exec_program(${QMAKE_EXECUTABLE} ARGS "-query ${VAR}" RETURN_VALUE return_code OUTPUT_VARIABLE output )
    if(NOT return_code)
        file(TO_CMAKE_PATH "${output}" output)
        set(${RESULT} ${output} PARENT_SCOPE)
    endif(NOT return_code)
endfunction(QUERY_QMAKE)

###############################################################################

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
    
    query_qmake( QT_INSTALL_BINS QT_INSTALL_BINS_DIR )
    query_qmake( QT_INSTALL_DATA QT_INSTALL_DATA_DIR )

    # Bundled dictionaries will be coverted to qtwebengine spellcheck dicts with qwebengine_convert_dict
    # by default and installed to qmake -query QT_INSTALL_DATA + ./qtwebengine_dictionaries.
    # Use -DINSTALL_BUNDLED_DICTS=0 to prevent this.
    set( CONVERTDICT ${QT_INSTALL_BINS_DIR}/qwebengine_convert_dict )
    if ( NOT DEFINED INSTALL_BUNDLED_DICTS )
        set ( INSTALL_BUNDLED_DICTS 1 )
    endif()

    set( SPELLCHECK_DICTS_SRC ${PROJECT_SOURCE_DIR}/dictionaries )
    # Create converted dics in bin folder so spellcheck can work in the build directory
    set( CONVERTED_DICTS_DEST ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qtwebengine_dictionaries )
    # Remove previous directories
    if ( EXISTS ${CONVERTED_DICTS_DEST} )
        file( REMOVE_RECURSE ${CONVERTED_DICTS_DEST} )
    endif()
    # Create convert the spellcheck dictionaries into the newly created destination folder
    if ( INSTALL_BUNDLED_DICTS )
        file( MAKE_DIRECTORY ${CONVERTED_DICTS_DEST} )
        foreach( DICT ${DICTIONARY_FILES} )
            execute_process( COMMAND ${CONVERTDICT} ${SPELLCHECK_DICTS_SRC}/${DICT}.dic ${CONVERTED_DICTS_DEST}/${DICT}.bdic )
        endforeach( DICT )    
    endif()

    # Standard Linux 'make install'
    install( TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} )
    install( FILES ${LINUX_DESKTOP_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/applications/ )
    install( FILES ${LINUX_DESKTOP_ICON_FILE} DESTINATION ${SHARE_INSTALL_PREFIX}/share/pixmaps RENAME pageedit.png )
    set( ICON_SIZE 32 48 128 256)
    foreach( SIZE ${ICON_SIZE} )
        install( FILES ${PROJECT_SOURCE_DIR}/icons/app_icon_${SIZE}.png DESTINATION
                               ${SHARE_INSTALL_PREFIX}/share/icons/hicolor/${SIZE}x${SIZE}/apps RENAME pageedit.png )
    endforeach( SIZE )
    install( FILES ${PROJECT_SOURCE_DIR}/icons/app_icon_scalable.svg DESTINATION
                               ${SHARE_INSTALL_PREFIX}/share/icons/hicolor/scalable/apps RENAME pageedit.svg )
    install( FILES ${QM_FILES} DESTINATION ${PAGEEDIT_SHARE_ROOT}/translations/ )
    # Install converted dicts to qmake -query QT_INSTALL_DATA + ./qtwebengine_dictionaries
    if ( INSTALL_BUNDLED_DICTS )
        install( DIRECTORY ${CONVERTED_DICTS_DEST} DESTINATION ${QT_INSTALL_DATA_DIR} )
    endif()
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

if ( MSVC )
    query_qmake( QT_INSTALL_BINS QT_INSTALL_BINS )
    query_qmake( QT_INSTALL_TRANSLATIONS QT_TRANSLATIONS_DIR )

    set( TEMP_PACKAGE_DIR ${CMAKE_BINARY_DIR}/temp_folder )
    set( MAIN_PACKAGE_DIR ${TEMP_PACKAGE_DIR}/PageEdit )
    set( OUTPUT_PACKAGE_DIR ${CMAKE_BINARY_DIR}/deploy )
    set( WINDEPLOYQT ${QT_INSTALL_BINS}/windeployqt.exe )
    set( CONVERTDICT ${QT_INSTALL_BINS}/qwebengine_convert_dict.exe )

    set( pf_x86 "ProgramFiles(x86)" )
    find_program(SEVENZIP_BIN
	    NAMES 7z 7za
	    HINTS "$ENV{${pf_x86}}/7-zip" "$ENV{ProgramFiles}/7-zip" "$ENV{ProgramW6432}/7-zip"
	    PATH_SUFFIXES bin
	    DOC "7zip executable"
	)
    message( "\n7zip location:  ${SEVENZIP_BIN}\n" )

    if( DEPLOY_SFX )
        set( SEVENZIP_ARGS a -t7z -sfx -mx9 -xr!bearer )
        set( SEVENZIP_OUTPUT "${PROJECT_NAME}_${PAGEEDIT_FULL_VERSION}.exe" )
    else()
        set( SEVENZIP_ARGS a -tzip -xr!bearer )
        set( SEVENZIP_OUTPUT "${PROJECT_NAME}_${PAGEEDIT_FULL_VERSION}.zip" )
    endif()

    add_custom_target( deploy
                       COMMAND ${CMAKE_COMMAND} -E echo "For this to work, 7zip needs to be installed (and possibly on the system path)."
                       COMMAND del ${SEVENZIP_OUTPUT}
                       COMMAND ${SEVENZIP_BIN} ${SEVENZIP_ARGS} ${SEVENZIP_OUTPUT} ${MAIN_PACKAGE_DIR}
                       WORKING_DIRECTORY ${OUTPUT_PACKAGE_DIR} )

    # Intermediate step to create folders and copy files
    set( TARGET_FOR_COPY copyfiles )
    add_custom_target( ${TARGET_FOR_COPY}
                       COMMENT "Copying files to temporary location..."
                       DEPENDS ${PROJECT_NAME} )

    add_dependencies( deploy ${TARGET_FOR_COPY} )

    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAIN_PACKAGE_DIR}
                        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_PACKAGE_DIR} )

    # Set the path of the application executable
    set( EXE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )

    # Copy the application executable
    add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy ${EXE_PATH} ${MAIN_PACKAGE_DIR} )

    add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD
                        COMMAND ${WINDEPLOYQT} --release --no-translations --no-compiler-runtime --dir ${MAIN_PACKAGE_DIR}
                        --libdir ${MAIN_PACKAGE_DIR} ${MAIN_PACKAGE_DIR}/${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} )

    # Copy the translation qm files
    if( NOT EXISTS ${MAIN_PACKAGE_DIR}/translations )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${MAIN_PACKAGE_DIR}/translations/ )
    endif()
    foreach( QM ${QM_FILES} )
        # Copy PageEdit's qm files
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${QM} ${MAIN_PACKAGE_DIR}/translations/ )
        # Copy Qt's qm files that coincide with the above
        # message( "QM = ${QM}")
        string( REGEX REPLACE "(.*)(pageedit_)(.*)(\\.qm)" "\\1;\\2;\\3;\\4" PATH_ELEMENTS "${QM}" )
        list( GET PATH_ELEMENTS 2 LANG_ID )
        # message( "LANG_ID = ${LANG_ID}")
        set( QTBASE_QM ${QT_TRANSLATIONS_DIR}/qtbase_${LANG_ID}.qm )
        # message( "QTBASE_QM = ${QTBASE_QM}" )
        if ( EXISTS ${QTBASE_QM} )
            add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${QTBASE_QM} ${MAIN_PACKAGE_DIR}/translations/ )
        endif()
    endforeach( QM )
    
    # Convert and copy the spellcheck dictionaries
    if( NOT EXISTS ${MAIN_PACKAGE_DIR}/qtwebengine_dictionaries )
        add_custom_command( TARGET ${TARGET_FOR_COPY} PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${MAIN_PACKAGE_DIR}/qtwebengine_dictionaries/ )
    endif()
    foreach( DICT ${DICTIONARY_FILES} )
        add_custom_command( TARGET ${TARGET_FOR_COPY} POST_BUILD
        COMMAND ${CONVERTDICT} ${PROJECT_SOURCE_DIR}/dictionaries/${DICT}.dic ${MAIN_PACKAGE_DIR}/qtwebengine_dictionaries/${DICT}.bdic )
    endforeach( DICT )

    # Remove the temp directory used for building the installer
    add_custom_command( TARGET deploy POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E remove_directory ${TEMP_PACKAGE_DIR}
                        COMMENT "Removing temporary directory..." )
endif( MSVC )
