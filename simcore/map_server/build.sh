#!/bin/bash

# 1 OPENDRIVE IO
echo "start build Opendrive IO"

cd opendrive_io
# check build 
folder_path="./build"

if [ -d "$folder_path" ]; then
    rm -rf build
    echo "Build exists"
else
    echo "Build does not exist"
fi
mkdir -p build
cd build
cmake  -DCMAKE_BUILD_TYPE=Release ..
make -j

# 2 map_parser
cd ../../hadmap_server/map_parser
mkdir -p build
cd build
cmake  -DCMAKE_BUILD_TYPE=Release ..
make -j

# 3 SERVICE
cd ../../../service
go mod tidy
go build
cp ./service ../build/bin/txSimService
