#!/bin/bash

# ====== Setting build parmameters ======
SIMAPP_ROOT="$(cd "$(dirname "$0")";pwd)"

# Clean
rm -f "$SIMAPP_ROOT/package-lock.json"
rm -rf "$SIMAPP_ROOT/node_modules"
# Clean desktop
rm -f "$SIMAPP_ROOT/desktop/package-lock.json"
rm -rf "$SIMAPP_ROOT/desktop/node_modules"
rm -rf "$SIMAPP_ROOT/desktop/build"
# Clean map-editor
rm -f "$SIMAPP_ROOT/map-editor/package-lock.json"
rm -rf "$SIMAPP_ROOT/map-editor/node_modules"
rm -rf "$SIMAPP_ROOT/map-editor/build"
# Clean scene-editor
rm -f "$SIMAPP_ROOT/scene-editor/package-lock.json"
rm -rf "$SIMAPP_ROOT/scene-editor/node_modules"
rm -rf "$SIMAPP_ROOT/scene-editor/build"

# 安装依赖
npm install

# 使用旧版本的依赖
cd desktop
npm i
cd ..
node ./scripts/prepare.mjs

# 打包
npm run build:scene
npm run build:map
npm run build:desktop
