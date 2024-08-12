#!/bin/bash

set -e

cd "${0%/*}"

if [ -d ../txSim ]
then
  rm -rf ../txSim
fi

mkdir ../txSim
mkdir ../txSim/inc
mkdir ../txSim/lib
mkdir ../txSim/msgs
mkdir ../txSim/doc

cp ./src/txsim_* ../txSim/inc/
cp ./src/visibility.h ../txSim/inc/
cp -r ./src/api/python ../txSim/inc/

cp ./build/lib/libtxsim-module-service.a ../txSim/lib/

cp ../sim_msg/*.proto ../txSim/msgs/
rm ../txSim/msgs/moduleService.proto
rm ../txSim/msgs/sim_cloud_service.proto
rm ../txSim/msgs/sim_cloud_city_service.proto
cp ../sim_msg/generate_cpp.sh ../txSim/msgs/
cp ../sim_msg/generate_cpp.bat ../txSim/msgs/

cp -r ./examples ../txSim/

cp -r ./docs/sphinx/* ../txSim/doc/
