#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file parser.py
# * \brief This file is used to parse the excel file
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import json
from abc import ABC, abstractmethod
from dataclasses import dataclass
from pathlib import Path
from typing import List

import excel2asam.exceptions as exp
import pandas as pd
import requests
from loguru import logger

from excel2asam.config import settings


@dataclass(order=True)
class ParserFactory(ABC):
    """解析抽象工厂类"""

    def __post_init__(self):
        self.names = self.get_names()

    def delete_explain_line(self, df: pd.DataFrame) -> pd.DataFrame:
        # 删除解释行
        if any(settings.sys.explain_string in str(cell) for cell in df.iloc[0].values if not pd.isna(cell)):
            df = df.iloc[1:]

        return df

    def get_dataframe_define(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' get_dataframe_define: ':-^35}")

        # 检查文档的名字中是否包含 settings.sys.sheet.define 的字段
        name_list = [x for x in self.names if settings.sys.sheet.define in x]
        df_list = [self.to_dataframe(x) for x in name_list]
        logger.opt(lazy=True).info(f"define name list: {name_list}")

        # 异常处理 df_list 是空
        if not df_list:
            err_code = f"Without any {settings.sys.sheet.define} in excel: '{self.names}'"
            logger.opt(lazy=True, colors=True).warning(err_code)
            raise exp.SheetDefineNotFoundError(err_code)

        return pd.concat(df_list, ignore_index=True)

    def get_dataframe_param(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' get_dataframe_param: ':-^35}")

        # 检查文档的名字中是否包含 settings.sys.sheet.param 的字段
        param_names = [x for x in self.names if settings.sys.sheet.param in x]

        # 只处理第一个 sheet_param 的信息, 如果没有则返回空的 dataframe
        return self.to_dataframe(param_names[0]) if param_names else pd.DataFrame()

    def get_dataframe_settings_user(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' get_dataframe_settings_user: ':-^35}")

        # 检查文档的名字中是否包含 settings.sys.sheet.settings_user 的字段
        settings_user_names = [x for x in self.names if settings.sys.sheet.settings_user in x]

        # 只处理第一个 sheet_settings_user 的信息, 如果没有则返回空的 dataframe
        return self.to_dataframe(settings_user_names[0]) if settings_user_names else pd.DataFrame()

    def get_dataframe_classify_user(self) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{' get_dataframe_classify_user: ':-^35}")

        # 检查文档的名字中是否包含 settings.sys.sheet.classify_user 的字段
        classify_user_names = [x for x in self.names if settings.sys.sheet.classify_user in x]

        # 只处理第一个 classify_user_names 的信息, 如果没有则返回空的 dataframe
        return self.to_dataframe(classify_user_names[0]) if classify_user_names else pd.DataFrame()

    @abstractmethod
    def get_names(self) -> List:
        pass

    @abstractmethod
    def to_dataframe(self, *args) -> pd.DataFrame:
        pass


@dataclass(order=True)
class ExcelParserFactory(ParserFactory):
    """本地表格解析器"""

    pathfile_xlsx: Path

    def __post_init__(self):
        self._xlsx = self._get_xlsx()
        self.names = self.get_names()

    def _get_xlsx(self) -> pd.ExcelFile:
        # 检查 excel 路径的合法性
        self.pathfile_xlsx = Path(self.pathfile_xlsx)
        logger.opt(lazy=True).info(f"Input excel pathdir: {self.pathfile_xlsx.parent}")
        logger.opt(lazy=True).info(f"Input excel name: {self.pathfile_xlsx.name}")
        if not self.pathfile_xlsx.is_file():
            raise exp.PathfileIllegalError("Input excel fullpath: is illegal")

        # 获取 excel 数据
        return pd.ExcelFile(self.pathfile_xlsx)

    def get_names(self) -> List:
        return self._xlsx.sheet_names

    def to_dataframe(self, *args) -> pd.DataFrame:
        #
        name = args[0]

        # 异常处理, 检查 sheet_define 是否存在
        if name not in self.names:
            return pd.DataFrame()

        # 读取 sheet 为参数的信息
        df = self._xlsx.parse(sheet_name=name, na_values="").convert_dtypes(dtype_backend="pyarrow")

        # 删除解释行
        return self.delete_explain_line(df)


