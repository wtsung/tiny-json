cmake_minimum_required(VERSION 3.19)
project(myjson_oop)

set(CMAKE_CXX_STANDARD 14)

add_executable(myjson_oop cmake-build-debug/JSONValue.cpp cmake-build-debug/JSONValue.h cmake-build-debug/test.cpp cmake-build-debug/t.cpp)