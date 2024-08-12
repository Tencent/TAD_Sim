#!/bin/bash

# Setting build parmameters
OSI_ROOT="$(cd "$(dirname "$0")";pwd)"
OSI_BUILD="$OSI_ROOT/build"
PATHDIR_DEPENDENCE_MSG="$OSI_ROOT/../../common/message/build"

# clean & mkdir
rm -rf "$OSI_BUILD"
mkdir "$OSI_BUILD"

# Check if the message dependecies exists
if [ ! -d "$PATHDIR_DEPENDENCE_MSG" ]; then
    echo "The directory $PATHDIR_DEPENDENCE_MSG does not exist."
    parent_dir=$(dirname "$PATHDIR_DEPENDENCE_MSG")
    echo "Running generate_cpp.sh in the parent directory: $parent_dir"
    chmod +777 "$parent_dir/generate_cpp.sh" && sh -x "$parent_dir/generate_cpp.sh"
else
    echo "The directory $PATHDIR_DEPENDENCE_MSG exists."
fi

# build
cd "$OSI_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$OSI_ROOT"
