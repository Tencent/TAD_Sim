@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "ENVPB_ROOT=%~dp0"
set "ENVPB_ROOT=%ENVPB_ROOT:~0,-1%"
set "ENVPB_BUILD=%ENVPB_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%ENVPB_BUILD%" rmdir /s /q "%ENVPB_BUILD%"
mkdir "%ENVPB_BUILD%"

@REM build
cd "%ENVPB_BUILD%"
cmake .. -G "Visual Studio 17 2022" -T "v143" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%ENVPB_ROOT%"
