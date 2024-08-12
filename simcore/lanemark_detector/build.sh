#!/bin/bash

# Setting build parmameters
LANEMARK_DETECTOR_ROOT="$(cd "$(dirname "$0")";pwd)"
LANEMARK_DETECTOR_BUILD="$LANEMARK_DETECTOR_ROOT/build"

# clean & mkdir
rm -rf "$LANEMARK_DETECTOR_BUILD"
mkdir "$LANEMARK_DETECTOR_BUILD"

# build
cd "$LANEMARK_DETECTOR_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$LANEMARK_DETECTOR_ROOT"
