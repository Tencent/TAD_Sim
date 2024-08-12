#!/bin/bash

MSG_DIR=$(dirname $0)
PATHDIR_BUILD="$MSG_DIR/build"

# clean & mkdir
rm -rf $PATHDIR_BUILD
mkdir -p $PATHDIR_BUILD

for f in $MSG_DIR/*.proto
do
  protoc -I=$MSG_DIR --python_out=$PATHDIR_BUILD $f
done
