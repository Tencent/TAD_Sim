// Copyright (c) 2016 Tencent, Inc. All Rights Reserved
// @author brbchen@tencent.com
// @date 2016/08/18
// @brief log declaration
#pragma once
#include <string.h>
#include <string>
#include "macros.h"

namespace txlog {
// Define the log level enumeration
enum LogLevel { ALL = 0, DEBUG, INFO, WARNING, ERROR, FATAL, OFF };

// Set the log level
TXSIMMAP_API void setLevel(LogLevel level);

// Log a trace message
TXSIMMAP_API void trace(const std::string& err);

// Log a debug message
TXSIMMAP_API void debug(const std::string& err);

// Log an info message
TXSIMMAP_API void info(const std::string& err);

// Log a warning message
TXSIMMAP_API void warning(const std::string& err);

// Log an error message
TXSIMMAP_API void error(const std::string& err);

// Log a fatal message
TXSIMMAP_API void fatal(const std::string& err);

// Log a formatted message
TXSIMMAP_API int printf(const char* format, ...);
}  // namespace txlog
