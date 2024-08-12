@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "IMU_GPS_ROOT=%~dp0"
set "IMU_GPS_ROOT=%IMU_GPS_ROOT:~0,-1%"
set "IMU_GPS_BUILD=%IMU_GPS_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%IMU_GPS_BUILD%" rmdir /s /q "%IMU_GPS_BUILD%"
mkdir "%IMU_GPS_BUILD%"

@REM build
cd "%IMU_GPS_BUILD%"
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%IMU_GPS_ROOT%"
