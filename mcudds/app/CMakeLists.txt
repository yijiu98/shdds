# 项目名称
project(mcudds C)

# 设置 C 标准
set(CMAKE_C_STANDARD 99)

# 包含头文件目录（ginclude 为仓库级公共头文件，无需再拷贝到 extern）
include_directories(
    ${CMAKE_SOURCE_DIR}/../ginclude
    ${CMAKE_SOURCE_DIR}/app/mculib
)

# 查找 mculib 目录下的所有源文件
file(GLOB SRC_FILES
    "${CMAKE_SOURCE_DIR}/app/mculib/*.c"
)

# 生成可执行文件
add_executable(mcudds main.c ${SRC_FILES})
