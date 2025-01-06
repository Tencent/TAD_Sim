#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file formater.py
# * \brief Read excel info which define by user, and then format info for gen xosc and xodr.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import re
from dataclasses import dataclass, field
from functools import reduce
from typing import Dict, Optional, Union

import excel2asam.exceptions as exp
import numpy as np
import pandas as pd
from loguru import logger

from excel2asam.config import settings
from excel2asam.map.lib.hadmap_py import Position, PositionLane, PositionWorld
from excel2asam.utils import DtypeConverter, SceneFilter, convert_string, raise_error_reason_by_diff, safe_literal_eval

pd.options.mode.copy_on_write = True
pd.options.future.no_silent_downcasting = True


@dataclass(order=True)
class _Define:
    paramter: str = field(repr=False, init=False, default="[\w\%\*\&\.\s\-\+\/\@\>\<\=]+")
    paramter_unit: str = field(repr=False, init=False, default="[\d\%\*\&\.\s\-\+\/\@\>\<\=]+")
    paramter_one2one: str = field(repr=False, init=False, default="[-+\\d\\.\\w\\*\\/\\%]+")
    # try_paramter: str = field(repr=False, init=False, default=" [-+\d\w%*&.\s/@><=]+")

    @classmethod
    def translate_character(cls) -> dict:
        return {
            # 空格
            " ": "",
            # 中文标点
            "，": ",",
            "。": ".",
            "：": ":",
            "；": ";",
            "【": "[",
            "】": "]",
            "》": ">",
            "《": "<",
            "≤": "<=",
            "≥": ">=",
            "＞": ">",
            "＜": "<",
            "！": "!",
            # 连续重复书写标点
            r"(,)+": r"\1",
            r"(:)+": r"\1",
            r"(>)+": r"\1",
            r"(<)+": r"\1",
            r"(!)+": r"\1",
            r"(\[)+": r"\1",
            r"(\])+": r"\1",
            # 处理所有开头结尾存在  换行或空行
            r"^[\n\s]+|[\n\s]+$": "",
        }

    @classmethod
    def translate_header(cls) -> dict:
        return {
            **settings.sys.l1.sub1.header.transfer,
            **settings.sys.l1.sub2.header.transfer,
            **settings.sys.l2.sub1.header.transfer,
            **settings.sys.l2.sub2.header.transfer,
            **settings.sys.l4.sub1.header.transfer,
            **settings.sys.l4.sub2.header.transfer,
            **settings.sys.l5.sub1.header.transfer,
            **settings.sys.l5.sub2.header.transfer,
        }

    @classmethod
    def translate_l0(cls) -> dict:
        return {
            # 格式化 Naming 列内的字符串
            settings.sys.generalized.cols.naming: {**_Define.translate_header(), r"\.": ""},
            # 格式化 Description 列内的字符串
            settings.sys.generalized.cols.description: {r"\n": " "},
        }

    @classmethod
    def translate_l1(cls) -> dict:
        default = {r"(无|没有|None|No|-)$": "0"}

        return {
            # 格式化 l1 列内的字符串
            "Road.Type": settings.sys.l1.RoadType.transfer,
            "Road.Curve": default,
            "Road.Direction": settings.sys.l1.roaddirection.transfer,
            "Road.Speedlimit": settings.sys.l1.roadspeedlimit.transfer,
            "Junction.Type": settings.sys.l1.JunctionType.transfer,
            "Junction.Num": default,
            "Lane.Type": settings.sys.l1.LaneType.transfer,
            "Lane.Num": default,
            "Lane.Num2": default,
            "Lane.Num2Dist": default,
            "Lane.Width": default,
            "Lane.Edge": settings.sys.l1.laneedge.transfer,
            "Marking.Type": {**settings.sys.l1.roadmarktype.transfer, **settings.sys.l1.roadmarkcolor.transfer},
            "Marking.Status": settings.sys.l1.roadmarkstatus.transfer,
        }

    @classmethod
    def translate_hadmapid(cls) -> list:
        return [*_Define.translate_l1().values(), settings.sys.l4.route.transfer]

    @classmethod
    def translate_l2(cls) -> dict:
        return {
            # 格式化 l2 列内的字符串
            "Trafficlight.Status": settings.sys.l2.pos.transfer,
            "Trafficlight.Cycle": settings.sys.l2.status.transfer,
        }

    @classmethod
    def translate_l4(cls, columns: list) -> dict:
        trigger_cols = [col for col in columns if "Trigger" in col]
        action_cols = [col for col in columns if "Action" in col]
        route_cols = [col for col in columns if "Route" in col]
        side_cols = [col for col in columns if "Side" in col]

        return {
            # 格式化 l4 列内的字符串
            # ""
            **dict.fromkeys(route_cols, settings.sys.l4.route.transfer),
            **dict.fromkeys(side_cols, settings.sys.l4.side.transfer),
            **dict.fromkeys(
                trigger_cols,
                {
                    **settings.sys.l4.ValueTriggerType.transfer,
                    **settings.sys.l4.EntityTriggerType.transfer,
                    **settings.sys.l4.DistMode.transfer,
                    **settings.sys.l4.rulemode.transfer,
                },
            ),
            **dict.fromkeys(
                action_cols,
                {
                    **settings.sys.l4.action.transfer,
                    **settings.sys.l4.EndtypeMode.transfer,
                    **settings.sys.l4.PrivateActionType.transfer,
                    **settings.sys.l4.GlobalActionType.transfer,
                    **settings.sys.l4.UserDefinedActionType.transfer,
                },
            ),
        }

    @classmethod
    def translate_l5(cls) -> dict:
        return {
            # 格式化 l5 列内的字符串
            "Env.Time": settings.sys.l5.Time.transfer,
            "Env.Weather": settings.sys.l5.Wether.transfer,
        }

    @classmethod
    def mapping_info(cls) -> dict:
        return {
            rf"^({cls.paramter}):": r"{'\1':",
            rf"\n({cls.paramter}):": r"'\1':",
            rf"\[({cls.paramter}):": r"{'\1':",
            rf":({cls.paramter}),": r":'\1',",
            rf",({cls.paramter}):": r",'\1':",
            rf":({cls.paramter})\]": r":'\1'},",
            r"$": "}",
        }

    @classmethod
    def pattern_trigger(cls) -> str:
        """
        > The pattern of trigger

        Returns:
          A string.
            - Ref
            - Disttype
            - Type
            - Rule
            - Value
        """
        trigger_types = settings.sys.l4.ValueTriggerType.enum.values() + settings.sys.l4.EntityTriggerType.enum.values()

        return (
            "与?(Ego|Npc\d+)?"
            f"({'|'.join(settings.sys.l4.DistMode.enum.values())})?"
            f"({'|'.join(trigger_types)})"
            f"({'|'.join(settings.sys.l4.rulemode.transfer.values())})"
            f"({cls.paramter})"
        )

    @classmethod
    def pattern_action(cls) -> str:
        """
        > The pattern of action

        Returns:
          A string.
            - Type
            - Value
            - Endtype
            - Endvalue
        """
        action_type = "|".join(
            settings.sys.l4.PrivateActionType.enum.values()
            + settings.sys.l4.GlobalActionType.enum.values()
            + settings.sys.l4.UserDefinedActionType.enum.values()
        )

        return (
            f"({action_type})"
            f"=({cls.paramter})"
            f"@({'|'.join(settings.sys.l4.EndtypeMode.enum.values())})"
            f"=({cls.paramter})"
        )

    @classmethod
    def pattern_unit(cls) -> str:
        return f"({cls.paramter_unit})({'|'.join(settings.sys.unitunified.transfer.keys())})?"

    @classmethod
    def pattern_one2one(cls) -> str:
        return f"({cls.paramter_one2one})({settings.sys.sign.one2one}?)"

    @classmethod
    def pattern_lanenum(cls) -> str:
        return "(\d+)[^\d]?(\d+)?@?(\d+[a-zA-Z])?"


