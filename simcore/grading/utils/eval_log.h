// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "glog/logging.h"

// Define different levels of logging macros
#define VLOG_0 VLOG(0) << "eval | "
#define VLOG_1 VLOG(1) << "eval | "
#define VLOG_2 VLOG(2) << "eval | "
#define VLOG_3 VLOG(3) << "eval | "

// Define conditional logging macros. Output one message every 100 times it appears
#define LOG_INFO LOG_EVERY_N(INFO, 100) << "eval | "
#define LOG_WARNING LOG_EVERY_N(WARNING, 100) << "eval | "
#define LOG_ERROR LOG_EVERY_N(ERROR, 100) << "eval | "

namespace eval {}
