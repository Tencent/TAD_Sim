#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file wrapper_xodr.py
# * \brief Generate xodr file conforming to OpenDRIVE (V1.4) standard.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2022-12-27
# * \version 1.0.0
# * \copyright (c) 2022 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Callable, List, Tuple

from scenariogeneration import xodr

from excel2asam.config import settings
from excel2asam.openx.wrapper_xodr_factory import CreatJunctionRoads, CreatLanes, CreatPlanview
from excel2asam.utils import KJunction, KLane, KMark, KRoad


@dataclass(order=True)
class WrapperRoad:
    """[summary]"""

    odrminor: int

    def __post_init__(self):
        self.mapping_road_type = {
            settings.sys.l1.RoadType.enum.straight: xodr.RoadType.motorway,
            settings.sys.l1.RoadType.enum.curveIn: xodr.RoadType.motorway,
            settings.sys.l1.RoadType.enum.curveOut: xodr.RoadType.motorway,
            settings.sys.l1.RoadType.enum.curveS: xodr.RoadType.motorway,
            settings.sys.l1.RoadType.enum.curvIng: xodr.RoadType.motorway,
        }

    def set_road(self, kroad: KRoad, klane: KLane, kmarks: List[KMark], road_type: int = -1) -> None:
        # Create Road from Planview and Lanes
        self.road = xodr.Road(
            road_id=settings.sys.map.virtual.start_road_id,
            planview=CreatPlanview().creat(kroad.type, kroad),
            lanes=CreatLanes().creat(kroad, klane, kmarks),
            road_type=road_type,
            name="",
            rule=xodr.TrafficRule.RHT,
        )

        speed = int(kroad.speedlimit) if kroad.speedlimit else None
        self.road.add_type(road_type=self.mapping_road_type[kroad.type], s=0, speed=speed, speed_unit="m/s")

        # elevationProfile 下需要有 elevation
        self.road.add_elevation(s=0.0, a=0.0, b=0.0, c=0.0, d=0.0)

        # OpenDRIVE 1.4 版本
        if self.odrminor <= 4:
            # Road 中没有 rule 属性
            self.road.rule = None

    def get_result(self) -> xodr.Road:
        return self.road


@dataclass(order=True)
class WrapperJunctionersRoads:
    """[summary]"""

    odrminor: int

    def __post_init__(self):
        pass

    def set_junctioners_roads(self, kjunction: KJunction, road: xodr.Road) -> None:
        # self.roads, self.directjunction = CreatJunctionRoads().creat(kjunction.type, road)
        self.junctioners, self.roads = [], []
        if kjunction.type != settings.sys.l1.JunctionType.enum.none or kjunction.num > 0:
            cjr = CreatJunctionRoads(road, kjunction.num, self.odrminor)
            self.junctioners, self.roads = cjr.creat(kjunction.type, kjunction)

    def get_result(self) -> Tuple:
        return self.junctioners, self.roads


