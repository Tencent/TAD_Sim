# ============================================================================
# file Dockerfile
# brief Dockerfile file.
#
# author xingboliu <xingboliu@tencent.com>
# date 2024-01-17
# version 1.1.1
# copyright (c) 2023 Tencent Co. Ltd. All rights reserved.
# ============================================================================
FROM ubuntu:22.04

# 禁用交互模式
ENV DEBIAN_FRONTEND=noninteractive

WORKDIR /work/

RUN apt-get update && apt-get --no-install-recommends install -y \
    doxygen-latex doxygen-doc doxygen-gui graphviz \
    python3 python3-pip python3-venv \
    locales git zip \
    && locale-gen en_US.UTF-8 \
    && python3 -m pip install --upgrade pip \
    # 加入 exhale 后对于 Sphinx 有特定版本依赖, 和其它库导致版本冲突
    # 此处实际验证后发现, 可以正常安装后升级 Sphinx 至最新,
    # 即使不是要求的特定版本, 但是工作是正常
    && pip3 install --no-cache-dir breathe==4.35.0 \
    && pip3 install --no-cache-dir exhale==0.3.6 \
    && pip3 install --no-cache-dir myst-parser==2.0.0 \
    && pip3 install --no-cache-dir sphinx-rtd-theme==2.0.0 \
    && pip3 install --no-cache-dir sphinx-markdown-tables==0.0.17 \
    && pip3 install --no-cache-dir sphinxcontrib-mermaid==0.9.2 \
    && pip3 install --no-cache-dir Sphinx==7.2.6 \
    #
    && pip3 install --no-cache-dir loguru \
    && pip3 install --no-cache-dir pytest

ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

CMD ["bash"]
