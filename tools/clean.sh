#!/bin/bash

# ====== Setting build parmameters ======
# 获取当前脚本所在目录
TOOLS_ROOT="$(cd "$(dirname "$0")";pwd)"
TADSIM_ROOT="$(dirname "$TOOLS_ROOT")"
echo -e "TADSIM_ROOT: $TADSIM_ROOT\n"

# ====== Clean simapp ======
echo "Clean simapp start..."
rm -f "$TADSIM_ROOT/simapp/package-lock.json"
rm -rf "$TADSIM_ROOT/simapp/node_modules"
# Clean simapp desktop
rm -f "$TADSIM_ROOT/simapp/desktop/package-lock.json"
rm -rf "$TADSIM_ROOT/simapp/desktop/node_modules"
rm -rf "$TADSIM_ROOT/simapp/desktop/build"
# Clean simapp map-editor
rm -f "$TADSIM_ROOT/simapp/map-editor/package-lock.json"
rm -rf "$TADSIM_ROOT/simapp/map-editor/node_modules"
rm -rf "$TADSIM_ROOT/simapp/map-editor/build"
# Clean simapp scene-editor
rm -f "$TADSIM_ROOT/simapp/scene-editor/package-lock.json"
rm -rf "$TADSIM_ROOT/simapp/scene-editor/node_modules"
rm -rf "$TADSIM_ROOT/simapp/scene-editor/build"
echo -e "Clean simapp successfully.\n"

# ====== Clean common ======
# Clean common/map_sdk
echo "Clean common/map_sdk start..."
rm -f "$TADSIM_ROOT/common/map_sdk/hadmap.tar.gz"
rm -f "$TADSIM_ROOT/common/map_sdk/datamodel/libdatamodel.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/datamodel/build"
rm -f "$TADSIM_ROOT/common/map_sdk/mapdb/libmapdb.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/mapdb/build"
rm -f "$TADSIM_ROOT/common/map_sdk/map_import/libmapimport.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/map_import/build"
rm -f "$TADSIM_ROOT/common/map_sdk/transmission/libtransmission.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/transmission/build"
rm -f "$TADSIM_ROOT/common/map_sdk/map_engine/libmapengine.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/map_engine/build"
rm -f "$TADSIM_ROOT/common/map_sdk/route_plan/librouteplan.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/route_plan/build"
rm -f "$TADSIM_ROOT/common/map_sdk/routingmap/libroutingmap.so"
rm -rf "$TADSIM_ROOT/common/map_sdk/routingmap/build"
echo -e "Clean common/map_sdk successfully.\n"
# Clean common/message
echo "Clean common/message start..."
rm -rf "$TADSIM_ROOT/common/message/build"
echo -e "Clean common/message successfully.\n"

