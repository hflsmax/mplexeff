#!/bin/bash

mkdir build
cd build
cmake .. -B . -DCMAKE_BUILD_TYPE=Release
make -j
./test_mpe_main
./lextest_mpe_main
./lexopttest_mpe_main