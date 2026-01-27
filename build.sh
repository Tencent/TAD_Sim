#!/bin/bash
set -e

build_project() {
  project="$1"
  build_script="$2"
  echo "=== Begin build $project"
  # Save the current directory
  original_dir="$(pwd)"
  # Go to the project directory
  cd "$project" || exit
  # Calling the compilation script
  if ! bash "$build_script"; then
      echo "Error: Build failed for $project"
      exit 1
  fi
  # Return to the original directory
  cd "$original_dir" || exit
  echo "=== End build $project"
}

# ====== Setting build parmameters ======
TADSIM_ROOT="$(cd "$(dirname "$0")";pwd)"
TADSIM_BUILD="$TADSIM_ROOT/build"
TADSIM_BUILD_SERVICE="$TADSIM_BUILD/service"
TADSIM_BUILD_SCENARIO="$TADSIM_BUILD/scenario"

# Configure local Electron cache for offline/docker builds
export ELECTRON_CACHE="$TADSIM_ROOT/electron_cache"
export ELECTRON_BUILDER_CACHE="$TADSIM_ROOT/electron_builder_cache"
# Set CMAKE_PREFIX_PATH for offline dependencies (cereal, tinyxml2, etc.)
export CMAKE_PREFIX_PATH="$TADSIM_ROOT/tad_deps"
export CPLUS_INCLUDE_PATH="$TADSIM_ROOT/tad_deps/include:$TADSIM_ROOT/tad_deps/include/eigen3:$CPLUS_INCLUDE_PATH"
export LIBRARY_PATH="$TADSIM_ROOT/tad_deps/lib:$TADSIM_ROOT/tad_deps/lib64:$LIBRARY_PATH"
export PATH="$PATH:$TADSIM_ROOT/tad_deps/bin"

# ====== Clean ======
echo "=== Begin clean"
# find . -type f -iname "*.sh" -exec chmod +x {} \;
# find . -type d \( -name "build" -o -name "node_modules" \) -exec rm -rf {} + 2>/dev/null
# [ -d "$TADSIM_BUILD" ] && rm -rf "$TADSIM_BUILD" || true
echo "=== End clean (Incremental Mode: Preserving build/ directory)"

# ====== Start compiling ======
build_project "simapp" "build.sh"
build_project "common" "build.sh"
build_project "simcore" "build.sh"
build_project "co_simulation" "build.sh"
build_project "adapter" "build.sh"

# ====== pack ======
echo "=== Begin pack"

# Helper for incremental copy
sync_dir() {
    src="$1"
    dst="$2"
    if [ -d "$src" ]; then
        mkdir -p "$dst"
        # Using rsync if available for efficiency, else cp -un (update, no-clobber)
        if command -v rsync >/dev/null 2>&1; then
            rsync -a --delete "$src/" "$dst/"
        else
            cp -rf "$src/"* "$dst/"
        fi
    fi
}

# Critical Check: Verify backend binaries exist before packaging
if [ ! -f "$TADSIM_ROOT/simcore/framework/build/bin/txsim-local-service" ]; then
    echo "Error: txsim-local-service missing. Build failed."
    exit 1
fi

if [ ! -f "$TADSIM_ROOT/simcore/map_server/build/bin/txSimService" ]; then
    echo "Error: txSimService missing. Build failed."
    exit 1
fi

# 前端迁移整合
echo "=== Pack simapp"
mkdir -p "$TADSIM_BUILD"
cp -f "$TADSIM_ROOT/simapp/desktop/web-config.json" "$TADSIM_BUILD/"
cp -f "$TADSIM_ROOT/simapp/desktop/package.json" "$TADSIM_BUILD/"
cp -f "$TADSIM_ROOT/simapp/desktop/electron-builder.yml" "$TADSIM_BUILD/"

