#!/bin/bash

set -e

cd "${0%/*}"

mkdir hdserver
mkdir hdserver/simdeps
cp ../map_parser/libscene_wrapper.so ./hdserver/
cp ../../service/service ./hdserver/txSimService
for b in ./hdserver/*; do if [ -f $b ]; then ldd $b; fi; done | grep "=> /" | awk '{print $3}' | sort | uniq | xargs -I {} cp -v {} ./hdserver/simdeps/
rm ./hdserver/simdeps/libc.so.*
mv ./hdserver/libscene_wrapper.so ./hdserver/simdeps/
