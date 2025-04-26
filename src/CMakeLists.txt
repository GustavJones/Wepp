cmake_minimum_required(VERSION 3.15)
project(Sources)

file(GLOB Components "*/")

foreach(Component ${Components})
  if(IS_DIRECTORY ${Component})
    if(EXISTS "${Component}/CMakeLists.txt")
      message(STATUS "Adding source: ${Component}")
      add_subdirectory(${Component})
    endif()
  endif()
endforeach()
