cmake_minimum_required(VERSION 3.15)
project(Wepp-Server CXX)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_SOURCE_DIR}/external/*/include")
add_executable(WeppServer ${SOURCES})
target_include_directories(WeppServer PRIVATE ${CMAKE_SOURCE_DIR}/include ${LIBRARY_INCLUDES})
target_link_libraries(WeppServer PRIVATE GNetworking)
target_compile_options(WeppServer PRIVATE -Wall -Werror)
