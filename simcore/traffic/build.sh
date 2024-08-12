#!/bin/bash

# Setting build parmameters
TRAFFIC_ROOT="$(cd "$(dirname "$0")";pwd)"
TRAFFIC_BUILD="$TRAFFIC_ROOT/build"

# clean & mkdir
rm -rf "$TRAFFIC_BUILD"
mkdir "$TRAFFIC_BUILD"

# build
cd "$TRAFFIC_BUILD"
chmod +777 ../../simcore ../../common
echo "Traffic build start..."
cmake -G Ninja -DOnlyTraffic=ON -DOnlyCloud=OFF -DCloudStandAlone=OFF -DCMAKE_BUILD_TYPE=Release ..
ninja -j8
echo "Traffic build successfully."

# Change the working directory back to the original directory where the script was run
cd "$TRAFFIC_ROOT"
