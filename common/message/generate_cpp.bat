@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Get path of compile tools
set "PROTO=%VCPKG_ROOT%\installed\x64-windows\tools\protobuf\protoc.exe"
set "GRPC_CPP_PLUGIN=%VCPKG_ROOT%\installed\x64-windows\tools\grpc\grpc_cpp_plugin.exe"
set "PROTO_PATH=%VCPKG_ROOT%\installed\x64-windows\tools\protobuf"

@REM Set DIR
set "MESSAGE_ROOT=%cd%"
set "MESSAGE_BUILD=%MESSAGE_ROOT%\build"

@REM Clean and mkdir
if exist %MESSAGE_BUILD% rmdir /s /q %MESSAGE_BUILD%
mkdir %MESSAGE_BUILD%

@REM Gen by protoc.exe
for %%G in (%MESSAGE_ROOT%\*.proto) do %PROTO% --proto_path=%PROTO_PATH% -I=%MESSAGE_ROOT% --cpp_out=%MESSAGE_BUILD% %%G

@REM Gen by protoc.exe with grpc_cpp_plugin.exe
%PROTO% -I=%MESSAGE_ROOT% --grpc_out=%MESSAGE_BUILD% --plugin=protoc-gen-grpc=%GRPC_CPP_PLUGIN% %MESSAGE_ROOT%\coordinatorService.proto
%PROTO% -I=%MESSAGE_ROOT% --grpc_out=%MESSAGE_BUILD% --plugin=protoc-gen-grpc=%GRPC_CPP_PLUGIN% %MESSAGE_ROOT%\simDriverService.proto
%PROTO% -I=%MESSAGE_ROOT% --grpc_out=%MESSAGE_BUILD% --plugin=protoc-gen-grpc=%GRPC_CPP_PLUGIN% %MESSAGE_ROOT%\simAgent.proto
%PROTO% -I=%MESSAGE_ROOT% --grpc_out=%MESSAGE_BUILD% --plugin=protoc-gen-grpc=%GRPC_CPP_PLUGIN% %MESSAGE_ROOT%\xil_interface.proto
