#!/usr/bin/env python3
# -*- coding: utf-8 -*-
import pandas as pd
import pytest

from excel2asam.apis import task_generalized_pb2, task_testcase_pb2
from excel2asam.apis.cloud_api import CloudApi
from excel2asam.config import settings

# --------------------------------- generalized ---------------------------------
# 测试数据
test_data_parameter = [
    (
        "Ego.Ini.Speed",
        "10*5*80&Kph",
        task_generalized_pb2.Parameter(  # type: ignore
            key="Ego.Ini.Speed",
            generalized_type=task_generalized_pb2.GENERALIZED_TYPE_ONE2NE,  # type: ignore
            unit="Kph",
            type=task_generalized_pb2.PARAMETER_TYPE_RANGE,  # type: ignore
            min_value=10,
            step_value=5,
            max_value=80,
        ),
    ),
    (
        "Ego.Ini.Latoffset",
        "0.58/1.16/0/-1.16/-0.58",
        task_generalized_pb2.Parameter(  # type: ignore
            key="Ego.Ini.Latoffset",
            generalized_type=task_generalized_pb2.GENERALIZED_TYPE_CARTESIAN_PRODUCT,  # type: ignore
            type=task_generalized_pb2.PARAMETER_TYPE_ENUM,  # type: ignore
            enum_value="0.58/1.16/0/-1.16/-0.58",
        ),
    ),
    (
        "Npc1.Rel.Long",
        "100*10*240&M",
        task_generalized_pb2.Parameter(  # type: ignore
            key="Npc1.Rel.Long",
            generalized_type=task_generalized_pb2.GENERALIZED_TYPE_ONE2NE,  # type: ignore
            unit="M",
            type=task_generalized_pb2.PARAMETER_TYPE_RANGE,  # type: ignore
            min_value=100,
            step_value=10,
            max_value=240,
        ),
    ),
    (
        "Ego.Ini.Latoffset",
        "60/30/20Mpss",
        task_generalized_pb2.Parameter(  # type: ignore
            key="Ego.Ini.Latoffset",
            generalized_type=task_generalized_pb2.GENERALIZED_TYPE_CARTESIAN_PRODUCT,  # type: ignore
            unit="Mpss",
            type=task_generalized_pb2.PARAMETER_TYPE_ENUM,  # type: ignore
            enum_value="60/30/20",
        ),
    ),
]


# 测试用例
@pytest.mark.parametrize("key, value, expected", test_data_parameter)
def test_deal_parameter(key, value, expected):
    # 使用提供的类名
    obj = CloudApi()
    # 注意：这里使用了Python的名称修饰, 因为serialization_parameter是一个私有方法
    # result = obj._CloudApi_serialization_parameter(key, value)
    result = obj._serialization_parameter(key, value)
    assert result == expected


def test_set_default_task_generalized():
    obj = CloudApi()  # 使用提供的类名
    result = obj.set_default_task_generalized()

    # 检查结果是否为 TaskGeneralized 类型
    assert isinstance(result, task_generalized_pb2.TaskGeneralized)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查其他属性是否为空
    assert len(result.funcs) == 0
    assert len(result.logics) == 0
    assert len(result.concretes) == 0
    assert len(result.maps) == 0
    assert len(result.unqualifieds) == 0
    assert result.func_number == 0
    assert result.logic_number == 0
    assert result.concrete_number == 0
    assert result.virtual_map_number == 0
    assert result.concrete_number_original == 0
    assert result.qualification_rate == 0

    # 检查任务状态是否为默认值
    assert result.status == task_generalized_pb2.TaskGeneralizedStatus.TASK_GENERALIZED_STATUS_ING


def test_generalized_status_to_fail():
    obj = CloudApi()  # 使用提供的类名
    result = obj.set_task_generalized_status_to_fail()

    # 检查结果是否为 TaskGeneralized 类型
    assert isinstance(result, task_generalized_pb2.TaskGeneralized)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查任务状态是否为失败
    assert result.status == task_generalized_pb2.TaskGeneralizedStatus.TASK_GENERALIZED_STATUS_FAIL

    # 检查其他属性是否为空
    assert len(result.funcs) == 0
    assert len(result.logics) == 0
    assert len(result.concretes) == 0
    assert len(result.maps) == 0
    assert len(result.unqualifieds) == 0
    assert result.func_number == 0
    assert result.logic_number == 0
    assert result.concrete_number == 0
    assert result.virtual_map_number == 0
    assert result.concrete_number_original == 0
    assert result.qualification_rate == 0


