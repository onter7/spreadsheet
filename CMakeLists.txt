cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(formulaAST)

set(CMAKE_CXX_STANDARD 17)
if(MSVC)
  set(
    CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} /JMC"
  )
else()
  set(
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-implicit-fallthrough"
  )
endif()

add_subdirectory(src)