@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "DISPLAY_ROOT=%~dp0"
set "DISPLAY_ROOT=%DISPLAY_ROOT:~0,-1%"
set "DISPLAY_BUILD=%DISPLAY_ROOT%\build"
set "PakConfig=Development"
set "PakMapList=0+36"
set "PakPlatform=Win64"
set "EnginePath=4.27"

@REM Clean & mkdir
IF EXIST "%DISPLAY_BUILD%" rmdir /s /q "%DISPLAY_BUILD%"
IF EXIST "%DISPLAY_ROOT%\Saved" rmdir /s /q "%DISPLAY_ROOT%\Saved"
IF EXIST "%DISPLAY_ROOT%\Binaries" rmdir /s /q "%DISPLAY_ROOT%\Binaries"
IF EXIST "%DISPLAY_ROOT%\Intermediate" rmdir /s /q "%DISPLAY_ROOT%\Intermediate"
mkdir "%DISPLAY_BUILD%"
mkdir "%DISPLAY_BUILD%\bin"

@REM prerequisites
call download_deps.bat

@REM build
echo "=================WIN64 PACKAGING CONFIGURATION================="
cd "%DISPLAY_ROOT%"
call "Auto_Pak.bat" %PakPlatform% %PakConfig% %PakMapList% %DISPLAY_ROOT%\Saved\ %EnginePath%

@REM deploy
echo "=================DEPLOY================="
cd "%DISPLAY_ROOT%"
xcopy "%DISPLAY_ROOT%\Saved\StagedBuilds\WindowsNoEditor\*" "%DISPLAY_BUILD%\bin\Display\" /y /i /e
xcopy "%DISPLAY_ROOT%\XMLFiles\*" "%DISPLAY_BUILD%\bin\Display\Display\XMLFiles\" /y /i /e

cd "%DISPLAY_BUILD%\bin"
tar zcvf display.tar.gz Display

@REM Change the working directory back to the original directory where the script was run
cd "%DISPLAY_ROOT%"
