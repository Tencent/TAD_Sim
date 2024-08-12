#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file rule_physical.py
# * \brief
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass

import numpy as np
import pandas as pd
from loguru import logger

from excel2asam.config import settings
from excel2asam.utils import SceneFilter


@dataclass(order=True)
class RulePhysical:
    scenes: pd.DataFrame
    scene_filter: SceneFilter

    def data_preprocessing(self, scenes: pd.DataFrame) -> pd.DataFrame:
        # 创建中间态的参数列, 便于后续进行 handle_by_lanenum_and_relside 和 _set_lane_id
        # bugfix: (xingboliu) 20230510 当 cols_side 为空时(即所有场景中只存在主车)
        # feat: (xingboliu) 20230816 此时生成 "Sides" 列, 在后续处理中增加判断, 以减少步骤以及内存占用
        if cols_side := [x for x in scenes.columns if "Side" in x]:
            scenes[cols_side] = scenes[cols_side].ffill(axis=1).fillna(0).infer_objects(copy=False)
            scenes[settings.sys.l0.cols.sides] = (
                scenes[cols_side].astype("int8[pyarrow]").apply(lambda x: set(x), axis=1)
            )

        return scenes

    def handle_by_speed_range(self, scenes: pd.DataFrame) -> pd.DataFrame:
        logger.opt(lazy=True).info("handle_by_speed_range")

        threshold_speed_min_m_s = settings.sys.rule.speed.min / 3.6
        threshold_speed_max_m_s = settings.sys.rule.speed.max / 3.6

        ini_speed_df = scenes.filter(regex="Ini.Speed").fillna(0)
        in_range_cond = (ini_speed_df >= threshold_speed_min_m_s) & (ini_speed_df <= threshold_speed_max_m_s)

        # 找到所有包含 Ini.Speed 列, 删除不在速度范围内的场景
        scenes = self.scene_filter.process(
            df=scenes,
            keep_cond=in_range_cond.all(axis=1),
            reason=f"物理速度范围之外, [{threshold_speed_min_m_s}, {threshold_speed_max_m_s * 3.6}] kph",
            assessment="物理规则",
        )

        return scenes

    def handle_by_lanenum_and_relside(self, scenes: pd.DataFrame) -> pd.DataFrame:
        if settings.sys.l0.cols.sides not in self.scenes.columns:
            return scenes

        logger.opt(lazy=True).info("handle_by_lanenum_and_relside")

        lane_num = self.scenes["Lane.Num"].to_numpy()
        sides_values = self.scenes[settings.sys.l0.cols.sides].to_numpy()

        # 创建一个与sides_values形状相同的布尔数组, 用于存储有效条件
        valid = np.zeros_like(lane_num, dtype=bool)

        # Lane.Num = 1
        # 有效范围: 应仅为 {0}
        # 限制规则: 且元素只能有 0
        cond = lane_num == 1
        valid[cond] = np.array([all(a == 0 for a in x) for x in sides_values[cond]])

        # Lane.Num = 2
        # 有效范围: 应仅为 {0, -1, 1}
        # 限制规则: 不能同时出现 {-1, 1}
        cond = lane_num == 2
        valid[cond] = np.array([x.issubset({0, -1, 1}) and not {-1, 1}.issubset(x) for x in sides_values[cond]])

        # Lane.Num = 3
        # 有效范围: 应仅为 [0, -1, 1, -2, 2]
        # 限制规则: 不能同时出现 {-2, 2}
        cond = lane_num == 3
        valid[cond] = np.array([x.issubset({0, -1, 1, -2, 2}) and not {-2, 2}.issubset(x) for x in sides_values[cond]])

        # Lane.Num = 4
        # 有效范围: 应仅为 {0, -1, 1, -2, 2, -3, 3}
        # 限制规则: 不能同时出现 {-2, 2} {-2, 3} {-3, 2} {-3, 3}
        cond = lane_num == 4
        valid[cond] = np.array(
            [
                x.issubset({0, -1, 1, -2, 2, -3, 3})
                and not {-2, 2}.issubset(x)
                and not {-2, 3}.issubset(x)
                and not {-3, 2}.issubset(x)
                and not {-3, 3}.issubset(x)
                for x in sides_values[cond]
            ]
        )

        cond = pd.Series(valid, index=scenes.index)
        scenes = self.scene_filter.process(
            df=scenes,
            keep_cond=cond,
            reason="设置位置无车道, 车道.数量与关系.侧面参数不匹配",
            assessment="物理规则",
        )

        return scenes

    def process(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' RulePhysical Processing: ':-^35}")

        return (
            self.scenes.pipe(self.data_preprocessing)
            .pipe(self.handle_by_speed_range)
            .pipe(self.handle_by_lanenum_and_relside)
        )
