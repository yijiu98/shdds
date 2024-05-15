cmake_minimum_required ( VERSION 3.10 )
#搜索库文件时优先搜索项目指定的目录
SET(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)

message(STATUS "CMake version: ${CMAKE_VERSION}")

project ( platformlib )
#设置库文件和可执行文件的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)#静态库
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/lib)#动态库
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/bin)#可执行文件

#项目源代码所在的目录，用于构建项目的源代码
add_subdirectory ( src )


