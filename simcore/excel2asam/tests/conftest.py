#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# conftest.py
from __future__ import annotations

import pytest


def pytest_addoption(parser):
    parser.addoption("--f", action="store", default="./", help="Input folder path")


@pytest.hookimpl(optionalhook=True)
def pytest_html_results_table_header(cells):
    cells.pop(-1)  # delete link column of header


@pytest.hookimpl(optionalhook=True)
def pytest_html_results_table_row(report, cells):
    cells.pop(-1)  # delete link column of cell
