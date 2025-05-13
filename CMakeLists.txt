cmake_minimum_required(VERSION 3.15)
project(Wepp)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(BUILD_SHARED_LIBS OFF)
set(BUILD_STATIC_LIBS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB SUBMODULES "external/*")

foreach(SUBMODULE ${SUBMODULES})
  if(IS_DIRECTORY ${SUBMODULE})
    if(EXISTS "${SUBMODULE}/CMakeLists.txt")
      message(STATUS "Adding submodule: ${SUBMODULE}")
      add_subdirectory(${SUBMODULE})
    endif()
  endif()
endforeach()

add_subdirectory(src)
