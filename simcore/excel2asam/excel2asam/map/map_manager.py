#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file mapper.py
# * \brief hadmap info calculation.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2021-11-11
# * \version 1.1.0
# * \copyright (c) 2021 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import List

from loguru import logger

from excel2asam.map.lib.hadmap_py import Map, Waypoint


@dataclass(order=True)
class MapManager:
    """地图方法封装, 使用上下文管理器方式"""

    pathfile: Path

    def __post_init__(self):
        """初始化 MapManager 类"""
        logger.opt(lazy=True).info("Loading hadmap")
        self.hadmap = Map()
        self.hadmap.load(str(self.pathfile))
        logger.opt(lazy=True).info("Finished Load hadmap")

    def __enter__(self):
        """进入上下文管理器"""
        return self  # .hadmap

    def __exit__(self, exc_type, exc_val, exc_tb):
        """退出上下文管理器"""
        self.hadmap.release()
        logger.opt(lazy=True).info("Released hadmap")

    def cal_rel_wpt(self, wpt: Waypoint, rel_side: int, rel_long: float, rel_lat: float, ini_heading: float) -> List:
        """
        计算相对路径点

        Args:
            wpt (Waypoint): 输入的路径点.
            rel_side (int): 相对侧面.
            rel_long (float): 相对经度.
            rel_lat (float): 相对纬度.
            ini_heading (float): 初始航向.

        Returns:
            List: 包含新路径点的列表.
        """
        new_wpts = []

        try:
            if output_wpt := wpt.next(distance=rel_long, direction="T"):  # NOCA:E999(设计如此)
                output_wpt.update(lane_id=-rel_side, offset=rel_lat, h=ini_heading)
                new_wpts.append(output_wpt)
        except Exception as e:
            logger.opt(lazy=True).error(
                (
                    f"Get Error: {e}",
                    f"Input info: {wpt.position = } {rel_side = } {rel_long = } {rel_lat = } {ini_heading =}",
                )
            )

        return new_wpts


# sourcery skip: remove-empty-nested-block
if __name__ == "__main__":
    pass
