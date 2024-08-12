@echo off

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
set "USER_GUIDELINES_ROOT=%~dp0"
set "USER_GUIDELINES_ROOT=%USER_GUIDELINES_ROOT:~0,-1%"
set "USER_GUIDELINES_BUILD=%USER_GUIDELINES_ROOT%\build"
set "PROJECT=desktop"
set "EDITION=standard"

@REM clean & mkdir
if exist "%USER_GUIDELINES_BUILD%" rmdir /s /q "%USER_GUIDELINES_BUILD%"
mkdir "%USER_GUIDELINES_BUILD%"

@REM 在 USER_GUIDELINES_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
%PYTHON% -m venv "%USER_GUIDELINES_BUILD%\myvenv"
call "%USER_GUIDELINES_BUILD%\myvenv\Scripts\activate"
call %PYTHON% -m pip install --upgrade pip

@REM 安装第三方依赖库
@REM 加入 exhale 后对于 Sphinx 有特定版本依赖, 和其它库导致版本冲突
@REM 此处实际验证后发现, 可以正常安装后升级 Sphinx 至最新,
@REM 即使不是要求的特定版本, 但是工作是正常
call %PYTHON% -m pip install --no-cache-dir breathe==4.35.0
call %PYTHON% -m pip install --no-cache-dir exhale==0.3.6
call %PYTHON% -m pip install --no-cache-dir myst-parser==2.0.0
call %PYTHON% -m pip install --no-cache-dir sphinx-rtd-theme==2.0.0
call %PYTHON% -m pip install --no-cache-dir sphinx-markdown-tables==0.0.17
call %PYTHON% -m pip install --no-cache-dir sphinxcontrib-mermaid==0.9.2
call %PYTHON% -m pip install --no-cache-dir Sphinx==7.2.6
call %PYTHON% -m pip install --no-cache-dir loguru
call %PYTHON% -m pip install --no-cache-dir pytest

@REM 运行生成文档
call %PYTHON% generate_doc.py -p %PROJECT% -e %EDITION%

@REM 取消激活虚拟环境
call deactivate

@REM Change the working directory back to the original directory where the script was run
cd "%USER_GUIDELINES_ROOT%"
