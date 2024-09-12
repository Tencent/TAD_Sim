#!/bin/bash

set -e

# 检查 python3 命令是否存在
if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
else
    # 如果 python3 命令不存在，检查 python 命令是否存在
    if command -v python >/dev/null 2>&1; then
        PYTHON=python
    else
        # 如果 python3 和 python 命令都不存在，显示错误信息并退出脚本
        echo "Error: Python 3.x not found. Please install Python 3.x or modify script to use appropriate command." >&2
        exit 1
    fi
fi
echo "Using Python: $PYTHON"


# define
USER_GUIDELINES_ROOT="$(cd "$(dirname "$0")";pwd)"
USER_GUIDELINES_BUILD="$USER_GUIDELINES_ROOT/build"
PROJECT="desktop"
EDITION="standard"

# clean & mkdir
rm -rf "$USER_GUIDELINES_BUILD"
mkdir "$USER_GUIDELINES_BUILD"

# 在 USER_GUIDELINES_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
$PYTHON -m venv "${USER_GUIDELINES_BUILD}/myvenv"
source "${USER_GUIDELINES_BUILD}/myvenv/bin/activate"
$PYTHON -m pip install --upgrade pip

# 安装第三方依赖库
# 加入 exhale 后对于 Sphinx 有特定版本依赖, 和其它库导致版本冲突
# 此处实际验证后发现, 可以正常安装后升级 Sphinx 至最新,
# 即使不是要求的特定版本, 但是工作是正常
$PYTHON -m pip install breathe==4.35.0
$PYTHON -m pip install exhale==0.3.6
$PYTHON -m pip install myst-parser==2.0.0
$PYTHON -m pip install sphinx-rtd-theme==2.0.0
$PYTHON -m pip install sphinx-markdown-tables==0.0.17
$PYTHON -m pip install sphinxcontrib-mermaid==0.9.2
$PYTHON -m pip install Sphinx==7.2.6
$PYTHON -m pip install loguru
$PYTHON -m pip install pytest

# 运行生成文档
$PYTHON generate_doc.py -p ${PROJECT} -e ${EDITION}

# 取消激活虚拟环境
deactivate

# Change the working directory back to the original directory where the script was run
cd "$USER_GUIDELINES_ROOT"
