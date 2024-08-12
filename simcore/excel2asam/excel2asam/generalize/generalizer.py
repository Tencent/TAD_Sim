#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file generalizer.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass
from functools import reduce
from string import Template

import numpy as np
import pandas as pd
from loguru import logger

from excel2asam.config import settings
from excel2asam.generalize.model_dbscan import ModelDbscan
from excel2asam.generalize.rule_physical import RulePhysical
from excel2asam.generalize.rule_rss import RuleRss
from excel2asam.utils import DtypeConverter, SceneFilter


@dataclass(order=True)
class GeneralizeAll:
    scenes: pd.DataFrame

    def generalization_by_cartesian_product(self, par_cols: list) -> None:
        def explode_column(df, col):
            return df.explode(col, ignore_index=True)

        logger.opt(lazy=True).info("generalization_by_cartesian_product")

        object_cols = self.scenes[par_cols].select_dtypes(include="object").columns

        # 使用 apply() 函数和 lambda 表达式替换 for 循环
        self.scenes = self.scenes.pipe(lambda df: reduce(explode_column, object_cols, df))

    def generalization_by_one_by_one(self, par_cols: list) -> None:
        logger.opt(lazy=True).info("generalization_by_one_by_one")

        suffix_one2one = settings.sys.suffix.one2one
        if cols_one2one := [x[: -len(suffix_one2one)] for x in par_cols if suffix_one2one in x]:
            headers = {
                lvl: [x for x in cols_one2one if any(hd in x for hd in settings.sys[lvl].sub1.header.transfer.values())]
                for lvl in ["l1", "l2", "l4", "l5"]
            }

            for cols in headers.values():
                if not cols:
                    continue
                logger.opt(lazy=True).debug(f"Deal cloumns: {cols}")

                # # Just for debug
                # self.scenes[cols].to_csv("check_explode.csv", encoding="utf_8_sig", index=True)
                # tmp = self.scenes[cols].map(lambda x: len(x) if isinstance(x, list) else "")
                # tmp["LengthSame"] = tmp.apply(lambda x: all(x == x[0]), axis=1)
                # tmp[["FuncId", "LogicId"]] = self.scenes[["FuncId", "LogicId"]]
                # tmp.to_csv("check_explode_len.csv", encoding="utf_8_sig", index=True)

                self.scenes = self.scenes.explode(cols, ignore_index=True)

            # 删除无用的后缀为 suffix_one2one 的列
            self.scenes.drop(columns=[x for x in self.scenes.columns if suffix_one2one in x], inplace=True)

        self.scenes = DtypeConverter(self.scenes).process().reset_index(drop=True)

    def _set_scenario_name_by_cond(self, cond: pd.Series) -> pd.Series:
        # TODO: 如果Naming列中写的是错误的变量名,则会报错,需要处理

        colset = (
            settings.sys.l1.sub1.header.transfer.values()
            + settings.sys.l2.sub1.header.transfer.values()
            + settings.sys.l4.sub1.header.transfer.values()
            + settings.sys.l5.sub1.header.transfer.values()
        )

        # Init naming_cols
        naming_cols = [s for s in self.scenes.columns if any(sub in s for sub in colset)]

        # Concatenating the values of the ConcreteId column and the calculated values of the naming column
        return pd.Series(
            np.where(
                cond,
                # 通过 Template.substitute 替换变量为值
                np.frompyfunc(lambda x, y: Template(x).substitute(y), 2, 1)(
                    self.scenes[settings.sys.generalized.cols.naming].astype(str).to_numpy(),
                    pd.DataFrame(
                        data=self.scenes[naming_cols].to_numpy(),
                        columns=[s.replace(".", "") for s in self.scenes[naming_cols].columns],
                    ).to_dict("records"),
                ),
                # 计算累加值
                self.scenes.groupby(settings.sys.l0.cols.concreteid, observed=True).cumcount().add(1).astype(str),
            )
        )

    def set_scenario_name(self) -> None:
        logger.opt(lazy=True).info("set_scenario_name")

        # 获取 naming mode, 如果配置中不存在则默认使用 parameter 模式
        naming_mode = settings.get("sys.naming", "parameter")

        # 处理 numerical 模式
        if naming_mode == "numerical":
            naming = self.scenes.groupby(settings.sys.l0.cols.concreteid, observed=True).cumcount().add(1).astype(str)
        # 处理 parameter 模式
        else:
            naming = self._set_scenario_name_by_cond(cond=self.scenes[settings.sys.generalized.cols.naming] != "")

        self.scenes[settings.sys.l0.cols.concreteid] = (
            self.scenes[settings.sys.l0.cols.concreteid]
            .str.cat(others=naming, sep=settings.sys.link_symbol)
            .str.replace(r"\+|%", "", regex=True)
        )

        # 处理重复的 ConcreteId
        if not self.scenes[settings.sys.l0.cols.concreteid].is_unique:
            self.scenes.drop_duplicates(subset=settings.sys.l0.cols.concreteid, keep="first", inplace=True)
            logger.opt(lazy=True).info("Name duplicate values, remove duplicate rows, keep only first")

        # 删除无用的后缀为 suffix_one2one 的列
        self.scenes.drop(columns=[settings.sys.generalized.cols.naming], inplace=True)

    def set_route_param(self) -> None:
        logger.opt(lazy=True).info("set_map_param_names")
        # 更改指定列的数据类型
        # self.scenes["Road.Curve"] = self.scenes["Road.Curve"].astype("int32[pyarrow]")
        # self.scenes["Junction.Num"] = self.scenes["Junction.Num"].astype("uint8[pyarrow]")
        # self.scenes["Lane.Num"] = self.scenes["Lane.Num"].astype("uint8[pyarrow]")
        # self.scenes["Lane.Num2"] = self.scenes["Lane.Num2"].astype("uint8[pyarrow]")
        # self.scenes["Lane.Num2Dist"] = self.scenes["Lane.Num2Dist"].astype("uint8[pyarrow]")

        #
        self.scenes[settings.sys.l0.cols.route_param] = (
            self.scenes[settings.sys.generalized.colset.hadmapids]
            .astype(str)
            .fillna("None")
            .apply(f"{settings.sys.link_symbol}".join, axis=1)
        )

        # self.scenes["RouteParm"].to_csv("RouteParm.csv", encoding="utf_8_sig", index=True)

    def unified_unit(self) -> None:
        logger.opt(lazy=True).info("unified_unit")

        val_unit_cols = [
            (x[: -len(settings.sys.suffix.unit)], x) for x in self.scenes.columns if settings.sys.suffix.unit in x
        ]

        # # 存储 val_unit_cols 列数据至csv文件
        # self.scenes[[x[0] for x in val_unit_cols]].to_csv("val_unit_cols.csv", encoding="utf_8_sig", index=True)

        # 换算单位至标准单位
        for col in val_unit_cols:
            self.scenes[col[0]] = np.round(
                self.scenes[col[0]].to_numpy() * self.scenes[col[1]].to_numpy(), settings.sys.decimal_places
            )

        # 删除无用的 unit_cols 的列
        self.scenes.drop(columns=[x[1] for x in val_unit_cols], inplace=True)

    def handle_default_data(self) -> None:
        logger.opt(lazy=True).info("handle_default_data")

        self.scenes.loc[:, "Env.Time"] = self.scenes["Env.Time"].where(
            cond=self.scenes["Env.Time"].isin([int(x) for x in settings.sys.l5.Time.transfer.values()]),
            other=int(list(settings.sys.l5.Time.transfer.values())[0]),
        )

        self.scenes.loc[:, "Env.Weather"] = self.scenes["Env.Weather"].where(
            cond=self.scenes["Env.Weather"].isin(settings.sys.l5.Wether.transfer.values()),
            other=list(settings.sys.l5.Wether.transfer)[0],
        )

    def change_veh_model(self) -> None:
        logger.opt(lazy=True).info("change_veh_model")
        # change ego
        self.scenes["Ego.Phy.Model"] = settings.sys.change.ego

        # change npc with veh
        for hname in [f"Npc{i}" for i in range(1, self.scenes[settings.sys.l0.cols.allnum_npc].max() + 1)]:
            if f"{hname}.Phy.Model" in self.scenes.columns:
                cond1 = self.scenes[f"{hname}.Phy.Category"] == "Veh"
                cond2 = self.scenes[f"{hname}.Phy.Model"].isin(["moto_001", "bike_001", "elecBike_001"])
                self.scenes.loc[cond1 & ~cond2, f"{hname}.Phy.Model"] = settings.sys.change.veh

    def process(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' GeneralizationFormater Processing: ':-^35}")

        par_cols = [x for x in self.scenes.columns if "." in x and settings.sys.suffix.unit not in x]
        # 交叉参数组合
        self.generalization_by_cartesian_product(par_cols)

        # 合并参数组合
        self.generalization_by_one_by_one(par_cols)

        # 创建场景名称
        self.set_scenario_name()

        #
        self.set_route_param()

        # 统一物理单位
        self.unified_unit()

        # 处理数据范围
        self.handle_default_data()

        # 更改 veh_model
        if settings.sys.change.enable == 1:
            self.change_veh_model()

        return DtypeConverter(self.scenes).process().reset_index(drop=True)


@dataclass(order=True)
class RuleFilter:
    scenes: pd.DataFrame
    scene_filter: SceneFilter

    def process(self) -> pd.DataFrame:
        self.scenes = RulePhysical(self.scenes, self.scene_filter).process()

        self.scenes = RuleRss(self.scenes, self.scene_filter).process()

        return self.scenes


@dataclass(order=True)
class ModelFilter:
    scenes: pd.DataFrame
    scene_filter: SceneFilter

    def process(self) -> pd.DataFrame:
        self.scenes = ModelDbscan(self.scenes, self.scene_filter).process()

        return self.scenes
