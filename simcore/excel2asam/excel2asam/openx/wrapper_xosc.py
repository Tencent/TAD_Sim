#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file wrapper_xosc.py
# * \brief Generate xosc file conforming to OpenSCENARIO (V1.0.0) standard.
# *
# * Used facade design patterns to support WorldPosition
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2021-11-30
# * \version 1.1.0
# * \copyright (c) 2021 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable, Dict, List, Optional, Union

from loguru import logger
from scenariogeneration import xosc

from excel2asam.config import settings
from excel2asam.map.lib.hadmap_py import Waypoint
from excel2asam.openx.wrapper_xosc_factory import CreatAction, CreatTrigger
from excel2asam.utils import KAction, KEvent, KTrigger, WrapperCatalogLoader, check_add_suffix

# TODO: 扩展 scenariogeneration 能力, 待第三方库具备这个能力后可删除
xosc.CatalogLoader.load_catalog = WrapperCatalogLoader.load_catalog


@dataclass(order=True)
class TADSimTrafficSignalAuto:
    """# 信控规则"""

    # 信控周期
    cycle: int
    # 路口的进口道路数量, T型路口3,十字路口4
    leg: int
    # 黄灯默认
    yellow: int = field(default=5)

    def __post_init__(self):
        # 绿灯, 黄灯, 红灯时长
        self.green, self.yellow, self.red = self._cal_green_yellow_red(self.cycle)
        # 相位总时长
        self.phase = (self.green + self.yellow + self.red) / self.leg
        #
        self.id_init = 10001

    def _cal_green_yellow_red(self, cycle: int):
        """按照自定义的生成规则生成:
        - 已知:
            用户输入 cycle周期
        - 规则:
            - 公式 cycle = red + green + yellow
            - 类型 cycle, red, green, yellow 都是int
            - yellow 是固定值
            - green 需要大于等于 red, 且在 1倍到 1.5 之间
        - 异常:
            - 全部设置为 leg
        """
        try:
            if not isinstance(cycle, int) or not isinstance(self.yellow, int):
                raise ValueError("Both cycle and self.yellow must be integers.")
            if cycle <= self.yellow:
                raise ValueError("Cycle must be greater than self.yellow.")

            green_and_red = cycle - self.yellow
            for red in range(1, green_and_red):
                green = green_and_red - red
                if green >= red and 1 <= green / red <= 1.5:
                    return green, self.yellow, red

            raise ValueError("Cannot find valid red and green values.")
        except ValueError:
            return self.leg, self.leg, self.leg  # 默认值绿灯, 黄灯, 红灯

    def _cal_start_t(self, leg_id: int) -> float:
        return 1 + leg_id * self.phase

    def _set_id(self, num: int) -> int:
        return int(self.id_init + num * 10)

    def set_name_parameter(self, count: int) -> str:
        return f"$TrafficInfo_{self._set_id(count)}"

    def set_name_traffic_signal_controller(self, count: int) -> str:
        return f"{self._set_id(count)}"

    def set_phase_stop(self) -> xosc.Phase:
        phase = xosc.Phase(name="stop", duration=float(self.red))
        phase.add_signal_state(signal_id="", state="true;flase;false")
        return phase

    def set_phase_attention(self) -> xosc.Phase:
        phase = xosc.Phase(name="attention", duration=float(self.yellow))
        phase.add_signal_state(signal_id="", state="false;true;false")
        return phase

    def set_phase_go(self) -> xosc.Phase:
        phase = xosc.Phase(name="go", duration=float(self.green))
        phase.add_signal_state(signal_id="", state="false;flase;true")
        return phase

    def set_route(self, route: str) -> str:
        # TODO: 需要按照类型写死
        return f"route:{route};"

    def set_start_t(self, leg_id: int) -> str:
        return f"start_t:{self._cal_start_t(leg_id)};"

    def set_routeid(self, leg_id: int) -> str:
        return f"routeid:{self._set_id(leg_id)};"

    def set_junctionid(self) -> str:
        return "junctionid:1;"

    def set_l_offset(self) -> str:
        return "l_offset:0.0;"

    def set_phase(self, lane_num: int) -> str:
        mapping = {
            "1": "L,T,R",
            "2": "L,T,R",
            "3": "L,T,R",
            "4": "L,T,T,R",
        }
        return f"phase:{mapping[str(lane_num)]};"

    def set_lane(self, lane_num: int) -> str:
        return "lane:" + ",".join(map(str, range(-1, -lane_num - 1, -1))) + ";"

    def set_status(self) -> str:
        return "status:Activated;"

    def set_plan(self, active_plan_num: int) -> str:
        """设置选择使用第几套信控方案生效, 默认系统提供的为 0"""
        return f"plan:{active_plan_num};"

    def set_phase_number(self, leg_id: int) -> str:
        return f"phaseNumber:{leg_id + 1};"


