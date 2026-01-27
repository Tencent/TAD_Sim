#!/bin/bash
# Initialize TAD_Sim Runtime Environment to prevent Error Code 2
TARGET_DIR="$HOME/.config/tadsim"
echo "Initializing $TARGET_DIR..."
mkdir -p "$TARGET_DIR/sys/scenario" "$TARGET_DIR/cache/debug_log" "$TARGET_DIR/data/scenario"
# Copy catalogs from local package to user config
SOURCE_DIR=$(dirname "$(readlink -f "$0")")
echo "Copying Catalogs..."
cp -rn "$SOURCE_DIR/Catalogs" "$TARGET_DIR/sys/scenario/" 2>/dev/null || true
# Copy other templates
echo "Copying Scenario Templates..."
cp -rn "$SOURCE_DIR/../scenario/"* "$TARGET_DIR/sys/scenario/" 2>/dev/null || true
# Copy models (Critical for Boost ::canonical fix)
echo "Copying 3D Models..."
mkdir -p "$TARGET_DIR/data/scenario/models"
cp -rn "$SOURCE_DIR/../build/electron/assets/models/"* "$TARGET_DIR/data/scenario/models/" 2>/dev/null || true
echo "Runtime environment initialized. You can now run tadsim."
