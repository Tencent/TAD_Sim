#!/bin/bash
wget https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/half.h -O ./src/half.h
wget https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/common.h -O ./src/common.h
wget https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/buffers.h -O ./src/buffers.h
wget https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/logger.h -O ./src/logger.h
mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Release ..
make
