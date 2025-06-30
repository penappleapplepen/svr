#!/bin/bash
set -e

if [ ! -d build ]; then
  cmake -B build -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
fi

cmake --build build
./build/tests/svr_tests
