cmake_minimum_required(VERSION 3.15)
project(Wepp-Builder CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_SOURCE_DIR}/external/*/include")

add_executable(Wepp-Builder ${SOURCES})
target_include_directories(Wepp-Builder PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${LIBRARY_INCLUDES})
target_link_libraries(Wepp-Builder PRIVATE Wepp-Server wxcore wxbase wxxrc wxaui wxpropgrid wxwebview)

if(WIN32)
  set_target_properties(Wepp-Builder PROPERTIES WIN32_EXECUTABLE TRUE)
endif()
