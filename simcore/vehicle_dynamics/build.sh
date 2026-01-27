#!/bin/bash

# Setting build parmameters
VD_ROOT="$(cd "$(dirname "$0")";pwd)"
VD_BUILD="$VD_ROOT/build"

# Clean & mkdir
rm -rf "$VD_BUILD"
mkdir "$VD_BUILD"

# build
cd "$VD_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# deplod
cp "$VD_ROOT/param/txcar.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/txcar_EV.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/txcar_Hybrid.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/txcar_ICE.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/txcar_template.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/txcar_template_hybrid.json" "$VD_BUILD/lib"
cp "$VD_ROOT/param/CS_car225_60R18.tir" "$VD_BUILD/lib"
cp "$VD_ROOT/param/pac2002_235_60R16.tir" "$VD_BUILD/lib"
cp "$VD_ROOT/param/TASS_car205_60R15.tir" "$VD_BUILD/lib"

# Change the working directory back to the original directory where the script was run
cd "$VD_ROOT"
