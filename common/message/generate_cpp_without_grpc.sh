#!/bin/bash

MSG_DIR=$(dirname $0)

if  [ ! $# -eq 1 ];then
    echo "$0 output_dir"
    exit 1
fi

OUT_DIR=$1

for f in $MSG_DIR/*.proto
do
  protoc -I=$MSG_DIR --cpp_out=$OUT_DIR $f
done
