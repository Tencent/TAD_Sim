#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import pytest

from excel2asam.utils import check_add_suffix


def test_check_add_suffix():
    # Test case 1: pathfile already ends with suffix
    assert check_add_suffix("file.txt", ".txt") == "file.txt"

    # Test case 2: pathfile doesn't end with suffix
    assert check_add_suffix("file", ".txt") == "file.txt"

    # Test case 3: suffix doesn't contain a period
    assert check_add_suffix("file", "txt") == "file.txt"

    # Test case 4: suffix is an empty string
    assert check_add_suffix("file.txt", "") == "file.txt"

    # Test case 5: suffix is None
    with pytest.raises(TypeError):
        check_add_suffix("file.txt", None)
