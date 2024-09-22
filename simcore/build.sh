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
  bash "$build_script"
  # Return to the original directory
  cd "$original_dir" || exit
  echo "=== End build $project"
}

# Start compiling
build_project "framework" "build.sh"
build_project "arbitrary" "build.sh"
build_project "envpb" "build.sh"
build_project "excel2asam" "build.sh"
build_project "grading" "build.sh"
build_project "lanemark_detector" "build.sh"
build_project "map_server" "build.sh"
build_project "traffic" "build.sh"
build_project "perfect_planning" "build.sh"
build_project "perfect_control" "build.sh"
build_project "protobuf_log" "build.sh"
build_project "sensors/imu_gps" "build.sh"
build_project "sensors/radar" "build.sh"
build_project "sensors/sensor_truth" "build.sh"
build_project "sensors/sim_label" "build.sh"
build_project "sensors/v2x" "build.sh"
build_project "post_script" "build.sh"
build_project "vehicle_dynamics" "build.sh"
