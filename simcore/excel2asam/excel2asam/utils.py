#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file utils.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import contextlib
import json
import subprocess
import time
from ast import literal_eval
from collections import defaultdict, deque
from dataclasses import dataclass
from functools import wraps
from pathlib import Path
from typing import Any, Callable, Dict, List, NamedTuple, Optional, Type, Union, cast

import excel2asam.exceptions as exp
import numpy as np
import pandas as pd
from dynaconf import Dynaconf
from loguru import logger

from excel2asam.config import settings
from excel2asam.map.lib.hadmap_py import Waypoint


def is_number(value: Any) -> bool:
    """
    判断给定的值是否为数字.

    Args:
        value (Any): 需要判断的值.

    Returns:
        bool: 如果给定的值是数字, 则返回True, 否则返回False.
    """
    with contextlib.suppress(ValueError):
        float(value)
        return True
    with contextlib.suppress(TypeError, ValueError):
        import unicodedata

        unicodedata.numeric(value)
        return True
    return False


def show_memory(unit="KB", threshold=1) -> None:
    """
    显示当前Python环境中全局变量的内存占用.

    Args:
        unit (str): 内存单位, 可选值为"B"（字节）、"KB"（千字节）、"MB"（兆字节）和"GB"（吉字节）, 默认为"KB".
        threshold (int): 内存占用阈值, 只有当变量的内存占用大于等于此阈值时才会显示, 默认为1.

    Returns:
        None
    """

    scale = {"B": 1, "KB": 1024, "MB": 1048576, "GB": 1073741824}[unit]
    for i in list(globals().keys()):
        memory = eval(f"getsizeof({i})") // scale
        if memory >= threshold:
            print(i, memory)


def check_add_suffix(pathfile: str, suffix: str) -> str:
    """
    > If the pathfile doesn't end with the suffix, add the suffix to the pathfile

    Args:
      pathfile (str): The path to the file you want to check.
      suffix (str): The suffix to add to the file name.

    Returns:
      The pathfile with the suffix added to the end of it.
    """
    # 如果路径文件不以指定的后缀结尾, 则添加指定的后缀
    if not pathfile.endswith(suffix):
        # 如果没有指定后缀, 则添加.后缀
        if "." not in suffix:
            suffix = f".{suffix}"
        # 添加指定的后缀
        pathfile += suffix
    # 返回添加后缀后的路径文件
    return pathfile


def value_dispatch(func):
    """
    根据第一个或第二个参数的值分发到不同的处理函数.

    Args:
        func (function): 默认处理函数, 当没有匹配到特定值的处理函数时, 会使用这个函数处理.

    Returns:
        function: 返回一个包装后的函数, 该函数可以根据第一个或第二个参数的值分发到不同的处理函数.
    """

    # 存储值与处理函数的映射关系
    registry = {}

    @wraps(func)
    def wrapper(*args, **kwargs):
        try:
            # 根据第一个或第二个参数找到对应的处理函数
            delegate = registry[args[1] if len(args) > 1 else args[0]]
        except KeyError:
            pass
        else:
            # 注意这里是使用 *args 和 **kwargs 来灵活处理函数参数
            # 调用找到的处理函数
            return delegate(*args, **kwargs)
        # 如果没有找到处理函数, 使用默认处理函数
        return func(*args, **kwargs)

    def register(value):
        """
        注册一个处理函数, 用于处理特定值的情况.

        Args:
            value: 需要处理的值

        Returns:
            function: 返回一个包装后的函数, 用于注册处理函数.
        """

        def wrap(func):
            if value in registry:
                raise ValueError(f"@value_dispatch: there is already a handler " f"registered for {value!r}")
            # 将值与处理函数的映射关系存储到registry中
            registry[value] = func
            return func

        return wrap

    def register_for_all(values):
        """
        注册一个处理函数, 用于处理多个值的情况.

        Args:
            values (list): 需要处理的值列表

        Returns:
            function: 返回一个包装后的函数，用于注册处理函数.
        """

        def wrap(func):
            for value in values:
                if value in registry:
                    raise ValueError(f"@value_dispatch: there is already a handler " f"registered for {value!r}")
                # 将值与处理函数的映射关系存储到registry中
                registry[value] = func
            return func

        return wrap

    # 给wrapper添加register方法，用于注册处理函数
    wrapper.register = register
    # 给wrapper添加register_for_all方法，用于注册处理函数
    wrapper.register_for_all = register_for_all
    return wrapper