# Only copy node_modules if they have changed or are missing (to avoid slow I/O)
if [ ! -d "$TADSIM_BUILD/node_modules" ] || [ "$TADSIM_ROOT/simapp/desktop/package.json" -nt "$TADSIM_BUILD/node_modules" ]; then
    echo "Info: Syncing node_modules..."
    sync_dir "$TADSIM_ROOT/simapp/desktop/node_modules" "$TADSIM_BUILD/node_modules"
fi

sync_dir "$TADSIM_ROOT/simapp/desktop/build" "$TADSIM_BUILD/build"
mkdir -p "$TADSIM_BUILD/build/electron/map-editor"
sync_dir "$TADSIM_ROOT/simapp/scene-editor/build" "$TADSIM_BUILD/build/electron"
sync_dir "$TADSIM_ROOT/simapp/map-editor/build" "$TADSIM_BUILD/build/electron/map-editor"

# 后端与算法的迁移整合
echo "=== Pack service"
mkdir -p "$TADSIM_BUILD_SERVICE"
cp -f "$TADSIM_ROOT/simcore/framework/tools/pb_info.json" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/grading/default_grading_kpi.json" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/datas/local_service.config.in.linux" "$TADSIM_BUILD_SERVICE/local_service.config.in"
cp -f "$TADSIM_ROOT/simcore/framework/cli/cli" "$TADSIM_BUILD_SERVICE/txsim-cli"
cp -f "$TADSIM_ROOT/simcore/framework/build/bin/txsim-local-service" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/framework/build/bin/txsim-module-launcher" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/framework/build/lib/libtxsim-module-impl.so" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/framework/src/node_addon/build/Release/txsim-play-service.node" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/map_server/build/bin/txSimService" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/map_server/build/bin/libscene_wrapper.so" "$TADSIM_BUILD_SERVICE/"
cp -f "$TADSIM_ROOT/simcore/map_server/build/bin/libOpenDrivePlugin.so" "$TADSIM_BUILD_SERVICE/"

mkdir -p "$TADSIM_BUILD_SERVICE/osi"
cp -f "$TADSIM_ROOT/adapter/osi/build/bin/libtxSimOSIAdapter.so" "$TADSIM_BUILD_SERVICE/osi/"
mkdir -p "$TADSIM_BUILD_SERVICE/carsim"
cp -f "$TADSIM_ROOT/co_simulation/carsim/build/lib/libcarsim_interface.so" "$TADSIM_BUILD_SERVICE/carsim/"
mkdir -p "$TADSIM_BUILD_SERVICE/arbitrary"
cp -f "$TADSIM_ROOT/simcore/arbitrary/build/lib/libsim_arbitrary.so" "$TADSIM_BUILD_SERVICE/arbitrary/"

sync_dir "$TADSIM_ROOT/simcore/catalogs" "$TADSIM_BUILD_SERVICE/Catalogs"
cp -f "$TADSIM_ROOT/simcore/catalogs/Vehicles/default.xosc" "$TADSIM_BUILD_SERVICE/"

mkdir -p "$TADSIM_BUILD_SERVICE/envpb"
cp -f "$TADSIM_ROOT/simcore/envpb/build/lib/libenvpb.so" "$TADSIM_BUILD_SERVICE/envpb/"
mkdir -p "$TADSIM_BUILD_SERVICE/excel2asam"
if [ -f "$TADSIM_ROOT/simcore/excel2asam/build/bin/excel2asam" ]; then
    cp -f "$TADSIM_ROOT/simcore/excel2asam/build/bin/excel2asam" "$TADSIM_BUILD_SERVICE/excel2asam/"
else
    echo "Warning: excel2asam binary not found. Skipping copy."
