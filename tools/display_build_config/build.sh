#!/bin/bash

# Setting build parmameters
DISPLAY_BUILD_CONFIG_ROOT="$(cd "$(dirname "$0")";pwd)"
DISPLAY_BUILD_CONFIG_BUILD="$DISPLAY_BUILD_CONFIG_ROOT/build"

# mkdir mini
# wget https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -O ./mini/ini.h

# Clean & mkdir
rm -rf "$DISPLAY_BUILD_CONFIG_BUILD"
mkdir "$DISPLAY_BUILD_CONFIG_BUILD"

# build
cd "$DISPLAY_BUILD_CONFIG_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$DISPLAY_BUILD_CONFIG_BUILD"