def test_process_task_generalized_once():
    # 创建示例 DataFrame
    concrete_data = {
        settings.sys.generalized.cols.funcid: ["Func1", "Func1"],
        settings.sys.generalized.cols.logicid: ["Logic1", "Logic2"],
        settings.sys.generalized.cols.label: ["Label1", "Label2"],
        settings.sys.generalized.cols.description: ["Description1", "Description2"],
        settings.sys.l0.cols.allnum_logic: [2, 2],
        settings.sys.l0.cols.allnum_concrete4func: [4, 4],
        settings.sys.l0.cols.allnum_concrete4logic: [4, 4],
        settings.sys.l0.cols.parameter: [{"key1": "10*5*80&Kph"}, {"key2": "0.58/1.16/0/-1.16/-0.58"}],
        settings.sys.l0.cols.mapfile: ["Map1.xodr", "Map2.xodr"],
        settings.sys.l0.cols.classify: ["Class1", "Class2"],
    }
    concrete_df = pd.DataFrame(concrete_data)

    obj = CloudApi()  # 使用提供的类名
    result = obj.process_task_generalized_once(concrete_df, virtual_real_is_virtual=True)

    # 检查结果是否为 TaskGeneralized 类型
    assert isinstance(result, task_generalized_pb2.TaskGeneralized)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查 funcs 和 logics 是否正确设置
    assert len(result.funcs) == 1
    assert len(result.logics) == 2

    # 检查虚拟地图是否正确设置
    assert len(result.maps) == 2
    assert result.maps[0].name == "Map1.xodr"
    assert result.maps[1].name == "Map2.xodr"

    # 检查其他属性是否为空
    assert len(result.concretes) == 0
    assert len(result.unqualifieds) == 0
    assert result.func_number == 0
    assert result.logic_number == 0
    assert result.concrete_number == 0
    assert result.virtual_map_number == 0
    assert result.concrete_number_original == 0
    assert result.qualification_rate == 0

    # 检查任务状态是否为默认值
    assert result.status == task_generalized_pb2.TaskGeneralizedStatus.TASK_GENERALIZED_STATUS_ING


def test_task_generalized_update():
    # 创建示例 DataFrame
    concrete_data = {
        settings.sys.generalized.cols.funcid: ["Func1", "Func1"],
        settings.sys.generalized.cols.logicid: ["Logic1", "Logic2"],
        settings.sys.l0.cols.concreteid: ["Concrete1", "Concrete2"],
        settings.sys.generalized.cols.label: ["Label1", "Label2"],
        settings.sys.generalized.cols.description: ["Description1", "Description2"],
        settings.sys.l0.cols.parameter: [{"key1": "10*5*80&Kph"}, {"key2": "0.58/1.16/0/-1.16/-0.58"}],
        settings.sys.l0.cols.mapfile: ["Map1.xodr", "Map2.xodr"],
        settings.sys.l0.cols.classify: ["Class1_Class11", "Class2"],
    }
    concrete_df = pd.DataFrame(concrete_data)

    invalids_data = {
        settings.sys.generalized.cols.funcid: ["ErrFunc1", "ErrFunc1"],
        settings.sys.generalized.cols.logicid: ["ErrLogic1", "ErrLogic2"],
        settings.sys.l0.cols.concreteid: ["ErrConcrete1", "ErrConcrete2"],
        settings.sys.l0.cols.reason: ["Reason1", "Reason2"],
        settings.sys.l0.cols.assessment: ["Dimension1", "Dimension2"],
        "status": ["已剔除", "已剔除"],
    }
    invalids_df = pd.DataFrame(invalids_data)

    obj = CloudApi()  # 使用提供的类名
    result = obj.process_task_generalized_update(
        concrete_df,
        start=0,
        end=1,
        status=task_generalized_pb2.TaskGeneralizedStatus.TASK_GENERALIZED_STATUS_FINISHED,
        invalids=invalids_df,
    )

    # 检查结果是否为 TaskGeneralized 类型
    assert isinstance(result, task_generalized_pb2.TaskGeneralized)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查 concretes 是否正确设置
    assert len(result.concretes) == 2
    assert result.concretes[0].func_name == "Func1"
    assert result.concretes[0].logic_name == "Logic1"
    assert result.concretes[0].concrete_name == "Concrete1"
    assert result.concretes[0].labels == ["Label1"]
    assert result.concretes[0].description == "Description1"
    assert result.concretes[0].tree_scene_set.classifys == [settings.sys.classify.root, "Class1", "Class11"]

    assert result.concretes[1].func_name == "Func1"
    assert result.concretes[1].logic_name == "Logic2"
    assert result.concretes[1].concrete_name == "Concrete2"
    assert result.concretes[1].labels == ["Label2"]
    assert result.concretes[1].description == "Description2"
    assert result.concretes[1].tree_scene_set.classifys == [settings.sys.classify.root, "Class2"]

    # 检查 unqualifieds 是否正确设置
    assert len(result.unqualifieds) == 2
    assert result.unqualifieds[0].func_name == "ErrFunc1"
    assert result.unqualifieds[0].logic_name == "ErrLogic1"
    assert result.unqualifieds[0].concrete_name == "ErrConcrete1"
    assert result.unqualifieds[0].cause_analysis == "Reason1"
    assert result.unqualifieds[0].assessment_dimensions == "Dimension1"
    assert result.unqualifieds[0].status == "已剔除"

    assert result.unqualifieds[1].func_name == "ErrFunc1"
    assert result.unqualifieds[1].logic_name == "ErrLogic2"
    assert result.unqualifieds[1].concrete_name == "ErrConcrete2"
    assert result.unqualifieds[1].cause_analysis == "Reason2"
    assert result.unqualifieds[1].assessment_dimensions == "Dimension2"
    assert result.unqualifieds[1].status == "已剔除"

    # 检查任务状态是否为预期值
    assert result.status == task_generalized_pb2.TaskGeneralizedStatus.TASK_GENERALIZED_STATUS_FINISHED

    # 检查其他属性是否为空
    assert len(result.funcs) == 0
    assert len(result.logics) == 0
    assert len(result.maps) == 0
    assert result.func_number == 0
    assert result.logic_number == 0
    assert result.concrete_number == 0
    assert result.virtual_map_number == 0
    assert result.concrete_number_original == 0
    assert result.qualification_rate == 0


