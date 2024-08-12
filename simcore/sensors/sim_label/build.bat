@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "SIM_LABEL_ROOT=%~dp0"
set "SIM_LABEL_ROOT=%SIM_LABEL_ROOT:~0,-1%"
set "SIM_LABEL_BUILD=%SIM_LABEL_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%SIM_LABEL_BUILD%" rmdir /s /q "%SIM_LABEL_BUILD%"
mkdir "%SIM_LABEL_BUILD%"

@REM build
cd "%SIM_LABEL_BUILD%"
cmake .. -A x64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%SIM_LABEL_ROOT%"
