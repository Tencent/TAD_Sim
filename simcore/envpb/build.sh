
set -e

# Setting build parmameters
ENVPB_ROOT="$(cd "$(dirname "$0")";pwd)"
ENVPB_BUILD="$ENVPB_ROOT/build"

# clean & mkdir
# rm -rf "$ENVPB_BUILD"
mkdir -p "$ENVPB_BUILD"

# build
cd "$ENVPB_BUILD"
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
ninja -j8

# Change the working directory back to the original directory where the script was run
cd "$ENVPB_ROOT"
