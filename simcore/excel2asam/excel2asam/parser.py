#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file parser.py
# * \brief This file is used to parse the file
# * ParserFactory (Abstract Factory)
# * ├── ExcelParser (Concrete class)
# * ├── FeishuBitableParser (Concrete class)
# * └── FeishuSpreadsheetParser (Concrete class)
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2025-01-04
# * \version 1.0.1
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


#
@dataclass(order=True)
class Feishu:
    # web api
    def request_api(self, url: str, method: str, payload: str, headers: dict, user: str = "") -> dict:
        try:
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

            # 如果正常会返回 0
            if result["code"]:
                raise exp.PermissionError(f"{user}: {result}")
            return result
        except requests.exceptions.RequestException as e:
            raise exp.RequestError(f"Request failed: {e}") from e

    def get_tenant_access_token(self, app_id: str, app_secret: str) -> str:
        """
        获取 tenant_access_token

        Args:
            app_id (str): The application ID.
            app_secret (str): The application secret.

        Returns:
            str: The tenant access token.
        """

        result = self.request_api(
            url=f"{settings.sys.feishu.host}/{settings.sys.feishu.url_tenant_access_token}",
            method="POST",
            payload=json.dumps({"app_id": app_id, "app_secret": app_secret}),
            headers={"Content-Type": "application/json; charset=utf-8"},
            user="get_tenant_access_token",
        )

        return result["tenant_access_token"]

    def update_token_from_wikispaces(self, tenant_access_token: str, token: str, obj_type: str = "wiki"):
        """
        更新 token 如果该文件是存在于知识空间即知识库中.
        """
        try:
            # get token from wikispaces
            # https://https://open.feishu.cn/document/server-docs/docs/wiki-v2/space-node/get_node
            result = self.request_api(
                url=f"{settings.sys.feishu.host}/{settings.sys.feishu.url_wikispaces}?obj_type={obj_type}&token={token}",
                method="GET",
                payload="",
                headers={"Authorization": f"Bearer {tenant_access_token}"},
                user="update_token_from_wikispaces",
            )

            logger.info("File in wikispaces, update token")
            token = result["data"]["node"]["obj_token"]

        except exp.PermissionError:
            logger.info("File not in wikispaces, no need to update")

        finally:
            return token


#
@dataclass(order=True)
class ParserFactory(ABC):
    """解析抽象工厂类"""

    # abstract method
    @abstractmethod
    def get_names(self, key: str) -> List:
        pass

    # abstract method
    @abstractmethod
    def to_dataframe(self, *args) -> pd.DataFrame:
        pass

    # abstract method
    @abstractmethod
    def get_dicts(self) -> dict:
        pass

    # abstract method
    @abstractmethod
    def get_info(self, *args) -> dict:
        pass


@dataclass(order=True)
class ExcelParser(ParserFactory):
    """本地表格解析器"""

    pathfile_xlsx: Path

    def __post_init__(self):
        logger.opt(lazy=True).info(f"{' ExcelParserFactory: ':=^55}")
        self._xlsx = self.get_info()

    def get_dicts(self) -> dict:
        pass

    def get_info(self) -> pd.ExcelFile:
        # 检查 excel 路径的合法性
        self.pathfile_xlsx = Path(self.pathfile_xlsx)
        logger.opt(lazy=True).info(f"Input excel pathdir: {self.pathfile_xlsx.parent}")
        logger.opt(lazy=True).info(f"Input excel name: {self.pathfile_xlsx.name}")
        if not self.pathfile_xlsx.is_file():
            raise exp.PathfileIllegalError("Input excel fullpath: is illegal")

        # 获取 excel 数据
        return pd.ExcelFile(self.pathfile_xlsx)

    def get_names(self, key: str) -> List:
        return [x for x in self._xlsx.sheet_names if key in x]

    def to_dataframe(self, *args) -> pd.DataFrame:
        key = args[0]
        multiindexnum = args[1]

        # 读取 sheet 为参数的信息
        header = 0 if multiindexnum == 1 else list(range(multiindexnum))
        return self._xlsx.parse(sheet_name=key, header=header, na_values="").convert_dtypes(dtype_backend="pyarrow")


