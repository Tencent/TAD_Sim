#!/bin/bash

set -e

# Get & set ENABLE_ENCRYPTION_TIMESTAMP
if [ -n "$1" ]; then
    ENABLE_ENCRYPTION_TIMESTAMP="$1"
else
    ENABLE_ENCRYPTION_TIMESTAMP=0
fi

# Setting build parmameters
FRAMEWORK_ROOT="$(cd "$(dirname "$0")";pwd)"
FRAMEWORK_BUILD="$FRAMEWORK_ROOT/build"
# Setting build parmameters CLI
FRAMEWORK_CLI="$FRAMEWORK_ROOT/cli"
MESSAGE_ROOT="$FRAMEWORK_ROOT/../../common/message"
# Setting build parmameters txSim
FRAMEWORK_TXSIM="$FRAMEWORK_BUILD/txSim"
FRAMEWORK_TXSIM_INC="$FRAMEWORK_TXSIM/inc"
FRAMEWORK_TXSIM_LIB="$FRAMEWORK_TXSIM/lib"
FRAMEWORK_TXSIM_MSG="$FRAMEWORK_TXSIM/msgs"
FRAMEWORK_TXSIM_DOC="$FRAMEWORK_TXSIM/doc"
FRAMEWORK_TXSIM_EXAMPLE="$FRAMEWORK_TXSIM/example"
SDK_NAME="txSimSDK.tar.gz"

# Clean & mkdir
rm -rf "$FRAMEWORK_BUILD"
mkdir -p "$FRAMEWORK_BUILD"
rm -rf "$FRAMEWORK_ROOT/src/node_addon/build"
mkdir -p "$FRAMEWORK_TXSIM"
mkdir -p "$FRAMEWORK_TXSIM_INC"
mkdir -p "$FRAMEWORK_TXSIM_LIB"
mkdir -p "$FRAMEWORK_TXSIM_MSG"
mkdir -p "$FRAMEWORK_TXSIM_MSG/deps"
mkdir -p "$FRAMEWORK_TXSIM_DOC"
mkdir -p "$FRAMEWORK_TXSIM_EXAMPLE"
rm -rf "$FRAMEWORK_ROOT/docs/api"
rm -rf "$FRAMEWORK_ROOT/docs/sphinx"

# build framework
cd "$FRAMEWORK_BUILD"
echo "framework build start..."
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DTXSIM_WITH_DOC=ON -DTXSIM_ENCRYPTION_TIMESTAMP_INT=$ENABLE_ENCRYPTION_TIMESTAMP ..
ninja -j8
echo -e "framework build successfully.\n"

# build framework cli
cd "$FRAMEWORK_CLI"
echo "framework_cli build start..."
go env -w GOPROXY=https://goproxy.io,direct
go mod tidy -compat=1.17
go build
echo -e "framework_cli build successfully.\n"

# deploy txSim
cd "$FRAMEWORK_ROOT"
echo "txSim build start..."
cp "$FRAMEWORK_ROOT/src/txsim_"* "$FRAMEWORK_TXSIM_INC/"
cp "$FRAMEWORK_ROOT/src/visibility.h" "$FRAMEWORK_TXSIM_INC/"
cp -r "$FRAMEWORK_ROOT/src/api/python/"* "$FRAMEWORK_TXSIM_INC/"
cp "$FRAMEWORK_BUILD/lib/libtxsim-module-service.a" "$FRAMEWORK_TXSIM_LIB/"
cp -r "$FRAMEWORK_ROOT/../cmake/helper_funcs.cmake" "$FRAMEWORK_TXSIM_MSG/deps/"
cp "$MESSAGE_ROOT/"*.proto "$FRAMEWORK_TXSIM_MSG/"
cp "$MESSAGE_ROOT/generate_cpp.sh" "$FRAMEWORK_TXSIM_MSG/"
rm "$FRAMEWORK_TXSIM_MSG/moduleService.proto"
rm "$FRAMEWORK_TXSIM_MSG/sim_cloud_service.proto"
rm "$FRAMEWORK_TXSIM_MSG/sim_cloud_city_service.proto"
cp -r "$FRAMEWORK_ROOT/examples/"* "$FRAMEWORK_TXSIM_EXAMPLE"
cp -r "$FRAMEWORK_ROOT/docs/sphinx/"* "$FRAMEWORK_TXSIM_DOC/"
tar zcvf "$FRAMEWORK_BUILD/$SDK_NAME" -C "$FRAMEWORK_TXSIM" .
echo -e "txSim build successfully.\n"

# Change the working directory back to the original directory where the script was run
cd "$FRAMEWORK_ROOT"
