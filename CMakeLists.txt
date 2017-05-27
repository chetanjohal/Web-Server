cmake_minimum_required(VERSION 3.6)
project(Web_Server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES mtws.c mtws.h)
add_executable(Web_Server ${SOURCE_FILES})