@dataclass(order=True)
class FeishuBitableParser(ParserFactory):
    """飞书多维表格解析器"""

    app_id: str
    app_secret: str
    token: str

    def __post_init__(self):
        logger.opt(lazy=True).info(f"{' FeishuBitableParser: ':=^55}")
        # 初始化一个 Feishu 实例
        self.feishu = Feishu()
        # 获取 access_token
        self.tenant_access_token = self.feishu.get_tenant_access_token(self.app_id, self.app_secret)
        # 更新 token 如果飞书多维表格属于知识库
        self.token = self.feishu.update_token_from_wikispaces(self.tenant_access_token, self.token)
        # 获取所有数据表
        self.dicts = self.get_dicts()

    def get_dicts(self) -> dict:
        """获取所有 tables 信息"""

        # 请求 URL
        # https://open.feishu.cn/document/server-docs/docs/bitable-v1/app-table/list
        url = f"{settings.sys.feishu.host}/{settings.sys.feishu.url_bitable}/{self.token}/tables"

        # 发送请求并获取响应
        result = self.feishu.request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
            user="FeishuBitableParser.get_dicts",
        )

        # 获取表格信息
        return {item["name"]: item["table_id"] for item in result["data"]["items"]}

    def get_info(self, table_id: str, page_token: str) -> dict:
        """获取用户信息"""

        # 请求 URL, 考虑分页存在多页
        # https://open.feishu.cn/document/uAjLw4CM/ukTMukTMukTM/reference/bitable-v1/app-table-record/search
        url = f"{settings.sys.feishu.host}/{settings.sys.feishu.url_bitable}/{self.token}/tables/{table_id}/records?page_size=500"
        if page_token:
            url += f"&page_token={page_token}"

        # 发送请求并获取响应
        return self.feishu.request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
            user="FeishuBitableParser.get_info",
        )

    def get_names(self, key: str) -> List:
        return [x for x in self.dicts.keys() if key in x]

    def to_dataframe(self, *args) -> pd.DataFrame:
        key = args[0]

        table_id = self.dicts[key]

        # 初始化一个空的列表
        all_page_items = []

        # 获取第一页数据
        records = self.get_info(table_id, "")

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
            records = self.get_info(table_id, records["data"]["page_token"])

        # 将所有响应数据转换为 pandas 数据帧
        df = pd.json_normalize(all_page_items).convert_dtypes(dtype_backend="pyarrow")

        # 使用str.replace()方法替换列名
        df.columns = df.columns.str.replace("fields.", "", regex=False)

        # 检查是否具有无用的列, 有则删除整列
        cols_to_drop = [col for col in ["id", "record_id", "SourceID", "序号"] if col in df.columns]
        return df.drop(cols_to_drop, axis=1)


@dataclass(order=True)
class FeishuSpreadsheetParser(ParserFactory):
    """飞书电子表格解析器"""

    app_id: str
    app_secret: str
    token: str

    def __post_init__(self):
        logger.opt(lazy=True).info(f"{' FeishuSpreadsheetParser: ':=^55}")
        # 初始化一个 Feishu 实例
        feishu = Feishu()
        # 获取 access_token
        self.tenant_access_token = feishu.get_tenant_access_token(self.app_id, self.app_secret)
        # 更新 token 如果飞书电子表格属于知识库
        self.token = feishu.update_token_from_wikispaces(self.tenant_access_token, self.token)
        # 获取所有工作表
        self.dicts = self.get_dicts()

    def _get_column_letter(self, col_num):
        if col_num < 1:
            raise ValueError("Column number must be greater than or equal to 1")

        result = ""
        while col_num > 0:
            col_num, remainder = divmod(col_num - 1, 26)
            result = chr(65 + remainder) + result

        return result

    def get_dicts(self) -> dict:
        """获取所有 sheets 信息"""
        # 请求 URL
        # https://open.feishu.cn/document/server-docs/docs/sheets-v3/spreadsheet-sheet/query?appId=cli_a4da1529e878500d
        url = f"{settings.sys.feishu.host}/{settings.sys.feishu.url_spreadsheets}/{self.token}/sheets/query"

        # 发送请求并获取响应
        result = self.feishu.request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
            user="FeishuSpreadsheetParser.get_dicts",
        )

        out = {}
        for item in result["data"]["sheets"]:
            column_count = row_count = 2
            if "grid_properties" in item:
                column_count = item["grid_properties"]["column_count"]
                row_count = item["grid_properties"]["row_count"]

            out[item["title"]] = {
                "sheet_id": item["sheet_id"],
                "column_count": column_count,
                "row_count": row_count,
            }

        # 获取表格信息
        return out

    def get_info(self, sheet_info: dict, page_token: str) -> dict:
        """获取用户信息"""

        # 请求 URL
        # https://open.feishu.cn/document/server-docs/docs/sheets-v3/data-operation/reading-a-single-range
        url = (
            f"{settings.sys.feishu.host}/{settings.sys.feishu.url_spreadsheets.replace('v3', 'v2')}/{self.token}/values/"
            f"{sheet_info['sheet_id']}!A1:{self._get_column_letter(sheet_info['column_count'])}{sheet_info['row_count']}"
            "?valueRenderOption=ToString&dateTimeRenderOption=FormattedString"
        )

        # 发送请求并获取响应
        return self.feishu.request_api(
            url=url,
            method="GET",
            payload="",
            headers={"Authorization": f"Bearer {self.tenant_access_token}"},
            user="FeishuSpreadsheetParser.get_info",
        )

    def get_names(self, key: str) -> List:
        return [x for x in self.dicts.keys() if key in x]

    def to_dataframe(self, *args) -> pd.DataFrame:
        key = args[0]
        multiindexnum = args[1]
        #
        sheet_info = self.dicts[key]
        records = self.get_info(sheet_info, "")

        # 异常处理, 无法获得数据 or 数据为空 时返回空的数据帧
        if records["code"] != 0 or records["data"]["valueRange"]["range"] == "":
            return pd.DataFrame()

        # 将所有响应数据转换为 pandas 数据帧, 去除整行为空, 或整列为空
        values = records["data"]["valueRange"]["values"]
        return (
            pd.DataFrame.from_records(data=values[multiindexnum:], columns=values[:multiindexnum])
            .dropna(how="all")
            .dropna(axis=1, how="all")
            .convert_dtypes(dtype_backend="pyarrow")
        )


