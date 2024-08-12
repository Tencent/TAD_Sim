// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_hwy_entry.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tx_billboard_flags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Billboard)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)
Base::txBool Billboard_Hwy::Initialize(const std::vector<Base::Info_Lane_t>& vec_lane_loc_info,
                                       const Base::txFloat _search_dist, const Base::txBool _use_end_pt) TX_NOEXCEPT {
  m_search_distance = (_search_dist > 0) ? (_search_dist) : (m_search_distance);
  LogInfo << TX_VARS(influnce_dist());
  Clear();
  for (const auto& ref_lane_loc_info : vec_lane_loc_info) {
    if (ref_lane_loc_info.IsOnLane()) {
      HwyEntryInfo entryInfoNode;
      hadmap::txLanePtr cur_lane_ptr = HdMap::HadmapCacheConCurrent::GetTxLanePtr(ref_lane_loc_info.onLaneUid);
      if (NonNull_Pointer(cur_lane_ptr) && NonNull_Pointer(cur_lane_ptr->getGeometry())) {
        if (_use_end_pt) {
          entryInfoNode.distance = 0.0;
        } else {
          entryInfoNode.distance = -1.0 * cur_lane_ptr->getLength();
        }

        entryInfoNode.lane_loc = ref_lane_loc_info;
        entryInfoNode.entry_pos.FromWGS84(cur_lane_ptr->getGeometry()->getEnd());
#if 0
        auto lane_0_2 =
            HdMap::HadmapCacheConCurrent::GetTxLanePtr(Base::txLaneUId(ref_lane_loc_info.onLaneUid.roadId, 0, -2));
        const auto _end = lane_0_2->getGeometry()->getEnd();
        const auto _0_2_last = lane_0_2->getGeometry()->getPoint(lane_0_2->getLength());
        LOG(INFO) << TX_VARS_NAME(1_3_start, Utils::ToString(cur_lane_ptr->getGeometry()->getStart()))
                  << TX_VARS_NAME(1_3_0, Utils::ToString(cur_lane_ptr->getGeometry()->getPoint(0.0)))
                  << TX_VARS_NAME(0_2_end, Utils::ToString(_end)) << TX_VARS_NAME(0_2_last, Utils::ToString(_0_2_last));
#endif
        GenerateInflunceLanes(cur_lane_ptr, entryInfoNode, influnce_dist());
      }
    }
  }
  m_isValid = true;
  if (FLAGS_LogLevel_Billboard) {
    for (const auto& refMap : entry_dict) {
      for (const auto& refEntry : refMap.second) {
        LOG(INFO) << TX_VARS_NAME(relative_lane, refMap.first) << TX_VARS_NAME(entry_lane, refEntry.first)
                  << TX_VARS_NAME(dist, refEntry.second.distance);
      }
    }
  }
  return IsValid();
}

Base::txFloat Billboard_Hwy::query_dist_to_hwy_entry(const Base::Info_Lane_t _loc_info) const TX_NOEXCEPT {
  if (IsValid()) {
    if (_Contain_(entry_dict, _loc_info)) {
      const auto& ref_list = entry_dict.at(_loc_info);
      if (_NonEmpty_(ref_list)) {
        Base::txFloat minRes = FLT_MAX;
        for (const auto& ref : ref_list) {
          if (ref.second.distance < minRes) {
            minRes = ref.second.distance;
          }
        }
        return minRes;  // (*ref_list.begin()).second.distance;
      } else {
        return error_res();
      }
    } else {
      return error_res();
    }
  } else {
    return error_res();
  }
}