# --------------------------------- testcase ---------------------------------
def test_set_fail_task_testcaselist():
    obj = CloudApi()  # 使用提供的类名
    result = obj.set_fail_task_testcaselist()

    # 检查结果是否为 TaskTestcaseList 类型
    assert isinstance(result, task_testcase_pb2.TaskTestcaseList)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查任务状态是否为失败
    assert result.status == task_testcase_pb2.TaskTestcaseStatus.TASK_TESTCASE_STATUS_FAIL

    # 检查其他属性是否为空
    assert len(result.tasks) == 0


def test_process_task_testcaselist():
    # 创建示例 DataFrame
    task_data = {
        settings.sys.testcase.cols.simtype: ["WorldSim", "LogSim"],
        # settings.sys.testcase.cols.is_perception_in_loop: [True, False],
        settings.sys.testcase.cols.name: ["Name1", "Name2"],
        settings.sys.testcase.cols.task_description: ["Description1", "Description2"],
        settings.sys.testcase.cols.priority: ["S", "A"],
        settings.sys.testcase.cols.ego_name: ["Ego1", "Ego2"],
        settings.sys.testcase.cols.kpis: [{"name": "Kpi1"}, {"name": "Kpi2"}],
        settings.sys.testcase.cols.logicid: [["Logic1"], ["Logic2"]],
        settings.sys.testcase.cols.task_label: ["Label11,Label12", "Label21, Label22"],
        settings.sys.testcase.cols.project: ["Project1", "Project2"],
        settings.sys.testcase.cols.ego_controller: ["Controller1", "Controller2"],
        settings.sys.testcase.cols.sets: ["目录C1-目录C11", "目录C2-目录C21"],
        settings.sys.testcase.cols.concreteid: ["Concrete1", "Concrete2"],
        settings.sys.l0.cols.is_generalized: [[False], [False]],
    }
    task_df = pd.DataFrame(task_data)

    obj = CloudApi()  # 使用提供的类名
    result = obj.process_task_testcaselist(task_df)

    # 检查结果是否为 TaskTestcaseList 类型
    assert isinstance(result, task_testcase_pb2.TaskTestcaseList)

    # 检查 task_generalized_id 是否与预期相符
    assert result.task_generalized_id == obj.task_generalized_id

    # 检查 link_symbol 是否与预期相符
    assert result.link_symbol == settings.sys.link_symbol

    # 检查 tasks 是否正确设置
    assert len(result.tasks) == 2

    # 检查任务状态是否为默认值
    assert result.status == task_testcase_pb2.TaskTestcaseStatus.TASK_TESTCASE_STATUS_FINISHED