#
@dataclass(order=True)
class Parser:
    input_mode: str
    input_data: str
    virtual_real_is_virtual: bool

    def __post_init__(self):
        logger.opt(lazy=True).info(f"{' Parse and check: ':=^55}")

        # 根据模式获得 parser_factory
        pf = self._get_parser_factory()

        # 读取定义配置
        self.define = self._get_dataframe_define(pf)
        self.define = self._delete_explain_line(self.define)
        # 读取参数配置
        self.param = pd.DataFrame() if self.virtual_real_is_virtual else self._get_dataframe_by_key(pf, "param")
        self.param = self._delete_explain_line(self.param)
        # 读取定义配置
        self.settings_user = self._get_dataframe_by_key(pf, "settings_user")
        self.settings_user = self._delete_explain_line(self.settings_user)
        # 读取分类属性
        self.classify_user = self._get_dataframe_by_key(pf, "classify_user")
        self.classify_user = self._delete_explain_line(self.classify_user)

    #
    def _get_parser_factory(self):
        # 根据模式获得 parser_factory
        if self.input_mode == "excel":
            pf = ExcelParser(Path(self.input_data))
        elif self.input_mode == "feishu_bitable":
            pf = FeishuBitableParser(
                app_id=settings.feishu.app_id,
                app_secret=settings.feishu.app_secret,
                token=self.input_data,
            )
        elif self.input_mode == "feishu_spreadsheet":
            pf = FeishuSpreadsheetParser(
                app_id=settings.feishu.app_id,
                app_secret=settings.feishu.app_secret,
                token=self.input_data,
            )
        else:
            raise ValueError(f"Invalid input_mode: {self.input_mode = }")

        return pf

    #
    def _delete_explain_line(self, df: pd.DataFrame) -> pd.DataFrame:
        """
        删除解释行

        Args:
            df (pd.DataFrame): The dataframe from which to delete the explain line.

        Returns:
            pd.DataFrame: The dataframe with the explain line deleted.
        """

        if not df.empty and any(
            settings.sys.explain_string in str(cell) for cell in df.iloc[0].values if not pd.isna(cell)
        ):
            df = df.iloc[1:]

        return df

    def _get_dataframe_define(self, pf: ParserFactory, multiindexnum: int = 1) -> pd.DataFrame:
        """
        获取定义配置的 DataFrame

        Returns:
            pd.DataFrame: 拼接后的定义配置 DataFrame
        """
        logger.opt(lazy=True).info(f"{' get_dataframe_define: ':-^35}")

        # 检查文档的名字中是否包含 settings.sys.sheet.define 的字段
        define_names = pf.get_names(settings.sys.sheet.define)

        # 异常处理 define_names 是空
        if not define_names:
            raise exp.SheetDefineNotFoundError(f"Without any {settings.sys.sheet.define}")

        # 处理输入错误 < 1 的情况
        multiindexnum = max(multiindexnum, 1)

        # 正常处理 define_names 时, 进而获取 sheet 中数据, 并返回拼接后结果
        logger.opt(lazy=True).info(f"define name list: {define_names}")
        df_list = [pf.to_dataframe(x, multiindexnum) for x in define_names]

        return pd.concat(df_list, ignore_index=True)

    def _get_dataframe_by_key(self, pf: ParserFactory, key: str, multiindexnum: int = 1) -> pd.DataFrame:
        logger.opt(lazy=True).info(f"{f' get_dataframe_{key}':-^35}")

        # 检查 key 是否存在于 settings.sys.sheet 中
        if key not in settings.sys.sheet:
            logger.opt(lazy=True, colors=True).warning(f"Key '{key}' not found in {settings.sys.sheet.keys()}")
            return pd.DataFrame()

        # 检查文档的名字中是否包含 settings.sys.sheet.param 的字段
        names = pf.get_names(settings.sys.sheet[key])

        # 处理输入错误 < 1 的情况
        multiindexnum = max(multiindexnum, 1)

        # 只处理第一个 names 的信息, 如果没有则返回空的 dataframe
        return pf.to_dataframe(names[0], multiindexnum) if names else pd.DataFrame()
