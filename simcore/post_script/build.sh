#!/bin/bash

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
SCRIPT_ROOT="$(cd "$(dirname "$0")";pwd)"
SCRIPT_BUILD="$SCRIPT_ROOT/build"
SCRIPT_BUILD_MESSAGE="$SCRIPT_ROOT/sim_msg"
SCRIPT_BUILD_DIST="$SCRIPT_BUILD/bin"
PATHDIR_SOURCE_MESSAGE="$SCRIPT_ROOT/../../common/message"

# clean & mkdir
rm -rf "$SCRIPT_BUILD"
mkdir -p "$SCRIPT_BUILD"
rm -rf "$SCRIPT_BUILD_MESSAGE"
mkdir -p "$SCRIPT_BUILD_MESSAGE"

# 拷贝 message 并生成产物至 sim_msg
cp "$PATHDIR_SOURCE_MESSAGE"/*.proto "$SCRIPT_BUILD_MESSAGE/"
cp "$PATHDIR_SOURCE_MESSAGE/generate_python_by_grpcio.sh" "$SCRIPT_BUILD_MESSAGE/"

# 在 SCRIPT_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
$PYTHON -m venv "$SCRIPT_BUILD/myvenv"
source "$SCRIPT_BUILD/myvenv/bin/activate"
$PYTHON -m pip install --upgrade pip

# ============================ gen proto python ============================
# 安装第三方依赖库 & gen proto python
$PYTHON -m pip install grpcio-tools
cd "$SCRIPT_BUILD_MESSAGE"
bash generate_python_by_grpcio.sh

cd "$SCRIPT_ROOT"
# =============================== vissim 后处理脚本 ===============================
# 增量安装第三方依赖库 & 使用 PyInstaller 打包 Python 脚本
$PYTHON -m pip install glog pyinstaller
pyinstaller --onefile \
            --nowindow \
            --distpath="$SCRIPT_BUILD_DIST" \
            --specpath="$SCRIPT_BUILD" \
            -p "$SCRIPT_BUILD_MESSAGE/build" \
            -F "$SCRIPT_ROOT/pb_save_agent.py" \
            -n "pb_save_agent"

# =============================== grading 后处理脚本 ===============================
# 增量安装第三方依赖库 & 使用 PyInstaller 打包 Python 脚本
$PYTHON -m pip install xlsxwriter xlrd
pyinstaller --onefile \
            --nowindow \
            --distpath="$SCRIPT_BUILD_DIST" \
            --specpath="$SCRIPT_BUILD" \
            -p "$SCRIPT_BUILD_MESSAGE/build" \
            -p "$SCRIPT_ROOT/data_process" \
            -F "$SCRIPT_ROOT/pb_process.py" \
            -n "post_process"

# 取消激活虚拟环境
deactivate

# Change the working directory back to the original directory where the script was run
cd "$SCRIPT_ROOT"
