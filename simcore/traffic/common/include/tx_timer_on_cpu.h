// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <chrono>
#include <cmath>
#include <ctime>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Base)

class TimingCPU {
 private:
  std::chrono::time_point<std::chrono::system_clock> m_StartTime;

 public:
  TimingCPU() TX_DEFAULT;
  ~TimingCPU() TX_DEFAULT;
  TimingCPU(const TimingCPU&) TX_DELETE;
  TimingCPU& operator=(const TimingCPU&) TX_DELETE;

 public:
  /**
   * @brief 启动计数器
   *
   * 此函数用于启动计时器，并记录当前系统时间作为计时器的起始时间。
   *
   */
  void StartCounter() TX_NOEXCEPT { m_StartTime = std::chrono::system_clock::now(); }

  /**
   * @brief 获取已经过去的微秒数
   *
   * 此函数用于获取自 m_StartTime 以来的已经过去的微秒数。
   *
   * @return int64_t 返回已经过去的微秒数
   */
  int64_t GetElapsedMicroseconds() TX_NOEXCEPT {
    std::chrono::time_point<std::chrono::system_clock> lastTime = m_StartTime;
    m_StartTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(m_StartTime - lastTime).count();
  }

  /**
   * @brief 获取已经过去的毫秒数
   *
   * 此函数用于获取自 m_StartTime 以来的已经过去的毫秒数。
   *
   * @return int64_t 返回已经过去的毫秒数
   */
  int64_t GetElapsedMilliseconds() TX_NOEXCEPT {
    std::chrono::time_point<std::chrono::system_clock> lastTime = m_StartTime;
    m_StartTime = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(m_StartTime - lastTime).count();
  }

  /**
   * @brief 获取当前时间的时间戳
   *
   * 此函数用于获取当前时间的时间戳。
   *
   * @return int64_t 返回当前时间的时间戳
   */
  static int64_t GetCurrentTimeCount() TX_NOEXCEPT {
    return std::chrono::system_clock::now().time_since_epoch().count();
  }
};  // TimingCPU class

TX_NAMESPACE_CLOSE(Base)
