@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Start compiling
call :build_project "framework" "build.bat"
call :build_project "arbitrary" "build.bat"
call :build_project "envpb" "build.bat"
call :build_project "excel2asam" "build.bat"
call :build_project "grading" "build.bat"
call :build_project "lanemark_detector" "build.bat"
call :build_project "map_server" "build.bat"
call :build_project "traffic" "build.bat"
call :build_project "perfect_planning" "build.bat"
call :build_project "perfect_control" "build.bat"
call :build_project "protobuf_log" "build.bat"
call :build_project "sensors/imu_gps" "build.bat"
call :build_project "sensors/radar" "build.bat"
call :build_project "sensors/sensor_truth" "build.bat"
call :build_project "sensors/sim_label" "build.bat"
call :build_project "sensors/v2x" "build.bat"
call :build_project "post_script" "build.bat"
call :build_project "vehicle_dynamics" "build.bat"
exit /b

:build_project
    set "project=%~1"
    set "build_script=%~2"
    echo "=== Begin build %project%"
    @REM Go to the project directory
    pushd "%project%"
    call %build_script%
    @REM Return to the original directory
    popd
    echo "=== End build %project%"
goto :eof
