@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "SENSOR_TRUTH_ROOT=%~dp0"
set "SENSOR_TRUTH_ROOT=%SENSOR_TRUTH_ROOT:~0,-1%"
set "SENSOR_TRUTH_BUILD=%SENSOR_TRUTH_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%SENSOR_TRUTH_BUILD%" rmdir /s /q "%SENSOR_TRUTH_BUILD%"
mkdir "%SENSOR_TRUTH_BUILD%"

@REM build
cd "%SENSOR_TRUTH_BUILD%"
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%SENSOR_TRUTH_ROOT%"
