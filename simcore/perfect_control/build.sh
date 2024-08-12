#!/bin/bash

# Setting build parmameters
PERFECT_CONTROL_ROOT="$(cd "$(dirname "$0")";pwd)"
PERFECT_CONTROL_BUILD="$PERFECT_CONTROL_ROOT/build"

# clean & mkdir
rm -rf "$PERFECT_CONTROL_BUILD"
mkdir "$PERFECT_CONTROL_BUILD"

# build
cd "$PERFECT_CONTROL_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$PERFECT_CONTROL_ROOT"
