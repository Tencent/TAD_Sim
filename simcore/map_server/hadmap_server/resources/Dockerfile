# 基础镜像 ubuntu 16.04
FROM docker.io/coolelite/simhdbase:v1.0

# 维护者
MAINTAINER hughhwang

RUN apt-get update && apt-get install -y curl net-tools iputils-ping s3cmd

# 拷贝文件
COPY hdserver	/home/hdserver/
COPY scenario	/home/hdserver/

# 标签
LABEL version="1.0" by="hughhwang@tencent.com"

#环境变量设置
ENV LD_LIBRARY_PATH=/home/hdserver/simdeps

ENTRYPOINT ["/home/hdserver/txSimService", "--scenario_dir=/home/hdserver", "--serv_addr=0.0.0.0:8080"] 
#ENTRYPOINT [/bin/bash]

EXPOSE 8080 
