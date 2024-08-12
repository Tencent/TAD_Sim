#!/bin/bash

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