# ====== Clean simcore ======
# Clean simcore/arbitrary
echo "Clean simcore/arbitrary start..."
rm -rf "$TADSIM_ROOT/simcore/arbitrary/build"
echo -e "Clean simcore/arbitrary successfully.\n"
# Clean simcore/envpb
echo "Clean simcore/envpb start..."
rm -rf "$TADSIM_ROOT/simcore/envpb/build"
echo -e "Clean simcore/envpb successfully.\n"
# Clean simcore/excel2asam
echo "Clean simcore/excel2asam start..."
rm -rf "$TADSIM_ROOT/simcore/excel2asam/build"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/map/lib"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/map_sdk_py/build"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/map_sdk_py/tests/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/apis/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/generalize/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/map/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/excel2asam/openx/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/excel2asam/tests/__pycache__"
echo -e "Clean simcore/excel2asam successfully.\n"
# Clean simcore/framework
echo "Clean simcore/framework start..."
rm -f "$TADSIM_ROOT/simcore/framework/cli/cli"
rm -f "$TADSIM_ROOT/simcore/framework/cli/go.sum"
rm -rf "$TADSIM_ROOT/simcore/framework/build"
rm -rf "$TADSIM_ROOT/simcore/framework/src/node_addon/build"
rm -rf "$TADSIM_ROOT/simcore/framework/docs/api"
rm -rf "$TADSIM_ROOT/simcore/framework/docs/sphinx"
echo -e "Clean simcore/framework successfully.\n"
# Clean simcore/grading
echo "Clean simcore/grading start..."
rm -rf "$TADSIM_ROOT/simcore/grading/build"
rm -f "$TADSIM_ROOT/simcore/grading/external_eval/txSimGradingSDK.tar.gz"
echo -e "Clean simcore/grading successfully.\n"
# Clean simcore/lanemark_detector
echo "Clean simcore/lanemark_detector start..."
rm -rf "$TADSIM_ROOT/simcore/lanemark_detector/build"
echo -e "Clean simcore/lanemark_detector successfully.\n"
# Clean simcore/map_server
echo "Clean simcore/map_server start..."
rm -f "$TADSIM_ROOT/simcore/map_server/service/service"
rm -f "$TADSIM_ROOT/simcore/map_server/service/go.sum"
rm -rf "$TADSIM_ROOT/simcore/map_server/build"
rm -rf "$TADSIM_ROOT/simcore/map_server/opendrive_io/build"
rm -rf "$TADSIM_ROOT/simcore/map_server/hadmap_server/map_parser/build"
echo -e "Clean simcore/map_server successfully.\n"
# Clean simcore/perfect_control
echo "Clean simcore/perfect_control start..."
rm -rf "$TADSIM_ROOT/simcore/perfect_control/build"
echo -e "Clean simcore/perfect_control successfully.\n"
# Clean simcore/perfect_planning
echo "Clean simcore/perfect_planning start..."
rm -rf "$TADSIM_ROOT/simcore/perfect_planning/build"
echo -e "Clean simcore/perfect_planning successfully.\n"
# Clean simcore/post_script
echo "Clean simcore/post_script start..."
rm -rf "$TADSIM_ROOT/simcore/post_script/build"
rm -rf "$TADSIM_ROOT/simcore/post_script/sim_msg"
rm -rf "$TADSIM_ROOT/simcore/post_script/data_process/__pycache__"
rm -rf "$TADSIM_ROOT/simcore/post_script/tests/__pycache__"
echo -e "Clean simcore/post_script successfully.\n"
# Clean simcore/protobuf_log
echo "Clean simcore/protobuf_log start..."
rm -rf "$TADSIM_ROOT/simcore/protobuf_log/build"
echo -e "Clean simcore/protobuf_log successfully.\n"
# Clean simcore/sensors/display
echo "Clean simcore/sensors/display start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/display/build"
echo -e "Clean simcore/sensors/display successfully.\n"
# Clean simcore/sensors/imu_gps
echo "Clean simcore/sensors/imu_gps start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/imu_gps/build"
echo -e "Clean simcore/sensors/imu_gps successfully.\n"
# Clean simcore/sensors/radar
echo "Clean simcore/sensors/radar start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/radar/build"
echo -e "Clean simcore/sensors/radar successfully.\n"
# Clean simcore/sensors/sensor_truth
echo "Clean simcore/sensors/sensor_truth start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/sensor_truth/build"
echo -e "Clean simcore/sensors/sensor_truth successfully.\n"
# Clean simcore/sensors/sim_label
echo "Clean simcore/sensors/sim_label start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/sim_label/build"
echo -e "Clean simcore/sensors/sim_label successfully.\n"
# Clean simcore/sensors/v2x
echo "Clean simcore/sensors/v2x start..."
rm -rf "$TADSIM_ROOT/simcore/sensors/v2x/build"
echo -e "Clean simcore/sensors/v2x successfully.\n"
# Clean simcore/traffic
echo "Clean simcore/traffic start..."
rm -f "$TADSIM_ROOT/simcore/traffic/app/version/version.h"
rm -rf "$TADSIM_ROOT/simcore/traffic/build"
echo -e "Clean simcore/traffic successfully.\n"
# Clean simcore/vehicle_dynamics
echo "Clean simcore/vehicle_dynamics start..."
rm -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build"
echo -e "Clean simcore/vehicle_dynamics successfully.\n"

# ====== Clean co_simulation ======
# Clean co_simulation/carsim
echo "Clean co_simulation/carsim start..."
rm -rf "$TADSIM_ROOT/co_simulation/carsim/build"
echo -e "Clean co_simulation/carsim successfully.\n"

# ====== Clean adapter ======
# Clean adapter/osi
echo "Clean adapter/osi start..."
rm -rf "$TADSIM_ROOT/adapter/osi/build"
echo -e "Clean adapter/osi successfully.\n"

# ====== Clean docs ======
# Clean docs/user_guidelines
echo "Clean docs/user_guidelines start..."
rm -rf "$TADSIM_ROOT/docs/user_guidelines/build"
echo -e "Clean docs/user_guidelines successfully.\n"

# ====== Clean top build ======
echo "Clean top build..."
rm -rf "$TADSIM_ROOT/build"
echo -e "Clean top build successfully.\n"

# ====== Recovery git change by build ======
echo "Recovery git change by build start..."
git checkout -- "$TADSIM_ROOT/simapp/map-editor/package.json"
git checkout -- "$TADSIM_ROOT/simcore/map_server/service/go.mod"
git checkout -- "$TADSIM_ROOT/simcore/perfect_planning/version/version.h"
echo -e "Recovery git change by build successfully.\n"
