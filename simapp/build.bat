@echo off

@REM Setting build parmameters
set "SIMAPP_ROOT=%~dp0"
set "SIMAPP_ROOT=%SIMAPP_ROOT:~0,-1%"

@REM Clean
IF EXIST "%SIMAPP_ROOT%\package-lock.json" del /f /q "%SIMAPP_ROOT%\package-lock.json"
IF EXIST "%SIMAPP_ROOT%\node_modules" rmdir /s /q "%SIMAPP_ROOT%\node_modules"
@REM Clean desktop
IF EXIST "%SIMAPP_ROOT%\desktop\package-lock.json" del /f /q "%SIMAPP_ROOT%\desktop\package-lock.json"
IF EXIST "%SIMAPP_ROOT%\desktop\node_modules" rmdir /s /q "%SIMAPP_ROOT%\desktop\node_modules"
IF EXIST "%SIMAPP_ROOT%\desktop\build" rmdir /s /q "%SIMAPP_ROOT%\desktop\build"
@REM Clean map-editor
IF EXIST "%SIMAPP_ROOT%\map-editor\package-lock.json" del /f /q "%SIMAPP_ROOT%\map-editor\package-lock.json"
IF EXIST "%SIMAPP_ROOT%\map-editor\node_modules" rmdir /s /q "%SIMAPP_ROOT%\map-editor\node_modules"
IF EXIST "%SIMAPP_ROOT%\map-editor\build" rmdir /s /q "%SIMAPP_ROOT%\map-editor\build"
@REM Clean scene-editor
IF EXIST "%SIMAPP_ROOT%\scene-editor\package-lock.json" del /f /q "%SIMAPP_ROOT%\scene-editor\package-lock.json"
IF EXIST "%SIMAPP_ROOT%\scene-editor\node_modules" rmdir /s /q "%SIMAPP_ROOT%\scene-editor\node_modules"
IF EXIST "%SIMAPP_ROOT%\scene-editor\build" rmdir /s /q "%SIMAPP_ROOT%\scene-editor\build"

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
