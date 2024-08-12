#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file wrapper_xodr_factory.py
# * \brief .xodr file generator
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2023-05-13
# * \version 1.0.0
# * \copyright (c) 2023 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

import copy
import math
from dataclasses import dataclass
from typing import List, Tuple, Union

from loguru import logger
from scenariogeneration import xodr

from excel2asam.config import settings
from excel2asam.utils import KJunction, KLane, KMark, KRoad, value_dispatch


@dataclass(order=True)
class CreatPlanview:
    def _set_planview(self, geometrys: list) -> xodr.PlanView:
        planview = xodr.PlanView()
        if not isinstance(geometrys, list):
            geometrys = [geometrys]

        for geometry in geometrys:
            planview.add_geometry(geometry)
        return planview

    def _logger_division_by_zero_error(self, road_type: str, curve: float) -> None:
        epsilon = 1e-9
        if abs(curve) < epsilon:
            # if int(curve) == 0:
            logger.opt(lazy=True).error(f"道路类型为: {road_type}, 但是曲率为: 0")

    @value_dispatch
    def creat(self, ktype: str, *args: tuple) -> str:
        # 泛型函数,通过传入参数 (ktype) 的类型不同而返回不同的处理结果
        target = settings.sys.l1.RoadType.enum.values()
        return f"参数 RoadType 输入错误: {ktype} 仅应为 {target} 中其一"

    @creat.register(settings.sys.l1.RoadType.enum.straight)
    def creat_straight(self, ktype: str, kroad: KRoad) -> xodr.PlanView:
        geometrys = [
            # 直道
            xodr.Line(kroad.total_length)
        ]
        return self._set_planview(geometrys)

    @creat.register(settings.sys.l1.RoadType.enum.curvIng)
    def creat_curvIng(self, ktype: str, kroad: KRoad) -> xodr.PlanView:
        # 异常处理
        self._logger_division_by_zero_error(ktype, kroad.curve)
        #
        geometrys = [
            # 弯道
            xodr.Arc(curvature=(1 / kroad.curve), length=kroad.total_length)
        ]
        return self._set_planview(geometrys)

    @creat.register(settings.sys.l1.RoadType.enum.curveIn)
    def creat_curveIn(self, ktype: str, kroad: KRoad) -> xodr.PlanView:
        # 异常处理
        self._logger_division_by_zero_error(ktype, kroad.curve)

        len_straight = int(kroad.total_length * settings.sys.map.virtual.ratio_of_len_to_total_straight)

        geometrys = [
            # 直道
            xodr.Line(len_straight),
            # 弯道
            xodr.Arc(curvature=(1 / kroad.curve), length=kroad.total_length - len_straight),
        ]

        return self._set_planview(geometrys)

    @creat.register(settings.sys.l1.RoadType.enum.curveOut)
    def creat_curveOut(self, ktype: str, kroad: KRoad) -> xodr.PlanView:
        # 异常处理
        self._logger_division_by_zero_error(ktype, kroad.curve)

        len_straight = int(kroad.total_length * settings.sys.map.virtual.ratio_of_len_to_total_straight)

        geometrys = [
            # 弯道
            xodr.Arc(curvature=1 / kroad.curve, length=kroad.total_length - len_straight),
            # 直道
            xodr.Line(len_straight),
        ]
        return self._set_planview(geometrys)

    @creat.register(settings.sys.l1.RoadType.enum.curveS)
    def creat_curveS(self, ktype: str, kroad: KRoad) -> xodr.PlanView:
        # 异常处理
        self._logger_division_by_zero_error(ktype, kroad.curve)

        # 距离各为 1/3 的长度
        geometrys = [
            # 弯道正
            xodr.Arc(curvature=1 / kroad.curve, length=int(kroad.total_length / 3)),
            # 弯道负
            xodr.Arc(curvature=-1 / kroad.curve, length=int(kroad.total_length / 3)),
            # 弯道正
            xodr.Arc(curvature=1 / kroad.curve, length=int(kroad.total_length / 3)),
        ]
        return self._set_planview(geometrys)


