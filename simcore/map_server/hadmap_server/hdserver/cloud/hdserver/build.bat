@echo off

set mode=%1
set BuildMode="Release|x64"
if "%mode%" == "debug" (
    set BuildMode="Debug|x64"
    goto :main
)

if "%mode%" == "release"(
    set BuildMode="Release|x64"
    goto :main
)

:printHelp

echo please input build mode.possible value is debug or release
goto :eof

:main

echo initialize environment 

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Auxiliary\Build\vcvars64.bat"

echo clean 

devenv ./build/scene_wrapper.sln /Clean

echo start build %BuildMode%

devenv ./build/scene_wrapper.sln /Build %BuildMode% /project ./build/scene_wrapper.vcxproj

echo build finished

:eof
