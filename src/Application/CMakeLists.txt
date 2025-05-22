cmake_minimum_required(VERSION 3.15)
project(Wepp-Server CXX)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_SOURCE_DIR}/external/*/include")
add_executable(Wepp ${SOURCES})
target_include_directories(Wepp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${LIBRARY_INCLUDES} ${OPENSSL_INCLUDE_DIR})
target_link_libraries(Wepp PRIVATE Wepp-Server)
