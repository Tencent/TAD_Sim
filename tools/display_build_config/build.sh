#!/bin/bash
mkdir mini
wget https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -O ./mini/ini.h
mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8
