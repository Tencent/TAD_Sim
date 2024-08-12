#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file exceptions.py
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================

from __future__ import annotations


class PathfileIllegalError(Exception):
    """Raised when pathfile is illegal in a class."""

    pass


class SheetDefineNotFoundError(Exception):
    """Raised when without sheet in excel."""

    pass


class ColumnsDefineNotFoundError(Exception):
    """Raised when without columns in excel."""

    pass


class EmptyError(Exception):
    """Raised when content is empty."""

    pass


class AfterFilterEmptyError(Exception):
    """Raised when content is empty after filter."""

    pass


class GeneralizedEmptyError(Exception):
    """Raised when generalized is empty."""

    pass


class TestcaseEmptyError(Exception):
    """Raised when testcase is empty."""

    pass


class MapConfigError(Exception):
    """Raised when testcase is empty."""

    pass
