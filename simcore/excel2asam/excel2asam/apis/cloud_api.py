#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file cloud_api.py
# * \brief api for cloud
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-08-01
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import re
from dataclasses import dataclass, field
from typing import Any

import pandas as pd
from google.protobuf import json_format
from loguru import logger

from excel2asam.apis import task_generalized_pb2, task_testcase_pb2
from excel2asam.config import settings


@dataclass(order=True)
class CloudApi:
    task_generalized_id: int = field(default=77777)
    group_id_func: int = field(default=88888)
    group_id_logic: int = field(default=99999)
    json_pb_is_pb: bool = field(default=True)

    def __post_init__(self) -> None:
        pass

    def _serialization_parameter(self, key: str, value: str) -> task_generalized_pb2.Parameter:  # type: ignore
        # print(key, value)

        tpa = task_generalized_pb2.Parameter()  # type: ignore

        # get & set key
        tpa.key = key

        # get & set GeneralizedType
        tpa.generalized_type = task_generalized_pb2.GENERALIZED_TYPE_CARTESIAN_PRODUCT  # type: ignore
        if settings.sys.sign.one2one in value:
            tpa.generalized_type = task_generalized_pb2.GENERALIZED_TYPE_ONE2NE  # type: ignore
            value = value.replace(settings.sys.sign.one2one, "")

        # get & set unit
        unit = next((unit for unit in settings.sys.unitcharacter.transfer.values() if unit in value), None)
        if unit is not None:
            value = value.replace(unit, "")
            tpa.unit = unit

        # get & set ParameterType and value
        if settings.sys.sign.enum in value:
            tpa.type = task_generalized_pb2.PARAMETER_TYPE_ENUM  # type: ignore
            tpa.enum_value = value
        elif settings.sys.sign.range in value:
            tpa.type = task_generalized_pb2.PARAMETER_TYPE_RANGE  # type: ignore
            tpa.min_value, tpa.step_value, tpa.max_value = list(map(float, value.split(settings.sys.sign.range)))
        else:
            tpa.type = task_generalized_pb2.PARAMETER_TYPE_ENUM  # type: ignore
            tpa.enum_value = value
        return tpa

    def _serialization_tree_scene_set(self, classify_str: str) -> task_generalized_pb2.TreeSceneSet:  # type: ignore
        tree_scene_set = task_generalized_pb2.TreeSceneSet()  # type: ignore

        if classify_str:
            tree_scene_set.classifys.extend(
                [
                    settings.sys.classify.root,
                    *re.split(
                        settings.sys.link_symbol,
                        classify_str.replace(
                            f"{settings.sys.link_symbol}{settings.sys.classify.default_value_chn}", ""
                        ).replace(f"{settings.sys.link_symbol}{settings.sys.classify.default_value}", ""),
                    ),
                ]
            )

        return tree_scene_set

    def set_api(self, pb: Any) -> str:
        if self.json_pb_is_pb:
            return pb.SerializeToString()

        return json_format.MessageToJson(
            message=pb,
            including_default_value_fields=True,
            preserving_proto_field_name=True,
            ensure_ascii=False,
        )

    # task_testcase
    def set_fail_task_testcaselist(self) -> task_testcase_pb2.TaskTestcaseList:  # type: ignore
        ttasks = self.set_default_task_testcaselist()
        ttasks.status = task_testcase_pb2.TASK_TESTCASE_STATUS_FAIL  # type: ignore

        logger.opt(lazy=True).info("set_fail_task_testcaselist fail")

        return ttasks

    def _serialization_task_testcase(self, row: pd.Series) -> task_testcase_pb2.TaskTestcase:  # type: ignore
        """
        It takes a row of the dataframe and returns a TaskTestcase object

        Args:
          row (pd.Series): pd.Series

        Returns:
          A TaskTestcase object
        """
        # 创建一个 TaskTestCase 对象
        ttask = task_testcase_pb2.TaskTestcase()  # type: ignore
        # 配置任务信息
        ttask.type = getattr(
            task_testcase_pb2.TaskTestcaseType,  # type: ignore
            "TASK_TESTCASE_TYPE_" + row[settings.sys.testcase.cols.simtype].upper(),
        )
        # ttask.is_perception_in_loop = "No"
        ttask.name = row[settings.sys.testcase.cols.name]
        ttask.description = row[settings.sys.testcase.cols.task_description]
        ttask.priority = getattr(
            task_testcase_pb2.TaskTestcasePriority,  # type: ignore
            "TASK_TESTCASE_PRIORITY_" + row[settings.sys.testcase.cols.priority],
        )
        ttask.ego_name = row[settings.sys.testcase.cols.ego_name]

        # 配置评测
        ttask.kpis.extend(task_testcase_pb2.Kpi(name=n) for n in row[settings.sys.testcase.cols.kpis])  # type: ignore

        # 配置场景
        for is_generalized, logicid in zip(
            row[settings.sys.l0.cols.is_generalized], row[settings.sys.testcase.cols.logicid]
        ):
            if is_generalized:
                ttask.logic_names_generalized.append(logicid)
            else:
                ttask.logic_names_non_generalized.append(logicid)

        ttask.logic_number = len(row[settings.sys.generalized.cols.logicid])

        ttask.labels.extend(re.split(settings.sys.split_symbol, row[settings.sys.testcase.cols.task_label]))
        ttask.project = row[settings.sys.testcase.cols.project]
        # ego_controller
        ttask.ego_controller = row[settings.sys.testcase.cols.ego_controller]
        # set
        ttask.sets.extend(re.split(settings.sys.split_symbol, row[settings.sys.testcase.cols.sets]))
        # concrete_name
        ttask.concrete_name.extend(row[settings.sys.testcase.cols.concreteid])

        return ttask

    def set_default_task_testcaselist(self):
        result = task_testcase_pb2.TaskTestcaseList()
        result.task_generalized_id = self.task_generalized_id
        result.link_symbol = settings.sys.link_symbol
        return result

    def process_task_testcaselist(self, testcase: pd.DataFrame) -> task_testcase_pb2.TaskTestcaseList:  # type: ignore
        ttasks = self.set_default_task_testcaselist()
        testcase_pb = testcase.apply(lambda x: self._serialization_task_testcase(x), axis=1)

        ttasks.tasks.extend(testcase_pb)
        logger.opt(lazy=True).info("process_task_testcaselist success")

        return ttasks

    # task_generalized
    def set_default_task_generalized(self) -> task_generalized_pb2.TaskGeneralized:  # type: ignore
        # 实例化
        gtask = task_generalized_pb2.TaskGeneralized()  # type: ignore
        # 赋值固定值
        gtask.task_generalized_id = self.task_generalized_id
        gtask.link_symbol = settings.sys.link_symbol
        return gtask

    def set_task_generalized_status_to_fail(self) -> task_generalized_pb2.TaskGeneralized:  # type: ignore
        # logger.opt(lazy=True, colors=True).warning("<yellow>concrete is empty, status setting to fail</yellow>")
        gtask = self.set_default_task_generalized()
        gtask.status = task_generalized_pb2.TASK_GENERALIZED_STATUS_FAIL  # type: ignore
        return gtask

    def process_task_generalized_once(
        self, concrete: pd.DataFrame, virtual_real_is_virtual: bool
    ) -> task_generalized_pb2.TaskGeneralized:  # type: ignore
        gtask = self.set_default_task_generalized()

        #
        gtask.unique_tree_scene_sets.extend(
            concrete[settings.sys.l0.cols.classify]
            .astype(str)
            .drop_duplicates()
            .apply(lambda x: self._serialization_tree_scene_set(x))
        )

        #
        gtask.funcs.extend(
            concrete.drop_duplicates(subset=[settings.sys.generalized.cols.funcid]).apply(
                lambda x: task_generalized_pb2.FunctionalScenario(  # type: ignore
                    labels=re.split(settings.sys.split_symbol, x[settings.sys.generalized.cols.label]),
                    description=x[settings.sys.generalized.cols.description],
                    func_name=x[settings.sys.generalized.cols.funcid],
                    # group_id=self.group_id_func,
                    logic_number=x[settings.sys.l0.cols.allnum_logic],
                    concrete_number=x[settings.sys.l0.cols.allnum_concrete4func],
                    tree_scene_set=self._serialization_tree_scene_set(x[settings.sys.l0.cols.classify]),
                ),
                axis=1,
            )
        )

        #
        gtask.logics.extend(
            concrete.drop_duplicates(subset=[settings.sys.generalized.cols.logicid]).apply(  # type: ignore
                lambda x: task_generalized_pb2.LogicalScenario(  # type: ignore
                    labels=re.split(settings.sys.split_symbol, x[settings.sys.generalized.cols.label]),
                    description=x[settings.sys.generalized.cols.description],
                    func_name=x[settings.sys.generalized.cols.funcid],
                    logic_name=x[settings.sys.generalized.cols.logicid],
                    # group_id=self.group_id_logic,
                    concrete_number=x[settings.sys.l0.cols.allnum_concrete4logic],
                    parameters=[
                        self._serialization_parameter(k, v) for k, v in x[settings.sys.l0.cols.parameter].items()
                    ],
                    tree_scene_set=self._serialization_tree_scene_set(x[settings.sys.l0.cols.classify]),
                ),
                axis=1,
            )
        )

        if virtual_real_is_virtual:
            # id and version 由调用者进行赋值
            gtask.maps.extend(
                concrete[settings.sys.l0.cols.mapfile]
                .astype(str)
                .drop_duplicates()
                .apply(lambda x: task_generalized_pb2.VirtualMap(name=x))  # type: ignore
            )

        logger.opt(lazy=True).info("process_task_generalized_once success")

        return gtask

    def process_task_generalized_update(
        self,
        concrete: pd.DataFrame,
        start: int,
        end: int,
        status: task_generalized_pb2.TaskGeneralizedStatus,  # type: ignore
        invalids: pd.DataFrame,
    ) -> task_generalized_pb2.TaskGeneralized:  # type: ignore
        gtask = self.set_default_task_generalized()

        if start < len(concrete):
            gtask.concretes.extend(
                concrete.loc[start:end].apply(
                    lambda x: task_generalized_pb2.ConcreteScenario(  # type: ignore
                        labels=re.split(settings.sys.split_symbol, x[settings.sys.generalized.cols.label]),
                        description=x[settings.sys.generalized.cols.description],
                        virtual_map_name=x[settings.sys.l0.cols.mapfile],
                        func_name=x[settings.sys.generalized.cols.funcid],
                        logic_name=x[settings.sys.generalized.cols.logicid],
                        concrete_name=x[settings.sys.l0.cols.concreteid],
                        tree_scene_set=self._serialization_tree_scene_set(x[settings.sys.l0.cols.classify]),
                    ),
                    axis=1,
                )
            )

        if start < len(invalids):
            gtask.unqualifieds.extend(
                invalids.loc[start:end]
                .astype(str)
                .fillna("")
                .apply(
                    lambda x: task_generalized_pb2.UnqualifiedScenario(  # type: ignore
                        func_name=x[settings.sys.generalized.cols.funcid],
                        logic_name=x[settings.sys.generalized.cols.logicid],
                        concrete_name=x[settings.sys.l0.cols.concreteid],
                        id=x.name,
                        assessment_dimensions=x[settings.sys.l0.cols.assessment],
                        cause_analysis=x[settings.sys.l0.cols.reason],
                        status="已剔除",
                    ),
                    axis=1,
                )
            )

        gtask.status = status
        logger.opt(lazy=True).info("process_task_generalized_update success")

        return gtask

    def process_task_generalized_final(
        self,
        total_func: int,
        total_logic: int,
        total_concretes: int,
        total_concretes_original: int,
        total_virtual_map: int,
        status: task_generalized_pb2.TaskGeneralizedStatus,  # type: ignore
    ):
        gtask = self.set_default_task_generalized()

        gtask.func_number = total_func
        gtask.logic_number = total_logic
        gtask.concrete_number = total_concretes
        gtask.virtual_map_number = total_virtual_map

        gtask.concrete_number_original = total_concretes_original
        gtask.qualification_rate = total_concretes / total_concretes_original

        gtask.status = status

        return gtask


# sourcery skip: remove-redundant-if
if __name__ == "__main__":
    pass
