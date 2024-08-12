#!/bin/bash
ulimit -c unlimited

set -x

root="${0%/*}"

# make -C ${root}/hdserver/build -j 12
if [ -d "/home/hdserver" ]; then
    rm -rf /home/hdserver
fi
mkdir -p /home/hdserver

sh ${root}/resources/deploy_hdserver.sh
cp -r ${root}/resources/hdserver_deploy/* /home/hdserver
cp -r ${root}/resources/scenario/* /home/hdserver
cp -r ${root}/resources/OpenX /home/hdserver
cp ${root}/src/service/service /home/hdserver/txSimService

export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/home/hdserver/simdeps

kill -9 $(ps aux |grep /home/hdserver/txSimService |grep -v grep |awk '{print $2}')
# /home/hdserver/txSimService --scenario_dir=/home/hdserver --serv_addr=0.0.0.0:8080 --enable_map_syncer=0
# valgrind --tool=memcheck --leak-check=full --log-file=memcheck.txt 
