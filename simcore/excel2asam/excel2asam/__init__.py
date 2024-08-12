#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from __future__ import annotations

import ctypes
import sys
from pathlib import Path

filename_binarys_so = [
    "libc.so.6",
    "libpthread.so.0",
    "libresolv.so.2",
    "libcrypt.so.1",
    "libroken.so.18",
    "libcom_err.so.2",
    "libheimbase.so.1",
    "libdl.so.2",
    "libasn1.so.8",
    "libm.so.6",
    "libwind.so.0",
    "libhcrypto.so.4",
    "libsqlite3.so.0",
    "libhx509.so.5",
    "libkrb5support.so.0",
    "libgcc_s.so.1",
    "libgmp.so.10",
    "libnettle.so.7",
    "libunistring.so.2",
    "libffi.so.7",
    "libkrb5.so.26",
    "libkeyutils.so.1",
    "libk5crypto.so.3",
    "libstdc++.so.6",
    "libicudata.so.66",
    "libhogweed.so.5",
    "libtasn1.so.6",
    "libidn2.so.0",
    "libp11-kit.so.0",
    "libheimntlm.so.0",
    "libkrb5.so.3",
    "libgeos-3.8.0.so",
    "liblzma.so.5",
    "libz.so.1",
    "libicuuc.so.66",
    "libbrotlicommon.so.1",
    "libgnutls.so.30",
    "libgssapi.so.3",
    "libsasl2.so.2",
    "liblber-2.4.so.2",
    "libcrypto.so.1.1",
    "libgssapi_krb5.so.2",
    "libgeos_c.so.1",
    "libproj.so.15",
    "libfreexl.so.1",
    "libxml2.so.2",
    "libbrotlidec.so.1",
    "libldap_r-2.4.so.2",
    "libssl.so.1.1",
    "libpsl.so.5",
    "libssh.so.4",
    "librtmp.so.1",
    "libnghttp2.so.14",
    "libspatialite.so.7",
    "libdatamodel.so",
    "libcurl.so.4",
    "libmapdb.so",
    "libmapimport.so",
    "libtransmission.so",
    "libmapengine.so",
    "librouteplan.so",
    "libroutingmap.so",
    "hadmap_py.cpython-311-x86_64-linux-gnu.so",
]

filename_binarys_dll = [
    "zlib1.dll",
    "liblzma.dll",
    "jpeg62.dll",
    "geos.dll",
    "libcurl.dll",
    "sqlite3.dll",
    "tiff.dll",
    "iconv-2.dll",
    "geos_c.dll",
    "proj.dll",
    "freexl-1.dll",
    "libxml2.dll",
    "datamodel.dll",
    "spatialite.dll",
    "transmission.dll",
    "mapimport.dll",
    "mapdb.dll",
    "mapengine.dll",
    "routeplan.dll",
    "routingmap.dll",
    "python311.dll",
    "hadmap_py.cp311-win_amd64.pyd",
    "hadmap_py.exp",
    "hadmap_py.lib",
]


# 获取当前 py 文件路径, 考虑通过 PyInstaller 编译为可执行程序后, __file__ 变量的行为变化问题
CURRENT_PATH_PY = Path(__file__).resolve().parent
if getattr(sys, "frozen", False):
    # CURRENT_PATH_PY = Path(sys.argv[0]).resolve().parent
    CURRENT_PATH_PY = Path(getattr(sys, "_MEIPASS", "."))

    # Just for debug
    # # 递归地列出当前目录及其子目录下的所有文件
    # for file in CURRENT_PATH_PY.rglob("*"):
    #     if file.is_file():
    #         print(file)

# 定义 lib 目录
PATHDIR_BINARY = CURRENT_PATH_PY / "map/lib"

filename_binarys_dict = {"linux": filename_binarys_so, "win32": filename_binarys_dll}

# 获取当前操作系统, 并基于不同系统得到不同平台的二进制文件名
if sys.platform == "linux":
    filename_binarys = filename_binarys_dict[sys.platform]

    # 遍历各个二进制文件, 并加载链接器
    for filename_binary in filename_binarys:
        pathfile_binary = PATHDIR_BINARY / filename_binary
        if pathfile_binary.is_file():
            ctypes.CDLL(str(pathfile_binary))
