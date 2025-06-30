#!/bin/bash
set -e

touch tests/CMakeLists.txt
cmake -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
cmake --build build
./build/tests/svr_tests
