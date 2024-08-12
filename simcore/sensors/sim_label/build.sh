#!/bin/bash

# Setting build parmameters
SIM_LABEL_ROOT="$(cd "$(dirname "$0")";pwd)"
SIM_LABEL_BUILD="$SIM_LABEL_ROOT/build"

# clean & mkdir
rm -rf "$SIM_LABEL_BUILD"
mkdir "$SIM_LABEL_BUILD"

# build
cd "$SIM_LABEL_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$SIM_LABEL_ROOT"
