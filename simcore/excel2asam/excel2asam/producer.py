#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file producer.py
# * \brief Define scene by Excel to generate scene file in OpenSCENARIO (.xosc)
# * and OpenDRIVE (.xodr) format.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import argparse
import sys
import traceback
from abc import ABC
from dataclasses import dataclass
from datetime import datetime
from multiprocessing.dummy import Pool as ThreadPool
from pathlib import Path
from time import sleep
from typing import Any, List, Set, Tuple

import pandas as pd
from loguru import logger

import excel2asam.exceptions as exp
from excel2asam.apis.desktop_api import DesktopApi
from excel2asam.config import settings
from excel2asam.formater import Formater
from excel2asam.generalize.generalizer import GeneralizeAll, ModelFilter, RuleFilter
from excel2asam.map.map_manager import MapManager
from excel2asam.map.mapper import RealMapper, VirtualMapper
from excel2asam.openx.wrapper_xodr import GenXodr
from excel2asam.openx.wrapper_xosc import GenXosc
from excel2asam.parser import ExcelParserFactory, FeishuBittableParserFactory, Parser, ParserFactory
from excel2asam.utils import SceneFilter, deep_update_settings


@dataclass(order=True)
class ProducerFactory(ABC):
    parser_factory: ParserFactory
    pathdir_catalog: Path
    pathdir_hadmap: Path
    pathdir_output: Path
    task_generalized_id: int

    def __post_init__(self) -> None:
        # for get total number info
        self._total_funcs = 0
        self._total_logics = 0
        self._total_concretes = 0
        self._total_concretes_original = 0
        self._total_virtual_map = 0
        self._total_invalid = 0

        #
        self._ini = 0
        self._interval_logic = 100
        self._interval_concrete = 1000

        # 设置输出文件夹并创建 log 文件
        self.pathdir_output_xosc, self.pathdir_output_xodr = self._set_output_folder_structure()

        #
        logger.opt(lazy=True).info(f"{' Input Parameter: ':=^55}")
        logger.opt(lazy=True).info(f"{self.pathdir_catalog = }")
        logger.opt(lazy=True).info(f"{self.pathdir_hadmap = }")
        logger.opt(lazy=True).info(f"{self.pathdir_output = }")
        logger.opt(lazy=True).info(f"{self.task_generalized_id = }")

        # 实例化 SceneFilter, 用于过滤场景并存储其信息
        self.scene_filter = SceneFilter()

        # user choose map mode
        self.virtual_real_is_virtual, self.mapfiles = self._user_choose_virtual_map()

    def _multi_threaded_generation(self, arg0: Any, arg1: List[dict]) -> None:
        pool = ThreadPool(settings.sys.thread_num)
        pool.map(arg0.generate, arg1)
        pool.close()
        pool.join()

    def _user_choose_virtual_map(self) -> Tuple[bool, Set[str]]:
        logger.opt(lazy=True).info(f"{' Check user choose map: ':=^55}")

        # 初始化 mapfiles 为空集合, 并添加类型注解
        mapfiles: Set[str] = set()

        # 检查 hadmap (如使用)路径的合法性
        logger.opt(lazy=True).info(f"Input map pathdir: {self.pathdir_hadmap}")

        if not self.pathdir_hadmap.exists():
            logger.opt(lazy=True).warning("Input map pathdir: is illegal")
            return True, mapfiles

        # 查找支持格式的地图文件的名称
        mapfiles = {
            pathfile.name for ext in settings.sys.map.support for pathfile in self.pathdir_hadmap.glob(f"*.{ext}")
        }

        if not mapfiles:
            logger.opt(lazy=True).warning("Input map fullpath: is illegal, Used 语义生成地图, 非地图库地图")
            return True, mapfiles

        return False, mapfiles

    def _check_map_config(self, mapfiles_define: Set, mapfiles_param: Set, mapfiles: Set) -> bool:
        logger.opt(lazy=True).info("Check map config")

        # 去除空字符串
        mapfiles_define.discard("")
        mapfiles_param.discard("")
        mapfiles.discard("")

        logger.opt(lazy=True).debug(f"mapfiles_define: {mapfiles_define}")
        logger.opt(lazy=True).debug(f"mapfiles_param: {mapfiles_param}")
        logger.opt(lazy=True).debug(f"mapfiles: {mapfiles}")

        if not mapfiles_define:
            raise exp.MapConfigError("mapfiles_define is empty.")
        if not mapfiles_param:
            raise exp.MapConfigError("mapfiles_param is empty.")
        if not mapfiles:
            raise exp.MapConfigError("mapfiles is empty.")

        # 计算交集
        intersection = mapfiles_define.intersection(mapfiles_param).intersection(mapfiles)

        # 检查交集结果是否包含 mapfiles_define 的所有内容
        if mapfiles_define.issubset(intersection):
            return True
        else:
            raise exp.MapConfigError("The intersection does not contain all elements of mapfiles_define.")

    def _cal_num_relationships(self, concrete: pd.DataFrame) -> pd.DataFrame:
        # 获得每一个功能场景下的逻辑场景数量
        concrete[settings.sys.l0.cols.allnum_logic] = (
            concrete.groupby(settings.sys.generalized.cols.funcid, observed=True)[settings.sys.generalized.cols.logicid]
            .transform("nunique")
            .astype("uint32[pyarrow]")
        )

        # 获得每一个功能场景下的具体场景总数量
        concrete[settings.sys.l0.cols.allnum_concrete4func] = (
            concrete.groupby(settings.sys.generalized.cols.funcid, observed=True)[settings.sys.l0.cols.concreteid]
            .transform("nunique")
            .astype("uint32[pyarrow]")
        )

        # 获得每一个逻辑场景下的具体场景总数量
        concrete[settings.sys.l0.cols.allnum_concrete4logic] = (
            concrete.groupby(settings.sys.generalized.cols.logicid, observed=True)[settings.sys.l0.cols.concreteid]
            .transform("nunique")
            .astype("uint32[pyarrow]")
        )

        return concrete

    def _set_output_folder_structure(self) -> Tuple:
        """
        > Create output folders

        Returns:
          The path to the xosc and xodr directories.
        """
        # 创建 xosc xodr 子文件夹
        try:
            # 设置 xodr 文件夹
            pathdir_output_xodr = self.pathdir_output / f"{settings.sys.output_xodr}"
            pathdir_output_xodr.mkdir(parents=True, exist_ok=True, mode=0o755)

            # 设置 xosc 文件夹, 同 xosc 文件夹下
            pathdir_output_xosc = self.pathdir_output / f"{settings.sys.output_xosc}/{self.task_generalized_id}"
            pathdir_output_xosc.mkdir(parents=True, exist_ok=True, mode=0o755)

            # 创建 log 文件, 同 xosc 文件在一起
            logger.opt(lazy=True).add(
                pathdir_output_xosc / f"{settings.sys.filename_log}.log", rotation="100 MB", encoding="utf-8"
            )

            return pathdir_output_xosc, pathdir_output_xodr

        except OSError as e:
            logger.error(f"Error creating output directories: {e}")
            raise

    def hook_send_maps_insert(self, scene: dict):
        pass

    def hook_send_scenarios_insert(self, scene: dict, count: int):
        pass

    def hook_send_status_monitor_cancle_to_exit(self, count: int) -> None:
        pass

    def hook_send_status_fail(self):
        pass

    def send_generalized_empty_error(self, *args):
        logger.opt(lazy=True).warning("GeneralizedEmptyError found, No task_generalized defined")
        self.hook_send_status_fail()

    def send_testcase_empty_error(self, *args):
        logger.opt(lazy=True).warning("TestcaseEmptyError found, No task_testcaselist defined")
        self.hook_send_status_fail()

    def send_except(self, *args):
        e = args[0]

        # 获取异常的 traceback 对象
        tbs = traceback.extract_tb(e.__traceback__)

        # 按行格式化 traceback 信息并存储到列表中
        formatted_tb_list = []
        for tb in tbs:
            formatted_tb = f"文件: {tb.filename}\n行号: {tb.lineno}\n函数: {tb.name}\n代码: {tb.line}\n"
            formatted_tb_list.append(formatted_tb)

        out = "\n".join(formatted_tb_list)

        # 打印异常信息
        logger.opt(lazy=True).error(f"Caught {e.__class__.__name__} exception: {e}, \n" f"{out}")

        #
        self.hook_send_status_fail()

    def send_finally(self, *args):
        logger.opt(lazy=True).info("Finished")

    def step1_get_logic_param(self) -> Set[pd.DataFrame, dict]:
        # 检查数据源路径合法性, 并解析
        parser = Parser(self.parser_factory, self.virtual_real_is_virtual)

        # 实例化 Formater, 并格式化 scenes 场景描述 和 param 地图参数 和 用户设置
        formater = Formater(parser.define, parser.param, parser.settings_user, parser.classify_user, self.scene_filter)

        # update global settings
        if formater.settings_user_dict:
            logger.opt(lazy=True).info("基于用户设置更新全局配置 settings")
            settings.sys = deep_update_settings(settings.sys, formater.settings_user_dict)

        # 格式化地图库 param 参数
        param = formater.param_dict

        # 格式化 scenes 场景描述, 筛选出所有逻辑场景
        scenes = formater.filter_task_generalized()
        logic = formater.logic(scenes)

        # 总数量增加上逻辑场景中无效的场景数量
        self._total_concretes_original += len(self.scene_filter.invalids)

        return logic, param

    def step2_get_mapfiles_define(self, logic: pd.DataFrame, param: dict) -> Set:
        # deal virtual map
        if self.virtual_real_is_virtual:
            mapfiles_define = {""}
            logger.opt(lazy=True).info("Base on Virtual map creat scene")
        # deal real map
        else:
            mapfiles_define = set(logic[settings.sys.generalized.cols.mapfile].unique())
            mapfiles_param = set(param.keys())
            if self._check_map_config(mapfiles_define, mapfiles_param, self.mapfiles):
                logger.opt(lazy=True).info("Base on Real map creat scene")

        return mapfiles_define

    def step3_get_concrete(self, logic: pd.DataFrame, param: dict, mapfiles_define: Set) -> pd.DataFrame:
        logger.opt(lazy=True).info("Process logic")

        # 逻辑场景泛化为原始的具体场景 (大量的)
        concrete = GeneralizeAll(logic).process()
        self._total_concretes_original += len(concrete)

        # 通过规则剔除掉不满足条件的具体场景
        concrete = RuleFilter(concrete, self.scene_filter).process()

        # 是否开启 Model 方式过滤具体场景场景
        if settings.sys.model.enable == 1:
            concrete = ModelFilter(concrete, self.scene_filter).process()

        # 处理虚拟地图情况
        if self.virtual_real_is_virtual:
            concrete = VirtualMapper(concrete).process()
            self._total_virtual_map += len(concrete.drop_duplicates([settings.sys.l0.cols.mapfile]))
        # 处理真实地图情况
        else:
            concrete_map_list = []
            for mapfile in mapfiles_define:
                logger.opt(lazy=True).info(f"{f' mapfile: {mapfile} ':-^55}")
                concrete_map = concrete[concrete[settings.sys.generalized.cols.mapfile] == mapfile]
                param_map = param[mapfile]

                # 通过 Map 处理相对应的内容列
                with MapManager(self.pathdir_hadmap / mapfile) as map_manager:
                    concrete_map_list.append(
                        RealMapper(concrete_map, map_manager, param_map, self.scene_filter).process()
                    )
            # 拼接所有 concrete
            concrete = pd.concat(concrete_map_list, ignore_index=True)

        # 处理过滤后场景为空, 返回状态, 便于后续判断
        if concrete.empty:
            raise exp.AfterFilterEmptyError("After process logic scenarios, there is no valid data")

        # 增加功能场景 - 逻辑场景 - 具体场景之间的数量关系
        concrete = self._cal_num_relationships(concrete)

        # 计算数量
        self._total_funcs += len(concrete.drop_duplicates([settings.sys.generalized.cols.funcid]))
        self._total_logics += len(concrete.drop_duplicates([settings.sys.generalized.cols.logicid]))
        self._total_concretes += len(concrete)
        self._total_invalid = len(self.scene_filter.invalids)

        return concrete

    def step4_ready_and_waiting(self, wait_time_sec: int) -> None:
        #
        logger.opt(lazy=True).info(f"预计生成场景总数: {self._total_concretes_original}")
        logger.opt(lazy=True).info(f"预计场景留存总数: {self._total_concretes}")
        logger.opt(lazy=True).info(f"预计生成剔除总数: {self._total_invalid}")

    def step5_generate_file_xodr(self, concrete: pd.DataFrame) -> None:
        # 生成 xodr 文件, 通过迭代器进行循环, 进而生成 xodr
        gxodr = GenXodr(
            gen_folder=self.pathdir_output_xodr,
            odrmajor=settings.get("sys.version.odrmajor", 1),
            odrminor=settings.get("sys.version.odrminor", 5),
        )
        concrete_map = (
            concrete.drop_duplicates([settings.sys.l0.cols.mapfile])
            .apply(lambda x: x.dropna().to_dict(), axis=1)
            .tolist()
        )

        for scene in concrete_map:
            gxodr.generate(scene)
            self.hook_send_maps_insert(scene)

        logger.opt(lazy=True).info(f"Finished xodr file {len(concrete_map)}")

    def step6_generate_file_xosc(self, concrete: pd.DataFrame) -> None:
        # 生成 xosc 文件, 通过范围进行循环, 进而生成 xosc (用于控制外调接口性能)
        gxosc = GenXosc(
            gen_folder=self.pathdir_output_xosc,
            oscminor=settings.get("sys.version.oscminor", 0),
            pathdir_catalog=self.pathdir_catalog,
        )
        concrete_xosc = concrete.apply(lambda x: x.dropna().to_dict(), axis=1).tolist()
        for count, scene in enumerate(concrete_xosc, start=1):
            gxosc.generate(scene)
            self.hook_send_scenarios_insert(scene, count)
            self.hook_send_status_monitor_cancle_to_exit(count)

        logger.opt(lazy=True).info(f"Finished xosc file {len(concrete_xosc)}")

    def process(self) -> None:
        try:
            # 读取并格式化所有 logic 场景 & 地图描述参数
            logic, param = self.step1_get_logic_param()
            # 获取 mapfiles_define
            mapfiles_define = self.step2_get_mapfiles_define(logic, param)
            # 获取全部 concrete 场景
            concrete = self.step3_get_concrete(logic, param, mapfiles_define)
            # 等待确认
            self.step4_ready_and_waiting(wait_time_sec=0.5)
            # 生成虚拟地图, 如果需要
            if self.virtual_real_is_virtual:
                self.step5_generate_file_xodr(concrete)
            # 生成场景文件
            self.step6_generate_file_xosc(concrete)

        # 处理未定义测试用例为空的情况
        except exp.TestcaseEmptyError:
            self.send_testcase_empty_error()

        # 处理未定义语义生成
        except exp.GeneralizedEmptyError:
            self.send_generalized_empty_error()

        # 处理选择场景库地图 & 地图参数定义错误的情况
        # except exp.MapConfigError:
        #    pass

        # 处理其余异常情况
        except Exception as e:
            self.send_except(e)

        finally:
            self.send_finally()
            sys.exit(0)  # 退出整个程序


