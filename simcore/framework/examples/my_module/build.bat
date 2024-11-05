@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "MY_MODULE_ROOT=%~dp0"
set "MY_MODULE_ROOT=%MY_MODULE_ROOT:~0,-1%"
set "MY_MODULE_BUILD=%MY_MODULE_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%MY_MODULE_BUILD%" rmdir /s /q "%MY_MODULE_BUILD%"
mkdir "%MY_MODULE_BUILD%"

@REM build
cd "%MY_MODULE_BUILD%"
cmake .. -G "Visual Studio 17 2022" -T "v143" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%MY_MODULE_ROOT%"
