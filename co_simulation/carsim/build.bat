@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "CARSIM_ROOT=%~dp0"
set "CARSIM_ROOT=%CARSIM_ROOT:~0,-1%"
set "CARSIM_BUILD=%CARSIM_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%CARSIM_BUILD%" rmdir /s /q "%CARSIM_BUILD%"
mkdir "%CARSIM_BUILD%"

@REM build
cd "%CARSIM_BUILD%"
cmake .. -G "Visual Studio 17 2022" -T "v143" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%CARSIM_ROOT%"
