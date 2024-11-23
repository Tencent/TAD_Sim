@echo off

@REM ======  Setting build parmameters ======
@REM 获取当前脚本所在目录并去掉路径末尾的反斜杠
set "TOOLS_ROOT=%~dp0"
set "TOOLS_ROOT=%TOOLS_ROOT:~0,-1%"
@REM 获取上一级目录
for %%A in ("%TOOLS_ROOT%") do set "TADSIM_ROOT=%%~dpA"
@REM 设置 TADSIM_ROOT 为上一级目录
set "TADSIM_ROOT=%TADSIM_ROOT:~0,-1%"
echo "TADSIM_ROOT: %TADSIM_ROOT%"
echo.

@REM ====== Clean simapp ======
echo "Clean simapp start..."
IF EXIST "%TADSIM_ROOT%\simapp\package-lock.json" del /f /q "%TADSIM_ROOT%\simapp\package-lock.json"
IF EXIST "%TADSIM_ROOT%\simapp\node_modules" rmdir /s /q "%TADSIM_ROOT%\simapp\node_modules"
@REM Clean simapp desktop
IF EXIST "%TADSIM_ROOT%\simapp\desktop\package-lock.json" del /f /q "%TADSIM_ROOT%\simapp\desktop\package-lock.json"
IF EXIST "%TADSIM_ROOT%\simapp\desktop\node_modules" rmdir /s /q "%TADSIM_ROOT%\simapp\desktop\node_modules"
IF EXIST "%TADSIM_ROOT%\simapp\desktop\build" rmdir /s /q "%TADSIM_ROOT%\simapp\desktop\build"
@REM Clean simapp map-editor
IF EXIST "%TADSIM_ROOT%\simapp\map-editor\package-lock.json" del /f /q "%TADSIM_ROOT%\simapp\map-editor\package-lock.json"
IF EXIST "%TADSIM_ROOT%\simapp\map-editor\node_modules" rmdir /s /q "%TADSIM_ROOT%\simapp\map-editor\node_modules"
IF EXIST "%TADSIM_ROOT%\simapp\map-editor\build" rmdir /s /q "%TADSIM_ROOT%\simapp\map-editor\build"
@REM Clean simapp scene-editor
IF EXIST "%TADSIM_ROOT%\simapp\scene-editor\package-lock.json" del /f /q "%TADSIM_ROOT%\simapp\scene-editor\package-lock.json"
IF EXIST "%TADSIM_ROOT%\simapp\scene-editor\node_modules" rmdir /s /q "%TADSIM_ROOT%\simapp\scene-editor\node_modules"
IF EXIST "%TADSIM_ROOT%\simapp\scene-editor\build" rmdir /s /q "%TADSIM_ROOT%\simapp\scene-editor\build"
echo "Clean simapp successfully."
echo.