@dataclass(order=True)
class WrapperPardecs:
    """[summary]"""

    def __post_init__(self):
        self._wrapper_pardecs = xosc.ParameterDeclarations()
        self.count = 0

    def add_trafficlight(self, tsa: TADSimTrafficSignalAuto, active_plan_num: int, lane_num: int, route: str) -> None:
        # 添加 ActivePlan 用来选择使用第几套信控方案, 默认系统提供的为 0
        self._wrapper_pardecs.add_parameter(
            xosc.Parameter("$ActivePlan", xosc.ParameterType.integer, str(active_plan_num))
        )

        # 增加交通灯, 自定义信息 (如果有交通灯, 默认需存在)
        for i in range(tsa.leg):
            parameter = xosc.Parameter(
                name=tsa.set_name_parameter(count=self.count),
                parameter_type=xosc.ParameterType.string,
                value=(
                    "".join(
                        [
                            tsa.set_route(route),
                            tsa.set_start_t(leg_id=i),
                            tsa.set_routeid(leg_id=i),
                            tsa.set_junctionid(),
                            tsa.set_l_offset(),
                            tsa.set_phase(lane_num),
                            tsa.set_lane(lane_num),
                            tsa.set_status(),
                            tsa.set_plan(active_plan_num=active_plan_num),
                            tsa.set_phase_number(leg_id=i),
                        ]
                    )
                ),
            )
            self._wrapper_pardecs.add_parameter(parameter)

            # 累加
            self.count += 1

    def get_result(self) -> xosc.ParameterDeclarations:
        """[summary]

        Returns:
            xosc.ParameterDeclarations: [description]
        """
        return self._wrapper_pardecs


@dataclass(order=True)
class WrapperCatalog:
    """[summary]"""

    def __post_init__(self):
        self._wrapper_catalog = xosc.Catalog()

    def get_result(self) -> xosc.Catalog:
        """
        This function returns the wrapper catalog

        Returns:
          xosc.Catalog: The wrapper catalog.
        """
        return self._wrapper_catalog


@dataclass(order=True)
class WrapperRoadNetwork:
    """[summary]"""

    def __post_init__(self):
        self.count = 0

    def set_roadnetwork(self, hadmap_name: str, scene_graph: Optional[str] = None) -> None:
        """
        It sets the road network for the simulation.

        Args:
          hadmap_name (str): The name of the hadmap file.
          scene_graph (str): The name of the scene_graph file.
            Defaults to None.
        """
        # for tadsim format
        # 检查地图后缀是否在支持的后缀列表中
        suffix = hadmap_name.split(".")[-1]
        if suffix not in settings.sys.map.support:
            logger.opt(lazy=True).info(f"{hadmap_name} suffix not in {settings.sys.map.support}")
            suffix = "xodr"

        # 添加后缀
        hadmap_name = check_add_suffix(hadmap_name, suffix)
        # rel_hadmap = f"../hadmap/{hadmap_name}"
        rel_hadmap = f"{settings.sys.pathdir_hadmap_infile}{hadmap_name}"
        self._wrapper_roadnetwork = xosc.RoadNetwork(rel_hadmap, scene_graph)

    def add_trafficlight(self, tsa: TADSimTrafficSignalAuto):
        for _ in range(tsa.leg):
            # 创建一个信号灯控制器 traffic_signal_controller
            traffic_signal_controller = xosc.TrafficSignalController(tsa.set_name_traffic_signal_controller(self.count))

            # 添加 stop, go, attention
            traffic_signal_controller.add_phase(phase=tsa.set_phase_stop())
            traffic_signal_controller.add_phase(phase=tsa.set_phase_go())
            traffic_signal_controller.add_phase(phase=tsa.set_phase_attention())

            # 添加到 road network
            self._wrapper_roadnetwork.add_traffic_signal_controller(traffic_signal_controller)

            # 累加
            self.count += 1

    def get_result(self) -> xosc.RoadNetwork:
        """
        The function returns the
        RoadNetwork object that is stored in the wrapper

        Returns:
          xosc.RoadNetwork: The wrapper object.
        """

        return self._wrapper_roadnetwork