@dataclass(order=True)
class CreatLanes:
    def _set_roadmark(self, kmark: KMark) -> xodr.RoadMark:
        mapping = {
            xodr.RoadMarkType.solid.name: xodr.std_roadmark_solid(),
            xodr.RoadMarkType.broken.name: xodr.std_roadmark_broken(),
            xodr.RoadMarkType.solid_solid.name: xodr.std_roadmark_solid_solid(),
            xodr.RoadMarkType.solid_broken.name: xodr.std_roadmark_solid_broken(),
            xodr.RoadMarkType.broken_broken.name: xodr.std_roadmark_broken_broken(),
            xodr.RoadMarkType.broken_solid.name: xodr.std_roadmark_broken_solid(),
        }

        roadmark = mapping[kmark.type]
        roadmark.color = xodr.RoadMarkColor[kmark.color]

        return roadmark

    def _set_lane_def(self, klane: KLane) -> Union[int, List]:
        """判断车道数量是否有变化, 创建 lane_def"""

        # 默认是没有变化, 直接使用 数量
        lane_def: Union[int, List] = klane.num

        # 如果车道数量有变化 (通过 klane.num2 是否为 0 判断), 则创建 [xodr.LaneDef]
        if klane.num2:
            # 考虑到主车所在的初始位置 start_s
            s_start = settings.sys.map.virtual.start_s + klane.num2dist

            #
            lane_def = [
                xodr.LaneDef(
                    s_start=s_start,
                    s_end=(
                        s_start + settings.sys.map.virtual.split_len_change
                        if klane.num2 > klane.num
                        else s_start + settings.sys.map.virtual.merge_len_change
                    ),
                    n_lanes_start=klane.num,
                    n_lanes_end=klane.num2,
                    sub_lane=-klane.num if klane.num > klane.num2 else -klane.num - 1,
                )
            ]

        return lane_def

    def creat(self, kroad: KRoad, klane: KLane, kmarks: List[KMark]) -> xodr.Lanes:
        # 获得 lane_def 设置
        lane_def = self._set_lane_def(klane)

        # 创建 lanes
        lanes = xodr.create_lanes_merge_split(
            # 默认创建右侧车道
            right_lane_def=lane_def,
            # 判断是否有反向车道, 如果有创建相同的
            left_lane_def=lane_def if kroad.direction == xodr.Direction.opposite.name else 0,
            road_length=kroad.total_length,
            center_road_mark=self._set_roadmark(kmarks[0]),
            lane_width=klane.width,
            lane_width_end=klane.width,
        )

        # 修改 lanes 中每一个车道的 roadmrk 设置为用户输入的 roadmrk
        for i in range(len(lanes.lanesections)):
            #
            for j in range(len(lanes.lanesections[i].rightlanes)):
                lanes.lanesections[i].rightlanes[j].lane_type = xodr.LaneType[klane.types[j]]
                lanes.lanesections[i].rightlanes[j].roadmark[0] = self._set_roadmark(kmarks[j + 1])
            #
            for j in range(len(lanes.lanesections[i].leftlanes)):
                lanes.lanesections[i].leftlanes[j].lane_type = xodr.LaneType[klane.types[j]]
                lanes.lanesections[i].leftlanes[j].roadmark[0] = self._set_roadmark(kmarks[j + 1])

        return lanes


