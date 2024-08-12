#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file formater.py
# * \brief Read excel info which define by user, and then format info for gen xosc and xodr.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2023-03-03
# * \version 2.2
# * \copyright (c) 2023 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import pandas as pd
from loguru import logger

from excel2asam.config import settings
from excel2asam.map.lib.hadmap_py import Orientation, Position, PositionLane, Waypoint
from excel2asam.map.map_manager import MapManager
from excel2asam.utils import SceneFilter


@dataclass(order=True)
class VirtualMapper:
    scenes: pd.DataFrame

    def __post_init__(self) -> None:
        self.wpt_cols = ["road_id", "lane_id", "offset", "s", "h"]
        self.start_road_id = int(settings.sys.map.virtual.start_road_id)

    def _set_mapfile(self) -> pd.Series:
        logger.opt(lazy=True).debug("设置虚拟地图名称")
        return self.scenes[settings.sys.l0.cols.route_param].cat.rename_categories(
            lambda x: x + settings.sys.map.virtual.suffix
        )

    def _cal_length_max_curve(self, radius: np.ndarray, cond: np.ndarray) -> np.ndarray:
        return (2 * 3.14 * radius[cond] * settings.sys.map.virtual.ratio_of_len_to_perimeter_curve).astype(int)

    def _cal_length_mixed_curve_straight(self, radius: np.ndarray, cond: np.ndarray, std_base_len: int) -> np.ndarray:
        straight_len = int(settings.sys.map.virtual.base_len * settings.sys.map.virtual.ratio_of_len_to_total_straight)
        curve_len = np.minimum(self._cal_length_max_curve(radius, cond), (std_base_len - straight_len)).astype(int)

        return straight_len + curve_len

    def _set_total_length(self) -> np.ndarray:
        logger.opt(lazy=True).debug("设置虚拟地图总长度")

        road_type = self.scenes["Road.Type"].to_numpy()
        radius = self.scenes["Road.Curve"].to_numpy()

        total_length = np.zeros_like(road_type, dtype=int)

        std_base_len = int(settings.sys.map.virtual.base_len)

        # 直道 & 弯道S = std_base_len
        cond = np.isin(road_type, [settings.sys.l1.RoadType.enum.straight, settings.sys.l1.RoadType.enum.curveS])
        total_length[cond] = std_base_len

        # 弯道中 = set_len_max_curve
        cond = np.isin(road_type, [settings.sys.l1.RoadType.enum.curvIng])
        total_length[cond] = self._cal_length_max_curve(radius, cond)

        # 弯道入口 & 弯道出口 _handle_mixed_case
        cond = np.isin(road_type, [settings.sys.l1.RoadType.enum.curveIn, settings.sys.l1.RoadType.enum.curveOut])
        total_length[cond] = self._cal_length_mixed_curve_straight(radius, cond, std_base_len)

        return total_length

    def _set_lane_id(self, cond: pd.Series) -> np.ndarray:
        # 设置虚拟地图时, "Ego.Ini.Laneid"
        lane_num = self.scenes.loc[cond, "Lane.Num"].to_numpy()

        # 创建一个与 lane_num 形状相同的数组, 用于存储 Ego.Ini.Laneid
        lane_id = np.zeros_like(lane_num, dtype=int)

        # Lane.Num = 1 或 2 时: 固定值 -1
        lane_id[lane_num == 1] = -1
        lane_id[lane_num == 2] = -1
        # Lane.Num = 3 或 4 时: 固定值 -2
        lane_id[lane_num == 3] = -2
        lane_id[lane_num == 4] = -2

        if settings.sys.l0.cols.sides in self.scenes.columns:
            sides_values = self.scenes.loc[cond, settings.sys.l0.cols.sides].to_numpy()
            # Lane.Num = 1时: 固定值 -1

            # Lane.Num = 2时: 都是 L1: -2; 其余: -1
            cond = lane_num == 2
            lane_id[cond] = np.array([-2 if any(a == -1 for a in x) else -1 for x in sides_values[cond]])

            # Lane.Num = 3时: 存在任一 L2: -3; 存在任一 R2: -1; 其余: -2
            cond = lane_num == 3
            lane_id[cond] = np.array([-3 if -2 in x else (-1 if 2 in x else -2) for x in sides_values[cond]])

            # Lane.Num = 4时: 都是 Same/L1/R1/至少有一个R2: -2; 至少有一个L2: -3; 至少有一个R3: -1; 至少有一个L3: -4
            cond = lane_num == 4
            lane_id[cond] = np.array(
                [-2 if len(x) == 1 else (-3 if -2 in x else (-1 if 3 in x else -4)) for x in sides_values[cond]]
            )

            self.scenes.drop(columns=[settings.sys.l0.cols.sides], inplace=True)

        return lane_id

    def _serialize_wpt(self, row: np.ndarray) -> Waypoint:
        # print(f"{row = }")
        return Waypoint(
            position=Position(
                lane=PositionLane(
                    road_id=int(row[0]),
                    lane_id=int(row[1]),
                    offset=row[2],
                    s=row[3],
                    orientation=Orientation(h=row[4], p=0.0, r=0.0),
                ),
            ),
        )

    def _define_mapping(self) -> dict:
        # start_road_id, end_road_id, start_diff_lane_id, end_diff_lane_id, start_diff_s, end_diff_s
        mapping = [
            # none
            (0, 0, 0, 0, 0, -1),
            # w2w
            (0, 0, -1, 1, -1, -1),
            # w2e
            (0, 3, -99, -99, -1, 0),
            # w2n
            (0, 2, -1, -1, -1, 0),
            # w2s
            (0, 1, -98, -98, -1, 0),
            # e2w
            (3, 0, 99, 99, 0, -1),
            # e2e
            (3, 3, 1, -1, 0, 0),
            # e2n
            (3, 2, 98, 98, 0, 0, 0),
            # e2s
            (3, 1, 1, -1, 0, 0),
            # n2w
            (2, 0, 98, 98, 0, -1),
            # n2e
            (2, 3, 1, -1, 0, 0),
            # n2n
            (2, 2, 1, -1, 0, 0),
            # n2s
            (2, 1, 99, -99, 0, 0),
            # s2w
            (1, 0, 1, 1, 0, -1),
            # s2e
            (1, 3, 98, -98, 0, 0),
            # s2n
            (1, 2, 99, -99, 0, 0),
            # s2s
            (1, 1, 1, -1, 0, 0),
            # opposite
            (0, 0, 1, 1, 0, 0),
            # entryIng
            (3, 2, 0, -98, -1, 0),
            # exitIng
            (0, 3, -98, -1, -1, 0),
            # mainIng
            (0, 2, -98, -98, -1, 0),
            # sideIng
            (),
            # roundaboutEntry
            (),
            # roundaboutExit
            (),
            # roundaboutIng
            (),
        ]

        return {index: value for index, value in enumerate(mapping)}

    def _get_ref_value(self, cond: pd.Series, hname: str, relname: str) -> np.ndarray:
        dfn = self.scenes.loc[cond]

        if f"{hname}.Rel.Ref" not in dfn.columns:
            return np.zeros(len(dfn))

        # 当前 hname 的参考者的纵向相对距离 (参考为 Ego 时为 0)
        rel_ref_colname = dfn[f"{hname}.Rel.Ref"].cat.rename_categories(lambda x: f"{x}{relname}")

        idx, cols = pd.factorize(rel_ref_colname)

        return dfn.reindex(cols, axis=1).fillna(0).to_numpy()[np.arange(len(dfn)), idx]

    def _rule_lane_id_with_route(self, cond: pd.Series, cond_route_none: pd.Series, route: pd.Series) -> np.ndarray:
        # route 用户设置为无时, 使用 Ego.Ini.Laneid

        # else
        # 如果 route1[2] 为98或-1, 将 route1[2] 除以98在乘列2
        cond_route_98 = np.isin(route.to_numpy(), [98, -98])
        # 如果 route1[2] 为99或-99且列2为1或2, 将 route1[2] 除以99在乘1
        # 如果 route1[2] 为99或-99且列2为3或4, 将 route1[2] 除以99在乘2
        cond_route_99 = np.isin(route.to_numpy(), [99, -99])
        cond_lane_1_2 = np.isin(self.scenes.loc[cond, "Lane.Num"].to_numpy(), [1, 2])
        cond_lane_3_4 = np.isin(self.scenes.loc[cond, "Lane.Num"].to_numpy(), [3, 4])

        # 其它情况下, 则等于 route1[2]
        return np.where(
            cond_route_none,
            self.scenes.loc[cond, "Ego.Ini.Laneid"].to_numpy(),
            np.where(
                cond_route_98,
                route.to_numpy() / 98 * self.scenes.loc[cond, "Lane.Num"].to_numpy(),
                np.where(
                    cond_route_99 & cond_lane_1_2,
                    route.to_numpy() / 99 * 1,
                    np.where(
                        cond_route_99 & cond_lane_3_4,
                        route.to_numpy() / 99 * 2,
                        route.to_numpy(),
                    ),
                ),
            ),
        )

    def _set_wpts_start(self, hname: str, cond: pd.Series, cond_route: pd.Series, route1: pd.Series) -> np.ndarray:
        logger.opt(lazy=True).info(f"设置 {hname}.Ini.Wpts Start")

        cond_route_none = self.scenes.loc[cond, f"{hname}.Ini.Route"] == int(settings.sys.l4.route.transfer.values()[0])
        cond_route_opp = self.scenes.loc[cond, f"{hname}.Ini.Route"] == int(settings.sys.l4.route.transfer.values()[17])

        # 设置起点的 road_id 值
        self.scenes.loc[cond, self.wpt_cols[0]] = self.start_road_id + route1.str[0]

        # 设置起点的 offset 值
        self.scenes.loc[cond, self.wpt_cols[2]] = self.scenes.loc[cond, f"{hname}.Ini.Latoffset"].to_numpy()

        # 设置起点的 h 值
        self.scenes.loc[cond, self.wpt_cols[4]] = self.scenes.loc[cond, f"{hname}.Ini.Heading"].to_numpy()

        # 设置起点的 s 值
        # route 用户设置为无时, 设置为配置文件中的 start_s, 其余为 0
        # 如果存在相对关系时 (考虑不同 route 的特殊需求), 加上关系的值, 公式如下
        # ref_long = abs(k * s + b + b_ref)
        # 其中 k = 0 / -1, s 是地图总长度除以(junction数量+1), b 是纵向相对距离, b_ref 是参考者的纵向相对距离
        # TODO: (xingboliu) 暂时不支持: 参考: 碰撞点 和 参考 Ego 这种形式同时存在
        self.scenes.loc[cond, self.wpt_cols[3]] = np.where(
            cond_route_none | cond_route_opp,
            float(settings.sys.map.virtual.start_s),
            float(settings.sys.map.virtual.junction_offset_s),
        )
        if f"{hname}.Rel.Long" in self.scenes.columns:
            self.scenes.loc[cond, self.wpt_cols[3]] = np.abs(
                route1.str[4].to_numpy()
                * self.scenes.loc[cond, settings.sys.l0.cols.total_length].to_numpy()
                / (self.scenes.loc[cond, "Junction.Num"].to_numpy() + 1)
                + self.scenes.loc[cond, f"{hname}.Rel.Long"].to_numpy()
                + self._get_ref_value(cond, hname, ".Rel.Long")
                + self.scenes.loc[cond, self.wpt_cols[3]].to_numpy()
            ).astype(float)

        # 设置起点的 lane_id
        self.scenes.loc[cond, self.wpt_cols[1]] = self._rule_lane_id_with_route(cond, cond_route_none, route1.str[2])
        if f"{hname}.Rel.Side" in self.scenes.columns:
            self.scenes.loc[cond, self.wpt_cols[1]] = (
                self.scenes.loc[cond, self.wpt_cols[1]]
                - self.scenes.loc[cond, f"{hname}.Rel.Side"]
                - self._get_ref_value(cond, hname, ".Rel.Side")
            )

        return np.apply_along_axis(self._serialize_wpt, 1, self.scenes.loc[cond, self.wpt_cols].to_numpy())

    def _set_wpts_end(self, hname: str, cond: pd.Series, cond_route: pd.Series, route1: pd.Series) -> np.ndarray:
        logger.opt(lazy=True).info(f"设置 {hname}.Ini.Wpts End")

        cond_route_none = self.scenes.loc[cond, f"{hname}.Ini.Route"] == int(settings.sys.l4.route.transfer.values()[0])
        cond_route_opp = self.scenes.loc[cond, f"{hname}.Ini.Route"] == int(settings.sys.l4.route.transfer.values()[17])

        # 设置终点的 road_id 值
        self.scenes.loc[cond, self.wpt_cols[0]] = self.start_road_id + route1.str[1]

        # 设置终点的 lane_id
        self.scenes.loc[cond, self.wpt_cols[1]] = self._rule_lane_id_with_route(cond, cond_route_none, route1.str[3])

        # 设置终点的 s 值
        self.scenes.loc[cond, self.wpt_cols[3]] = np.abs(
            route1.str[5].to_numpy()
            * self.scenes.loc[cond, settings.sys.l0.cols.total_length].to_numpy()
            / (self.scenes.loc[cond, "Junction.Num"].to_numpy() + 1)
            + abs(settings.sys.map.virtual.end_s_offset)
        )

        #
        end_wpts = np.apply_along_axis(self._serialize_wpt, 1, self.scenes.loc[cond, self.wpt_cols].to_numpy())

        return np.where(~(cond_route_none | cond_route_opp), end_wpts, np.where(hname == "Ego", end_wpts, 0))

    def _set_trafficlight(self):
        logger.opt(lazy=True).info("设置 trafficlight route")
        self.scenes["Junction.Type"]
        self.scenes["Trafficlight.Status"]

        # leg_mapping = settings.sys.l1.JunctionType.leg.get(key=scene.get("Junction.Type"), default=0)

    def process(self) -> pd.DataFrame:
        """
        设置虚拟地图时, "Ego.Ini.Wpts" 和 "Npc.Ini.Wpts" 的值
        """
        logger.opt(lazy=True).info("set_wpts_virtualmap")

        # 设置 mapfile 虚拟地图名称, 后续生成地图名字(带后缀)
        self.scenes[settings.sys.l0.cols.mapfile] = self._set_mapfile()

        # 设置虚拟地图道路总长度
        self.scenes[settings.sys.l0.cols.total_length] = self._set_total_length()

        # 设置 Ego & Npc 的 Wpts 值
        for hname in ["Ego", *[f"Npc{i}" for i in range(1, self.scenes[settings.sys.l0.cols.allnum_npc].max() + 1)]]:
            logger.opt(lazy=True).debug(f"处理 {hname}")

            # 是否存在该交通参与者
            cond = self.scenes[f"{hname}.Phy.Model"].notna()

            # 异常处理, 所有行不存在任意一个 hname
            if not cond.any():
                continue

            # 设置 Ini.Wpts
            if hname == "Ego":
                self.scenes.loc[cond, "Ego.Ini.Laneid"] = pd.Series(self._set_lane_id(cond), dtype="int8[pyarrow]")

            cond_route = self.scenes.loc[cond, f"{hname}.Ini.Route"] == int(settings.sys.l4.route.transfer.values()[0])
            route1 = self.scenes.loc[cond, f"{hname}.Ini.Route"].map(self._define_mapping())

            self.scenes.loc[cond, f"{hname}.Ini.Wpts"] = pd.Series(
                data=np.column_stack(
                    (
                        self._set_wpts_start(hname, cond, cond_route, route1),
                        self._set_wpts_end(hname, cond, cond_route, route1),
                    )
                ).tolist(),
                index=cond.index[cond],
            )

        # 清理临时列
        self.scenes.drop(
            columns=[*self.wpt_cols, settings.sys.l0.cols.route_param, *self.scenes.filter(regex="Ini\.Route").columns],
            inplace=True,
        )

        return self.scenes.reset_index(drop=True)


