// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_test_gflags.h"
#include <glog/logging.h>
#include "tx_tc_marco.h"

DEFINE_string(test_app_name, "Traffic_clone", "the dll/so Name");
DEFINE_string(test_ip_addr_port, "", "rpc server location");
DEFINE_string(test_TrafficSystemOnCloudCfgPath, "./virtual_city.json", "");
DEFINE_string(test_DLLPath, "", "");
DEFINE_double(test_CloudEgoVisionRadius, 410000, "");
DEFINE_bool(LogLevel_DLLTest, false, "");

void PrintFlagsValues() {
  LOG(INFO) << TXST_FLAGS(test_app_name);
  LOG(INFO) << TXST_FLAGS(test_ip_addr_port);
  LOG(INFO) << TXST_FLAGS(test_TrafficSystemOnCloudCfgPath);
  LOG(INFO) << TXST_FLAGS(test_CloudEgoVisionRadius);
  LOG(INFO) << TXST_FLAGS(test_DLLPath);
  LOG(INFO) << TXST_FLAGS(LogLevel_DLLTest);
}
