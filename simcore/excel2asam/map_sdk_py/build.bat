@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "MAPSDKPY_ROOT=%~dp0"
set "MAPSDKPY_ROOT=%MAPSDKPY_ROOT:~0,-1%"
set "MAPSDKPY_BUILD=%MAPSDKPY_ROOT%\build"

@REM Clean & mkdir
if exist "%MAPSDKPY_BUILD%" rmdir /s /q "%MAPSDKPY_BUILD%"
mkdir "%MAPSDKPY_BUILD%"

@REM build hadmap py
cd "%MAPSDKPY_BUILD%"
cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM deploy
xcopy "%MAPSDKPY_BUILD%\lib\Release\*.pyd" "%MAPSDKPY_BUILD%\bin\Release\" /y /i /e
xcopy "%MAPSDKPY_BUILD%\lib\Release\python*.dll" "%MAPSDKPY_BUILD%\\bin\Release\" /y /i /e

@REM Change the working directory back to the original directory where the script was run
cd "%MAPSDKPY_ROOT%"