@REM ====== Clean common ======
@REM Clean common map_sdk
echo "Clean common map_sdk start..."
IF EXIST "%TADSIM_ROOT%\common\map_sdk\hadmap.tar.gz" del /f /q "%TADSIM_ROOT%\common\map_sdk\hadmap.tar.gz"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\hadmap" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\hadmap"
@REM Clean common map_sdk datamodel
IF EXIST "%TADSIM_ROOT%\common\map_sdk\datamodel\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\datamodel\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\datamodel\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\datamodel\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\datamodel\*.dll" del "%TADSIM_ROOT%\common\map_sdk\datamodel\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\datamodel\*.lib" del "%TADSIM_ROOT%\common\map_sdk\datamodel\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\datamodel\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\datamodel\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\mapdb\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\mapdb\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\mapdb\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\mapdb\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\mapdb\*.dll" del "%TADSIM_ROOT%\common\map_sdk\mapdb\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\mapdb\*.lib" del "%TADSIM_ROOT%\common\map_sdk\mapdb\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\mapdb\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\mapdb\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_import\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_import\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_import\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_import\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_import\*.dll" del "%TADSIM_ROOT%\common\map_sdk\map_import\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_import\*.lib" del "%TADSIM_ROOT%\common\map_sdk\map_import\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_import\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_import\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\transmission\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\transmission\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\transmission\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\transmission\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\transmission\*.dll" del "%TADSIM_ROOT%\common\map_sdk\transmission\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\transmission\*.lib" del "%TADSIM_ROOT%\common\map_sdk\transmission\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\transmission\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\transmission\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_engine\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_engine\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_engine\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_engine\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_engine\*.dll" del "%TADSIM_ROOT%\common\map_sdk\map_engine\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_engine\*.lib" del "%TADSIM_ROOT%\common\map_sdk\map_engine\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\map_engine\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\map_engine\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\route_plan\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\route_plan\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\route_plan\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\route_plan\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\route_plan\*.dll" del "%TADSIM_ROOT%\common\map_sdk\route_plan\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\route_plan\*.lib" del "%TADSIM_ROOT%\common\map_sdk\route_plan\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\route_plan\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\route_plan\build"
@REM
IF EXIST "%TADSIM_ROOT%\common\map_sdk\routingmap\Release" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\routingmap\Release"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\routingmap\Debug" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\routingmap\Debug"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\routingmap\*.dll" del "%TADSIM_ROOT%\common\map_sdk\routingmap\*.dll"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\routingmap\*.lib" del "%TADSIM_ROOT%\common\map_sdk\routingmap\*.lib"
IF EXIST "%TADSIM_ROOT%\common\map_sdk\routingmap\build" rmdir /s /q "%TADSIM_ROOT%\common\map_sdk\routingmap\build"
echo "Clean common map_sdk successfully."
echo.
@REM Clean common message
echo "Clean common message start..."
IF EXIST "%TADSIM_ROOT%\common\message\build" rmdir /s /q "%TADSIM_ROOT%\common\message\build"
echo "Clean common message successfully."
echo.

