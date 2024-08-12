// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "routingmap/routing_log.h"

#if defined(WIN32) || defined(WIN64)
#  include <direct.h>
#  define GETCWD _getcwd
#else
#  define GETCWD getcwd
#  include <unistd.h>
#endif

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

// #define ROUTING_LOG_ON
// #define ON_SCREEN

namespace hadmap {
RoutingLog* RoutingLog::ins = NULL;

RoutingLog* RoutingLog::getInstance() {
  if (ins == NULL) ins = new RoutingLog();
  return ins;
}

RoutingLog::RoutingLog() { logPath = ""; }

void RoutingLog::init(const std::string& logName) {
#ifdef ROUTING_LOG_ON

  char* buf;
  if ((buf = GETCWD(NULL, 0)) == NULL) {
    std::cout << "getcwd error" << std::endl;
  } else {
    logPath = std::string(buf) + '/' + logName;
    std::cout << "[Routing Map] logpath " << logPath << std::endl;
  }

  time_t rawtime;
  struct tm* timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  std::ofstream out;
  out.open(logPath, std::ofstream::out);
  if (out.is_open()) {
    out << asctime(timeinfo) << std::endl;
    out.close();
  }

#endif
}

RoutingLog::~RoutingLog() {}

void RoutingLog::Out(const std::string& info) {
#ifdef ROUTING_LOG_ON

#  ifdef ON_SCREEN
  std::cout << info << std::endl;
#  else
  std::ofstream out;
  out.open(logPath, std::ofstream::app);
  if (out.is_open()) {
    out << info << std::endl;
    out.close();
  }
#  endif
#endif
}

void RoutingLog::Out(const std::vector<roadpkid>& rids) {
#ifdef ROUTING_LOG_ON

#  ifdef ON_SCREEN
  std::cout << "[ ";
  for (auto id : rids) std::cout << id << " ";
  std::cout << "]" << std::endl;
#  else
  std::ofstream out;
  out.open(logPath, std::ofstream::app);
  if (out.is_open()) {
    out << "[ ";
    for (auto id : rids) out << id << " ";
    out << "]" << std::endl;
    out.close();
  }
#  endif
#endif
}

void RoutingLog::Out(const txPoint& pos) {
#ifdef ROUTING_LOG_ON

#  ifdef ON_SCREEN
  std::cout << std::setprecision(10) << pos.x << "," << pos.y << "," << pos.z << std::endl;
#  else
  std::ofstream out;
  out.open(logPath, std::ofstream::app);
  if (out.is_open()) {
    out << std::setprecision(10) << pos.x << "," << pos.y << "," << pos.z << std::endl;
    out.close();
  }
#  endif
#endif
}
}  // namespace hadmap
