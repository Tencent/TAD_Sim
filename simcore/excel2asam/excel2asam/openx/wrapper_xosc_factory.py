#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import annotations

from abc import ABC, abstractmethod
from dataclasses import dataclass
from typing import Any, Dict

import ephem
from scenariogeneration import xosc

from excel2asam.config import settings
from excel2asam.map.lib.hadmap_py import Waypoint
from excel2asam.utils import KAction, KTrigger, value_dispatch


def select_position_type(wpt: Waypoint) -> str:
    """Selects the position type based on the keys of the waypoint dictionary.

    Args:
        wpt (Waypoint): The waypoint.

    Returns:
        str: The position type.
    """
    # mapping = {
    #     PositionType.WORLD: settings.sys.l4.PositionType.enum.world,
    #     PositionType.LANE: settings.sys.l4.PositionType.enum.lane,
    # }
    # ktype = wpt.position.type

    # if ktype in mapping:
    #     return mapping[ktype]
    # else:
    #     raise ValueError("参数输入错误, 目前仅支持 world & lane")

    return settings.sys.l4.PositionType.enum.lane


# 工厂模式创建 Position
@dataclass(order=True)
class PositionFactory:
    @value_dispatch
    def creat(self, ktype: str, wpt: dict) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.PositionType.enum.values()}"中其一'

    @creat.register(settings.sys.l4.PositionType.enum.world)
    def creat_world(self, ktype: str, wpt: Waypoint) -> xosc.WorldPosition:
        # 如果设置了初始.航向
        if wpt.position.world.h:
            return xosc.WorldPosition(
                x=wpt.position.world.x, y=wpt.position.world.y, z=wpt.position.world.z, h=wpt.position.world.h
            )
        # 未设置初始.航向时, 不发送 h
        return xosc.WorldPosition(x=wpt.position.world.x, y=wpt.position.world.y, z=wpt.position.world.z)

    @creat.register(settings.sys.l4.PositionType.enum.lane)
    def creat_lane(self, ktype: str, wpt: Waypoint) -> xosc.LanePosition:
        # 如果设置了初始.航向
        if wpt.position.lane.orientation.h:
            return xosc.LanePosition(
                s=wpt.position.lane.s,
                offset=wpt.position.lane.offset,
                lane_id=wpt.position.lane.lane_id,
                road_id=wpt.position.lane.road_id,
                orientation=xosc.Orientation(h=wpt.position.lane.orientation.h),
            )

        # 未设置初始.航向时, 不发送 h
        return xosc.LanePosition(
            s=wpt.position.lane.s,
            offset=wpt.position.lane.offset,
            lane_id=wpt.position.lane.lane_id,
            road_id=wpt.position.lane.road_id,
        )


# 工厂模式创建 Condition
class ConditionFactory(ABC):
    @abstractmethod
    def creat(self, ktype: str, ktrig: KTrigger) -> None:
        pass


@dataclass(order=True)
class ValueConditionFactory(ConditionFactory):
    @value_dispatch
    def creat(self, ktype: str, ktrig: KTrigger) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.ValueTriggerType.enum.values()}"中其一'

    @creat.register(settings.sys.l4.ValueTriggerType.enum.simulationTime)
    def creat_simulation_time(self, ktype: str, ktrig: KTrigger) -> xosc.SimulationTimeCondition:
        return xosc.SimulationTimeCondition(value=ktrig.value, rule=xosc.Rule[ktrig.rule])