@REM ====== Clean simcore ======
@REM Clean simcore arbitrary
echo "Clean simcore arbitrary start..."
IF EXIST "%TADSIM_ROOT%\simcore\arbitrary\build" rmdir /s /q "%TADSIM_ROOT%\simcore\arbitrary\build"
echo "Clean simcore arbitrary successfully."
echo.
@REM Clean simcore envpb
echo "Clean simcore envpb start..."
IF EXIST "%TADSIM_ROOT%\simcore\envpb\build" rmdir /s /q "%TADSIM_ROOT%\simcore\envpb\build"
echo "Clean simcore envpb successfully."
echo.
@REM Clean simcore excel2asam
echo "Clean simcore excel2asam start..."
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\build" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\build"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\map\lib" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\map\lib"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\map_sdk_py\build" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\map_sdk\build"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\map_sdk_py\tests\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\map_sdk_py\tests\__pycache__"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\apis\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\apis\__pycache__"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\generalize\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\generalize\__pycache__"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\map\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\map\__pycache__"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\openx\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\excel2asam\openx\__pycache__"
IF EXIST "%TADSIM_ROOT%\simcore\excel2asam\tests\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\excel2asam\tests\__pycache__"
echo "Clean simcore excel2asam successfully."
echo.
@REM Clean simcore framework
echo "Clean simcore framework start..."
IF EXIST "%TADSIM_ROOT%\simcore\framework\cli\cli.exe" del /f /q "%TADSIM_ROOT%\simcore\framework\cli\cli.exe"
IF EXIST "%TADSIM_ROOT%\simcore\framework\cli\go.sum" del /f /q "%TADSIM_ROOT%\simcore\framework\cli\go.sum"
IF EXIST "%TADSIM_ROOT%\simcore\framework\build" rmdir /s /q "%TADSIM_ROOT%\simcore\framework\build"
IF EXIST "%TADSIM_ROOT%\simcore\framework\src\node_addon\build" rmdir /s /q "%TADSIM_ROOT%\simcore\framework\src\node_addon\build"
IF EXIST "%TADSIM_ROOT%\simcore\framework\docs\api" rmdir /s /q "%TADSIM_ROOT%\simcore\framework\docs\api"
IF EXIST "%TADSIM_ROOT%\simcore\framework\docs\sphinx" rmdir /s /q "%TADSIM_ROOT%\simcore\framework\docs\sphinx"
echo "Clean simcore framework successfully."
echo.
@REM Clean simcore grading
echo "Clean simcore grading start..."
IF EXIST "%TADSIM_ROOT%\simcore\grading\build" rmdir /s /q "%TADSIM_ROOT%\simcore\grading\build"
IF EXIST "%TADSIM_ROOT%\simcore\grading\external_eval\txSimGradingSDK.tar.gz" del /f /q "%TADSIM_ROOT%\simcore\grading\external_eval\txSimGradingSDK.tar.gz"
echo "Clean simcore grading successfully."
echo.
@REM Clean simcore lanemark_detector
echo "Clean simcore lanemark_detector start..."
IF EXIST "%TADSIM_ROOT%\simcore\lanemark_detector\build" rmdir /s /q "%TADSIM_ROOT%\simcore\lanemark_detector\build"
echo "Clean simcore lanemark_detector successfully."
echo.
@REM Clean simcore map_server
echo "Clean simcore map_server start..."
IF EXIST "%TADSIM_ROOT%\simcore\map_server\service\service.exe" del /f /q "%TADSIM_ROOT%\simcore\map_server\service\service.exe"
IF EXIST "%TADSIM_ROOT%\simcore\map_server\service\go.sum" del /f /q "%TADSIM_ROOT%\simcore\map_server\service\go.sum"
IF EXIST "%TADSIM_ROOT%\simcore\map_server\build" rmdir /s /q "%TADSIM_ROOT%\simcore\map_server\build"
IF EXIST "%TADSIM_ROOT%\simcore\map_server\opendrive_io\build" rmdir /s /q "%TADSIM_ROOT%\simcore\map_server\opendrive_io\build"
IF EXIST "%TADSIM_ROOT%\simcore\map_server\hadmap_server\map_parser\build" rmdir /s /q "%TADSIM_ROOT%\simcore\map_server\hadmap_server\map_parser\build"
echo "Clean simcore map_server successfully."
echo.
@REM Clean simcore perfect_control
echo "Clean simcore perfect_control start..."
IF EXIST "%TADSIM_ROOT%\simcore\perfect_control\build" rmdir /s /q "%TADSIM_ROOT%\simcore\perfect_control\build"
echo "Clean simcore perfect_control successfully."
echo.
@REM Clean simcore perfect_planning
echo "Clean simcore perfect_planning start..."
IF EXIST "%TADSIM_ROOT%\simcore\perfect_planning\build" rmdir /s /q "%TADSIM_ROOT%\simcore\perfect_planning\build"
echo "Clean simcore perfect_planning successfully."
echo.
@REM Clean simcore post_script
echo "Clean simcore post_script start..."
IF EXIST "%TADSIM_ROOT%\simcore\post_script\build" rmdir /s /q "%TADSIM_ROOT%\simcore\post_script\build"
IF EXIST "%TADSIM_ROOT%\simcore\post_script\sim_msg" rmdir /s /q "%TADSIM_ROOT%\simcore\post_script\sim_msg"
IF EXIST "%TADSIM_ROOT%\simcore\post_script\data_process\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\post_script\data_process\__pycache__
IF EXIST "%TADSIM_ROOT%\simcore\post_script\tests\__pycache__" rmdir /s /q "%TADSIM_ROOT%\simcore\post_script\tests\__pycache__
echo "Clean simcore post_script successfully."
echo.
@REM Clean simcore protobuf_log
echo "Clean simcore protobuf_log start..."
IF EXIST "%TADSIM_ROOT%\simcore\protobuf_log\build" rmdir /s /q "%TADSIM_ROOT%\simcore\protobuf_log\build"
echo "Clean simcore protobuf_log successfully."
echo.
@REM Clean simcore sensors display
echo "Clean simcore sensors display start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\display\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\display\build"
echo "Clean simcore sensors display successfully."
echo.
@REM Clean simcore sensors imu_gps
echo "Clean simcore sensors imu_gps start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\imu_gps\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\imu_gps\build"
echo "Clean simcore sensors imu_gps successfully."
echo.
@REM Clean simcore sensors radar
echo "Clean simcore sensors radar start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\radar\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\radar\build"
echo "Clean simcore sensors radar successfully."
echo.
@REM Clean simcore sensors sensor_truth
echo "Clean simcore sensors sensor_truth start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\sensor_truth\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\sensor_truth\build"
echo "Clean simcore sensors sensor_truth successfully."
echo.
@REM Clean simcore sensors sim_label
echo "Clean simcore sensors sim_label start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\sim_label\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\sim_label\build"
echo "Clean simcore sensors sim_label successfully."
echo.
@REM Clean simcore sensors v2x
echo "Clean simcore sensors v2x start..."
IF EXIST "%TADSIM_ROOT%\simcore\sensors\v2x\build" rmdir /s /q "%TADSIM_ROOT%\simcore\sensors\v2x\build"
echo "Clean simcore sensors v2x successfully."
echo.
@REM Clean simcore traffic
echo "Clean simcore traffic start..."
IF EXIST "%TADSIM_ROOT%\simcore\traffic\app\version\version.h" del /f /q "%TADSIM_ROOT%\simcore\traffic\app\version\version.h"
IF EXIST "%TADSIM_ROOT%\simcore\traffic\build" rmdir /s /q "%TADSIM_ROOT%\simcore\traffic\build"
echo "Clean simcore traffic successfully."
echo.
@REM Clean simcore vehicle_dynamics
echo "Clean simcore vehicle_dynamics start..."
IF EXIST "%TADSIM_ROOT%\simcore\vehicle_dynamics\build" rmdir /s /q "%TADSIM_ROOT%\simcore\vehicle_dynamics\build"
echo "Clean simcore vehicle_dynamics successfully."
echo.

