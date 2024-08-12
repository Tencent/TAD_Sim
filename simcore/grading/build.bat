@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Check if the parameter "static" is passed in
if "%~1"=="static" (
    echo Using static triplet and static libraries
    set TRIPLET=-DVCPKG_TARGET_TRIPLET=x64-windows-static
    set STATIC_LIBS=-DUSE_STATIC_LIBS=ON
) else (
    echo Using default triplet and shared libraries
    set TRIPLET=
    set STATIC_LIBS=-DUSE_STATIC_LIBS=OFF
)

@REM Setting build parmameters
set "GRADING_ROOT=%~dp0"
set "GRADING_ROOT=%GRADING_ROOT:~0,-1%"
set "GRADING_BUILD=%GRADING_ROOT%\build"

@REM Clean & mkdir
IF EXIST "%GRADING_BUILD%" rmdir /s /q "%GRADING_BUILD%"
mkdir "%GRADING_BUILD%"

@REM Build the generated Visual Studio 2022 project for a 64-bit architecture
@REM using the Visual Studio 2022 (v143) toolset in Release configuration
@REM with 8 parallel jobs and verbose output
cd "%GRADING_BUILD%"
cmake .. -G "Visual Studio 17 2022" -A x64 -T v143 ^
         -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" %TRIPLET% %STATIC_LIBS%
cmake --build . --config Release -j 8 --verbose

@REM Execute the SDK compilation script and copy the output to the 'build' directory
@REM Note: the SDK is available when "static" is passed in
call "%GRADING_ROOT%\external_eval\package_sdk_windows.bat"
xcopy "%GRADING_ROOT%\external_eval\txSimGradingSDK_windows.tar.gz" "%GRADING_ROOT%\build\bin" /r /y

@REM Change the working directory back to the original directory where the script was run
cd "%GRADING_ROOT%"