@dataclass(order=True)
class GenXodr:
    """Generate xodr file conforming to OpenDRIVE standard."""

    gen_folder: Path
    odrmajor: int
    odrminor: int

    def __post_init__(self):
        pass

    def _format_road(self, scene: dict) -> KRoad:
        """
        The function _format_road takes a scene dictionary as input and returns a KRoad object with the specified
        attributes.

        Args:
          scene (dict): The `scene` parameter is a dictionary that contains information about a road scene.

        Returns:
          an instance of the `KRoad` class.
        """
        return KRoad(
            type=scene["Road.Type"],
            curve=scene["Road.Curve"],
            direction=scene["Road.Direction"],
            speedlimit=(
                scene["Road.Speedlimit"] if "Road.Speedlimit" in scene and scene["Road.Speedlimit"] != "none" else 0.0
            ),
            total_length=scene[settings.sys.l0.cols.total_length],
        )

    def _format_junction(self, scene: dict) -> KJunction:
        """
        The function takes a scene dictionary as input and returns a KJunction object with the type and number extracted
        from the scene dictionary.

        Args:
          scene (dict): The `scene` parameter is a dictionary that contains information about a junction in a scene.

        Returns:
          an instance of the `KJunction` class.
        """
        return KJunction(
            type=scene["Junction.Type"],
            num=scene["Junction.Num"],
            trafficlight_status=scene["Trafficlight.Status"],
            trafficlight_cycle=scene["Trafficlight.Cycle"],
        )

    def _format_lane(self, scene: dict) -> KLane:
        """
        The function `_format_lane` takes a scene dictionary as input and returns a `KLane` object with formatted lane
        information.

        Args:
          scene (dict): The `scene` parameter is a dictionary that contains information about a lane in a scene.

        Returns:
          an instance of the `KLane` class.
        """
        lanetype_values_re = "|".join(str(c.name) for c in xodr.LaneType) if scene.get("Lane.Type") else None

        target_num = max(scene["Lane.Num"], scene["Lane.Num2"])

        # 处理已定义的车道线
        if lanetype_values_re:
            comp_lanetype = re.compile(rf"({lanetype_values_re}+)")
            lanetypes = comp_lanetype.findall(scene["Lane.Type"])

            actual_num = len(lanetypes)

            # 获取车道类型
            lanetypes = (
                lanetypes[:target_num]
                if actual_num >= target_num
                else lanetypes + [xodr.LaneType.driving.name] * (target_num - actual_num)
            )
        # 处理未定义车道线 (车道线类型数量和 ！= 车道数量 + 1)
        else:
            lanetypes = [xodr.LaneType.driving.name] * target_num

        return KLane(
            types=lanetypes,
            num=scene["Lane.Num"],
            num2=scene["Lane.Num2"],
            num2dist=scene["Lane.Num2Dist"],
            width=scene["Lane.Width"],
            edge=scene["Lane.Edge"],
            # TODO(xingboliu): xodr 暂无该接口
            # speedlimit=scene["Lane.Speedlimit"],
        )

    def _format_lanemarks(self, scene: dict) -> List[KMark]:
        target_num = max(scene["Lane.Num"], scene["Lane.Num2"]) + 1

        if scene["Marking.Type"] and scene["Marking.Type"] != "none":
            # 使用列表推导式代替 map 和 join 函数
            road_mark_type_values_re = "|".join(str(c.name) for c in xodr.RoadMarkType)
            road_mark_color_values_re = "|".join(str(c.name) for c in xodr.RoadMarkColor)
            comp_marks = re.compile(rf"({road_mark_type_values_re})({road_mark_color_values_re})+")

            # 定义的所有车道线信息
            marks = comp_marks.findall(scene["Marking.Type"])

            # 处理未完全按照车道数量定义车道线信息的情况
            actual_num = len(marks)
            if target_num < actual_num:
                marks = marks[:target_num]
            else:
                # 未填写的车道线补充为 虚白, 最右侧为 实白
                marks.extend(
                    [(xodr.RoadMarkType.broken.name, xodr.RoadMarkColor.white.name)] * (target_num - actual_num - 1)
                )
                marks.append((xodr.RoadMarkType.solid.name, xodr.RoadMarkColor.white.name))
        # 处理未定义车道线信息的情况, 最左侧和最右侧为 实白, 其余中间为 虚白
        else:
            # 最左侧 - 实白
            marks = [(xodr.RoadMarkType.solid.name, xodr.RoadMarkColor.white.name)]
            # 中间 - 虚白
            marks.extend([(xodr.RoadMarkType.broken.name, xodr.RoadMarkColor.white.name)] * (target_num - 2))
            # 最右侧 - 实白
            marks.append((xodr.RoadMarkType.solid.name, xodr.RoadMarkColor.white.name))

        return [KMark(type=mark[0], color=mark[1], status=scene["Marking.Status"]) for mark in marks]

    def generate(self, scene: dict, func: Callable[..., Any] = None) -> None:
        # 获得生成的虚拟地图名称
        name = scene[settings.sys.l0.cols.mapfile]
        print("\n ############################################# ")
        print(f"generating {name} ...")
        print(f"{scene['LogicId'] = }")

        # 创建 xodr.OpenDrive 实例化对象
        odr = xodr.OpenDrive(
            name=name.replace(settings.sys.map.virtual.suffix, ""),
            revMajor=f"{self.odrmajor}",
            revMinor=f"{self.odrminor}",
            geo_reference=settings.sys.map.virtual.geo_reference,
        )

        # 获得基础 road
        wr = WrapperRoad(self.odrminor)
        wr.set_road(self._format_road(scene), self._format_lane(scene), self._format_lanemarks(scene))
        road_base = wr.get_result()

        # 获得 roads 和 junctioners
        print(f"{self.odrminor = }")
        wjr = WrapperJunctionersRoads(self.odrminor)
        wjr.set_junctioners_roads(self._format_junction(scene), road_base)
        junctioners, roads = wjr.get_result()

        #
        if not junctioners and not roads:
            print("no junctioner and no roads, use road_base")
            odr.add_road(road_base)

        # print(f"{len(roads) = }")
        # odr.add_road(roads[-1])

        # 存在 roads 时
        for road in roads:
            print(f"adding road {road.id} ...")
            odr.add_road(road)

        # 存在 junctioners 时
        for i, junctioner in enumerate(junctioners, start=1):
            # for road in junctioner.junction_roads:
            #     print(f"junctioner{i} adding road {road.id} ...")
            odr.add_junction_creator(junctioner)

        # 自调整 roads_and_lanes
        odr.adjust_roads_and_lanes()

        # adjust the remaining elevations
        # odr.adjust_elevations()

        # adjust the roadmarks
        # odr.adjust_roadmarks()

        # 写入文件
        odr.write_xml(filename=self.gen_folder / f"{name}", prettyprint=True)

        # 如果用户传入了回调函数，就执行它
        if func is not None:
            func(scene)


# sourcery skip: remove-empty-nested-block, remove-redundant-if
if __name__ == "__main__":
    pass
