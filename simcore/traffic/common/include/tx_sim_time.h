// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/format.hpp>
#include "tx_marco.h"
#include "tx_type_def.h"

TX_NAMESPACE_OPEN(Base)

// @brief 时间段
class txDuration {
 public:
  txDuration() {}
  txDuration(const txFloat _l, const txFloat _r) : left_close(_l), right_open(_r) {}

  /**
   * @brief 获取时间段左闭时间
   *
   * @return txFloat
   */
  txFloat left() const TX_NOEXCEPT { return left_close; }

  /**
   * @brief 获取时间段右开时间
   *
   * @return txFloat
   */
  txFloat right() const TX_NOEXCEPT { return right_open; }

 protected:
  txFloat left_close = 0.0;
  txFloat right_open = 0.0;
};

class TimeParamManager {
 public:
#if ON_CLOUD
  TimeParamManager() : absTime_s(0.0), relativeTime_s(0.), passTime_s(0.), timeStamp_ms(0.) {}
#endif /*ON_CLOUD*/
  TimeParamManager(const txFloat _abs, const txFloat _relative, const txFloat _pass, const txFloat _time_stamp)
      : absTime_s(_abs), relativeTime_s(_relative), passTime_s(_pass), timeStamp_ms(_time_stamp) {}

  TimeParamManager(const TimeParamManager& refTimeMgr)
      : absTime_s(refTimeMgr.AbsTime()),
        relativeTime_s(refTimeMgr.RelativeTime()),
        passTime_s(refTimeMgr.PassTime()),
        timeStamp_ms(refTimeMgr.TimeStamp()) {}

  /**
   * @brief 获取事件参数管理器的格式化输出
   *
   * @return txString 格式化的时间字符串
   */
  txString str() const TX_NOEXCEPT {
    return (boost::format(" abs_s=%1%, relative_s=%2%, pass_s=%3%, time_stamp_ms=%4%") % AbsTime() % RelativeTime() %
            PassTime() % TimeStamp())
        .str();
  }

  /**
   * @brief 重写= 赋值操作
   *
   * @param refTimeMgr
   * @return TimeParamManager&
   */
  TimeParamManager& operator=(const TimeParamManager& refTimeMgr) TX_NOEXCEPT {
    absTime_s = (refTimeMgr.AbsTime());
    relativeTime_s = (refTimeMgr.RelativeTime());
    passTime_s = (refTimeMgr.PassTime());
    timeStamp_ms = (refTimeMgr.TimeStamp());
    return *this;
  }

  /**
   * @brief 获取绝对时间
   *
   * @return const txFloat
   */
  const txFloat AbsTime() const TX_NOEXCEPT { return absTime_s; }

  /**
   * @brief 获取相对时间
   *
   * @return const txFloat
   */
  const txFloat RelativeTime() const TX_NOEXCEPT { return relativeTime_s; }

  /**
   * @brief 获取已经经过的时长
   *
   * @return const txFloat
   */
  const txFloat PassTime() const TX_NOEXCEPT { return passTime_s; }

  /**
   * @brief 获取当前时间戳
   *
   * @return const txFloat
   */
  const txFloat TimeStamp() const TX_NOEXCEPT { return timeStamp_ms; }

  friend std::ostream& operator<<(std::ostream& os, const TimeParamManager& v) TX_NOEXCEPT {
    os << v.str();
    return os;
  }

  /**
   * @brief 工具时间戳转ms
   *
   * @param sec 秒数时间戳
   * @return txFloat
   */
  static txFloat Second2TimeStamp(const txFloat sec) TX_NOEXCEPT { return sec * 1000.0; }

 protected:
  /*[simulation_time] :  abs = 0.02, relative = 0.02, pass = 0.02, time_stamp = 20 time_stamp = 20,*/
  txFloat absTime_s = 0.0;
  txFloat relativeTime_s = 0.0;
  txFloat passTime_s = 0.0;
  txFloat timeStamp_ms = 0.0;
};
TX_NAMESPACE_CLOSE(Base)
