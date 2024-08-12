# 1、安装docker后，执行以下命令构建镜像
# docker build -t tx_sim_grading_sdk-build_env:v1.0.0 .
# 2、从镜像构建容器并运行
# docker run -it --user=root --name tx_sim_grading_sdk -v (被挂载的sdk目录:镜像内目录) --net=host tx_sim_grading_sdk-build_env:v1.0.0 /bin/bash
# 3、退出后，执行docker start tx_sim_grading_sdk && docker exec -it tx_sim_grading_sdk /bin/bash可重新进入
# 4、（若要删除镜像和重新构建容器）docker stop tx_sim_grading_sdk && docker rm -f tx_sim_grading_sdk，再执行第2步

FROM docker.io/ubuntu:16.04

RUN apt update && apt install -y --no-install-recommends \
  wget ca-certificates unzip git autoconf automake libtool make g++ unzip build-essential pkg-config curl tzdata

RUN ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
  echo 'Asia/Shanghai' > /etc/timezone && dpkg-reconfigure -f noninteractive tzdata

WORKDIR /root/build

# speed up the building via ninja
RUN wget https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip \
  && unzip ninja-linux.zip -d /usr/local/bin && rm ninja-linux.zip && ninja --version

# cmake 3.20.1
RUN wget -O cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.20.1/cmake-3.20.1-Linux-x86_64.sh \
  && sh cmake-linux.sh -- --skip-license --prefix=/usr/local \
  && rm cmake-linux.sh && cmake --version

# protobuf v3.12.2
RUN wget https://github.com/protocolbuffers/protobuf/releases/download/v3.12.2/protobuf-all-3.12.2.tar.gz \
  && tar -xzvf protobuf-all-3.12.2.tar.gz && cd protobuf-3.12.2 \
  && ./configure --disable-shared CXXFLAGS="-fPIC" && make -j8 && make install \
  && cd .. && rm -rf protobuf-*

# jsoncpp v1.9.4  
RUN wget -O jsoncpp-1.9.4.tar.gz https://github.com/open-source-parsers/jsoncpp/archive/1.9.4.tar.gz \
  && tar zxf jsoncpp-1.9.4.tar.gz && cd jsoncpp-1.9.4 && mkdir build && cd build \
  && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
           -DJSONCPP_WITH_TESTS=OFF -DJSONCPP_WITH_POST_BUILD_UNITTEST=OFF .. \
  && ninja && ninja install \
  && cd ../.. && rm -rf jsoncpp-1.9.4*

# pugixml v1.11.1
RUN wget https://github.com/zeux/pugixml/releases/download/v1.11.1/pugixml-1.11.1.tar.gz \
  && tar xf pugixml-1.11.1.tar.gz && cd pugixml-1.11.1 && mkdir build && cd build \
  && cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local .. \
  && ninja && ninja install \
  && cd ../.. && rm -rf pugixml*

# boost v1.75.0
RUN wget -O boost_1_75_0.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.bz2 \
  && tar --bzip2 -xf boost_1_75_0.tar.bz2 && cd boost_1_75_0 \
  && ./bootstrap.sh --prefix=/usr/local \
  && ./b2 install --with-filesystem --with-locale --with-regex --with-system --with-thread --with-date_time \
                  --with-program_options \
  && cd .. && rm -rf boost_*

RUN ldconfig \
  && apt update && apt install -y --no-install-recommends \
     libgflags-dev libgoogle-glog-dev libcurl4-openssl-dev \
     libspatialite-dev libeigen3-dev libsqlite3-dev \
  && apt autoremove -y && apt-get clean -y && rm -rf /var/lib/apt/lists/*

# adding soci install dir into runtime ld search path.
RUN echo /usr/local/lib64 > /etc/ld.so.conf.d/local_lib64.conf && ldconfig

CMD ["bash"]
