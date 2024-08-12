// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/log.h"
#include <cstdarg>
#include <cstdio>
#include <iostream>

namespace txlog {

static int log_level = WARNING;

void setLevel(LogLevel level) { log_level = level; }

void trace(const std::string& err) {
  if (log_level <= ALL) std::cout << "[TRACE] " << err << std::endl;
}

void debug(const std::string& err) {
  if (log_level <= DEBUG) std::cout << "[DEBUG] " << err << std::endl;
}

void info(const std::string& err) {
  if (log_level <= INFO) std::cout << "[INFO] " << err << std::endl;
}

void warning(const std::string& err) {
  if (log_level <= WARNING) std::cout << "[WARNING] " << err << std::endl;
}

void error(const std::string& err) {
  if (log_level <= ERROR) std::cout << "[ERROR] " << err << std::endl;
}

void fatal(const std::string& err) {
  if (log_level <= FATAL) std::cout << "[FATAL] " << err << std::endl;
}

int printf(const char* format, ...) {
  constexpr int max_size = 10240;
  static char buffer[max_size] = {0};
  memset(buffer, 0, max_size);

  va_list va;
  va_start(va, format);
  int ret = vsnprintf(buffer, max_size, format, va);
  va_end(va);
  std::cout << buffer;
  return ret;
}
}  // namespace txlog
