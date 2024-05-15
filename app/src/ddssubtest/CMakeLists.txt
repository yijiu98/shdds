project(ddssubtest)
message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_NAME       = ${PROJECT_NAME}")

add_definitions(-DPROJECT_NAME=\"${PROJECT_NAME}\")

if (CMAKE_MODE_RELEASE)
include_directories(
    ${CMAKE_BINARY_DIR}/../extern/include
)

link_directories(
    ${CMAKE_BINARY_DIR}/../extern/lib
)
else()
include_directories(
    ${CMAKE_BINARY_DIR}/../extern/include
)

link_directories(
    ${CMAKE_BINARY_DIR}/../extern/lib
)
endif()

# ble mgr lib link
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/include)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/dep/libs)

add_compile_options(-D_GNU_SOURCE)


include_directories(CmdMgr)

aux_source_directory(CmdMgr CmdMgr_SRC)


add_executable( ${PROJECT_NAME} sub.cpp  ${CmdMgr_SRC} ) 

target_link_libraries(${PROJECT_NAME} PRIVATE pthread rt  shdds) 

