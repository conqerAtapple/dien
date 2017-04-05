cmake_minimum_required(VERSION 2.8.9)

project (dien_tests)

set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(DIEN_DIR "../")

include_directories(include ${DIEN_DIR}/include ${GTEST_DIR}/include)

set(CMAKE_CXX_FLAGS "-O3 -g -pthread -std=c++0x")

file(GLOB SOURCES "src/*.cpp")

set (LINK_LIBS glog gtest)

add_executable(dien_tests ${SOURCES})

target_link_libraries(dien_tests ${LINK_LIBS})
