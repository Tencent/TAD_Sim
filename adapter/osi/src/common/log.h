// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "glog/logging.h"

#define VLOG_0 VLOG(0) << "adapter | "
#define VLOG_1 VLOG(1) << "adapter | "
#define VLOG_2 VLOG(2) << "adapter | "
#define VLOG_3 VLOG(3) << "adapter | "
#define LOG_INFO LOG_EVERY_N(INFO, 100) << "adapter | "
#define LOG_WARNING LOG_EVERY_N(WARNING, 100) << "adapter | "
#define LOG_ERROR LOG_EVERY_N(ERROR, 100) << "adapter | "

namespace adapter {}
