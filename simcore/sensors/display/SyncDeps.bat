rem V1.3 Written by JiangyuLin
@echo off

rem Process args
set /a DoCopy=1
set /a DoBuild=1
if "%1"=="-nobuild" (
	set /a DoBuild=0
	echo NoBuild!
)
if "%1"=="-nocopy" (
	set /a DoCopy=0
	echo NoCopy!
)

rem Define folder name
set FolderName_Display=Display
set FolderName_SimCore=SimCore
set FolderName_Deps=deps
set FolderName_SimMsg=sim_msg
set FolderName_HadMap=hadmap

rem Init folder directory
set AbsDir_This=%~dp0
set AbsDir_Root=%AbsDir_This%..
set AbsDir_Display=%AbsDir_This%
set AbsDir_SimCore=
set AbsDir_Deps=
set AbsDir_SimMsg=
set AbsDir_HadMap=

rem Define path 
set FilePath_SimModule=%AbsDir_Display%Plugins\SimModule
set FilePath_HadMap=%AbsDir_Display%Plugins\HadMap

REM set ReDir_SimCore=%ReDir_Root%\SimCore
REM set REDir_Deps=%ReDir_SimCore%\deps
REM set ReDir_SimMsg=%ReDir_SimCore%\sim_msg
REM set ReDir_HadMap=%ReDir_SimCore%\hadmap

REM echo %AbsDir_This%
REM echo ReDir_Root=%ReDir_Root%
REM echo ReDir_SimCore=%ReDir_SimCore%
REM echo REDir_Deps=%REDir_Deps%
REM echo ReDir_SimMsg=%ReDir_SimMsg%
REM echo ReDir_HadMap=%ReDir_HadMap%

goto LocateDir

:LocateDir
echo LocateDir..
cd /d %AbsDir_Root%
rem To flash var immedately
setlocal enabledelayedexpansion
rem Find SimCore
if exist !cd!\%FolderName_SimCore% (
	set AbsDir_SimCore=!cd!\%FolderName_SimCore%
	cd %FolderName_SimCore%
	rem Find deps
	if exist !cd!\%FolderName_Deps% (
		set AbsDir_Deps=!cd!\%FolderName_Deps%
	) else (
		goto Exit_DepsError
	)
	rem Fimd msg
	if exist !cd!\%FolderName_SimMsg% (
		set AbsDir_SimMsg=!cd!\%FolderName_SimMsg%
	) else (
		goto Exit_MsgError
	)
	rem Find hadmap
	if exist !cd!\%FolderName_HadMap% (
		set AbsDir_HadMap=!cd!\%FolderName_HadMap%
	) else (
		goto Exit_HadMapError
	)
) else (
	goto Exit_SimCoreError
)

REM echo AbsDir_SimCore=%AbsDir_SimCore%
REM echo AbsDir_Deps=%AbsDir_Deps%
REM echo AbsDir_SimMsg=%AbsDir_SimMsg%
REM echo AbsDir_HadMap=%AbsDir_HadMap%
cd /d %AbsDir_Root%

goto GenerateMsg


:GenerateMsg
echo GenerateMsg..
if exist %AbsDir_SimMsg%\generate_cpp.bat (
	call "%AbsDir_SimMsg%\generate_cpp.bat"
) else (
	goto Exit_MsgError
)

if %DoBuild% equ 1 (
	goto BuildSimModule
) else (
	goto CopySimModule
)
rem goto BuildSimModule

:BuildSimModule
echo BuildSimModule..
setlocal ENABLEDELAYEDEXPANSION
if exist %AbsDir_SimCore%\framework\build_sln.bat (
	call "%AbsDir_SimCore%\framework\build_sln.bat"
	if !errorlevel! neq 0 (
		echo Build SimModule failed!
		goto Exit_SimCoreError
	)
) else (
	echo build_sln.bat is missing!
	goto Exit_SimCoreError
)

if %DoCopy% equ 1 (
	goto CopySimModule
) else (
	goto Exit_Successed
)
rem goto CopySimModule

:CopySimModule
echo CopySimModule..
if exist %AbsDir_SimCore%\framework\includes\txSimModule.h (
	xcopy %AbsDir_SimCore%\framework\includes\txSimModule.h %FilePath_SimModule%\ThirdParty\SimModuleSDK\Inc\ /E /R /Y
) else (
	echo txSimModule.h is missing!
	goto Exit_SimCoreError
)
if exist %AbsDir_SimCore%\framework\includes\txSimModuleServer.h (
	xcopy %AbsDir_SimCore%\framework\includes\txSimModuleServer.h %FilePath_SimModule%\ThirdParty\SimModuleSDK\Inc\ /E /R /Y
) else (
	echo txSimModuleServer.h is missing!
	goto Exit_SimCoreError
)
if exist %AbsDir_SimCore%\framework\build\lib\Release\txSimModule.lib (
	xcopy %AbsDir_SimCore%\framework\build\lib\Release\txSimModule.lib %FilePath_SimModule%\ThirdParty\SimModuleSDK\Lib\Win64\Release\ /E /R /Y
) else (
	echo txSimModule.lib is missing!
	goto Exit_SimCoreError
)
if exist %AbsDir_SimCore%\deps\lib\Release\libzmq.lib (
	xcopy %AbsDir_SimCore%\deps\lib\Release\libzmq.lib %FilePath_SimModule%\ThirdParty\SimModuleSDK\Lib\Win64\Release\ /E /R /Y
) else (
	echo libzmq.lib is missing!
	goto Exit_DepsError
)
goto ModifyMsg

