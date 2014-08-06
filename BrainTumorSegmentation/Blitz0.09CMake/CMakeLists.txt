cmake_minimum_required(VERSION 2.8)

project(blitz)
 
include_directories(./include /usr/local/)

set(srcfiles 
    ./src/globals.cpp
)


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_library(blitz STATIC ${srcfiles}) 

add_subdirectory(test)


