#!/bin/bash

build_project() {
  project="$1"
  build_script="$2"
  echo "=== Begin build $project"
  # Save the current directory
  original_dir="$(pwd)"
  # Go to the project directory
  cd "$project" || exit
  # Calling the compilation script
  "./$build_script"
  # Return to the original directory
  cd "$original_dir" || exit
  echo "=== End build $project"
}

# ====== Setting build parmameters ======
TADSIM_ROOT="$(cd "$(dirname "$0")";pwd)"
TADSIM_BUILD="$TADSIM_ROOT/build"
TADSIM_BUILD_SERVICE="$TADSIM_BUILD/service"
TADSIM_BUILD_SCENARIO="$TADSIM_BUILD/scenario"

# ====== Clean ======
echo "=== Begin clean"
# find . -type f -iname "*.sh" -exec chmod +x {} \;
# find . -type d \( -name "build" -o -name "node_modules" \) -exec rm -rf {} + 2>/dev/null
[ -d "$TADSIM_BUILD" ] && rm -rf "$TADSIM_BUILD" || true
echo "=== End clean"

# ====== Start compiling ======
build_project "simapp" "build.sh"
build_project "common" "build.sh"
build_project "simcore" "build.sh"
build_project "co_simulation" "build.sh"
build_project "adapter" "build.sh"

# ====== pack ======
echo "=== Begin pack"

# 前端迁移整合
echo "=== Begin pack simapp"
mkdir -p "$TADSIM_BUILD"
cp -rf "$TADSIM_ROOT/simapp/desktop/web-config.json" "$TADSIM_BUILD/"
cp -rf "$TADSIM_ROOT/simapp/desktop/package.json" "$TADSIM_BUILD/"
cp -rf "$TADSIM_ROOT/simapp/desktop/electron-builder.yml" "$TADSIM_BUILD/"
#
mkdir -p "$TADSIM_BUILD/node_modules"
cp -rf "$TADSIM_ROOT/simapp/desktop/node_modules/"* "$TADSIM_BUILD/node_modules/"
#
mkdir -p "$TADSIM_BUILD/build"
cp -rf "$TADSIM_ROOT/simapp/desktop/build/"* "$TADSIM_BUILD/build/"
#
mkdir -p "$TADSIM_BUILD/build/electron/map-editor"
cp -rf "$TADSIM_ROOT/simapp/scene-editor/build/"* "$TADSIM_BUILD/build/electron/"
cp -rf "$TADSIM_ROOT/simapp/map-editor/build/"* "$TADSIM_BUILD/build/electron/map-editor/"
echo "=== End pack simapp"