def timer(func):
    """
    The `timer` function is a decorator that measures the execution time of a given function and prints it out.

    Args:
      func: The function that we want to time.

    Returns:
      The `timer` function returns the `wrapper` function, which is a decorated version of the original function passed
    as an argument to `timer`.
    """

    @wraps(func)
    def wrapper(*args, **kwargs):
        begin_time = time.perf_counter()
        res = func(*args, **kwargs)
        time_elapsed = time.perf_counter() - begin_time
        print(f"{func.__name__} | {time_elapsed} sec")
        return res

    return wrapper


def convert_string(s: str) -> Union[int, float, dict, list, str]:
    """
    尝试将字符串转换为整数、浮点数、字典或列表. 如果无法转换，则返回原始字符串.

    Args:
        s (str): 要转换的字符串.

    Returns:
        Union[int, float, dict, list, str]: 转换后的值，可以是整数、浮点数、字典、列表或原始字符串.
    """
    if not isinstance(s, str):
        return s

    # 去除字符串 s 的空格和回车
    s = s.strip()

    try:
        return int(s)
    except ValueError:
        try:
            return float(s)
        except ValueError:
            try:
                return json.loads(s)
            except ValueError:
                return s


def safe_literal_eval(index: str, string: str) -> Union[dict, list, str, int, float, pd.NA]:
    """
    尝试对字符串进行安全的字面值求值. 如果求值失败，记录错误并返回 pd.NA.

    Args:
        index (str): 求值字符串对应的 index 标记.
        string (str): 要进行字面值求值的字符串.

    Returns:
        Union[dict, list, str, int, float, pd.NA]: 求值后的值，可以是字典、列表、字符串、整数、浮点数或 pd.NA.
    """
    try:
        return literal_eval(string)
    except (ValueError, SyntaxError):
        logger.opt(lazy=True).error(f"Error in row {index}: {string}")
        return pd.NA


def deep_update_settings(source: Dynaconf, overrides: Dict) -> Dynaconf:
    """
    将 overrides 字典中的设置深度合并到 source 字典中.

    Args:
        source (Dynaconf): 要更新的源 Dynaconf 对象.
        overrides (Dict): 包含要覆盖的设置的字典.

    Returns:
        Dynaconf: 更新后的 Dynaconf 对象.
    """
    for key, value in overrides.items():
        if isinstance(value, dict):
            source[key] = deep_update_settings(source.get(key, {}), value)
        else:
            source[key] = value
    return source


def raise_error_reason_by_diff(cols1: list, cols2: list) -> None:
    """
    检查两个列表中是否存在差异列，如果存在，则抛出异常.

    Args:
        cols1 (list): 第一个列表.
        cols2 (list): 第二个列表.

    Returns:
        None: 如果没有差异列，则返回 None.

    Raises:
        ColumnsDefineNotFoundError: 如果存在差异列，则抛出此异常.
    """
    if reason := set(cols1).difference(set(cols2)):
        raise exp.ColumnsDefineNotFoundError(f"ColumnsDefineNotFoundError: Without columns of: {reason}")


def process_range(total: int, interval: int, ini: int, func: Callable[..., Any], *args: Any) -> None:
    if not isinstance(total, int) or total <= 0:
        raise ValueError("total must be a positive integer")
    if not isinstance(interval, int) or interval <= 0:
        raise ValueError("interval must be a positive integer")
    if not isinstance(ini, int) or ini < 0:
        raise ValueError("initial position must be a non-negative integer")

    total_iterations = (
        0 if (total <= 0 or interval <= 0) else (total - ini) // interval + ((total - ini) % interval != 0)
    )

    for i in range(total_iterations):
        start = max(ini + i * interval, 0)
        end = min(start + interval - 1, total)
        func(start, end, *args)


@dataclass(order=True)
class SceneFilter:
    """
    场景过滤器，用于处理场景数据.
    """

    def __post_init__(self):
        """
        初始化SceneFilter实例.
        """

        self.cols_common = [
            settings.sys.generalized.cols.funcid,
            settings.sys.generalized.cols.logicid,
            settings.sys.l0.cols.concreteid,
        ]
        self.invalids = pd.DataFrame(
            columns=[*self.cols_common, settings.sys.l0.cols.reason, settings.sys.l0.cols.assessment]
        )

    def process(
        self,
        df: pd.DataFrame,
        keep_cond: pd.Series,
        reason: str,
        assessment: str,
        exp_class: Optional[Type[Exception]] = None,
    ) -> pd.DataFrame:
        """
        处理场景数据，删除不符合条件的场景.

        Args:
            df (pd.DataFrame): 场景数据
            keep_cond (pd.Series): 保留条件
            reason (str): 删除场景的原因
            assessment (str): 评估结果
            exp_class (Optional[Type[Exception]], optional): 异常类, 当场景数据为空时抛出.默认为None.

        Returns:
            pd.DataFrame: 处理后的场景数据
        """
        if (~keep_cond).any():
            ids = df.loc[~keep_cond, self.cols_common]
            ids[settings.sys.l0.cols.reason] = reason
            ids[settings.sys.l0.cols.assessment] = assessment

            # 拼接到 invalids, 且重新设置索引从 1 开始
            self.invalids = self.invalids.merge(ids, how="outer")
            self.invalids.index = self.invalids.index + 1

            logger.opt(lazy=True).info(f"{settings.sys.l0.cols.reason}: {reason}, Delete invalid scenes: {len(ids)}")

            # 保留删选后的场景行
            df = df[keep_cond].reset_index(drop=True)

            # 异常处理, 检查场景数据是否为空
            if df.empty and exp_class is not None:
                raise exp_class(f"After {reason}, there is no valid scenedata")

        return df


