@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "TRAFFIC_ROOT=%~dp0"
set "TRAFFIC_ROOT=%TRAFFIC_ROOT:~0,-1%"
set "TRAFFIC_BUILD=%TRAFFIC_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%TRAFFIC_BUILD%" rmdir /s /q "%TRAFFIC_BUILD%"
mkdir "%TRAFFIC_BUILD%"

@REM build
cd "%TRAFFIC_BUILD%"
echo "Traffic build start..."
cmake .. -DOnlyTraffic=ON -DOnlyCloud=OFF -DCloudStandAlone=OFF -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release
echo "Traffic build successfully."

@REM Change the working directory back to the original directory where the script was run
cd "%TRAFFIC_ROOT%"
