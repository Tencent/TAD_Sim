#!/bin/bash

# Save the directory path of the current script into 'root_dir'
root_dir=$(pwd)
# Return to the root_dir directory
cd $root_dir

# Copy necessary header files of third-party libraries
dst_dir=./deps/include/framework && mkdir -p $dst_dir && cp -r ../../framework/src/*.h $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/datamodel/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/map_engine/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/map_import/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/mapdb/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/route_plan/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/routingmap/include/* $dst_dir
dst_dir=./deps/include/map_sdk && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/transmission/include/* $dst_dir
dst_dir=./deps/include/message && mkdir -p $dst_dir && cp -r ../../../common/message/build/*.h $dst_dir

# Copy necessary lib files of third-party libraries
#dst_dir=./deps/lib/map_sdk/x64/Release && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/devel/bin/* $dst_dir
dst_dir=./deps/lib/map_sdk/x64/Release && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/hadmap/lib/libmap* $dst_dir
dst_dir=./deps/lib/map_sdk/x64/Release && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/hadmap/lib/librout* $dst_dir
dst_dir=./deps/lib/map_sdk/x64/Release && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/hadmap/lib/libtransmission.so $dst_dir
dst_dir=./deps/lib/map_sdk/x64/Release && mkdir -p $dst_dir && cp -r ../../../common/map_sdk/hadmap/lib/libdatamodel.so $dst_dir

# Copy necessary header files of txSimGrading
dst_dir=./include/actors && mkdir -p $dst_dir && cp -r ../actors/*.h $dst_dir
dst_dir=./include/eval && mkdir -p $dst_dir && cp -r ../eval/*.h $dst_dir
dst_dir=./include/factory && mkdir -p $dst_dir && cp -r ../factory/eval_factory.h $dst_dir
dst_dir=./include/factory && mkdir -p $dst_dir && cp -r ../factory/eval_factory_interface.h $dst_dir
dst_dir=./include/manager && mkdir -p $dst_dir && cp -r ../manager/*.h $dst_dir
dst_dir=./include/stateflow && mkdir -p $dst_dir && cp -r ../stateflow/*.h $dst_dir
dst_dir=./include/utils && mkdir -p $dst_dir && cp -r ../utils/*.h $dst_dir

# Copy necessary lib files of txSimGrading
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libsim_msg.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libeval_actors.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libeval_base.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libmanager.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libfactory.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libutils.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libstateflow.a $dst_dir
dst_dir=./lib/x64/Release && mkdir -p $dst_dir && cp -r ../build/bin/libeval_no_check_kpis.a $dst_dir

# Use the tar command to compress related files and package them into SDK
tar zcvf txSimGradingSDK.tar.gz --transform 's,^,txSimGradingSDK/,' \
        deps \
        include \
        lib \
        src \
        CMakeLists.txt \
        external_grading_kpi.json \
        build_linux.sh \
        Dockerfile

# Remove the temporary file folders
rm -rf deps include lib

# Change the working directory back to the original directory where the script was run
cd $root_dir
