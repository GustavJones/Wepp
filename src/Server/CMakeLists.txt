cmake_minimum_required(VERSION 3.15)
project(Wepp-Server CXX)

option(OPENSSL_USE_STATIC_LIBS "Build Application with Static OpenSSL Libraries" TRUE)

if((NOT DEFINED OPENSSL_ROOT_DIR) OR (OPENSSL_ROOTDIR STREQUAL ""))
  message(STATUS "No SSL Root directory defined. Reading from environment variables.")
  set(OPENSSL_ROOT_DIR  $ENV{OPENSSL_ROOT_DIR})
endif()

message(STATUS "-----------------------------------------------")
message(STATUS "Using OpenSSL at directory: ${OPENSSL_ROOT_DIR}")
message(STATUS "-----------------------------------------------")

find_package(OpenSSL REQUIRED)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/../../external/*/include")

add_library(Wepp-Server STATIC ${SOURCES})
target_include_directories(Wepp-Server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include ${OPENSSL_INCLUDE_DIR})
target_include_directories(Wepp-Server PUBLIC ${LIBRARY_INCLUDES})
target_link_libraries(Wepp-Server PRIVATE GNetworking ${OPENSSL_LIBRARIES})
target_link_libraries(Wepp-Server PUBLIC GParsing-HTTP GLog)
