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

# Start compiling
build_project "carsim" "build.sh"
