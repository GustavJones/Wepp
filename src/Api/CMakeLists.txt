cmake_minimum_required(VERSION 3.15)
project(Wepp-Api CXX)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_SOURCE_DIR}/external/*/include")
add_executable(Wepp-Api ${SOURCES})
target_include_directories(Wepp-Api PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../include" ${LIBRARY_INCLUDES})
target_link_libraries(Wepp-Api PRIVATE Wepp-Server)
