@echo off

@REM Setting build parmameters
set "DISPLAY_BUILD_CONFIG_ROOT=%~dp0"
set "DISPLAY_BUILD_CONFIG_ROOT=%DISPLAY_BUILD_CONFIG_ROOT:~0,-1%"
set "DISPLAY_BUILD_CONFIG_BUILD=%DISPLAY_BUILD_CONFIG_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%DISPLAY_BUILD_CONFIG_BUILD%" rmdir /s /q "%DISPLAY_BUILD_CONFIG_BUILD%"
mkdir "%DISPLAY_BUILD_CONFIG_BUILD%"

@REM md mini
@REM curl https://raw.githubusercontent.com/metayeti/mINI/master/src/mini/ini.h -o ./mini/ini.h

@REM build
cd "%DISPLAY_BUILD_CONFIG_BUILD%"
cmake  .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%DISPLAY_BUILD_CONFIG_BUILD%"
