// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/date_time/posix_time/posix_time.hpp>
#include "tx_header.h"

TX_NAMESPACE_OPEN(Utils)

/**
 * @brief 将秒转换为毫秒
 *
 * 将给定的秒值乘以 1000.0，从而得到对应的毫秒值。
 *
 * @param _s 待转换的秒值
 * @return Base::txFloat 转换后的毫秒值
 */
inline Base::txFloat SecondToMillisecond(const Base::txFloat _s) TX_NOEXCEPT { return _s * 1000.0; }

/**
 * @brief 将毫秒转换为秒
 *
 * 将给定的毫秒值除以 1000.0，从而得到对应的秒值。
 *
 * @param _ms 待转换的毫秒值
 * @return Base::txFloat 转换后的秒值
 */
inline Base::txFloat MillisecondToSecond(const Base::txFloat _ms) TX_NOEXCEPT { return _ms / 1000.0; }

/**
 * @brief 将毫秒转换为微秒
 *
 * 将给定的毫秒值乘以 1000.0，从而得到对应的微秒值。
 *
 * @param _ms 待转换的毫秒值
 * @return Base::txFloat 转换后的微秒值
 */
inline Base::txFloat MillisecondToMicrosecond(const Base::txFloat _ms) TX_NOEXCEPT { return _ms * 1000.0; }

/**
 * @brief 将微秒转换为毫秒
 * @param _ms 微秒
 * @return 返回对应的毫秒数
 */
inline Base::txFloat MicrosecondToMillisecond(const Base::txFloat _ms) TX_NOEXCEPT { return _ms / 1000.0; }

/**
 * @brief 获取当前时间戳
 * @return 当前时间戳，单位为秒
 */
inline std::time_t GetCurrentTimeStamp() TX_NOEXCEPT {
  boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
  std::time_t timestamp = boost::posix_time::to_time_t(now);
  return timestamp;
}

/**
 * @brief 获取当前时间戳，格式为 YYYYMMDDHHMMSS
 * @return 当前时间戳，格式为 YYYYMMDDHHMMSS
 */
inline Base::txString GetYYYYMMDDHHMMSS() TX_NOEXCEPT {
  std::time_t t = std::time(nullptr);
  std::tm* now = std::localtime(&t);
  /*Current Time and Date: 02-08-2018 18:42:41*/
  char buffer[128];
  strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", now);
  return Base::txString(buffer);
}

TX_NAMESPACE_CLOSE(Utils)