fi
mkdir -p "$TADSIM_BUILD_SERVICE/grading"
cp -f "$TADSIM_ROOT/simcore/grading/build/bin/libtxSimGrading.so" "$TADSIM_BUILD_SERVICE/grading/"
mkdir -p "$TADSIM_BUILD_SERVICE/lanemark_detector"
cp -f "$TADSIM_ROOT/simcore/lanemark_detector/build/lib/libtxSimLaneMarkDetector.so" "$TADSIM_BUILD_SERVICE/lanemark_detector/"
mkdir -p "$TADSIM_BUILD_SERVICE/perfect_control"
cp -f "$TADSIM_ROOT/simcore/perfect_control/build/bin/libtxSimPerfectControl.so" "$TADSIM_BUILD_SERVICE/perfect_control/"
mkdir -p "$TADSIM_BUILD_SERVICE/perfect_planning"
cp -f "$TADSIM_ROOT/simcore/perfect_planning/build/bin/libtx_perfect_planning.so" "$TADSIM_BUILD_SERVICE/perfect_planning/"
mkdir -p "$TADSIM_BUILD_SERVICE/protobuf_log"
cp -f "$TADSIM_ROOT/simcore/protobuf_log/build/bin/libtxSimLogger.so" "$TADSIM_BUILD_SERVICE/protobuf_log/"
cp -f "$TADSIM_ROOT/simcore/protobuf_log/build/bin/libtxSimLogplayer.so" "$TADSIM_BUILD_SERVICE/protobuf_log/"

mkdir -p "$TADSIM_BUILD_SERVICE/data/script/"
cp -f "$TADSIM_ROOT/simcore/post_script/build/bin/"* "$TADSIM_BUILD_SERVICE/data/script/"
mkdir -p "$TADSIM_BUILD_SERVICE/imu_gps"
cp -f "$TADSIM_ROOT/simcore/sensors/imu_gps/build/lib/libimu_gps.so" "$TADSIM_BUILD_SERVICE/imu_gps/"
mkdir -p "$TADSIM_BUILD_SERVICE/radar"
cp -f "$TADSIM_ROOT/simcore/sensors/radar/build/lib/libtxsim_radar.so" "$TADSIM_BUILD_SERVICE/radar/"
mkdir -p "$TADSIM_BUILD_SERVICE/sensor_truth"
cp -f "$TADSIM_ROOT/simcore/sensors/sensor_truth/build/lib/libtxsimSensorTruth.so" "$TADSIM_BUILD_SERVICE/sensor_truth/"
mkdir -p "$TADSIM_BUILD_SERVICE/sim_label"
cp -f "$TADSIM_ROOT/simcore/sensors/sim_label/build/lib/libsim_label.so" "$TADSIM_BUILD_SERVICE/sim_label/"
if [ -f "$TADSIM_ROOT/simcore/sensors/sim_label/models.zip" ]; then
    unzip -o "$TADSIM_ROOT/simcore/sensors/sim_label/models.zip" -d "$TADSIM_BUILD_SERVICE/sim_label/"
fi
mkdir -p "$TADSIM_BUILD_SERVICE/v2x"
cp -f "$TADSIM_ROOT/simcore/sensors/v2x/build/lib/libtxsim_v2x.so" "$TADSIM_BUILD_SERVICE/v2x/"
mkdir -p "$TADSIM_BUILD_SERVICE/traffic"
cp -f "$TADSIM_ROOT/simcore/traffic/build/bin/txSimTraffic" "$TADSIM_BUILD_SERVICE/traffic/"
mkdir -p "$TADSIM_BUILD_SERVICE/vehicle_dynamics"
cp -f "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/"*.so "$TADSIM_BUILD_SERVICE/vehicle_dynamics/" 2>/dev/null || true
cp -f "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/"*.tir "$TADSIM_BUILD_SERVICE/vehicle_dynamics/" 2>/dev/null || true
cp -f "$TADSIM_ROOT/simcore/vehicle_dynamics/build/lib/"*.json "$TADSIM_BUILD_SERVICE/vehicle_dynamics/" 2>/dev/null || true

mkdir -p "$TADSIM_BUILD_SERVICE/upgrade_tools"
cp -f "$TADSIM_ROOT/simcore/map_server/build/bin/updator-scenario" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
cp -f "$TADSIM_ROOT/simcore/protobuf_log/upgrade_pblog.sh" "$TADSIM_BUILD_SERVICE/upgrade_tools/"
cp -f "$TADSIM_ROOT/simcore/framework/tools/run_upgrade.sh" "$TADSIM_BUILD_SERVICE/upgrade_tools/run.sh"
cp -f "$TADSIM_ROOT/simcore/framework/build/bin/upgrade-db" "$TADSIM_BUILD_SERVICE/upgrade_tools/"

