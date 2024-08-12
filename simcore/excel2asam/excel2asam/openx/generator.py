#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file generator.py
# * \brief generate scene file in OpenSCENARIO (.xosc) format.
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2023-03-31
# * \version 0.0.1
# * \copyright (c) 2023 Tencent Co. Ltd. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass
from multiprocessing.dummy import Pool as ThreadPool
from pathlib import Path
from typing import Any, List

import pandas as pd
from loguru import logger

from excel2asam.config import settings
from excel2asam.openx.wrapper_xodr import GenXodr
from excel2asam.openx.wrapper_xosc import GenXosc


@dataclass(order=True)
class Generator:
    pathdir_output_xosc: Path
    pathdir_output_xodr: Path
    pathdir_catalog: Path

    def __post_init__(self):
        # init xosc
        self.gxosc = GenXosc(
            gen_folder=self.pathdir_output_xosc,
            oscminor=settings.get("sys.version.oscminor", 0),
            pathdir_catalog=self.pathdir_catalog,
        )
        # init xodr
        self.gxodr = GenXodr(
            gen_folder=self.pathdir_output_xodr,
            odrmajor=settings.get("sys.version.odrmajor", 1),
            odrminor=settings.get("sys.version.odrminor", 5),
        )

    def _multi_threaded_generation(self, arg0: Any, arg1: List[dict]) -> None:
        pool = ThreadPool(settings.sys.thread_num)
        pool.map(arg0.generate, arg1)
        pool.close()
        pool.join()

    def to_xosc(self, concrete: pd.DataFrame) -> None:
        # do xosc gen
        concrete_xosc = concrete.apply(lambda x: x.dropna().to_dict(), axis=1).tolist()

        self._multi_threaded_generation(self.gxosc, concrete_xosc)
        # for scene in concrete_xosc:
        #     self.gxosc.generate(scene)

        logger.opt(lazy=True).info(f"Finished xosc file {len(concrete_xosc)}")

    def to_xodr(self, concrete: pd.DataFrame, task_generalized_id: int = 0) -> None:
        logger.opt(lazy=True).info(f"{' Generating virtual xodrs: ':-^35}")

        # # 因为数据平台上传地图时限制地图名称的长度,所以需要重命名地图名称,控制长度
        # # 将 settings.sys.l0.cols.mapfile 列重命名, 用前缀+数字替换原有名称
        # logger.opt(lazy=True).info("Reset MapFile for reduce mapname length")
        # concrete[settings.sys.l0.cols.mapfile] = concrete[settings.sys.l0.cols.mapfile].astype("category")
        # concrete[settings.sys.l0.cols.mapfile] = concrete[settings.sys.l0.cols.mapfile].cat.rename_categories(
        #     {
        #         category: f"VirtualMap_{task_generalized_id}_{num:03}"
        #         for num, category in enumerate(concrete[settings.sys.l0.cols.mapfile].cat.categories, start=1)
        #     }
        # )

        # do xodr gen
        concrete_map = (
            concrete.drop_duplicates([settings.sys.l0.cols.mapfile])
            .apply(lambda x: x.dropna().to_dict(), axis=1)
            .tolist()
        )

        # self._multi_threaded_generation(self.gxodr, concrete_map)
        for scene in concrete_map:
            self.gxodr.generate(scene)

        logger.opt(lazy=True).info(f"Finished xodr file {len(concrete_map)}")

        return None


# sourcery skip: remove-empty-nested-block, remove-redundant-if
if __name__ == "__main__":
    pass