# 后端与算法的迁移整合
echo "=== Begin pack service"
mkdir -p "$TADSIM_BUILD_SERVICE"
cp -rf "$TADSIM_ROOT/simcore/framework/tools/pb_info.json" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/grading/default_grading_kpi.json" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/datas/local_service.config.in.linux" "$TADSIM_BUILD_SERVICE/local_service.config.in"
cp -rf "$TADSIM_ROOT/simcore/framework/cli/cli" "$TADSIM_BUILD_SERVICE/txsim-cli"
cp -rf "$TADSIM_ROOT/simcore/framework/build/bin/txsim-local-service" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/framework/build/bin/txsim-module-launcher" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/framework/build/lib/libtxsim-module-impl.so" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/framework/src/node_addon/build/Release/txsim-play-service.node" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/map_server/build/bin/txSimService" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/map_server/build/bin/libscene_wrapper.so" "$TADSIM_BUILD_SERVICE/"
cp -rf "$TADSIM_ROOT/simcore/map_server/build/bin/libOpenDrivePlugin.so" "$TADSIM_BUILD_SERVICE/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/osi"
cp -rf "$TADSIM_ROOT/adapter/osi/build/bin/libtxSimOSIAdapter.so" "$TADSIM_BUILD_SERVICE/osi/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/carsim"
cp -rf "$TADSIM_ROOT/co_simulation/carsim/build/lib/libcarsim_interface.so" "$TADSIM_BUILD_SERVICE/carsim/"
mkdir -p "$TADSIM_BUILD_SERVICE/arbitrary"
#
cp -rf "$TADSIM_ROOT/simcore/arbitrary/build/lib/libsim_arbitrary.so" "$TADSIM_BUILD_SERVICE/arbitrary/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/Catalogs"
cp -rf "$TADSIM_ROOT/simcore/catalogs/"* "$TADSIM_BUILD_SERVICE/Catalogs/"
cp -rf "$TADSIM_ROOT/simcore/catalogs/Vehicles/default.xosc" "$TADSIM_BUILD_SERVICE/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/envpb"
cp -rf "$TADSIM_ROOT/simcore/envpb/build/lib/libenvpb.so" "$TADSIM_BUILD_SERVICE/envpb/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/excel2asam"
cp -rf "$TADSIM_ROOT/simcore/excel2asam/build/bin/excel2asam" "$TADSIM_BUILD_SERVICE/excel2asam/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/grading"
cp -rf "$TADSIM_ROOT/simcore/grading/build/bin/libtxSimGrading.so" "$TADSIM_BUILD_SERVICE/grading/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/lanemark_detector"
cp -rf "$TADSIM_ROOT/simcore/lanemark_detector/build/lib/libtxSimLaneMarkDetector.so" "$TADSIM_BUILD_SERVICE/lanemark_detector/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/perfect_control"
cp -rf "$TADSIM_ROOT/simcore/perfect_control/build/bin/libtxSimPerfectControl.so" "$TADSIM_BUILD_SERVICE/perfect_control/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/perfect_planning"
cp -rf "$TADSIM_ROOT/simcore/perfect_planning/build/bin/libtx_perfect_planning.so" "$TADSIM_BUILD_SERVICE/perfect_planning/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/protobuf_log"
cp -rf "$TADSIM_ROOT/simcore/protobuf_log/build/bin/libtxSimLogger.so" "$TADSIM_BUILD_SERVICE/protobuf_log/"
cp -rf "$TADSIM_ROOT/simcore/protobuf_log/build/bin/libtxSimLogplayer.so" "$TADSIM_BUILD_SERVICE/protobuf_log/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/data/script/"
cp -rf "$TADSIM_ROOT/simcore/post_script/build/bin/"* "$TADSIM_BUILD_SERVICE/data/script/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/imu_gps"
cp -rf "$TADSIM_ROOT/simcore/sensors/imu_gps/build/lib/libimu_gps.so" "$TADSIM_BUILD_SERVICE/imu_gps/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/radar"
cp -rf "$TADSIM_ROOT/simcore/sensors/radar/build/lib/libtxsim_radar.so" "$TADSIM_BUILD_SERVICE/radar/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/sensor_truth"
cp -rf "$TADSIM_ROOT/simcore/sensors/sensor_truth/build/lib/libtxsimSensorTruth.so" "$TADSIM_BUILD_SERVICE/sensor_truth/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/sim_label"
cp -rf "$TADSIM_ROOT/simcore/sensors/sim_label/build/lib/libsim_label.so" "$TADSIM_BUILD_SERVICE/sim_label/"
unzip "$TADSIM_ROOT/simcore/sensors/sim_label/models.zip" -d "$TADSIM_BUILD_SERVICE/sim_label/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/v2x"
cp -rf "$TADSIM_ROOT/simcore/sensors/v2x/build/lib/libtxsim_v2x.so" "$TADSIM_BUILD_SERVICE/v2x/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/traffic"
cp -rf "$TADSIM_ROOT/simcore/traffic/build/bin/txSimTraffic" "$TADSIM_BUILD_SERVICE/traffic/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/vehicle_dynamics"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libtx_car.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libtx_simvehicle.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libtxcar_ev.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libtxcar_hybrid.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libtxcar_ice.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/libterrain.so" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/CS_car225_60R18.tir" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/pac2002_235_60R16.tir" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/TASS_car205_60R15.tir" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar_EV.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar_Hybrid.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar_ICE.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar_template_hybrid.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar_template.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
cp -rf "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/txcar.json" "$TADSIM_BUILD_SERVICE/vehicle_dynamics/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/upgrade_tools"
cp -rf "$TADSIM_ROOT/simcore/map_server/build/bin/updator-scenario" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
cp -rf "$TADSIM_ROOT/simcore/protobuf_log/upgrade_pblog.sh" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
# cp -rf "$TADSIM_ROOT/simcore/framework/tools/upgrade_sensor" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
# cp -rf "$TADSIM_ROOT/simcore/framework/tools/upgrade_grading_report" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
# cp -rf "$TADSIM_ROOT/simcore/framework/tools/upgrade_environment" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
cp -rf "$TADSIM_ROOT/simcore/framework/tools/run_upgrade.sh" "$TADSIM_BUILD_SERVICE/upgrade_tools/run.sh"
cp -rf "$TADSIM_ROOT/simcore/framework/build/bin/upgrade-db" "$TADSIM_BUILD_SERVICE/upgrade_tools/"

