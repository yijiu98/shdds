project(mcubridge)
message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME       = ${PROJECT_NAME}")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

message(STATUS "CMAKE_BINARY_DIR is: ${CMAKE_BINARY_DIR}")
include_directories(
    ${CMAKE_BINARY_DIR}/../app/mculib
    ${CMAKE_BINARY_DIR}/../app/extern
    ${CMAKE_BINARY_DIR}/../app/extern/include
    ${CMAKE_BINARY_DIR}/../app/bridgeMgr
)

link_directories(
    ${CMAKE_BINARY_DIR}/../app/extern/lib
)

add_compile_options(-D_GNU_SOURCE)

message(STATUS "CMAKE_SOURCE_DIR is: ${CMAKE_SOURCE_DIR}")
file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/app/mculib/*.c"
    "${CMAKE_SOURCE_DIR}/app/bridgeMgr/*.cpp"
)

add_executable( ${PROJECT_NAME} main.cpp  ${SRC_FILES}) 

target_link_libraries(${PROJECT_NAME} PRIVATE pthread rt shdds) 

