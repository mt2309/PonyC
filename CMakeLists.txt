cmake_minimum_required(VERSION 2.8)

PROJECT(ponyC)

set (CMAKE_CXX_COMPILER clang++)
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/Cellar/boost/1.51.0/")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "/usr/local/Cellar/boost/1.51.0/lib")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -std=c++11")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

include_directories($(ponyC_SOURCE_DIR)/include)
add_subdirectory(src)