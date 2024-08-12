#!/bin/bash

set -e

sim_root=$1
log_file=$2/scenario_post_process.log
job_id=$3
scenario_id=$4

grading_post_process_tool=$sim_root/service_data/script/post_process


echo "------------------ job: $job_id scenario: $scenario_id: start scenario post processing ..." >> $log_file

# post processing grading report
#for pblog in $sim_root/service_data/sim_data/pblog/*.pblog; do
#    echo "***** processing grading pblog file: $pblog ..." >> $log_file
#    $grading_post_process_tool -f $pblog >> $log_file 2>&1
#    rm -f $pblog
#done

echo "------------------ scenario post processing done." >> $log_file
echo "" >> $log_file
