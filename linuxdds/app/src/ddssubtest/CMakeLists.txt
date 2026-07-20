project(ddssubtest)

add_executable(${PROJECT_NAME} sub.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/../ginclude)

target_link_libraries(${PROJECT_NAME} PRIVATE shdds)
