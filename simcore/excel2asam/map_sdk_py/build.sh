#!/bin/bash

# define
MAPSDKPY_ROOT="$(cd "$(dirname "$0")";pwd)"
MAPSDKPY_BUILD="$MAPSDKPY_ROOT/build"

# clean & mkdir
rm -rf "$MAPSDKPY_BUILD"
mkdir -p "$MAPSDKPY_BUILD"

# build hadmap py
cd "$MAPSDKPY_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$MAPSDKPY_ROOT"
