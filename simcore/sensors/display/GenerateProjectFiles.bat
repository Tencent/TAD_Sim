@ECHO OFF
REM V1.2 Written by JiangyuLin.

rem ## (Required)Must be assigned by parameter.
SET EngineVersion=%1
rem ## (Required)Can be assigned by parameter. Default value is Display.
SET ProjectName=%2

SET UnrealEngineDir=%1

if "%1"=="-help" (
echo GenerateProjectFiles.bat [EngineVersion] [ProjectName=Display]
echo Or
echo GenerateProjectFiles.bat [EnginePath] [ProjectName=Display]
echo EXAMPLE: GenerateProjectFiles.bat 4.22 Or GenerateProjectFiles.bat D:\\UE4.22 Display
goto Exit
)

if "%ProjectName%"=="" (
echo ProjectName is none! Use default name : Display.
SET ProjectName=Display
)
echo ProjectName is %ProjectName%

if "%EngineVersion%"=="" (
echo Generate failed! Need engine version or engine path as parameter! 
goto Exit
)

if exist %UnrealEngineDir% (
	goto Generate
) else (
	goto SearchEnginePath
)

:SearchEnginePath
REM echo Locating where Unreal Engine is installed by checking the Windows registry...
for /F "tokens=2*" %%A in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%EngineVersion%" /v "InstalledDirectory"') do (
set UnrealEngineDir=%%B
)
if "%UnrealEngineDir%"=="" goto CannotFindEnginePath

goto Generate

:Generate
set UBTPath=%UnrealEngineDir%\Engine\Binaries\DotNET\UnrealBuildTool.exe
echo Using Unreal Engine directory: "%UnrealEngineDir%\Engine"
"%UBTPath%" -projectfiles -project="%~dp0\%ProjectName%.uproject" -game -rocket -progress -IntelliSense -NoXGE
goto CopyFiles

:CopyFiles
echo =====COPY FILES======
echo Copy Additional files...
REM xcopy %FilePath%XMLFiles\*.* %OutputFilePath%\%ProjectName%\XMLFiles\ /E /R /Y 
xcopy %~dp0\Plugins\HadMap\ThirdParty\HadMapSDK\Lib\x64\Release\*.dll %~dp0\Binaries\Win64\ /E /R /Y 
REM xcopy %FilePath%Saved\Config\WindowsNoEditor\Game.ini %OutputFilePath%\%ProjectName%\Saved\Config\WindowsNoEditor\ /E /R  /Y 

REM echo echo off >%OutputFilePath%\Start.bat
goto Exit

:CannotFindEnginePath
echo Generate failed! Can`t find unreal engine path!
goto Exit

:Exit
