@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "RADAR_ROOT=%~dp0"
set "RADAR_ROOT=%RADAR_ROOT:~0,-1%"
set "RADAR_BUILD=%RADAR_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%RADAR_BUILD%" rmdir /s /q "%RADAR_BUILD%"
mkdir "%RADAR_BUILD%"

@REM build
cd "%RADAR_BUILD%"
curl https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -o ini.h
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%RADAR_ROOT%"
