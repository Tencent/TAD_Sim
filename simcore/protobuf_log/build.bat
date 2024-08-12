@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "PROTOBUF_LOG_ROOT=%~dp0"
set "PROTOBUF_LOG_ROOT=%PROTOBUF_LOG_ROOT:~0,-1%"
set "PROTOBUF_LOG_BUILD=%PROTOBUF_LOG_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%PROTOBUF_LOG_BUILD%" rmdir /s /q "%PROTOBUF_LOG_BUILD%"
mkdir "%PROTOBUF_LOG_BUILD%"

@REM build
cd "%PROTOBUF_LOG_BUILD%"
cmake .. -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release -j 8 --verbose

@REM Change the working directory back to the original directory where the script was run
cd "%PROTOBUF_LOG_ROOT%"
