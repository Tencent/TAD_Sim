@echo off

SETLOCAL

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Get & set ENABLE_ENCRYPTION_TIMESTAMP
if NOT "%~1"=="" (
    set ENABLE_ENCRYPTION_TIMESTAMP=%~1
) else (
    set ENABLE_ENCRYPTION_TIMESTAMP=0
)

@REM Setting build parmameters
set "FRAMEWORK_ROOT=%~dp0"
set "FRAMEWORK_ROOT=%FRAMEWORK_ROOT:~0,-1%"
set "FRAMEWORK_BUILD=%FRAMEWORK_ROOT%\build"
set "FRAMEWORK_CLI=%FRAMEWORK_ROOT%\cli"
set "MESSAGE_ROOT=%FRAMEWORK_ROOT%\..\..\common\message"
set "FRAMEWORK_TXSIM=%FRAMEWORK_ROOT%\txSim"
set "FRAMEWORK_TXSIM_INC=%FRAMEWORK_TXSIM%\inc"
set "FRAMEWORK_TXSIM_LIB=%FRAMEWORK_TXSIM%\lib"
set "FRAMEWORK_TXSIM_MSG=%FRAMEWORK_TXSIM%\msgs"
set "FRAMEWORK_TXSIM_DOC=%FRAMEWORK_TXSIM%\doc"
set "FRAMEWORK_TXSIM_EXAMPLE=%FRAMEWORK_TXSIM%\example"
set "SDK_NAME=txSimSDK_windows.tar.gz"

@REM Clean & mkdir
IF EXIST "%FRAMEWORK_BUILD%" rmdir /s /q "%FRAMEWORK_BUILD%"
mkdir "%FRAMEWORK_BUILD%"
IF EXIST "%FRAMEWORK_TXSIM%" rmdir /s /q "%FRAMEWORK_TXSIM%"
mkdir "%FRAMEWORK_TXSIM%"
mkdir "%FRAMEWORK_TXSIM_INC%"
mkdir "%FRAMEWORK_TXSIM_LIB%"
mkdir "%FRAMEWORK_TXSIM_MSG%"
mkdir "%FRAMEWORK_TXSIM_DOC%"
mkdir "%FRAMEWORK_TXSIM_EXAMPLE%"

@REM build framework
echo "framework build start..."
cd "%FRAMEWORK_BUILD%"
set VCINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.com
cmake .. -DTXSIM_ENCRYPTION_TIMESTAMP_INT=%ENABLE_ENCRYPTION_TIMESTAMP% %FRAMEWORK_ROOT% ^
         -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release

for %%b in (txsim-module-service) do xcopy %FRAMEWORK_BUILD_LIB%\Release\%%b.lib %FRAMEWORK_TXSIM_LIB%\
echo "framework build successfully."

@REM build framework cli
echo "framework_cli build start..."
cd "%FRAMEWORK_CLI%"
go mod tidy -compat="1.17"
go build
echo "framework_cli build successfully."

@REM deploy txSim
echo "txSim build start..."
cd "%FRAMEWORK_ROOT%"
xcopy "%FRAMEWORK_ROOT%\src\txsim*.h" "%FRAMEWORK_TXSIM_INC%\"
xcopy "%FRAMEWORK_ROOT%\src\visibility.h" "%FRAMEWORK_TXSIM_INC%\"
xcopy "%MESSAGE_ROOT%\*.proto" "%FRAMEWORK_TXSIM_MSG%\"
xcopy %MESSAGE_ROOT%\generate_cpp.bat %FRAMEWORK_TXSIM_MSG%\
del "%FRAMEWORK_TXSIM_MSG%\moduleService.proto"
del "%FRAMEWORK_TXSIM_MSG%\sim_cloud_service.proto"
del "%FRAMEWORK_TXSIM_MSG%\sim_cloud_city_service.proto"
xcopy "%FRAMEWORK_ROOT%\examples\*" "%FRAMEWORK_TXSIM_EXAMPLE%\"
xcopy "%FRAMEWORK_ROOT%\docs\sphinx\*" "%FRAMEWORK_TXSIM_DOC%\"
cd /d "%FRAMEWORK_TXSIM%\
tar zcvf "%FRAMEWORK_BUILD%\%SDK_NAME%" ./*
cd "%FRAMEWORK_ROOT%"
rd /s /q "%FRAMEWORK_TXSIM%"

@REM Change the working directory back to the original directory where the script was run
cd "%FRAMEWORK_ROOT%"
