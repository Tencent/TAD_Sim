#!/bin/bash

# Setting build parmameters
IMU_GPS_ROOT="$(cd "$(dirname "$0")";pwd)"
IMU_GPS_BUILD="$IMU_GPS_ROOT/build"

# clean & mkdir
rm -rf "$IMU_GPS_BUILD"
mkdir "$IMU_GPS_BUILD"

# build
cd "$IMU_GPS_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$IMU_GPS_ROOT"
