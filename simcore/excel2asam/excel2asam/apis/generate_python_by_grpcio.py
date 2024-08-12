#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file generate_python_by_grpcio.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-08-01
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import re
import subprocess
from pathlib import Path

# 1. 获取当前脚本所在的目录, 使用 pathlib 库
pathdir = Path(__file__).resolve().parent

# 2. 查找所有的 proto 文件
pathfiles = list(pathdir.glob("*.proto"))
# 记录所有proto文件名
proto_names = [f.stem for f in pathfiles]

# 3. 定义生成产物文件夹目录
FOLDER_NAME = "apis"

pathdir_output = pathdir
for file in pathdir_output.iterdir():
    # 如果它是一个文件并且文件名包含'_pb2.py'
    if file.is_file() and "_pb2.py" in file.name:
        # 删除文件
        file.unlink()

for pathfile in pathfiles:
    PATHFILE_OUT = str(pathdir_output / f"{pathfile.stem}_pb2.py")
    # 使用 grpc_tools.protoc 生成 Python 格式的产物
    # 注意这里使用了 pathlib 的语法
    subprocess.run(
        [
            "python3",
            "-m",
            "grpc_tools.protoc",
            f"--proto_path={pathdir}",
            f"--python_out={pathdir_output}",
            # f"--grpc_python_out={pathdir_output}",
            str(pathfile),
        ],
        check=True,
    )

    # 5. 修改生成的产物中, import proto文件名需要改为 import apis.proto文件名
    # 注意这里使用了 pathlib 的语法
    with open(PATHFILE_OUT, "r+") as f:
        # 读取文件内容
        content = f.read()

        # 在文件开始第一个非注释行前增加一行注释
        content = re.sub(r"(\n\s*[^#\n].*)", r"\1\n# pylint: skip-file\n# flake8: noqa", content, count=1)

        # 使用正则表达式匹配 import 语句中的文件名
        PATTERN = r"^import (" + "|".join(proto_names) + r")(_pb2)?"
        matches = re.findall(PATTERN, content, flags=re.MULTILINE)
        for match in matches:
            original_import = match[0] + (match[1] or "")
            new_import = f"{FOLDER_NAME}.{match[0]}{'_pb2' if match[1] else ''}"
            if original_import != new_import:
                # 修改 import 语句中的文件名
                content = content.replace(f"import {original_import}", f"import {new_import}")

        # 把修改后的内容写回文件
        f.seek(0)
        f.write(content)
        f.truncate()
