cmake_minimum_required(VERSION 2.8.9)

project (dien)

include_directories(include)

set(CMAKE_CXX_FLAGS "-O3 -lpthread -std=c++11")

file(GLOB SOURCES "src/*.cpp")
set (LINK_LIBS glog)

add_library(dien SHARED ${SOURCES})

target_link_libraries(dien ${LINK_LIBS})
