cmake_minimum_required(VERSION 3.0)

project(MYJSON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -Wall -std=c++11")

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(main main.cpp myjson.cpp)
