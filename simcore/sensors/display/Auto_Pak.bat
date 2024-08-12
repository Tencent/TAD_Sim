REM V2.0 Written by JiangyuLin.
@echo off
echo ========================
if "%1"=="-help" (
	goto HELP_INFO
) else (
	goto START_PACKAGE
)

:HELP_INFO
echo This script is used for parameterized packaging and generation of UE4 execution programs.
echo The script must be located in the root directory of the UE4 project. 
echo Parameterizable items: 
echo @Target platform[Win64/Linux]; 
echo @Target configuration[Shipping/Developmemt]; 
echo @Code of the maps to be packed[see 'MapName' section in Config/DefaultGame.ini]; 
echo @Output path[Absolute path]; 
echo @Engine path or installed engine version[Absolute path or version number];
echo PARAM: [Platform] [Config] [MapList] [SavedPath] [EngineVersion]
goto EXIT

:START_PACKAGE
echo =====START PACKAGE======
:SET_PARAMETERS
echo =====SET PARAMETERS======
SET FilePath=%~dp0
rem ## (Required)Must be assigned.
REM SET EngineVersion=UE_4.22
SET EngineVersion=%5
rem ## (Optional)Can be manually assigned here.
rem ## Custom package path by arg
SET SavedFilePath=%4
if "%SavedFilePath%"=="" (
	SET SavedFilePath=%FilePath%Saved
)
rem ## Custom package map list by arg
SET PakMapList=%3
if "%PakMapList%"=="" (
	SET PakMapList=
)
rem ## Custom build config
SET PakConfig=%2
if "%PakConfig%"=="" (
	SET PakConfig=Developmemt
)
rem ## Custom build config
SET PakPlatform=%1
if "%PakConfig%"=="" (
	SET PakConfig=Win64
)

rem ## Get uproject file and project name
SET ProjectName=
for /f "delims=." %%N in ('dir /b %FilePath%*.uproject*') do (
	set "ProjectName=%%N"
)

echo [Argv]ProjectName:%ProjectName%
echo [Argv]Config:%PakConfig%
echo [Argv]Platform:%PakPlatform%
echo [Argv]MapList:%PakMapList%
echo [Argv]SavedPath:%SavedFilePath%
echo [Argv]InstalledEngineVersion:%EnginePath%

REM REM Iterate Arguments
REM :LOOP
	REM set index=%1
	REM if %index%! == ! goto END
REM rem	add your logic in here
	REM for /f "delims==, tokens=1,*" %%i in (%index%) do (
		REM echo %%i %%j
		REM if %%i equ "-SavePath" (
			REM %SavedFilePath%="%%j"
		REM )
		REM if %%i equ "-MapList" (
			REM %PakMapList%="%%j"
		REM )
	REM )
	REM echo %index%
	REM shift
	REM goto LOOP
REM :END
REM echo "Loop ends."
REM echo %SavedFilePath%
REM echo %PakMapList%
REM pause

REM if "%1"=="" (
	REM SET SavedFilePath=%FilePath%Saved
	REM if "%2"=="" (
		REM SET PakMapList=
	REM ) else (
		REM SET PakMapList=%2
	REM )
REM ) else (
	REM SET SavedFilePath=%1
REM )

if not defined ProjectName (
	echo Can not find .uproject file in %FilePath%!
	goto ExitProjectMissing
)

:SearchEnginePath
echo =====SEARCH ENGINE PATH======
SET bFindEnginePath=false
for /F "tokens=2*" %%A in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%EngineVersion%" /v "InstalledDirectory"') do (
set EnginePath=%%B
SET bFindEnginePath=true
)
if "%bFindEnginePath%"=="true" (
	echo Use installed engine, version:%EngineVersion%
	goto CONFIG_PACKAGE_MAPS
) else (
	echo Engine not installed, version:%EngineVersion%
	goto ReadPathFile
)

:ReadPathFile
echo =====READ PATH FILE======
if exist %FilePath%Auto_Path.txt (
	REM echo Auto_Path.txt exist!
	for /f "delims=[" %%i in (%FilePath%Auto_Path.txt) do (
		echo %%i
		SET EnginePath=%%i
		echo Use the engine path in the Auto_Path.txt.
		goto CONFIG_PACKAGE_MAPS
	)
	echo Auto_Path.txt is emty!
	goto ExitPathMissing
)
echo Auto_Path.txt not exist!
goto ExitPathMissing

REM for %%a in (c d e f g h i j k l m n o p q r s t u v w x y z) do (
REM echo Searching engine path...
REM if exist %%a:\ (
REM cd /d %%a:\
REM for /f "delims=" %%b in ('dir /s /b /ad "%EngineVersion%"') do (
REM echo Path is : %%b
REM if exist %%b\Engine\Build\BatchFiles\RunUAT.bat (
REM SET EnginePath=%%b
REM cd /d %FilePath%
REM echo Create Auto_Path.txt.
REM echo %%b>Auto_Path.txt
REM goto RunUATbat
REM )
REM )))
REM echo Cant find engine path!
REM goto ExitPathMissing

:CONFIG_PACKAGE_MAPS
echo =====CONFIG PACKAGE MAPS======
rem ## Config map list to pak
if not exist %FilePath%Config\Windows (
	md %FilePath%Config\Windows
)
if exist %FilePath%PakMapsConfig.exe (
	call %FilePath%PakMapsConfig.exe %FilePath%Config\DefaultGame.ini %FilePath%Config\Windows\WindowsGame.ini %PakMapList%
	goto RunUATbat
) else (
	goto ExitPakMapsConfigMissing
)

