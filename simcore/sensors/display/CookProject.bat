@ECHO OFF
REM V1.2 Written by JiangyuLin.

SET FilePath=%~dp0
rem ## (Required)Must be assigned by parameter.
SET EngineVersion=%1
rem ## (Required)Can be assigned by parameter. Default value is Display.
SET ProjectName=%2

SET UnrealEngineDir=%1

SET PakMapList=%3

if "%1"=="-help" (
echo CookProject.bat [EngineVersion] [ProjectName=Display] [PackageMapCode]
echo EXAMPLE: CookProject.bat 4.22 Display 1+2
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
	goto Cook
) else (
	goto SearchEnginePath
)

:SearchEnginePath
REM echo Locating where Unreal Engine is installed by checking the Windows registry...
for /F "tokens=2*" %%A in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%EngineVersion%" /v "InstalledDirectory"') do (
set UnrealEngineDir=%%B
)
if "%UnrealEngineDir%"=="" goto CannotFindEnginePath
goto Cook

REM :Generate
REM set UBTPath=%UnrealEngineDir%\Engine\Binaries\DotNET\UnrealBuildTool.exe
REM echo Using Unreal Engine directory: "%UnrealEngineDir%\Engine"
REM "%UBTPath%" -projectfiles -project="%~dp0\%ProjectName%.uproject" -game -rocket -progress -IntelliSense -NoXGE
REM goto Exit

:Cook
set UE4EditorCmdPath=%UnrealEngineDir%\Engine\Binaries\Win64\UE4Editor-Cmd.exe
echo Using directory: "%UE4EditorCmdPath%"
REM ## Configrate Package Maps
if not exist %FilePath%Config\Windows (
	md %FilePath%Config\Windows
)
if exist %FilePath%PakMapsConfig.exe (
	call %FilePath%PakMapsConfig.exe %FilePath%Config\DefaultGame.ini %FilePath%Config\Windows\WindowsGame.ini %PakMapList%
	REM call %FilePath%PakMapsConfig.exe %FilePath%Config\DefaultGame.ini %PakMapList%
) else (
	goto ExitPakMapsConfigMissing
)
REM -abslog="%~dp0\Saved\Cook.txt"
"%UE4EditorCmdPath%" "%~dp0\%ProjectName%.uproject" -run=Cook -TargetPlatform=WindowsNoEditor -fileopenlog -unversioned -stdout -CrashForUAT -unattended -NoLogTimes -UTF8Output
goto CopyFile

:CopyFile
xcopy %~dp0\Saved\Config\Windows\*.* %~dp0\Saved\Cooked\WindowsNoEditor\%ProjectName%\Saved\Config\WindowsNoEditor\ /E /R /Y 
goto Exit

:CannotFindEnginePath
echo Generate failed! Can`t find unreal engine path!
goto Exit

:ExitPakMapsConfigMissing
echo PakMapsConfig.exe is missing.
goto Exit

:Exit
cd %FilePath%