# Optimized Dependency Scan: only run if service binaries are newer than simdeps
mkdir -p "$TADSIM_BUILD_SERVICE/simdeps/"
if [ "$TADSIM_BUILD_SERVICE/txsim-local-service" -nt "$TADSIM_BUILD_SERVICE/simdeps" ]; then
    echo "Info: Updating shared library dependencies..."
    find "$TADSIM_BUILD_SERVICE" -type f \( -executable -o -name "*.so" \) | while read -r FILE; do
      [ -x "$FILE" ] && DEPS=$(ldd "$FILE" 2>/dev/null | awk '{if ($3 != "(SD)" && $3 != "(DO)" && $3 != "") print $3}')
      [[ "$FILE" == *.so ]] && DEPS=$(ldd "$FILE" 2>/dev/null | awk '/(=>)/ {print $3}')
      for dep in $DEPS; do
        [ -f "$dep" ] && cp -uL "$dep" "$TADSIM_BUILD_SERVICE/simdeps"
      done
    done
    rm -f "$TADSIM_BUILD_SERVICE/simdeps/libc.so."* "$TADSIM_BUILD_SERVICE/simdeps/libpthread.so."* "$TADSIM_BUILD_SERVICE/simdeps/libdl.so."* \
          "$TADSIM_BUILD_SERVICE/simdeps/libresolv.so."* "$TADSIM_BUILD_SERVICE/simdeps/libm.so."* "$TADSIM_BUILD_SERVICE/simdeps/librt.so."* \
          "$TADSIM_BUILD_SERVICE/simdeps/libstdc++.so."* "$TADSIM_BUILD_SERVICE/simdeps/libterrain.so" "$TADSIM_BUILD_SERVICE/simdeps/libtx_car.so"
    mv "$TADSIM_BUILD_SERVICE/libscene_wrapper.so" "$TADSIM_BUILD_SERVICE/simdeps/" 2>/dev/null || true
    mv "$TADSIM_BUILD_SERVICE/libOpenDrivePlugin.so" "$TADSIM_BUILD_SERVICE/simdeps/" 2>/dev/null || true
    mv "$TADSIM_BUILD_SERVICE/libtxsim-module-impl.so" "$TADSIM_BUILD_SERVICE/simdeps/" 2>/dev/null || true
fi

mkdir -p "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/"
cp -u "$TADSIM_BUILD_SERVICE/simdeps/libboost_filesystem.so."* "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/" 2>/dev/null || true
cp -u "$TADSIM_BUILD_SERVICE/simdeps/libjsoncpp.so."* "$TADSIM_BUILD_SERVICE/simdeps/nodedeps/" 2>/dev/null || true

# Data resources (Standard Fix for Backend Error 2)
echo "=== Pack data"
TADSIM_BUILD_DATA="$TADSIM_BUILD/data/scenario"
sync_dir "$TADSIM_ROOT/simcore/catalogs" "$TADSIM_BUILD_DATA/Catalogs"
sync_dir "$TADSIM_ROOT/datas/default/models" "$TADSIM_BUILD_DATA/models"
# Also sync FBX models from frontend assets (Required for Sedan/sk_redcar)
sync_dir "$TADSIM_ROOT/simapp/scene-editor/src/models-manager/assets/models" "$TADSIM_BUILD_DATA/models"

# ====== Create Deployment Helper ======
echo "=== Create Deployment Helper"
cat > "$TADSIM_BUILD_SERVICE/deploy_runtime.sh" << 'EOF'
#!/bin/bash
# Initialize TAD_Sim Runtime Environment to prevent Error Code 1 and 2
TARGET_DIR="$HOME/.config/tadsim"
echo "Initializing $TARGET_DIR..."
mkdir -p "$TARGET_DIR/sys/scenario" "$TARGET_DIR/data/scenario" "$TARGET_DIR/cache/debug_log"

