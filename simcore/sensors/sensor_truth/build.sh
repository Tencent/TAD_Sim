#!/bin/bash

# Setting build parmameters
SENSOR_TRUTH_ROOT="$(cd "$(dirname "$0")";pwd)"
SENSOR_TRUTH_BUILD="$SENSOR_TRUTH_ROOT/build"

# clean & mkdir
rm -rf "$SENSOR_TRUTH_BUILD"
mkdir "$SENSOR_TRUTH_BUILD"

# build
cd "$SENSOR_TRUTH_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$SENSOR_TRUTH_ROOT"
