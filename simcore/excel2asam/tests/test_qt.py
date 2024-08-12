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

import pytest

from excel2asam.config import settings
from excel2asam.parser import ExcelParserFactory, FeishuBittableParserFactory
from excel2asam.producer import LocalProducerFactory

# 当前文件夹路径
PATHDIR_CUR = Path(__file__).resolve().parent
# 测试数据路径
PATHDIR_TESTDATA = PATHDIR_CUR / "testdata"
# 获得当日时间以 年月日 格式
TIMEDATE = datetime.now().strftime("%Y%m%d")


def get_pathfiles_by_suffixes(pathdir: Path, *suffixes: str) -> list:
    """
    This function takes a directory path and any number of file suffixes as
    arguments, and returns a list of files in the directory (and its subdirectories)
    that end with any of the given suffixes.

    Args:
        pathdir (Path): The path of the directory to search for the files.
        *suffixes (str): Any number of suffixes to filter the files.

    Returns:
        list: A list of files (including the full path), ordered by the file name,
        that end with any of the given suffixes.

    Raises:
        Exception: If the provided pathdir is not a valid directory.
    """
    # Convert suffixes to tuples
    suffixes = tuple(f".{x}" if x[0] != "." else x for x in suffixes)

    if not pathdir.is_dir():
        raise Exception(f"{pathdir} is not a valid directory.")

    # Use rglob for recursive search and sort the result for preserving order
    return [f for f in sorted(pathdir.rglob("*")) if f.is_file() and f.suffix in suffixes]


@pytest.mark.parametrize("pathfile", get_pathfiles_by_suffixes(PATHDIR_TESTDATA, "xlsx", "xlsm"), ids=lambda x: x.name)
def test_map_virtual_excel(pathfile):
    # 创建实例, 初始化, 处理文件
    pder = LocalProducerFactory(
        parser_factory=ExcelParserFactory(pathfile),
        pathdir_catalog=PATHDIR_CUR.parent / "catalogs",
        pathdir_hadmap=Path(""),
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )
    pder.process()


@pytest.mark.parametrize("pathfile", get_pathfiles_by_suffixes(PATHDIR_TESTDATA, "xlsx", "xlsm"), ids=lambda x: x.name)
def test_map_real_excel(pathfile):
    # 创建实例, 初始化, 处理文件
    pder = LocalProducerFactory(
        parser_factory=ExcelParserFactory(pathfile),
        pathdir_catalog=PATHDIR_CUR.parent / "catalogs",
        pathdir_hadmap=PATHDIR_TESTDATA / "map",
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )
    pder.process()


def test_map_virtual_feishu_tadsim(dtype: str):
    # 创建实例, 初始化, 处理文件
    pder = LocalProducerFactory(
        parser_factory=FeishuBittableParserFactory(
            app_id=settings.feishu.app_id,
            app_secret=settings.feishu.app_secret,
            app_token=settings.feishu.tadsim.app_token,
        ),
        pathdir_catalog=PATHDIR_CUR.parent / "catalogs",
        pathdir_hadmap=Path(""),
        pathdir_output=Path(""),
        task_generalized_id=1023,
    )
    pder.set_api()
    pder.process()
