// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_sim_point.h"
#include "tx_billboard_flags.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class Billboard_Hwy {
 public:
  struct HwyEntryInfo {
    Base::Info_Lane_t lane_loc;
    Coord::txWGS84 entry_pos;
    Base::txFloat distance;
  };

 public:
  Billboard_Hwy() TX_DEFAULT;
  virtual ~Billboard_Hwy() TX_DEFAULT;
  virtual Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid; }
  virtual void Clear() TX_NOEXCEPT {
    m_isValid = false;
    entry_dict.clear();
  }
  virtual Base::txBool Initialize(const std::vector<Base::Info_Lane_t>& vec_lane_loc_info,
                                  const Base::txFloat _search_dist, const Base::txBool _use_end_pt = true) TX_NOEXCEPT;
  Base::txFloat influnce_dist() TX_NOEXCEPT { return m_search_distance; }
  static Base::txFloat error_res() TX_NOEXCEPT { return -1.0; }
  virtual Base::txFloat query_dist_to_hwy_entry(const Base::Info_Lane_t _loc_info) const TX_NOEXCEPT;

 protected:
  virtual void GenerateInflunceLanes(const hadmap::txLanePtr& _entry_lane, HwyEntryInfo ParentNode,
                                     Base::txFloat search_dist) TX_NOEXCEPT;

 protected:
  Base::txBool m_isValid = false;
  Base::txFloat m_search_distance = 1000.0; /*1 km*/
  std::unordered_map<Base::Info_Lane_t /*vehicle on lane*/,
                     std::unordered_map<Base::Info_Lane_t /*entry lane uid, avoid dead loop*/, HwyEntryInfo,
                                        Utils::Info_Lane_t_HashCompare>,
                     Utils::Info_Lane_t_HashCompare>
      entry_dict;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