void Billboard_Hwy::GenerateInflunceLanes(const hadmap::txLanePtr& _entry_lane_ptr, HwyEntryInfo ParentNode,
                                          Base::txFloat search_dist) TX_NOEXCEPT {
  if (NonNull_Pointer(_entry_lane_ptr) && search_dist > 0.0) {
    const Base::Info_Lane_t srcLaneLocInfo(_entry_lane_ptr->getTxLaneId());
    std::unordered_map<Base::Info_Lane_t, HwyEntryInfo, Utils::Info_Lane_t_HashCompare>& refCurrentLaneHaveEntryDict =
        entry_dict[srcLaneLocInfo];
    if (_NotContain_(refCurrentLaneHaveEntryDict, ParentNode.lane_loc) ||
        ((_Contain_(refCurrentLaneHaveEntryDict, ParentNode.lane_loc)) &&
         (refCurrentLaneHaveEntryDict.at(ParentNode.lane_loc).distance >
          (ParentNode.distance + _entry_lane_ptr->getLength())))) {
      /*1. register LaneHaveEntryDict*/
      const Base::txFloat src_lane_length = _entry_lane_ptr->getLength();
      ParentNode.distance += src_lane_length;
      search_dist -= src_lane_length;
      refCurrentLaneHaveEntryDict[ParentNode.lane_loc] = ParentNode;
      // LOG(INFO) << "[refCurrentLaneHaveEntryDict]" << TX_VARS(ParentNode.lane_loc) << TX_VARS(srcLaneLocInfo)  <<
      // TX_VARS(ParentNode.distance);
      /*2. search pre section set*/
      const Base::Info_Lane_t src_lane_loc_info(_entry_lane_ptr->getTxLaneId());
      std::set<Base::txLaneUId> pre_laneUid_set =
          HdMap::HadmapCacheConCurrent::GetPreLaneSetByLaneUid(src_lane_loc_info.onLaneUid);
      std::set<Base::txSectionUId> pre_sectionUid_set;
      for (const auto& laneUid : pre_laneUid_set) {
        // LOG(INFO) << TX_VARS_NAME(src_lane, Utils::ToString(src_lane_loc_info.onLaneUid)) << TX_VARS_NAME(pre_lane,
        // Utils::ToString(laneUid));
        pre_sectionUid_set.insert(std::make_pair(laneUid.roadId, laneUid.sectionId));
      }
      /*3. recursion pre lanes*/
      for (const auto& refSectionUid : pre_sectionUid_set) {
        hadmap::txSectionPtr section_ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(refSectionUid);
        if (NonNull_Pointer(section_ptr)) {
          const auto& refLanes = section_ptr->getLanes();
          for (const auto& refLanePtr : refLanes) {
            if (NonNull_Pointer(refLanePtr)) {
              GenerateInflunceLanes(refLanePtr, ParentNode, search_dist);
            }
          }
        }
      }

      /*4. search pre lanelink*/
      Base::LocInfoSet pre_link_set = HdMap::HadmapCacheConCurrent::GetLanePrevLinkSet(src_lane_loc_info.onLaneUid);
      for (const auto& link_loc_info : pre_link_set) {
        hadmap::txLaneLinkPtr link_ptr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(link_loc_info.onLinkFromLaneUid,
                                                                                        link_loc_info.onLinkToLaneUid);
        if (NonNull_Pointer(link_ptr) && NonNull_Pointer(link_ptr->getGeometry())) {
          if (_NotContain_(entry_dict[link_loc_info], ParentNode.lane_loc) ||
              (entry_dict[link_loc_info].at(ParentNode.lane_loc).distance >
               (ParentNode.distance + link_ptr->getGeometry()->getLength()))) {
            /*4.1 register link*/
            const Base::txFloat link_length = link_ptr->getGeometry()->getLength();
            HwyEntryInfo new_node = ParentNode;
            new_node.distance += link_length;
            entry_dict[link_loc_info][new_node.lane_loc] = new_node;
            /*4.2 search link from road*/
            hadmap::txSectionPtr section_ptr = HdMap::HadmapCacheConCurrent::GetTxSectionPtr(
                std::make_pair(link_loc_info.onLinkFromLaneUid.roadId, link_loc_info.onLinkFromLaneUid.sectionId));
            if (NonNull_Pointer(section_ptr)) {
              const auto& refLanes = section_ptr->getLanes();
              for (const auto& refLanePtr : refLanes) {
                if (NonNull_Pointer(refLanePtr)) {
                  GenerateInflunceLanes(refLanePtr, new_node, search_dist - link_length);
                }
              }
            }
          }
        }
      }
    }
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
