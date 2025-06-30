# CMakeLists.txt for benchmarks
cmake_minimum_required(VERSION 3.10)

project(svr_benchmarks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB BENCH_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(svr_benchmarks ${BENCH_SOURCES})
target_link_libraries(svr_benchmarks PRIVATE pthread)