@dataclass(order=True)
class DtypeConverter:
    """
    数据类型转换器, 用于将DataFrame中的列转换为最佳数据类型以减少内存占用.
    """

    df: pd.DataFrame

    def _get_best_int_dtypes(self, int_cols: pd.Index) -> dict:
        """
        将int列转换为最佳类型.

        Args:
            int_cols (pd.Index): int列的索引

        Returns:
            dict: 包含最佳数据类型的字典，键为列名，值为最佳数据类型.
        """

        if int_cols.empty:
            return {}

        # 定义类型
        int_data_types = [
            (np.uint8, "uint8[pyarrow]"),
            (np.uint16, "uint16[pyarrow]"),
            (np.uint32, "uint32[pyarrow]"),
            (np.uint64, "uint64[pyarrow]"),
            (np.int8, "int8[pyarrow]"),
            (np.int16, "int16[pyarrow]"),
            (np.int32, "int32[pyarrow]"),
            (np.int64, "int64[pyarrow]"),
        ]
        # 创建一个表示类型范围的 NumPy 数组
        type_ranges = np.array(
            [
                (np.iinfo(cast(np.integer, dtype)).min, np.iinfo(cast(np.integer, dtype)).max)
                for dtype, _ in int_data_types
            ]
        )

        # 找到列的最大最小值, 全部为 NAN 列按照 0 处理
        max_min_values = self.df[int_cols].fillna(0).agg(["max", "min"])

        # 计算列范围与类型范围的交集
        intersection = np.logical_and(
            max_min_values.loc["min"].values[:, None] >= type_ranges[:, 0],
            max_min_values.loc["max"].values[:, None] <= type_ranges[:, 1],
        )

        # 找到匹配的类型并返回对应的字符串
        best_int_dtypes = [int_data_types[i][1] for i in np.argmax(intersection, axis=1)]

        # 将结果存储在字典中, 其中键是列名, 值是最佳数据类型
        return dict(zip(int_cols, best_int_dtypes))

    def _get_best_float_dytapes(self, float_cols: pd.Index) -> dict:
        """
        将float列转换为最佳类型.

        Args:
            float_cols (pd.Index): float列的索引

        Returns:
            dict: 包含最佳数据类型的字典，键为列名，值为最佳数据类型.
        """

        if float_cols.empty:
            return {}

        # 定义类型, pyarrow不支持直接转换为 float16[pyarrow], 所以此处定义为 np.float16, 后续再转换为 float16[pyarrow]
        float_data_types = [
            (np.float32, "float32[pyarrow]"),
            (np.float64, "float64[pyarrow]"),
        ]

        best_float_dtypes = []
        for col in float_cols:
            # 找到每一列小数点后最大小数位数
            max_decimal_places = (
                self.df[col].astype(str).apply(lambda s: len(s) - s.index(".") - 1 if "." in s else 0).max()
            )
            # 计算列范围与类型范围
            if max_decimal_places <= np.finfo(np.float32).precision:
                best_float_dtypes.append(float_data_types[0][1])
            else:
                best_float_dtypes.append(float_data_types[1][1])

        return dict(zip(float_cols, best_float_dtypes))

    def _get_mapping(self) -> dict:
        """
        获取最佳数据类型映射.

        Returns:
            dict: 包含最佳数据类型映射的字典，键为列名，值为最佳数据类型.
        """

        int_cols = self.df.select_dtypes(include="int").columns
        str_cols = self.df.select_dtypes(include="string").columns
        float_cols = self.df.select_dtypes(include="float").columns

        # 将string列转换为category类型
        mapping = dict.fromkeys(str_cols, "category")
        # 将float列转换为Float32类型
        mapping.update(self._get_best_float_dytapes(float_cols))
        # 将int列转换为最佳类型
        mapping.update(self._get_best_int_dtypes(int_cols))

        return mapping

    def process(self) -> pd.DataFrame:
        """
        处理DataFrame, 将列转换为最佳数据类型以减少内存占用.

        Returns:
            pd.DataFrame: 处理后的DataFrame
        """

        # 列名排序并自动转化格式
        self.df = self.df.sort_index(axis=1).convert_dtypes(dtype_backend="pyarrow")

        # print("================================")
        # print(self.df.dtypes.to_string())
        # self.df.info(memory_usage="deep")
        # print("\n")
        # # print(self.df.dtypes)

        # # 计算每列的内存占用情况
        # memory_usage = self.df.memory_usage(deep=True)

        # for column in self.df.columns:
        #     memory_usage_mb = memory_usage[column] / (1024 * 1024)
        #     print(f"Column: {column}\tType: {self.df[column].dtype}\tMemory: {memory_usage_mb:.2f} MB")

        # print("================================")

        # 减少内存, 列转换为 best 类型
        return self.df.astype(self._get_mapping())


