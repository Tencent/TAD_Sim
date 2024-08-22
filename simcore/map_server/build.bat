
@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)
@echo off
set "folder=build"
if exist "%build_dir%" (
    echo build dir exists. Calling clean.bat...
    call clean.bat
) else (
    echo build dir does not exist.
)

@REM opendriveio
cd opendrive_io
md build
cd build
cmake  .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build . --config Release


@REM map_parser
cd ../../hadmap_server/map_parser
md build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build . --config Release


@REM txSimService
cd ../../../service
go env -w GOPROXY=https://goproxy.io,direct
go mod tidy
go build

if not exist "../build/bin/Release" (
    mkdir "../build/bin/Release"
)

copy /Y "./service.exe" "../build/bin/Release/txSimService.exe"
