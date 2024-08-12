#!/bin/bash
CUR_DIR=$(dirname $(readlink -f "$0"))
SRC_DIR=${CUR_DIR}/../../

read -p "Input docker file (Default is ${CUR_DIR}/Dockerfile): " docker_file
[[ -z $docker_file ]] && { docker_file=${CUR_DIR}/Dockerfile; echo -e "\033[32m=> Using default Dockerfile.\033[0m"; }

read -p "Input tag prefix (Default is cloud): " version
[[ -z $version ]] && { version="cloud"; echo -e "\033[32m=> Using default tag prefix.\033[0m"; }

read -p "Input repo type (Default is csighub.tencentyun.com/simcloud/sim_hadmap): " docker_repo
[[ -z $docker_repo ]] && { docker_repo="csighub.tencentyun.com/simcloud/sim_hadmap"; echo -e "\033[32m=> Using default repo.\033[0m"; }

time_stamp=$(date +%Y%m%d.%H%M%S)
git_branch=$(git rev-parse --abbrev-ref HEAD|sed 's/\//_/g')
git_rev=$(git rev-parse --short HEAD)
docker_tag=$docker_repo:${version}_${git_branch}.${git_rev}.${time_stamp}

cp ~/.gitconfig ~/.git-credentials $SRC_DIR

echo -e "In ${SRC_DIR}"
echo -e "\033[32m=> Building: ${docker_tag}\033[0m"
docker build -t "$docker_tag" -f "$docker_file" $SRC_DIR

rm $SRC_DIR/.gitconfig $SRC_DIR/.git-credentials
