#!/bin/bash

# Setting build parmameters
ARBITRARY_ROOT="$(cd "$(dirname "$0")";pwd)"
ARBITRARY_BUILD="$ARBITRARY_ROOT/build"

# Clean & mkdir
rm -rf "$ARBITRARY_BUILD"
mkdir "$ARBITRARY_BUILD"

# build
cd "$ARBITRARY_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$ARBITRARY_ROOT"
