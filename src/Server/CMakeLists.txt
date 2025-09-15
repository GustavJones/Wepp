cmake_minimum_required(VERSION 3.15)
project(Wepp-Server CXX)

option(OPENSSL_USE_STATIC_LIBS "Build Application with Static OpenSSL Libraries" TRUE)

if((NOT DEFINED OPENSSL_ROOT_DIR) OR (OPENSSL_ROOT_DIR STREQUAL ""))
  message(STATUS "No SSL Root directory defined. Reading from environment variables.")
  set(OPENSSL_ROOT_DIR  $ENV{OPENSSL_ROOT_DIR})
endif()

if(DEFINED OPENSSL_ROOT_DIR)
  message(STATUS "-----------------------------------------------")
  message(STATUS "Using OpenSSL at directory: ${OPENSSL_ROOT_DIR}")
  message(STATUS "-----------------------------------------------")
endif()

find_package(OpenSSL REQUIRED)

file(GLOB SOURCES "*.cpp")
file(GLOB LIBRARY_INCLUDES "${CMAKE_SOURCE_DIR}/external/*/include")
add_library(Wepp-Server ${SOURCES})
target_include_directories(Wepp-Server PUBLIC ${OPENSSL_INCLUDE_DIR})
target_include_directories(Wepp-Server PRIVATE ${CMAKE_SOURCE_DIR}/include ${LIBRARY_INCLUDES})

if(WIN32)
  target_link_libraries(Wepp-Server PRIVATE crypt32)
endif()

target_link_libraries(Wepp-Server PRIVATE  Wepp-FileHandling GLog GNetworking GParsing-HTTP ${OPENSSL_LIBRARIES})
