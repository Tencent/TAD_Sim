#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file model_dbscan.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

import math
from dataclasses import dataclass, field
from typing import List, Optional

import numpy as np
import pandas as pd
from loguru import logger
from sklearn import metrics
from sklearn.cluster import DBSCAN
from sklearn.decomposition import PCA
from sklearn.preprocessing import LabelEncoder, MinMaxScaler

from excel2asam.config import settings
from excel2asam.utils import SceneFilter


@dataclass(order=True)
class ModelDbscan:
    df: pd.DataFrame
    scene_filter: SceneFilter
    n_x: pd.DataFrame = field(default_factory=pd.DataFrame)
    n_features: float = field(default=24)

    def __post_init__(self):
        self.n_row = self.df.shape[0]

    def _find_closest_ego_npc(self, row: pd.Series):
        min_distance = float("inf")
        closest_npc = None
        if row["AllnumNpc"] > 0:
            for i in range(1, row["AllnumNpc"] + 1):
                if not pd.isnull(row[f"Npc{i}.Rel.Ref"]) and row[f"Npc{i}.Rel.Ref"] == "Ego":
                    if (
                        not pd.isnull(row[f"Npc{i}.Rel.Long"])
                        and row[f"Npc{i}.Rel.Long"] > 0
                        and row[f"Npc{i}.Rel.Long"] < min_distance
                    ):
                        min_distance = row[f"Npc{i}.Rel.Long"]
                        closest_npc = i

        if closest_npc is not None:
            if not pd.isnull(row[f"Npc{closest_npc}.AllnumDyn"]) and row[f"Npc{closest_npc}.AllnumDyn"] > 0:
                return pd.Series(
                    [
                        row[f"Npc{closest_npc}.Phy.Category"],
                        row[f"Npc{closest_npc}.Phy.Model"],
                        row[f"Npc{closest_npc}.Ini.Speed"],
                        row[f"Npc{closest_npc}.Dyn1.Action1.Type"],
                        row[f"Npc{closest_npc}.Dyn1.Trigger.Type"],
                        row[f"Npc{closest_npc}.Dyn1.Trigger.Value"],
                    ],
                    index=[
                        "npc_category",
                        "npc_model",
                        "npc_ini_speed",
                        "npc_action_type",
                        "npc_trigger_type",
                        "npc_trigger_value",
                    ],
                )
            else:
                return pd.Series(
                    [
                        row[f"Npc{closest_npc}.Phy.Category"],
                        row[f"Npc{closest_npc}.Phy.Model"],
                        row[f"Npc{closest_npc}.Ini.Speed"],
                        "none",
                        "none",
                        0,
                    ],
                    index=[
                        "npc_category",
                        "npc_model",
                        "npc_ini_speed",
                        "npc_action_type",
                        "npc_trigger_type",
                        "npc_trigger_value",
                    ],
                )
        return pd.Series(
            ["none", "none", 0, "none", "none", 0],
            index=[
                "npc_category",
                "npc_model",
                "npc_ini_speed",
                "npc_action_type",
                "npc_trigger_type",
                "npc_trigger_value",
            ],
        )

    def _ego_feature_trans(self, row: pd.Series):
        if not pd.isnull(row["Ego.AllnumDyn"]) and row["Ego.AllnumDyn"] > 0:
            return pd.Series(
                [
                    row["Ego.Phy.Model"],
                    row["Ego.Ini.Speed"],
                    row["Ego.Dyn1.Action1.Type"],
                    row["Ego.Dyn1.Trigger.Type"],
                    row["Ego.Dyn1.Trigger.Value"],
                ],
                index=[
                    "ego_model",
                    "ego_ini_speed",
                    "ego_action_type",
                    "ego_trigger_type",
                    "ego_trigger_value",
                ],
            )
        else:
            return pd.Series(
                [
                    row["Ego.Phy.Model"],
                    row["Ego.Ini.Speed"],
                    "none",
                    "none",
                    0,
                ],
                index=[
                    "ego_model",
                    "ego_ini_speed",
                    "ego_action_type",
                    "ego_trigger_type",
                    "ego_trigger_value",
                ],
            )

    def _feature_select(self):
        """
        In this step, we selected most affected features related to scenarios.
        """
        # map
        f_road = [
            "Road.Type",
            "Road.Curve",
            "Road.Direction",
            "Road.Speedlimit",
            "Junction.Num",
            "Junction.Type",
            "Lane.Edge",
            "Lane.Num",
            "Lane.Type",
            "Lane.Width",
        ]
        f_trafficlight = ["Trafficlight.Cycle"]
        # env
        f_environment = ["Env.Time", "Env.Weather"]
        # ego
        f_ego = ["ego_model", "ego_ini_speed", "ego_action_type", "ego_trigger_type", "ego_trigger_value"]
        # obj
        f_npc = [
            "npc_category",
            "npc_model",
            "npc_ini_speed",
            "npc_action_type",
            "npc_trigger_type",
            "npc_trigger_value",
        ]
        features = f_road + f_trafficlight + f_ego + f_npc + f_environment
        self.df[f_ego] = self.df.apply(self._ego_feature_trans, axis=1)
        self.df[f_npc] = self.df.apply(self._find_closest_ego_npc, axis=1)
        self.n_x = self.df[features]
        self.n_features = len(features) - 1

    def _data_process(self):
        """
        Filling missing values and normalization
        """
        categorical_features = [
            "Road.Type",
            "Road.Direction",
            "Junction.Type",
            "Lane.Edge",
            "Lane.Type",
            "Trafficlight.Cycle",
            "ego_model",
            "ego_action_type",
            "ego_trigger_type",
            "npc_category",
            "npc_model",
            "npc_action_type",
            "npc_trigger_type",
            "Env.Time",
            "Env.Weather",
        ]
        numerical_features = [x for x in self.n_x.columns if x not in categorical_features]
        self.n_x[categorical_features] = self.n_x[categorical_features].astype(str).fillna("none")
        self.n_x[numerical_features] = self.n_x[numerical_features].fillna(0)

        # 对每个分类特征列使用LabelEncoder
        for c_feature in categorical_features:
            label_encoder = LabelEncoder()
            self.n_x[c_feature] = label_encoder.fit_transform(self.n_x[c_feature])

        # 对特征列进行MinMaxScaler归一化
        scaler = MinMaxScaler()
        x_transformed = scaler.fit_transform(self.n_x)
        self.n_x = pd.DataFrame(x_transformed, columns=self.n_x.columns, index=self.n_x.index)

    def pca(self, n_components: int = 3):
        """
        PCA对数据进行降维
        :param n_components:
        :return:
        """
        pca = PCA(n_components=n_components)
        return pca.fit_transform(self.n_x)

    def dbscan(self, eps: float = 0.1, min_samples: int = 4, percentage: float = 0.2, fig: bool = True):
        """
        DBSCAN: 基于密度的场景泛化聚类算法
        :param eps: DBSCA中半径参数
        :param min_samples: DBSCAN中半径eps内最小样本数目
        :param fig: 是否对降维后的样本进行画图显示
        :return:
        """
        self._feature_select()
        self._data_process()

        result = pd.Series([False] * self.n_row)
        if self.n_x.empty:
            return result

        n_components = 3 if self.n_x.shape[0] > 3 else self.n_x.shape[0]
        pca_weights = self.pca(n_components)

        clf = DBSCAN(eps=eps, min_samples=min_samples)

        y = clf.fit_predict(pca_weights)

        # if fig:
        #     plt.ion()  # 开启交互式绘图, 之后plt.show()失效, 图片会自动显示
        #     plt.scatter(pca_weights[:, 0], pca_weights[:, 1], c=y)
        #     # plt.show()
        #     plt.savefig("./plt.png")

        # 计算轮廓系数
        # metrics.silhouette_score 函数计算轮廓系数. 轮廓系数越高, 聚类质量越好.
        # 该分数的范围是 -1 到 1, 其中 -1 是最差, 1 是最好.
        if len(np.unique(y)) > 1:
            silhouette_score = metrics.silhouette_score(self.n_x, y)
            logger.opt(lazy=True).info(f"silhouette_score = {silhouette_score}")

        self.n_x = pd.DataFrame(self.n_x)
        self.n_x["label"] = y
        # 计算每个簇的几何中心
        cluster_centers = self.n_x.groupby("label").mean().values

        # 计算每个样本到其对应簇中心的距离
        distances: List[Optional[float]] = []
        logger.opt(lazy=True).info(f"feature numbers = {self.n_features}")
        for _, row in self.n_x.iterrows():
            if row["label"] == -1:  # 如果是噪声点, 跳过
                distances.append(None)
            else:
                cluster_center = cluster_centers[int(row["label"])]
                distance = float(np.linalg.norm(row[: int(self.n_features)] - cluster_center[:-1]))
                distances.append(distance)
        self.n_x["distance_to_center"] = distances

        # 密度采样获取有价值的场景
        labels = self.n_x["label"].unique()
        sampled_df = pd.DataFrame()

        for label in labels:
            class_df = self.n_x[self.n_x["label"] == label].sort_values(by="distance_to_center")
            num_samples = len(class_df)
            density = np.linspace(0, num_samples - 1, num=math.ceil(num_samples * percentage), dtype=int)

            # 使用密度函数进行采样
            sampled_class_df = class_df.iloc[density]
            sampled_df = pd.concat([sampled_df, sampled_class_df])

        result[sampled_df.index] = True
        logger.opt(lazy=True).info(f"Clustering algorithm model screening num = {result.sum()}")
        return result

    def process(self):
        logger.opt(lazy=True).info(f"{' FilterFormater Processing: ':-^35}")
        if self.df.empty:
            return self.df

        return self.scene_filter.process(
            df=self.df,
            keep_cond=self.dbscan(
                eps=settings.sys.model.eps,
                min_samples=settings.sys.model.min_samples,
                percentage=settings.sys.model.percentage,
                fig=False,
            ),
            reason="通过 Clustering 算法筛选的场景",
            assessment="聚类规则",
        )