@dataclass(order=True)
class LocalProducerFactory(ProducerFactory):
    def __post_init__(self) -> None:
        super().__post_init__()


@dataclass(order=True)
class DesktopProducerFactory(ProducerFactory):
    def __post_init__(self):
        # 调用 Producer 类的 __post_init__ 方法
        super().__post_init__()
        self.api = DesktopApi(self.pathdir_output_xosc)

    def hook_send_maps_insert(self, scene: dict):
        self.api.send_maps_insert(str(self.pathdir_output_xodr / scene[settings.sys.l0.cols.mapfile]))

    def hook_send_scenarios_insert(self, scene: dict, count: int):
        scene_name = scene[settings.sys.l0.cols.concreteid]

        #
        self.api.send_scenarios_insert(
            scene_id=count,
            scene_name=scene_name,
            map_name=scene[settings.sys.l0.cols.mapfile],
            scene_info=scene.get(settings.sys.generalized.cols.description, "Multi"),
        )

    def hook_send_status_monitor_cancle_to_exit(self, count: int) -> bool:
        result = self.api.send_semantic_statu(
            gen_status=0,  # 生成中
            all_count=self._total_concretes,
            non_count=self._total_invalid,
            finished_count=count,
        )

        if int(result) == 2:
            logger.opt(lazy=True).info("semantic/statu returned 2, exiting program...")
            sys.exit(0)  # 退出整个程序

    def step4_ready_and_waiting(self, wait_time_sec: int) -> None:
        super().step4_ready_and_waiting(wait_time_sec)

        i = 0
        while True:
            result = self.api.send_semantic_statu(
                gen_status=4,  # 准备完成
                all_count=self._total_concretes,
                non_count=self._total_invalid,
                finished_count=self._ini,
            )

            if result == 2:
                logger.opt(lazy=True).info("semantic/statu returned 2, exiting program...")
                sys.exit(0)  # 退出整个程序
            elif result == 1:
                logger.opt(lazy=True).info("semantic/statu returned 1, continuing execution...")
                break  # 退出循环，继续往下执行
            else:
                message = "Waiting for semantic/statu to return 2 or 1..."
                if i == 0:
                    logger.opt(lazy=True).info(message)
                    i = 1
                else:
                    print(message)

                # 等待 wait_time_sec 秒后再次检查
                sleep(wait_time_sec)

    def hook_send_status_fail(self):
        self.api.send_semantic_statu(
            gen_status=2,  # 生成失败
            all_count=self._total_concretes,
            non_count=self._total_invalid,
            finished_count=self._ini,
        )