@dataclass(order=True)
class WrapperEntities:
    def __post_init__(self):
        """xosc.Entities instantiation"""
        self._wrapper_entities = xosc.Entities()

    def set_entities(
        self,
        entityname: str,
        entityobject: Union[xosc.CatalogReference, xosc.Vehicle, xosc.Pedestrian, xosc.MiscObject],
        controller: Union[xosc.CatalogReference, xosc.Controller, None] = None,
    ) -> None:
        # 构建
        self._wrapper_entities.add_scenario_object(entityname, entityobject, controller)

    def get_result(self) -> xosc.Entities:
        """
        This function returns the wrapper entities

        Returns:
          xosc.Entities: The wrapper_entities object.
        """
        return self._wrapper_entities


@dataclass(order=True)
class WrapperInit:
    """ """

    creat_action: CreatAction

    def __post_init__(self):
        """[summary]"""
        # xosc instantiation
        self._wrapper_init = xosc.Init()

    def set_absolute_speed(self, entityname: str, speed: float) -> None:
        kaction = KAction(type=settings.sys.l4.PrivateActionType.enum.absoluteSpeed, value=speed)
        self._wrapper_init.add_init_action(
            entityname=entityname, action=self.creat_action.choose(kaction.type, kaction)
        )

    def set_worldposition(self, entityname: str, wpt: dict) -> None:
        # Teleport Action
        kaction = KAction(type=settings.sys.l4.PrivateActionType.enum.teleport, wpts=[wpt])
        self._wrapper_init.add_init_action(
            entityname=entityname, action=self.creat_action.choose(kaction.type, kaction)
        )

    def set_routing(self, entityname: str, wpts: list) -> None:
        # print(f"{entityname = }")
        # Routing Actions
        kaction = KAction(type=settings.sys.l4.PrivateActionType.enum.assignRoute, wpts=wpts)
        self._wrapper_init.add_init_action(
            entityname=entityname, action=self.creat_action.choose(kaction.type, kaction)
        )

    def set_activate_controller(self) -> None:
        # TODO:(xingboliu) input without settings Activate Controller, need to add
        pass

    def set_environment(self, ktime: float, kwether: str) -> None:
        # kaction = KAction(type=settings.sys.l4.GlobalActionType.enum.environment, endtype=ktime, user=kwether)
        kaction = KAction(type=settings.sys.l4.GlobalActionType.enum.environment, value=ktime, user=kwether)
        self._wrapper_init.add_global_action(self.creat_action.choose(kaction.type, kaction))

    def get_result(self) -> xosc.Init:
        """ """
        return self._wrapper_init


