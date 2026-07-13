#!/bin/bash
set -e

# Setting build parmameters
MAP_SERVER_ROOT="$(cd "$(dirname "$0")";pwd)"
MAP_SERVER_BUILD="$MAP_SERVER_ROOT/build"
# Setting build parmameters OPENDRIVE_IO
OPENDRIVE_IO_ROOT="$MAP_SERVER_ROOT/opendrive_io"
OPENDRIVE_IO_BUILD="$OPENDRIVE_IO_ROOT/build"
# Setting build parmameters MAP_PARSER
MAP_PARSER_ROOT="$MAP_SERVER_ROOT/hadmap_server/map_parser"
MAP_PARSER_BUILD="$MAP_PARSER_ROOT/build"
# Setting build parmameters SERVER
SERVER_ROOT="$MAP_SERVER_ROOT/service"

# Clean & mkdir
rm -rf "$MAP_SERVER_BUILD"
mkdir -p "$MAP_SERVER_BUILD/bin"
# Clean & mkdir OPENDRIVE_IO
rm -rf "$OPENDRIVE_IO_BUILD"
mkdir -p "$OPENDRIVE_IO_BUILD"
# Clean & mkdir MAPPARSER
rm -rf "$MAP_PARSER_BUILD"
mkdir -p "$MAP_PARSER_BUILD"
# Clean & mkdir service
[ -e "$SERVER_ROOT/go.sum" ] && rm "$SERVER_ROOT/go.sum"
[ -e "$SERVER_ROOT/service" ] && rm -r "$SERVER_ROOT/service"

# build
# build opendriveio
cd "$OPENDRIVE_IO_BUILD"
echo "opendrive_io build start..."
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
# deploy opendriveio
cp -rf "$OPENDRIVE_IO_BUILD/bin/"* "$MAP_SERVER_BUILD/bin/"
echo -e "opendrive_io build successfully.\n"

# build map_parser
cd "$MAP_PARSER_BUILD"
echo "map_parser build start..."
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j8
# deploy map_parser
cp -rf "$MAP_PARSER_BUILD/bin/"* "$MAP_SERVER_BUILD/bin/"
cp -rf "$MAP_PARSER_BUILD/lib/"* "$MAP_SERVER_BUILD/bin/"
echo -e "map_parser build successfully.\n"

# build server (txSimService)
cd "$SERVER_ROOT"
echo "service build start..."

# Use a reliable multi-mirror proxy chain so Go never falls back to a direct
# `cloud.google.com/go?go-get=1` fetch (which 429s under rate limiting).
# `goproxy.cn` (Alibaba) is the most reliable mirror on CN networks; the other
# mirrors are fallbacks before `direct`.
# GOSUMDB=off skips the checksum-database lookup for extra reliability.
go env -w GOPROXY=https://goproxy.cn,https://goproxy.io,https://proxy.golang.org,direct
go env -w GOSUMDB=off
go env -w GOFLAGS=-mod=mod

# Retry wrapper to ride out intermittent 429 / network errors.
go_with_retry() {
  local attempt=1
  until "$@"; do
    if [ "$attempt" -ge 5 ]; then
      echo "ERROR: go command failed after $attempt attempts: $*" >&2
      return 1
    fi
    echo "go command failed (attempt $attempt), retrying in $((attempt*2))s..."
    sleep $((attempt*2))
    attempt=$((attempt+1))
  done
}

go_with_retry go mod tidy
go_with_retry go build
# deploy server
cp "$SERVER_ROOT/service" "$MAP_SERVER_BUILD/bin/txSimService"
echo -e "service build successfully.\n"

# Change the working directory back to the original directory where the script was run
cd "$MAP_SERVER_ROOT"