@dataclass(order=True)
class EntityConditionFactory(ConditionFactory):
    @value_dispatch
    def creat(self, ktype: str, ktrig: KTrigger) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.EntityTriggerType.enum.values()}"中其一'

    @creat.register(settings.sys.l4.EntityTriggerType.enum.timeHeadway)
    def creat_time_headway(self, ktype: str, ktrig: KTrigger) -> xosc.TimeHeadwayCondition:
        return xosc.TimeHeadwayCondition(
            entity=ktrig.ref,
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
            alongroute=True,
            freespace=True,
            # distance_type=ktrig.disttype,  # osc 1.1 only
            # coordinate_system=CoordinateSystem.road  # osc 1.1 only
        )

    @creat.register(settings.sys.l4.EntityTriggerType.enum.timeToCollision)
    def creat_time_to_collision(self, ktype: str, ktrig: KTrigger) -> xosc.TimeToCollisionCondition:
        mapping_alongroute = {
            xosc.RelativeDistanceType.longitudinal.name: True,
            xosc.RelativeDistanceType.cartesianDistance.name: False,
        }

        return xosc.TimeToCollisionCondition(
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
            alongroute=mapping_alongroute[ktrig.disttype],
            freespace=True,
            entity=ktrig.ref,
            position=None,
            # distance_type=ktrig.disttype,  # osc 1.1 only
        )

    @creat.register(settings.sys.l4.EntityTriggerType.enum.absoluteDistance)
    def creat_absolute_distance(self, ktype: str, ktrig: KTrigger) -> xosc.DistanceCondition:
        # print("creat_absolute_distance")
        return xosc.DistanceCondition(
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
            position=PositionFactory().creat(select_position_type(wpt=ktrig.wpt), ktrig.wpt),
            alongroute=True,
            freespace=True,
            # distance_type=ktrig.disttype,  # osc 1.1 only
            # coordinate_system=CoordinateSystem.road  # osc 1.1 only
        )

    @creat.register(settings.sys.l4.EntityTriggerType.enum.relativeDistance)
    def creat_relative_distance(self, ktype: str, ktrig: KTrigger) -> xosc.RelativeDistanceCondition:
        # print(f"{ktrig.ref = }")
        return xosc.RelativeDistanceCondition(
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
            dist_type=xosc.RelativeDistanceType[ktrig.disttype],
            entity=ktrig.ref,
        )

    @creat.register(settings.sys.l4.EntityTriggerType.enum.absoluteSpeed)
    def creat_absolute_speed(self, ktype: str, ktrig: KTrigger) -> xosc.SpeedCondition:
        return xosc.SpeedCondition(
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
        )

    @creat.register(settings.sys.l4.EntityTriggerType.enum.relativeSpeed)
    def creat_relative_speed(self, ktype: str, ktrig: KTrigger) -> xosc.RelativeSpeedCondition:
        return xosc.RelativeSpeedCondition(
            value=ktrig.value,
            rule=xosc.Rule[ktrig.rule],
            entity=ktrig.ref,
        )


# 工厂模式创建 Action
class ActionFactory(ABC):
    @abstractmethod
    def creat(self, ktype: str, kaction: KAction) -> None:
        pass


