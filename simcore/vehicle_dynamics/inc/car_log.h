// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

/* always include this header in a cpp file */

#define _TXCAR_USE_GLOG_ 1

#ifdef _TXCAR_USE_GLOG_
#  define GLOG_USE_GLOG_EXPORT
#  include "glog/logging.h"

#  define LOG_0 VLOG(0)
#  define LOG_1 VLOG(1)
#  define LOG_2 VLOG(2)
#  define LOG_3 VLOG(3)
#  define LOG_ERROR LOG(ERROR)
#  define LOG_INFO LOG(INFO)
#  define LOG_WARNING LOG(WARNING)
#  define LOG_DEBUG DLOG(INFO)
#  define LOG_WARNING_Every_100 LOG_EVERY_N(WARNING, 100)
#else
#  include <iostream>

#  define LOG_0 std::cout
#  define LOG_1 std::cout
#  define LOG_2 std::cout
#  define LOG_3 std::cout
#  define LOG_ERROR std::cerr
#  define LOG_INFO std::cout
#  define LOG_WARNING std::cerr
#  define LOG_DEBUG std::cout
#  define LOG_WARNING_Every_100 std::cerr
#endif
