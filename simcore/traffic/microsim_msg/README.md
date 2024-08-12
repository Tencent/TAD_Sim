# message
提供中微观一体化实时在线仿真标准数据结构

### 一、安装Google Protocol Buffers
1. 版本: v3.14.0
2. 下载地址：https://github.com/protocolbuffers/protobuf/releases
3. 安装指南, 以Ubuntu 16.04为例
* sudo apt-get install autoconf automake libtool curl make g++ unzip
* cd protobuf-3.14.0
* sh ./autogen.sh
* ./configure -prefix=安装目录 (如:/usr/local/)
* sudo make
* sudo make check
* sudo make install
* sudo ldconfig
* protoc --version

### 二、编译自定义proto文件
1. 参照its_sim.proto编写proto文件
2. 修改Makefile
3. make clean; make