@dataclass(order=True)
class PrivateActionFactory(ActionFactory):
    @value_dispatch
    def creat(self, ktype: str, kaction: KAction) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.PrivateActionType.enum.values()}"中其一'

    # longitudinal Action
    @creat.register(settings.sys.l4.PrivateActionType.enum.absoluteSpeed)
    def create_absolute_speed(self, ktype: str, kaction: KAction) -> xosc.AbsoluteSpeedAction:
        td = xosc.TransitionDynamics(
            # 加速减速时使用 linear
            shape=xosc.DynamicsShapes.step if abs(float(kaction.endvalue)) < 1e-6 else xosc.DynamicsShapes.linear,
            # 加速减速时使用 rate
            dimension=xosc.DynamicsDimension.time
            if abs(float(kaction.endvalue)) < 1e-6
            else xosc.DynamicsDimension.rate,
            value=kaction.endvalue,
        )
        # print("IN 速度")
        return xosc.AbsoluteSpeedAction(speed=kaction.value, transition_dynamics=td)

    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeSpeed)
    def create_relative_speed(self, ktype: str, kaction: KAction) -> xosc.RelativeSpeedAction:
        td = xosc.TransitionDynamics(
            # 加速减速时使用 linear
            shape=xosc.DynamicsShapes.step if abs(float(kaction.endvalue)) < 1e-6 else xosc.DynamicsShapes.linear,
            dimension=xosc.DynamicsDimension.rate,
            value=kaction.endvalue,
        )
        # print("IN 相对速度")
        return xosc.RelativeSpeedAction(
            speed=float(kaction.value) * kaction.endvalue,
            entity=kaction.ref,
            transition_dynamics=td,
            valuetype=xosc.SpeedTargetValueType.delta,
            continuous=True,
        )

    @creat.register(settings.sys.l4.PrivateActionType.enum.longitudinalDistance)
    def create_longitudinal_distance(self, ktype: str, kaction: KAction) -> xosc.LongitudinalDistanceAction:
        return xosc.LongitudinalDistanceAction(
            entity=kaction.ref,
            freespace=True,
            continuous=True,
            max_acceleration=None,
            max_deceleration=None,
            max_speed=None,
            distance=kaction.value,
            timeGap=None,
        )

    @creat.register(settings.sys.l4.PrivateActionType.enum.longitudinalTimegap)
    def create_longitudinal_timegap(self, ktype: str, kaction: KAction) -> None:
        # TODO
        pass

    # lateral Actions
    @creat.register(settings.sys.l4.PrivateActionType.enum.absoluteLaneChange)
    def creat_aolute_lane_change(self, ktype: str, kaction: KAction) -> None:
        # TODO
        pass

    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeLaneChange2Left)
    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeLaneChange2Right)
    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeLaneOffset2Left)
    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeLaneOffset2Right)
    def create_relativel_lane_change(self, ktype: str, kaction: KAction) -> xosc.RelativeLaneChangeAction:
        td = xosc.TransitionDynamics(
            shape=xosc.DynamicsShapes.sinusoidal,
            dimension=xosc.DynamicsDimension.time,
            value=kaction.value,
        )

        mapping = {
            settings.sys.l4.PrivateActionType.enum.relativeLaneChange2Left: (1, 0.0),
            settings.sys.l4.PrivateActionType.enum.relativeLaneChange2Right: (-1, 0.0),
            settings.sys.l4.PrivateActionType.enum.relativeLaneOffset2Left: (0, abs(float(kaction.endvalue))),
            settings.sys.l4.PrivateActionType.enum.relativeLaneOffset2Right: (0, -abs(float(kaction.endvalue))),
        }

        target_lane_id, target_lane_offset = mapping[kaction.type]

        return xosc.RelativeLaneChangeAction(
            lane=target_lane_id,
            entity=kaction.ref,
            transition_dynamics=td,
            target_lane_offset=target_lane_offset,
        )

    @creat.register(settings.sys.l4.PrivateActionType.enum.relativeLaneOffset)
    def creat_relativel_lane_offset(self, ktype: str, kaction: KAction) -> xosc.RelativeLaneOffsetAction:
        return xosc.RelativeLaneOffsetAction(
            value=kaction.value,
            entity=kaction.ref,
            shape=xosc.DynamicsShapes.linear,
            maxlatacc=3.5,
        )

    @creat.register(settings.sys.l4.PrivateActionType.enum.absoluteLaneOffset)
    def creat_absolute_lane_offset(self, ktype: str, kaction: KAction) -> xosc.AbsoluteLaneOffsetAction:
        return xosc.AbsoluteLaneOffsetAction(
            value=kaction.value,
            shape=xosc.DynamicsShapes.linear,
            maxlatacc=3.5,
            continuous=True,
        )

    # Controller Action
    @creat.register(settings.sys.l4.PrivateActionType.enum.activateController)
    def create_activate_controller(self, ktype: str, kaction: KAction) -> xosc.ActivateControllerAction:
        mapping_lat_lon = {
            settings.sys.l4.action.automode.autopilot: (True, True),
            settings.sys.l4.action.automode.longitudinal: (False, True),
            settings.sys.l4.action.automode.lateral: (True, False),
            settings.sys.l4.action.automode.off: (False, False),
        }

        # TODO: 1.0和1.1有区别, 目前只支持1.0, 需要增加通过版本判断
        return xosc.ActivateControllerAction(*mapping_lat_lon[int(kaction.value)])

    @creat.register(settings.sys.l4.PrivateActionType.enum.overrideControllerValue)
    def create_override_controller_value(self, ktype: str, kaction: KAction) -> xosc.ControllerAction:
        # 初始化
        override = xosc.OverrideControllerValueAction()

        override.set_clutch(active=False, value=0)
        override.set_brake(active=False, value=0)
        override.set_throttle(active=False, value=0)
        override.set_steeringwheel(active=False, value=0)
        override.set_parkingbrake(active=False, value=0)
        override.set_gear(active=False, value=0)

        # 选择类型
        mapping: Dict[int, Any] = {
            settings.sys.l4.action.overridemode.clutch: override.set_clutch,
            settings.sys.l4.action.overridemode.brake: override.set_brake,
            settings.sys.l4.action.overridemode.throttle: override.set_throttle,
            settings.sys.l4.action.overridemode.steeringWheel: override.set_steeringwheel,
            settings.sys.l4.action.overridemode.parkingBrake: override.set_parkingbrake,
            settings.sys.l4.action.overridemode.gear: override.set_gear,
        }
        mapping[int(kaction.value)](active=True, value=kaction.endvalue)

        # 适配 tadsim 识别设置为  none
        properties = xosc.Properties()
        # properties.add_property("controller_catalog", "controller_catalog")
        controller = xosc.Controller(name="none", properties=properties)
        assign_controller = xosc.AssignControllerAction(controller)

        # TODO: 1.0和1.1有区别, 目前只支持1.0, 需要增加通过版本判断
        return xosc.ControllerAction(overrideControllerValueAction=override, assignControllerAction=assign_controller)

    @creat.register(settings.sys.l4.PrivateActionType.enum.assignController)
    def create_assign_controller(self, ktype: str, kaction: KAction) -> xosc.ControllerAction:
        override = xosc.OverrideControllerValueAction()
        override.set_clutch(active=False, value=0)
        override.set_brake(active=False, value=0)
        override.set_throttle(active=False, value=0)
        override.set_steeringwheel(active=False, value=0)
        override.set_parkingbrake(active=False, value=0)
        override.set_gear(active=False, value=0)

        # TODO: kaction.endtype, kaction.endvalue 目前是无效的
        properties = xosc.Properties()
        mapping = {
            "resume_sw": properties.add_property("resume_sw", "true"),
            "cancel_sw": properties.add_property("cancel_sw", "true"),
            "speed_inc_sw": properties.add_property("speed_inc_sw", "true"),
            "speed_dec_sw": properties.add_property("speed_dec_sw", "true"),
            "set_timegap": properties.add_property("set_timegap", kaction.endvalue),
            "set_speed": properties.add_property("set_speed", kaction.endvalue),
        }

        if kaction.endtype in mapping:
            mapping[kaction.endtype]

        controller = xosc.Controller(name="controller", properties=properties)
        assign_controller = xosc.AssignControllerAction(controller)
        # print(f"{assign_controller = }")
        # TODO: 1.0和1.1有区别, 目前只支持1.0, 需要增加通过版本判断
        return xosc.ControllerAction(
            overrideControllerValueAction=override,
            assignControllerAction=assign_controller,
        )

    # Teleport Action
    @creat.register(settings.sys.l4.PrivateActionType.enum.teleport)
    def create_teleport_worldposition(self, ktype: str, kaction: KAction) -> xosc.TeleportAction:
        worldposition = PositionFactory().creat(select_position_type(kaction.wpts[0]), kaction.wpts[0])
        # if kaction.
        return xosc.TeleportAction(worldposition)

    # Routing Actions
    @creat.register(settings.sys.l4.PrivateActionType.enum.acquirePosition)
    def create_acquire_position(self, ktype: str, kaction: KAction) -> None:
        # TODO
        pass

    @creat.register(settings.sys.l4.PrivateActionType.enum.visibility)
    def create_visibility(self, ktype: str, kaction: KAction) -> None:
        # TODO
        pass

    @creat.register(settings.sys.l4.PrivateActionType.enum.assignRoute)
    def create_assign_route(self, ktype: str, kaction: KAction) -> xosc.AssignRouteAction:
        route = xosc.Route(f"Route{kaction.ref}")
        # print(f"{kaction.wpts = }")
        # print("create_assign_route")
        for wpt in kaction.wpts:
            # print(f"{wpt = }")
            # print(f"{select_position_type(wpt) = }")
            worldposition = PositionFactory().creat(select_position_type(wpt), wpt)
            # print(f"{worldposition = }")
            route.add_waypoint(worldposition, xosc.RouteStrategy.shortest)

        return xosc.AssignRouteAction(route)

    @creat.register(settings.sys.l4.PrivateActionType.enum.followTrajectory)
    def create_follow_trajectory(self, ktype: str, kaction: KAction) -> xosc.FollowTrajectoryAction:
        positionlist = [
            xosc.RelativeObjectPosition(
                entity=kaction.ref,
                dx=0,
                dy=0,
                orientation=xosc.Orientation(h=0, reference=xosc.ReferenceContext.relative),
            )
        ]

        positionlist.append(
            xosc.RelativeObjectPosition(
                entity=kaction.ref,
                dx=0,
                dy=0,
                orientation=xosc.Orientation(h=kaction.value, reference=xosc.ReferenceContext.relative),
            )
        )

        polyline = xosc.Polyline([0, 0], positionlist)

        traj = xosc.Trajectory(f"Turning{kaction.ref}", False)
        traj.add_shape(polyline)

        return xosc.FollowTrajectoryAction(
            trajectory=traj,
            following_mode=xosc.FollowingMode.position,
            reference_domain=xosc.ReferenceContext.relative,
            scale=1,
            offset=0,
        )


