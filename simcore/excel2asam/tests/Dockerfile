# 第一阶段：构建 Python 3.11.5 和 Allure 2.24.0
FROM ubuntu:22.04 as builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get -y update --fix-missing \
    && apt-get --no-install-recommends install -y build-essential \
    wget ca-certificates \
    liblzma-dev libffi-dev libssl-dev libbz2-dev zlib1g-dev \
    # install python
    && wget --no-check-certificate https://www.python.org/ftp/python/3.11.5/Python-3.11.5.tgz \
    && tar -zxvf Python-3.11.5.tgz \
    && cd Python-3.11.5 \
    && ./configure --prefix=/usr/local --enable-shared LDFLAGS="-Wl,-rpath /usr/local/lib" \
    && make -j12 \
    && make install \
    && cd .. \
    && /usr/local/bin/python3 -m pip install --upgrade pip \
    && pip3 install --no-cache-dir pytest pytest-html pytest-xdist allure-pytest xmlschema \
    # install allure
    && wget --no-check-certificate https://repo.maven.apache.org/maven2/io/qameta/allure/allure-commandline/2.24.0/allure-commandline-2.24.0.tgz \
    && tar -zxvf allure-commandline-2.24.0.tgz \
    && ln -s /allure-2.24.0/bin/allure /usr/local/bin/allure \
    # clean
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# 第二阶段：安装库和设置工作目录
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

COPY --from=builder /usr/local /usr/local
COPY --from=builder /allure-2.24.0 /allure-2.24.0

RUN apt-get -y update --fix-missing \
    && apt-get --no-install-recommends install -y \
    git curl cmake libgoogle-glog-dev p7zip-full protobuf-compiler libprotobuf-dev \
    vim libproj-dev libspatialite-dev libcurl4-openssl-dev \
    libncurses-dev libgdbm-dev libz-dev tk-dev libsqlite3-dev \
    libreadline-dev \
    # allure needed
    openjdk-11-jre-headless \
    # hadmap build
    libboost-dev libeigen3-dev \
    # set allure
    && echo 'export PATH="/allure-2.24.0/bin:$PATH"' >> ~/.bashrc \
    # set JAVA_HOME ENV
    # && echo 'export PATH="/usr/lib/jvm/java-11-openjdk-amd64/bin:$PATH"' >> ~/.bashrc \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work/
CMD ["bash"]