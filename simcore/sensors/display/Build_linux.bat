@echo off
echo =================LINUX PACKAGING CONFIGURATION=================
set AbsDir_This=%~dp0
set PakPath=%AbsDir_This%Saved\
set PakMapList=%1
set PakConfig=Development
SET PakPlatform=Linux

:ReadPathFile
if exist %AbsDir_This%Auto_Path.txt (
	echo Auto_Path.txt exist!
	for /f "delims=[" %%i in (%AbsDir_This%Auto_Path.txt) do (
		echo %%i
		SET EnginePath=%%i
	)
) else (
echo Auto_Path.txt not exist!
SET EnginePath=4.27
)

mkdir %AbsDir_This%Binaries
mkdir %AbsDir_This%Binaries\Win64
echo %CUDA_PATH%

copy "%CUDA_PATH%\bin\nppisu64_11.dll" "%AbsDir_This%Binaries\Win64\nppisu64_11.dll"
copy "%CUDA_PATH%\bin\nppc64_11.dll" "%AbsDir_This%Binaries\Win64\nppc64_11.dll"
copy "%CUDA_PATH%\bin\nvjpeg64_11.dll" "%AbsDir_This%Binaries\Win64\nvjpeg64_11.dll"
copy "%CUDA_PATH%\bin\nppidei64_11.dll" "%AbsDir_This%Binaries\Win64\nppidei64_11.dll"
copy "%CUDA_PATH%\bin\nppig64_11.dll" "%AbsDir_This%Binaries\Win64\nppig64_11.dll"
copy "%CUDA_PATH%\bin\cublas64_11.dll" "%AbsDir_This%Binaries\Win64\cublas64_11.dll"
copy "%CUDA_PATH%\bin\cudnn64_8.dll" "%AbsDir_This%Binaries\Win64\cudnn64_8.dll"
copy "%CUDA_PATH%\bin\cublasLt64_11.dll" "%AbsDir_This%Binaries\Win64\cublasLt64_11.dll"
copy "%CUDA_PATH%\bin\nvrtc64_112_0.dll" "%AbsDir_This%Binaries\Win64\nvrtc64_112_0.dll"


call "%AbsDir_This%Auto_Pak.bat" %PakPlatform% %PakConfig% %PakMapList% %PakPath% %EnginePath%
