#!/bin/bash

# 检查 python 命令是否存在
if command -v python >/dev/null 2>&1; then
    PYTHON=python
else
    # 如果 python 命令不存在，检查 python3 命令是否存在
    if command -v python3 >/dev/null 2>&1; then
        PYTHON=python3
    else
        # 如果 python 和 python3 命令都不存在，显示错误信息并退出脚本
        echo "Error: Python 3.x not found. Please install Python 3.x or modify script to use appropriate command." >&2
        exit 1
    fi
fi
echo "Using Python: $PYTHON"

# Set DIR
MSG_DIR=$(dirname $0)
PATHDIR_BUILD="$MSG_DIR/build"

# clean & mkdir
rm -rf $PATHDIR_BUILD
mkdir -p $PATHDIR_BUILD

#  Gen by grpc_tools.protoc
for f in $MSG_DIR/*.proto
do
  $PYTHON -m grpc.tools.protoc -I=$MSG_DIR --python_out=$PATHDIR_BUILD $f
done