@dataclass(order=True)
class CreatJunctionRoads:
    road_base: xodr.Road
    junction_num: int
    odrminor: int

    def __post_init__(self):
        #
        self.start_junction_id = settings.sys.map.virtual.start_junction_id
        self.start_road_id = settings.sys.map.virtual.start_road_id
        #
        self.rightlane_num = len(self.road_base.lanes.lanesections[-1].rightlanes)
        self.width = self.road_base.lanes.lanesections[-1].rightlanes[0].get_width(s=0)
        self.length = self.road_base.planview._raw_geometries[0].length / (self.junction_num + 1)
        self.speed_limit = self.road_base.types[0].speed

        #
        self.rule_ramp = self._define_mapping_rule_ramp()

        #
        self.radius = float(settings.sys.map.virtual.junction_radius)

    def _cal_curvature_acrlength(self, tangent_len: float, gradient_rate: float, entryexitlane_num: int) -> tuple:
        """基于渐变率公式计算曲率值和弧线长度"""
        # 如果渐变率为0, 则曲率无法计算
        epsilon = 1e-9
        if abs(gradient_rate) < epsilon:
            return 0.0, 0.0, 0.0

        # 计算切线的夹角(弧度)
        # 已知渐变率定义 https://blog.csdn.net/weixin_39901571/article/details/112622712
        # gradient_rate = (w3 - w2) / tangent_len
        w2 = (self.rightlane_num + entryexitlane_num) * self.width
        w3 = gradient_rate * tangent_len + w2
        tangent_angle = math.atan(w3 / tangent_len)

        # 计算半径
        radius = tangent_len / (2 * math.sin(tangent_angle) * math.cos(tangent_angle))

        # 计算曲率
        curvature = -1 / radius

        # 计算弧长 arc_length = radius * arc_angle)
        arc_length = radius * (2 * tangent_angle)

        print(f"{ tangent_angle = } { radius = }, { curvature = } {arc_length = }")

        return curvature, arc_length

    def _define_mapping_rule_ramp(self) -> dict:
        # 参考规范: https://zc.chd.edu.cn/_mediafile/zhangchi/2018/06/29/1ydea0hjxb.pdf
        # 表 10.2.5
        # 图 10.2.3 和 图 10.2.4
        return {
            "decel_onelane": {
                "120": {"l1": 145, "l2": 100, "gradient_rate": 0.04, "l3": 0, "l": 245},
                "100": {"l1": 125, "l2": 90, "gradient_rate": 0.04444, "l3": 0, "l": 215},
                "80": {"l1": 110, "l2": 80, "gradient_rate": 0.05, "l3": 0, "l": 190},
                "60": {"l1": 95, "l2": 70, "gradient_rate": 0.05714, "l3": 0, "l": 165},
            },
            "decel_twolane": {
                "120": {"l1": 225, "l2": 90, "gradient_rate": 0.04444, "l3": 300, "l": 615},
                "100": {"l1": 190, "l2": 80, "gradient_rate": 0.05, "l3": 250, "l": 520},
                "80": {"l1": 170, "l2": 70, "gradient_rate": 0.05714, "l3": 200, "l": 440},
                "60": {"l1": 140, "l2": 60, "gradient_rate": 0.06667, "l3": 180, "l": 380},
            },
            "accel_onelane_direct": {
                "120": {"l1": 230, "l2": 180, "gradient_rate": 0.02222, "l3": 0, "l": 410},
                "100": {"l1": 200, "l2": 160, "gradient_rate": 0.025, "l3": 0, "l": 360},
                "80": {"l1": 180, "l2": 160, "gradient_rate": 0.05714, "l3": 0, "l": 340},
                "60": {"l1": 155, "l2": 140, "gradient_rate": 0.06667, "l3": 0, "l": 295},
            },
            "accel_onelane_paralle": {
                "120": {"l1": 230, "l2": 90, "gradient_rate": 0.02222, "l3": 0, "l": 320},
                "100": {"l1": 200, "l2": 80, "gradient_rate": 0.025, "l3": 0, "l": 280},
                "80": {"l1": 180, "l2": 70, "gradient_rate": 0.05714, "l3": 0, "l": 250},
                "60": {"l1": 155, "l2": 60, "gradient_rate": 0.06667, "l3": 0, "l": 215},
            },
            "accel_twolane": {
                "120": {"l1": 400, "l2": 180, "gradient_rate": 0.02222, "l3": 400, "l": 980},
                "100": {"l1": 350, "l2": 160, "gradient_rate": 0.025, "l3": 350, "l": 860},
                "80": {"l1": 310, "l2": 150, "gradient_rate": 0.02667, "l3": 300, "l": 760},
                "60": {"l1": 270, "l2": 140, "gradient_rate": 0.02857, "l3": 250, "l": 660},
            },
        }

    def _copy_set_road_from_road_base(self, new_id: int, length: float) -> xodr.Road:
        road = copy.deepcopy(self.road_base)
        road.id = new_id
        road.planview._raw_geometries[0].length = length
        return road

    def _set_lane(self, params: list, roadmark: xodr.RoadMark = xodr.std_roadmark_solid()) -> xodr.Lane:
        # 配置车道
        rd2_lane = xodr.Lane()
        rd2_lane.add_roadmark(roadmark)
        for param in params:
            rd2_lane.add_lane_width(a=param["a"], b=param["b"], soffset=param["soffset"])
            rd2_lane.lane_type = param["type"]

        return rd2_lane

    def _set_junctioners_roads_intersection(self, kjunction: KJunction, angles: list, radii: list) -> Tuple[List, List]:
        # 初始化
        roads = []
        junctioner = xodr.CommonJunctionCreator(id=self.start_junction_id, name=kjunction.type)

        # 循环遍历创建 junctioner 和 roads
        for i, (angle, radius) in enumerate(zip(angles, radii)):
            # 设置 road id, 每增加一条路, id 加 1
            road_id = self.start_road_id + i

            # object 和 signal id 需要全局唯一, 不能相同
            obj_id = settings.sys.map.virtual.start_object_id
            sig_id = settings.sys.map.virtual.start_signal_id

            # 复制 base_road 属性, 然后修改 id 和 length
            road = self._copy_set_road_from_road_base(new_id=road_id, length=self.length)

            #
            hdg = -1.571
            h_offset = 0.0
            t_obj = self.rightlane_num * self.width + 1
            t_sig = t_obj / 2
            s = self.length

            # 如果需要, 添加横杆和信号灯到 road 上
            if kjunction.trafficlight_status:
                # 创建横杆
                obj = xodr.Object(
                    s=s,
                    t=t_obj,
                    Type="pole",
                    # TODO(xingboliu): Adjust OpenDRIVE 1.4
                    subtype=None,
                    id=obj_id,
                    name="Cross_Pole",
                    # TODO(xingboliu): Adjust OpenDRIVE 1.4
                    dynamic=xodr.Dynamic.yes,
                    # dynamic=(None if self.odrminor == 4 else xodr.Dynamic.yes),
                    zOffset=0,
                    orientation=xodr.Orientation.positive,
                    hdg=hdg,
                    pitch=0,
                    roll=0,
                    width=0,
                    length=0,
                    height=0,
                    radius=None,
                    validLength=0.0,
                )

                # 创建 信号灯 并绑定与 横杆 关系
                sig = xodr.Signal(
                    s=s,
                    t=t_sig,
                    country=settings.sys.map.virtual.country,
                    countryRevision=settings.sys.map.virtual.country_revision,
                    Type="1000001",
                    # TODO(xingboliu): Adjust OpenDRIVE 1.4
                    subtype="-1",
                    # subtype=(None if self.odrminor == 4 else "-1"),
                    id=sig_id,
                    name="verticalOmnidirectionalLight",
                    # TODO(xingboliu): Adjust OpenDRIVE 1.4
                    dynamic=xodr.Dynamic.yes,
                    # dynamic=(None if self.odrminor == 4 else xodr.Dynamic.yes),
                    value=None,
                    unit=None,
                    zOffset=5.95,
                    orientation=xodr.Orientation.positive,
                    hOffset=h_offset,
                    pitch=0,
                    roll=0,
                    height=1.331,
                    width=0.481,
                )

                #  计算 relate_control
                relate_control_value = sig_id - 1 - 2 * i
                relate_control = 100000000 + relate_control_value if i < 3 else relate_control_value
                # for tadsim 自定义
                sig.add_userdata(xodr.UserData("pole_id", str(obj_id)))
                sig.add_userdata(xodr.UserData("relate_road", str(road_id)))
                sig.add_userdata(xodr.UserData("relate_control", str(relate_control)))

                # 将 横杆 和 信号灯 添加到 road
                road.add_object(obj)
                road.add_signal(sig)

                # 每次循环将 setting 全局变量加一, 以保证 id 不重复
                settings.sys.map.virtual.start_object_id += 1
                settings.sys.map.virtual.start_signal_id += 1

            # junctioner 增加信息
            junctioner.add_incoming_road_circular_geometry(
                road=road, radius=radius, angle=angle, road_connection="successor"
            )

            # junctioner add connection to all previous roads
            for j in range(i):
                junctioner.add_connection(road_one_id=(self.start_road_id + j), road_two_id=(road_id))

            # 添加 road 到 roads
            roads.append(road)

        junctioner.add_constant_elevation(0)

        return [junctioner], roads

    def _set_junctioners_roads_entryexit_by_direct(self, mode: str, roads: List[xodr.Road]) -> List:
        logger.opt(lazy=True).debug("_set_junctioners_roads_entryexit_by_direct")
        #
        rd1 = roads[0]
        rd2 = roads[1]
        rd3 = roads[2]
        rd4 = roads[3]

        # 判别道路是入口, 直道, 还是出口
        names = ("Converging", "Straight") if mode == "Converging" else ("Straight", "Diverging")

        # 创建 junctioner1
        rd1.add_successor(xodr.ElementType.junction, self.start_junction_id)
        rd2.add_predecessor(xodr.ElementType.junction, self.start_junction_id)
        junctioner1 = xodr.DirectJunctionCreator(id=self.start_junction_id, name=names[0])
        junctioner1.add_connection(incoming_road=rd1, linked_road=rd2)

        # 创建 junctioner2
        rd2.add_successor(xodr.ElementType.junction, self.start_junction_id + 1)
        rd3.add_predecessor(xodr.ElementType.junction, self.start_junction_id + 1)
        junctioner2 = xodr.DirectJunctionCreator(id=self.start_junction_id + 1, name=names[1])
        junctioner2.add_connection(incoming_road=rd2, linked_road=rd3)

        # 对于入口或出口分开设置
        if mode == "Converging":
            rd4.add_successor(xodr.ElementType.junction, self.start_junction_id)
            junctioner1.add_connection(
                incoming_road=rd4,
                linked_road=rd2,
                incoming_lane_ids=-1,
                linked_lane_ids=-self.rightlane_num - 1,
            )
        else:
            rd4.add_predecessor(xodr.ElementType.junction, self.start_junction_id + 1)
            junctioner2.add_connection(
                incoming_road=rd2,
                linked_road=rd4,
                incoming_lane_ids=-self.rightlane_num - 1,
                linked_lane_ids=-1,
            )

        return [junctioner1, junctioner2]

    def _set_junctioners_roads_entryexit_by_common(self, mode: str, roads: List[xodr.Road]) -> List:
        logger.opt(lazy=True).debug("_set_junctioners_roads_entryexit_by_common")
        #
        rd1 = roads[0]
        rd2 = roads[1]
        rd3 = roads[2]
        rd4 = roads[3]

        # 判别道路是入口, 直道, 还是出口
        names = ("Converging", "Straight") if mode == "Converging" else ("Straight", "Diverging")

        #

        rd4_rightlane_num = len(rd4.lanes.lanesections[-1].rightlanes)

        a = (self.rightlane_num + rd4_rightlane_num / 2) * self.width

        b = self.radius

        radius = round(math.sqrt(a**2 + b**2), 3)
        angle = round(math.atan(a / b), 3)
        print(f"{angle = } {radius = }")

        # 创建 junctioner1
        junctioner1_id = self.start_junction_id
        junctioner1 = xodr.CommonJunctionCreator(id=junctioner1_id, name=names[0], startnum=junctioner1_id)
        junctioner1.add_incoming_road_circular_geometry(rd1, self.radius, math.pi, "successor")
        junctioner1.add_incoming_road_circular_geometry(rd2, self.radius, 0, "predecessor")
        junctioner1.add_connection(road_one_id=rd2.id, road_two_id=rd1.id)

        # 创建 junctioner2
        junctioner2_id = self.start_junction_id + 100
        junctioner2 = xodr.CommonJunctionCreator(id=junctioner2_id, name=names[1], startnum=junctioner2_id)
        junctioner2.add_incoming_road_circular_geometry(rd2, self.radius, math.pi, "successor")
        junctioner2.add_incoming_road_circular_geometry(rd3, self.radius, 0, "predecessor")
        junctioner2.add_connection(road_one_id=rd3.id, road_two_id=rd2.id)

        # 对于入口或出口分开设置
        rd4_rightlane_num = len(rd4.lanes.lanesections[-1].rightlanes)
        if mode == "Converging":
            for i in range(0, rd4_rightlane_num):
                rd4.lanes.lanesections[-1].rightlanes[i].lane_type = xodr.LaneType.entry
            junctioner1.add_incoming_road_circular_geometry(rd4, radius, math.pi + angle, "successor")
            junctioner1.add_connection(
                road_one_id=rd2.id, road_two_id=rd4.id, lane_one_id=-self.rightlane_num - 1, lane_two_id=-1
            )
        else:
            for i in range(0, rd4_rightlane_num):
                rd4.lanes.lanesections[-1].rightlanes[i].lane_type = xodr.LaneType.exit
            junctioner2.add_incoming_road_circular_geometry(rd4, self.radius, 2 * math.pi - angle, "predecessor")
            junctioner2.add_connection(
                road_one_id=rd4.id, road_two_id=rd2.id, lane_one_id=-1, lane_two_id=-self.rightlane_num - 1
            )

        # 处理高程设置为 0
        junctioner1.add_constant_elevation(0)
        junctioner2.add_constant_elevation(0)

        return [junctioner1, junctioner2]

    def _set_junctioners_roads_entryexit(
        self,
        mode: str,
        r2_len: float,
        rd2_lanes: list,
        rd4_geometry: list,
        rd4_rightlanes: Union[List[xodr.LaneDef], int],
    ) -> Tuple[List, List]:
        # 创建 3 条 Road, 并修改中间的 rd2
        rd1 = self._copy_set_road_from_road_base(self.start_road_id, self.length)
        rd2 = self._copy_set_road_from_road_base(self.start_road_id + 1, r2_len)
        rd2.lanes.lanesections[0].rightlanes[-1].roadmark[0] = xodr.std_roadmark_broken()
        for rd2_lane in rd2_lanes:
            rd2.lanes.lanesections[0].add_right_lane(rd2_lane)
        rd3 = self._copy_set_road_from_road_base(self.start_road_id + 2, self.length)

        # 创建入口或出口的道路, 如果多条车道, 中间为虚线, 并设置高程为 0
        rd4 = xodr.create_road(
            geometry=rd4_geometry,
            id=self.start_road_id + 3,
            left_lanes=0,
            right_lanes=rd4_rightlanes,
            lane_width=self.width,
        )
        for i in range(len(rd4.lanes.lanesections)):
            if len(rd4.lanes.lanesections[i].rightlanes) > 1:
                rd4.lanes.lanesections[i].rightlanes[0].roadmark[0] = xodr.std_roadmark_broken()
        rd4.add_elevation(s=0.0, a=0.0, b=0.0, c=0.0, d=0.0)

        # 创建 roads
        roads = [rd1, rd2, rd3, rd4]

        # 创建 junctioners
        junctioners = (
            self._set_junctioners_roads_entryexit_by_direct(mode, roads)
            if self.odrminor >= 7
            else self._set_junctioners_roads_entryexit_by_common(mode, roads)
        )

        return junctioners, roads

    @value_dispatch
    def creat(self, ktype: str, kjunction: KJunction) -> str:
        # 泛型函数,通过传入参数 (ktype) 的类型不同而返回不同的处理结果
        target = settings.sys.l1.JunctionType.enum.values()

        return f"参数 JunctionType 输入错误: {ktype} 仅应为 {target}中其一"

    @creat.register(settings.sys.l1.JunctionType.enum.crossroad)
    def creat_crossroad(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.crossroad")

        angles = [0, math.pi / 2, 3 * math.pi / 2, math.pi]
        radii = [self.radius] * len(angles)

        return self._set_junctioners_roads_intersection(kjunction, angles, radii)

    @creat.register(settings.sys.l1.JunctionType.enum.tRd)
    def creat_t_rd(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.tRd")

        angles = [0, math.pi / 2, 3 * math.pi / 2]
        radii = [self.radius] * len(angles)

        return self._set_junctioners_roads_intersection(kjunction, angles, radii)

    @creat.register(settings.sys.l1.JunctionType.enum.yJunc)
    def creat_y_junc(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.yJunc")

        angles = [0, 1.3 * math.pi / 2, 3.2 * math.pi / 2]
        radii = [self.radius] * len(angles)

        return self._set_junctioners_roads_intersection(kjunction, angles, radii)

    # TODO: implement
    @creat.register(settings.sys.l1.JunctionType.enum.analogousConvergingTwolane)
    def creat_analogous_converging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.analogousConvergingTwolane")

        # TODO: implement
        return [], []

    # TODO: implement
    @creat.register(settings.sys.l1.JunctionType.enum.analogousConverging)
    def creat_analogousConverging(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.analogousConverging")

        # TODO: implement
        return [], []

    @creat.register(settings.sys.l1.JunctionType.enum.directConvergingTwolane)
    def creat_direct_converging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.directConvergingTwolane")

        # 规范定义
        # rule = {"120": {"l1": 400, "l2": 180, "gradient_rate": 0.02222, "l3": 400, "l": 980}}
        rule = self.rule_ramp["accel_twolane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        accel_len = rule_by_spd["l1"]
        keeping_len = rule_by_spd["l3"]
        converging_len = rule_by_spd["l2"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=accel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=1
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Converging",
            r2_len=(keeping_len + converging_len),
            rd2_lanes=[
                self._set_lane(
                    [
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": 0,
                            "type": xodr.LaneType.driving,
                        },
                        # 减少阶段
                        {
                            "a": self.width,
                            "b": -(self.width / converging_len),
                            "soffset": keeping_len,
                            "type": xodr.LaneType.entry,
                        },
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Line(length=self.length - arclength),
                xodr.Arc(curvature=curvature, length=arclength),
            ],
            rd4_rightlanes=[
                xodr.LaneDef(
                    s_start=(self.length - arclength * 3 / 4),
                    s_end=self.length,
                    n_lanes_start=2,
                    n_lanes_end=1,
                    sub_lane=-1,
                )
            ],
        )

    @creat.register(settings.sys.l1.JunctionType.enum.directConverging)
    def creat_direct_converging(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.directConverging")

        # 规范定义
        rule = self.rule_ramp["accel_onelane_direct"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        accel_len = rule_by_spd["l1"]
        converging_len = rule_by_spd["l2"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=accel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=1
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Converging",
            r2_len=converging_len,
            rd2_lanes=[
                self._set_lane(
                    [
                        # 减少阶段
                        {
                            "a": self.width,
                            "b": -(self.width / converging_len),
                            "soffset": 0,
                            "type": xodr.LaneType.entry,
                        }
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Line(self.length - arclength),
                xodr.Arc(curvature=curvature, length=arclength),
            ],
            rd4_rightlanes=1,
        )

    @creat.register(settings.sys.l1.JunctionType.enum.parallelConvergingTwolane)
    def creat_parallel_converging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.parallelConvergingTwolane")

        # 规范定义
        # rule = {"120": {"l1": 400, "l2": 180, "gradient_rate": 0.02222, "l3": 400, "l": 980}}
        rule = self.rule_ramp["accel_twolane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        accel_len = rule_by_spd["l1"] / 2
        keeping_len = rule_by_spd["l1"] / 2
        converging_len = rule_by_spd["l2"]
        keeping_len2 = rule_by_spd["l3"]
        converging_len2 = rule_by_spd["l2"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=accel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=2
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Converging",
            r2_len=(keeping_len + converging_len + keeping_len2 + converging_len2),
            rd2_lanes=[
                self._set_lane(
                    params=[
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": 0,
                            "type": xodr.LaneType.driving,
                        },
                        # 减少阶段
                        {
                            "a": self.width,
                            "b": -(self.width / converging_len2),
                            "soffset": (keeping_len + converging_len + keeping_len2),
                            "type": xodr.LaneType.entry,
                        },
                    ],
                    roadmark=xodr.std_roadmark_broken(),
                ),
                self._set_lane(
                    [
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": 0,
                            "type": xodr.LaneType.driving,
                        },
                        # 减少阶段
                        {
                            "a": self.width,
                            "b": -(self.width / converging_len),
                            "soffset": keeping_len,
                            "type": xodr.LaneType.entry,
                        },
                        # 消失阶段
                        {
                            "a": 0,
                            "b": 0,
                            "soffset": (keeping_len + converging_len),
                            "type": xodr.LaneType.entry,
                        },
                    ]
                ),
            ],
            rd4_geometry=[
                xodr.Line(self.length - arclength),
                xodr.Arc(curvature=curvature, length=arclength),
            ],
            rd4_rightlanes=2,
        )

    @creat.register(settings.sys.l1.JunctionType.enum.parallelConverging)
    def creat_parallel_converging(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.parallelConverging")

        # 规范定义
        # rule = {"120": {"l1": 230, "l2": 90, "gradient_rate": 0.02222, "l3": 0, "l": 320}}
        rule = self.rule_ramp["accel_onelane_paralle"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        accel_len = rule_by_spd["l1"] / 2
        keeping_len = rule_by_spd["l1"] / 2
        converging_len = rule_by_spd["l2"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=accel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=1
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Converging",
            r2_len=(keeping_len + converging_len),
            rd2_lanes=[
                self._set_lane(
                    [
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": 0,
                            "type": xodr.LaneType.driving,
                        },
                        # 减少阶段
                        {
                            "a": self.width,
                            "b": -(self.width / converging_len),
                            "soffset": keeping_len,
                            "type": xodr.LaneType.entry,
                        },
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Line(self.length - arclength),
                xodr.Arc(curvature=curvature, length=arclength),
            ],
            rd4_rightlanes=1,
        )

    # TODO: implement
    @creat.register(settings.sys.l1.JunctionType.enum.analogousDivergingTwolane)
    def creat_analogous_diverging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.analogousDivergingTwolane")

        # TODO: implement
        return [], []

    # TODO: implement
    @creat.register(settings.sys.l1.JunctionType.enum.analogousDiverging)
    def creat_analogous_diverging(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.analogousDiverging")

        # TODO: implement
        return [], []

    @creat.register(settings.sys.l1.JunctionType.enum.directDivergingTwolane)
    def creat_direct_diverging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        # 参考 https://zc.chd.edu.cn/_mediafile/zhangchi/2018/06/29/1ydea0hjxb.pdf
        logger.opt(lazy=True).debug("Creating JunctionType.enum.directDivergingTwolane")

        # 规范定义
        rule = self.rule_ramp["decel_twolane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        diverging_len = rule_by_spd["l2"]
        keeping_len = rule_by_spd["l3"]
        decel_len = rule_by_spd["l1"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=decel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=2
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Diverging",
            r2_len=(diverging_len + keeping_len),
            rd2_lanes=[
                self._set_lane(
                    [
                        # 增加阶段
                        {
                            "a": 0,
                            "b": (self.width / diverging_len),
                            "soffset": 0,
                            "type": xodr.LaneType.exit,
                        },
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": diverging_len,
                            "type": xodr.LaneType.entry,
                        },
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Arc(curvature=curvature, length=arclength),
                xodr.Line(self.length - arclength),
            ],
            rd4_rightlanes=[
                xodr.LaneDef(
                    s_start=0.01,
                    s_end=(decel_len / 2),
                    n_lanes_start=1,
                    n_lanes_end=2,
                    sub_lane=-1,
                )
            ],
        )

    @creat.register(settings.sys.l1.JunctionType.enum.directDiverging)
    def creat_direct_diverging(self, ktype: str, kjunction: KJunction) -> Tuple:
        # 参考 https://zc.chd.edu.cn/_mediafile/zhangchi/2018/06/29/1ydea0hjxb.pdf
        logger.opt(lazy=True).debug("Creating JunctionType.enum.directDiverging")

        # 规范定义
        # rule = {"120": {"l1": 145, "l2": 100, "gradient_rate": 0.04, "l3": 0, "l": 245}}
        rule = self.rule_ramp["decel_onelane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        diverging_len = rule_by_spd["l2"]
        decel_len = rule_by_spd["l1"]

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=decel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=1
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Diverging",
            r2_len=diverging_len,
            rd2_lanes=[
                self._set_lane(
                    [
                        # 增加阶段
                        {
                            "a": 0,
                            "b": (self.width / diverging_len),
                            "soffset": 0,
                            "type": xodr.LaneType.exit,
                        }
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Arc(curvature=curvature, length=arclength),
                xodr.Line(self.length - decel_len),
            ],
            rd4_rightlanes=1,
        )

    @creat.register(settings.sys.l1.JunctionType.enum.parallelDivergingTwolane)
    def creat_parallel_diverging_twolane(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.parallelDivergingTwolane")

        # 规范定义
        rule = self.rule_ramp["decel_twolane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        diverging_len = rule_by_spd["l2"]
        keeping_len = rule_by_spd["l3"]
        diverging_len2 = rule_by_spd["l2"]
        keeping_len2 = (rule_by_spd["l1"] - rule_by_spd["l2"]) / 2
        decel_len = keeping_len2

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=decel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=2
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Diverging",
            r2_len=(diverging_len + keeping_len + keeping_len2 + keeping_len2),
            rd2_lanes=[
                self._set_lane(
                    params=[
                        # 增加阶段
                        {
                            "a": 0,
                            "b": (self.width / diverging_len),
                            "soffset": 0,
                            "type": xodr.LaneType.exit,
                        },
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": diverging_len,
                            "type": xodr.LaneType.driving,
                        },
                    ],
                    roadmark=xodr.std_roadmark_broken(),
                ),
                self._set_lane(
                    [
                        # 增加阶段
                        {
                            "a": 0,
                            "b": (self.width / diverging_len2),
                            "soffset": (diverging_len + keeping_len),
                            "type": xodr.LaneType.exit,
                        },
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": (diverging_len + keeping_len + diverging_len2),
                            "type": xodr.LaneType.driving,
                        },
                    ]
                ),
            ],
            rd4_geometry=[
                xodr.Arc(curvature=curvature, length=arclength),
                xodr.Line(self.length - decel_len),
            ],
            rd4_rightlanes=2,
        )

    @creat.register(settings.sys.l1.JunctionType.enum.parallelDiverging)
    def creat_parallel_diverging(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.parallelDiverging")

        # 规范定义
        rule = self.rule_ramp["decel_onelane"]

        # 通过速度筛选参数
        rule_by_spd = rule["120"]

        # 获得不同阶段参数
        diverging_len = rule_by_spd["l2"]
        keeping_len = rule_by_spd["l1"] / 2
        decel_len = rule_by_spd["l1"] / 2

        # 计算曲率和弧长
        curvature, arclength = self._cal_curvature_acrlength(
            tangent_len=decel_len, gradient_rate=rule_by_spd["gradient_rate"], entryexitlane_num=1
        )

        #
        return self._set_junctioners_roads_entryexit(
            mode="Diverging",
            r2_len=(diverging_len + keeping_len),
            rd2_lanes=[
                self._set_lane(
                    [
                        # 增加阶段
                        {
                            "a": 0,
                            "b": (self.width / diverging_len),
                            "soffset": 0,
                            "type": xodr.LaneType.exit,
                        },
                        # 平行阶段
                        {
                            "a": self.width,
                            "b": 0,
                            "soffset": diverging_len,
                            "type": xodr.LaneType.driving,
                        },
                    ]
                )
            ],
            rd4_geometry=[
                xodr.Arc(curvature=curvature, length=arclength),
                xodr.Line(self.length - decel_len),
            ],
            rd4_rightlanes=1,
        )

    # TODO: implement
    @creat.register(settings.sys.l1.JunctionType.enum.roundabout)
    def creat_roundabout(self, ktype: str, kjunction: KJunction) -> Tuple:
        logger.opt(lazy=True).debug("Creating JunctionType.enum.roundabout")

        # TODO: implement
        return [], []


# sourcery skip: remove-empty-nested-block, remove-redundant-if
if __name__ == "__main__":
    pass
