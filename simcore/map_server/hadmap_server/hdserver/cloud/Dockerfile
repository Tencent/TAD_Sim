FROM csighub.tencentyun.com/simcloud/simdevtools:ubuntu18.04-buildenv4 as build

COPY . /root/src
COPY ./.gitconfig /root/.gitconfig
COPY ./.git-credentials /root/.git-credentials
RUN dos2unix /root/.git-credentials

ARG RES_DIR=/root/src/hdserver/cloud/resources
ARG DEPLOY_DIR=$RES_DIR/hdserver_deploy

WORKDIR /root/src
RUN cmake -S . -B build -DHADMAP_SVR_UPGRADE_SCENE_EVENTS=ON -DHADMAP_SVR_UPGRADE_MAP_EDITOR=ON -DLIBS_DIR:PATH=/root/linuxlibs && make -C build -j 16
RUN cd ./hdserver/cloud/src/service && /usr/local/go/bin/go build
RUN mkdir -p $DEPLOY_DIR/simdeps \
    && cp ./hdserver/cloud/libscene_wrapper.so $DEPLOY_DIR/ \
    && cp ./hdserver/cloud/src/service/service $DEPLOY_DIR/txSimService \
    && for b in $DEPLOY_DIR/*; do if [ -f $b ]; then ldd $b; fi; done | grep "=> /" | awk '{print $3}' | sort | uniq | xargs -I {} cp -v {} $DEPLOY_DIR/simdeps/ \
    && mv $DEPLOY_DIR/libscene_wrapper.so $DEPLOY_DIR/simdeps/


FROM csighub.tencentyun.com/simcloud/simdevtools:ubuntu18.04-runenv4

ARG RES_DIR=/root/src/hdserver/cloud/resources
ARG DEPLOY_DIR=$RES_DIR/hdserver_deploy

WORKDIR /home/hdserver
COPY --from=build $DEPLOY_DIR ./
COPY --from=build $RES_DIR/scenario /home/hdserver/
COPY --from=build $RES_DIR/OpenX /home/hdserver/OpenX

#环境变量设置
ENV LD_LIBRARY_PATH=/home/hdserver/simdeps
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

ENTRYPOINT ["/home/hdserver/txSimService"]
CMD ["--scenario_dir=/home/hdserver", "--serv_addr=0.0.0.0:8080", "--enable_map_syncer=0"]

EXPOSE 8080
