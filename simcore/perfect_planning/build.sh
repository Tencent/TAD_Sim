
set -e

# 1 traffic > perfect planning
echo "Please first build traffic"

# Setting build parmameters
PERFECT_PLANNING_ROOT="$(cd "$(dirname "$0")";pwd)"
PERFECT_PLANNING_BUILD="$PERFECT_PLANNING_ROOT/build"

# build
# rm -rf "$PERFECT_PLANNING_BUILD"
mkdir -p "$PERFECT_PLANNING_BUILD"

cd "$PERFECT_PLANNING_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$PERFECT_PLANNING_ROOT"
