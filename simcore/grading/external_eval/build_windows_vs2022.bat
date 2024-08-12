@echo off
:: Disable command echoing to prevent cluttering the command prompt output

if defined VCPKG_ROOT (
    :: If the environment variable VCPKG_ROOT is defined, this command line will output its value.
    echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    :: If VCPKG_ROOT is not defined, this command line will prompt the user to set this variable
    echo Please set system variables: VCPKG_ROOT
    pause
)

:: Save the directory path of the current script into 'thisDir'
set thisDir=%~dp0

:: Return to the original script directory
cd %thisDir%

:: Check if the 'build' directory exists within the script's directory
if exist .\build (
    :: If it does, remove the 'build' directory and all of its contents
rd /s /q .\build
)

:: Create a new 'build' directory
md .\build

:: Change the working directory to the newly created 'build' directory
cd .\build

:: Use CMake to generate a Visual Studio 2022 project targeting a 64-bit architecture
:: with the Visual Studio 2022 (v143) toolset
:: Note: You can change "Visual Studio 17 2022" to your actual vs version
:: but it is not recommended to change the toolset of v143
cmake .. -G "Visual Studio 17 2022" -A x64 -T v143 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake ^
         -DVCPKG_TARGET_TRIPLET=x64-windows-static

:: Build the generated Visual Studio 2022 project for a 64-bit architecture
:: using the Visual Studio 2022 (v143) toolset in Release configuration
:: with 8 parallel jobs and verbose output
cmake --build . --config Release -j 8 --verbose

:: Change the working directory back to the original directory where the script was run
cd %thisDir%
