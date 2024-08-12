#!/bin/bash

# Setting build parmameters
GRADING_ROOT="$(cd "$(dirname "$0")";pwd)"
GRADING_BUILD="$GRADING_ROOT/build"

# clean & mkdir
rm -rf "$GRADING_BUILD"
mkdir "$GRADING_BUILD"

# build
cd "$GRADING_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Execute the SDK compilation script and copy the output to the 'build' directory
cd "$GRADING_ROOT/external_eval"
sh ./package_sdk_linux.sh
cp ./txSimGradingSDK_linux.tar.gz ../build/bin/

# Change the working directory back to the original directory where the script was run
cd "$GRADING_ROOT"
