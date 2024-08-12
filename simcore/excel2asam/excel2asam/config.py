#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file config.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import sys
from pathlib import Path

from dynaconf import Dynaconf

# 获取当前 py 文件路径以及当前命令行路径, 考虑通过 PyInstaller 编译为可执行程序后, __file__ 变量的行为变化问题
CURRENT_PATH_PY = Path(__file__).resolve().parent
CURRENT_PATH = Path.cwd()
if getattr(sys, "frozen", False):
    CURRENT_PATH_PY = Path(sys.argv[0]).resolve().parent
    CURRENT_PATH = Path(getattr(sys, "_MEIPASS", "."))


settings_files = {
    CURRENT_PATH_PY / "settings.toml",
    CURRENT_PATH_PY / ".secrets.toml",
    CURRENT_PATH / "settings.toml",
    CURRENT_PATH / ".secrets.toml",
}

#
settings = Dynaconf(envvar_prefix="DYNACONF", settings_files=list(settings_files))

# print(f"{settings.to_dict() = }")

# `envvar_prefix` = export envvars with `export DYNACONF_FOO=bar`.
# `settings_files` = Load these files in the order.
