# CMakeLists.txt for benchmarks
cmake_minimum_required(VERSION 3.10)

project(svr_benchmarks)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${CMAKE_SOURCE_DIR}/src)


# Build each benchmark as a separate executable to avoid multiple main() errors
add_executable(bench_spin_lock_variants bench_spin_lock_variants.cpp)
target_link_libraries(bench_spin_lock_variants PRIVATE pthread svr)

add_executable(bench_spsc bench_spsc.cpp)
target_link_libraries(bench_spsc PRIVATE pthread svr)
