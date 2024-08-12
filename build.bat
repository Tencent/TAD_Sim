@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM ======  Setting build parmameters ======
set "TADSIM_ROOT=%~dp0"
set "TADSIM_ROOT=%TADSIM_ROOT:~0,-1%"
set "TADSIM_BUILD=%TADSIM_ROOT%\build"
set "TADSIM_BUILD_SERVICE=%TADSIM_BUILD%\service"
set "TADSIM_BUILD_SCENARIO=%TADSIM_BUILD%\scenario"

@REM ======  Clean ======
echo "=== Begin clean"
@REM for /r %%f in (*.bat) do (attrib +r +a "%%f")
for /r /d %%D in (build node_modules) do rmdir /s /q "%%D" 2>nul
IF EXIST "%TADSIM_BUILD%" rmdir /s /q "%TADSIM_BUILD%"
echo "=== End clean"

@REM ======  Start compiling ======
call :build_project "simapp" "build.bat"
call :build_project "common" "build.bat"
call :build_project "simcore" "build.bat"
call :build_project "co_simulation" "build.bat"
call :build_project "adapter" "build.bat"

goto :pack

:build_project
    set "project=%~1"
    set "build_script=%~2"
    echo "=== Begin build %project%"
    @REM Go to the project directory
    pushd "%project%"
    call "%build_script%"
    @REM Return to the original directory
    popd
    echo "=== End build %project%"
goto :eof

:pack
@REM ======  pack ======
echo "=== Begin pack"
@REM 前端迁移整合
echo "=== Begin pack simapp"
mkdir "%TADSIM_BUILD%"
copy "%TADSIM_ROOT%\simapp\desktop\web-config.json" "%TADSIM_BUILD%\" /y
copy "%TADSIM_ROOT%\simapp\desktop\electron-builder.yml" "%TADSIM_BUILD%\" /y
copy "%TADSIM_ROOT%\simapp\desktop\package.json" "%TADSIM_BUILD%\" /y

mkdir "%TADSIM_BUILD%\node_modules"
xcopy "%TADSIM_ROOT%\simapp\desktop\node_modules\*" "%TADSIM_BUILD%\node_modules\" /y /i /e

mkdir "%TADSIM_BUILD%\build"
xcopy "%TADSIM_ROOT%\simapp\desktop\build\*" "%TADSIM_BUILD%\build\" /y /i /e

mkdir "%TADSIM_BUILD%\build\electron\map-editor"
xcopy "%TADSIM_ROOT%\simapp\scene-editor\build\*" "%TADSIM_BUILD%\build\electron\" /y /i /e
xcopy "%TADSIM_ROOT%\simapp\map-editor\build\*" "%TADSIM_BUILD%\build\electron\map-editor\" /y /i /e
echo "=== End pack simapp"

