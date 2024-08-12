#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Configuration file for the Sphinx documentation builder.

# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# pylint: disable=invalid-name

import os

# -- Project information -------------------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information
project = "TAD Sim"
copyright = "Copyright (C) 2024 Tencent Inc.  All rights reserved"
author = "TAD Sim"
release = "1.0.0"  # The full version, including alpha/beta/rc tags
# version = "1.0.0"

# 设置版本区分的信息
EDITION_CHN = os.environ.get("EDITION_CHN", "商业版")
rst_prolog = f"""
.. |EDITION_CHN| replace:: {EDITION_CHN}
"""


# -- General configuration -----------------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration
# intersphinx_mapping = {
#     "python": ("https://docs.python.org/3/", None),
#     "sphinx": ("https://www.sphinx-doc.org/en/master/", None),
# }
source_parsers = {
    ".md": "recommonmark.parser.CommonMarkParser",
}
source_suffix = {
    ".rst": "restructuredtext",
    ".md": "markdown",
}
extensions = [
    "sphinx_markdown_tables",
    "sphinxcontrib.mermaid",  # 在 Sphinx 文档中使用 Mermaid 图表描述语言来创建流程图、序列图
    "sphinx.ext.duration",  # 解析和显示时间间隔的扩展
    "sphinx.ext.doctest",  # 允许在 Sphinx 文档中嵌入代码片段, 并执行它们, 然后显示输出和期望的输出
    "sphinx.ext.autodoc",  # 自动生成基于 Python 模块、类、函数等的 docstrings 的文档
    "sphinx.ext.autosummary",  # 自动生成摘要文件, 适用于为多个类或函数生成统一的概览或总结
    "sphinx.ext.intersphinx",  # 允许在 Sphinx 文档中引用其他 Sphinx 项目中的对象, 例如模块、类或函数
    "myst_parser",  # 支持使用 Myst Markdown 语法
    "breathe",  # 与 Doxygen 工具一起使用, 将 Doxygen 注释转换为 Sphinx 文档
    "exhale",  # 与 Breathe 结合使用, 可以生成 C++ API 文档
]
intersphinx_disabled_domains = ["std"]
templates_path = ["_templates"]
language = "zh_CN"
exclude_patterns = ["build", "Thumbs.db", ".DS_Store"]
autosectionlabel_prefix_document = True

primary_domain = "cpp"
highlight_language = "cpp"

# -- Options for HTML output ---------------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output
html_theme = "sphinx_rtd_theme"
html_title = "Docs"
html_search_language = "zh_CN"
html_static_path = ["_static"]
html_css_files = ["extra.css"]
html_show_sphinx = False  # 移除底部 "利用 Sphinx 构建"
html_show_sourcelink = False  # 移除底部 "主题 由 Read the Docs 开发"
html_context = {
    "show_source": False,  # 移除 "查看页面源码" 的选项按钮
}

# -- Options for myst_parser configuration -------------------------------------------
myst_enable_extensions = [
    "html_image",
]
myst_heading_anchors = 4

# -- Options for breathe configuration -----------------------------------------------
TADSIM_DOCS_DIR_DOXYGEN = os.environ.get("TADSIM_DOCS_DIR_DOXYGEN", "build/_doxyout")
breathe_projects = {
    "txSimSDK": f"{TADSIM_DOCS_DIR_DOXYGEN}/xml",
}
breathe_default_project = "txSimSDK"

# -- Options for exhale configuration -----------------------------------------------
TADSIM_DOCS_DIR_EXHALE = os.environ.get("TADSIM_DOCS_DIR_EXHALE", "./api")
exhale_args = {
    "containmentFolder": f"{TADSIM_DOCS_DIR_EXHALE}",
    "rootFileName": "library_root.rst",
    "rootFileTitle": "Library API",
    "doxygenStripFromPath": "..",
    "createTreeView": True,
    "fullToctreeMaxDepth": 1,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    # "exhaleExecutesDoxygen": True,
    # "exhaleDoxygenStdin":    "INPUT = ../src"
}