:ModifyMsg
echo ModifyMsg..

call "%AbsDir_Display%FixCC.exe" -path=%AbsDir_SimMsg%
REM setlocal DISABLEDELAYEDEXPANSION
REM set AddStr_4=#pragma warning(disable : 4800)
REM set AddStr_0=#pragma warning(disable : 4146)
REM set AddStr_1=#pragma warning(disable : 4125)
REM set AddStr_2=#include "AllowWindowsPlatformTypes.h"
REM set AddStr_3=#include "HideWindowsPlatformTypes.h"
REM set EnterStr=echo.
REM del /f temp.txt
REM for /r %AbsDir_SimMsg% %%i in (*.pb.cc) do (
	REM echo %%i
	REM set /a Begine=0
	REM set /a End=0
	REM set /a n=0
 	REM for	/f "tokens=*" %%m in (%%i) do (
		REM set /a n+=1
		REM set Str=%%m
		REM setlocal ENABLEDELAYEDEXPANSION
		REM REM if "!Str:~0,8!" equ "#include" (
			REM REM if !Begine!==0 (
				REM REM echo !Str! 
				REM REM REM echo %AddStr_4%>>temp.txt
				REM REM REM echo %AddStr_0%>>temp.txt
				REM REM REM echo %AddStr_1%>>temp.txt
				REM REM REM echo %AddStr_2%>>temp.txt
				REM REM set /a Begine=1
			REM REM )
		REM REM )
		REM REM if "!Str:~0,8!" neq "#include" (
			REM REM if !Begine!==1 (
				REM REM if !End!==0 (
					REM REM echo !Str!
					REM REM REM echo %AddStr_3%>>temp.txt
					REM REM set /a End=1
				REM REM )
			REM REM )
		REM REM )
		REM endlocal
		REM call echo %%Str%%
		REM pause
		REM REM echo !Str!>>temp.txt
	REM )
	REM REM move /y temp.txt %%i
REM )

goto CopyMsg

:CopyMsg
echo CopyMsg..
echo %AbsDir_Display%Source\ProtoFiles

set MsgTargetPath=%AbsDir_Display%Source\Display\SimMsg\

xcopy %AbsDir_SimMsg%\basic.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\basic.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\controlSim.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\controlSim.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\grading.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\grading.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\header.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\header.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\laneMarks.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\laneMarks.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\localLocation.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\localLocation.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\location.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\location.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\moduleService.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\moduleService.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\obstacles.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\obstacles.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\planOutput.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\planOutput.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\planStatus.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\planStatus.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\traffic.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\traffic.pb.cc %MsgTargetPath% /E /R /Y

xcopy %AbsDir_SimMsg%\trajectory.pb.h %MsgTargetPath% /E /R /Y
xcopy %AbsDir_SimMsg%\trajectory.pb.cc %MsgTargetPath% /E /R /Y
goto BuildHadMap

:BuildHadMap
echo BuildHadMap..
goto CopyHadMap

:CopyHadMap
echo CopyHadMap..
rem include file
if exist %AbsDir_HadMap%\include (
	xcopy %AbsDir_HadMap%\include\*.* %FilePath_HadMap%\ThirdParty\HadMapSDK\Inc\ /E /R /Y
) else (
	echo %AbsDir_HadMap%\include is missing!
	goto Exit_HadMapError
)
rem lib
if exist %AbsDir_HadMap%\lib\x64\Release (
	xcopy %AbsDir_HadMap%\lib\x64\Release\*.* %FilePath_HadMap%\ThirdParty\HadMapSDK\Lib\x64\Release\ /E /R /Y
) else (
	echo \lib\x64\Release is missing!
	goto Exit_HadMapError
)
rem deps lib
if exist %AbsDir_HadMap%\deps\lib\x64 (
	xcopy %AbsDir_HadMap%\deps\lib\x64\*.* %FilePath_HadMap%\ThirdParty\HadMapSDK\Deps\lib\x64\ /E /R /Y
) else (
	echo \deps\lib\x64 is missing!
	goto Exit_HadMapError
)
rem deps dll
if exist %AbsDir_HadMap%\deps\dll\x64 (
	xcopy %AbsDir_HadMap%\deps\dll\x64\*.* %AbsDir_Display%\Binaries\Win64\ /E /R /Y
) else (
	echo \deps\dll\x64 is missing!
	goto Exit_HadMapError
)
goto Exit_Successed

rem =================================================================================
rem =================================================================================

rem Exit type:

:Exit_SimCoreError
echo SimCore Error
goto Exit_Failed

:Exit_DepsError
echo deps Error
goto Exit_Failed

:Exit_MsgError
echo sim_msg Error
goto Exit_Failed

:Exit_HadMapError
echo hadMap Error
goto Exit_Failed

:Exit_Failed
echo Failed
goto Exit
rem pause

:Exit_Successed
echo Successed
goto Exit
rem pause

:Exit
