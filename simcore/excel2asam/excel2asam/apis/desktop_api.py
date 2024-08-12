#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file desktop_api.py
# * \brief api for desktop
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-08-01
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import json
from dataclasses import dataclass
from pathlib import Path

from loguru import logger
import requests

from excel2asam.config import settings


@dataclass(order=True)
class DesktopApi:

    pathdir_output_xosc: Path

    def _request_api(self, url: str, method: str, payload: str, headers: dict) -> dict:
        # 使用 requests 库
        response = requests.request(method, url, headers=headers, data=payload)
        # 获取响应数据
        data = response.text

        # 将响应数据解析为 JSON
        result = json.loads(data)
        logger.debug(f"_request_api: {result = }")

        if result.get("Error"):
            logger.opt(lazy=True).error(f"Get error: {result['Error']}")
        return result

    def send_semantic_statu(self, gen_status: int, all_count: int, non_count: int, finished_count: int) -> int:
        result = self._request_api(
            url=settings.sys.tadsim.url_semantic_statu,
            method="POST",
            payload=json.dumps(
                {
                    # 状态 0: 生成中 1: 生成完成 2: 生成失败 3: 已暂停 4: 准备完成
                    "taskGenStatus": gen_status,
                    # 场景生成数
                    "taskAllCount": all_count,
                    # 场景被剔除数
                    "taskNonCount": non_count,
                    # 场景完成数
                    "taskFinishedCount": finished_count,
                    # 场景生成的文件夹
                    "dir": str(self.pathdir_output_xosc),
                }
            ),
            headers={"Content-Type": "application/json"},
        )

        # 0: waiting 等待开始 1: ing 开始 2: finshed 退出,前端主动退出
        return int(result["code"])

    def send_scenarios_insert(
        self,
        scene_id: int,
        scene_name: str,
        map_name: str,
        scene_info: str = "",
        scene_type: str = "xosc",  # sim/xosc
        traffictype: str = "traffic"  # traffic/simrec
    ) -> int:
        #
        map_name = (
            map_name if settings.sys.map.virtual.suffix in map_name else f"{map_name}{settings.sys.map.virtual.suffix}"
        )

        result = self._request_api(
            url=settings.sys.tadsim.url_scenarios_insert,
            method="POST",
            payload=json.dumps(
                {
                    # 场景id
                    "id": scene_id,
                    # 场景名称（不带后缀）
                    "name": scene_name,
                    # 地图名称
                    "map": map_name,
                    # 完整路径
                    "path": str(self.pathdir_output_xosc / scene_name),
                    # 工况
                    "info": scene_info,
                    # 场景格式类型
                    "type": scene_type,
                    # 场景类型
                    "traffictype": traffictype,
                }
            ),
            headers={"Content-Type": "application/json"},
        )

        # 0: 成功 -1: 失败
        return int(result["code"])

    def send_maps_insert(self, map_path: str) -> int:
        #
        map_path = (
            map_path if settings.sys.map.virtual.suffix in map_path else f"{map_path}{settings.sys.map.virtual.suffix}"
        )

        result = self._request_api(
            url=settings.sys.tadsim.url_maps_insert,
            method="POST",
            payload=json.dumps({"map_path": map_path}),
            headers={"Content-Type": "application/json"},
        )

        # 0: 成功 -1: 失败
        return int(result["code"])


# sourcery skip: remove-redundant-if
if __name__ == "__main__":
    pass
