# Build examples/clean_text to ensure c++ code can
# be linked to newly built gumbo shared library.

cmake_minimum_required(VERSION 3.0) 

project(clean_text)

set(SOURCES clean_text.cc)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} BEFORE PUBLIC ${GUMBO_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${GUMBO_LIBRARIES})
