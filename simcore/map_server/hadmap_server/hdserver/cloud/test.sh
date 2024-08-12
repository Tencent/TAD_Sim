#!/bin/bash

export LD_LIBRARY_PATH=$(pwd)/hdserver
export GOPATH=$GOPATH:$(pwd):~/go:/usr/local/go

function run_rest_test() {
    old_dir=$(pwd)

    echo -e "\n\nrun rest test"
    dir=src/service
    cd $dir

    echo $GOPATH

    go test -v sim_service_test.go
    #go test -v rest_api_test.go -args ../../../cfg/sim_data_service.yaml ../../../cfg/config_example.conf #--config=../../../cfg/config_example.conf
    #go test -v gorm_proxy_test.go -args ../../../cfg/sim_data_service.yaml ../../../cfg/config_example.conf
    cd $old_dir
}

function run_https_test() {
    old_dir=$(pwd)

    echo -e "\n\nrun https test"
    dir=src/service
    cd $dir

    echo $GOPATH

    go test -v rest/https_test.go
    
    cd $old_dir
}

function run_all_test() {
    run_rest_test
}

echo "gopath: ", $GOPATH

if [ $# -eq 0 ]; then
    run_all_test
elif [ "$1" = "https" ]; then
    run_https_test
elif [ "$1" = "rest" ]; then
    run_rest_test
else
    echo "unknown test target: ", $1
fi
