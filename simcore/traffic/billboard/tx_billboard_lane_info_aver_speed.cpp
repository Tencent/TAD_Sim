// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_lane_info_aver_speed.h"
#include "tx_math.h"
#include "tx_time_utils.h"

TX_NAMESPACE_OPEN(TrafficFlow)

void LaneInfoAverSpeed::ClearOldInfo(const Base::txFloat pass_time_s,
                                     const Base::txInt buffer_time_ms /*5min = 5 * 60s*1000 = 300000*/) TX_NOEXCEPT {
  if (buffer_time_ms > 0) {
    cur_aver_speed_ms = 0.0;
    cur_sum_veh_cnt = 0;
    cur_sum_speed_ms = 0.0;
    const Base::txFloat currentPassTime_ms = Utils::SecondToMillisecond(pass_time_s);
    for (auto itr = std::begin(list_info); itr != std::end(list_info);) {
      const auto& refSlice = *itr;
      if ((currentPassTime_ms - refSlice.pass_time_ms) >= buffer_time_ms) {
        itr = list_info.erase(itr);
      } else {
        break;
      }
    }
    // LOG(WARNING) << TX_VARS(currentPassTime_ms) << TX_VARS(buffer_time_ms) <<TX_VARS(int(currentPassTime_ms)) <<
    // TX_VARS(int(currentPassTime_ms) % buffer_time_ms);

    if ((static_cast<int>(currentPassTime_ms) % buffer_time_ms) < 20) {
      veh_id_set.clear();
      LOG(WARNING) << "veh_id_set clear()";
    }
  }
}

void LaneInfoAverSpeed::RegisterVehInfo(const Base::txFloat pass_time_s, const Base::txInt id,
                                        const Base::txFloat speed_ms) TX_NOEXCEPT {
  if ((list_info).empty()) {
    /*empty*/
    SliceInfo newNode;
    newNode.Init(pass_time_s);
    list_info.emplace_back(newNode);
  } else {
    if (CallFail(Math::isEqual(list_info.back().pass_time_s, pass_time_s))) {
      SliceInfo newNode;
      newNode.Init(pass_time_s);
      list_info.emplace_back(newNode);
    }
  }
  list_info.back().sum_speed_ms += speed_ms;
  list_info.back().sum_veh_cnt += 1;
  if (FLAGS_compute_vehicle_cnt) {
    veh_id_set.insert(id);
  }
}

Base::txInt LaneInfoAverSpeed::VehicleCountInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT {
  Base::txInt res = 0;
  for (auto itr = std::begin(list_info); itr != std::end(list_info); ++itr) {
    res += (*itr).sum_veh_cnt;
  }
  return res;
}

Base::txInt LaneInfoAverSpeed::VehicleGroupByInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT {
  // tbb::concurrent_unordered_set<Base::txInt> veh_id_set_all;
  // for (auto itr = std::begin(list_info); itr != std::end(list_info); ++itr)
  // {
  //  for (const auto &elem : (*itr).veh_id_set)
  //  {
  //    veh_id_set_all.insert(elem);
  //  }
  // }
  // return veh_id_set_all.size();
  return veh_id_set.size();
}

void LaneInfoAverSpeed::ComputeAverSpeedMS(const Base::txFloat pass_time_s) TX_NOEXCEPT {
  cur_aver_speed_ms = 0.0;
  cur_sum_veh_cnt = 0;
  cur_sum_speed_ms = 0.0;

  for (auto itr = std::begin(list_info); itr != std::end(list_info); ++itr) {
    cur_sum_veh_cnt += (*itr).sum_veh_cnt;
    cur_sum_speed_ms += (*itr).sum_speed_ms;
  }
  if (cur_sum_veh_cnt > 0) {
    cur_aver_speed_ms = cur_sum_speed_ms / cur_sum_veh_cnt;
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
