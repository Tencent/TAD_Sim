@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "MAP_SERVER_ROOT=%~dp0"
set "MAP_SERVER_ROOT=%MAP_SERVER_ROOT:~0,-1%"
set "MAP_SERVER_BUILD=%MAP_SERVER_ROOT%\build"
@REM Setting build parmameters OPENDRIVE_IO
set "OPENDRIVE_IO_ROOT=%MAP_SERVER_ROOT%\opendrive_io"
set "OPENDRIVE_IO_BUILD=%OPENDRIVE_IO_ROOT%\build"
@REM Setting build parmameters MAP_PARSER
set "MAP_PARSER_ROOT=%MAP_SERVER_ROOT%\hadmap_server\map_parser"
set "MAP_PARSER_BUILD=%MAP_PARSER_ROOT%\build"
@REM Setting build parmameters SERVER
set "SERVER_ROOT=%MAP_SERVER_ROOT%\service"

@REM Clean & mkdir
IF EXIST "%MAP_SERVER_BUILD%" rmdir /s /q "%MAP_SERVER_BUILD%"
mkdir "%MAP_SERVER_BUILD%\bin\Release"
@REM Clean & mkdir OPENDRIVE_IO
IF EXIST "%OPENDRIVE_IO_BUILD%" rmdir /s /q "%OPENDRIVE_IO_BUILD%"
mkdir "%OPENDRIVE_IO_BUILD%"
@REM Clean & mkdir MAPPARSER
IF EXIST "%MAP_PARSER_BUILD%" rmdir /s /q "%MAP_PARSER_BUILD%"
mkdir "%MAP_PARSER_BUILD%"
@REM Clean & mkdir service
IF EXIST "%SERVER_ROOT%\go.sum" del /f /q "%SERVER_ROOT%\go.sum"
IF EXIST "%SERVER_ROOT%\service.exe" del /f /q "%SERVER_ROOT%\service.exe"

@REM build
@REM build opendriveio
cd "%OPENDRIVE_IO_BUILD%"
echo "opendrive_io build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build . --config Release
echo "opendrive_io build successfully."
echo.

@REM build map_parser
cd "%MAP_PARSER_BUILD%"
echo "map_parser build start..."
cmake .. -DCMAKE_BUILD_TYPE=Release  -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake -A x64
cmake --build . --config Release
echo "map_parser build successfully."
echo.

@REM build server (txSimService)
cd "%SERVER_ROOT%"
echo "service build start..."
go env -w GOPROXY=https://goproxy.io,direct
go mod tidy
go build
echo "service build successfully."
echo.

@REM deploy
xcopy "%OPENDRIVE_IO_BUILD%\bin\Release\*" "%MAP_SERVER_BUILD%\bin\Release\" /y /i /e
xcopy "%OPENDRIVE_IO_BUILD%\lib\Release\*" "%MAP_SERVER_BUILD%\bin\Release\" /y /i /e
xcopy "%MAP_PARSER_BUILD%\bin\Release\*" "%MAP_SERVER_BUILD%\bin\Release\" /y /i /e
xcopy "%MAP_PARSER_BUILD%\lib\Release\*" "%MAP_SERVER_BUILD%\bin\Release\" /y /i /e
copy "%SERVER_ROOT%\service.exe" "%MAP_SERVER_BUILD%\bin\Release\txSimService.exe" /y

@REM Change the working directory back to the original directory where the script was run
cd "%MAP_SERVER_ROOT%"
