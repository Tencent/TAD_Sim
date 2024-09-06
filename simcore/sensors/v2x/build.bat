@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "V2X_ROOT=%~dp0"
set "V2X_ROOT=%V2X_ROOT:~0,-1%"
set "V2X_BUILD=%V2X_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%V2X_BUILD%" rmdir /s /q "%V2X_BUILD%"
mkdir "%V2X_BUILD%"

@REM build
cd "%V2X_BUILD%"
@REM curl https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -o ./ini.h
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%V2X_ROOT%"