@REM ====== Clean co_simulation ======
echo "Clean co_simulation carsim start..."
@REM Clean co_simulation carsim
IF EXIST "%TADSIM_ROOT%\co_simulation\carsim\build" rmdir /s /q "%TADSIM_ROOT%\co_simulation\carsim\build"
echo "Clean co_simulation carsim successfully."
echo.

@REM ====== Clean adapter ======
@REM Clean adapter osi
echo "Clean adapter osi start..."
IF EXIST "%TADSIM_ROOT%\adapter\osi\build" rmdir /s /q "%TADSIM_ROOT%\adapter\osi\build"
echo "Clean adapter osi successfully."
echo.

@REM ====== Clean docs ======
@REM Clean docs user_guidelines
echo "Clean docs user_guidelines start..."
IF EXIST "%TADSIM_ROOT%\docs\user_guidelines\build" rmdir /s /q "%TADSIM_ROOT%\docs\user_guidelines\build"
echo "Clean docs user_guidelines successfully."
echo.

@REM ====== Clean build ======
@REM Clean build
echo "Clean build start..."
IF EXIST "%TADSIM_ROOT%\build" rmdir /s /q "%TADSIM_ROOT%\build"
echo "Clean build successfully."
echo.

@REM ====== Recovery git change by build ======
echo "Recovery git change by build start..."
git checkout -- "%TADSIM_ROOT%\simapp\map-editor\package.json"
git checkout -- "%TADSIM_ROOT%\simcore\map_server\service\go.mod"
git checkout -- "%TADSIM_ROOT%\simcore\perfect_planning\version\version.h"
echo "Recovery git change by build successfully."
echo.
