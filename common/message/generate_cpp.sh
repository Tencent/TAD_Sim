#!/bin/bash

MESSAGE_ROOT=$(dirname $0)
MESSAGE_BUILD="$MESSAGE_ROOT/build"

# clean & mkdir
rm -rf $MESSAGE_BUILD
mkdir -p $MESSAGE_BUILD

for f in $MESSAGE_ROOT/*.proto
do
  protoc -I=$MESSAGE_ROOT --cpp_out=$MESSAGE_BUILD $f
done

GRPC_CPP_PLUGIN=`which grpc_cpp_plugin`

protoc -I=$MESSAGE_ROOT --grpc_out=$MESSAGE_BUILD --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN $MESSAGE_ROOT/coordinatorService.proto
protoc -I=$MESSAGE_ROOT --grpc_out=$MESSAGE_BUILD --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN $MESSAGE_ROOT/simDriverService.proto
protoc -I=$MESSAGE_ROOT --grpc_out=$MESSAGE_BUILD --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN $MESSAGE_ROOT/simAgent.proto
protoc -I=$MESSAGE_ROOT --grpc_out=$MESSAGE_BUILD --plugin=protoc-gen-grpc=$GRPC_CPP_PLUGIN $MESSAGE_ROOT/xil_interface.proto
