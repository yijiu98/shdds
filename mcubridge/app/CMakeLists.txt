project(mcubridge)

# linuxdds 产物位置（需先构建 linuxdds，根目录 build.sh 会自动处理顺序）
set(SHDDS_ROOT ${CMAKE_SOURCE_DIR}/../linuxdds)

include_directories(
    ${CMAKE_SOURCE_DIR}/app/mculib
    ${CMAKE_SOURCE_DIR}/app/bridgeMgr
    ${CMAKE_SOURCE_DIR}/../ginclude
    ${SHDDS_ROOT}/lib/src/libshdds/include
)

link_directories(${SHDDS_ROOT}/out/lib)

# 运行时从 linuxdds/out/lib 加载 libshdds.so
set(CMAKE_BUILD_RPATH ${SHDDS_ROOT}/out/lib)

add_compile_options(-D_GNU_SOURCE)

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_SOURCE_DIR}/app/mculib/*.c"
    "${CMAKE_SOURCE_DIR}/app/bridgeMgr/*.cpp"
)

add_executable(${PROJECT_NAME} main.cpp ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread rt shdds)
