cmake_minimum_required(VERSION 2.8)

PROJECT(ponyC)

set (CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Weverything -std=c++11 -stdlib=libc++")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum -Wno-padded -Wno-c++98-compat -Wno-exit-time-destructors -Wno-global-constructors -Wno-sign-conversion")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/../bin)

include_directories(${ponyC_SOURCE_DIR}/include)
add_subdirectory(src)
