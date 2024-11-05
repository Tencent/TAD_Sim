#!/bin/bash

# Setting build parmameters
MY_MODULE_ROOT="$(cd "$(dirname "$0")";pwd)"
MY_MODULE_BUILD="$MY_MODULE_ROOT/build"

# Clean & mkdir
rm -rf "$MY_MODULE_BUILD"
mkdir "$MY_MODULE_BUILD"

# build
cd "$MY_MODULE_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$MY_MODULE_ROOT"