@dataclass(order=True)
class _LogicFormater:
    scenes: pd.DataFrame
    scene_filter: SceneFilter

    def __post_init__(self):
        self.support_cols = [settings.sys.generalized.cols[level] for level in ("l1", "l2", "l4", "l5")]

    def _postl4(self, string: str) -> str:
        # print(string)
        # 将 L4 格式转换和其他相同
        out = []

        # 预处理
        string = string.strip().replace(":[", ",").replace("]", "")
        # 遍历每一行
        for item in string.split("\n"):
            # 获得 head, 构建为 X.X (如 Phy.Type)
            head, *line = item.split(",")
            # 特殊处理, 遇到 Npc 后重置
            if not line:
                if "Npc" in head:
                    out.append("]\n")
                out.append(f"{head}[")
            else:
                # 如果 head 为空, 表示出现了空行, 跳过
                if not head:
                    continue
                # 如果 len(line) == 1, 表示出现了只有一个值的行, 重置 line
                line = line[0].split(".") if len(line) == 1 else line
                out.extend(f"{head}.{x}," for x in line)

        out.append("]")
        return "".join(out).replace(",]", "]")

    def data_preprocessing(self) -> None:
        logger.opt(lazy=True).info("data_preprocessing")
        # step2: 英文大小写统一
        # 处理 l1, l2, l5, Naming 列所有字符, 都转换为 title 形式
        cols = [
            settings.sys.generalized.cols.l1,
            settings.sys.generalized.cols.l2,
            settings.sys.generalized.cols.l5,
            settings.sys.generalized.cols.naming,
        ]
        self.scenes[cols] = self.scenes[cols].apply(lambda y: y.str.title())
        # 处理 l4, 除了 型号 和 ] 之间的所有字符(模型名称对大小写敏感), 都转换为 title 形式
        pattern = re.compile(r"(型号:*[^]]+\])")
        self.scenes[settings.sys.generalized.cols.l4] = np.frompyfunc(
            lambda s: "".join([x if pattern.match(x) else x.title() for x in pattern.split(s)]), 1, 1
        )(
            self.scenes[settings.sys.generalized.cols.l4].to_numpy(),
        )

        # step3: 去除不含 Ego 的行, 每个用例至少包含一个 Ego
        self.scenes = self.scene_filter.process(
            df=self.scenes,
            keep_cond=self.scenes[settings.sys.generalized.cols.l4].str.contains(r"^Ego", na=False),
            reason="每一行场景至少包含一个主车 Ego",
            assessment="设计规则",
            exp_class=exp.AfterFilterEmptyError,
        )

        # step4: 预处理 L4 格式
        self.scenes[settings.sys.generalized.cols.l4] = np.frompyfunc(self._postl4, 1, 1)(
            self.scenes[settings.sys.generalized.cols.l4]
            .replace(
                to_replace={
                    # 遇到 L4 格式全部为一行情况, 如通过在线文档下载后很容易会变为这种情况
                    "Ego:物理": "Ego:\n物理",
                    "Ego物理": "Ego:\n物理",
                    r"(Npc\d+):物理": r"\1:\n物理",
                    r"(Npc\d+)物理": r"\1:\n物理",
                    r"\](?!\n)": "]\n",
                    # 考虑用户输入 Ego 和 Npc 时为未输入 : 的情况
                    "Ego\n": "Ego:\n",
                    r"(Npc\d+)\n": r"\1:\n",
                    # 考虑用户输入 DistMode 和 TriggerType 反了的情况
                    "ttc坐标": "坐标Ttc",
                    "ttc欧式": "欧式Ttc",
                    # 考虑用户输入 Latoffset 为百分数的情况 (法规场景)
                    r"\+75%": "-1.16",
                    "-75%": "1.16",
                    r"\+50%": "-0.58",
                    "-50%": "0.58",
                    r"100%|\+100%|-100%": "0",
                    r"50%": "0",
                    r"\+25%|25%": "-0.62",
                    "75%": "-0.62",
                },
                regex=True,
            )
            .to_numpy()
        )

        # 处理历史兼容问题, 描述做过修改
        self.scenes[settings.sys.generalized.cols.l2] = (
            self.scenes[settings.sys.generalized.cols.l2].str.replace(",状态", ",周期").str.replace("[位置", "[状态")
        )

        # step5: 替换 unitcharacter and mapping_info
        self.scenes.replace(
            to_replace={
                **dict.fromkeys(self.support_cols, {**settings.sys.unitcharacter.transfer, **_Define.mapping_info()}),
            },
            regex=True,
            inplace=True,
        )
        # self.scenes[self.support_cols].to_csv("support_cols_before.csv", encoding="utf_8_sig")

        # step6: 字符串转换为字典
        # self.scenes[self.support_cols] = np.frompyfunc(safe_literal_eval, 2, 1)(
        #     np.column_stack([np.array(self.scenes[settings.sys.testcase.cols.concreteid])] * len(self.support_cols)),
        #     self.scenes[self.support_cols].to_numpy(),
        # )
        for col in self.support_cols:
            logger.opt(lazy=True).debug(f"{col} column to safe_literal_eval")
            self.scenes[col] = np.frompyfunc(safe_literal_eval, 2, 1)(
                self.scenes[settings.sys.testcase.cols.concreteid].to_numpy(),
                self.scenes[col].to_numpy(),
            )
        # self.scenes[self.support_cols].to_csv("support_cols.csv", encoding="utf_8_sig")

        # step7: 去除任一一行包含 safe_literal_eval 返回为 pd.NA 的行
        self.scenes = self.scene_filter.process(
            df=self.scenes,
            keep_cond=self.scenes[self.support_cols].notnull().all(axis=1),
            reason="错误描述定义",
            assessment="设计规则",
            exp_class=exp.AfterFilterEmptyError,
        )

    def _dict_split_concat(self, cols: list, keys: Optional[list] = None) -> pd.DataFrame:
        return pd.concat(  # type: ignore
            [
                pd.DataFrame.from_records(values, index=self.scenes[col].index)
                for col, values in zip(cols, [self.scenes[col].to_numpy() for col in cols])
            ],
            axis=1,
            keys=keys,
        )

    def handle_muiltcolumn(self) -> None:
        logger.opt(lazy=True).info("handle_muiltcolumn")
        # 创建临时变量, 方便后续操作
        cols_tmp = [
            settings.sys.generalized.cols.funcid,
            settings.sys.generalized.cols.logicid,
            settings.sys.generalized.cols.label,
            settings.sys.generalized.cols.description,
            settings.sys.generalized.cols.naming,
            settings.sys.generalized.cols.mapfile,
            settings.sys.l0.cols.concreteid,
        ]
        # 考虑是否有 Classify 列
        if settings.sys.l0.cols.classify in self.scenes.columns:
            cols_tmp.append(settings.sys.l0.cols.classify)

        df_l0 = self.scenes[cols_tmp]

        # 获取一级参数
        # self.scenes = self._dict_split_concat(cols=self.support_cols, keys=self.support_cols)  # 3 level
        self.scenes = self._dict_split_concat(cols=self.support_cols)  # 2 level

        # 处理缺省值数据 nan
        self.scenes.where(cond=self.scenes.notna(), other=lambda x: [{}], inplace=True)

        # 获得二级参数
        self.scenes = self._dict_split_concat(cols=self.scenes.columns.tolist(), keys=self.scenes.columns.tolist())

        # 多level列名合并为单level
        self.scenes.columns = self.scenes.columns.map(".".join)

        # 添加 Config 相关信息
        self.scenes[cols_tmp] = df_l0.to_numpy()
        # self.scenes.to_csv("handle_muiltcolumn.csv", encoding="utf_8_sig")

    def handle_translate(self) -> None:
        logger.opt(lazy=True).info("handle_translate")
        # t0 = time.time()
        # 列名 header - 转换 (中译英)
        pat_header = re.compile(rf"{'|'.join(_Define.translate_header().keys())}\d+?")
        self.scenes.rename(
            lambda s: pat_header.sub(lambda x: _Define.translate_header()[x.group()], s),
            axis=1,
            inplace=True,
        )

        # # 保留翻译前的数据 Just For debug
        # cols = settings.sys.generalized.colset.hadmapids
        # cols.remove("Lane.Num2")
        # cols.remove("Lane.Num2Dist")
        # self.scenes["HadmapIdOrigin"] = (
        #     self.scenes[cols].fillna("none").astype(str).apply(f"{settings.sys.link_symbol}".join, axis=1)
        # ).str.replace(rf"{'|'.join(settings.sys.unitunified.transfer.keys())}", "", regex=True)
        # # self.scenes["HadmapIdOrigin"].drop_duplicates().to_csv(
        # #     f"{settings.sys.sheet.define}_HadmapOrigin.csv", encoding="utf_8_sig", index=False
        # # )
        # self.scenes["FuncLogicId"] = self.scenes["FuncId"].str.cat(self.scenes["LogicId"], sep=settings.sys.link_symbol)
        # self.scenes[["FuncLogicId", "HadmapIdOrigin"]].to_csv(
        #     f"{settings.sys.sheet.define}_HadmapOriginName.csv", encoding="utf_8_sig", index=False
        # )

        # 转换 (中译英)
        self.scenes.replace(
            to_replace={
                **_Define.translate_l0(),
                **_Define.translate_l1(),
                **_Define.translate_l2(),
                **_Define.translate_l4(self.scenes.columns.tolist()),
                **_Define.translate_l5(),
            },
            regex=True,
            inplace=True,
        )
        # self.scenes.to_csv("handle_translate.csv", encoding="utf_8_sig")

    def handle_add_info(self) -> None:
        logger.opt(lazy=True).info("handle_add_info")

        # 计算每一行 Ego 的数量, 并创建一个新的列
        ego_cols = [x for x in self.scenes.columns if "Ego" in x and "Model" in x]
        ego_count = (
            self.scenes[ego_cols].notna().sum(axis=1).astype("uint8[pyarrow]").rename(settings.sys.l0.cols.allnum_ego)
        )

        # 计算每一行 Npc 的数量, 并创建一个新的列
        npc_cols = [x for x in self.scenes.columns if "Npc" in x and "Model" in x]
        npc_count = (
            self.scenes[npc_cols].notna().sum(axis=1).astype("uint8[pyarrow]").rename(settings.sys.l0.cols.allnum_npc)
        )

        # 计算每一行 Dyn 的数量
        dyn_counts = []
        action_counts = []
        for hname in ["Ego", *[f"Npc{x}" for x in range(1, npc_count.max() + 1)]]:
            # 判断是否存在任意一个 Npc
            if self.scenes[f"{hname}.Phy.Model"].isna().all():
                continue

            # 计算每一个 Npc 的 Dyn 数量
            dyn_cols = [x for x in self.scenes.columns if f"{hname}.Dyn" in x and "Trigger" in x]
            dyn_count = (
                self.scenes[dyn_cols]
                .notna()
                .sum(axis=1)
                .astype("uint8[pyarrow]")
                .rename(f"{hname}.{settings.sys.l0.cols.allnum_dyn}")
            )
            dyn_counts.append(dyn_count)

            # 计算每一个 Dyn 下 Action 的数量
            for j in range(1, dyn_count.max() + 1):
                # 计算每一个 Dyn 下 Action 的数量
                action_cols = [x for x in self.scenes.columns if f"{hname}.Dyn{j}.Action" in x]
                action_counts.append(
                    self.scenes[action_cols]
                    .notna()
                    .sum(axis=1)
                    .astype("uint8[pyarrow]")
                    .rename(f"{hname}.Dyn{j}.{settings.sys.l0.cols.allnum_action}")
                )

        # 通过 pd.concat 一次性添加新列
        self.scenes = pd.concat([self.scenes, ego_count, npc_count, *dyn_counts, *action_counts], axis=1)

    def handle_lanenum(self) -> None:
        logger.opt(lazy=True).info("handle_lanenum")

        comp = _Define.pattern_lanenum()
        new_cols = ["Lane.Num", "Lane.Num2", "Lane.Num2Dist"]

        # 使用 str.extract 提取新列，并命名为 new_cols
        extracted_df = self.scenes["Lane.Num"].str.extract(pat=comp).fillna("0")
        extracted_df.columns = new_cols

        # 删除原来的 "Lane.Num" 列
        self.scenes = self.scenes.drop(columns=["Lane.Num"])

        # 一次性将新列添加到原 DataFrame 中，避免多次插入导致的碎片化
        self.scenes = pd.concat([self.scenes, extracted_df], axis=1)

    def handle_trigger(self):
        logger.opt(lazy=True).info("handle_trigger")

        # find columns
        cols = self.scenes.filter(regex="\.Trigger").columns

        comp = re.compile(_Define.pattern_trigger(), flags=re.X)

        # 基于性能考虑, 创建临时list
        temp_list = []

        # 循环操作每一个相关列
        for col in cols:
            new_cols = [f"{col}.Ref", f"{col}.Disttype", f"{col}.Type", f"{col}.Rule", f"{col}.Value"]
            temp_df = self.scenes[col].str.extract(pat=comp)
            temp_df.columns = new_cols

            temp_list.append(temp_df)

        # 如果 temp_list 为空, 直接返回
        if not temp_list:
            return

        # 删除无用列
        self.scenes.drop(columns=cols, inplace=True)

        # 基于性能考虑, 创建新的 DataFrame
        new_scenes = pd.concat(temp_list, axis=1)

        # 连接原始的DataFrame和新的DataFrame
        self.scenes = pd.concat([self.scenes, new_scenes], axis=1)

    def handle_action(self):
        logger.opt(lazy=True).info("handle_action")

        # find columns
        cols = self.scenes.filter(regex="\.Action").columns

        comp = re.compile(_Define.pattern_action(), flags=re.X)

        # 基于性能考虑, 创建临时list
        temp_list = []

        # 循环操作每一个相关列
        for col in cols:
            # 找到没有包含 @ 的行, 结尾补充 @
            cond = self.scenes[col].str.contains("@", na=False) | self.scenes[col].isna()
            self.scenes.loc[~cond, col] = self.scenes.loc[~cond, col] + "@none=0"
            # 拆分为多列
            new_cols = [f"{col}.Type", f"{col}.Value", f"{col}.Endtype", f"{col}.Endvalue"]
            temp_df = self.scenes[col].str.extract(pat=comp)
            temp_df.columns = new_cols

            temp_list.append(temp_df)

        # 如果 temp_list 为空, 直接返回
        if not temp_list:
            return

        # 删除无用列
        self.scenes.drop(columns=cols, inplace=True)

        # 基于性能考虑, 创建新的 DataFrame
        new_scenes = pd.concat(temp_list, axis=1)

        # 连接原始的DataFrame和新的DataFrame
        self.scenes = pd.concat([self.scenes, new_scenes], axis=1)

    def _get_par_cols(self) -> list:
        return [x for x in self.scenes.columns if "." in x and settings.sys.suffix.unit not in x and "Allnum" not in x]

    def handle_add_info_parameter(self) -> None:
        logger.opt(lazy=True).info("handle_add_info_parameter")

        # find columns
        par_cols = self._get_par_cols()

        # 创建一个新的 DataFrame，用于存储参数信息
        parameter_series = self.scenes[par_cols].apply(
            lambda row: {col: row[col] for col in par_cols if pd.notna(row[col])}, axis=1
        )

        # 如果 parameter_series 为空, 直接返回
        if parameter_series.empty:
            return

        # 将新的 DataFrame 与原始 DataFrame 连接起来
        self.scenes = pd.concat([self.scenes, parameter_series.rename(settings.sys.l0.cols.parameter)], axis=1)

    def handle_unit(self) -> None:
        logger.opt(lazy=True).info("handle_unit")
        # find columns
        cols = [s for s in self.scenes.columns if any(sub in s for sub in settings.sys.generalized.colset.units)]

        # 遍历并转换创建为多列 [value, unit]
        comp = _Define.pattern_unit()

        # 获得所有 格式化单位
        unitunified_keys = "|".join(settings.sys.unitunified.transfer.keys())

        # 基于性能考虑, 创建临时list
        temp_list = []

        # 循环操作每一个 unit 相关列
        for col in cols:
            if not self.scenes[col].str.contains(pat=unitunified_keys, na=False, regex=True).any():
                # logger.opt(lazy=True).info(f"{col} No unit entered")
                continue

            unit = f"{col}{settings.sys.suffix.unit}"
            temp_df = self.scenes[col].str.extract(pat=comp)
            temp_df.columns = [col, unit]

            # unit 列由 str 转换为 facor 数值, 如未定义单位默认转换为 1
            temp_df[unit] = temp_df[unit].map(settings.sys.unitunified.transfer, na_action="ignore").fillna(1)

            temp_list.append(temp_df)

            # 删除原始列, 因为 new_scenes 中会存在相同列名的列
            self.scenes = self.scenes.drop(columns=col)

        # 如果 temp_list 为空, 直接返回
        if not temp_list:
            return

        # 基于性能考虑, 创建新的 DataFrame
        new_scenes = pd.concat(temp_list, axis=1)

        # 连接原始的DataFrame和新的DataFrame
        self.scenes = pd.concat([self.scenes, new_scenes], axis=1)

        # 填充空值
        self.scenes[cols] = self.scenes[cols].fillna("0").replace({"": "0"})

    def handle_all_parameter(self) -> None:
        logger.opt(lazy=True).info("handle_all_parameter")
        #
        par_cols = self._get_par_cols()
        comp_one2one = re.compile(_Define.pattern_one2one())

        # 基于性能考虑, 创建新的 DataFrame
        new_scenes = pd.DataFrame()
        #
        for col in par_cols:
            # 找到带有 & 的列, 并拆分为2列,原有列将不含有 &, 新列如果包含将以 "&" 表示
            cond_one2one = self.scenes[col].str.contains(settings.sys.sign.one2one, regex=False, na=False)
            if cond_one2one.any():
                one2one = f"{col}{settings.sys.suffix.one2one}"
                new_scenes[[col, one2one]] = self.scenes[col].str.extract(pat=comp_one2one)

                self.scenes = self.scenes.drop(columns=col)

        # 连接原始的DataFrame和新的DataFrame
        self.scenes = pd.concat([self.scenes, new_scenes], axis=1)

        for col in par_cols:
            # 找到带有 /, 处理为 [x, ..., x]
            cond_enum = self.scenes[col].str.contains(settings.sys.sign.enum, regex=False, na=False)
            if cond_enum.any():
                self.scenes.loc[cond_enum, col] = np.frompyfunc(
                    lambda values: [convert_string(v) for v in values],
                    1,
                    1,
                )(self.scenes.loc[cond_enum, col].str.split(settings.sys.sign.enum, regex=False).to_numpy())

            # 找到带有 *, 处理为 [x, ..., x]
            cond_range = self.scenes[col].str.contains(rf"\{settings.sys.sign.range}", regex=True, na=False)
            if cond_range.any():
                self.scenes.loc[cond_range, col] = np.frompyfunc(
                    # bugfix(230914): 解决由浮点数精度导致,结果数量不对的问题
                    lambda x: np.linspace(
                        start=float(x[0]), stop=float(x[2]), num=abs(int((float(x[2]) - float(x[0])) / float(x[1]))) + 1
                    ).tolist(),
                    1,
                    1,
                )(self.scenes.loc[cond_range, col].str.split(rf"\{settings.sys.sign.range}", regex=True).to_numpy())

            # 其余参数, 如果是数值字符串, 转换为数值
            self.scenes.loc[~(cond_enum | cond_range), col] = np.frompyfunc(
                convert_string,
                1,
                1,
            )(self.scenes.loc[~(cond_enum | cond_range), col].to_numpy())

    def handle_all_parameter_for_one2one(self) -> None:
        logger.opt(lazy=True).info("handle_all_parameter_for_one2one")

        suffix_one2one = settings.sys.suffix.one2one

        par_cols = self._get_par_cols()
        if cols_one2one := [x[: -len(suffix_one2one)] for x in par_cols if suffix_one2one in x]:  # NOCA:E999(设计如此)
            headers = {
                lvl: [x for x in cols_one2one if any(hd in x for hd in settings.sys[lvl].sub1.header.transfer.values())]
                for lvl in ["l1", "l2", "l4", "l5"]
            }

            for col_name in headers.values():
                # print(f"{col_name = }")
                if not col_name:
                    continue

                col_sign = [f"{x}{suffix_one2one}" for x in col_name]
                row_name = self.scenes.index[self.scenes[col_sign].isin([settings.sys.sign.one2one]).any(axis=1)]

                # step2: 非list cell变为list
                self.scenes.loc[row_name, col_name] = np.frompyfunc(lambda x: x if isinstance(x, list) else [x], 1, 1)(
                    self.scenes.loc[row_name, col_name].to_numpy()
                )

                # step3: 计算每行元素list的目标长度,补充元素的list长度,更新数值
                tmp = self.scenes.loc[row_name, col_name]
                cond1 = np.frompyfunc(lambda x: len(x) <= 1, 1, 1)(tmp.to_numpy())
                cond2 = self.scenes.loc[row_name, col_sign].isin([settings.sys.sign.one2one]).to_numpy()
                cond3 = np.array(~cond1 & ~cond2, dtype=bool)

                # bugfix: (xingboliu) 20230509 最大长度仅计算 one2one 的列
                max_len = np.frompyfunc(len, 1, 1)(tmp.where(cond2, "")).max(axis=1).astype("uint8[pyarrow]")

                to_list_func = np.frompyfunc(lambda x: [x], 1, 1)
                self.scenes.loc[row_name, col_name] = np.where(
                    cond1,
                    to_list_func((tmp.T * max_len).T.to_numpy()),
                    # bugfix: (xingboliu) 20230507  找到非 one2one 的列, 且不为空, 且不为数字, 且不为一维数组
                    np.where(
                        cond2,
                        to_list_func(tmp.to_numpy()),
                        np.frompyfunc(lambda x: [[i] * x.count(i) for i in set(x)] if x else x, 1, 1)(
                            (tmp.where(cond3, "").T * max_len).T.to_numpy()
                        ),
                    ),
                )

    def process(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' _LogicFormater Processing: ':=^55}")

        self.data_preprocessing()
        self.handle_muiltcolumn()
        self.handle_translate()
        self.handle_add_info()
        self.handle_lanenum()
        self.handle_trigger()
        self.handle_action()

        self.handle_add_info_parameter()

        self.handle_unit()

        self.handle_all_parameter()
        self.handle_all_parameter_for_one2one()

        # self.scenes = self.scenes.convert_dtypes(dtype_backend="pyarrow").sort_index(axis=1).reset_index(drop=True)
        return DtypeConverter(self.scenes).process().reset_index(drop=True)


