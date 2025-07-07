# CMakeLists.txt for src
cmake_minimum_required(VERSION 3.10)

project(svr_src)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Define the svr interface library with the correct name for linking
add_library(svr INTERFACE)
target_include_directories(svr INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
