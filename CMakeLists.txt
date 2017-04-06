cmake_minimum_required(VERSION 3.7)
project(RTree)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp RTree.h)
add_executable(RTree ${SOURCE_FILES})