#!/bin/bash

# Setting build parmameters
CARSIM_ROOT="$(cd "$(dirname "$0")";pwd)"
CARSIM_BUILD="$CARSIM_ROOT/build"

# Clean & mkdir
rm -rf "$CARSIM_BUILD"
mkdir "$CARSIM_BUILD"

# build
cd "$CARSIM_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$CARSIM_ROOT -D_OPEN_LOOP_=OFF ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$CARSIM_ROOT"
