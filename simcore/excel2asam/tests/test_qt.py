#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file test_qt.py
# * \brief validate qt.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

from datetime import datetime
from pathlib import Path
import inspect

import pytest

from excel2asam.producer import LocalProducer
import excel2asam.exceptions as exp

# 当前文件夹路径
PATHDIR_CUR = Path(__file__).resolve().parent
# 测试数据路径
PATHDIR_TESTDATA = PATHDIR_CUR / "testdata"
PATHDIR_CATALOGS = PATHDIR_TESTDATA / "catalogs"
PATHDIR_MAPS = PATHDIR_TESTDATA / "maps"
# 获得当日时间以 年月日 格式
TIMEDATE = datetime.now().strftime("%Y%m%d")


def _get_expected(filename: str) -> str:
    for name, obj in inspect.getmembers(exp, inspect.isclass):
        # 确保类是在 exp 模块中定义的
        if obj.__module__ != exp.__name__:
            continue

        #
        if name in filename:
            return name

    # 程序正常结束后, 需要
    return ""


def get_pathfiles_expecteds_by_suffixes_name(pathdir: Path, name: str, *suffixes: str) -> list:
    # Convert suffixes to tuples
    suffixes = tuple(f".{x}" if x[0] != "." else x for x in suffixes)

    if not pathdir.is_dir():
        print(f"{pathdir} is not a valid directory.")
        return []

    return [
        [f, _get_expected(f.stem)]
        for f in sorted(pathdir.rglob("*"))
        if f.is_file() and (f.suffix in suffixes) and (name in f.name)
    ]


@pytest.mark.parametrize(
    ("pathfile", "expected"),
    get_pathfiles_expecteds_by_suffixes_name(PATHDIR_TESTDATA, "", "xlsx", "xlsm"),
    ids=lambda x: x,
)
@pytest.mark.excel
def test_map_virtual_excel(pathfile, expected):
    # 创建实例, 初始化, 处理文件
    pder = LocalProducer(
        input_mode="excel",
        input_data=pathfile,
        pathdir_catalogs=[PATHDIR_CATALOGS],
        pathdir_hadmap=Path(""),
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )

    if expected:
        with pytest.raises(getattr(exp, expected)):
            pder.process()
    else:
        pder.process()


@pytest.mark.parametrize(
    ("pathfile", "expected"),
    get_pathfiles_expecteds_by_suffixes_name(PATHDIR_TESTDATA, "真实地图", "xlsx", "xlsm"),
    ids=lambda x: x,
)
@pytest.mark.excel
def test_map_real_excel(pathfile, expected):
    # 创建实例, 初始化, 处理文件
    pder = LocalProducer(
        input_mode="excel",
        input_data=pathfile,
        pathdir_catalogs=[PATHDIR_CATALOGS],
        pathdir_hadmap=PATHDIR_MAPS,
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )

    if expected:
        with pytest.raises(getattr(exp, expected)):
            pder.process()
    else:
        pder.process()


@pytest.mark.skip(reason="without feishu info")
@pytest.mark.feishu_bitable
def test_map_virtual_feishu_bitable():
    # 创建实例, 初始化, 处理文件
    pder = LocalProducer(
        input_mode="feishu_bitable",
        input_data="",
        pathdir_catalogs=[PATHDIR_CATALOGS],
        pathdir_hadmap=Path(""),
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )
    pder.process()
