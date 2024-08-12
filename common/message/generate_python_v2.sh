#!/bin/bash

MSG_DIR=$(dirname $0)
PATHDIR_BUILD="$MSG_DIR/build"

# clean & mkdir
rm -rf $PATHDIR_BUILD
mkdir -p $PATHDIR_BUILD

for f in $MSG_DIR/*.proto
do
  python3 -m grpc_tools.protoc -I./ --python_out=$PATHDIR_BUILD --grpc_python_out=$PATHDIR_BUILD $f
done
