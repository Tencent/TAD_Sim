// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_time_utils.h"
#include "tx_billboard_flags.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class LaneInfoAverSpeed {
 public:
  struct SliceInfo {
    void Init(const Base::txFloat _pass_time_s) {
      pass_time_s = _pass_time_s;
      pass_time_ms = Utils::SecondToMillisecond(pass_time_s);
      sum_speed_ms = 0.0;
      sum_veh_cnt = 0;
    }

    Base::txFloat pass_time_s = 0.0;
    Base::txInt pass_time_ms = 0.0;
    Base::txFloat sum_speed_ms = 0.0;
    Base::txInt sum_veh_cnt = 0;
  };

 public:
  LaneInfoAverSpeed() TX_DEFAULT;
  ~LaneInfoAverSpeed() TX_DEFAULT;

 public:
  void ClearOldInfo(const Base::txFloat pass_time_s,
                    const Base::txInt buffer_time_ms /*5min = 5 * 60s*1000 = 300000*/) TX_NOEXCEPT;
  void RegisterVehInfo(const Base::txFloat pass_time_s, const Base::txInt id, const Base::txFloat speed_ms) TX_NOEXCEPT;
  Base::txInt VehicleCountInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT;
  Base::txInt VehicleGroupByInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT;
  void ComputeAverSpeedMS(const Base::txFloat pass_time_s) TX_NOEXCEPT;
  Base::txFloat AverSpeedMSInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_aver_speed_ms; }
  Base::txFloat SumSpeedMSInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_sum_speed_ms; }
  Base::txInt SumVehCntInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_sum_veh_cnt; }

 protected:
  std::list<SliceInfo> list_info;
  Base::txFloat cur_aver_speed_ms = 0.0;
  Base::txInt cur_sum_veh_cnt = 0;
  Base::txFloat cur_sum_speed_ms = 0.0;
  tbb::concurrent_unordered_set<Base::txInt> veh_id_set;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
