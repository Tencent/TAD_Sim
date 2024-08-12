#!/bin/bash

set -e

cd "${0%/*}"

if [ ! -d hdserver_deploy ];then
    mkdir hdserver_deploy
    if [ ! -d hdserver_deploy/simdeps/ ];then
        mkdir hdserver_deploy/simdeps
    fi
fi
cp ../../../hdserver/cloud/libscene_wrapper.so ./hdserver_deploy/
if [ -f ../src/service/service ];then
    cp ../src/service/service ./hdserver_deploy/txSimService
fi
if [ -f ../../../build/hdserver/cloud/xosc_convert_tool ];then
    cp ../../../build/hdserver/cloud/xosc_convert_tool ./hdserver_deploy/xosc_convert_tool
fi
for b in ./hdserver_deploy/*; do if [ -f $b ]; then ldd $b; fi; done | grep "=> /" | awk '{print $3}' | sort | uniq | xargs -I {} cp -v {} ./hdserver_deploy/simdeps/
mv ./hdserver_deploy/libscene_wrapper.so ./hdserver_deploy/simdeps/