@dataclass(order=True)
class _FilterTaskGeneralized:
    """过滤 <语义生成> 任务所需数据"""

    define: pd.DataFrame
    scene_filter: SceneFilter
    classify_user_dict: dict
    cols_classify_req: list

    def __post_init__(self) -> None:
        #
        self.cols_classify, self.cols_nonreq = self._getcols_classify_replace_nonreq()
        #
        logger.opt(lazy=True).debug(f"{self.cols_classify = }")
        logger.opt(lazy=True).debug(f"{self.cols_classify_req = }")
        logger.opt(lazy=True).debug(f"{self.cols_nonreq = }")

    def _getcols_classify_replace_nonreq(self) -> tuple:
        # 计算交集从而获得 cols_classify, 并根据 dict_keys 的顺序进行排序
        dict_keys = list(self.classify_user_dict.keys())
        cols_classify = [x for x in dict_keys if x in self.define.columns]
        cols_classify.sort(key=dict_keys.index)

        #
        cols_nonreq = [x for x in cols_classify if x not in self.cols_classify_req]

        return cols_classify, cols_nonreq

    def _classify_eng(self) -> pd.Series:
        """将所有 classify 列翻译成英文, 并用 link_symbol 连接需要使用的列"""

        # 翻译元素, 之后逐行拼接多列内容, 且如果元素为空字符串时, 拼接时忽略
        return (
            self.define[self.cols_classify]
            .replace(to_replace=self.classify_user_dict, regex=True)
            .fillna("")  # 使用空字符串填充空值
            .agg(
                lambda x: (
                    f"{settings.sys.classify.prefix}"
                    f"{settings.sys.link_symbol}"
                    f"{settings.sys.link_symbol.join(a for a in x if a)}"
                ),
                axis=1,
            )
        )

    def _classify_chn(self, df: pd.DataFrame) -> np.ndarray:
        # 将 DataFrame 转换为 NumPy 数组
        arr = df.to_numpy()

        # 更正列名匹配部分
        conditions = [
            (df[list(self.classify_user_dict.keys())[0]] == k, v) for k, v in settings.sys.classify.conditions.items()
        ]

        # 初始化结果数组
        concatenated_arr = np.empty(arr.shape[0], dtype=np.dtype("O"))

        # 使用循环拼接多个列
        for condition, indices in conditions:
            temp_arr = arr[condition][:, indices[0] - 1].astype(str)

            for index in indices[1:]:
                try:
                    temp_arr = np.core.defchararray.add(
                        temp_arr,
                        np.core.defchararray.add(settings.sys.link_symbol, arr[condition][:, index - 1].astype(str)),
                    )
                except IndexError:
                    logger.opt(lazy=True).warning(
                        f"Caught IndexError: index {index} is out of bounds for arr with size {arr.size}"
                    )
                    continue

            concatenated_arr[condition] = temp_arr

        return concatenated_arr

    def process_classify(self, df: pd.DataFrame) -> pd.DataFrame:
        # 默认该列设置为空字符串
        df[settings.sys.l0.cols.classify] = ""

        #
        if self.cols_classify_req and set(self.cols_classify_req).issubset(df.columns):
            logger.opt(lazy=True).info("add _classify_eng and _classify_chn")

            # self.cols_classify_req 列, 未填写的补充为 default_value
            df[self.cols_classify_req] = df[self.cols_classify_req].fillna(settings.sys.classify.default_value)

            # 对于非必须的, 如果列存在, NAN 元素补充为 "", 如果列不存在, 创建列并补充为 ""
            df = df.assign(**{col: "" for col in self.cols_nonreq if col not in df.columns}).fillna(
                {col: "" for col in self.cols_nonreq}
            )

            # 在当前 ConcreteId 前增加 _classify_eng 前缀
            df[settings.sys.l0.cols.concreteid] = self._classify_eng().str.cat(
                df[settings.sys.l0.cols.concreteid], sep=settings.sys.link_symbol
            )

            # 创建 Classify 列
            df[settings.sys.l0.cols.classify] = self._classify_chn(df[self.cols_classify])

        return df

    def process(self) -> pd.DataFrame:
        # sourcery skip: extract-method
        logger.opt(lazy=True).info(f"{' Formatting generalized: ':=^55}")

        # 异常处理, 检查是否存在语义生成需要的列
        cols_generalized_req = settings.sys.generalized.colset.reqs
        raise_error_reason_by_diff(cols_generalized_req, list(self.define.columns))

        # 筛选出需要的列
        self.define = self.define[[*settings.sys.generalized.cols.values(), *self.cols_classify]]
        logger.opt(lazy=True).debug(f"{self.define.columns.tolist() = }")

        # 如果为非必须列 且 不存在于self.define中, 补充为空字符串
        cols_generalized_nonreq = set(settings.sys.generalized.cols.values()).difference(set(cols_generalized_req))
        if cols_generalized_nonreq_miss := list(cols_generalized_nonreq.difference(set(self.define.columns))):
            self.define[cols_generalized_nonreq_miss] = ""

        # 创建 ConcreteId, 考虑 funcid 和 logicid 是否存在的情况
        self.define.loc[:, settings.sys.l0.cols.concreteid] = self.define.apply(
            lambda x: (
                f"{x[settings.sys.generalized.cols.funcid]}"
                f"{settings.sys.link_symbol}"
                f"{x[settings.sys.generalized.cols.logicid]}"
            )
            if x[settings.sys.generalized.cols.funcid] and x[settings.sys.generalized.cols.logicid]
            else x[settings.sys.generalized.cols.logicid] or x[settings.sys.generalized.cols.funcid],
            axis=1,
        )

        # 只处理 valid_scene 的行, 否则剔除
        self.define = self.scene_filter.process(
            df=self.define,
            keep_cond=self.define[settings.sys.generalized.cols.mask] == settings.sys.generalized.mask_value,
            reason=f"只处理完成场景设计状态的行 (Mask 列内容为 {settings.sys.generalized.mask_value})",
            assessment="设计规则",
            exp_class=exp.AfterFilterEmptyError,
        )

        # 存在 分类分级列的内容时合并处理, 不存在是为空
        self.define = self.process_classify(self.define)

        # 去除 ConcreteId 重复的行, 仅保留第一出现的
        self.define = self.scene_filter.process(
            df=self.define,
            keep_cond=~self.define[settings.sys.l0.cols.concreteid].duplicated(keep="first"),
            reason="只处理具体场景名称不重复的行, 仅保留第一次出现",
            assessment="设计规则",
            exp_class=exp.AfterFilterEmptyError,
        )

        # 异常处理, 检查场景数据是否为空
        if self.define[settings.sys.generalized.cols.l4].replace("", np.nan).isna().all():
            raise exp.GeneralizedEmptyError("task_generalized is empty")

        # just for large case test
        # scenes = pd.DataFrame(np.repeat(scenes.to_numpy(), 30, axis=0), columns=scenes.columns)
        # scenes["LogicId"] = scenes["LogicId"].str.cat(scenes.groupby("LogicId").cumcount().add(1).astype(str))

        # 选择配置和泛化需要的列
        return (
            self.define[
                [
                    settings.sys.l0.cols.concreteid,
                    settings.sys.l0.cols.classify,
                    *settings.sys.generalized.cols.values(),
                ]
            ]
            .reset_index(drop=True)
            .convert_dtypes(dtype_backend="pyarrow")
        )


