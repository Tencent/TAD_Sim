@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Setting build parmameters
set "OSI_ROOT=%~dp0"
set "OSI_ROOT=%OSI_ROOT:~0,-1%"
set "OSI_BUILD=%OSI_ROOT%\build"
set "PATHDIR_DEPENDENCE_MSG=%OSI_ROOT%\..\..\common\message\build"
set "PATHDIR_DEPENDENCE_MSG_GENERATER=%OSI_ROOT%\..\..\common\message"

@REM Clean & mkdir
IF EXIST "%OSI_BUILD%" rmdir /s /q "%OSI_BUILD%"
mkdir "%OSI_BUILD%"

@REM Check message dependencies
if not exist "%PATHDIR_DEPENDENCE_MSG%" (
    echo "The directory %PATHDIR_DEPENDENCE_MSG% does not exist."
    echo "Running generate_cpp.bat in the parent directory: %PATHDIR_DEPENDENCE_MSG_GENERATER%"
    cd "%PATHDIR_DEPENDENCE_MSG_GENERATER%"
    call generate_cpp.bat
) else (
    echo "The directory %PATHDIR_DEPENDENCE_MSG% exists."
)

@REM build
cd "%OSI_BUILD%"
cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

@REM Change the working directory back to the original directory where the script was run
cd "%OSI_ROOT%"
