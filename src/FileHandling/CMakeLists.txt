cmake_minimum_required(VERSION 3.15)
project(Wepp-FileHandling CXX)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../../external/*/include")

add_library(Wepp-FileHandling STATIC ${SOURCES})
target_include_directories(Wepp-FileHandling PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${LIBRARY_INCLUDES})
