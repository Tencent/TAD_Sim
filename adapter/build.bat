@echo off

@REM Get system variables: VCPKG_ROOT
if defined VCPKG_ROOT (
    @REM echo VCPKG_ROOT: %VCPKG_ROOT%
) else (
    echo Please set system variables: VCPKG_ROOT
    pause
)

@REM Start compiling
call :build_project "osi" "build.bat"
exit /b

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