@dataclass(order=True)
class _FilterTaskTestcase:
    """过滤 <测试用例> 任务所需数据"""

    define: pd.DataFrame
    scene_filter: SceneFilter

    def process(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' Formatting testcase: ':=^55}")

        # 异常处理, 检查是否存在测试用例需要的列
        raise_error_reason_by_diff(settings.sys.testcase.cols.values(), list(self.define.columns))

        testcase = self.define[settings.sys.testcase.cols.values()]
        testcase[settings.sys.l0.cols.is_generalized] = self.define[settings.sys.generalized.cols.l4] != ""

        # 异常处理, 检查场景数据是否为空
        if testcase.empty:
            raise exp.TestcaseEmptyError("task_testcase is empty")

        cols_base = [
            x
            for x in testcase.columns
            if x
            not in [
                settings.sys.testcase.cols.kpis,
                settings.sys.testcase.cols.concreteid,
                settings.sys.testcase.cols.logicid,
                settings.sys.testcase.cols.funcid,
                settings.sys.l0.cols.is_generalized,
            ]
        ]

        return testcase.groupby(cols_base, as_index=False).apply(
            lambda x: pd.Series(
                {
                    settings.sys.testcase.cols.kpis: list(set(",".join(x[settings.sys.testcase.cols.kpis]).split(","))),
                    settings.sys.testcase.cols.concreteid: list(
                        set(",".join(x[settings.sys.testcase.cols.concreteid]).split(","))
                    ),
                    settings.sys.testcase.cols.logicid: list(x[settings.sys.testcase.cols.logicid]),
                    settings.sys.testcase.cols.funcid: list(x[settings.sys.testcase.cols.funcid]),
                    settings.sys.l0.cols.is_generalized: list(x[settings.sys.l0.cols.is_generalized]),
                }
            )
        )


