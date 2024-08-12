#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file generate_doc.py
# * \brief TAD Sim User Guidelines Generation.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-03-15
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import argparse
import fileinput
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from pathlib import Path

from loguru import logger


@dataclass(order=True)
class ProjectEdition:
    """CommandLineArgumentValidator."""

    vaild_settings: dict = field(
        default_factory=lambda: {
            "desktop": ["standard"],
        }
    )

    edition_chn_mapping: dict = field(
        default_factory=lambda: {
            "standard": "标准版",
        }
    )

    def __post_init__(self):
        # 获取用户命令行输入
        self._set_args()
        # 校验输入信息
        self._validate()

    def _set_args(self):
        self.parser = argparse.ArgumentParser(description="TAD Sim User Guidelines Generation")
        self.parser.add_argument(
            "-p",
            "--project",
            type=str,
            help=f"TAD Sim 产品类型, 必须为 {list(self.vaild_settings.keys())} 之一",
            required=True,
            choices=list(self.vaild_settings.keys()),
        )
        self.parser.add_argument(
            "-e",
            "--edition",
            type=str,
            help=f"TAD Sim 的版本, {self.vaild_settings}",
            required=True,
        )
        self.args = self.parser.parse_args()

    def _validate(self):
        project = self.args.project
        edition = self.args.edition

        if edition not in self.vaild_settings[project]:
            self.parser.error(
                f"Invalid edition: {edition}. Valid options for {project} are: {self.vaild_settings[project]}"
            )

        logger.opt(lazy=True).info(f"User input: project={project}, edition={edition}")

    @property
    def edition_chn(self):
        return self.edition_chn_mapping[self.args.edition]


