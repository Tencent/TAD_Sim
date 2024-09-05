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
@REM Setting build parmameters CLI
set "FRAMEWORK_CLI=%FRAMEWORK_ROOT%\cli"
set "MESSAGE_ROOT=%FRAMEWORK_ROOT%\..\..\common\message"
@REM Setting build parmameters txSim
set "FRAMEWORK_TXSIM=%FRAMEWORK_BUILD%\txSim"
set "FRAMEWORK_TXSIM_INC=%FRAMEWORK_TXSIM%\inc"
set "FRAMEWORK_TXSIM_LIB=%FRAMEWORK_TXSIM%\lib"
set "FRAMEWORK_TXSIM_MSG=%FRAMEWORK_TXSIM%\msgs"
set "FRAMEWORK_TXSIM_DOC=%FRAMEWORK_TXSIM%\doc"
set "FRAMEWORK_TXSIM_EXAMPLE=%FRAMEWORK_TXSIM%\example"
set "SDK_NAME=txSimSDK.tar.gz"

@REM Clean & mkdir
IF EXIST "%FRAMEWORK_BUILD%" rmdir /s /q "%FRAMEWORK_BUILD%"
mkdir "%FRAMEWORK_BUILD%"
@REM Clean & mkdir CLI
IF EXIST "%FRAMEWORK_CLI%\cli" del /f /q "%FRAMEWORK_CLI%\cli"
IF EXIST "%FRAMEWORK_CLI%\go.sum" del /f /q "%FRAMEWORK_CLI%\go.sum"
IF EXIST "%FRAMEWORK_ROOT%\src\node_addon\build" rmdir /s /q "%FRAMEWORK_ROOT%\src\node_addon\build"
@REM Clean & mkdir txSim
mkdir "%FRAMEWORK_TXSIM%"
mkdir "%FRAMEWORK_TXSIM_INC%"
mkdir "%FRAMEWORK_TXSIM_LIB%"
mkdir "%FRAMEWORK_TXSIM_MSG%"
mkdir "%FRAMEWORK_TXSIM_DOC%"
mkdir "%FRAMEWORK_TXSIM_EXAMPLE%"

@REM build framework
cd "%FRAMEWORK_BUILD%"
echo "framework build start..."
set VCINSTALLDIR=%ProgramFiles%\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.com
cmake .. -DTXSIM_ENCRYPTION_TIMESTAMP_INT=%ENABLE_ENCRYPTION_TIMESTAMP% %FRAMEWORK_ROOT% ^
         -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake"
cmake --build . --config Release
echo "framework build successfully."
echo.

@REM build framework cli
cd "%FRAMEWORK_CLI%"
echo "framework_cli build start..."
go env -w GOPROXY=https://goproxy.io,direct
go mod tidy -compat="1.17"
go build
echo "framework_cli build successfully."
echo.

@REM deploy txSim
cd "%FRAMEWORK_ROOT%"
echo "txSim build start..."
xcopy "%FRAMEWORK_ROOT%\src\txsim*.h" "%FRAMEWORK_TXSIM_INC%\"
xcopy "%FRAMEWORK_ROOT%\src\visibility.h" "%FRAMEWORK_TXSIM_INC%\"
xcopy "%MESSAGE_ROOT%\*.proto" "%FRAMEWORK_TXSIM_MSG%\"
xcopy %MESSAGE_ROOT%\generate_cpp.bat %FRAMEWORK_TXSIM_MSG%\
del "%FRAMEWORK_TXSIM_MSG%\moduleService.proto"
del "%FRAMEWORK_TXSIM_MSG%\sim_cloud_service.proto"
del "%FRAMEWORK_TXSIM_MSG%\sim_cloud_city_service.proto"
xcopy "%FRAMEWORK_ROOT%\examples\*" "%FRAMEWORK_TXSIM_EXAMPLE%\"
xcopy "%FRAMEWORK_ROOT%\docs\sphinx\*" "%FRAMEWORK_TXSIM_DOC%\"
for %%b in (txsim-module-service) do xcopy %FRAMEWORK_BUILD%\lib\Release\%%b.lib %FRAMEWORK_TXSIM_LIB%\

cd /d "%FRAMEWORK_TXSIM%\
tar zcvf "%FRAMEWORK_BUILD%\%SDK_NAME%" ./*
cd "%FRAMEWORK_ROOT%"
rd /s /q "%FRAMEWORK_TXSIM%"
echo "txSim build successfully."
echo.

@REM Change the working directory back to the original directory where the script was run
cd "%FRAMEWORK_ROOT%"
