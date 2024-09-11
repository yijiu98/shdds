cmake_minimum_required ( VERSION 3.10 )

# Print some standard CMake variables
message(STATUS "CMake version: ${CMAKE_VERSION}")

project ( mcudds )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)

add_subdirectory ( app )
