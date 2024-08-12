#!/bin/bash

# Setting build parmameters
PROTOBUF_LOG_ROOT="$(cd "$(dirname "$0")";pwd)"
PROTOBUF_LOG_BUILD="$PROTOBUF_LOG_ROOT/build"

# clean & mkdir
rm -rf "$PROTOBUF_LOG_BUILD"
mkdir "$PROTOBUF_LOG_BUILD"

# build
cd "$PROTOBUF_LOG_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$PROTOBUF_LOG_ROOT"
