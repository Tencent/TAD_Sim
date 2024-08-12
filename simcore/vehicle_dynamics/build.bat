@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "VD_ROOT=%~dp0"
set "VD_ROOT=%VD_ROOT:~0,-1%"
set "VD_BUILD=%VD_ROOT%\build"

@REM Clean and mkdir
if exist "%VD_BUILD%" rmdir /s /q "%VD_BUILD%"
mkdir "%VD_BUILD%"

@REM build
cd %VD_BUILD%
cmake .. -G "Visual Studio 17 2022" -T "v143" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM deplod
xcopy "%VD_ROOT%\param\txcar.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\txcar_EV.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\txcar_Hybrid.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\txcar_ICE.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\txcar_template.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\txcar_template_hybrid.json" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\CS_car225_60R18.tir" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\pac2002_235_60R16.tir" "%VD_BUILD%\bin\Release" /y
xcopy "%VD_ROOT%\param\TASS_car205_60R15.tir" "%VD_BUILD%\bin\Release" /y

@REM Change the working directory back to the original directory where the script was run
cd "%VD_ROOT%"
