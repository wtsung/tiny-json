cmake_minimum_required(VERSION 3.19)
project(tiny-json)

set(GOOGLETEST_VERSION 1.9.0)
set(CMAKE_CXX_STANDARD 14)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest/include
)

add_subdirectory( ./googletest)

link_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/googletest
)


add_executable(tiny-json tiny_json_test.cc tiny_json.h tiny_json.cc googletest parser.cc parser.h tiny_json_bench.cc)
target_link_libraries(TinyJson-local gtest)