# event relate define
class KTrigger(NamedTuple):
    type: str
    value: float = 0.0
    rule: str = "greaterThan"
    disttype: str = "longitudinal"
    ref: str = ""
    wpt: Waypoint = Waypoint()


class KAction(NamedTuple):
    type: str
    value: float = 0.0
    endtype: str = settings.sys.l4.EndtypeMode.enum.none
    endvalue: float = 0.0
    ref: str = ""
    wpts: List[Waypoint] = [Waypoint()]
    user: Any = None


class KEvent(NamedTuple):
    trigger: KTrigger
    actions: list[KAction]


# L1_Road relate define
class KRoad(NamedTuple):
    type: str
    curve: float
    direction: str
    speedlimit: float = 0.0
    total_length: int = int(settings.sys.map.virtual.base_len)


class KJunction(NamedTuple):
    type: str = settings.sys.l1.JunctionType.enum.none
    num: int = 0
    trafficlight_status: int = 0
    trafficlight_cycle: int = 0


class KLane(NamedTuple):
    types: list
    num: int
    num2: int = 0
    num2dist: int = 0
    width: float = 3.5
    edge: str = "none"
    # speedlimit: float


class KMark(NamedTuple):
    type: str
    color: str
    status: str


@dataclass(order=True)
class BinaryDependencySorter:
    pathdir_binary: Path
    binary_name: str

    def __post_init__(self):
        self.filenames_binary = self.find_filenames_binary()
        self.dependency_graph = self.build_dependency_graph()

    def find_filenames_binary(self) -> List[str]:
        """Find all binary file names in the given directory matching the binary_name pattern."""
        filenames_binary = []
        for file in self.pathdir_binary.glob(self.binary_name):
            if file.is_file():
                filenames_binary.append(file.name)
        return filenames_binary

    def get_dependencies(self, pathfile_binary: Path) -> List[str]:
        """Get the dependencies of a binary file."""
        try:
            output = subprocess.check_output(["ldd", str(pathfile_binary)], text=True)
            dependencies = []
            for line in output.splitlines():
                parts = line.split("=>")
                if len(parts) > 0:
                    dependency_name = parts[0].strip().split()[0]
                    if dependency_name in self.filenames_binary:
                        dependencies.append(dependency_name)
            return dependencies
        except Exception as e:
            print(f"Error while getting dependencies for {pathfile_binary}: {e}")
            return []

    def build_dependency_graph(self) -> Dict[str, List[str]]:
        """Build the dependency graph."""
        dependency_graph = defaultdict(list)
        for filename in self.filenames_binary:
            filepath = self.pathdir_binary / filename
            dependencies = self.get_dependencies(filepath)
            dependency_graph[filename].extend(dependencies)
        return dependency_graph

    def topological_sort(self) -> List[str]:
        """Perform a topological sort on the dependency graph and detect cycles."""
        in_degree = {node: 0 for node in self.dependency_graph}
        for node in self.dependency_graph:
            for neighbor in self.dependency_graph[node]:
                in_degree[neighbor] += 1

        queue = deque([node for node in self.dependency_graph if in_degree[node] == 0])
        sorted_list = []
        visited_count = 0

        while queue:
            node = queue.popleft()
            sorted_list.append(node)
            visited_count += 1
            for neighbor in self.dependency_graph[node]:
                in_degree[neighbor] -= 1
                if in_degree[neighbor] == 0:
                    queue.append(neighbor)

        if visited_count != len(self.dependency_graph):
            raise Exception("Cycle detected in dependency graph")

        return sorted_list

    def sort_binaries(self) -> List[str]:
        """Sort the binaries by their dependencies."""
        try:
            sorted_binaries = self.topological_sort()
            # Reverse the list to have least dependent files first
            return sorted_binaries[::-1]
        except Exception as e:
            print(e)
            return []


if __name__ == "__main__":
    pass
