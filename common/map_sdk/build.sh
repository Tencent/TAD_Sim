#!/bin/bash
BuildType=
BnewPROJ=

for i in "$@"
do
  if [ "$i" = "-DEBUG" ]; then
    BuildType="-DCMAKE_BUILD_TYPE=debug"
  fi
done

if [ "$PROJ_NEW_API" = "TRUE" ]; then
  BnewPROJ="-DPROJ_NEW语API"
fi

cd datamodel
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..

cd mapdb
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..

cd map_import
mkdir build
cd build
cmake $BuildType $BnewPROJ ..
make -j
cd ../..

cd transmission
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..

cd map_engine
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..

cd route_plan
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..

cd routingmap
mkdir build
cd build
cmake $BuildType ..
make -j
cd ../..


mkdir hadmap
mkdir hadmap/lib
mkdir hadmap/include

cp -r ./datamodel/include/* ./hadmap/include
cp -r ./map_engine/include/* ./hadmap/include
cp -r ./map_import/include/* ./hadmap/include
cp -r ./mapdb/include/* ./hadmap/include
cp -r ./route_plan/include/* ./hadmap/include
cp -r ./routingmap/include/* ./hadmap/include
cp -r ./transmission/include/* ./hadmap/include

cp ./datamodel/*.so ./hadmap/lib
cp ./map_engine/*.so ./hadmap/lib
cp ./map_import/*.so ./hadmap/lib
cp ./mapdb/*.so ./hadmap/lib
cp ./route_plan/*.so ./hadmap/lib
cp ./routingmap/*.so ./hadmap/lib
cp ./transmission/*.so ./hadmap/lib

cd hadmap/lib
ldd libdatamodel.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapengine.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapimport.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapdb.so | awk '{print $3}' | xargs -i cp -L {} .
ldd librouteplan.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libroutingmap.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libtransmission.so | awk '{print $3}' | xargs -i cp -L {} .

cd ../..
tar -czf hadmap.tar.gz ./hadmap
