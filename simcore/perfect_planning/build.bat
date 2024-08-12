@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "PERFECT_PLANNING_ROOT=%~dp0"
set "PERFECT_PLANNING_ROOT=%PERFECT_PLANNING_ROOT:~0,-1%"
set "PERFECT_PLANNING_BUILD=%PERFECT_PLANNING_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%PERFECT_PLANNING_BUILD%" rmdir /s /q "%PERFECT_PLANNING_BUILD%"
mkdir "%PERFECT_PLANNING_BUILD%"

@REM build
cd "%PERFECT_PLANNING_BUILD%"
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%PERFECT_PLANNING_ROOT%"
