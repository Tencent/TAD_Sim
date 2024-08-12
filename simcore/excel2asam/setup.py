#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# /*============================================================================
# * \file setup.py
# * \brief Package the source code.
# *
# * Install the python package locally
# *     python3 setup.py install
# * Use the development mode to install this package.
# * After each code modification, the latest code can be used immediately
# *     python3 setup.py develop
# *
# * \author xingboliu <xingboliu@tencent.com>
# * \date 2024-04-10
# * \version 1.0.0
# * \Copyright 2024 Tencent Inc. All rights reserved.
# /*============================================================================
import setuptools

with open("README.md", "r", encoding="utf-8") as f:
    long_description = f.read()

with open("requirements_dev.txt") as f:
    required = f.read().splitlines()

setuptools.setup(
    name="excel2asam",
    version="0.0.1",
    license="",
    author="xingboliu",
    author_email="xingboliu@tencent.com",
    description="Through excel to generation of OpenSCENARIO xml files",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=setuptools.find_packages(),
    keywords=["excel", "OpenSCENARIO", "OpenDRIVE", "OpenX" "xml"],
    install_requires=required,
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
    ],
    python_requires=">=3.6",
    include_package_data=True,
)
