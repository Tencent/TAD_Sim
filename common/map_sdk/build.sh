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
  BnewPROJ="-DPROJ_NEW_API"
fi

# Setting build parmameters
MAP_SDK_ROOT="$(cd "$(dirname "$0")";pwd)"
MAP_SDK_BUILD="$MAP_SDK_ROOT/hadmap"
# Setting build parmameters DATAMODEL
DATAMODEL_ROOT="$MAP_SDK_ROOT/datamodel"
DATAMODEL_BUILD="$DATAMODEL_ROOT/build"
# Setting build parmameters MAPDB
MAPDB_ROOT="$MAP_SDK_ROOT/mapdb"
MAPDB_BUILD="$MAPDB_ROOT/build"
# Setting build parmameters MAP_IMPORT
MAP_IMPORT_ROOT="$MAP_SDK_ROOT/map_import"
MAP_IMPORT_BUILD="$MAP_IMPORT_ROOT/build"
# Setting build parmameters TRANSMISSION
TRANSMISSION_ROOT="$MAP_SDK_ROOT/transmission"
TRANSMISSION_BUILD="$TRANSMISSION_ROOT/build"
# Setting build parmameters MAP_ENGINE
MAP_ENGINE_ROOT="$MAP_SDK_ROOT/map_engine"
MAP_ENGINE_BUILD="$MAP_ENGINE_ROOT/build"
# Setting build parmameters ROUTE_PLAN
ROUTE_PLAN_ROOT="$MAP_SDK_ROOT/route_plan"
ROUTE_PLAN_BUILD="$ROUTE_PLAN_ROOT/build"
# Setting build parmameters ROUTINGMAP
ROUTINGMAP_ROOT="$MAP_SDK_ROOT/routingmap"
ROUTINGMAP_BUILD="$ROUTINGMAP_ROOT/build"
#
SDK_NAME="hadmap.tar.gz"

# Clean & mkdir
rm -rf "$MAP_SDK_BUILD"
mkdir -p "$MAP_SDK_BUILD/lib"
mkdir -p "$MAP_SDK_BUILD/include"
[ -e "$MAP_SDK_ROOT/$SDK_NAME" ] && rm "$MAP_SDK_ROOT/$SDK_NAME"
# Clean & mkdir DATAMODEL
rm -rf "$DATAMODEL_BUILD"
mkdir -p "$DATAMODEL_BUILD"
[ -e "$DATAMODEL_ROOT/libdatamodel.so" ] && rm "$DATAMODEL_ROOT/libdatamodel.so"
# Clean & mkdir MAPDB
rm -rf "$MAPDB_BUILD"
mkdir -p "$MAPDB_BUILD"
[ -e "$MAPDB_ROOT/libmapdb.so" ] && rm "$MAPDB_ROOT/libmapdb.so"
# Clean & mkdir MAP_IMPORT
rm -rf "$MAP_IMPORT_BUILD"
mkdir -p "$MAP_IMPORT_BUILD"
[ -e "$MAP_IMPORT_ROOT/libmapimport.so" ] && rm "$MAP_IMPORT_ROOT/libmapimport.so"
# Clean & mkdir TRANSMISSION
rm -rf "$TRANSMISSION_BUILD"
mkdir -p "$TRANSMISSION_BUILD"
[ -e "$TRANSMISSION_ROOT/libtransmission.so" ] && rm "$TRANSMISSION_ROOT/libtransmission.so"
# Clean & mkdir MAP_ENGINE
rm -rf "$MAP_ENGINE_BUILD"
mkdir -p "$MAP_ENGINE_BUILD"
[ -e "$MAP_ENGINE/libmapengine.so" ] && rm "$MAP_ENGINE/libmapengine.so"
# Clean & mkdir ROUTE_PLAN
rm -rf "$ROUTE_PLAN_BUILD"
mkdir -p "$ROUTE_PLAN_BUILD"
[ -e "$ROUTE_PLAN/librouteplan.so" ] && rm "$ROUTE_PLAN/librouteplan.so"
# Clean & mkdir ROUTINGMAP
rm -rf "$ROUTINGMAP_BUILD"
mkdir -p "$ROUTINGMAP_BUILD"
[ -e "$ROUTINGMAP/libroutingmap.so" ] && rm "$ROUTINGMAP/libroutingmap.so"

# build
# build datamodel
cd "$DATAMODEL_BUILD"
echo "datamodel build start..."
cmake $BuildType ..
make -j8
echo -e "datamodel build successfully.\n"

# build mapdb
cd "$MAPDB_BUILD"
echo "mapdb build start..."
cmake $BuildType ..
make -j8
echo -e "mapdb build successfully.\n"

# build map_import
cd "$MAP_IMPORT_BUILD"
echo "map_import build start..."
cmake $BuildType $BnewPROJ ..
make -j8
echo -e "map_import build successfully.\n"

# build transmission
cd "$TRANSMISSION_BUILD"
echo "transmission build start..."
cmake $BuildType ..
make -j8
echo -e "transmission build successfully.\n"

# build map_engine
cd "$MAP_ENGINE_BUILD"
echo "map_engine build start..."
cmake $BuildType ..
make -j8
echo -e "map_engine build successfully.\n"

# build route_plan
cd "$ROUTE_PLAN_BUILD"
echo "route_plan build start..."
cmake $BuildType ..
make -j8
echo -e "route_plan build successfully.\n"

# build routingmap
cd "$ROUTINGMAP_BUILD"
echo "routingmap build start..."
cmake $BuildType ..
make -j8
echo -e "routingmap build successfully.\n"

# deploy
cp -r "$DATAMODEL_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$MAPDB_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$MAP_IMPORT_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$TRANSMISSION_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$MAP_ENGINE_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$ROUTE_PLAN_ROOT/include/"* "$MAP_SDK_BUILD/include"
cp -r "$ROUTINGMAP_ROOT/include/"* "$MAP_SDK_BUILD/include"

cp "$DATAMODEL_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$MAPDB_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$MAP_IMPORT_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$TRANSMISSION_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$MAP_ENGINE_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$ROUTE_PLAN_ROOT/"*.so "$MAP_SDK_BUILD/lib"
cp "$ROUTINGMAP_ROOT/"*.so "$MAP_SDK_BUILD/lib"

cd "$MAP_SDK_BUILD/lib"
ldd libdatamodel.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapengine.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapimport.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libmapdb.so | awk '{print $3}' | xargs -i cp -L {} .
ldd librouteplan.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libroutingmap.so | awk '{print $3}' | xargs -i cp -L {} .
ldd libtransmission.so | awk '{print $3}' | xargs -i cp -L {} .

cd "$MAP_SDK_ROOT"
tar zcvf "$MAP_SDK_ROOT/$SDK_NAME" -C "$MAP_SDK_BUILD" .

# Change the working directory back to the original directory where the script was run
cd "$MAP_SDK_ROOT"