:RunUATbat
echo =====RUN UAT BAT======
SET RunUATPath=%EnginePath%\Engine\Build\BatchFiles\RunUAT.bat
SET ReBuildPath=%EnginePath%\Engine\Build\BatchFiles\ReBuild.bat
SET ProjectPath=%FilePath%%ProjectName%.uproject

echo Use Project Path : %ProjectPath%
echo Use Engine Path : %EnginePath%
echo Use Platform : %PakPlatform%
echo Use Config : %PakConfig%
echo Use Saved File Path : %SavedFilePath%
REM goto CheckOutputFiles
rem ## RunUAT
REM call "%ReBuildPath%" DisplayEditor Win64 Development "%ProjectPath%" -waitmutex
rem call "%RunUATPath%" BuildCookRun -project=%ProjectPath% -noP4 -platform=Win64 -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory="%SavedFilePath%"
call "%RunUATPath%" BuildCookRun -nocompile -compileeditor -installed -nop4 -project="%ProjectPath%" -cook -stage -archive -archivedirectory=%SavedFilePath% -package -clientconfig=%PakConfig% -ue4exe=UE4Editor-Cmd.exe -pak -prereqs -nodebuginfo -targetplatform=%PakPlatform% -build -utf8output -RunAutomationTests
rem call "%RunUATPath%" BuildCookRun -compile -compileeditor -installed -nop4 -project="%ProjectPath%" -cook -stage -archive -archivedirectory="%SavedFilePath%" -package -clientconfig=Development -ue4exe=UE4Editor-Cmd.exe -clean -pak -prereqs -nodebuginfo -targetplatform=Win64 -build -utf8output      -ExecCmds="Automation RunTests Display; quit"%SavedFilePath%
echo Exit Code is %errorlevel%
goto CheckOutputFiles

:CheckOutputFiles
echo =====CHECK OUTPUT FILES======
REM ## Get Path Of Output files 
SET OutputFilePath=XXX
SET LauncherFile=XXX
if "%PakPlatform%"=="Linux" (
	SET OutputFilePath=%SavedFilePath%\StagedBuilds\LinuxNoEditor
	SET LauncherFile=%ProjectName%.sh
)
if "%PakPlatform%"=="Win64" (
	SET OutputFilePath=%SavedFilePath%\StagedBuilds\WindowsNoEditor
	SET LauncherFile=%ProjectName%.exe
)
REM ## Check Files Exist
if exist %OutputFilePath%\%LauncherFile% (
	echo Output File Exist:%OutputFilePath%\%LauncherFile%
	goto CopyFiles
) else (
	echo Output File Does Not Exist:%OutputFilePath%\%LauncherFile%
	goto EXIT_BUILDCOOKPACKAGE_FAILED
)

:CopyFiles
echo =====COPY FILES======
echo Copy Additional files...
xcopy %FilePath%XMLFiles\*.* %OutputFilePath%\%ProjectName%\XMLFiles\ /E /R /Y 
REM xcopy %FilePath%\Binaries\ThirdParty\*.* %OutputFilePath%\%ProjectName%\Binaries\ThirdParty\ /E /R /Y 
if "%PakPlatform%"=="Linux" (
	xcopy %FilePath%%ProjectName%.sh %OutputFilePath%\ /R /Y 
	REM ## xcopy %FilePath%Binaries\Linux\*.so %OutputFilePath%\%ProjectName%\Binaries\Linux\ /E /R  /Y
	xcopy %FilePath%Saved\Config\WindowsNoEditor\Game.ini %OutputFilePath%\%ProjectName%\Saved\Config\LinuxNoEditor\ /E /R  /Y
	xcopy %FilePath%Binaries\Linux\Display.target %OutputFilePath%\%ProjectName%\Binaries\Linux\ /E /R  /Y 
)
if "%PakPlatform%"=="Win64" (
	REM ## xcopy %FilePath%\Binaries\Win64\*.dll %OutputFilePath%\%ProjectName%\Binaries\Win64\ /E /R /Y 
	xcopy %FilePath%Saved\Config\WindowsNoEditor\Game.ini %OutputFilePath%\%ProjectName%\Saved\Config\WindowsNoEditor\ /E /R  /Y 
	xcopy %FilePath%Binaries\Win64\Display.pdb %OutputFilePath%\%ProjectName%\Binaries\Win64\ /E /R  /Y
)
REM echo echo off >%OutputFilePath%\Start.bat
goto EXIT_SUCCESS


REM ## Exit type


:ExitProjectMissing
echo ERROR: PROJECT FILE IS MISSING!
goto EXIT_FAIL

:ExitPathMissing
echo Delete Auto_Path.txt.
del %FilePath%Auto_Path.txt
echo ERROR: ENGINE PATH IS MISSING!
goto EXIT_FAIL

:ExitPakMapsConfigMissing
echo ERROR: PAKMAPSCONFIG.EXE IS MISSING!
goto EXIT_FAIL

:EXIT_BUILDCOOKPACKAGE_FAILED
echo ERROR: BUILD_COOK_PACKAGE IS FAILED!
goto EXIT_FAIL

:EXIT_FAIL
echo =====PACKAGE FAILED======
exit /b 1
goto EXIT

:EXIT_SUCCESS
echo =====PACKAGE SUCCESS======
exit /b 0
goto EXIT

:EXIT
cd %FilePath%
echo ========================
