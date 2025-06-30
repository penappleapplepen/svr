# CMakeLists.txt for tests
cmake_minimum_required(VERSION 3.10)

project(svr_tests)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always enable ThreadSanitizer
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=thread -fno-omit-frame-pointer")

# Add the main source directory for includes
include_directories(${CMAKE_SOURCE_DIR}/src)

# Add GoogleTest (assumes it's already downloaded by main CMake)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
# enable_testing()

# Gather all test source files
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Add executable for tests
add_executable(svr_tests ${TEST_SOURCES})

# Link with GoogleTest and pthread (if needed)
target_link_libraries(svr_tests PRIVATE gtest_main svr_lib)
target_include_directories(svr_tests PRIVATE ${gtest_SOURCE_DIR}/include)

# Add a test to CTest
# (Do not call enable_testing() here to avoid CTest/Testing directory)
# add_test(NAME svr_tests COMMAND svr_tests)  # Commented out to avoid CTest/Testing
