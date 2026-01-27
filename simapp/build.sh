#!/bin/bash
set -e

# ====== Setting build parmameters ======
SIMAPP_ROOT="$(cd "$(dirname "$0")";pwd)"

# Clean
# rm -f "$SIMAPP_ROOT/package-lock.json"
# rm -rf "$SIMAPP_ROOT/node_modules"
# Clean desktop
# rm -f "$SIMAPP_ROOT/desktop/package-lock.json"
# rm -rf "$SIMAPP_ROOT/desktop/node_modules"
# rm -rf "$SIMAPP_ROOT/desktop/build"
# Clean map-editor
# rm -f "$SIMAPP_ROOT/map-editor/package-lock.json"
# rm -rf "$SIMAPP_ROOT/map-editor/node_modules"
# rm -rf "$SIMAPP_ROOT/map-editor/build"
# Clean scene-editor
# rm -f "$SIMAPP_ROOT/scene-editor/package-lock.json"
# rm -rf "$SIMAPP_ROOT/scene-editor/node_modules"
# rm -rf "$SIMAPP_ROOT/scene-editor/build"

# 安装依赖 (Only if missing or package.json changed)
if [ ! -d "$SIMAPP_ROOT/node_modules" ]; then
    echo "Info: simapp node_modules missing. Installing..."
    npm config set registry https://registry.npmmirror.com
    npm install
else
    echo "Info: simapp node_modules exists. Skipping install."
fi

# 使用旧版本的依赖
cd desktop
if [ ! -d "node_modules" ]; then
    echo "Info: desktop node_modules missing. Installing..."
    npm i
else
    echo "Info: desktop node_modules exists. Skipping install."
fi
cd ..

# Ensure prepare script is run (required for specific versions)
# But only if needed? Better to run it once if we just installed.
node ./scripts/prepare.js

# 打包 UI (Incremental Build)
echo "=== Building UI Components"

# scene-editor
if [ -d "$SIMAPP_ROOT/scene-editor/build" ]; then
    echo "Info: scene-editor/build exists. Skipping build."
else
    npm run build:scene
fi

# map-editor
if [ -d "$SIMAPP_ROOT/map-editor/build" ]; then
    echo "Info: map-editor/build exists. Skipping build."
else
    npm run build:map
fi

# desktop
if [ -d "$SIMAPP_ROOT/desktop/build" ]; then
    echo "Info: desktop/build exists. Skipping build."
else
    npm run build:desktop
fi