# find all the dependencies and copy them to simdeps.
mkdir -p "$TADSIM_BUILD_SERVICE/simdeps/"
find "$TADSIM_BUILD_SERVICE" -type f \( -executable -o -name "*.so" \) | while read -r FILE; do
  if [ -x "$FILE" ]; then
    # Getting dependency libraries for executables
    DEPS=$(ldd "$FILE" 2>/dev/null | awk '{if ($3 != "(SD)" && $3 != "(DO)" && $3 != "") print $3}')
  elif [[ "$FILE" == *.so ]]; then
    # Getting the dependencies of a dynamic-link library
    DEPS=$(ldd "$FILE" 2>/dev/null | awk '/(=>)/ {print $3}')
  fi
  for dep in $DEPS; do
    if [ -f "$dep" ]; then
      cp -L "$dep" "$TADSIM_BUILD_SERVICE/simdeps"
    fi
  done
done
rm "$TADSIM_BUILD_SERVICE/simdeps/libc.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libpthread.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libdl.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libresolv.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libm.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/librt.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libstdc++.so."*
rm "$TADSIM_BUILD_SERVICE/simdeps/libterrain.so"
rm "$TADSIM_BUILD_SERVICE/simdeps/libtx_car.so"
mv "$TADSIM_BUILD_SERVICE/libscene_wrapper.so" "$TADSIM_BUILD_SERVICE/simdeps/"
mv "$TADSIM_BUILD_SERVICE/libOpenDrivePlugin.so" "$TADSIM_BUILD_SERVICE/simdeps/"
mv "$TADSIM_BUILD_SERVICE/libtxsim-module-impl.so" "$TADSIM_BUILD_SERVICE/simdeps/"
#
mkdir -p "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/"
cp "$TADSIM_BUILD_SERVICE/simdeps/libboost_filesystem.so."* "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/"
cp "$TADSIM_BUILD_SERVICE/simdeps/libjsoncpp.so."* "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/"
echo "=== End pack service"

# 系统默认自带 scenario 的迁移整合
echo "=== Begin pack scenario"
mkdir -p "$TADSIM_BUILD_SCENARIO"
cp -rf "$TADSIM_ROOT/datas/default/"* "$TADSIM_BUILD_SCENARIO/"
echo "=== End pack scenario"

# ====== 打包为可执行程序应用 ======
echo "=== Begin gen"
cd "$TADSIM_ROOT"
chmod 777 -R "$TADSIM_BUILD"
cd "$TADSIM_BUILD"
export electron_mirror=https://registry.npmmirror.com/-/binary/electron/
export electron_builder_binaries_mirror=https://mirrors.huaweicloud.com/electron-builder-binaries/
# # 修改版本号
# if command -v python >/dev/null 2>&1; then
#     python "$TADSIM_ROOT/tools/modify_version.py"
# else
#     # 如果 python 命令不存在，检查 python3 命令是否存在
#     if command -v python3 >/dev/null 2>&1; then
#         python3 "$TADSIM_ROOT/tools/modify_version.py"
#     else
#         # 如果 python 和 python3 命令都不存在，显示错误信息, 并不修改版本
#         echo "Warning: python & python3 not found. skip modify_version use default 2.0.0" >&2
#     fi
# fi

# 重新安装依赖, 拷贝后会有概率不能正常使用环境
npm install
# 编译, 最终结果为 build/release/tadsim_x.x.x_amd64.deb
npm run release
echo "=== End gen"

# ====== 拷贝 SDK 产物 ======
echo "=== Begin copy SDK zips"
cp -rf "$TADSIM_ROOT/simcore/grading/external_eval/txSimGradingSDK_linux.tar.gz" "$TADSIM_BUILD/release/"
cp -rf "$TADSIM_ROOT/simcore/framework/build/txSimSDK_linux.tar.gz" "$TADSIM_BUILD/release/"
cp -rf "$TADSIM_ROOT/common/map_sdk/hadmap.tar.gz" "$TADSIM_BUILD/release/txSimMapSDK_linux.tar.gz"
echo "=== End copy SDK zips"
