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

@REM define
set "SCRIPT_ROOT=%~dp0"
set "SCRIPT_ROOT=%SCRIPT_ROOT:~0,-1%"
set "SCRIPT_BUILD=%SCRIPT_ROOT%\build"
set "SCRIPT_BUILD_MESSAGE=%SCRIPT_ROOT%\sim_msg"
set "SCRIPT_BUILD_DIST=%SCRIPT_BUILD%\bin\Release"
set "PATHDIR_SOURCE_MESSAGE=%SCRIPT_ROOT%\..\..\common\message"

@REM clean & mkdir
if exist "%SCRIPT_BUILD%" rmdir /s /q "%SCRIPT_BUILD%"
mkdir "%SCRIPT_BUILD%"
if exist "%SCRIPT_BUILD_MESSAGE%" rmdir /s /q "%SCRIPT_BUILD_MESSAGE%"
mkdir "%SCRIPT_BUILD_MESSAGE%"

@REM 拷贝 message 并生成产物至 sim_msg
xcopy "%PATHDIR_SOURCE_MESSAGE%\*.proto" "%SCRIPT_BUILD_MESSAGE%" /y
xcopy "%PATHDIR_SOURCE_MESSAGE%\generate_python_by_grpcio.bat" "%SCRIPT_BUILD_MESSAGE%" /y

@REM 在 SCRIPT_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
%PYTHON% -m venv "%SCRIPT_BUILD%\myvenv"
call "%SCRIPT_BUILD%\myvenv\Scripts\activate"
call %PYTHON% -m pip install --upgrade pip

@REM ============================ gen proto python ============================
@REM 安装第三方依赖库 & gen proto python
call %PYTHON% -m pip install grpcio-tools
pushd "%SCRIPT_BUILD_MESSAGE%"
call generate_python_by_grpcio.bat
popd

@REM =================== gen vissim related executable file ===================
@REM 增量安装第三方依赖库 & 使用 PyInstaller 打包 Python 脚本
call %PYTHON% -m pip install glog pyinstaller
call pyinstaller --onefile ^
                 --nowindow ^
                 --distpath="%SCRIPT_BUILD_DIST%" ^
                 --specpath="%SCRIPT_BUILD%" ^
                 -p "%SCRIPT_BUILD_MESSAGE%\build" ^
                 -F "%SCRIPT_ROOT%\pb_save_agent.py" ^
                 -n pb_save_agent

@REM =================== gen grading related executable file ===================
@REM 增量安装第三方依赖库 & 使用 PyInstaller 打包 py 脚本
call %PYTHON% -m pip install xlsxwriter xlrd
call pyinstaller --onefile ^
                 --nowindow ^
                 --distpath="%SCRIPT_BUILD_DIST%" ^
                 --specpath="%SCRIPT_BUILD%" ^
                 -p "%SCRIPT_BUILD_MESSAGE%\build" ^
                 -p "%SCRIPT_ROOT%\data_process" ^
                 -F "%SCRIPT_ROOT%\pb_process.py" ^
                 -n post_process

@REM 取消激活虚拟环境
call deactivate

@REM Change the working directory back to the original directory where the script was run
cd "%SCRIPT_ROOT%"

endlocal