@dataclass(order=True)
class Formater:
    define: pd.DataFrame
    param: pd.DataFrame
    settings_user: pd.DataFrame
    classify_user: pd.DataFrame
    scene_filter: SceneFilter

    def __post_init__(self) -> None:
        # 去除中文及重复标点问题
        self.define.rename(columns=_Define.translate_character(), inplace=True)
        self.define = self.define.replace(to_replace=_Define.translate_character(), regex=True)
        self.settings_user.rename(columns=_Define.translate_character(), inplace=True)
        self.classify_user.rename(columns=_Define.translate_character(), inplace=True)
        if not self.param.empty:
            self.param.rename(columns=_Define.translate_character(), inplace=True)
            self.param = self.param.replace(to_replace=_Define.translate_character(), regex=True)

        # 格式化 场景定义
        self.define = self._format_define(self.define)

        # 格式化 地图参数
        self.param_dict = self._format_param(self.param)

        # 格式化 用户自定义配置
        self.settings_user_dict = self._format_settings_user(self.settings_user)

    def _df_to_nested_dict(self, df: pd.DataFrame) -> dict:
        result: Dict = {}
        for row in df.itertuples(index=False):
            value = convert_string(row[-1])
            keys = list(filter(lambda x: isinstance(x, str) and x != "", row[:-1]))
            keys = [k.split(".") for k in keys]
            keys = [item for sublist in keys for item in sublist]

            current_level = result
            for k in keys[:-1]:
                if k not in current_level:
                    current_level[k] = {}
                current_level = current_level[k]
            current_level[keys[-1]] = value
        return result

    def _extract_number_or_infinity(self, col_name: str) -> Union[int, float]:
        match = re.search(r"\d+", col_name)
        return int(match.group()) if match else float("inf")

    def _deal_param_name(self, input_str: str, translates: list) -> str:
        # 拆分字符串
        parts = input_str.split(settings.sys.link_symbol)

        # 处理用户设置地图库地图参数时, 没有 "Lane.Num2" 和 "Lane.Num2Dist" 的情况时,
        # translates 字典移除 "Lane.Num2" 和 "Lane.Num2Dist"
        # len(translates) = 15
        # len(parts) = 可能是 12 或者是 13, 因为 route 有的不存在
        if len(translates) - len(parts) in [2, 3]:
            # 在指定位置插入元素, 补充 "Lane.Num2" 和 "Lane.Num2Dist" 的默认值 "0"
            parts.insert(list(_Define.translate_l1().keys()).index("Lane.Num2"), "0")
            parts.insert(list(_Define.translate_l1().keys()).index("Lane.Num2Dist"), "0")

        # 不存在 route 的补充默认值
        if len(translates) - len(parts) in [1]:
            parts.append(settings.sys.l4.route.transfer.values()[0])

        # 异常处理, 参数数量不对
        if len(translates) - len(parts) not in [0]:
            logger.opt(lazy=True).info(f"The length of the input parameter is wrong: {input_str}")
            return ""

        # 使用多个字典进行转换
        parts = [
            reduce(lambda x, d: re.sub(d[0], d[1], x), translates[i].items(), part) for i, part in enumerate(parts)
        ]

        return settings.sys.link_symbol.join(parts)

    def _format_define(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        格式化场景定义
        """
        # 重命名列名以除标记 settings.sys.classify.req_mask (默认为 *)
        df.columns = df.columns.str.replace(settings.sys.classify.req_mask, "")

        # 异常处理-重要列, 检查是否存在重要列, testcase 和 generalized 同时都需要的列 ["FuncId", "LogicId"]
        cols_important = list(set(settings.sys.testcase.cols.values()) & set(settings.sys.generalized.cols.values()))
        raise_error_reason_by_diff(cols_important, list(df.columns))

        # 异常处理-重复值, 创建中间变量, 标记合并单元格, 标记重复的值, 只保留第一次出现以及和它相同的 merged_cel 行
        task_name = settings.sys.testcase.cols.name
        if task_name in df.columns:
            df["merged_cel"] = (~df[task_name].isna()).cumsum()
            df["is_duplicated"] = df[task_name].duplicated() & df[task_name].notna()
            df = self.scene_filter.process(
                df=df,
                keep_cond=~df["merged_cel"].isin(df.loc[df["is_duplicated"], "merged_cel"]),
                reason="Filtering by testcase task name duplicated",
                assessment="设计规则",
                exp_class=exp.TestcaseEmptyError,
            )

        # 通过列名列表, 创建一个用于填充空值的字典, 指定列 NaN 替换为空字符串, 其余列 NaN 替换为上一行的值
        cols_to_fill = [
            settings.sys.testcase.cols.task_description,
            settings.sys.testcase.cols.task_label,
            settings.sys.testcase.cols.sets,
            settings.sys.testcase.cols.kpis,
            settings.sys.testcase.cols.concreteid,
            *settings.sys.generalized.cols.values(),
        ]
        fill_values = dict.fromkeys(cols_to_fill, "")
        df.fillna(value=fill_values, inplace=True)
        df.ffill(inplace=True)

        # 异常处理-重要列值, 只处理存在仿真类型的行, 否则剔除
        if settings.sys.testcase.cols.simtype in df.columns:
            df = self.scene_filter.process(
                df=df,
                keep_cond=df[settings.sys.testcase.cols.simtype].str.title().isin(settings.sys.testcase.simtype_values),
                reason="Filtering by testcase simulationType",
                assessment="设计规则",
                exp_class=exp.TestcaseEmptyError,
            )

        # 异常处理-列 Label 和 列 Naming 需要存在
        if settings.sys.generalized.cols.label not in df.columns:
            df[settings.sys.generalized.cols.label] = ""
        if settings.sys.generalized.cols.naming not in df.columns:
            df[settings.sys.generalized.cols.naming] = ""
        if settings.sys.generalized.cols.mapfile not in df.columns:
            df[settings.sys.generalized.cols.mapfile] = ""

        return df

    def _format_param(self, df: pd.DataFrame) -> dict:
        logger.opt(lazy=True).info(f"{' Formatting _format_param: ':=^55}")
        if df.empty:
            return {}

        # 统一列名为 "Name" 和 "Routes"
        for k, v in settings.sys.map.real.param.transfer.items():
            df.columns = df.columns.str.replace(k, v, regex=True)

        # 处理未定义 mapfile 列, 补充为空字符串
        if settings.sys.map.real.param.mapfile not in df.columns:
            df[settings.sys.map.real.param.mapfile] = ""

        # 任一元素为空, 整行删除
        df = df.dropna()

        # 格式化 param["Name"] 列
        df.loc[:, settings.sys.map.real.param.name] = df[settings.sys.map.real.param.name].apply(
            lambda x: self._deal_param_name(x, _Define.translate_hadmapid())
        )

        # 格式化 param["Routes"] 列
        df[settings.sys.map.real.param.routes] = (
            df[settings.sys.map.real.param.routes]
            .astype(str)
            .str.split(";")
            .map(
                lambda string_list: list(
                    map(
                        lambda x: Position(world=PositionWorld(x=float(x[0]), y=float(x[1]), z=float(x[2])))
                        if len(x) == 3
                        else Position(
                            lane=PositionLane(road_id=int(x[0]), lane_id=int(x[1]), offset=float(x[2]), s=float(x[3])),
                        ),
                        list(map(lambda x: x.split(","), string_list)),
                    )
                )
            )
        )

        # 将 DataFrame 转换为嵌套字典
        return (
            df.groupby(settings.sys.map.real.param.mapfile)
            .apply(lambda x: dict(zip(x[settings.sys.map.real.param.name], x[settings.sys.map.real.param.routes])))
            .to_dict()
        )

    def _format_settings_user(self, df: pd.DataFrame) -> dict:
        # 处理 excel 中不存在 参数设置 sheet 的情况
        if df.empty:
            return {}

        # 统一列名
        for k, v in settings.sys.l0.settings_user.transfer.items():
            df.columns = df.columns.str.replace(k, v, regex=True)

        # 需要使用的列, 合并单元格填充会填充为上一行的值, 然后将 DataFrame 转换为嵌套字典, 进行返回
        need_cols = [
            settings.sys.l0.settings_user.col1,
            settings.sys.l0.settings_user.col2,
            settings.sys.l0.settings_user.value,
        ]

        return self._df_to_nested_dict(df[need_cols].ffill())

    def _getcols_classify_chn_eng_req(self, cols: list) -> tuple:
        #
        col_suffix = settings.sys.classify.col_suffix
        req_mask = settings.sys.classify.req_mask

        # 获取中文列(基于中文列和英文列同步出现的前提下), 并按阿拉伯数字部分进行排序
        cols_classify_chn = [
            col
            for col in cols
            if not col.endswith(col_suffix)
            and (f"{col}{col_suffix}" in cols or f"{col.replace(req_mask, '')}{col_suffix}" in cols)
        ]
        cols_classify_chn = sorted(cols_classify_chn, key=self._extract_number_or_infinity)

        # 获取必须列
        cols_classify_req = [col.replace(req_mask, "") for col in cols_classify_chn if req_mask in col]

        # 去除 req_mask 标记
        cols_classify_chn = [col.replace(req_mask, "") for col in cols_classify_chn]

        # 获取中文列
        cols_classify_eng = [f"{col}{col_suffix}" for col in cols_classify_chn]

        # 处理全部没有 req_mask 时, 则全部都是必须列
        if not cols_classify_req:
            cols_classify_req = cols_classify_chn

        return cols_classify_chn, cols_classify_eng, cols_classify_req

    def _format_classify_user(self, df: pd.DataFrame, cols_classify_chn: list, cols_classify_eng: list) -> dict:
        output_dict: Dict = {}

        # 处理 excel 中不存在设置的情况
        if df.empty:
            return output_dict

        # 重命名列名以除标记 settings.sys.classify.req_mask (默认为 *)
        df.columns = df.columns.str.replace(settings.sys.classify.req_mask, "")

        for col1, col2 in zip(cols_classify_chn, cols_classify_eng):
            # 挑选出两组数据皆非 NA 的情况
            non_na_mask = df[col1].notna() & df[col2].notna()
            output_dict[col1] = pd.Series(df[col2][non_na_mask].values, index=df[col1][non_na_mask].values).to_dict()

        return output_dict

    def filter_task_generalized(self):
        # 获得 中文, 英文, 必须列 的列表
        cols_classify_chn, cols_classify_eng, cols_classify_req = self._getcols_classify_chn_eng_req(
            list(self.classify_user.columns)
        )

        # 格式化 分类属性
        classify_user_dict = self._format_classify_user(self.classify_user, cols_classify_chn, cols_classify_eng)

        #
        ftg = _FilterTaskGeneralized(self.define, self.scene_filter, classify_user_dict, cols_classify_req)
        return ftg.process()

    def filter_task_testcase(self):
        ftt = _FilterTaskTestcase(self.define, self.scene_filter)
        return ftt.process()

    def logic(self, df: pd.DataFrame) -> pd.DataFrame:
        lfer = _LogicFormater(df, self.scene_filter)
        return lfer.process()


# sourcery skip: remove-empty-nested-block, remove-redundant-if
if __name__ == "__main__":
    # pyinstrument formater.py
    pass
