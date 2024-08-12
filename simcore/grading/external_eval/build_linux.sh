#!/bin/bash

# Save the directory path of the current script into 'root_dir'
root_dir=$(pwd)
# Return to the root_dir directory
cd $root_dir

# Check if the 'build' directory exists within the script's directory
if [ -d "./build" ]; then
  # If it does, remove the 'build' directory and all of its contents
  rm -rf ./build
fi

# Create a new 'build' directory
mkdir -p ./build

# Change the working directory to the newly created 'build' directory
cd ./build

# Use the CMake build tool to generate a Makefile and set the build type to Release
# Then, use the make command to compile the project in parallel
# Here, 8 parallel tasks are used to speed up the compilation process
cmake -DCMAKE_BUILD_TYPE=Release .. && make -j8


# Find all the dynamic library files that libtxSimGradingExternal.so depends on
# and copy them to the current directory
# cd ./bin
# ldd libtxSimGradingExternal.so | awk '{print $3}' | xargs -i cp -L {} . 2>/dev/null

# Change the working directory back to the original directory where the script was run
cd $root_dir
