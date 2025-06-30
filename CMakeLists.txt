cmake_minimum_required(VERSION 3.10)
project(svr)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB_RECURSE SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add include directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/helpers
    ${PROJECT_SOURCE_DIR}/src/memory
    ${PROJECT_SOURCE_DIR}/src/multithreading
    ${PROJECT_SOURCE_DIR}/src/templates
)

add_executable(svr ${SOURCES})