# Locate package source
SOURCE_DIR=$(dirname "$(readlink -f "$0")")
# In unpacked build, service is at resources/app/service
# Root directory (where tadsim is) is 3 levels up
ROOT_DIR=$(dirname $(dirname $(dirname "$SOURCE_DIR")))

# Copy catalogs to BOTH sys and data (Critical for Sedan finding fix)
echo "Syncing catalogs..."
cp -rf "$SOURCE_DIR/Catalogs" "$TARGET_DIR/sys/scenario/" 2>/dev/null || true
cp -rf "$SOURCE_DIR/Catalogs" "$TARGET_DIR/data/scenario/" 2>/dev/null || true

# Copy other templates
cp -rn "$SOURCE_DIR/../scenario/"* "$TARGET_DIR/sys/scenario/" 2>/dev/null || true

# Copy ALL models (Critical for FBX/Sedan support)
echo "Syncing 3D models..."
mkdir -p "$TARGET_DIR/data/scenario/models"
if [ -d "$SOURCE_DIR/../build/electron/assets/models" ]; then
    cp -rf "$SOURCE_DIR/../build/electron/assets/models/"* "$TARGET_DIR/data/scenario/models/" 2>/dev/null || true
fi

# Fix Library Mismatches (GCC Compatibility)
if [ -f "$SOURCE_DIR/simdeps/libgcc_s.so.1" ]; then
    echo "Fixing GCC library mismatch..."
    mv "$SOURCE_DIR/simdeps/libgcc_s.so.1" "$SOURCE_DIR/simdeps/libgcc_s.so.1.bak" 2>/dev/null || true
fi

# Patch local_service.config for Portability
CONFIG_FILE="$TARGET_DIR/sys/local_service.config"
if [ -f "$CONFIG_FILE" ]; then
    echo "Patching local_service.config with current paths..."
    # Match the fixed build-time prefix and replace with the current ROOT_DIR
    FIXED_PREFIX="/home/sunny/mds_workspace/TAD_Sim/build/release/linux-unpacked"
    sed -i "s|$FIXED_PREFIX|$ROOT_DIR|g" "$CONFIG_FILE"
fi

echo "Runtime environment initialized. You can now run tadsim."
EOF
chmod +x "$TADSIM_BUILD_SERVICE/deploy_runtime.sh"

# ====== 打包为可执行程序应用 ======
echo "=== Generate Application"
cd "$TADSIM_BUILD"
if [ ! -d "node_modules" ]; then
    npm install
fi

# 编译, 最终结果为 build/release/tadsim_x.x.x_amd64.deb
npm run release
echo "=== Build Completed Successfully ==="

# Final Step: Launch if local
if [ -d "release/linux-unpacked" ]; then
    echo "=== Creating Portable Tarball ==="
    # Ensure data directory exists in the release package (Trigger for Code 2)
    mkdir -p release/linux-unpacked/resources/data/scenario/Catalogs

    cd release/linux-unpacked
    tar -czf "../../tadsim_portable_linux_x64.tar.gz" *
    cd ../..
    echo ">>> Generated: build/tadsim_portable_linux_x64.tar.gz"
fi

# ====== 拷贝 SDK 产物 ======
echo "=== Begin copy SDK zips"
cp -rf "$TADSIM_ROOT/simcore/grading/external_eval/txSimGradingSDK.tar.gz" "$TADSIM_BUILD/release/txSimGradingSDK_linux.tar.gz" 2>/dev/null || true
cp -rf "$TADSIM_ROOT/simcore/framework/build/txSimSDK.tar.gz" "$TADSIM_BUILD/release/txSimSDK_linux.tar.gz" 2>/dev/null || true
cp -rf "$TADSIM_ROOT/common/map_sdk/hadmap.tar.gz" "$TADSIM_BUILD/release/txSimMapSDK_linux.tar.gz" 2>/dev/null || true
echo "=== End copy SDK zips"
