# ============================================================================
# file Dockerfile
# brief Dockerfile file.
#
# author xingboliu <xingboliu@tencent.com>
# date 2024-06-22
# version 1.0.0
# Copyright 2024 Tencent Inc. All rights reserved.
# ============================================================================
FROM ubuntu:20.04

# 禁用交互模式
ENV DEBIAN_FRONTEND=noninteractive

# Install base libs and python
COPY requirements_dev.txt ./

RUN apt-get -y update \
    && apt-get --no-install-recommends install -y build-essential \
    git wget curl cmake libgoogle-glog-dev p7zip-full protobuf-compiler libprotobuf-dev \
    vim libproj-dev libspatialite-dev libcurl4-openssl-dev \
    libncurses-dev libgdbm-dev libz-dev tk-dev libsqlite3-dev \
    libreadline-dev liblzma-dev libffi-dev libssl-dev libbz2-dev ca-certificates \
    # hadmap build
    libboost-dev libeigen3-dev \
    # Install Python
    && wget --no-check-certificate https://www.python.org/ftp/python/3.11.8/Python-3.11.8.tgz \
    && tar -zxvf Python-3.11.8.tgz \
    && cd Python-3.11.8 \
    && ./configure --prefix=/usr/local --enable-shared LDFLAGS="-Wl,-rpath /usr/local/lib" \
    && make -j12 \
    && make install \
    # 设置 pip mirrors
    && echo "[global]\nindex-url = https://mirrors.tencent.com/pypi/simple\ntrusted-host = mirrors.cloud.tencent.com\n" > /etc/pip.conf \
    # 升级 pip
    && /usr/local/bin/python3 -m pip install -i https://mirrors.tencent.com/pypi/simple/ --upgrade pip \
    # 安装 Python 依赖包
    && cd .. \
    && pip3 install --no-cache-dir -r requirements_dev.txt \
    # 清理不需要的文件夹
    && rm -rf /var/lib/apt/lists/* Python-3.11.* requirements_dev.txt

# 设置CMAKE_PREFIX_PATH环境变量
ENV CMAKE_PREFIX_PATH="/usr/local/lib/python3.11/site-packages/pybind11/share/cmake/pybind11:${CMAKE_PREFIX_PATH}"

# 设置工作目录
WORKDIR /work/
CMD ["bash"]