@REM 后端与算法的迁移整合
echo "=== Begin pack service"
mkdir "%TADSIM_BUILD_SERVICE%"
copy "%TADSIM_ROOT%\simcore\framework\tools\pb_info.json" "%TADSIM_BUILD_SERVICE%\"  /y
copy "%TADSIM_ROOT%\simcore\grading\default_grading_kpi.json" "%TADSIM_BUILD_SERVICE%\" /y
copy "%TADSIM_ROOT%\datas\local_service.config.in.windows" "%TADSIM_BUILD_SERVICE%\local_service.config.in" /y
copy "%TADSIM_ROOT%\simcore\framework\cli\cli.exe" "%TADSIM_BUILD_SERVICE%\txsim-cli.exe" /y
copy "%TADSIM_ROOT%\simcore\framework\build\bin\Release\txsim-local-service.exe" "%TADSIM_BUILD_SERVICE%\" /y
copy "%TADSIM_ROOT%\simcore\framework\build\bin\Release\txsim-module-launcher.exe" "%TADSIM_BUILD_SERVICE%\" /y
xcopy "%TADSIM_ROOT%\simcore\framework\build\bin\Release\*.dll" "%TADSIM_BUILD_SERVICE%\" /y /i /e
copy "%TADSIM_ROOT%\simcore\framework\src\node_addon\build\Release\txsim-play-service.node" "%TADSIM_BUILD_SERVICE%\" /y
xcopy "%TADSIM_ROOT%\simcore\framework\src\node_addon\build\Release\*.dll" "%TADSIM_BUILD_SERVICE%\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\osi"
xcopy "%TADSIM_ROOT%\adapter\osi\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\osi\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\carsim"
xcopy "%TADSIM_ROOT%\co_simulation\carsim\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\carsim\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\arbitrary"
xcopy "%TADSIM_ROOT%\simcore\arbitrary\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\arbitrary\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\Catalogs"
xcopy "%TADSIM_ROOT%\simcore\catalogs\*" "%TADSIM_BUILD_SERVICE%\Catalogs\" /y /i /e
xcopy "%TADSIM_ROOT%\simcore\catalogs\Vehicles\default.xosc" "%TADSIM_BUILD_SERVICE%\" /y
@REM
mkdir "%TADSIM_BUILD_SERVICE%\envpb"
xcopy "%TADSIM_ROOT%\simcore\envpb\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\envpb\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\excel2asam"
xcopy "%TADSIM_ROOT%\simcore\excel2asam\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\excel2asam\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\grading"
xcopy "%TADSIM_ROOT%\simcore\grading\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\grading\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\lanemark_detector"
xcopy "%TADSIM_ROOT%\simcore\lanemark_detector\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\lanemark_detector\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\txSimService"
xcopy "%TADSIM_ROOT%\simcore\map_server\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\txSimService\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\perfect_control"
xcopy "%TADSIM_ROOT%\simcore\perfect_control\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\perfect_control\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\perfect_planning"
xcopy "%TADSIM_ROOT%\simcore\perfect_planning\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\perfect_planning\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\protobuf_log"
xcopy "%TADSIM_ROOT%\simcore\protobuf_log\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\protobuf_log\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\data\script\"
xcopy "%TADSIM_ROOT%\simcore\post_script\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\data\script\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\imu_gps"
xcopy "%TADSIM_ROOT%\simcore\sensors\imu_gps\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\imu_gps\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\radar"
xcopy "%TADSIM_ROOT%\simcore\sensors\radar\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\radar\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\sensor_truth"
xcopy "%TADSIM_ROOT%\simcore\sensors\sensor_truth\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\sensor_truth\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\sim_label"
xcopy "%TADSIM_ROOT%\simcore\sensors\sim_label\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\sim_label\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\v2x"
xcopy "%TADSIM_ROOT%\simcore\sensors\v2x\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\v2x\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\traffic"
xcopy "%TADSIM_ROOT%\simcore\traffic\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\traffic\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\vehicle_dynamics"
xcopy "%TADSIM_ROOT%\simcore\vehicle_dynamics\build\bin\Release\*" "%TADSIM_BUILD_SERVICE%\vehicle_dynamics\" /y /i /e
@REM
mkdir "%TADSIM_BUILD_SERVICE%\upgrade_tools"
copy "%TADSIM_ROOT%\simcore\framework\tools\run_upgrade.bat" "%TADSIM_BUILD_SERVICE%\upgrade_tools\run.bat" /y
copy "%TADSIM_ROOT%\simcore\framework\build\bin\Release\upgrade-db.exe" "%TADSIM_BUILD_SERVICE%\upgrade_tools\" /y
copy "%TADSIM_ROOT%\simcore\protobuf_log\upgrade_pblog.bat" "%TADSIM_BUILD_SERVICE%\upgrade_tools\" /y
@REM copy "%TADSIM_ROOT%\simcore\framework\tools\upgrade_sensor.exe" "%TADSIM_BUILD_SERVICE%\upgrade_tools\" /y
@REM copy "%TADSIM_ROOT%\simcore\framework\tools\upgrade_grading_report.exe" "%TADSIM_BUILD_SERVICE%\upgrade_tools\" /y
@REM copy "%TADSIM_ROOT%\simcore\framework\tools\upgrade_environment.exe" "%TADSIM_BUILD_SERVICE%\upgrade_tools\" /y
mkdir "%TADSIM_BUILD_SERVICE%\deps\win32\vc++"
copy "%TADSIM_ROOT%\tools\vcredist_x64_2022.exe" "%TADSIM_BUILD_SERVICE%\deps\win32\vc++\"  /y
echo "=== End pack service"

@REM 系统默认自带 scenario 的迁移整合
echo "=== Begin pack scenario"
mkdir "%TADSIM_BUILD_SCENARIO%"
xcopy "%TADSIM_ROOT%\datas\default\*" "%TADSIM_BUILD_SCENARIO%\" /y /i /e
echo "=== End pack scenario"

@REM ======  打包成 exe 应用 ======
echo "=== Begin gen"
cd %TADSIM_BUILD%
set electron_mirror=https://registry.npmmirror.com/-/binary/electron/
set electron_builder_binaries_mirror=https://mirrors.huaweicloud.com/electron-builder-binaries/
@REM @REM 修改版本号
@REM where /q python
@REM IF ERRORLEVEL 1 (
@REM     @REM 如果 python 命令不存在, 检查 python3 命令是否存在
@REM     where /q python3
@REM     IF ERRORLEVEL 1 (
@REM         @REM 如果 python 和 python3 命令都不存在, 显示错误信息并退出脚本
@REM         echo Warning: python & python3 not found. skip modify_version use default 2.0.0.
@REM         exit /b
@REM     ) ELSE (
@REM         python3 "%TADSIM_ROOT%\tools\modify_version.py"
@REM     )
@REM ) ELSE (
@REM     python "%TADSIM_ROOT%\tools\modify_version.py"
@REM )
@REM 编译, 最终结果为 build/release/tadsim-x.x.x.exe
call npm run release
echo "=== End gen"

@REM ======  拷贝 SDK 产物 ======
echo "=== Begin copy SDK zips"
copy "%TADSIM_ROOT%\simcore\grading\external_eval\txSimGradingSDK_windows.tar.gz" "%TADSIM_BUILD%\release\"  /y
copy "%TADSIM_ROOT%\simcore\framework\build\txSimSDK_windows.tar.gz" "%TADSIM_BUILD%\release\"  /y
copy "%TADSIM_ROOT%\common\map_sdk\hadmap.tar.gz" "%TADSIM_BUILD%\release\txSimMapSDK_windows.tar.gz"  /y
echo "=== End copy SDK zips"

exit /b