@dataclass(order=True)
class WrapperStory:
    """Creat story class"""

    creat_action: CreatAction
    creat_trigger: CreatTrigger

    def __post_init__(self):
        """The WrapperStory class initializes the object without parameter."""
        # valid triggering point, valid start or stop
        self.name_starttrigger = "start"
        # define name
        self.name_maneuver = "Maneuver"
        self.name_event = "Event"
        self.name_trigger = "Condition"
        self.name_action = "Action"
        #
        self._wrapper_story = xosc.Story("Story")
        self._act = self._set_act("Act")

    def _set_event(self, num_event: int, kevent: KEvent) -> xosc.Event:
        """
        Creat event and add one starttrigger & muilt action.

        Args:
          num_event (int): the number of the event, which is used to name the event.
          kevent (KEvent): KEvent

        Returns:
          The event is being returned.
        """
        # instantiation new event
        event = xosc.Event(name=f"{self.name_event}{num_event}", priority=xosc.Priority.overwrite)
        event.add_trigger(
            trigger=self.creat_trigger.choose(
                kevent.trigger.type, kevent.trigger, self.name_trigger, self.name_starttrigger
            )
        )
        # add muilt action
        for i, _ in enumerate(kevent.actions):
            # print(f"{kevent.actions[i] = }")
            event.add_action(
                actionname=f"{self.name_action}{i+1}",
                action=self.creat_action.choose(kevent.actions[i].type, kevent.actions[i]),
            )
        # print(f"{event = }")
        return event

    def _set_maneuvergroup(self, entityname: str, kevents: List[KEvent]) -> xosc.ManeuverGroup:
        # 定义 ManeuverGroup
        maneuvergroup = xosc.ManeuverGroup(entityname)
        maneuvergroup.add_actor(entityname)

        # 不存在 kevents 时, 直接返回
        # print(f"{kevents = }")
        if not kevents:
            return maneuvergroup

        # 存在 kevents 时, 处理 ManeuverGroup, 定义 maneuver, 循环创建 event, 添加到 maneuver 内
        maneuver = xosc.Maneuver(self.name_maneuver)
        for i, kevent in enumerate(kevents, start=1):
            maneuver.add_event(event=self._set_event(i, kevent))

        maneuvergroup.add_maneuver(maneuver)
        return maneuvergroup

    def _set_act(self, name: str) -> xosc.Act:
        # define act
        starttrigger = self.creat_trigger.choose(
            settings.sys.l4.ValueTriggerType.enum.simulationTime,
            ktrig=KTrigger(type=settings.sys.l4.ValueTriggerType.enum.simulationTime, value=0.0),
            name=self.name_starttrigger,
            triggeringpoint=self.name_starttrigger,
        )
        return xosc.Act(name, starttrigger)

    def set_story(self, entityname: str, kevents: List[KEvent]) -> None:
        # add maneuvergroup to xosc.Act
        self._act.add_maneuver_group(self._set_maneuvergroup(entityname, kevents))

    def get_result(self) -> xosc.Story:
        """
        This function returns the story object that was created by the wrapper

        Returns:
          The wrapper story.
        """
        self._wrapper_story.add_act(self._act)
        return self._wrapper_story


