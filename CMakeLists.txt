# Root CMakeLists.txt for svr project
cmake_minimum_required(VERSION 3.10)

project(svr)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add src directory
add_subdirectory(src)

# Add tests directory
add_subdirectory(tests)

# Add benchmarks directory
add_subdirectory(benchmarks)
