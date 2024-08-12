@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM 检查 python 命令是否存在
where /q python
IF ERRORLEVEL 1 (
    @REM 如果 python 命令不存在, 检查 python3 命令是否存在
    where /q python3
    IF ERRORLEVEL 1 (
        @REM 如果 python 和 python3 命令都不存在, 显示错误信息并退出脚本
        echo Error: Python 3.x not found. Please install Python 3.x or modify script to use appropriate command.
        exit /b
    ) ELSE (
        SET "PYTHON=python3"
    )
) ELSE (
    SET "PYTHON=python"
)
echo "Using Python: %PYTHON%"

@REM Get path of compile tools
set "PROTO=%VCPKG_ROOT%\installed\x64-windows\tools\protobuf\protoc.exe"
set "GRPC_CPP_PLUGIN=%VCPKG_ROOT%\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe"
set "PROTO_PATH=%VCPKG_ROOT%\installed\x64-windows\tools\protobuf"

@REM Set DIR
set "MSG_DIR=%cd%"
set "PATHDIR_BUILD=%MSG_DIR%\build"

@REM Clean and mkdir
if exist %PATHDIR_BUILD% rmdir /s /q %PATHDIR_BUILD%
mkdir %PATHDIR_BUILD%

@REM Gen by grpc_tools.protoc
for %%G in ("%MSG_DIR%\*.proto") do (
    %PYTHON% -m grpc_tools.protoc --proto_path="%MSG_DIR%" --python_out="%PATHDIR_BUILD%" "%%G"
)
