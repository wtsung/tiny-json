cmake_minimum_required(VERSION 3.19)
project(TinyJson-local)

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


add_executable(TinyJson-local test.cpp JsonNode.h JsonNode.cpp googletest)
target_link_libraries(TinyJson-local gtest)
