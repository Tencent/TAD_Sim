ARG BASE_MIRROR=docker.io/library/
ARG BASE_IMAGE=ubuntu:18.04
ARG BUILDPLATFORM=linux/amd64

FROM --platform=${BUILDPLATFORM} ${BASE_MIRROR}${BASE_IMAGE}
ARG TARGETPLATFORM
RUN echo "I am running on ${BUILDPLATFORM}, building for ${TARGETPLATFORM}" > /log

# Disable interactive mode
ENV DEBIAN_FRONTEND=noninteractive

# Setting Version
ARG NINJA_VERSION=1.10.2
ARG CMAKE_VERSION=3.20.1
ARG NODE_VERSION=17.9.1
ARG GRPC_VERSION=1.32.0
ARG ZEROMQ_VERSION=4.3.4
ARG CPPZMQ_VERSION=4.7.1
ARG BOOST_VERSION=1_75_0
ARG TINYXML2_VERSION=7.0.1
ARG JSONCPP_VERSION=1.9.4
ARG GOOGLETEST_VERSION=1.10.0
ARG LIBRDKAFKA_VERSION=1.3.0
ARG LOG4CPP_VERSION=1.1.3
ARG PUGIXML_VERSION=1.11.1
ARG SQLITE_VERSION=3340100
ARG SOCI_VERSION=4.0.1
ARG GLM_VERSION=0.9.9.5
ARG URIPARSER_VERSION=0.9.6
ARG GO_VERSION=1.17.8
ARG PYTHON_VERSION=3.9.4
ARG PIP_VERSION=3.9
ARG CHRONO_VERSION=8.0.0
ARG CEREAL_VERSION=1.3.0
ARG NLOHMANN_VERSION=3.11.3

#
WORKDIR /build/

# Install basic dependencies
RUN apt-get update \
    && apt-get upgrade -y \
    && apt-get install -y --no-install-recommends \
    #
    wget \
    ca-certificates \
    unzip \
    git \
    autoconf \
    automake \
    libtool \
    build-essential \
    pkg-config \
    libsasl2-dev \
    liblz4-dev \
    # building SOCI
    libsqlite3-dev \
    #
    libxml2-dev \
    libfftw3-dev \
    libtinyxml-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    lcov \
    libssl-dev \
    libcurl4-openssl-dev \
    libxerces-c-dev \
    xsdcxx \
    libtbb-dev \
    libspatialite-dev \
    # building Chrono
    libeigen3-dev \
    #
    libflann-dev \
    libproj-dev \
    libffi-dev \
    liblog4cplus-dev \
    doxygen \
    doxygen-latex \
    doxygen-doc \
    graphviz \
    locales \
    # below are the required dependencies when building python
    zlib1g-dev \
    libncurses5-dev \
    libncursesw5-dev \
    libreadline-gplv2-dev \
    libgdbm-dev \
    libdb5.3-dev \
    libbz2-dev \
    libexpat1-dev \
    liblzma-dev \
    tk-dev \
    #
    && locale-gen en_US.UTF-8 \
    && echo 'en_US.UTF-8 UTF-8' > /etc/locale.gen \
    && locale-gen \
    && echo 'LANG="en_US.UTF-8"' > /etc/locale.conf \
    #
    && apt-get autoremove -y \
    && apt-get clean -y \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Install ninja
RUN wget -O ninja-linux.zip https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/ninja-linux.zip \
    && unzip ninja-linux.zip -d /usr/local/bin \
    && rm ninja-linux.zip \
    && ninja --version \
    && rm -rf ninja*

# Install cmake
RUN wget -O cmake-${CMAKE_VERSION}-linux-x86_64.sh https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
    && sh cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/usr/local \
    && cmake --version \
    && rm cmake-${CMAKE_VERSION}-linux-x86_64.sh

