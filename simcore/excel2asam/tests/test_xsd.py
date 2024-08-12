#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file test_xsd.py
# * \brief validate xml meet xsd rule.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import datetime
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass
from enum import Enum, auto
from pathlib import Path

import pytest
import xmlschema

# import allure


class ValidationSuffix(Enum):
    """Enum for Suffix"""

    UNKNOWN = auto()
    XOSC = auto()
    XODR = auto()
    XML = auto()


@dataclass(order=True)
class XmlValidation:
    def __post_init__(self):
        pathdir_cur = Path(__file__).resolve().parent
        self.xsds = self._load_xsds(pathdir_cur / "schemas")

    def _load_xsds(self, pathdir_xsds: Path, suffix: str = ".xsd") -> dict:
        """Load all xsd files in the given pathdir_xsds."""
        if not isinstance(pathdir_xsds, Path):
            raise ValueError("Input pathdir_xsds must be a pathlib.Path object.")

        xsds = {}
        for item in pathdir_xsds.iterdir():
            # 对于文件夹, key 为文件夹名称, value 为其下 xsd 后缀且包含 "core" 的文件路径
            if item.is_dir():
                core_files = list(item.rglob(f"*core*{suffix}"))
                if core_files:
                    xsds[item.stem] = core_files[0]
                else:
                    xsds[item.stem] = item
            # 对于 xsd 后缀文件, key 为文件的名字, value 为文件的路径
            elif item.is_file() and item.suffix == suffix:
                xsds[item.stem] = item

        return xsds

    def _dentifying_suffix(self, pathfile_xml: Path):
        # 获得扩展名并去掉"."，字母全部大写
        suffix_name = pathfile_xml.suffix.lstrip(".").upper()

        suffix = ValidationSuffix.__members__.get(suffix_name, ValidationSuffix.UNKNOWN)

        return suffix != ValidationSuffix.UNKNOWN

    def _choose_xsd(self, tree: ET.ElementTree) -> Path:
        default = ""
        name = default

        root = tree.getroot()

        # 对于 OpenDRIVE
        for header in root.findall("header"):
            name = "OpenDRIVE"
            rev_major = header.get("revMajor", default)
            rev_minor = header.get("revMinor", default)

        # 对于 OpenSCENARIO
        for file_header in root.findall("FileHeader"):
            name = "OpenSCENARIO"
            rev_major = file_header.get("revMajor", default)
            rev_minor = file_header.get("revMinor", default)

        xsd_key = f"{name}_{rev_major}_{rev_minor}"

        return self.xsds[xsd_key]

    def xml_syntax(self, pathfile_xml: Path):
        try:
            # XML文档的结构良好性验证
            self.tree = ET.parse(pathfile_xml)
        except ET.ParseError as e:
            raise AssertionError(f"XML structure error: {e}")

    def xsd_syntax(self):
        try:
            # XML文档的良构性验证
            pathfile_xsd = self._choose_xsd(self.tree)
            xsd = xmlschema.XMLSchema(pathfile_xsd)
            xsd.validate(self.tree)
        except xmlschema.XMLSchemaValidatorError as e:
            raise AssertionError(f"XSD validation error: {e}")

    def validate(self, pathfile_xml: Path):
        if not self._dentifying_suffix(pathfile_xml):
            raise AssertionError(f"Unknown suffix: only support {ValidationSuffix._member_names_}")

        self.xml_syntax(pathfile_xml)

        self.xsd_syntax()


# pytest 在整个测试会话期间只创建一次 vtor fixture 实例
@pytest.fixture(scope="session")
def vtor():
    return XmlValidation()


def pytest_generate_tests(metafunc):
    """通过测试方法的名字, 获得对应的参数 pathfile 信息"""

    def get_pathfiles(pathdir: Path, suffix: str = ".xml"):
        """通过后缀得到所给的文件夹下所有符合后缀条件的文件"""
        if "." not in suffix:
            suffix = f".{suffix}"

        return sorted(list(pathdir.rglob(f"*{suffix}")))

    # 从参数 --f 中获得需要检查的文件夹路径
    pathdir_input = Path(metafunc.config.getoption("--f"))

    # based on the test method's name, provide the params list from the correct file type
    pathfiles = []
    if "xosc" in metafunc.function.__name__:
        pathfiles = get_pathfiles(pathdir_input, ".xosc")
    elif "xodr" in metafunc.function.__name__:
        pathfiles = get_pathfiles(pathdir_input, ".xodr")

    metafunc.parametrize("pathfile", pathfiles, ids=lambda x: x.name)


# @allure.story("test_valid_xosc")
def test_valid_xosc(pathfile: Path, vtor):
    vtor.validate(pathfile)


# @allure.story("test_valid_xodr")
def test_valid_xodr(pathfile: Path, vtor):
    vtor.validate(pathfile)


def main():
    # Preparation variables
    pathfile_cur = Path(__file__).resolve()

    # Set report name by time
    now = datetime.datetime.now()
    formatted_time = now.strftime("%Y-%m-%d_%H-%M-%S")
    filename_report = f"OpenSCENARIO_OpenDRIVE_Syntax_Check_Report_{formatted_time}.html"

    # 获取参数 --f, 检查其后是否包含内容
    f = None
    for arg in sys.argv[1:]:
        if arg.startswith("--f="):
            f = arg  # .split("=")[1]
            break

    if f is None:
        print("错误: 请提供 --f 参数")
        print("使用方法: python3 test_xsd.py --f=path/to/input")
        sys.exit(1)

    pathdir_input = Path(f.split("=")[1])  # 提取路径

    pytest.main(
        [
            # 并发执行
            "-n",
            "auto",
            # 输出每个测试用例的详细信息
            "-v",
            # 输出 html 报告
            f"--html={pathdir_input / filename_report}",
            "--self-contained-html",
            # # 输出 allure 报告, 暂不支持 TODO
            # "--alluredir",
            # f"--alluredir={pathdir_input / filename_report}",
            # 当前 py 代码
            pathfile_cur,
            # 参数
            f,
        ]
    )


if __name__ == "__main__":
    main()
