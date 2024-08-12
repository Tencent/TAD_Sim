#!/bin/bash

# Setting build parmameters
DISPLAY_ROOT="$(cd "$(dirname "$0")";pwd)"
DISPLAY_BUILD="$DISPLAY_ROOT/build"
ENGINE_ROOT="/home/ue4/UnrealEngine/Engine"

# clean & mkdir
rm -rf "$DISPLAY_BUILD"
mkdir "$DISPLAY_BUILD"
mkdir "$DISPLAY_BUILD/bin"

# prerequisites
echo =================LINUX PREPARE=================
sh ./download_deps.sh

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
mv ./Saved/StagedBuilds/Display ./build/bin/Display

cd "$DISPLAY_BUILD/bin"
tar -czf display.tar.gz ./Display

# Change the working directory back to the original directory where the script was run
cd "$DISPLAY_ROOT"
