FROM csighub.tencentyun.com/simcloud/simdevtools:ubuntu18.04-runenv4

# 拷贝文件
COPY hdserver_deploy /home/hdserver/
COPY scenario /home/hdserver/
COPY OpenX /home/hdserver/OpenX

# 标签

#环境变量设置
ENV LD_LIBRARY_PATH=/home/hdserver/simdeps
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

ENTRYPOINT ["/home/hdserver/txSimService"]
CMD ["--scenario_dir=/home/hdserver", "--serv_addr=0.0.0.0:8080", "--enable_map_syncer=0"]

EXPOSE 8080 