@dataclass(order=True)
class RealMapper:
    scenes: pd.DataFrame
    map_manager: MapManager
    param: dict
    scene_filter: SceneFilter

    def __post_init__(self):
        pass

    def _update_offset_heading_to_wpts0(self, cond: pd.Series, cols: list) -> None:
        self.scenes.loc[cond, cols[0]] = self.scenes.loc[cond, cols].apply(
            lambda row: [
                wpt.update(offset=row[cols[1]], h=row[cols[2]])
                # 找到第一个元素, 且该元素不为 None 时, 增加 offset
                if i == 0 and wpt
                # 其余情况, 返回原值
                else wpt
                for i, wpt in enumerate(row[cols[0]])
            ],
            axis=1,
        )

    def process(self) -> pd.DataFrame:
        """
        设置真实地图时, "Ego.Ini.Wpts" 和 "Npc.Ini.Wpts" 的值
        """
        logger.opt(lazy=True).info("set_wpts_realmap")

        # 将 param 参数的所有 wpt 的 position 类型统一为 lane
        for key, route in self.param.items():
            self.param[key] = [Waypoint(map=self.map_manager.hadmap, position=position) for position in route]
        logger.opt(lazy=True).debug(f"finished param format: {self.param}")

        # 1. 方法 np.vectorize
        np_wgs84 = np.vectorize(self.param.get, otypes=[list])
        np_cal_rel_wpt = np.vectorize(self.map_manager.cal_rel_wpt, otypes=[dict])

        # 2. Ego info and set xodr
        # 通过 param 直接获取 Ego 的 Ini.Wpts, 并将第一个点增加 offset 和 Heading
        self.scenes["Ego.Ini.Wpts"] = np_wgs84(
            self.scenes[settings.sys.l0.cols.route_param]
            .str.cat(self.scenes["Ego.Ini.Route"].astype("string"), sep=settings.sys.link_symbol)
            .to_numpy()
        )
        self._update_offset_heading_to_wpts0(
            cond=self.scenes["Ego.Ini.Wpts"].notna(), cols=["Ego.Ini.Wpts", "Ego.Ini.Latoffset", "Ego.Ini.Heading"]
        )

        # # Just For debug
        # self.scenes["ego_map_route"] = self.scenes[settings.sys.l0.cols.route_param].str.cat(
        #     self.scenes["Ego.Ini.Route"].astype("string"), sep=settings.sys.link_symbol
        # )
        # self.scenes[["FuncLogicId", "HadmapIdOrigin", "ego_map_route", "Ego.Ini.Wpts"]].to_csv(
        #     f"{settings.sys.sheet.define}_ego_wpt.csv", encoding="utf_8_sig", index=True
        # )

        # 过滤 MapSDK 查找不到位置点的行, 并记录信息
        self.scenes = self.scene_filter.process(
            df=self.scenes,
            # 找到具有 Ego 和 Wpts 名字的列, 对应的每一行中存在任一 nan 的情况, 存在为 False, 不存在为 True
            keep_cond=~self.scenes.filter(regex="Ego.*Wpts|Wpts.*Ego").isna().any(axis=1),
            reason="基于真实地图, 读取设置的 Ego 位置点, 在地图中查找位置不存在道路",
            assessment="物理规则",
        )

        # 异常处理, 提前返回
        if self.scenes.empty:
            logger.opt(lazy=True).debug("scenes is empey")
            return self.scenes

        # 3. Npc info and set xodr
        # for i in range(1, self.scenes[settings.sys.l0.cols.allnum_npc].max() + 1):
        route_none_value = int(settings.sys.l4.route.transfer.values()[0])
        for hname in [f"Npc{i}" for i in range(1, self.scenes[settings.sys.l0.cols.allnum_npc].max() + 1)]:
            # 处理整列都不存在 Rel.Ref 关系的 Npc 列情况
            if f"{hname}.Rel.Ref" not in self.scenes.columns:
                self.scenes[f"{hname}.Rel.Ref"] = np.nan

            # 条件: 任一列存在 Rel.Ref 关系的 Npc 列情况
            cond_rel = self.scenes[f"{hname}.Rel.Ref"].notna()
            # 处理不存在 Rel 的 Npc, 通过 param 直接获取 {hname} 的 Ini.Wpts, 并将第一个点增加 offset 和 Heading
            self.scenes.loc[~cond_rel, f"{hname}.Ini.Wpts"] = np_wgs84(
                self.scenes.loc[~cond_rel, settings.sys.l0.cols.route_param].str.cat(
                    self.scenes.loc[~cond_rel, f"{hname}.Ini.Route"].astype("string"), sep=settings.sys.link_symbol
                ),
            )
            self._update_offset_heading_to_wpts0(
                cond=self.scenes[f"{hname}.Ini.Wpts"].notna(),
                cols=[f"{hname}.Ini.Wpts", f"{hname}.Ini.Latoffset", f"{hname}.Ini.Heading"],
            )

            # 处理存在 Rel 的 Npc
            if cond_rel.any():
                self.scenes.loc[cond_rel, f"{hname}.Ini.Wpts"] = np.where(
                    # 条件
                    self.scenes.loc[cond_rel, f"{hname}.Ini.Route"].fillna(route_none_value) == route_none_value,
                    # 处理存在 Route 设置为 none, 通过相对 Ego 关系计算的交通参与者位置点
                    np_cal_rel_wpt(
                        self.scenes.loc[cond_rel].apply(lambda x: x[f"{x[f'{hname}.Rel.Ref']}.Ini.Wpts"][0], axis=1),
                        self.scenes.loc[cond_rel, f"{hname}.Rel.Side"].to_numpy(),
                        self.scenes.loc[cond_rel, f"{hname}.Rel.Long"].to_numpy(),
                        self.scenes.loc[cond_rel, f"{hname}.Ini.Latoffset"].to_numpy(),
                        self.scenes.loc[cond_rel, f"{hname}.Ini.Heading"].to_numpy(),
                    ),
                    # 处理 Route 设置不为 none, 通过 param 直接获取 {hname} 的 Ini.Wpts
                    np_wgs84(
                        self.scenes.loc[cond_rel, settings.sys.l0.cols.route_param].str.cat(
                            self.scenes.loc[cond_rel, f"{hname}.Ini.Route"].astype("string"),
                            sep=settings.sys.link_symbol,
                        ),
                    ),
                )

        # 过滤 MapSDK 查找不到位置点的行, 并记录信息
        self.scenes = self.scene_filter.process(
            df=self.scenes,
            # 找到具有 Npc 和 Wpts 名字的列, 对应的每一行中存在任一 [] 的情况, 存在为 False, 不存在为 True
            keep_cond=~self.scenes.filter(regex="Npc.*Wpts|Wpts.*Npc").apply(
                lambda row: any(isinstance(item, list) and len(item) == 0 for item in row), axis=1
            ),
            reason="基于真实地图,  通过相对 Ego 关系计算的交通参与者位置点, 不存在",
            assessment="物理规则",
        )

        # 清理临时列
        route_cols = self.scenes.filter(regex="Ini\.Route").columns
        self.scenes.drop(columns=[settings.sys.l0.cols.route_param, *route_cols], inplace=True)

        return self.scenes.reset_index(drop=True)
