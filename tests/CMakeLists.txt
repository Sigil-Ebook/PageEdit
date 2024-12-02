# Build examples/clean_text to ensure c++ code can
# be linked to newly built gumbo shared library.

cmake_minimum_required(VERSION 3.18)


set( test_executable run_tests )

if( APPLE )
   set(CMAKE_MACOSX_RPATH 1)
endif()

if( UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

set(CXX_SOURCES attribute.cc
                char_ref.cc
                parser.cc
                string_buffer.cc
                string_piece.cc
                test_utils.cc
                tokenizer.cc
                utf8.cc
                vector.cc )


target_include_directories(${PROJECT_NAME} BEFORE PUBLIC ${GUMBO_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS})

add_executable(${test_executable}  ${CXX_SOURCES} )
target_link_libraries( ${test_executable} ${GUMBO_LIBRARIES} GTest::GTest GTest::Main)
  
