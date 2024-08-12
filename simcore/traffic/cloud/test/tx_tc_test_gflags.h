// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <gflags/gflags.h>

DECLARE_string(test_app_name);
DECLARE_string(test_ip_addr_port);
DECLARE_string(test_TrafficSystemOnCloudCfgPath);
DECLARE_string(test_DLLPath);
DECLARE_double(test_CloudEgoVisionRadius);
DECLARE_bool(LogLevel_DLLTest);

extern void PrintFlagsValues();