@dataclass(order=True)
class GlobalActionFactory(ActionFactory):
    def __post_init__(self):
        self._now = ephem.now()
        self._year, self._month, self._day, self._hour, self._minute, self._second = self._now.tuple()
        self._sun = self._set_weather_sun()

    def _set_weather_sun(self) -> xosc.Sun:
        # 设置位置信息
        city = ephem.city(settings.sys.l5.city)
        latitude = str(city.lat)  # 纬度
        longitude = str(city.lon)  # 经度

        # 计算太阳位置和状态
        observer = ephem.Observer()
        observer.lat = latitude
        observer.lon = longitude
        observer.elevation = 0  # 海拔高度(单位: 米)
        observer.date = self._now

        sun = ephem.Sun(observer)
        sun.compute(observer)

        # 计算太阳方位角和高度角, 单位: rad
        azimuth = ephem._plusminus_pi(sun.az)
        elevation = ephem._plusminus_pi(sun.alt)

        # 计算太阳强度
        # zenith_distance = ephem.pi / 2 - sun.alt
        # airmass = 1 / ephem.cos(zenith_distance)
        # intensity = sun.radius**2 * airmass**0.6 * 1320  # 光通量密度(单位: lux)
        intensity = 0 if elevation < 0 else 1367 * ephem.cos(elevation)

        return xosc.Sun(
            intensity=round(intensity, settings.sys.decimal_places),
            azimuth=round(azimuth, settings.sys.decimal_places),
            elevation=round(elevation, settings.sys.decimal_places),
        )

    def _set_timeofday(self, hour: int, animation: bool = True) -> xosc.TimeOfDay:
        # bugfix: (xingboliu) 20230514 修复时间设置不正确的问题, 秒需要 int
        return xosc.TimeOfDay(animation, self._year, self._month, self._day, hour, self._minute, int(self._second))

    def _set_weather_fog(self, config: dict) -> xosc.Fog:
        """
        雾的分类等级:
            轻雾: 能见度在1000米以上
            雾 : 能见度在1000米至200米之间
            浓雾: 能见度在200米至50米之间
            强浓雾: 能见度在50米至20米之间
            特强浓雾: 能见度在20米以下
        """
        bounding_box = xosc.BoundingBox(width=2000, length=2000, height=2000, x_center=0, y_center=0, z_center=0)

        return xosc.Fog(visual_range=config["fog_visual_range"], bounding_box=bounding_box)

    def _set_weather_precipitation(self, config: dict) -> xosc.Precipitation:
        """
        雨的分类等级:
            特大暴雨: 每小时降水量超过250毫米
            大暴雨: 每小时降水量在100毫米至250毫米之间
            暴雨: 每小时降水量在50毫米至100毫米之间
            大雨: 每小时降水量在10毫米至50毫米之间
            中雨: 每小时降水量在2.5毫米至10毫米之间
            小雨: 每小时降水量小于2.5毫米

        雪的分类等级:
            暴雪: 每小时降雪量超过10毫米,伴有强风/低温等恶劣天气条件
            大雪: 每小时降雪量在4毫米至10毫米之间
            中雪: 每小时降雪量在1毫米至4毫米之间
            小雪: 每小时降雪量小于1毫米

        版本区别
            osc 1.0: intensity. Range: [0..1]
            osc 1.1: precipitationIntensity. Unit [mm/h]. Range [0...inf[
        """
        return xosc.Precipitation(
            precipitation=xosc.PrecipitationType[config["precipitation_type"]],
            intensity=float(config["precipitation_intensity"]) / 250,
        )

    def _set_weather_cloudstate(self, config: dict) -> xosc.CloudState:
        return xosc.CloudState[config["cloud_state"]]

    def _set_weather(self, config: dict) -> xosc.Weather:
        return xosc.Weather(
            cloudstate=self._set_weather_cloudstate(config),
            # atmosphericPressure=None,
            # temperature=None,
            sun=self._sun,
            fog=self._set_weather_fog(config),
            precipitation=self._set_weather_precipitation(config),
            # wind=None,
            # dome_image=None,
            # dome_azimuth_offset=None,
        )

    def _set_roadcondition(self, config: dict) -> xosc.RoadCondition:
        """
        常见天气条件下, 沥青路面道路摩擦系数的典型数值:
            晴天: 0.6-0.8
            小雨或小雪: 0.4-0.6
            中雨或中雪: 0.2-0.4
            大雨或大雪: 0.1-0.2
            暴雨或暴雪: 0.05-0.1
        """
        return xosc.RoadCondition(friction_scale_factor=config["friction_scale_factor"])

    def _set_parameters(self):
        pardecs = xosc.ParameterDeclarations()
        pardecs.add_parameter(xosc.Parameter("TimeStamp", xosc.ParameterType.integer, "0"))
        pardecs.add_parameter(xosc.Parameter("wind_speed", xosc.ParameterType.double, "2.0"))
        pardecs.add_parameter(xosc.Parameter("tmperature", xosc.ParameterType.double, "298.15"))
        pardecs.add_parameter(xosc.Parameter("UsingSunByUser", xosc.ParameterType.boolean, "false"))

        return pardecs

    @value_dispatch
    def creat(self, ktype: str, kaction: KAction) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.GlobalActionType.enum.values()}"中其一'

    @creat.register(settings.sys.l4.GlobalActionType.enum.environment)
    def create_environment(self, ktype: str, kaction: KAction) -> xosc.EnvironmentAction:
        # print("in create_environment")
        config = settings.sys.l5.Wether.config[kaction.user]
        environment = xosc.Environment(
            name="Environment",
            timeofday=self._set_timeofday(int(kaction.value)),
            weather=self._set_weather(config),
            roadcondition=self._set_roadcondition(config),
            parameters=self._set_parameters(),
        )

        return xosc.EnvironmentAction(environment)


