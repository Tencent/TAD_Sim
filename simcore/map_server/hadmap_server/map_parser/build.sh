#!/bin/bash

mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja
