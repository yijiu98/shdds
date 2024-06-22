# 指定项目的名称
project(shdds)

# 设置库的版本号
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME       = ${PROJECT_NAME}")

# 表示定义一个宏，后面紧跟着要定义的宏名称和其对应的值
add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

# 指定头文件的目录
include_directories(include src)

# 将指定目录下的所有源文件添加到变量 SUBTEST 中
aux_source_directory(src SUBTEST)

# 创建一个共享库，并将变量 SUBTEST 中的源文件添加到该库中
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SUBTEST})

# 设置库的版本
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}
    SOVERSION ${VERSION_MAJOR}
)

# 设置了目标库的属性，如输出名称、是否清理输出等
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)

# 将项目中的 include 文件夹复制到构建目录中，以便于构建后的程序可以使用这些头文件
file(COPY include DESTINATION ${CMAKE_BINARY_DIR}/../build)
