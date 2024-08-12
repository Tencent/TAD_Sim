#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import platform
import subprocess
from pathlib import Path
from typing import Optional, Union

import pytest

PATHDIR_CUR_PY = Path(__file__).resolve().parent


class Base:
    def execute_run(self, cmd: Union[list, str], cwd: Optional[Path] = None):
        # 在 pathdir_repo 中执行给定的 git 命令
        print(" ".join(cmd))
        return subprocess.run(cmd, cwd=cwd, check=False, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def execute_run_safety(self, cmd: Union[list, str], cwd: Optional[Path] = None) -> bool:
        result = self.execute_run(cmd, cwd)

        # 命令执行失败时处理
        if result.returncode != 0:
            print(f"Error executing cmd: {result.stderr.decode()}, {cmd}")
            return False
        return True

    def get_input_pathfiles(self, pathdir: Path, suffix: str = "xml"):
        if "." not in suffix:
            suffix = f".{suffix}"

        return sorted(list(pathdir.rglob(f"*{suffix}")))


class TestCompilationProduct(Base):
    @staticmethod
    def pytest_generate_tests(metafunc):
        if "pathfile" in metafunc.fixturenames:
            test_instance = TestCompilationProduct()
            path_files = test_instance.get_input_pathfiles(PATHDIR_CUR_PY, "pblog")
            ids = [x.name for x in path_files]
            # metafunc就是当前测试函数对应的function object，metafunc.parametrize可以实现动态生成测试推理。
            metafunc.parametrize("pathfile", path_files, ids=ids)

    def __find_name(self, name) -> str:
        # 获取当前系统环境
        current_os = platform.system()
        print("当前系统环境:", current_os)

        # 根据当前系统环境获取虚拟环境的激活指令
        if current_os == "Windows":
            name = f"{name}.exe"
        elif current_os == "Linux" or current_os == "Darwin":
            name = f"{name}"
        else:
            print("not support current os")

        return name

    def test_post_process(self, pathfile):
        pathfile_e = str(PATHDIR_CUR_PY.parent / f"dist/{self.__find_name('post_process')}")

        cmd = [pathfile_e, "--f", str(pathfile)]

        result = self.execute_run_safety(cmd)
        if not result:  # If cmd execution is not successful pytest will fail
            pytest.fail(f"Command execution failed: {cmd}")

    def test_pb_save_agent(self, pathfile):
        pathfile_e = str(PATHDIR_CUR_PY.parent / f"dist/{self.__find_name('pb_save_agent')}")
        cmd = [
            pathfile_e,
            "--pblog_path",
            str(pathfile),
            "--start_ms",
            "0",
            "--end_ms",
            "200",
            "--frames_path",
            "out.frames ",
            "--simrec_path",
            "out.simrec",
        ]

        result = self.execute_run_safety(cmd)
        if not result:  # If cmd execution is not successful pytest will fail
            pytest.fail(f"Command execution failed: {cmd}")