@dataclass(order=True)
class UserDefinedActionFactory(ActionFactory):
    def __post_init__(self):
        self._target_commad = {
            settings.sys.l4.action.commandmode["LaneChange"],
            settings.sys.l4.action.commandmode["EmergencyStop"],
        }

    @value_dispatch
    def creat(self, ktype: str, kaction: KAction) -> str:
        return f'参数输入错误: {ktype} 仅应为 "{settings.sys.l4.UserDefinedActionType.enum.values()}"中其一'

    @creat.register(settings.sys.l4.UserDefinedActionType.enum.command)
    @creat.register(settings.sys.l4.UserDefinedActionType.enum.status)
    def create_user_define_action(self, ktype: str, kaction: KAction) -> xosc.UserDefinedAction:
        if ktype == settings.sys.l4.UserDefinedActionType.enum.command:
            # <CustomCommandAction type="Command" >LaneChange: active=true, value=1 </CustomCommandAction>
            commandmode = settings.sys.l4.action.commandmode
            command = list(commandmode.keys())[list(commandmode.values()).index(int(kaction.value))]
            custype = "Command"

            if command in self._target_commad:
                cuscontent = f"{command}:active=true"
            else:
                cuscontent = f"{command}:active=true,value={kaction.endvalue}"
        else:
            # <CustomCommandAction type="Status">Gear:value=9</CustomCommandAction>
            statusmode = settings.sys.l4.action.statusmode
            status = list(statusmode.keys())[list(statusmode.values()).index(int(kaction.value))]
            custype = "Status"
            cuscontent = f"{status}:value={kaction.endvalue}"

        custom = xosc.CustomCommandAction(type=custype, content=cuscontent)

        return xosc.UserDefinedAction(custom)


