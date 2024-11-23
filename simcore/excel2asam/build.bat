@echo off

setlocal enabledelayedexpansion

@REM 检查 python 命令是否存在
where /q python
IF ERRORLEVEL 1 (
    @REM 如果 python 命令不存在, 检查 python3 命令是否存在
    where /q python3
    IF ERRORLEVEL 1 (
        @REM 如果 python 和 python3 命令都不存在, 显示错误信息并退出脚本
        echo Error: Python 3.x not found. Please install Python 3.x or modify script to use appropriate command.
        exit /b
    ) ELSE (
        SET "PYTHON=python3"
    )
) ELSE (
    SET "PYTHON=python"
)
echo "Using Python: %PYTHON%"

@REM
set "BINARY_NAME=*.dll"

@REM define
set "EXCEL2ASAM_ROOT=%~dp0"
set "EXCEL2ASAM_ROOT=%EXCEL2ASAM_ROOT:~0,-1%"
set "EXCEL2ASAM_PROJECT=%EXCEL2ASAM_ROOT%\excel2asam"
set "EXCEL2ASAM_PROJECT_BINARY=%EXCEL2ASAM_PROJECT%\map\lib"
set "EXCEL2ASAM_BUILD=%EXCEL2ASAM_ROOT%\build"
set "EXCEL2ASAM_BUILD_DIST=%EXCEL2ASAM_BUILD%\bin\Release"

@REM clean & mkdir
if exist "%EXCEL2ASAM_BUILD%" rmdir /s /q "%EXCEL2ASAM_BUILD%"
mkdir "%EXCEL2ASAM_BUILD%"
if exist "%EXCEL2ASAM_PROJECT_BINARY%" rmdir /s /q "%EXCEL2ASAM_PROJECT_BINARY%"
mkdir "%EXCEL2ASAM_PROJECT_BINARY%"

@REM @REM 编译 map_sdk_py 并移动产物至指定位置
@REM cd "%EXCEL2ASAM_ROOT%\map_sdk_py"
@REM call build.bat
@REM cd "%EXCEL2ASAM_ROOT%"
@REM xcopy "%EXCEL2ASAM_ROOT%\map_sdk_py\build\bin\Release\*" "%EXCEL2ASAM_PROJECT_BINARY%\" /y /i /e
xcopy "%EXCEL2ASAM_ROOT%\map_sdk_py\lib_win_py3118\*" "%EXCEL2ASAM_PROJECT_BINARY%\" /y /i /e

@REM 遍历指定目录及其子目录，只添加 BINARY_NAME 文件
set "ADD_BINARY_ARGS="
for /r "%EXCEL2ASAM_PROJECT_BINARY%" %%f in ("%BINARY_NAME%") do (
    set "ADD_BINARY_ARGS=!ADD_BINARY_ARGS! --add-binary=%%f:map\lib"
)

@REM 在 EXCEL2ASAM_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
%PYTHON% -m venv "%EXCEL2ASAM_BUILD%\myvenv"
call "%EXCEL2ASAM_BUILD%\myvenv\Scripts\activate"
call %PYTHON% -m pip install --upgrade pip

@REM 安装第三方依赖库 & 使用 PyInstaller 打包 py 脚本
call %PYTHON% -m pip install pyinstaller
call %PYTHON% -m pip install -r requirements.txt
call pyinstaller --onefile ^
                 --nowindow ^
                 --distpath="%EXCEL2ASAM_BUILD_DIST%" ^
                 --specpath="%EXCEL2ASAM_BUILD%" ^
                 --add-data="%EXCEL2ASAM_PROJECT%\settings.toml:." ^
                 %ADD_BINARY_ARGS% ^
                 -p "%EXCEL2ASAM_ROOT%\excel2asam" ^
                 -F "%EXCEL2ASAM_ROOT%\excel2asam\producer.py" ^
                 -n "excel2asam"

@REM 取消激活虚拟环境
call deactivate

@REM Change the working directory back to the original directory where the script was run
cd "%EXCEL2ASAM_ROOT%"

endlocal
