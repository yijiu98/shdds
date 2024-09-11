cmake_minimum_required(VERSION 3.10)

# 项目名称
project(mcudds)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)
# 设置 C++ 标准
# set(CMAKE_CXX_STANDARD 11)  # 或者使用 C++14, C++17 之类的版本
# set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 包含头文件目录
include_directories(
    ${CMAKE_SOURCE_DIR}/app/extern
    ${CMAKE_SOURCE_DIR}/app/src
)

# 查找 app/src 目录下的所有源文件
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/app/src/*.c"
)

# 生成可执行文件
add_executable(mcudds main.c ${SRC_FILES})

# 链接需要的库 (如果有依赖的库可以在这里链接)
# target_link_libraries(mcudds ...)
# 添加 C++ 库的链接
# target_link_libraries(mcudds PRIVATE stdc++)