# 选择器选择
@dataclass(order=True)
class CreatTrigger:
    def __post_init__(self):
        self.mapping_conditionedge = {
            "greaterOrEqual": xosc.ConditionEdge.rising,
            "lessOrEqual": xosc.ConditionEdge.falling,
            "notEqualTo": xosc.ConditionEdge.risingOrFalling,
            "greaterThan": xosc.ConditionEdge.rising,
            "lessThan": xosc.ConditionEdge.falling,
            "equalTo": xosc.ConditionEdge.risingOrFalling,
        }
        self._value_condition = ValueConditionFactory()
        self._entity_condition = EntityConditionFactory()

    @value_dispatch
    def choose(self, ktype: str, *args: tuple) -> str:
        # 泛型函数,通过传入参数 (ktype) 的类型不同而返回不同的处理结果
        target = settings.sys.l4.ValueTriggerType.enum.values() + settings.sys.l4.EntityTriggerType.enum.values()
        return f"参数输入错误: {ktype} 仅应为 [{target}]中其一"

    @choose.register_for_all(settings.sys.l4.ValueTriggerType.enum.values())
    def set_value_triggers(self, ktype: str, ktrig: KTrigger, name: str, triggeringpoint: str) -> xosc.ValueTrigger:
        # print(f"{ktrig = }")
        return xosc.ValueTrigger(
            name=name,
            delay=0.0,
            # conditionedge=xosc.ConditionEdge.risingOrFalling,
            conditionedge=self.mapping_conditionedge[ktrig.rule],
            valuecondition=self._value_condition.creat(ktrig.type, ktrig),
            triggeringpoint=triggeringpoint,
        )

    @choose.register_for_all(settings.sys.l4.EntityTriggerType.enum.values())
    def set_entity_triggers(self, ktype: str, ktrig: KTrigger, name: str, triggeringpoint: str) -> xosc.EntityTrigger:
        return xosc.EntityTrigger(
            name=name,
            delay=0.0,
            # conditionedge=xosc.ConditionEdge.risingOrFalling,
            conditionedge=self.mapping_conditionedge[ktrig.rule],
            entitycondition=self._entity_condition.creat(ktrig.type, ktrig),
            triggerentity=ktrig.ref,
            triggeringrule=xosc.TriggeringEntitiesRule.any,
            triggeringpoint=triggeringpoint,
        )


