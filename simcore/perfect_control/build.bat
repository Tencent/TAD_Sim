@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "PERFECT_CONTROL_ROOT=%~dp0"
set "PERFECT_CONTROL_ROOT=%PERFECT_CONTROL_ROOT:~0,-1%"
set "PERFECT_CONTROL_BUILD=%PERFECT_CONTROL_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%PERFECT_CONTROL_BUILD%" rmdir /s /q "%PERFECT_CONTROL_BUILD%"
mkdir "%PERFECT_CONTROL_BUILD%"

@REM build
cd "%PERFECT_CONTROL_BUILD%"
cmake .. -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release -j 8 --verbose

@REM Change the working directory back to the original directory where the script was run
cd "%PERFECT_CONTROL_ROOT%"