@dataclass(order=True)
class FeishuBittableParserFactory(ParserFactory):
    """飞书多维表格解析器"""

    app_id: str
    app_secret: str
    app_token: str

    def __post_init__(self):
        # 获取 access_token
        self.tenant_access_token = self._get_tenant_access_token()
        #
        self.tables_dict = self._get_tables_dict()
        #
        self.names = self.get_names()

    def _request_api(self, url: str, method: str, payload: str, headers: dict) -> dict:
        # 使用 requests 库
        response = requests.request(method, url, headers=headers, data=payload)
        # 获取响应数据
        data = response.text

        # # 使用 http.client 库
        # # 发送请求
        # import http.client
        # conn = http.client.HTTPSConnection(settings.sys.feishu.host)
        # conn.request(method, url, payload, headers)
        # # 获取响应数据
        # response = conn.getresponse()
        # data = response.read()
        # # 关闭连接
        # conn.close()

        # 将响应数据解析为 JSON
        result = json.loads(data)
        if result.get("Error"):
            logger.opt(lazy=True).error(f"Get error: {result['Error']}")
        return result

    def _get_tenant_access_token(self) -> str:
        """获取 tenant_access_token"""

        result = self._request_api(
            url=settings.sys.feishu.url_tenant_access_token,
            method="POST",
            payload=json.dumps({"app_id": self.app_id, "app_secret": self.app_secret}),
            headers={"Content-Type": "application/json"},
        )

        return result["tenant_access_token"]

    def _get_tables_dict(self) -> dict:
        """获取所有 tables 信息"""

        # 请求 URL
        url = f"{settings.sys.feishu.url_apps}/{self.app_token}/tables"

        # 发送请求并获取响应
        result = self._request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
        )

        # 获取表格信息
        return {item["name"]: item["table_id"] for item in result["data"]["items"]}

    def _get_bittable_info(self, table_id: str, page_token: str) -> dict:
        """获取用户信息"""

        # 请求 URL, 考虑分页存在多页
        url = f"{settings.sys.feishu.url_apps}/{self.app_token}/tables/{table_id}/records?page_size=500"
        if page_token:
            url += f"&page_token={page_token}"

        # 发送请求并获取响应
        return self._request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
        )

    def get_names(self) -> List:
        return self.tables_dict.keys()

    def to_dataframe(self, *args) -> pd.DataFrame:
        #
        name = args[0]

        # 异常处理, 检查 sheet_define 是否存在
        if name not in self.names:
            return pd.DataFrame()

        table_id = self.tables_dict[name]

        # 初始化一个空的列表
        all_page_items = []

        # 获取第一页数据
        records = self._get_bittable_info(table_id, "")

        # 异常处理, 无法获得数据 or 数据为空 时返回空的数据帧
        if records["code"] != 0 or records["data"]["total"] == 0:
            return pd.DataFrame()

        # 循环获取分页数据
        while True:
            # 添加当前页的数据到 all_page_items
            all_page_items.extend(records["data"]["items"])

            # 检查是否有更多数据
            if not records["data"]["has_more"]:
                break

            # 获取下一页数据
            records = self._get_bittable_info(table_id, records["data"]["page_token"])

        # 将所有响应数据转换为 pandas 数据帧
        df = pd.json_normalize(all_page_items).convert_dtypes(dtype_backend="pyarrow")

        # 使用str.replace()方法替换列名
        df.columns = df.columns.str.replace("fields.", "", regex=False)

        # 检查是否具有无用的列, 有则删除整列
        cols_to_drop = [col for col in ["id", "record_id", "SourceID", "序号"] if col in df.columns]
        df = df.drop(cols_to_drop, axis=1)

        # 删除解释行
        return self.delete_explain_line(df)


@dataclass(order=True)
class Parser:
    parser_factory: ParserFactory
    virtual_real_is_virtual: bool

    def __post_init__(self):
        logger.opt(lazy=True).info(f"{' Parse and check: ':=^55}")

        # 读取定义配置
        self.define = self.parser_factory.get_dataframe_define()
        # 读取参数配置
        self.param = pd.DataFrame() if self.virtual_real_is_virtual else self.parser_factory.get_dataframe_param()
        # 读取定义配置
        self.settings_user = self.parser_factory.get_dataframe_settings_user()
        # 读取分类属性
        self.classify_user = self.parser_factory.get_dataframe_classify_user()
