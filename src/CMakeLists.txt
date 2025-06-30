# CMakeLists.txt for src
cmake_minimum_required(VERSION 3.10)

project(svr_src)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(svr_lib INTERFACE)

target_include_directories(svr_lib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
