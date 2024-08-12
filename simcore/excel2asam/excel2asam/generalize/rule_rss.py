#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file rule_rss.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
from __future__ import annotations

from dataclasses import dataclass

import pandas as pd

from excel2asam.utils import SceneFilter


@dataclass(order=True)
class RuleRss:
    scenes: pd.DataFrame
    scene_filter: SceneFilter

    def process(self):
        return self.scenes
