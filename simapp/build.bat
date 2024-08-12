@echo off

@REM 安装依赖
call npm install

@REM 使用旧版本的依赖
cd desktop
call npm i
cd ..
call node ./scripts/prepare.mjs

@REM 打包
call npm run build:scene
call npm run build:map
call npm run build:desktop