@dataclass(order=True)
class CreatAction:
    def __post_init__(self):
        self._private_action = PrivateActionFactory()
        self._global_action = GlobalActionFactory()
        self._userdefined_action = UserDefinedActionFactory()

    @value_dispatch
    def choose(self, ktype: str, *args: tuple) -> str:
        # 泛型函数,通过传入参数 (ktype) 的类型不同而返回不同的处理结果
        target = (
            settings.sys.l4.PrivateActionType.enum.values()
            + settings.sys.l4.GlobalActionType.enum.values()
            + settings.sys.l4.UserDefinedActionType.enum.values()
        )
        return f"参数输入错误: {ktype} 仅应为 [{target}]中其一"

    @choose.register_for_all(settings.sys.l4.PrivateActionType.enum.values())
    def choose_private_actions(self, ktype: str, kaction: KAction):
        # print(f"{kaction = }")
        # return PrivateActionFactory().creat(kaction.type, kaction)
        return self._private_action.creat(kaction.type, kaction)

    @choose.register_for_all(settings.sys.l4.GlobalActionType.enum.values())
    def choose_global_actions(self, ktype: str, kaction: KAction):
        # print("in _global_action")
        # return GlobalActionFactory().creat(kaction.type, kaction)
        return self._global_action.creat(kaction.type, kaction)

    @choose.register_for_all(settings.sys.l4.UserDefinedActionType.enum.values())
    def choose_user_defined_actions(self, ktype: str, kaction: KAction):
        # return UserDefinedActionFactory().creat(kaction.type, kaction)
        return self._userdefined_action.creat(kaction.type, kaction)


# sourcery skip: remove-empty-nested-block, remove-redundant-if
if __name__ == "__main__":
    pass