@dataclass(order=True)
class DocGenerator:
    name_build: str = field(default="build")
    name_doxyout: str = field(default="_doxyout")
    name_sphinx: str = field(default="_sphinx")
    name_static: str = field(default="_static")
    name_source: str = field(default="source")
    name_api: str = field(default="api")
    name_extra: str = field(default="extra.css")
    name_doxyfile: str = field(default="Doxyfile")
    name_conf: str = field(default="conf.py")
    prefix_project: str = field(default="user_guidelines")

    def __post_init__(self) -> None:
        # 定义各种文件夹路径
        self.root = Path(__file__).resolve().parent
        self.tadsim_docs_dir = self.root / self.name_build
        self.tadsim_docs_dir_doxygen = self.tadsim_docs_dir / self.name_doxyout
        self.tadsim_docs_dir_sphinx = self.tadsim_docs_dir / self.name_sphinx

        # 获得命令行参数, 所有的项目和版本名称应当统一转换成小写以避免大小写敏感对比混淆
        pe = ProjectEdition()
        args = pe.args
        self.edition = args.edition.lower()
        self.lower_project_name = args.project.lower()
        self.edition_chn = pe.edition_chn

    def _set_log_file(self, pathdir: Path, filename_log: str) -> None:
        """
        > Create log files

        Args:
          pathdir (Path): The path of the output directory
          filename_log (str): The name of the log file

        Returns:
          None
        """
        # 获取当前时间戳并格式化
        current_time = time.strftime("%Y%m%d-%H%M%S")
        pathfile_log = pathdir / f"{filename_log}-{current_time}.log"

        # 创建输出文件夹(检查文件是否存在,存在则删除重新创建)
        # 检查文件是否存在
        if pathfile_log.exists():
            pathfile_log.unlink()

        # 创建 log 文件
        logger.opt(lazy=True).add(pathfile_log, rotation="100 MB", encoding="utf-8")

    def _rmtree(self, pathdir: Path) -> None:
        """Windows, shutil.rmtree() 函数可能在尝试删除某些文件时遇到权限问题,
        为了解决这个问题, 您可以定义一个自定义的删除函数, 用于处理这些权限问题
        """

        def remove_readonly(func, p, _excinfo):
            p_pathdir = Path(p)
            if p_pathdir.exists():  # 检查路径是否存在
                p_pathdir.chmod(p_pathdir.stat().st_mode | 0o666)
                func(p)
            else:
                pass

        shutil.rmtree(str(pathdir), onerror=remove_readonly)
        logger.opt(lazy=True).info(f"Remove {pathdir}")

    def _setup(self):
        self.tadsim_docs_dir_doxygen.mkdir(parents=True, exist_ok=True, mode=0o755)
        self.tadsim_docs_dir_sphinx.mkdir(parents=True, exist_ok=True, mode=0o755)

        # 创建 log 文件
        self._set_log_file(pathdir=self.tadsim_docs_dir, filename_log="file")

    def _set_dist(self) -> None:
        # 去除非字母数字符号, 以避免引入预期之外的特殊字符
        sanitized_project_name = re.sub(r"\W+", "", self.lower_project_name)
        project = f"{self.prefix_project}_{sanitized_project_name}"

        # 创建一系列以项目和版本名称为路径的文件夹, 以便于管理存储数据
        self.tadsim_docs_dir_dist = self.tadsim_docs_dir / f"{project}_{self.edition}"

        if not self.tadsim_docs_dir_dist.exists():
            self.tadsim_docs_dir_dist.mkdir(parents=True, mode=0o755)

        # 设置原始 markdown 文件的路径
        self.source = self.root / f"{project}/{self.name_source}"

    def _preprocess(self, edition: str, pathdir: Path, suffix: str = ".md") -> None:
        logger.opt(lazy=True).info("Filtering ...")

        # 预先编译正则表达式
        sign_start = re.compile(r"<!--\s*ifconfig::\s*(\w+)\s*(not in|in)\s*\((.*?)\)\s*-->")
        sign_end = re.compile(r"<!--\s*end\s*-->")

        # 根据后缀名定义文件列表
        pathfiles = [str(path) for path in pathdir.glob(f"**/*{suffix}")]

        # 使用 fileinput.input() 函数读取多个文件
        with fileinput.input(files=pathfiles, inplace=True, backup=".bak", encoding="utf-8") as f:
            keep = True

            # 遍历行, 处理起始为 <!-- ifconfig:: --> 终止为 <!-- end --> 标记的行
            # 通过传入的 edition 判断是否包含其中的信息(支持 in 和 not in 的表达), 实现通过配置来控制文档生成
            # 最终覆盖原文件
            for line in f:
                # logger.opt(lazy=True).debug(f"{line}, {keep}")

                # 检查是否遇到 <!-- ifconfig:: --> 标记
                ifconfig_match = sign_start.match(line)
                end_match = sign_end.match(line)

                #
                if ifconfig_match:
                    operator = ifconfig_match.group(2)
                    editions = [edition.lower() for edition in ifconfig_match.group(3).replace(" ", "").split(",")]

                    # logger.opt(lazy=True).debug(f"ifconfig:: {edition} {operator} {editions}")
                    if operator == "in":
                        keep = edition in editions
                    elif operator == "not in":
                        keep = edition not in editions

                # 检查是否遇到 <!-- end --> 标记
                elif end_match:
                    keep = True
                    continue

                # logger.opt(lazy=True).debug(f"After keep: {keep}")

                # 根据 keep 和 是否为 ifconfig_match 和 sign_end 决定是否保留当前行
                if (keep) and (not ifconfig_match or not sign_end):
                    sys.stdout.write(line)

    def _run_doxygen(self) -> None:
        """
        sed -i "s|^OUTPUT_DIRECTORY.*|OUTPUT_DIRECTORY=${TADSIM_DOCS_DIR_DOXYGEN}|" Doxyfile
        doxygen Doxyfile
        """

        logger.opt(lazy=True).info("Run doxygen")

        #
        pathfile_doxyfile = self.root / self.name_doxyfile
        current_file_path = Path(__file__).resolve().parent
        relative_pathfile_doxyfile = str(pathfile_doxyfile.relative_to(current_file_path))
        logger.opt(lazy=True).debug(f"{relative_pathfile_doxyfile = }")

        # 定义原始的和目标的字符串
        original_string = r"^OUTPUT_DIRECTORY.*"
        target_string = f"OUTPUT_DIRECTORY       = {str(self.tadsim_docs_dir_doxygen)}"
        target_string = target_string.replace("\\", "/")

        # 使用 fileinput 模块操作文件
        for line in fileinput.input(str(pathfile_doxyfile), inplace=True, encoding="utf-8"):
            # Replace the target string
            line = re.sub(original_string, target_string, line, flags=re.MULTILINE)
            # Write the file out again
            sys.stdout.write(line)

        # execute the doxygen command
        logger.opt(lazy=True).debug("execute the doxygen command")
        result = subprocess.run(["doxygen", relative_pathfile_doxyfile], shell=True, capture_output=True, text=True)

        if result.returncode == 0:
            logger.opt(lazy=True).debug("finished execute the doxygen command.")
        else:
            logger.opt(lazy=True).error("Doxygen execution failed, Exit the program.")
            logger.opt(lazy=True).debug(f"{result.returncode = }")
            logger.opt(lazy=True).debug(f"{result.stdout = }")
            logger.opt(lazy=True).debug(f"{result.stderr = }")
            sys.exit(1)

    def _run_sphinx(self) -> None:
        """
        export TADSIM_DOCS_DIR_DOXYGEN="${TADSIM_DOCS_DIR_DOXYGEN}"
        export TADSIM_DOCS_DIR_EXHALE="${TADSIM_DOCS_DIR_EXHALE}"
        sphinx-build -b html "${TADSIM_DOCS_DIR_SPHINX}" "${tadsim_docs_dir_dist}/html"
        """

        logger.opt(lazy=True).info("Run sphinx-build")

        # Set environment variable
        os.environ["TADSIM_DOCS_DIR_DOXYGEN"] = str(self.tadsim_docs_dir_doxygen)
        os.environ["TADSIM_DOCS_DIR_EXHALE"] = str(self.tadsim_docs_dir_sphinx / self.name_api)
        os.environ["EDITION_CHN"] = self.edition_chn

        # Run Sphinx build using subprocess module
        subprocess.run(
            ["sphinx-build", "-b", "html", str(self.tadsim_docs_dir_sphinx), str(self.tadsim_docs_dir_dist / "html")]
        )

    def _copy_project_for_preprocess(self, source: Path, dist: Path) -> None:
        logger.opt(lazy=True).info("Copying ...")
        # Copy each item in the source directory to the target directory
        for item in source.iterdir():
            if item.is_dir():
                shutil.copytree(item, self.tadsim_docs_dir_sphinx / item.name)
            else:
                shutil.copy2(item, self.tadsim_docs_dir_sphinx)
        shutil.copy2(self.root / self.name_extra, self.tadsim_docs_dir_sphinx / self.name_static)
        shutil.copy2(self.root / self.name_conf, self.tadsim_docs_dir_sphinx)

    def cleanup(self):
        """Clean up temporary directories."""
        self._rmtree(self.tadsim_docs_dir)
        self._rmtree(self.tadsim_docs_dir_doxygen)
        self._rmtree(self.tadsim_docs_dir_sphinx)

    def process(self):
        # 清理环境, 设置 log
        logger.opt(lazy=True).info("Setup to clean & mkdir folders")
        self._setup()

        # 设置产物目录
        logger.opt(lazy=True).info("Set dist")
        self._set_dist()

        # 生成文档
        logger.opt(lazy=True).info("Generating docs ...")
        # 拷贝项目整体文件夹至生成目录
        self._copy_project_for_preprocess(self.source, self.tadsim_docs_dir_dist)

        # 预处理, 通过标记筛选文件内容
        self._preprocess(edition=self.edition, pathdir=self.tadsim_docs_dir_sphinx)

        # 运行 Doxygen
        self._run_doxygen()

        # 运行 Sphinx
        self._run_sphinx()


if __name__ == "__main__":
    doc = DocGenerator()
    doc.process()
