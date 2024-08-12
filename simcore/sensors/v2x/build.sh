#!/bin/bash

# Setting build parmameters
V2X_ROOT="$(cd "$(dirname "$0")";pwd)"
V2X_BUILD="$V2X_ROOT/build"

# clean & mkdir
rm -rf "$V2X_BUILD"
mkdir "$V2X_BUILD"

# build
cd "$V2X_BUILD"
wget https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -O ./ini.h
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$V2X_ROOT"
