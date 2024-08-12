curl https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/half.h -o ./src/half.h
curl https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/common.h -o ./src/common.h
curl https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/buffers.h -o ./src/buffers.h
curl https://raw.githubusercontent.com/NVIDIA/TensorRT/release/8.2/samples/common/logger.h -o ./src/logger.h
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