def parse_arguments() -> argparse.Namespace:
    """设置命令行参数."""

    #
    parser = argparse.ArgumentParser()
    #
    parser.add_argument(
        "--input_mode",
        type=str,
        required=False,
        choices=["excel", "feishu", "tencentdoc"],
        default="excel",
        help="Mode parameter, (default=excel) (optional)",
    )
    parser.add_argument(
        "--input_data",
        type=str,
        required=True,
        help="Input data (required)",
    )
    parser.add_argument(
        "--pathdir_catalog",
        type=str,
        required=False,
        default="./catalogs",
        help="Directory of the catalogs files (default='./catalogs') (optional)",
    )
    parser.add_argument(
        "--pathdir_hadmap",
        type=str,
        required=False,
        default="./not_choose_pathdir_hadmap",
        help="Directory of the map files (default='./not_choose_pathdir_hadmap') (optional)",
    )
    parser.add_argument(
        "--pathdir_output",
        type=str,
        required=False,
        default="",
        help="Directory of the output (default='') (optional)",
    )
    parser.add_argument(
        "--producer_mode",
        type=str,
        required=False,
        choices=["local", "desktop", "cloud"],
        default="local",
        help="Mode parameter, (default=local) (optional)",
    )

    return parser.parse_args()


def main():
    # 获取命令行参数
    args = parse_arguments()
    # 获取配置文件
    input_mode = args.input_mode
    input_data = args.input_data
    pathdir_catalog = Path(args.pathdir_catalog)
    pathdir_hadmap = Path(args.pathdir_hadmap)
    pathdir_output = Path(args.pathdir_output) if args.pathdir_output else Path.cwd()
    producer_mode = args.producer_mode
    task_generalized_id = int(datetime.now().strftime("%Y%m%d%H%M%S"))

    # 根据模式获得 parser_factory
    if input_mode == "excel":
        parser_factory = ExcelParserFactory(Path(input_data))
    elif input_mode == "feishu":
        parser_factory = FeishuBittableParserFactory(
            app_id=settings.feishu.app_id,
            app_secret=settings.feishu.app_secret,
            app_token=input_data,
        )
    else:
        raise ValueError(f"Invalid input_mode: {args.input_mode = }")

    # 根据模式获得 producer_factory
    params = parser_factory, pathdir_catalog, pathdir_hadmap, pathdir_output, task_generalized_id
    if producer_mode == "local":
        producer_factory = LocalProducerFactory(*params)
    elif producer_mode == "desktop":
        producer_factory = DesktopProducerFactory(*params)
    else:
        raise ValueError(f"Invalid producer_mode: {producer_mode}")

    # 执行生成动作
    producer_factory.process()


if __name__ == "__main__":
    main()