# Install Node.js and npm
RUN wget -O node-v${NODE_VERSION}-linux-x64.tar.xz https://nodejs.org/download/release/v${NODE_VERSION}/node-v${NODE_VERSION}-linux-x64.tar.xz \
    && tar xf node-v${NODE_VERSION}-linux-x64.tar.xz \
    && cd node-v${NODE_VERSION}-linux-x64 \
    && rm CHANGELOG.md LICENSE README.md \
    && cp -r * /usr/local/ \
    && cd .. \
    && node --version \
    && npm config set registry https://mirrors.tencent.com/npm/ \
    && npm install -g cmake-js node-addon-api \
    && rm -rf node* \
    && mkdir -p /root/.cmake-js/node-x64/v${NODE_VERSION}/include/node \
    && cp -r /usr/local/include/node/* /root/.cmake-js/node-x64/v${NODE_VERSION}/include/node

# Install grpc and protobuf
RUN git config --global url."https://github.com/".insteadOf git://github.com/ \
    && git clone --recurse-submodules -b v${GRPC_VERSION} https://github.com/grpc/grpc \
    && mkdir -p grpc/cmake/build \
    && cd grpc/cmake/build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF ../.. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../../.. \
    && rm -rf grpc*

# Install ZeroMQ
RUN wget -O zeromq-${ZEROMQ_VERSION}.tar.gz https://github.com/zeromq/libzmq/releases/download/v${ZEROMQ_VERSION}/zeromq-${ZEROMQ_VERSION}.tar.gz \
    && tar zxf zeromq-${ZEROMQ_VERSION}.tar.gz \
    && cd zeromq-${ZEROMQ_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_CURVE=OFF -DWITH_PERF_TOOL=OFF -DZMQ_BUILD_TESTS=OFF .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf zeromq*

# Install cppzmq
RUN wget -O cppzmq-${CPPZMQ_VERSION}.tar.gz https://github.com/zeromq/cppzmq/archive/v${CPPZMQ_VERSION}.tar.gz \
    && tar zxf cppzmq-${CPPZMQ_VERSION}.tar.gz \
    && cd cppzmq-${CPPZMQ_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCPPZMQ_BUILD_TESTS=OFF .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf cppzmq*

# Install Boost
RUN wget -O boost_${BOOST_VERSION}.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_${BOOST_VERSION}.tar.bz2 \
    && tar --bzip2 -xf boost_${BOOST_VERSION}.tar.bz2 \
    && cd boost_${BOOST_VERSION} \
    && ./bootstrap.sh --prefix=/usr/local \
    && ./b2 install --with-filesystem --with-locale --with-regex --with-system \
    --with-thread --with-date_time --with-program_options \
    && cd .. \
    && rm -rf boost_*

# Install TinyXML2
RUN wget -O tinyxml2-${TINYXML2_VERSION}.tar.gz https://github.com/leethomason/tinyxml2/archive/${TINYXML2_VERSION}.tar.gz \
    && tar zxf tinyxml2-${TINYXML2_VERSION}.tar.gz \
    && cd tinyxml2-${TINYXML2_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DBUILD_SHARED_LIBS=OFF -DBUILD_TESTING=OFF .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf tinyxml2*

# Install JSONCPP
RUN wget -O jsoncpp-${JSONCPP_VERSION}.tar.gz https://github.com/open-source-parsers/jsoncpp/archive/${JSONCPP_VERSION}.tar.gz \
    && tar zxf jsoncpp-${JSONCPP_VERSION}.tar.gz \
    && cd jsoncpp-${JSONCPP_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf jsoncpp*

# Install Google Test
RUN wget -O googletest-release-${GOOGLETEST_VERSION}.tar.gz https://github.com/google/googletest/archive/release-${GOOGLETEST_VERSION}.tar.gz \
    && tar zxf googletest-release-${GOOGLETEST_VERSION}.tar.gz \
    && cd googletest-release-${GOOGLETEST_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf googletest-release*

# Install librdkafka
RUN wget -O librdkafka-${LIBRDKAFKA_VERSION}.tar.gz https://github.com/edenhill/librdkafka/archive/v${LIBRDKAFKA_VERSION}.tar.gz \
    && tar zxf librdkafka-${LIBRDKAFKA_VERSION}.tar.gz \
    && cd librdkafka-${LIBRDKAFKA_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf librdkafka*

# Install log4cpp
RUN wget -O log4cpp-${LOG4CPP_VERSION}.tar.gz https://nchc.dl.sourceforge.net/project/log4cpp/log4cpp-1.1.x%20%28new%29/log4cpp-1.1/log4cpp-${LOG4CPP_VERSION}.tar.gz \
    && tar zxf log4cpp-${LOG4CPP_VERSION}.tar.gz \
    && cd log4cpp \
    && ./configure --prefix=/usr/local \
    && make -j$(nproc) \
    && make install \
    && cd .. \
    && rm -rf log4cpp*

# Install pugixml
RUN wget -O pugixml-${PUGIXML_VERSION}.tar.gz https://github.com/zeux/pugixml/releases/download/v${PUGIXML_VERSION}/pugixml-${PUGIXML_VERSION}.tar.gz \
    && tar xf pugixml-${PUGIXML_VERSION}.tar.gz \
    && cd pugixml-${PUGIXML_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf pugixml*

# Install SQLite
RUN wget -O sqlite-amalgamation-${SQLITE_VERSION}.zip https://www.sqlite.org/2021/sqlite-amalgamation-${SQLITE_VERSION}.zip \
    && unzip sqlite-amalgamation-${SQLITE_VERSION}.zip \
    && cd sqlite-amalgamation-${SQLITE_VERSION} \
    && gcc -O2 -c -o sqlite3.o sqlite3.c \
    && ar rcs libsqlite3_34_1.a sqlite3.o \
    && mv libsqlite3_34_1.a /usr/local/lib/ \
    && cp sqlite3.h sqlite3ext.h /usr/local/include/ \
    && cd .. \
    && rm -rf sqlite*

# Install SOCI
RUN wget -O soci-${SOCI_VERSION}.tar.gz https://github.com/SOCI/soci/archive/${SOCI_VERSION}.tar.gz \
    && tar zxf soci-${SOCI_VERSION}.tar.gz \
    && cd soci-${SOCI_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DWITH_BOOST=ON -DWITH_SQLITE3=ON -DSOCI_SQLITE3=ON -DSOCI_CXX11=ON .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf soci* \
    # Creating the ld.so configuration file
    && echo /usr/local/lib64 > /etc/ld.so.conf.d/local_lib64.conf

# Install GLM
RUN wget -O glm-${GLM_VERSION}.tar.gz https://github.com/g-truc/glm/archive/${GLM_VERSION}.tar.gz \
    && tar zxf glm-${GLM_VERSION}.tar.gz \
    && cd glm-${GLM_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf glm*

# Install uriparser
RUN wget -O uriparser-${URIPARSER_VERSION}.tar.gz https://github.com/uriparser/uriparser/releases/download/uriparser-${URIPARSER_VERSION}/uriparser-${URIPARSER_VERSION}.tar.gz \
    && tar zxf uriparser-${URIPARSER_VERSION}.tar.gz \
    && cd uriparser-${URIPARSER_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DURIPARSER_BUILD_DOCS=OFF -DURIPARSER_BUILD_TESTS=OFF .. \
    && ninja -j$(nproc) \
    && ninja install \
    && cd ../.. \
    && rm -rf uriparser*

# Install Go
RUN wget -O go${GO_VERSION}.linux-amd64.tar.gz https://go.dev/dl/go${GO_VERSION}.linux-amd64.tar.gz \
    && tar -C /usr/local -xzf go${GO_VERSION}.linux-amd64.tar.gz \
    && rm go${GO_VERSION}.linux-amd64.tar.gz

# Install Python
RUN wget -O Python-${PYTHON_VERSION}.tgz https://www.python.org/ftp/python/${PYTHON_VERSION}/Python-${PYTHON_VERSION}.tgz \
    && tar xf Python-${PYTHON_VERSION}.tgz \
    && cd Python-${PYTHON_VERSION} \
    && ./configure --prefix=/usr/local --enable-shared --enable-optimizations --with-ensurepip \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && cd .. \
    && rm -fr Python-${PYTHON_VERSION}*
# config default python version
RUN ln -sf /usr/local/bin/python${PIP_VERSION} /usr/local/bin/python3 \
    && ln -sf /usr/local/bin/pip${PIP_VERSION} /usr/local/bin/pip3 \
    # config pip
    && echo "[global]\nindex-url = https://mirrors.tencent.com/pypi/simple\ntrusted-host = mirrors.cloud.tencent.com\n" > /etc/pip.conf \
    && /usr/local/bin/python${PIP_VERSION} -m pip install -i https://mirrors.tencent.com/pypi/simple/ --upgrade pip \
    && pip install -i https://mirrors.tencent.com/pypi/simple/ wheel pyinstaller breathe exhale sphinx_rtd_theme pybind11 \
    && rm -rf /root/.cache/pip

# Install Chrono
RUN wget -O chrono-${CHRONO_VERSION}.tar.gz https://github.com/projectchrono/chrono/archive/${CHRONO_VERSION}.tar.gz  \
    && tar zxf chrono-${CHRONO_VERSION}.tar.gz \
    && cd chrono-${CHRONO_VERSION} \
    && mkdir build \
    && cd build \
    && cmake -G Ninja .. \
    && ninja \
    && ninja install \
    && cd  ../.. \
    && rm -rf chrono*

# Install cereal
RUN wget -O cereal-${CEREAL_VERSION}.tar.gz https://github.com/USCiLab/cereal/archive/refs/tags/v${CEREAL_VERSION}.tar.gz \
    && tar zxf cereal-${CEREAL_VERSION}.tar.gz \
    && cd cereal-${CEREAL_VERSION} \
    && cp -r include/cereal /usr/local/include/ \
    && cd .. \
    && rm -rf cereal*

# Install nlohmann JSON
RUN wget -O json-${NLOHMANN_VERSION}.tar.gz https://github.com/nlohmann/json/archive/refs/tags/v${NLOHMANN_VERSION}.tar.gz \
    && tar zxf json-${NLOHMANN_VERSION}.tar.gz \
    && cd json-${NLOHMANN_VERSION} \
    && cp -r include/nlohmann /usr/local/include/ \
    && cd .. \
    && rm -rf json*

# Update the dynamic linker cache & Clean up
RUN ldconfig \
    && rm -rf /usr/local/share/doc /usr/local/share/man /usr/local/share/locale/*

#
ENV PATH="$PATH:/usr/local/go/bin"
ENV NODE_PATH="/usr/local/lib/node_modules"
ENV CMAKE_JS_INC="/root/.cmake-js/node-x64/v17.9.1/include/node"
ENV LANG=en_US.UTF-8 LANGUAGE=en_US:en LC_ALL=en_US.UTF-8

CMD ["bash"]

# Use docker build to build by current platform
# docker build . -t tadsim:desktop --build-arg BASE_MIRROR=ccr.ccs.tencentyun.com/library/

# Use docker buildx to build choose platform (linux/amd64 or linux/arm64)
# docker buildx build . --platform linux/amd64 -t tadsim:desktop BASE_MIRROR=ccr.ccs.tencentyun.com/library/

# Run a docker container interactively, sets it as the working directory, and starts a Bash shell.
# docker run --rm --network host --privileged -v .:/build -it tadsim:desktop /bin/bash