@dataclass(order=True)
class GenXosc:
    """Generate xosc file conforming to OpenSCENARIO (V1.0.0) standard."""

    gen_folder: Path
    oscminor: int
    pathdir_catalog: Path

    def __post_init__(self):
        self.author = settings.sys.author

        wrapper_catalog = WrapperCatalog()
        self.catalog = wrapper_catalog.get_result()

        self._entry_library = self._get_entry_library(self.pathdir_catalog)

        self.creat_action = CreatAction()
        self.creat_trigger = CreatTrigger()

    def _get_entry_library(self, pathdir: Path, suffix: str = "xosc") -> dict:
        pathfile_catalogs = pathdir.glob(f"**/*.{suffix}")

        mapping = {
            "Vehicle": "Veh",
            "Pedestrian": "Vru",
            "MiscObject": "Misc",
            "Controller": "Controller",
            "Environment": "Env",
            "Maneuver": "Maneuver",
            "Trajectory": "Trajectory",
            "Route": "Route",
        }

        catalog_library = xosc.CatalogLoader()
        for pathfile in pathfile_catalogs:
            logger.opt(lazy=True).debug(f"Loading catalog file: {pathfile}")
            catalog_library.load_catalog(pathfile.stem, str(pathfile.parent))

        out: Dict[str, Any] = {}
        for catalogname, catalog in catalog_library.all_catalogs.items():
            for entry in catalog:
                entryname = entry.attrib["name"]
                try:
                    value = catalog_library.parse(xosc.CatalogReference(catalogname, entryname))
                except (ValueError, NotImplementedError, xosc.OpenSCENARIOVersionError) as e:
                    logger.opt(lazy=True).warning(f"Warning sip {entryname}, cause {e}")
                    continue

                out[f"{mapping[entry.tag]}_{entryname}"] = value

        return out

    def _format_ktrig(self, hname: str, scene: dict, i: int) -> KTrigger:
        # print(scene.get(f"{hname}.Dyn{i}.Trigger.Ref", f"{hname}"))
        krule = scene[f"{hname}.Dyn{i}.Trigger.Rule"]
        if self.oscminor == 0:
            mapping = {"greaterOrEqual": "greaterThan", "lessOrEqual": "lessThan", "notEqualTo": "lessThan"}
            krule = mapping.get(krule, krule)

        return KTrigger(
            type=scene[f"{hname}.Dyn{i}.Trigger.Type"],
            value=scene[f"{hname}.Dyn{i}.Trigger.Value"],
            rule=krule,
            disttype=scene.get(f"{hname}.Dyn{i}.Trigger.Disttype", "longitudinal"),
            ref=scene.get(f"{hname}.Dyn{i}.Trigger.Ref", f"{hname}"),
        )

    def _format_kaction(self, hname: str, scene: dict, i: int, j: int) -> KAction:
        ktype = scene[f"{hname}.Dyn{i}.Action{j}.Type"]
        kvalue = scene[f"{hname}.Dyn{i}.Action{j}.Value"]
        kendtype = scene[f"{hname}.Dyn{i}.Action{j}.Endtype"]
        kendvalue = scene[f"{hname}.Dyn{i}.Action{j}.Endvalue"]
        kref = hname

        # print(ktype, kvalue, kendtype, kendvalue, kref)

        target = [
            settings.sys.l4.PrivateActionType.enum.acceleration,
            settings.sys.l4.PrivateActionType.enum.deceleration,
        ]
        if ktype in target:
            kvalue = abs(kvalue) if ktype == target[0] else -abs(kvalue)
            # 统一表示速度=10m_s 和 加速度=2m_s2@终止速度=10m_s 的情况
            kvalue, kendvalue = kendvalue, kvalue

            if kendtype in {settings.sys.l4.EndtypeMode.enum.speed}:
                ktype = settings.sys.l4.PrivateActionType.enum.absoluteSpeed

            if kendtype in {settings.sys.l4.EndtypeMode.enum.time}:
                ktype = settings.sys.l4.PrivateActionType.enum.relativeSpeed
                # kref = hname

        return KAction(type=ktype, value=kvalue, endtype=kendtype, endvalue=kendvalue, ref=kref)

    def _get_entity_with_default(self, category: str, model: str, entry_library_keys: List[str]):
        default_model_by_category = {"Veh": "Sedan", "Vru": "human", "Misc": "Cone"}

        k_name = f"{category}_{model}"
        if k_name in entry_library_keys:
            out = f"{category}_{model}"
        else:
            out = f"{category}_{default_model_by_category[category]}"

        return out

    def generate(self, scene: dict, func: Callable[..., Any] = None) -> None:
        # print("============== start =====================")
        # print(f"{scene['ConcreteId'] = } {scene[settings.sys.l0.cols.mapfile] = }")
        # print("============== end =====================")

        # 创建 wrapper_pardecs
        wrapper_pardecs = WrapperPardecs()

        # 创建 wrapper_roadnetwork
        wrapper_roadnetwork = WrapperRoadNetwork()
        wrapper_roadnetwork.set_roadnetwork(f"{scene[settings.sys.l0.cols.mapfile]}")

        # 如果存在交通灯, 则进行信控相关语义灯的配置, tadsim 在 wrapper_roadnetwork 和 wrapper_pardecs 自定义内容
        leg_mapping = settings.sys.l1.JunctionType.leg
        leg = leg_mapping[scene["Junction.Type"]] if scene["Junction.Type"] in leg_mapping.keys() else 0

        # TODO: 暂时屏蔽掉改功能, 因为位置点还是设置有问题存在
        # have_trafficlight = True if (scene["Trafficlight.Status"] and leg) else False
        have_trafficlight = False
        if have_trafficlight:
            # 设置的信控方案, 存在用户设置则使用用户, 否则使用系统预设, 并设置选择的方案
            # user_cycle = scene["Trafficlight.Cycle"]
            user_cycle = scene.get("Trafficlight.Cycle")
            cycle, active_plan_num = (user_cycle, 1) if user_cycle else (settings.sys.l2.trafficlight.sys_cycle, 0)

            # 初始化 tsa
            tsa = TADSimTrafficSignalAuto(cycle=cycle, leg=leg)
            # 添加 tsa 到 wrapper_roadnetwork
            wrapper_roadnetwork.add_trafficlight(tsa=tsa)
            # 添加 tsa 到 wrapper_roadnetwork
            wrapper_pardecs.add_trafficlight(
                tsa=tsa,
                active_plan_num=active_plan_num,
                lane_num=scene["Lane.Num"],
                # route="0.018177,-0.000198",
                route=str(scene.get("Trafficlight.Route")),
            )

        # 设置环境
        wrapper_init = WrapperInit(self.creat_action)
        wrapper_init.set_environment(ktime=float(scene.get("Env.Time", 12.0)), kwether=str(scene.get("Env.Weather")))

        # 设置所有的成员的信息 wrapper_entities wrapper_story wrapper_init
        wrapper_entities = WrapperEntities()
        wrapper_story = WrapperStory(self.creat_action, self.creat_trigger)
        for hname in ["Ego", *[f"Npc{i}" for i in range(1, scene[settings.sys.l0.cols.allnum_npc] + 1)]]:
            # print(f"{hname = }")
            # 设置物理属性
            entityname = hname  # 设置相同, 为后续扩展留下接口
            controller = scene.get(f"{hname}.Phy.Controller")
            entity = self._get_entity_with_default(
                category=scene[f"{hname}.Phy.Category"],
                model=scene[f"{hname}.Phy.Model"],
                entry_library_keys=self._entry_library.keys(),
            )
            entityobject = self._entry_library[entity]
            wrapper_entities.set_entities(entityname, entityobject, controller)

            # 设置初始速度
            if f"{hname}.Ini.Speed" in scene:
                speed = scene[f"{hname}.Ini.Speed"]
                # print(f"{speed = }")
                wrapper_init.set_absolute_speed(entityname, speed)

            # 设置 waypoint
            if f"{hname}.Ini.Wpts" in scene:
                wpts = scene[f"{hname}.Ini.Wpts"]
                # if isinstance(wpts[0], Waypoint):
                #     wrapper_init.set_worldposition(entityname, wpts[0])
                # # 判断 wpts 中第二位是否存在(是否为 0.0), 构建 wpts 时, 不存在 wpt 的值会赋值为 0.0
                # if len(wpts) > 1 and isinstance(wpts[1], Waypoint):
                #     wrapper_init.set_routing(entityname, wpts)

                if isinstance(wpts[0], Waypoint):
                    if len(wpts) == 1:
                        wrapper_init.set_worldposition(entityname, wpts[0])
                    elif len(wpts) > 1 and all(isinstance(wpt, Waypoint) for wpt in wpts):
                        wrapper_init.set_routing(entityname, wpts)

            # 设置 kevents
            kevents = [
                KEvent(
                    trigger=self._format_ktrig(hname, scene, i),
                    actions=[
                        self._format_kaction(hname, scene, i, j)
                        for j in range(1, scene[f"{hname}.Dyn{i}.{settings.sys.l0.cols.allnum_action}"] + 1)
                    ],
                )
                for i in range(1, scene[f"{hname}.{settings.sys.l0.cols.allnum_dyn}"] + 1)
            ]

            # print(f"{kevents = }")
            wrapper_story.set_story(entityname, kevents)

        # 合并生成 Storyboard
        storyboard = xosc.StoryBoard(wrapper_init.get_result())
        storyboard.add_story(wrapper_story.get_result())

        # 生成xosc格式的场景文件
        sce = xosc.Scenario(
            name=scene.get(settings.sys.generalized.cols.description, "Multi"),
            author=settings.sys.author,
            parameters=wrapper_pardecs.get_result(),
            catalog=self.catalog,
            roadnetwork=wrapper_roadnetwork.get_result(),
            entities=wrapper_entities.get_result(),
            storyboard=storyboard,
            osc_minor_version=self.oscminor,
        )

        # 写入文件
        sce.write_xml(
            filename=self.gen_folder / f"{scene[settings.sys.l0.cols.concreteid]}.xosc",
            prettyprint=True,
            encoding="utf-8",
        )

        # 如果用户传入了回调函数，就执行它
        if func is not None:
            func(scene)


# sourcery skip: remove-redundant-if
if __name__ == "__main__":
    pass
