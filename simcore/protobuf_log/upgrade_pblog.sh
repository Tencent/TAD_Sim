#!/bin/bash

service_sys_dir=$2/sys/service_data
service_data_dir=$2/data/service_data
script_dir=$service_sys_dir/script

sim_data_dir=$service_data_dir/sim_data
pblog_dir_data=$service_data_dir/sim_data/pblog

#check and copy post_process to script_dir
if [[ ! -d $script_dir ]];then
    mkdir -p $script_dir
fi

#create sim_data_dir as needed
if [[ ! -d $sim_data_dir ]];then
    mkdir -pv $sim_data_dir
fi

if [[ ! -d $pblog_dir_data ]];then
    mkdir -pv $pblog_dir_data
fi

# copy scripts
cp -v $1/service/data/script/post_process $script_dir
cp -v $1/service/data/script/pb_save_agent $script_dir
