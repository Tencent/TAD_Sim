#!/bin/bash

set -e

# 检查 python3 命令是否存在
if command -v python3 >/dev/null 2>&1; then
    PYTHON=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON=python
    # Check if python is actually python 3
    if ! $PYTHON -c 'import sys; exit(0 if sys.version_info.major == 3 else 1)'; then
       echo "Error: 'python' command found but it is not Python 3.x." >&2
       exit 1
    fi
else
    echo "Error: Python 3.x not found. Please install Python 3.x." >&2
    exit 1
fi
echo "Using Python: $PYTHON"

#
BINARY_NAME="*.so*"

# define
echo "DEBUG: BASH_SOURCE=${BASH_SOURCE[0]}"
echo "DEBUG: 0=$0"
echo "DEBUG: PWD=$(pwd)"
EXCEL2ASAM_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
echo "DEBUG: EXCEL2ASAM_ROOT=$EXCEL2ASAM_ROOT"
EXCEL2ASAM_PROJECT="$EXCEL2ASAM_ROOT/excel2asam"
EXCEL2ASAM_PROJECT_BINARY="$EXCEL2ASAM_PROJECT/map/lib"
EXCEL2ASAM_BUILD="$EXCEL2ASAM_ROOT/build"
EXCEL2ASAM_BUILD_DIST="$EXCEL2ASAM_BUILD/bin"

# clean & mkdir
rm -rf "$EXCEL2ASAM_BUILD"
mkdir -p "$EXCEL2ASAM_BUILD"
rm -rf "$EXCEL2ASAM_PROJECT_BINARY"
mkdir -p "$EXCEL2ASAM_PROJECT_BINARY"

# 编译 map_sdk_py 并移动产物至指定位置
cd "$EXCEL2ASAM_ROOT/map_sdk_py"
bash "build.sh"
cd "$EXCEL2ASAM_ROOT"
cp -rf "$EXCEL2ASAM_ROOT/map_sdk_py/build/lib"/* "$EXCEL2ASAM_PROJECT_BINARY/"

# 遍历指定目录及其子目录，只添加 BINARY_NAME 文件
ADD_BINARY_ARGS=""
for file in $(find "$EXCEL2ASAM_PROJECT_BINARY" -type f -name "$BINARY_NAME"); do
  ADD_BINARY_ARGS="$ADD_BINARY_ARGS --add-binary=$file:map/lib"
done

# 在 EXCEL2ASAM_BUILD 文件夹中创建虚拟环境 & 激活虚拟环境 & 升级 pip
$PYTHON -m venv "$EXCEL2ASAM_BUILD/myvenv"
source "$EXCEL2ASAM_BUILD/myvenv/bin/activate"
$PYTHON -m pip install --upgrade pip

# 安装第三方依赖库 & 使用 PyInstaller 打包 Python 脚本
# Check if pyinstaller module exists
if $PYTHON -c "import PyInstaller" >/dev/null 2>&1; then
    echo "Info: PyInstaller already installed. Skipping download."
else
    echo "Info: PyInstaller not found. Attempting install..."
    echo "Info: PyInstaller not found. Attempting install..."
    $PYTHON -m pip install --retries 0 --timeout 5 pyinstaller || echo "Warning: Failed to install pyinstaller (Offline). Build continues..."
fi

# Try to install requirements, but don't fail if offline and packages might be present
$PYTHON -m pip install --retries 0 --timeout 5 -r requirements.txt || echo "Warning: Failed to install requirements (Offline). Assuming pre-installed."
# Check if PyInstaller is available before running
if ! command -v pyinstaller >/dev/null 2>&1 && ! $PYTHON -m PyInstaller --version >/dev/null 2>&1; then
    echo "Warning: PyInstaller not found. Skipping excel2asam generation (offline mode)."
    deactivate
    exit 0
fi

pyinstaller --onefile \
            --nowindow \
            --distpath="$EXCEL2ASAM_BUILD_DIST" \
            --specpath="$EXCEL2ASAM_BUILD" \
            --add-data="$EXCEL2ASAM_PROJECT/settings.toml:." \
            $ADD_BINARY_ARGS \
            -p "$EXCEL2ASAM_PROJECT" \
            -F "$EXCEL2ASAM_PROJECT/producer.py" \
            -n "excel2asam"

# 取消激活虚拟环境
deactivate

# Change the working directory back to the original directory where the script was run
cd "$EXCEL2ASAM_ROOT"
