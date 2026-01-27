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
  if ! bash "$build_script"; then
      echo "Error: Build failed for $project"
      exit 1
  fi
  # Return to the original directory
  cd "$original_dir" || exit
  echo "=== End build $project"
}

# Start compiling
# Framework is the base for most, build it first
build_project "framework" "build.sh"
# Traffic is a dependency for planning modules, so build it sequentially
build_project "traffic" "build.sh"

# Parallelize the remaining independent modules
echo ">>> Starting parallel build of sub-modules..."

# Arrays to keep track of PIDs and project names
PIDs=()
PROJECTS=()

# Helper for parallel execution
p_build() {
  project="$1"
  script="$2"
  build_project "$project" "$script" &
  PIDs+=($!)
  PROJECTS+=("$project")
}

p_build "arbitrary" "build.sh"
p_build "envpb" "build.sh"
p_build "excel2asam" "build.sh"
p_build "grading" "build.sh"
p_build "lanemark_detector" "build.sh"
p_build "map_server" "build.sh"
p_build "perfect_planning" "build.sh"
p_build "perfect_control" "build.sh"
p_build "protobuf_log" "build.sh"
p_build "sensors/imu_gps" "build.sh"
p_build "sensors/radar" "build.sh"
p_build "sensors/sensor_truth" "build.sh"
p_build "sensors/sim_label" "build.sh"
p_build "sensors/v2x" "build.sh"
p_build "post_script" "build.sh"
p_build "vehicle_dynamics" "build.sh"

# Wait for all background builds to finish and check status
FAIL=0
for i in "${!PIDs[@]}"; do
  pid=${PIDs[$i]}
  proj=${PROJECTS[$i]}
  wait $pid
  if [ $? -ne 0 ]; then
    echo "Error: Build failed for $proj"
    FAIL=1
  fi
done

if [ $FAIL -ne 0 ]; then
  echo "Error: One or more parallel builds failed."
  exit 1
fi

echo ">>> Simcore parallel build finished successfully."
