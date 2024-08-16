#!/bin/bash

# Setting build parmameters
DISPLAY_ROOT="$(cd "$(dirname "$0")";pwd)"
DISPLAY_BUILD="$DISPLAY_ROOT/build"
ENGINE_ROOT="/home/ue4/UnrealEngine/Engine"

# prerequisites
echo =================LINUX PREPARE=================
sh ./download_deps.sh
mkdir $DISPLAY_ROOT/Plugins/BoostLib/deps
ln -s /usr/include/boost $DISPLAY_ROOT/Plugins/BoostLib/deps/boost

# build
echo =================LINUX PACKAGING CONFIGURATION=================
export LD_LIBRARY_PATH="$DISPLAY_ROOT/Binaries/Linux/ubuntu18_20:$LD_LIBRARY_PATH"
cd "$ENGINE_ROOT/Build/BatchFiles/"
./RunUAT.sh BuildCookRun -utf8output \
                         -platform=Linux \
                         -clientconfig=Development \
                         -serverconfig=Development \
                         -project="$DISPLAY_ROOT/Display.uproject" \
                         -noP4 \
                         -nodebuginfo \
                         -cook \
                         -build \
                         -stage \
                         -prereqs \
                         -pak \
                         -archive \
                         -archivedirectory="$DISPLAY_ROOT/Saved"

# deploy
cd "$DISPLAY_ROOT"
cp "$DISPLAY_ROOT/Binaries/Linux/Display.sym" "$DISPLAY_ROOT/Saved/StagedBuilds/LinuxNoEditor/Display/Binaries/Linux/"

cd "$DISPLAY_ROOT/Saved/StagedBuilds"
mv LinuxNoEditor Display
cp -r ../../NeuralStyle ./Display/Display/
cp -r ../../XMLFiles ./Display/Display/
cp -f ../../Display.sh ./Display/Display.sh
cp -f ../../Display-cloud.sh ./Display/Display-cloud.sh
chmod +x ./Display/Display.sh
chmod +x ./Display/Display-cloud.sh
cd ../..

# clean & mkdir
rm -rf "$DISPLAY_ROOT/Build"
rm -rf "$DISPLAY_BUILD"
mkdir "$DISPLAY_BUILD"
mkdir "$DISPLAY_BUILD/bin"

# package
mv ./Saved/StagedBuilds/Display $DISPLAY_BUILD/bin/Display
rm $DISPLAY_BUILD/bin/Display/Display/Binaries/Linux/ubuntu18_20/libcuda.so.1

cd "$DISPLAY_BUILD/bin"
tar -czf display.tar.gz ./Display

# Change the working directory back to the original directory where the script was run
cd "$DISPLAY_ROOT"
