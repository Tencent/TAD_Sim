// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HdMap/tx_hd_map_cache_concurrent.h"
#include <fstream>
#include "HdMap/tbb_utils.h"
#include "mapengine/hadmap_codes.h"
#include "tx_frame_utils.h"
#include "tx_hadmap_utils.h"
#include "tx_spatial_query.h"
#define BOOST_STACKTRACE_USE_ADDR2LINE
#include <algorithm>
#include <boost/stacktrace.hpp>
#include <numeric>
#include "HdMap/tx_road_network.h"
#include "tbb/concurrent_vector.h"
#if USE_HashedRoadNetwork
#  include "HdMap/tx_hashed_road.h"
#endif /*USE_HashedRoadNetwork*/
#include "nlohmann/json.hpp"
#include "tx_sys_info.h"
const bool showMapCacheLog = true;
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_MapSDK)
TX_NAMESPACE_OPEN(HdMap)

void HadmapCacheConCurrent::ShowCacheInfo() TX_NOEXCEPT {
  LOG(INFO) << TX_VARS(pMapHandle) << std::endl
            << TX_VARS(s_id2roadPtr.size()) << std::endl
            << TX_VARS(s_sectionUid2sectionPtr.size()) << std::endl
            << TX_VARS(s_id2lanelinkPtr.size()) << std::endl
            << TX_VARS(s_id2lanePtr.size())
            << std::endl
            /*<< TX_VARS(s_uid2LaneInfo.size()) << std::endl
            << TX_VARS(s_uid2LaneLinkInfo.size()) << std::endl*/
            << TX_VARS(s_lanelocinfo2GeomInfo.size()) << std::endl
            << TX_VARS(s_roadlocinfo2GeomInfo.size()) << std::endl
            << TX_VARS(s_fromUidtoUid_lanelinkPtr.size()) << std::endl
            << TX_VARS(s_roadId2roadStartEnd.size()) << std::endl
            << TX_VARS(s_roadId2junction.size()) << std::endl
            << TX_VARS(s_uid2LaneLength.size()) << std::endl
            << TX_VARS(s_linkId2LinkLength.size()) << std::endl
            << TX_VARS(s_lane_next_laneset.size()) << std::endl
            << TX_VARS(s_lane_pre_laneset.size()) << std::endl
            << TX_VARS(s_dead_end_road_flag_map_dead_point.size()) << std::endl
            << TX_VARS(s_RoadId2NextRoadSet.size()) << std::endl
            << TX_VARS(s_RoadId2PreRoadSet.size()) << std::endl
            << TX_VARS(s_fromRoadIdToRoadId2LaneLinkIdSet.size()) << std::endl
            << TX_VARS(s_fromRoadIdToRoadId2LaneLinkIdSet4Route.size()) << std::endl
            << TX_VARS(s_fromRoadId2LaneLinkSet.size());
}

Base::txLaneID HadmapCacheConCurrent::LaneUtilityInfo::GetLaneIndex() const TX_NOEXCEPT {
  if (laneInfo.isOnLaneLink) {
    return Utils::ComputeLaneIndex(-1);
  } else {
    return Utils::ComputeLaneIndex(laneInfo.onLaneUid.laneId);
  }
}

hadmap::txMapHandle *HadmapCacheConCurrent::pMapHandle = nullptr;
hadmap::PointVec HadmapCacheConCurrent::s_envelope;
HadmapCacheConCurrent::id2roadPtrType HadmapCacheConCurrent::s_id2roadPtr;
HadmapCacheConCurrent::sectionUid2sectionPtrType HadmapCacheConCurrent::s_sectionUid2sectionPtr;
HadmapCacheConCurrent::id2lanePtrType HadmapCacheConCurrent::s_id2lanePtr;
HadmapCacheConCurrent::id2lanelinkPtrType HadmapCacheConCurrent::s_id2lanelinkPtr;
// HadmapCacheConCurrent::uid2LaneInfoType HadmapCacheConCurrent::s_uid2LaneInfo;
// HadmapCacheConCurrent::uid2LaneLinkInfoType HadmapCacheConCurrent::s_uid2LaneLinkInfo;
HadmapCacheConCurrent::laneLocInfo2LaneInfoType HadmapCacheConCurrent::s_lanelocinfo2GeomInfo;
HadmapCacheConCurrent::laneLocInfo2LaneShapeType HadmapCacheConCurrent::s_laneLocInfo2LaneShapeType;
HadmapCacheConCurrent::RoadLocInfo2GeomInfoType HadmapCacheConCurrent::s_roadlocinfo2GeomInfo;
HadmapCacheConCurrent::fromUid_toUid2lanelinkPtrType HadmapCacheConCurrent::s_fromUidtoUid_lanelinkPtr;
HadmapCacheConCurrent::road2StartEndType HadmapCacheConCurrent::s_roadId2roadStartEnd;

HadmapCacheConCurrent::road2junctionType HadmapCacheConCurrent::s_roadId2junction;

HadmapCacheConCurrent::uid2LaneLinkSetType HadmapCacheConCurrent::s_lane_from_linkset;
HadmapCacheConCurrent::uid2LaneLinkSetType HadmapCacheConCurrent::s_lane_to_linkset;
HadmapCacheConCurrent::lanelinkLocInfo2LaneSetType HadmapCacheConCurrent::s_link_from_laneset;
HadmapCacheConCurrent::lanelinkLocInfo2LaneSetType HadmapCacheConCurrent::s_link_to_laneset;

HadmapCacheConCurrent::uid2LengthType HadmapCacheConCurrent::s_uid2LaneLength;
HadmapCacheConCurrent::uid2LaneMarkType HadmapCacheConCurrent::s_uid2LaneMark;
HadmapCacheConCurrent::lanelinkId2LengthType HadmapCacheConCurrent::s_linkId2LinkLength;

HadmapCacheConCurrent::uid2LaneUidSet HadmapCacheConCurrent::s_lane_next_laneset;
HadmapCacheConCurrent::uid2LaneUidSet HadmapCacheConCurrent::s_lane_pre_laneset;

HadmapCacheConCurrent::uid2HdMapLocateInfo HadmapCacheConCurrent::s_dead_end_road_flag_map_dead_point;

HadmapCacheConCurrent::road2RoadSetType HadmapCacheConCurrent::s_RoadId2NextRoadSet;
HadmapCacheConCurrent::road2RoadSetType HadmapCacheConCurrent::s_RoadId2PreRoadSet;

HadmapCacheConCurrent::roadIdPair2LaneLinkSetType HadmapCacheConCurrent::s_fromRoadIdToRoadId2LaneLinkIdSet;
HadmapCacheConCurrent::roadIdPair2LaneLinkSetType HadmapCacheConCurrent::s_fromRoadIdToRoadId2LaneLinkIdSet4Route;

HadmapCacheConCurrent::LocInfoBlackListType HadmapCacheConCurrent::s_locInfo_blacklist;

HadmapCacheConCurrent::roadId2LaneLinkSetType HadmapCacheConCurrent::s_fromRoadId2LaneLinkSet;
HadmapCacheConCurrent::laneBoundaryId2LaneBoundaryPtrType HadmapCacheConCurrent::s_id2BoundaryPtr;

HadmapCacheConCurrent::uid2RefuseChangeLane HadmapCacheConCurrent::s_laneUid2RefuseChangeLane_Left;
HadmapCacheConCurrent::uid2RefuseChangeLane HadmapCacheConCurrent::s_laneUid2RefuseChangeLane_Right;

HadmapCacheConCurrent::evId2RefuseChangeLaneSet HadmapCacheConCurrent::s_special_refuse_change_lane_left;
HadmapCacheConCurrent::evId2RefuseChangeLaneSet HadmapCacheConCurrent::s_special_refuse_change_lane_right;

HadmapCacheConCurrent::DstReRefuseLaneChangeSet HadmapCacheConCurrent::s_dst_refuse_lane_change_left;
HadmapCacheConCurrent::DstReRefuseLaneChangeSet HadmapCacheConCurrent::s_dst_refuse_lane_change_right;

HadmapCacheConCurrent::lanelinkExpandVisionType HadmapCacheConCurrent::s_lanelink_expand_vision;

HadmapCacheConCurrent::HadmapFilter HadmapCacheConCurrent::s_hadmap_filer;

HadmapCacheConCurrent::hdmapTrick_LaneUidLeftRightType HadmapCacheConCurrent::s_hdmapTrick_LaneUidLeftRightType;
HadmapCacheConCurrent::hdmapTrick_RoadLaneKeepTimeType HadmapCacheConCurrent::s_hdmapTrick_RoadLaneKeepTimeType;

Base::txBool HadmapCacheConCurrent::sValid = false;

Coord::txENU HadmapCacheConCurrent::s_bottom_left;
Coord::txENU HadmapCacheConCurrent::s_top_right;

Base::txBool HadmapCacheConCurrent::Release() TX_NOEXCEPT {
  sValid = false;
  if (NonNull_Pointer(pMapHandle)) {
    hadmap::hadmapClose(&pMapHandle);
  }
  pMapHandle = nullptr;
  s_id2roadPtr.clear();
  s_sectionUid2sectionPtr.clear();
  s_id2lanelinkPtr.clear();
  s_id2lanePtr.clear();
  /*s_uid2LaneInfo.clear();
  s_uid2LaneLinkInfo.clear();*/
  s_lanelocinfo2GeomInfo.clear();
  s_laneLocInfo2LaneShapeType.clear();
  s_roadlocinfo2GeomInfo.clear();
  s_fromUidtoUid_lanelinkPtr.clear();
  s_roadId2roadStartEnd.clear();
  s_roadId2junction.clear();

  s_uid2LaneLength.clear();
  s_linkId2LinkLength.clear();
  s_lane_next_laneset.clear();
  s_lane_pre_laneset.clear();
  ReleaseRoadGraphCache();
  s_dead_end_road_flag_map_dead_point.clear();
  s_RoadId2NextRoadSet.clear();
  s_RoadId2PreRoadSet.clear();
  s_fromRoadIdToRoadId2LaneLinkIdSet.clear();
  s_fromRoadIdToRoadId2LaneLinkIdSet4Route.clear();
  init_black_list(/*s_landlink_blacklist.clear();*/);
  s_fromRoadId2LaneLinkSet.clear();
  s_id2BoundaryPtr.clear();
  s_laneUid2RefuseChangeLane_Left.clear();
  s_laneUid2RefuseChangeLane_Right.clear();
  s_special_refuse_change_lane_left.clear();
  s_special_refuse_change_lane_right.clear();
  s_dst_refuse_lane_change_left.clear();
  s_dst_refuse_lane_change_right.clear();
  s_lanelink_expand_vision.clear();
  s_hadmap_filer.Clear();
  Geometry::SpatialQuery::HashedRoadNetworkRTree2D::getInstance().Clear(TX_MARK("HashedLaneInfo Release Success."));
  s_envelope.clear();

  s_hdmapTrick_LaneUidLeftRightType.clear();
  s_hdmapTrick_RoadLaneKeepTimeType.clear();
  return true;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::DebugOutput(const Base::txString _strFile) TX_NOEXCEPT {
  std::ofstream outfile(_strFile);
  if (outfile) {
    {
      /*s_roadId2roadStartEnd*/
      outfile << "################   s_roadId2roadStartEnd Start   #################" << std::endl;
      outfile << TX_VARS(s_roadId2roadStartEnd.size());
      auto range = s_roadId2roadStartEnd.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(roadId, (*itr).first) << " : ";
        const RoadStartEndInfo &roadStartEnd = (*itr).second;
        outfile << TX_VARS_NAME(startPt, roadStartEnd[0]) << ", " << TX_VARS_NAME(endPt, roadStartEnd[1]);
        outfile << std::endl;
      }
      outfile << "################   s_roadId2roadStartEnd End   #################" << std::endl;
    }
    {
      /*s_roadId2junction*/
      outfile << "################   s_roadId2junction Start   #################" << std::endl;
      outfile << TX_VARS(s_roadId2junction.size());
      auto range = s_roadId2junction.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(roadId, (*itr).first) << " : " << TX_VARS_NAME(junction, (*itr).second.StrWGS84())
                << std::endl;
      }
      outfile << "################   s_roadId2junction End   #################" << std::endl;
    }
    {
      /*s_uid2LaneLength*/
      outfile << "################   s_uid2LaneLength Start   #################" << std::endl;
      outfile << TX_VARS(s_uid2LaneLength.size());
      auto range = s_uid2LaneLength.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " length : " << (*itr).second << std::endl;
      }
      outfile << "################   s_uid2LaneLength End   #################" << std::endl;
    }

    {
      /*s_linkId2LinkLength*/
      outfile << "################   s_linkId2LinkLength Start   #################" << std::endl;
      outfile << TX_VARS(s_linkId2LinkLength.size());
      auto range = s_linkId2LinkLength.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(lanelinkId, (*itr).first) << " length : " << (*itr).second << std::endl;
      }
      outfile << "################   s_linkId2LinkLength End   #################" << std::endl;
    }

    {
      /*s_lane_next_laneset*/
      outfile << "################   s_lane_next_laneset Start   #################" << std::endl;
      outfile << TX_VARS(s_lane_next_laneset.size());
      auto range = s_lane_next_laneset.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " : ";
        const auto next_laneset = (*itr).second;
        for (const auto laneUid : next_laneset) {
          outfile << Utils::ToString(laneUid);
        }
        outfile << std::endl;
      }
      outfile << "################   s_lane_next_laneset End   #################" << std::endl;
    }

    {
      /*s_lane_pre_laneset*/
      outfile << "################   s_lane_pre_laneset Start   #################" << std::endl;
      outfile << TX_VARS(s_lane_pre_laneset.size());
      auto range = s_lane_pre_laneset.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " : ";
        const auto next_laneset = (*itr).second;
        for (const auto laneUid : next_laneset) {
          outfile << Utils::ToString(laneUid);
        }
        outfile << std::endl;
      }
      outfile << "################   s_lane_pre_laneset End   #################" << std::endl;
    }
    {
      /*s_lane_from_linkset*/
      outfile << "################   s_lane_from_linkset Start   #################" << std::endl;
      std::map<Base::txLaneUId, Base::LocInfoSet> s_lane_from_linkset_single;
      outfile << TX_VARS(s_lane_from_linkset.size());
      {
        auto range = s_lane_from_linkset.range();
        for (auto itr = range.begin(); itr != range.end(); ++itr) {
          s_lane_from_linkset_single[(*itr).first] = (*itr).second;
        }
      }
      auto &range = s_lane_from_linkset_single;
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " : ";
        const auto from_linkset = (*itr).second;
        for (const auto linkId : from_linkset) {
          outfile << linkId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_lane_from_linkset End   #################" << std::endl;
    }
    {
      /*s_lane_to_linkset*/
      outfile << "################   s_lane_to_linkset Start   #################" << std::endl;
      outfile << TX_VARS(s_lane_to_linkset.size());
      std::map<Base::txLaneUId, Base::LocInfoSet> s_lane_to_linkset_single;
      {
        auto range = s_lane_to_linkset.range();
        for (auto itr = range.begin(); itr != range.end(); ++itr) {
          s_lane_to_linkset_single[(*itr).first] = (*itr).second;
        }
      }
      auto &range = s_lane_to_linkset_single;
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " : ";
        const auto to_linkset = (*itr).second;
        for (const auto linkId : to_linkset) {
          outfile << linkId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_lane_to_linkset End   #################" << std::endl;
    }
    {
      /*s_link_from_laneset*/
      outfile << "################   s_link_from_laneset Start   #################" << std::endl;
      outfile << TX_VARS(s_link_from_laneset.size());
      std::map<Base::Info_Lane_t, std::set<Base::txLaneUId>> s_link_from_laneset_single;
      {
        auto range = s_link_from_laneset.range();
        for (auto itr = range.begin(); itr != range.end(); ++itr) {
          s_link_from_laneset_single[(*itr).first] = (*itr).second;
        }
      }
      auto &range = s_link_from_laneset_single;
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(lanelinkId, (*itr).first) << " : ";
        const auto from_laneset = (*itr).second;
        for (const auto laneUid : from_laneset) {
          outfile << Utils::ToString(laneUid);
        }
        outfile << std::endl;
      }
      outfile << "################   s_link_from_laneset End   #################" << std::endl;
    }
    {
      /*s_link_to_laneset*/
      outfile << "################   s_link_to_laneset Start   #################" << std::endl;
      outfile << TX_VARS(s_link_to_laneset.size());
      std::map<Base::Info_Lane_t, std::set<Base::txLaneUId>> s_link_to_laneset_single;
      {
        auto range = s_link_to_laneset.range();
        for (auto itr = range.begin(); itr != range.end(); ++itr) {
          s_link_to_laneset_single[(*itr).first] = (*itr).second;
        }
      }
      auto &range = s_link_to_laneset_single;
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(lanelinkId, (*itr).first) << " : ";
        const auto to_laneset = (*itr).second;
        for (const auto laneUid : to_laneset) {
          outfile << Utils::ToString(laneUid);
        }
        outfile << std::endl;
      }
      outfile << "################   s_link_to_laneset End   #################" << std::endl;
    }
    {
      /*s_dead_end_road_flag_map_dead_point*/
      outfile << "################   s_dead_end_road_flag_map_dead_point Start   #################" << std::endl;
      outfile << TX_VARS(s_dead_end_road_flag_map_dead_point.size());
      std::map<Base::txLaneUId, txWGS84> s_dead_end_road_flag_map_dead_point_single;
      {
        auto range = s_dead_end_road_flag_map_dead_point.range();
        for (auto itr = range.begin(); itr != range.end(); ++itr) {
          s_dead_end_road_flag_map_dead_point_single[(*itr).first] = (*itr).second;
        }
      }
      auto &range = s_dead_end_road_flag_map_dead_point_single;
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(laneUid, Utils::ToString((*itr).first)) << " : " << (*itr).second << std::endl;
      }
      outfile << "################   s_dead_end_road_flag_map_dead_point End   #################" << std::endl;
    }
    {
      /*s_RoadId2NextRoadSet*/
      outfile << "################   s_RoadId2NextRoadSet Start   #################" << std::endl;
      outfile << TX_VARS(s_RoadId2NextRoadSet.size());
      auto range = s_RoadId2NextRoadSet.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(roadId, ((*itr).first)) << " : ";
        const auto NextRoadSet = (*itr).second;
        for (const auto roadId : NextRoadSet) {
          outfile << roadId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_RoadId2NextRoadSet End   #################" << std::endl;
    }
    {
      /*s_RoadId2PreRoadSet*/
      outfile << "################   s_RoadId2PreRoadSet Start   #################" << std::endl;
      outfile << TX_VARS(s_RoadId2PreRoadSet.size());
      auto range = s_RoadId2PreRoadSet.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(roadId, ((*itr).first)) << " : ";
        const auto PreRoadSet = (*itr).second;
        for (const auto roadId : PreRoadSet) {
          outfile << roadId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_RoadId2PreRoadSet End   #################" << std::endl;
    }

    {
      /*s_fromRoadIdToRoadId2LaneLinkIdSet*/
      outfile << "################   s_fromRoadIdToRoadId2LaneLinkIdSet Start   #################" << std::endl;
      outfile << TX_VARS(s_fromRoadIdToRoadId2LaneLinkIdSet.size());
      auto range = s_fromRoadIdToRoadId2LaneLinkIdSet.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(fromRoadId, ((*itr).first.first)) << " - "
                << TX_VARS_NAME(ToRoadId, ((*itr).first.second)) << " : ";
        const auto LaneLinkIdSet = (*itr).second;
        for (const auto linkId : LaneLinkIdSet) {
          outfile << linkId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_fromRoadIdToRoadId2LaneLinkIdSet End   #################" << std::endl;
    }
    {
      /*s_fromRoadId2LaneLinkSet*/
      outfile << "################   s_fromRoadId2LaneLinkSet Start   #################" << std::endl;
      outfile << TX_VARS(s_fromRoadId2LaneLinkSet.size());
      auto range = s_fromRoadId2LaneLinkSet.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        outfile << TX_VARS_NAME(fromRoadId, ((*itr).first)) << " : ";
        const auto LaneLinkIdSet = (*itr).second;
        for (const auto linkId : LaneLinkIdSet) {
          outfile << linkId << ", ";
        }
        outfile << std::endl;
      }
      outfile << "################   s_fromRoadId2LaneLinkSet End   #################" << std::endl;
    }

    outfile.close();
    LOG(WARNING) << "output debug finish.";
    txPAUSE;
    return true;
  } else {
    return false;
  }
}

Base::txBool HadmapCacheConCurrent::Initialize(hadmap::txMapHandle *hdr) TX_NOEXCEPT {
  Release();
  pMapHandle = hdr;
  return NonNull_Pointer(pMapHandle);
}

hadmap::PointVec HadmapCacheConCurrent::MakeEnvelope(const hadmap::txPoint &ref_origin_GPS,
                                                     const Base::txFloat Envelope_Offset) TX_NOEXCEPT {
  hadmap::PointVec envelope;
  envelope.push_back(hadmap::txPoint(__Lon__(ref_origin_GPS) - Envelope_Offset,
                                     __Lat__(ref_origin_GPS) - Envelope_Offset, __Alt__(ref_origin_GPS)));
  envelope.push_back(hadmap::txPoint(__Lon__(ref_origin_GPS) + Envelope_Offset,
                                     __Lat__(ref_origin_GPS) + Envelope_Offset, __Alt__(ref_origin_GPS)));
  return envelope;
}

hadmap::MAP_DATA_TYPE HadmapCacheConCurrent::MapType(const Base::txString mapPath) TX_NOEXCEPT {
  return Utils::ComputeMapType(mapPath);
}

void HadmapCacheConCurrent::PrintMapFile(const Base::txString mapPath) TX_NOEXCEPT {
  LOG(INFO) << TX_VARS(mapPath);
  const hadmap::MAP_DATA_TYPE mapType = MapType(mapPath);
  if (hadmap::MAP_DATA_TYPE::OPENDRIVE == mapType) {
    std::ifstream infile(mapPath);
    std::string line;
    std::string token;
    token.resize(20);
    std::fill_n(token.begin(), token.size(), '#');
    LOG(INFO) << token;
    while (std::getline(infile, line)) {
      LOG(INFO) << line;
    }
    LOG(INFO) << token;
    infile.close();
  } else {
    LOG(INFO) << "[unsupport_print_map_type]" << TX_VARS(mapPath);
  }
}

Base::txBool HadmapCacheConCurrent::Initialize(const InitParams_t &refParam) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  Release();

  {
    FilePath file_name = FilePath(refParam.strHdMapFilePath).filename();
    InitializeHadmapFilter(refParam.strHadmapFilter, file_name.string());
  }
  { InitializeHdMapTrick(refParam.strHdMapFilePath); }

  Coord::txReferencePoint::SetReferencePoint(refParam.SceneOriginGPS);
  const hadmap::MAP_DATA_TYPE mapType = MapType(refParam.strHdMapFilePath);

  if (CallSucc(refParam.op_map_range.has_value())) {
    const auto range = (*refParam.op_map_range);
    s_envelope.clear();
    Coord::txWGS84 bottom_left;
    bottom_left.FromWGS84(range.bottom_left);
    Coord::txWGS84 top_right;
    top_right.FromWGS84(range.top_right);
    const Base::txFloat range_step = FLAGS_MapMultiRangeStep;
    bottom_left.TranslateLocalPos(Base::txVec3(-1.0 * range_step, -1.0 * range_step, 0.0));
    top_right.TranslateLocalPos(Base::txVec3(range_step, range_step, 0.0));
    s_envelope.emplace_back(bottom_left.WGS84());
    s_envelope.emplace_back(top_right.WGS84());
    LOG(INFO) << "Use local map loader." << TX_VARS(range_step)
              << TX_VARS_NAME(input_bottom_left, Utils::ToString(range.bottom_left))
              << TX_VARS_NAME(input_top_right, Utils::ToString(range.top_right))
              << TX_VARS_NAME(step_bottom_left, Utils::ToString(s_envelope[0]))
              << TX_VARS_NAME(step_top_right, Utils::ToString(s_envelope[1]));
  } else {
    s_envelope = MakeEnvelope(refParam.SceneOriginGPS, FLAGS_Envelope_Offset);
    LOG(INFO) << "Use all map loader." << Utils::ToString(s_envelope[0]) << Utils::ToString(s_envelope[1])
              << TX_VARS(FLAGS_Envelope_Offset);
  }
  std::vector<std::tuple<hadmap::txPoint /*left_bottom*/, hadmap::txPoint /*right_top*/>> map_range_list;
  map_range_list.emplace_back(std::make_tuple(s_envelope[0], s_envelope[1]));
  LOG(INFO) << "connect hadmap : " << refParam.strHdMapFilePath << Utils::ToString(s_envelope[0])
            << Utils::ToString(s_envelope[1]) << " start...";
#ifdef ON_CLOUD
  if (CallSucc(Utils::CheckRetCode(
          hadmap::hadmapConnect(refParam.strHdMapFilePath.c_str(), map_range_list, mapType, &pMapHandle)))) {
#else  /*ON_CLOUD*/
  if (CallSucc(Utils::CheckRetCode(hadmap::hadmapConnect(refParam.strHdMapFilePath.c_str(), mapType, &pMapHandle)))) {
#endif /*ON_CLOUD*/
    LOG(INFO) << "connect hadmap : " << refParam.strHdMapFilePath << " success.";
#if __TX_Mark__("GetAllRoads")

    Coord::txWGS84 envelope_0(s_envelope[0]), envelope_1(s_envelope[1]);
    auto edgeLength = envelope_0.ToENU().ENU2D() - envelope_1.ToENU().ENU2D();
    const Base::txFloat envelope_area = std::fabs(edgeLength[0]) * std::fabs(edgeLength[1]);
    LogInfo << TX_VARS(envelope_area) << TX_VARS(envelope_0) << TX_VARS(envelope_1);

    hadmap::txLanes pLanes;
    LOG(INFO) << "call hadmap::getLanes start.";
    hadmap::getLanes(pMapHandle, s_envelope, pLanes);
    LOG(INFO) << "call hadmap::getLanes end. " << TX_VARS_NAME(lane_size, pLanes.size());
    hadmap::txLaneLinks pLaneLinks;
    hadmap::getLaneLinks(pMapHandle, s_envelope, pLaneLinks);
    LOG(INFO) << "call hadmap::getLaneLinks end. " << TX_VARS_NAME(lanelink_size, pLaneLinks.size());

    bool wholeData = true;
    hadmap::txRoads pRoads;
    hadmap::getRoads(pMapHandle, s_envelope, wholeData, pRoads);
    LOG(INFO) << "call hadmap::getRoads end. " << TX_VARS_NAME(road_size, pRoads.size());
    Utils::Sys::ShowMemoryInfo();
    ParallelCreateLaneCache(pLanes, pLaneLinks, pRoads);
    LOG(WARNING) << "call ParallelCreateLaneCache end.";

    if (FLAGS_LogLevel_MapSDK) {
      hadmap::txLaneLinks uniqueLaneLinks;
      hadmap::txLanes uniqueLanes;
      MapCheck(pLanes, pLaneLinks, pRoads, uniqueLanes, uniqueLaneLinks);
    }
    ParallelCreateLanePrevPostGraph(pLanes, pLaneLinks, pRoads);

    LOG(WARNING) << "call CreateLanePrevPostGraph end.";
    if (FLAGS_EnableCheckRefuseLaneChange) {
      LOG(WARNING) << "call CheckRefuseChangeLane start.";
      CheckRefuseChangeLane(pRoads);
      LOG(WARNING) << "call CheckRefuseChangeLane end.";
    }

    Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Initialize(pLanes, pLaneLinks);
#endif /*__TX_Mark__("GetAllRoads")*/
    sValid = true;
    Hadmap_Filter(pLanes, pLaneLinks, pRoads);
    HdMap::txRoadNetwork::Initialize(pLanes, pLaneLinks);
#if USE_HashedRoadNetwork
    Geometry::SpatialQuery::HashedRoadNetworkRTree2D::getInstance().Initialize(pLanes, pLaneLinks);
#endif /*USE_HashedRoadNetwork*/
    if (FLAGS_LogLevel_MapSDK) {
      ParallelMapAABB(pRoads);
    }

#if USE_HashedRoadNetwork
    // HashedRoadCacheConCurrent::ShowDebugInfo();
#endif /*USE_HashedRoadNetwork*/
    // DebugOutput("d:/roadinfo.txt");
    MakeConsistency();
    if (FLAGS_LogLevel_GeomShape) {
      auto range = s_laneLocInfo2LaneShapeType.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        const auto laneLocInfo = (*itr).first;
        if (laneLocInfo.IsOnLaneLink()) {
          LOG(INFO) << "[GeomShape]" << TX_VARS_NAME(From, Utils::ToString(laneLocInfo.onLinkFromLaneUid))
                    << TX_VARS_NAME(To, Utils::ToString(laneLocInfo.onLinkToLaneUid))
                    << TX_VARS_NAME(Shape, (*itr).second);
        }
      }
    }
    Utils::Sys::ShowMemoryInfo();
    return true;
  } else {
    LOG(INFO) << "connect hadmap : " << refParam.strHdMapFilePath << " failure.";
    Release();
    return false;
  }
}

HadmapCacheConCurrent::txWGS84 HadmapCacheConCurrent::GetRoadStart(const Base::txRoadID _roadId) TX_NOEXCEPT {
  return GetRoadStartEnd(_roadId).front();
}

HadmapCacheConCurrent::txWGS84 HadmapCacheConCurrent::GetRoadEnd(const Base::txRoadID _roadId) TX_NOEXCEPT {
  return GetRoadStartEnd(_roadId).back();
}

HadmapCacheConCurrent::txWGS84 HadmapCacheConCurrent::GetRoad2Junction(const txRoadID _roadId) TX_NOEXCEPT {
  road2junctionType::const_accessor ca;
  if (CallFail(s_roadId2junction.find(ca, _roadId))) {
    ca.release();
    //        std::ostringstream oss;
    //        oss << TX_VARS_NAME(selfRoadId, _roadId);
    const txWGS84 selfRoadEnd = GetRoadEnd(_roadId);
    std::tuple<Base::txFloat, txWGS84> res = std::make_tuple(0.0, selfRoadEnd);

    std::set<txRoadID> visitedRoadIdSet{_roadId};
    boost::optional<txRoadID> opRoadId = _roadId;
    txRoadID objRoadId;
    do {
      objRoadId = *opRoadId;
      opRoadId = boost::none;
      Base::txBool needSearchNextRoad = true;
      /*1. all next lanelink length is zero*/
      const auto nextLinkIdSet = GetLaneLinksFromRoad(objRoadId);
      //            oss << "{";
      for (const auto linkUid : nextLinkIdSet) {
        const Base::txFloat linkLen = GetLaneLinkLength(linkUid);
        //                oss << "{" << TX_VARS(linkUid) << TX_VARS(linkLen) << "}";
        if (linkLen > 0.5) {
          needSearchNextRoad = false;
          break;
        }
      }
      //            oss << "}" << TX_COND(needSearchNextRoad);
      if (needSearchNextRoad) {
        /*2. next road is single.*/
        const auto nextRoadIdSet = GetNextRoadByRoadId(objRoadId);
        //                oss << TX_VARS(nextRoadIdSet.size());
        if (1 == nextRoadIdSet.size() && (*nextRoadIdSet.begin()) != objRoadId /*avoid dead-loop*/) {
          objRoadId = (*nextRoadIdSet.begin());
          txWGS84 curRoadEnd = GetRoadEnd(objRoadId);
          const Base::txFloat newDist = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(selfRoadEnd, curRoadEnd);
          //                   oss << TX_VARS((*nextRoadIdSet.begin())) << TX_VARS(newDist);
          if (std::get<0>(res) < newDist) {
            res = std::make_tuple(newDist, curRoadEnd);
          }
          opRoadId = objRoadId;
          visitedRoadIdSet.insert(objRoadId);
        }
      }
    } while (opRoadId.has_value() && visitedRoadIdSet.count(*opRoadId) <= 0);
    //        LOG(INFO) << "[road2junction]" << TX_VARS_NAME(start_road, _roadId) << TX_VARS_NAME(end_road, objRoadId)
    //        << oss.str();
    {
      road2junctionType::accessor a;
      s_roadId2junction.insert(a, _roadId);
      a->second = std::get<1>(res);
      a.release();
    }
    return std::get<1>(res);
  } else {
    return (*ca).second;
  }
}

HadmapCacheConCurrent::RoadStartEndInfo HadmapCacheConCurrent::GetRoadStartEnd(const txRoadID _roadId) TX_NOEXCEPT {
  road2StartEndType::const_accessor ca;
  if (CallFail(s_roadId2roadStartEnd.find(ca, _roadId))) {
    ca.release();
    std::array<txWGS84, 2> start_end;
    hadmap::txRoadPtr curRoad = GetTxRoadPtr(_roadId);
    if (NonNull_Pointer(curRoad)) {
      for (auto curLane : curRoad->getSections().front()->getLanes()) {
        if (Utils::IsLaneValid(curLane)) {
          start_end[0].FromWGS84(curLane->getGeometry()->getStart());
          break;
        }
      }

      for (auto curLane : curRoad->getSections().back()->getLanes()) {
        if (Utils::IsLaneValid(curLane)) {
          start_end[1].FromWGS84(curLane->getGeometry()->getEnd());
          break;
        }
      }
      {
        road2StartEndType::accessor a;
        s_roadId2roadStartEnd.insert(a, _roadId);
        a->second = start_end;
        a.release();
      }
      return start_end;
    } else {
      LOG(FATAL) << "GetTxRoadPtr " << TX_VARS(_roadId) << " failure.";
      return start_end;
    }
  } else {
    return (*ca).second;
  }
}

Base::txFloat HadmapCacheConCurrent::GetRoadLength(const txRoadID _roadId) TX_NOEXCEPT {
  auto road_ptr = GetTxRoadPtr(_roadId);
  if (NonNull_Pointer(road_ptr)) {
    return road_ptr->getLength();
  } else {
    return 0.0;
  }
}

hadmap::txRoadPtr HadmapCacheConCurrent::GetTxRoadPtr(const Base::txRoadID _roadId) TX_NOEXCEPT {
  id2roadPtrType::const_accessor ca;
  if (CallFail(s_id2roadPtr.find(ca, _roadId))) {
    ca.release();
    TX_MARK("road is not Register");
    hadmap::txRoadPtr roadPtr = nullptr;
    const auto opCode = hadmap::getRoad(pMapHandle, _roadId, true, roadPtr);
    if (Utils::CheckRetCode(opCode) && NonNull_Pointer(roadPtr)) {
      id2roadPtrType::accessor a;
      s_id2roadPtr.insert(a, _roadId);
      a->second = roadPtr;
      a.release();
      return roadPtr;
    } else {
      LOG(WARNING) << "hadmap::getRoad error, " << TX_VARS(_roadId);
      return nullptr;
    }
  } else {
    return (*ca).second;
  }
}
#if 0
hadmap::txLaneLinkPtr HadmapCacheConCurrent::GetTxLaneLinkPtr(const Base::txLaneLinkID _lanelinkId) TX_NOEXCEPT {
    id2lanelinkPtrType::const_accessor ca;
    if (CallFail(s_id2lanelinkPtr.find(ca, _lanelinkId))) {
        ca.release();
        TX_MARK("lanelink is not Register");
        hadmap::txLaneLinkPtr linkPtr = nullptr;
        const auto opCode = hadmap::getLaneLink(pMapHandle, _lanelinkId, linkPtr);
        if (Utils::CheckRetCode(opCode) && NonNull_Pointer(linkPtr)) {
            id2lanelinkPtrType::accessor a;
            s_id2lanelinkPtr.insert(a, _lanelinkId);
            a->second = linkPtr;
            a.release();
            return linkPtr;
        } else {
            LOG(WARNING) << "hadmap::getLaneLink error, " << TX_VARS(_lanelinkId);
            LOG(INFO) << boost::stacktrace::stacktrace(); txPAUSE;
            return nullptr;
        }
    } else {
        return (*ca).second;
    }
}
#endif
hadmap::txLaneLinkPtr HadmapCacheConCurrent::InitTxLaneLinkPtr(const txLaneUId &_fromLaneUid,
                                                               const txLaneUId &_toLaneUid,
                                                               hadmap::txLaneLinkPtr linkPtr) TX_NOEXCEPT {
  fromUid_toUid2lanelinkPtrType::accessor a;
  const auto from_to_uid = std::make_pair(_fromLaneUid, _toLaneUid);
  s_fromUidtoUid_lanelinkPtr.insert(a, from_to_uid);
  a->second = linkPtr;
  return a->second;
}

hadmap::txLaneLinkPtr HadmapCacheConCurrent::GetTxLaneLinkPtr(const txLaneUId &_fromLaneUid,
                                                              const txLaneUId &_toLaneUid) TX_NOEXCEPT {
  fromUid_toUid2lanelinkPtrType::const_accessor ca;
  if (CallSucc(s_fromUidtoUid_lanelinkPtr.find(ca, std::make_pair(_fromLaneUid, _toLaneUid)))) {
    return ca->second;
  } else {
    return nullptr;
  }
}

hadmap::txLanePtr HadmapCacheConCurrent::GetTxLanePtr(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  id2lanePtrType::const_accessor ca;
  if (CallFail(s_id2lanePtr.find(ca, _laneUid))) {
    ca.release();
    TX_MARK("lane is not Register");
    hadmap::txLanePtr lanePtr = nullptr;
    const auto opCode = hadmap::getLane(pMapHandle, _laneUid, lanePtr);
    if (Utils::CheckRetCode(opCode) && NonNull_Pointer(lanePtr)) {
      id2lanePtrType::accessor a;
      s_id2lanePtr.insert(a, _laneUid);
      a->second = lanePtr;
      a.release();
      return lanePtr;
    } else {
      LOG(WARNING) << "hadmap::getLane error, " << TX_VARS(Utils::ToString(_laneUid));
      return nullptr;
    }
  } else {
    return (*ca).second;
  }
}

hadmap::txLaneBoundaryPtr HadmapCacheConCurrent::GetTxBoundaryPtr(const txBoundaryID _boundaryId) TX_NOEXCEPT {
  laneBoundaryId2LaneBoundaryPtrType::const_accessor ca;
  if (CallFail(s_id2BoundaryPtr.find(ca, _boundaryId))) {
    ca.release();
    TX_MARK("laneboundary is not Register");
    hadmap::txLaneBoundaryPtr laneboundaryPtr = nullptr;
    const auto opCode = hadmap::getBoundary(pMapHandle, _boundaryId, laneboundaryPtr);
    if (Utils::CheckRetCode(opCode) && NonNull_Pointer(laneboundaryPtr)) {
      laneBoundaryId2LaneBoundaryPtrType::accessor a;
      s_id2BoundaryPtr.insert(a, _boundaryId);
      a->second = laneboundaryPtr;
      a.release();
      return laneboundaryPtr;
    } else {
      LOG(WARNING) << "hadmap::getBoundary error, " << TX_VARS(_boundaryId);
      return nullptr;
    }
  } else {
    return (*ca).second;
  }
}

hadmap::txSectionPtr HadmapCacheConCurrent::GetTxSectionPtr(const Base::txSectionUId &_sectionUid) TX_NOEXCEPT {
  sectionUid2sectionPtrType::const_accessor ca;
  if (CallFail(s_sectionUid2sectionPtr.find(ca, _sectionUid))) {
    ca.release();
    TX_MARK("section is not Register");
    hadmap::txSections resSections;
    const auto opCode =
        hadmap::getSections(pMapHandle, hadmap::txSectionId(_sectionUid.first, _sectionUid.second), resSections);
    if (Utils::CheckRetCode(opCode) && CallFail(resSections.empty())) {
      sectionUid2sectionPtrType::accessor a;
      s_sectionUid2sectionPtr.insert(a, _sectionUid);
      a->second = resSections[0];
      a.release();
      return resSections[0];
    } else {
      LOG(WARNING) << "hadmap::getSections error, " << TX_VARS_NAME(roadId, _sectionUid.first)
                   << TX_VARS_NAME(sectionId, _sectionUid.second);
      return nullptr;
    }
  } else {
    return (*ca).second;
  }
}

const hadmap::txLanes HadmapCacheConCurrent::GetLanesUnderSection(const txSectionUId &_sectionUid) TX_NOEXCEPT {
  hadmap::txSectionPtr resSectionPtr = GetTxSectionPtr(_sectionUid);
  if (NonNull_Pointer(resSectionPtr)) {
    return resSectionPtr->getLanes();
  } else {
    return hadmap::txLanes();
  }
}

const Base::txSize HadmapCacheConCurrent::GetLaneCountOnSection(const txSectionUId &_sectionUid) TX_NOEXCEPT {
  return GetLanesUnderSection(_sectionUid).size();
}

const Base::txSize HadmapCacheConCurrent::GetSectionCountOnRoad(const txRoadID &_roadId) TX_NOEXCEPT {
  const hadmap::txRoadPtr resRoadPtr = GetTxRoadPtr(_roadId);
  if (NonNull_Pointer(resRoadPtr)) {
    return resRoadPtr->getSections().size();
  } else {
    LOG(FATAL) << "Could not find road " << TX_VARS(_roadId);
    return 0;
  }
}

void HadmapCacheConCurrent::ParallelCreateLaneCache(hadmap::txLanes &lanePtrVec, hadmap::txLaneLinks &lanelinkPtrVec,
                                                    hadmap::txRoads &roadPtrVec) TX_NOEXCEPT {
  using namespace tbb;
#if __TX_Mark__("create road info before lane&lanelink, lane&lanelink using GetTxSectionPtr&GetRoadPtr")
  LOG(WARNING) << "parse road start.";
  const Base::txSize nRoadSize = roadPtrVec.size();
  tbb::parallel_for(blocked_range<size_t>(0, nRoadSize), TBB::parallel_create_road_cache(roadPtrVec));
  LOG(WARNING) << "parse road end.";
#endif /*__TX_Mark__*/

#if __TX_Mark__("before lane, for Make_Lane_Next_Laneset")
  LOG(WARNING) << "parse link start.";
  const Base::txSize nLaneLinkSize = lanelinkPtrVec.size();
  tbb::parallel_for(blocked_range<size_t>(0, nLaneLinkSize), TBB::parallel_create_lanelink_cache(lanelinkPtrVec));
  LOG(WARNING) << "parse link end.";
#endif

#if ON_CLOUD
  LOG(WARNING) << TX_VARS_NAME(RAW_LANE_SIZE, lanePtrVec.size());
  using LaneUidCollection = tbb::concurrent_hash_map<txLaneUId, txBool, LaneUIdHashCompare>;
  LaneUidCollection has_laneUid;
  LaneUidCollection link_from_to_laneuid_need_append;
  /*tbb::concurrent_vector<txLaneUId> con_vec_laneUid;*/
  tbb::parallel_for(static_cast<std::size_t>(0), lanePtrVec.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(lanePtrVec[idx])) {
      LaneUidCollection::accessor a;
      has_laneUid.insert(a, lanePtrVec[idx]->getTxLaneId());
      a->second = true;
      a.release();
    }
  }); /*lamda function*/
      /* parallel_for */

  tbb::parallel_for(static_cast<std::size_t>(0), lanelinkPtrVec.size(), [&](const std::size_t idx) {
    if (NonNull_Pointer(lanelinkPtrVec[idx])) {
      {
        const Base::txLaneUId fromLaneUid = lanelinkPtrVec[idx]->fromTxLaneId();
        LaneUidCollection::const_accessor ca;
        if (CallFail(has_laneUid.find(ca, fromLaneUid))) {
          LaneUidCollection::accessor a;
          link_from_to_laneuid_need_append.insert(a, fromLaneUid);
          a->second = true;
          a.release();
        }
        ca.release();
      }
      {
        const Base::txLaneUId toLaneUid = lanelinkPtrVec[idx]->toTxLaneId();
        LaneUidCollection::const_accessor ca;
        if (CallFail(has_laneUid.find(ca, toLaneUid))) {
          LaneUidCollection::accessor a;
          link_from_to_laneuid_need_append.insert(a, toLaneUid);
          a->second = true;
          a.release();
        }
        ca.release();
      }
    }
  }); /*lamda function*/
      /* parallel_for */

  for (const auto &refPair : link_from_to_laneuid_need_append) {
    const Base::txLaneUId &refLaneUid = refPair.first;
    hadmap::txLanePtr lane_ptr = nullptr;
    hadmap::getLane(txMapHdr, refLaneUid, lane_ptr);
    if (NonNull_Pointer(lane_ptr)) {
      lanePtrVec.emplace_back(lane_ptr);
    } else {
      LOG(WARNING) << TX_VARS(refLaneUid) << " in nullptr.";
    }
  }

  LOG(WARNING) << TX_VARS_NAME(APPEND_LANE_SIZE, lanePtrVec.size());
#endif /*ON_CLOUD*/
  LOG(WARNING) << "parse lane start.";
  const Base::txSize nLaneSize = lanePtrVec.size();
  tbb::parallel_for(blocked_range<size_t>(0, nLaneSize), TBB::parallel_create_lane_cache(lanePtrVec));
  LOG(WARNING) << "parse lane end.";
}

void HadmapCacheConCurrent::ReleaseRoadGraphCache() TX_NOEXCEPT {
  s_lane_from_linkset.clear();
  s_lane_to_linkset.clear();
  s_link_from_laneset.clear();
  s_link_to_laneset.clear();
  /*
  s_lane_from_linkset.clear();
  s_link_from_laneset.clear();

  s_lane_to_linkset.clear();
  s_link_to_laneset.clear();

  s_lane_next_laneset.clear();
  s_lane_pre_laneset.clear();

  s_id2roadPtr.clear();
  s_id2lanelinkPtr.clear();
  s_id2lanePtr.clear();
  s_from_to_2_lanelinkId.clear();
  s_id2road_start_end.clear();
  */
}

void HadmapCacheConCurrent::ParallelCreateLanePrevPostGraph(const hadmap::txLanes &lanePtrVec,
                                                            const hadmap::txLaneLinks &lanelinkPtrVec,
                                                            const hadmap::txRoads &roadPtrVec) TX_NOEXCEPT {
  using namespace tbb;
  {
    const Base::txSize nLaneLinkSize = lanelinkPtrVec.size();
    tbb::parallel_for(blocked_range<size_t>(0, nLaneLinkSize),
                      TBB::parallel_Make_LaneLink_Pre_Next_Info(lanelinkPtrVec));
  }
  {
    const Base::txSize nLaneSize = lanePtrVec.size();
    tbb::parallel_for(blocked_range<size_t>(0, nLaneSize), TBB::parallel_Make_Lane_Pre_Next_Laneset(lanePtrVec));
  }
  {
    const Base::txSize nLaneSize = lanePtrVec.size();
    tbb::parallel_for(blocked_range<size_t>(0, nLaneSize), TBB::parallel_make_dead_end_laneset(lanePtrVec));
  }

  if (true) {
    tbb::parallel_for(static_cast<std::size_t>(0), roadPtrVec.size(), [&](const std::size_t idx) {
      if (NonNull_Pointer(roadPtrVec[idx])) {
        GetRoad2Junction(roadPtrVec[idx]->getId());
      }
    }); /*lamda function*/
        /* parallel_for */
  } else {
    for (std::size_t idx = 0; idx < roadPtrVec.size(); ++idx) {
      if (NonNull_Pointer(roadPtrVec[idx])) {
        GetRoad2Junction(roadPtrVec[idx]->getId());
      }
    }
  }
}

void HadmapCacheConCurrent::ParallelMapAABB(hadmap::txRoads &roadPtrVec) TX_NOEXCEPT {
  tbb::concurrent_vector<hadmap::txPoint> conVec;
  conVec.reserve(roadPtrVec.size() * 3);

  tbb::parallel_for(static_cast<std::size_t>(0), roadPtrVec.size(), [&](const std::size_t idx) {
    hadmap::txRoadPtr roadPtr = roadPtrVec[idx];
    if (NonNull_Pointer(roadPtr)) {
      const auto &refSections = roadPtr->getSections();
      if (_NonEmpty_(refSections)) {
        if (NonNull_Pointer(refSections.front()) && _NonEmpty_(refSections.front()->getLanes()) &&
            NonNull_Pointer(refSections.front()->getLanes().front()) &&
            NonNull_Pointer(refSections.front()->getLanes().front()->getGeometry())) {
          // Coord::txWGS84
          // tmpWGS84(refSections.front()->getLanes().front()->getGeometry()->getStart());
          conVec.push_back(refSections.front()->getLanes().front()->getGeometry()->getStart());
        }

        if (NonNull_Pointer(refSections.back()) && _NonEmpty_(refSections.back()->getLanes()) &&
            NonNull_Pointer(refSections.back()->getLanes().back()) &&
            NonNull_Pointer(refSections.back()->getLanes().back()->getGeometry())) {
          // Coord::txWGS84 tmpWGS84(refSections.back()->getLanes().back()->getGeometry()->getEnd());
          conVec.push_back(refSections.back()->getLanes().back()->getGeometry()->getEnd());
        }
      }
    }
  }); /*lamda function*/
      /* parallel_for */

  LOG(WARNING) << "1";
  Utils::MaxMin::EnuPointVec wgs84List;
  wgs84List.reserve(conVec.size());
  std::transform(conVec.begin(), conVec.end(), std::back_inserter(wgs84List), [](hadmap::txPoint &value) {
    return Utils::MaxMin::txVec3(__Lon__(value), __Lat__(value), __Alt__(value));
  });
  LOG(WARNING) << "1";
  Base::txVec3 _wgs84_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  Base::txVec3 _wgs84_min(FLT_MAX, FLT_MAX, FLT_MAX);
  for (const auto &curWGS84 : wgs84List) {
    std::tie(_wgs84_max, _wgs84_min) = Utils::MaxMin::join(_wgs84_max, _wgs84_min, curWGS84);
  }
  Base::txVec3 mapCenterWGS84 = (_wgs84_max + _wgs84_min) / 2.0;
  LOG(WARNING) << "HdMap Center : " << Utils::ToString(mapCenterWGS84);

  Utils::MaxMin::EnuPointVec enuList;
  enuList.reserve(wgs84List.size());
  std::transform(wgs84List.begin(), wgs84List.end(), std::back_inserter(enuList), [](Base::txVec3 &wgs84) {
    Coord::txWGS84 tmp(hadmap::txPoint(wgs84.x(), wgs84.y(), wgs84.z()));
    return tmp.ToENU().ENU();
  });
  Base::txVec3 _enu_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  Base::txVec3 _enu_min(FLT_MAX, FLT_MAX, FLT_MAX);

  for (const auto &curEnu : enuList) {
    std::tie(_enu_max, _enu_min) = Utils::MaxMin::join(_enu_max, _enu_min, curEnu);
  }
  Coord::txENU top_right_enu(_enu_max + Base::txVec3(1000.0, 1000.0, 1000.0));
  Coord::txENU bottom_left_enu(_enu_min - Base::txVec3(1000.0, 1000.0, 1000.0));

  s_top_right.FromENU(_enu_max);
  s_bottom_left.FromENU(_enu_min);
  LOG(WARNING) << TX_VARS_NAME(Top_Right_Boundary, Utils::ToString(top_right_enu.ToWGS84().WGS84()))
               << TX_VARS_NAME(Bottom_Left_Boundary, Utils::ToString(bottom_left_enu.ToWGS84().WGS84()))
               << TX_VARS_NAME(Top_Right, Utils::ToString(s_top_right.ToWGS84().WGS84()))
               << TX_VARS_NAME(Bottom_Left, Utils::ToString(s_bottom_left.ToWGS84().WGS84()));
}

void HadmapCacheConCurrent::CheckRefuseChangeLane(const hadmap::txRoads &roadPtrVec) TX_NOEXCEPT {
  tbb::parallel_for(static_cast<std::size_t>(0), roadPtrVec.size(), [&](const std::size_t idx) {
    hadmap::txRoadPtr roadPtr = roadPtrVec[idx];
    if (NonNull_Pointer(roadPtr)) {
      const auto &refSections = roadPtr->getSections();
      for (const auto &refSecPtr : refSections) {
        const auto &refLanes = refSecPtr->getLanes();
        for (const auto &refLanePtr : refLanes) {
          const auto laneUid = refLanePtr->getTxLaneId();
          const auto &refNextLaneUidSet = GetNextLaneSetByLaneUid(laneUid);
          if (refNextLaneUidSet.size() > 1) {
            AddRefuseChangeLane_All(laneUid);
            for (const auto refNextLaneUid : refNextLaneUidSet) {
              AddRefuseChangeLane_All(refNextLaneUid);
            }
          }
          const auto &refPrevLaneUidSet = GetPreLaneSetByLaneUid(laneUid);
          if (refPrevLaneUidSet.size() > 1) {
            AddRefuseChangeLane_All(laneUid);
            for (const auto refPrefLaneUid : refPrevLaneUidSet) {
              AddRefuseChangeLane_All(refPrefLaneUid);
            }
          }
        }
      }
    }
  }); /*lamda function*/
      /* parallel_for */
  if (FLAGS_LogLevel_MapSDK) {
    ShowRefuseChangeLaneInfo();
  }
}

void HadmapCacheConCurrent::ShowRefuseChangeLaneInfo() TX_NOEXCEPT {
  LOG(INFO) << "[refuse_changelane_laneUid]";
  auto range = s_laneUid2RefuseChangeLane_Left.range();
  for (auto itr = range.begin(); itr != range.end(); ++itr) {
    const auto laneUid = (*itr).first;
    LOG(INFO) << "[refuse_left]" << Utils::ToString(laneUid) << std::endl;
  }

  range = s_laneUid2RefuseChangeLane_Right.range();
  for (auto itr = range.begin(); itr != range.end(); ++itr) {
    const auto laneUid = (*itr).first;
    LOG(INFO) << "[refuse_right]" << Utils::ToString(laneUid) << std::endl;
  }
}

Base::txBool HadmapCacheConCurrent::IsRefuseChangeLane_Left(const Base::txLaneUId &_laneUid) TX_NOEXCEPT {
  uid2RefuseChangeLane::const_accessor ca;
  if (CallSucc(s_laneUid2RefuseChangeLane_Left.find(ca, _laneUid))) {
    return true;
  } else {
    /*if (_NonEmpty_(s_special_refuse_change_lane_left)) {
        for (const auto& ref_evId_LaneUidSet : s_special_refuse_change_lane_left) {
            const auto& refLaneUidSet = ref_evId_LaneUidSet.second;
            if (_Contain_(refLaneUidSet, _laneUid)) {
                return true;
            }
        }
    }*/
    return false;
  }
}

Base::txBool HadmapCacheConCurrent::IsRefuseChangeLane_Right(const Base::txLaneUId &_laneUid) TX_NOEXCEPT {
  uid2RefuseChangeLane::const_accessor ca;
  if (CallSucc(s_laneUid2RefuseChangeLane_Right.find(ca, _laneUid))) {
    return true;
  } else {
    /*if (_NonEmpty_(s_special_refuse_change_lane_right)) {
        for (const auto& ref_evId_LaneUidSet: s_special_refuse_change_lane_right) {
            const auto& refLaneUidSet = ref_evId_LaneUidSet.second;
            if (_Contain_(refLaneUidSet, _laneUid)) {
                return true;
            }
        }
    }*/
    return false;
  }
}

void HadmapCacheConCurrent::AddRefuseChangeLane(const std::vector<Base::txLaneUId> &vec_cfg_laneUid,
                                                const SwitchDir _dir) TX_NOEXCEPT {
  if (SwitchDir::eLeft == _dir) {
    for (const auto &refLaneUid : vec_cfg_laneUid) {
      uid2RefuseChangeLane::accessor a;
      if (CallFail(s_laneUid2RefuseChangeLane_Left.find(a, refLaneUid))) {
        s_laneUid2RefuseChangeLane_Left.insert(a, refLaneUid);
      }
      a.release();
    }
  } else if (SwitchDir::eRight == _dir) {
    for (const auto &refLaneUid : vec_cfg_laneUid) {
      uid2RefuseChangeLane::accessor a;
      if (CallFail(s_laneUid2RefuseChangeLane_Right.find(a, refLaneUid))) {
        s_laneUid2RefuseChangeLane_Right.insert(a, refLaneUid);
      }
      a.release();
    }
  }
}

void HadmapCacheConCurrent::AddSpecialRefuseChangeLane(const Base::txSysId evId,
                                                       evId2RefuseChangeLaneSet::mapped_type &vec_cfg_laneUid,
                                                       const SwitchDir _dir) TX_NOEXCEPT {
  if (SwitchDir::eLeft == _dir) {
    s_special_refuse_change_lane_left[evId] = vec_cfg_laneUid;
  } else if (SwitchDir::eRight == _dir) {
    s_special_refuse_change_lane_right[evId] = vec_cfg_laneUid;
  }
}
Base::txBool HadmapCacheConCurrent::IsSpecialRefuseChangeLane_Left(const txLaneUId &_laneUid,
                                                                   const txFloat distOnCurve) TX_NOEXCEPT {
  for (const auto &ref_evId_vec : s_special_refuse_change_lane_left) {
    for (const auto &ref_tuple : ref_evId_vec.second) {
      if (std::get<0>(ref_tuple) == _laneUid && std::get<1>(ref_tuple) < distOnCurve &&
          distOnCurve <= std::get<2>(ref_tuple)) {
        return true;
      }
    }
  }
  return false;
}

Base::txBool HadmapCacheConCurrent::IsSpecialRefuseChangeLane_Right(const txLaneUId &_laneUid,
                                                                    const txFloat distOnCurve) TX_NOEXCEPT {
  for (const auto &ref_evId_vec : s_special_refuse_change_lane_right) {
    for (const auto &ref_tuple : ref_evId_vec.second) {
      if (std::get<0>(ref_tuple) == _laneUid && std::get<1>(ref_tuple) < distOnCurve &&
          distOnCurve <= std::get<2>(ref_tuple)) {
        return true;
      }
    }
  }
  return false;
}

void HadmapCacheConCurrent::RemoveSpecialRefuseChangeLane(const Base::txSysId evId) TX_NOEXCEPT {
  auto itr_right = s_special_refuse_change_lane_right.find(evId);
  if (std::end(s_special_refuse_change_lane_right) != itr_right) {
    s_special_refuse_change_lane_right.erase(itr_right);
  }

  auto itr_left = s_special_refuse_change_lane_left.find(evId);
  if (std::end(s_special_refuse_change_lane_left) != itr_left) {
    s_special_refuse_change_lane_left.erase(itr_left);
  }
}

void HadmapCacheConCurrent::AddRefuseChangeLane_All(const txLaneUId &_laneUid) TX_NOEXCEPT {
  {
    uid2RefuseChangeLane::accessor a;
    s_laneUid2RefuseChangeLane_Left.insert(a, _laneUid);
    // a->second = refLanePtr->getGeometry()->getEnd();
  }
  {
    uid2RefuseChangeLane::accessor a;
    s_laneUid2RefuseChangeLane_Right.insert(a, _laneUid);
    // a->second = refLanePtr->getGeometry()->getEnd();
  }
}

void HadmapCacheConCurrent::MapCheck(const hadmap::txLanes &lanePtrVec, const hadmap::txLaneLinks &lanelinkPtrVec,
                                     const hadmap::txRoads &roadPtrVec, hadmap::txLanes &uniqueLanePtrVec,
                                     hadmap::txLaneLinks &uniqueLanelinkPtrVec) TX_NOEXCEPT {
  LOG(INFO) << "Start Map Check .....";
  LOG(INFO) << "Check LaneLink Lane id unique... ";

  std::unordered_set<Base::Info_Lane_t, Info_Lane_t_HashCompare> lane_link_set;
  uniqueLanePtrVec.reserve(lanePtrVec.size());
  for (const auto lanePtr : lanePtrVec) {
    if (NonNull_Pointer(lanePtr)) {
      const Base::txLaneUId cur_LaneUid = lanePtr->getTxLaneId();
      const Base::Info_Lane_t cur_info(cur_LaneUid);
      if (CallFail(_Contain_(lane_link_set, cur_info))) {
        lane_link_set.insert(cur_info);
        uniqueLanePtrVec.emplace_back(lanePtr);
      } else {
        LOG(WARNING) << "unique check error. " << TX_VARS_NAME(LaneUid, Utils::ToString(cur_LaneUid));
      }
    }
  }

  uniqueLanelinkPtrVec.reserve(lanelinkPtrVec.size());
  for (const auto linkPtr : lanelinkPtrVec) {
    if (NonNull_Pointer(linkPtr)) {
      const Base::txLaneUId fromLaneUid = linkPtr->fromTxLaneId();
      const Base::txLaneUId toLaneUid = linkPtr->toTxLaneId();
      const Base::Info_Lane_t cur_info(linkPtr->getId(), fromLaneUid, toLaneUid);
      const Base::Info_Lane_t from_info(fromLaneUid);
      const Base::Info_Lane_t to_info(toLaneUid);

      if (CallFail(_Contain_(lane_link_set, cur_info)) && CallSucc(_Contain_(lane_link_set, from_info)) &&
          CallSucc(_Contain_(lane_link_set, to_info))) {
        lane_link_set.insert(cur_info);
        uniqueLanelinkPtrVec.emplace_back(linkPtr);
      } else {
        LOG(WARNING) << "unique check error. " << TX_COND(_Contain_(lane_link_set, cur_info))
                     << TX_COND(_Contain_(lane_link_set, from_info)) << TX_COND(_Contain_(lane_link_set, to_info))
                     << TX_VARS(cur_info);
      }
    }
  }
  LOG(INFO) << "Check LaneLink Lane id unique pass ";  // txPAUSE;
  LOG(INFO) << "Check LaneLink From To Lane... " << TX_VARS(lanelinkPtrVec.size());
  for (const auto linkPtr : lanelinkPtrVec) {
    const auto toLaneUid = linkPtr->toTxLaneId();
    const auto fromLaneUid = linkPtr->fromTxLaneId();
    const auto linkId = linkPtr->getId();
    hadmap::txLanePtr resLanePtr = nullptr;
    hadmap::getLane(pMapHandle, toLaneUid, resLanePtr);
    if (!(resLanePtr)) {
      hadmap::getLane(pMapHandle, fromLaneUid, resLanePtr);
      if (resLanePtr) {
        const auto pt = resLanePtr->getGeometry()->getEnd();
        LOG(WARNING) << _StreamPrecision_ << "point " << pt.x << ", " << pt.y << std::endl;
      }
      LOG(WARNING) << TX_VARS_NAME(mapSDK_Cannot_Fine_LaneUid, Utils::ToString(toLaneUid))
                   << TX_VARS_NAME(LinkId, (linkPtr->getId())) << TX_VARS(Utils::ToString(fromLaneUid))
                   << TX_VARS(Utils::ToString(toLaneUid));
    }

    if (Null_Pointer(GetTxLanePtr(fromLaneUid))) {
      LOG(WARNING) << TX_VARS(linkId) << TX_VARS_NAME(from_lane_is_null, Utils::ToString(fromLaneUid));
    }

    if (Null_Pointer(GetTxLanePtr(toLaneUid))) {
      LOG(WARNING) << TX_VARS(linkId) << TX_VARS_NAME(to_lane_is_null, Utils::ToString(toLaneUid));
    }

    // LOG(INFO) << TX_VARS_NAME(fromUid, Utils::ToString(fromLaneUid)) << TX_VARS_NAME(toUid,
    // Utils::ToString(toLaneUid));
  }
  LOG(INFO) << "Check LaneLink From To Lane Pass.";

  LOG(INFO) << "Check Lane Uid On Section....";
  for (const auto roadPtr : roadPtrVec) {
    const Base::txRoadID _roadId = roadPtr->getId();
    const auto refSections = roadPtr->getSections();
    const hadmap::txPoint roadStartPt = roadPtr->getGeometry()->getStart();
    const Base::txSize nSectionCnt = refSections.size();
    for (Base::txSize sectionId = 0; sectionId < nSectionCnt; ++sectionId) {
      hadmap::txSectionPtr _sectionPtr = GetTxSectionPtr(std::make_pair(_roadId, sectionId));
      if (NonNull_Pointer(_sectionPtr)) {
        txAssert(_sectionPtr->getId() == sectionId);
        const auto refLanes = _sectionPtr->getLanes();
        const Base::txSize nLaneCnt = refLanes.size();
        for (Base::txSize laneId = 1; laneId <= nLaneCnt; ++laneId) {
          hadmap::txLanePtr lanePtr = GetTxLanePtr(Base::txLaneUId(_roadId, sectionId, laneId * -1));
          if (Null_Pointer(lanePtr)) {
            LOG(WARNING) << _StreamPrecision_ << "map check, can not find lane "
                         << TX_VARS(Utils::ToString(Base::txLaneUId(_roadId, sectionId, laneId * -1)))
                         << TX_VARS(Utils::ToString(roadStartPt));
          }
        }
      } else {
        LOG(WARNING) << _StreamPrecision_ << "map check, can not find section " << TX_VARS(_roadId)
                     << TX_VARS(sectionId) << TX_VARS(Utils::ToString(roadStartPt));
      }
    }
  }
  LOG(INFO) << "Check Lane Uid On Section Pass.";
  LOG(INFO) << "Start Map Check Pass.";  // txPAUSE;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::RegisterLaneSyncInit(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  if (Utils::IsLaneValid(lanePtr)) {
    const Base::txLaneUId _laneUid = lanePtr->getTxLaneId();
    txBool ret = true;
#if __TX_Mark__("GetTxLanePtr")
    { ret = ret TX_MARK("Logic short circuit") && NonNull_Pointer(GetTxLanePtr(_laneUid)); }
#endif /*__TX_Mark__("GetTxLanePtr")*/

#if __TX_Mark__("GetLaneDir & GetLanePos")
    { ret = ret TX_MARK("Logic short circuit") && NonNull_Pointer(RegisterLaneInfoSyncRunTime(lanePtr)); }
#endif /*__TX_Mark__("GetLaneDir & GetLanePos")*/

#if __TX_Mark__("GetLineData")
    { ret = ret TX_MARK("Logic short circuit") && HdMap::HadmapCacheConCurrent::UpdateLaneDataByRoadRunTime(lanePtr); }
#endif /*__TX_Mark__("GetLineData")*/

#if __TX_Mark__("GetLaneLength")
    { HdMap::HadmapCacheConCurrent::GetLaneLength(_laneUid); }
#endif /*GetLaneLength*/

#if __TX_Mark__("GetLaneMark_LR")
    if (FLAGS_CacheLaneMarkLR) {
      HdMap::HadmapCacheConCurrent::GetLaneMark_LR(/*_laneUid*/ lanePtr);
    }
#endif /*__TX_Mark__("GetLaneMark_LR")*/
    return ret;
  } else {
    LOG(WARNING) << "RegisterLaneSyncInit failure, lanePtr invalid.";
    return false;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::RegisterLaneLinkSyncInit(const hadmap::txLaneLinkPtr _lanelinkPtr)
    TX_NOEXCEPT {
  if (Utils::IsLaneLinkValid_InRoad(_lanelinkPtr)) {
    const Base::txLaneLinkID _linkId = _lanelinkPtr->getId();
    const Base::Info_Lane_t _link_locInfo(_linkId, _lanelinkPtr->fromTxLaneId(), _lanelinkPtr->toTxLaneId());
    txBool ret = true;
#if __TX_Mark__("GetTxLaneLinkPtr")
    {
      ret = ret TX_MARK("Logic short circuit") &&
            NonNull_Pointer(
                InitTxLaneLinkPtr(_link_locInfo.onLinkFromLaneUid, _link_locInfo.onLinkToLaneUid, _lanelinkPtr));
    }
#endif /*__TX_Mark__("GetTxLaneLinkPtr")*/
    if (Utils::IsLaneLinkValid(_lanelinkPtr)) {
#if __TX_Mark__("GetLaneLinkDir & GetLaneLinkPos")
      { ret = ret TX_MARK("Logic short circuit") && NonNull_Pointer(RegisterLaneLinkInfoSyncRunTime(_lanelinkPtr)); }
#endif /*__TX_Mark__("GetLaneLinkDir & GetLaneLinkPos")*/

#if __TX_Mark__("GetLineLinkData")
      {
        ret = ret TX_MARK("Logic short circuit") &&
              HdMap::HadmapCacheConCurrent::UpdateLaneLinksDataByRoadRunTime(_lanelinkPtr);
      }
#endif /*__TX_Mark__("GetLineLinkData")*/
    }

#if __TX_Mark__("GetLaneLinkLength")
    { HdMap::HadmapCacheConCurrent::GetLaneLinkLength(_link_locInfo); }
#endif /*GetLaneLinkLength*/
    return ret;
  } else {
    LOG(WARNING) << "RegisterLaneLinkSyncInit failure, lanePtr invalid.";
    return false;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::RegisterRoadSectionSyncInit(const hadmap::txRoadPtr _roadPtr)
    TX_NOEXCEPT {
  if (NonNull_Pointer(_roadPtr)) {
    const Base::txRoadID _roadId = _roadPtr->getId();
    txBool ret = true;
#if __TX_Mark__("GetTxRoadPtr")
    { ret = ret && NonNull_Pointer(GetTxRoadPtr(_roadId)); }
#endif /*__TX_Mark__("GetTxRoadPtr")*/

#if __TX_Mark__("GetTxSectionPtr")
    {
      const hadmap::txSections curSections = _roadPtr->getSections();
      for (const auto curSectionPtr : curSections) {
        if (NonNull_Pointer(curSectionPtr)) {
          const auto sectionId = curSectionPtr->getId();
          const Base::txSectionUId sectionUid = std::make_pair(_roadId, sectionId);
          ret = ret TX_MARK("Logic short circuit") && NonNull_Pointer(GetTxSectionPtr(sectionUid));
        }
      }
    }
#endif /*__TX_Mark__("GetTxSectionPtr")*/

#if __TX_Mark__("GetRoadStartEnd")
    { GetRoadStartEnd(_roadId); }
#endif /*GetRoadStartEnd*/
    if (CallFail(ret)) {
      LOG(FATAL) << "RegisterRoadSectionSyncInit failure.";
    }
    return ret;
  } else {
    LOG(FATAL) << "RegisterRoadSectionSyncInit failure, roadPtr invalid.";
    return false;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::RegisterLaneLinkPrePostTopoInfoInit(
    const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT {
  const Base::txLaneLinkID linkId = _lanelinkPtr->getId();
  const Base::txLaneUId fromUid = _lanelinkPtr->fromTxLaneId();
  const Base::txLaneUId toUid = _lanelinkPtr->toTxLaneId();
  const Base::Info_Lane_t locInfo(linkId, fromUid, toUid);

  {
    uid2LaneLinkSetType::accessor a;
    s_lane_from_linkset.insert(a, fromUid);
    (a->second).insert(locInfo);
    a.release();
  }
  {
    uid2LaneLinkSetType::accessor a;
    s_lane_to_linkset.insert(a, toUid);
    (a->second).insert(locInfo);
    a.release();
  }
  {
    lanelinkLocInfo2LaneSetType::accessor a;
    s_link_from_laneset.insert(a, locInfo);
    (a->second).insert(fromUid);
    a.release();
  }
  {
    lanelinkLocInfo2LaneSetType::accessor a;
    s_link_to_laneset.insert(a, locInfo);
    (a->second).insert(toUid);
    a.release();
  }

  {
    road2RoadSetType::accessor a;
    s_RoadId2PreRoadSet.insert(a, toUid.roadId);
    (a->second).insert(fromUid.roadId);
    a.release();
  }
  {
    road2RoadSetType::accessor a;
    s_RoadId2NextRoadSet.insert(a, fromUid.roadId);
    (a->second).insert(toUid.roadId);
    a.release();
  }
  {
    roadId2LaneLinkSetType::accessor a;
    s_fromRoadId2LaneLinkSet.insert(a, fromUid.roadId);
    (a->second).insert(locInfo);
    a.release();
  }
  return true;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::Make_Lane_Next_Laneset(const hadmap::txLanePtr lanePtr,
                                                                        const Base::txSize nSectionCnt) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retSet;
  if (Utils::IsLaneValid(lanePtr)) {
    const Base::txLaneUId laneUid = lanePtr->getTxLaneId();

    if (CallFail(Utils::isLastSection(laneUid, nSectionCnt))) {
      TX_MARK("not on last section.");
      const Coord::txWGS84 curLaneEndPt(lanePtr->getGeometry()->getEnd());
      const hadmap::txLanes &nextSectionLanes =
          GetLanesUnderSection(std::make_pair(laneUid.roadId, Utils::NextSectionId(laneUid.sectionId)));
      for (const auto &nextSectionLane : nextSectionLanes) {
        if (Utils::IsLaneValid(nextSectionLane)) {
          const Coord::txWGS84 curNextLaneStartPt(nextSectionLane->getGeometry()->getStart());
          const Base::txFloat dist_between_lanes = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(
              curLaneEndPt TX_MARK("pre_lane_end_point"), curNextLaneStartPt TX_MARK("post_lane_start_point"));
          if (FLAGS_Connect_Lane_Dist >= dist_between_lanes) {
            retSet.insert(nextSectionLane->getTxLaneId());
          }
        }
      }
    } else {
      TX_MARK("on last section. 1. on junction; 2. link length is zero.");
      TX_MARK("search road_road link is zero.");

      const Base::LocInfoSet nextLinksFromCurrentLane = GetLaneNextLinkSet(laneUid);
      for (const auto &link_locInfo : nextLinksFromCurrentLane) {
        const txFloat curLinkLength = GetLaneLinkLength(link_locInfo);
        if (curLinkLength <= FLAGS_Connect_Lane_Dist) {
          const std::set<Base::txLaneUId> resLaneUidSet = GetLaneLinkToLaneUidSet(link_locInfo);
          std::copy(std::begin(resLaneUidSet), std::end(resLaneUidSet), std::inserter(retSet, std::end(retSet)));
        }
      }
    }
  }
  return retSet;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::Make_Lane_Pre_Laneset(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retSet;
  if (Utils::IsLaneValid(lanePtr)) {
    const Base::txLaneUId laneUid = lanePtr->getTxLaneId();
    if (CallFail(Utils::isFirstSection(laneUid))) {
      const Coord::txWGS84 curLaneStartPt(lanePtr->getGeometry()->getStart());
      const hadmap::txLanes &prevSectionLanes =
          GetLanesUnderSection(std::make_pair(laneUid.roadId, Utils::PrevSectionId(laneUid.sectionId)));
      for (const auto &prevSectionLane : prevSectionLanes) {
        if (Utils::IsLaneValid(prevSectionLane)) {
          const Coord::txWGS84 curPrevLaneEndPt(prevSectionLane->getGeometry()->getEnd());
          const Base::txFloat dist_between_lanes = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(
              curLaneStartPt TX_MARK("next_lane_start_point"), curPrevLaneEndPt TX_MARK("prev_lane_end_point"));
          if (FLAGS_Connect_Lane_Dist >= dist_between_lanes) {
            retSet.insert(prevSectionLane->getTxLaneId());
          }
        }
      }
    } else {
      TX_MARK("on first section. 1. on junction; 2. link length is zero.");
      TX_MARK("search road_road link is zero.");
      const Base::LocInfoSet prevLinksFromCurrentLane = GetLanePrevLinkSet(laneUid);
      for (const auto &link_locInfo : prevLinksFromCurrentLane) {
        const txFloat curLinkLength = GetLaneLinkLength(link_locInfo);
        if (curLinkLength <= FLAGS_Connect_Lane_Dist) {
          const std::set<Base::txLaneUId> resLaneUidSet = GetLaneLinkFromLaneUidSet(link_locInfo);
          std::copy(std::begin(resLaneUidSet), std::end(resLaneUidSet), std::inserter(retSet, std::end(retSet)));
        }
      }
    }
  }
  return retSet;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::RegisterLanePrePostTopoInfoInit(const hadmap::txLanePtr curLane)
    TX_NOEXCEPT {
  if ((Utils::IsLaneValid(curLane))) {
    const Base::txLaneUId &laneUid = curLane->getTxLaneId();
    const Base::txSize nSectionSize = GetSectionCountOnRoad(laneUid.roadId);

    const std::set<txLaneUId> nextLaneUidSet = Make_Lane_Next_Laneset(curLane, nSectionSize);
    {
      uid2LaneUidSet::accessor a;
      s_lane_next_laneset.insert(a, laneUid);
      a->second.insert(nextLaneUidSet.begin(), nextLaneUidSet.end());
      a.release();
    }
    const std::set<txLaneUId> prevLaneUidSet = Make_Lane_Pre_Laneset(curLane);
    {
      uid2LaneUidSet::accessor a;
      s_lane_pre_laneset.insert(a, laneUid);
      a->second.insert(prevLaneUidSet.begin(), prevLaneUidSet.end());
      a.release();
    }
    return true;
  } else {
    LOG(FATAL) << "lane is invalid.";
    return false;
  }
}

void HadmapCacheConCurrent::RegisterRoadInfoSyncRunTime(const hadmap::txRoadPtr _roadPtr) TX_NOEXCEPT {
  Base::Info_Road_t _roadInfo(_roadPtr->getId());
  RoadLocInfo2GeomInfoType::const_accessor ca;
  if (CallFail(s_roadlocinfo2GeomInfo.find(ca, _roadInfo))) {
    ca.release();
    const hadmap::txLineCurve *targetLaneCurve = dynamic_cast<const hadmap::txLineCurve *>(_roadPtr->getGeometry());
    if (NonNull_Pointer(targetLaneCurve)) {
      txAssert(hadmap::CoordType::COORD_WGS84 == targetLaneCurve->getCoordType());
      std::vector<controlPoint> controlPointVec;
      if (SubdivisionCurves(targetLaneCurve, controlPointVec, CurveType::catmull_rom)) {
        const auto controlPointSize = controlPointVec.size();
        if (controlPointSize >= txLaneInfo::CatmullRom_ControlPointSize) {
          auto laneInfoPtr = std::make_shared<txRoadInfo>(_roadInfo, controlPointVec);
          if (NonNull_Pointer(laneInfoPtr)) {
            RoadLocInfo2GeomInfoType::accessor a;
            s_roadlocinfo2GeomInfo.insert(a, _roadInfo);
            a->second = laneInfoPtr;
            a.release();
          } else {
            LOG(FATAL) << " make_shared txRoadInfo Pointer failure." << _roadInfo;
          }
          return;
        } else if (controlPointSize >= 2 TX_MARK("may be 3")) {
          auto laneInfoPtr =
              std::make_shared<txRoadShortInfo>(_roadInfo, controlPointVec.front(), controlPointVec.back());
          if (NonNull_Pointer(laneInfoPtr)) {
            RoadLocInfo2GeomInfoType::accessor a;
            s_roadlocinfo2GeomInfo.insert(a, _roadInfo);
            a->second = laneInfoPtr;
            a.release();
          } else {
            LOG(FATAL) << " make_shared txRoadShortInfo Pointer failure." << _roadInfo;
          }
          return;
        }
        LOG(FATAL) << " RegisterLaneSync " << TX_VARS(controlPointSize);
        return;
      } else {
        LOG(FATAL) << " SubdivisionCurves failure." << _roadInfo;
      }
    } else {
      LOG(FATAL) << " road covert txLineCure Failure. roadId = " << _roadInfo;
    }
  } else {
    TX_MARK("road has been register.");
  }
}

void HadmapCacheConCurrent::RegisterRoadInfoSyncRunTime(const hadmap::txLaneLinkPtr _lanelinkPtr) TX_NOEXCEPT {
  if (Utils::IsLaneLinkValid(_lanelinkPtr)) {
    Base::Info_Road_t _roadInfo(_lanelinkPtr->fromRoadId(), _lanelinkPtr->toRoadId());
    RoadLocInfo2GeomInfoType::const_accessor ca;
    if (CallFail(s_roadlocinfo2GeomInfo.find(ca, _roadInfo))) {
      ca.release();
      const hadmap::txLineCurve *targetLaneLinkCurve =
          dynamic_cast<const hadmap::txLineCurve *>(_lanelinkPtr->getGeometry());
      if (NonNull_Pointer(targetLaneLinkCurve)) {
        std::vector<controlPoint> controlPointVec;
        if (SubdivisionCurves(targetLaneLinkCurve, controlPointVec, CurveType::cubic_spline)) {
          const auto controlPointSize = controlPointVec.size();
          if (controlPointSize > txLaneSplineInfo::CubicSpline_ControlPointSize) {
            // LOG(WARNING) << TX_VARS(lanelinkUid) << " create txLaneSplineInfo.";
            auto laneInfoPtr =
                std::make_shared<txRoadSplineInfo>(_roadInfo, controlPointVec, targetLaneLinkCurve->getLength());
            if (NonNull_Pointer(laneInfoPtr)) {
              RoadLocInfo2GeomInfoType::accessor a;
              s_roadlocinfo2GeomInfo.insert(a, _roadInfo);
              a->second = laneInfoPtr;
              a.release();
            } else {
              LOG(FATAL) << " make_shared txRoadSplineInfo Pointer failure." << _roadInfo;
            }
            return;
          } else if (controlPointSize >= 2) {
            // LOG(WARNING) << TX_VARS(lanelinkUid) << " create txLaneShortInfo.";
            auto laneInfoPtr =
                std::make_shared<txRoadShortInfo>(_roadInfo, controlPointVec.front(), controlPointVec.back());
            if (NonNull_Pointer(laneInfoPtr)) {
              RoadLocInfo2GeomInfoType::accessor a;
              s_roadlocinfo2GeomInfo.insert(a, _roadInfo);
              a->second = laneInfoPtr;
              a.release();
            } else {
              LOG(FATAL) << " make_shared txRoadShortInfo Pointer failure." << _roadInfo;
            }
            return;
          }

          LOG(FATAL) << " RegisterLaneSync " << TX_VARS(controlPointSize);
        } else {
          LOG(WARNING) << " SubdivisionCurves failure." << _roadInfo << TX_VARS(targetLaneLinkCurve->getLength());
        }
      } else {
        LOG(FATAL) << " link covert txLineCure Failure. txLaneLinkId = " << _roadInfo;
      }
    } else {
      TX_MARK("link has been register.");
    }
  }
}

txLaneInfoInterfacePtr HadmapCacheConCurrent::RegisterLaneInfoSyncRunTime(const hadmap::txLanePtr _lanePtr)
    TX_NOEXCEPT {
  const Base::Info_Lane_t _laneInfo(_lanePtr->getTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _laneInfo))) {
    ca.release();
    const hadmap::txLineCurve *targetLaneCurve = dynamic_cast<const hadmap::txLineCurve *>(_lanePtr->getGeometry());

    if (NonNull_Pointer(targetLaneCurve)) {
      txAssert(hadmap::CoordType::COORD_WGS84 == targetLaneCurve->getCoordType());
      std::vector<controlPoint> controlPointVec;
      if (SubdivisionCurves(targetLaneCurve, controlPointVec, CurveType::catmull_rom)) {
        const auto controlPointSize = controlPointVec.size();
        if (controlPointSize >= txLaneInfo::CatmullRom_ControlPointSize) {
          auto laneInfoPtr = std::make_shared<txLaneInfo>(_laneInfo.onLaneUid, controlPointVec);
          if (NonNull_Pointer(laneInfoPtr)) {
            laneLocInfo2LaneInfoType::accessor a;
            s_lanelocinfo2GeomInfo.insert(a, _laneInfo);
            a->second = laneInfoPtr;
            a.release();

            {
              laneLocInfo2LaneShapeType::accessor shape_a;
              s_laneLocInfo2LaneShapeType.insert(shape_a, _laneInfo);
              shape_a->second = laneInfoPtr->GetShape();
              shape_a.release();
            }
          }

          return laneInfoPtr;
        } else if (controlPointSize >= 2 TX_MARK("may be 3")) {
          auto laneInfoPtr =
              std::make_shared<txLaneShortInfo>(_laneInfo.onLaneUid, controlPointVec.front(), controlPointVec.back());
          if (NonNull_Pointer(laneInfoPtr)) {
            laneLocInfo2LaneInfoType::accessor a;
            s_lanelocinfo2GeomInfo.insert(a, _laneInfo);
            a->second = laneInfoPtr;
            a.release();

            {
              laneLocInfo2LaneShapeType::accessor shape_a;
              s_laneLocInfo2LaneShapeType.insert(shape_a, _laneInfo);
              shape_a->second = laneInfoPtr->GetShape();
              shape_a.release();
            }
          }
          return laneInfoPtr;
        }
        LOG(FATAL) << " RegisterLaneSync " << TX_VARS(controlPointSize);
        return nullptr;
      } else {
        LOG(FATAL) << " SubdivisionCurves failure." << TX_VARS_NAME(txLaneId, Utils::ToString(_laneInfo.onLaneUid));
        return nullptr;
      }
    } else {
      LOG(FATAL) << " lane covert txLineCure Failure. txLaneId = " << Utils::ToString(_laneInfo.onLaneUid);
      return nullptr;
    }
  } else {
    TX_MARK("lane has been register.");
    return ca->second;
  }
}

txLaneInfoInterfacePtr HadmapCacheConCurrent::RegisterLaneLinkInfoSyncRunTime(const hadmap::txLaneLinkPtr _lanelinkPtr)
    TX_NOEXCEPT {
  const Base::Info_Lane_t _laneInfo(_lanelinkPtr->getId(), _lanelinkPtr->fromTxLaneId(), _lanelinkPtr->toTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _laneInfo))) {
    ca.release();
    const hadmap::txLineCurve *targetLaneLinkCurve =
        dynamic_cast<const hadmap::txLineCurve *>(_lanelinkPtr->getGeometry());
    if (NonNull_Pointer(targetLaneLinkCurve)) {
      std::vector<controlPoint> controlPointVec;
      if (SubdivisionCurves(targetLaneLinkCurve, controlPointVec, CurveType::cubic_spline)) {
        const auto controlPointSize = controlPointVec.size();
        if (controlPointSize > txLaneSplineInfo::CubicSpline_ControlPointSize) {
          // LOG(WARNING) << TX_VARS(lanelinkUid) << " create txLaneSplineInfo.";
          auto laneInfoPtr = std::make_shared<txLaneSplineInfo>(_laneInfo.onLinkId_without_equal,
                                                                _laneInfo.onLinkFromLaneUid, _laneInfo.onLinkToLaneUid,
                                                                controlPointVec, targetLaneLinkCurve->getLength());
          if (laneInfoPtr) {
            laneLocInfo2LaneInfoType::accessor a;
            s_lanelocinfo2GeomInfo.insert(a, _laneInfo);
            a->second = laneInfoPtr;
            a.release();

            {
              laneLocInfo2LaneShapeType::accessor shape_a;
              s_laneLocInfo2LaneShapeType.insert(shape_a, _laneInfo);
              shape_a->second = laneInfoPtr->GetShape();
              shape_a.release();
            }
          }
          return laneInfoPtr;
        } else if (controlPointSize >= 2) {
          // LOG(WARNING) << TX_VARS(lanelinkUid) << " create txLaneShortInfo.";
          auto laneInfoPtr = std::make_shared<txLaneShortInfo>(_laneInfo.onLinkId_without_equal,
                                                               _laneInfo.onLinkFromLaneUid, _laneInfo.onLinkToLaneUid,
                                                               controlPointVec.front(), controlPointVec.back());
          if (laneInfoPtr) {
            laneLocInfo2LaneInfoType::accessor a;
            s_lanelocinfo2GeomInfo.insert(a, _laneInfo);
            a->second = laneInfoPtr;
            a.release();

            {
              laneLocInfo2LaneShapeType::accessor shape_a;
              s_laneLocInfo2LaneShapeType.insert(shape_a, _laneInfo);
              shape_a->second = laneInfoPtr->GetShape();
              shape_a.release();
            }
          }
          return laneInfoPtr;
        }

        LOG(FATAL) << " RegisterLaneSync " << TX_VARS(controlPointSize);
        return nullptr;
      } else {
        LOG(FATAL) << " SubdivisionCurves failure." << TX_VARS_NAME(txLaneLinkId, _laneInfo.onLinkId_without_equal);
        return nullptr;
      }
    } else {
      LOG(FATAL) << " lane covert txLineCure Failure. txLaneLinkId = " << _laneInfo.onLinkId_without_equal;
      return nullptr;
    }
  } else {
    TX_MARK("lane has been register.");
    return ca->second;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::SubdivisionCurves(const hadmap::txLineCurve *_laneLineCurve,
                                                                       std::vector<controlPoint> &controlPointVec,
                                                                       const CurveType curveType) TX_NOEXCEPT {
  controlPointVec.clear();
  const txFloat length = _laneLineCurve->getLength();
  if (length > 1.0) {
    if (CurveType::catmull_rom == curveType) {
#if 1
      const Base::txFloat curLaneLength = _laneLineCurve->getLength();
      const Base::txFloat sampleInterval =
          ((curLaneLength / 10.0) > FLAGS_MapLocationInterval) ? (FLAGS_MapLocationInterval) : ((curLaneLength / 10.0));
      Base::txFloat curDistance = 0.0;

      controlPointVec.reserve((curLaneLength / sampleInterval) + 2);

      while (curDistance < curLaneLength) {
        Coord::txWGS84 pos(_laneLineCurve->getPoint(curDistance));
        const auto vertex = pos.ToENU().ENU2D();
        controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
        curDistance += sampleInterval;
      }
      Coord::txWGS84 pos(_laneLineCurve->getEnd());
      const auto vertex = pos.ToENU().ENU2D();
      // !!! the end point cannot be repeated
      const hadmap::txPoint p1(controlPointVec.back()[0], controlPointVec.back()[1], 0.0);
      const hadmap::txPoint p2(vertex.x(), vertex.y(), 0.0);
      if (Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(p1, p2) > std::numeric_limits<double>::epsilon()) {
        controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
      }
#else
      hadmap::PointVec resampleVec;
      _laneLineCurve->sample(FLAGS_LaneInfoSampleStep, resampleVec);
      controlPointVec.reserve(resampleVec.size() + 5);
      for (const auto &s : resampleVec) {
        auto vertex = Coord::txWGS84(s).ToENU().ENU2D();
        controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
      }
#endif
    } else if (CurveType::cubic_spline == curveType) {
      hadmap::PointVec vec_point;
      _laneLineCurve->getPoints(vec_point);
      const auto ptSize = vec_point.size();
      const Base::txFloat linklenght = _laneLineCurve->getLength();
      /*LOG_IF(WARNING, linklenght < 3.0) << "[txLaneSplineInfoImpl] start. "
          << TX_VARS(ptSize) << TX_VARS(linklenght)
          << TX_VARS(Utils::ToString(_laneLineCurve->getStart()));*/
      if (ptSize > 4 && linklenght > 3.0) {
        /*LOG(WARNING) << _StreamPrecision_ << "[txLaneSplineInfoImpl] "
            << TX_VARS(ptSize) << TX_VARS(linklenght)
            << TX_VARS_NAME(index_0, Coord::txWGS84(vec_point[0]))
            << TX_VARS_NAME(startPoint, Coord::txWGS84(_laneLineCurve->getStart()))
            << TX_VARS_NAME(index_size_1, Coord::txWGS84(vec_point[vec_point.size()-1]))
            << TX_VARS_NAME(endPoint, Coord::txWGS84(_laneLineCurve->getEnd()));*/

        const Base::txVec2 vertex = Coord::txWGS84(vec_point[0]).ToENU().ENU2D();
        controlPointVec.reserve(ptSize);
        controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
        // LOG(WARNING) << _StreamFormat_ << TX_VARS_NAME(index, 0) << txCurPos.Lon() << ", " << txCurPos.Lat();
        Base::txVec2 last_vertex = vertex;
        for (Base::txSize i = 1; i < ptSize; ++i) {
          /*LOG(WARNING) << _StreamPrecision_ << TX_VARS(ptSize) << TX_VARS(linklenght)
              << TX_VARS_NAME(idx, i) << TX_VARS(Coord::txWGS84(vec_point[i]));*/

          const Base::txVec2 vertex = Coord::txWGS84(vec_point[i]).ToENU().ENU2D();
          if ((last_vertex - vertex).squaredNorm() > 1.0 || (i + 1) == ptSize) {
            controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
            // LOG(WARNING) << _StreamFormat_ << TX_VARS_NAME(index, i) << txCurPos.Lon() << ", " << txCurPos.Lat();
            last_vertex = vertex;
          }
        }
      } else {
        controlPointVec.reserve(2);
        {
          /*Base::txHdMapLocateInfo txCurPos(vec_point[0]); txCurPos.GetLocalPos_2D();*/
          const Base::txVec2 vertex = Coord::txWGS84(vec_point.front()).ToENU().ENU2D();
          controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
          // LOG(WARNING) << _StreamFormat_ << "[txLaneSplineInfoImpl] " << txCurPos.Lon() << ", " << txCurPos.Lat() <<
          // std::endl;
        }
        {
          /*Base::txHdMapLocateInfo txCurPos(vec_point.back()); txCurPos.GetLocalPos_2D();*/
          const Base::txVec2 vertex = Coord::txWGS84(vec_point.back()).ToENU().ENU2D();
          controlPointVec.emplace_back(controlPoint{vertex.x(), vertex.y()});
          // LOG(WARNING) << _StreamFormat_ << "[txLaneSplineInfoImpl] " << txCurPos.Lon() << ", " << txCurPos.Lat() <<
          // std::endl;
        }
      }
      // LOG(WARNING) << "[txLaneSplineInfoImpl] end.";
    } else {
      LOG(WARNING) << "[Error] unsupport curve type.";
      return false;
    }
  } else {
    controlPointVec.reserve(2);
    try {
      /*Base::txHdMapLocateInfo startPoint = _laneLineCurve->getStart(); startPoint.GetLocalPos_2D();*/
      auto startVertex = Coord::txWGS84(_laneLineCurve->getStart()).ToENU().ENU2D();
      controlPointVec.emplace_back(controlPoint{startVertex.x(), startVertex.y()});

      /*Base::txHdMapLocateInfo endPoint = _laneLineCurve->getEnd(); endPoint.GetLocalPos_2D();*/
      auto endVertex = Coord::txWGS84(_laneLineCurve->getEnd()).ToENU().ENU2D();
      controlPointVec.emplace_back(controlPoint{endVertex.x(), endVertex.y()});
    } catch (const std::length_error &) {
      LOG(WARNING) << "[std::length_error] " << TX_VARS_NAME(length, (_laneLineCurve->getLength()))
                   << TX_VARS_NAME(point_size, _laneLineCurve->size());
      return false;
    } catch (const std::exception &) {
      LOG(WARNING) << "[std::exception] " << TX_VARS_NAME(length, (_laneLineCurve->getLength()))
                   << TX_VARS_NAME(point_size, _laneLineCurve->size());
      return false;
    } catch (...) {
      LOG(WARNING) << "[...] " << TX_VARS_NAME(length, (_laneLineCurve->getLength()))
                   << TX_VARS_NAME(point_size, _laneLineCurve->size());
      return false;
    }
  }
  return true;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::UpdateLaneDataByRoadRunTime(const hadmap::txLanePtr _lanePtr)
    TX_NOEXCEPT {
  return true;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::UpdateLaneLinksDataByRoadRunTime(
    const hadmap::txLaneLinkPtr lankPtr) TX_NOEXCEPT {
  return true;
  TX_MARK("assume concurrent map do not have current lane utility info.");
  if (Utils::IsLaneLinkValid(lankPtr)) {
    const hadmap::txCurve *pCuve = lankPtr->getGeometry();
    if (pCuve && pCuve->getLength() > 0.5) {
      const Base::txLaneUId fromLaneUid = lankPtr->fromTxLaneId();
      const Base::txLaneUId toLaneUid = lankPtr->toTxLaneId();
      {
        fromUid_toUid2lanelinkPtrType::accessor a;
        s_fromUidtoUid_lanelinkPtr.insert(a, std::make_pair(fromLaneUid, toLaneUid));
        a->second = lankPtr;
        a.release();
      }
      return true;
    } else {
      LOG(WARNING) << "not driving link." << TX_VARS(lankPtr->getId())
                   << (NonNull_Pointer(lankPtr) ? (pCuve->getLength()) : (0));
      return true;
      TX_MARK("return true;");
    }
  } else {
    LOG(FATAL) << "UpdateLaneLinksDataByRoadRunTime param error.";
    return false;
  }
}

Base::LocInfoSet HadmapCacheConCurrent::GetLaneNextLinkSet(const txLaneUId _laneUid) TX_NOEXCEPT {
  Base::LocInfoSet resLinkSet;
  uid2LaneLinkSetType::const_accessor ca;
  if (CallSucc(s_lane_from_linkset.find(ca, _laneUid))) {
    resLinkSet = ca->second;
    ca.release();
  }
  return resLinkSet;
}

hadmap::txLaneLinks HadmapCacheConCurrent::GetLaneNextLinks(const txLaneUId _laneUid) TX_NOEXCEPT {
  const Base::LocInfoSet locInfoSet = GetLaneNextLinkSet(_laneUid);
  hadmap::txLaneLinks refVec;
  refVec.reserve(locInfoSet.size());

  for (const Base::Info_Lane_t &link_locInfo : locInfoSet) {
    if (CallFail(IsLinkIdInBlackList(link_locInfo))) {
      /*LOG(WARNING) << "[GetLaneNextLinks]" << TX_VARS(link_locInfo.onLinkFromLaneUid) <<
       * TX_VARS(link_locInfo.onLinkToLaneUid);*/
      refVec.emplace_back(GetTxLaneLinkPtr(link_locInfo.onLinkFromLaneUid, link_locInfo.onLinkToLaneUid));
    }
  }
  /*LOG(WARNING) << "[GetLaneNextLinks]" << TX_VARS(_laneUid) << TX_VARS(locInfoSet.size()) << TX_VARS(refVec.size());*/
  return refVec;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetLaneLinkToLaneUidSet(const Base::Info_Lane_t &_link_locInfo)
    TX_NOEXCEPT {
  std::set<Base::txLaneUId> resLaneUidSet;
  lanelinkLocInfo2LaneSetType::const_accessor ca;
  if (CallSucc(s_link_to_laneset.find(ca, _link_locInfo))) {
    resLaneUidSet = ca->second;
    ca.release();
  }
  return resLaneUidSet;
}

Base::LocInfoSet HadmapCacheConCurrent::GetLanePrevLinkSet(const txLaneUId _laneUid) TX_NOEXCEPT {
  Base::LocInfoSet resLinkSet;
  uid2LaneLinkSetType::const_accessor ca;
  if (CallSucc(s_lane_to_linkset.find(ca, _laneUid))) {
    resLinkSet = ca->second;
    ca.release();
  }
  return resLinkSet;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetLaneLinkFromLaneUidSet(const Base::Info_Lane_t &_link_locInfo)
    TX_NOEXCEPT {
  std::set<Base::txLaneUId> resLaneUidSet;
  lanelinkLocInfo2LaneSetType::const_accessor ca;
  if (CallSucc(s_link_from_laneset.find(ca, _link_locInfo))) {
    resLaneUidSet = ca->second;
    ca.release();
  }
  return resLaneUidSet;
}

HadmapCacheConCurrent::txFloat HadmapCacheConCurrent::GetLaneLength(const txLaneUId &_laneUid) TX_NOEXCEPT {
  uid2LengthType::const_accessor ca;
  if (CallFail(s_uid2LaneLength.find(ca, _laneUid))) {
    ca.release();
    const hadmap::txLanePtr resLanePtr = GetTxLanePtr(_laneUid);
    txFloat resLaneLength = 0.0;
    if (Utils::IsLaneValid(resLanePtr)) {
      resLaneLength = resLanePtr->getLength();
    }
    {
      uid2LengthType::accessor a;
      s_uid2LaneLength.insert(a, _laneUid);
      a->second = resLaneLength;
      a.release();
    }
    return resLaneLength;
  } else {
    return (*ca).second;
  }
}

std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> HadmapCacheConCurrent::GetLaneMark_LR(const hadmap::txLanePtr lanePtr)
    TX_NOEXCEPT {
  uid2LaneMarkType::const_accessor ca;
  const Base::txLaneUId _laneUid = lanePtr->getTxLaneId();
  if (CallFail(s_uid2LaneMark.find(ca, _laneUid))) {
    ca.release();

    std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> lane_mark =
        std::make_pair(hadmap::LANE_MARK_None, hadmap::LANE_MARK_None);
#if __TX_Mark__("20211223")
    {
      hadmap::txLaneBoundaryPtr laneLeftBoundaryPtr = lanePtr->getLeftBoundary();
      if (NonNull_Pointer(laneLeftBoundaryPtr)) {
        std::get<0>(lane_mark) = (laneLeftBoundaryPtr->getLaneMark());
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) << TX_VARS(lanePtr->getLeftBoundaryId()) <<
        // std::hex << "Ox" << (laneLeftBoundaryPtr->getLaneMark());
      } else {
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) <<
        // TX_COND(NonNull_Pointer(laneLeftBoundaryPtr));
      }
    }
    {
      hadmap::txLaneBoundaryPtr laneRightBoundaryPtr = lanePtr->getRightBoundary();
      if (NonNull_Pointer(laneRightBoundaryPtr)) {
        std::get<1>(lane_mark) = (laneRightBoundaryPtr->getLaneMark());
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) << TX_VARS(lanePtr->getRightBoundaryId()) <<
        // std::hex << "Ox" << (laneRightBoundaryPtr->getLaneMark());
      } else {
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) <<
        // TX_COND(NonNull_Pointer(laneRightBoundaryPtr));
      }
    }
#endif
    {
      uid2LaneMarkType::accessor a;
      s_uid2LaneMark.insert(a, _laneUid);
      a->second = lane_mark;
      a.release();
    }
    return lane_mark;
  } else {
    return (*ca).second;
  }
}

std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> HadmapCacheConCurrent::GetLaneMark_LR(const txLaneUId &_laneUid)
    TX_NOEXCEPT {
  uid2LaneMarkType::const_accessor ca;
  if (CallFail(s_uid2LaneMark.find(ca, _laneUid))) {
    ca.release();
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_laneUid);
    std::pair<hadmap::LANE_MARK, hadmap::LANE_MARK> lane_mark =
        std::make_pair(hadmap::LANE_MARK_None, hadmap::LANE_MARK_None);
    {
      hadmap::txLaneBoundaryPtr laneLeftBoundaryPtr = nullptr;
      const auto opCode = hadmap::getBoundary(txMapHdr, lanePtr->getLeftBoundaryId(), laneLeftBoundaryPtr);
      if (Utils::CheckRetCode(opCode) && NonNull_Pointer(laneLeftBoundaryPtr)) {
        std::get<0>(lane_mark) = (laneLeftBoundaryPtr->getLaneMark());
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) << TX_VARS(lanePtr->getLeftBoundaryId()) <<
        // std::hex << "Ox" << (laneLeftBoundaryPtr->getLaneMark());
      } else {
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) <<
        // TX_COND(NonNull_Pointer(laneLeftBoundaryPtr));
      }
    }
    {
      hadmap::txLaneBoundaryPtr laneRightBoundaryPtr = nullptr;
      const auto opCode = hadmap::getBoundary(txMapHdr, lanePtr->getRightBoundaryId(), laneRightBoundaryPtr);
      if (Utils::CheckRetCode(opCode) && NonNull_Pointer(laneRightBoundaryPtr)) {
        std::get<1>(lane_mark) = (laneRightBoundaryPtr->getLaneMark());
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) << TX_VARS(lanePtr->getRightBoundaryId()) <<
        // std::hex << "Ox" << (laneRightBoundaryPtr->getLaneMark());
      } else {
        // LOG(INFO) << TX_VARS_NAME(laneUid, Utils::ToString(_laneUid)) <<
        // TX_COND(NonNull_Pointer(laneRightBoundaryPtr));
      }
    }
    {
      uid2LaneMarkType::accessor a;
      s_uid2LaneMark.insert(a, _laneUid);
      a->second = lane_mark;
      a.release();
    }
    return lane_mark;
  } else {
    return (*ca).second;
  }
}

HadmapCacheConCurrent::txFloat HadmapCacheConCurrent::GetLaneLinkLength(const Base::Info_Lane_t &_link_locInfo)
    TX_NOEXCEPT {
  lanelinkId2LengthType::const_accessor ca;
  if (CallFail(s_linkId2LinkLength.find(ca, _link_locInfo))) {
    ca.release();
    const hadmap::txLaneLinkPtr resLinkPtr =
        GetTxLaneLinkPtr(_link_locInfo.onLinkFromLaneUid, _link_locInfo.onLinkToLaneUid);
    txFloat resLinkLength = 0.0;
    if (Utils::IsLaneLinkValid(resLinkPtr)) {
      resLinkLength = resLinkPtr->getGeometry()->getLength();
    }
    {
      lanelinkId2LengthType::accessor a;
      s_linkId2LinkLength.insert(a, _link_locInfo);
      a->second = resLinkLength;
      a.release();
    }
    return resLinkLength;
  } else {
    return (*ca).second;
  }
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetNextLaneSetByLaneUid(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retLaneUidSet;
  uid2LaneUidSet::const_accessor ca;
  if (CallSucc(s_lane_next_laneset.find(ca, _laneUid))) {
    retLaneUidSet = ca->second;
    ca.release();
  }
  return retLaneUidSet;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetNextLaneSetByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  if (CallSucc(Utils::IsLaneValid(lanePtr))) {
    const Base::txLaneUId _laneUid = lanePtr->getTxLaneId();
    return GetNextLaneSetByLaneUid(_laneUid);
  }
  return std::set<Base::txLaneUId>();
}

hadmap::txLanes HadmapCacheConCurrent::GetNextLanesByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  if (CallSucc(Utils::IsLaneValid(lanePtr))) {
    const Base::txLaneUId _laneUid = lanePtr->getTxLaneId();
    return GetNextLanesByLaneUid(_laneUid);
  }
  return hadmap::txLanes();
}

hadmap::txLanes HadmapCacheConCurrent::GetNextLanesByLaneUid(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  const std::set<Base::txLaneUId> nextLaneSet = GetNextLaneSetByLaneUid(_laneUid);
  hadmap::txLanes nextLanes;
  nextLanes.reserve(nextLaneSet.size());
  for (const auto laneUid : nextLaneSet) {
    nextLanes.emplace_back(HdMap::HadmapCacheConCurrent::GetTxLanePtr(laneUid));
  }
  return nextLanes;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetPreLaneSetByLaneUid(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  std::set<Base::txLaneUId> retLaneUidSet;
  uid2LaneUidSet::const_accessor ca;
  if (CallSucc(s_lane_pre_laneset.find(ca, _laneUid))) {
    retLaneUidSet = ca->second;
    ca.release();
  }
  return retLaneUidSet;
}

std::set<Base::txLaneUId> HadmapCacheConCurrent::GetPreLaneSetByLaneUid(const hadmap::txLanePtr lanePtr) TX_NOEXCEPT {
  if (CallSucc(Utils::IsLaneValid(lanePtr))) {
    const Base::txLaneUId _laneUid = lanePtr->getTxLaneId();
    return GetPreLaneSetByLaneUid(_laneUid);
  }
  return std::set<Base::txLaneUId>();
}

void HadmapCacheConCurrent::add_dead_end_road(const Base::txLaneUId &laneUid, const txWGS84 &endPt) TX_NOEXCEPT {
  uid2HdMapLocateInfo::accessor a;
  s_dead_end_road_flag_map_dead_point.insert(a, laneUid);
  a->second = endPt;
  a.release();
}

Base::txBool HadmapCacheConCurrent::IsDeadEndRoad(const Base::txLaneUId &laneUid, txWGS84 &endPt) TX_NOEXCEPT {
  uid2HdMapLocateInfo::const_accessor ca;
  if (CallSucc(s_dead_end_road_flag_map_dead_point.find(ca, laneUid))) {
    endPt = ca->second;
    ca.release();
    return true;
  } else {
    return false;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::Make_dead_end_laneSet_by_laneUid(const hadmap::txLanePtr lanePtr)
    TX_NOEXCEPT {
  const auto laneUid = lanePtr->getTxLaneId();
  const auto &nextLaneSet = GetNextLaneSetByLaneUid(lanePtr);
  if (CallSucc(nextLaneSet.empty())) {
    TX_MARK("current lane do not have next laneSet.");
    const Base::LocInfoSet resLinkIdSet = GetLaneNextLinkSet(laneUid);
    if (CallSucc(resLinkIdSet.empty())) {
      TX_MARK("is dead end road");
      const txWGS84 dead_end_road_end_pos(lanePtr->getGeometry()->getEnd());
      Base::txInt nCnt = 0;
      std::queue<Base::txLaneUId> deadend_laneuid_queue;
      deadend_laneuid_queue.push(laneUid);

      while (CallFail(deadend_laneuid_queue.empty()) && (nCnt < 100 /*exp*/)) {
        ++nCnt;
        const Base::txLaneUId curLaneUid = deadend_laneuid_queue.front();
        deadend_laneuid_queue.pop();
        add_dead_end_road(curLaneUid, dead_end_road_end_pos);
        const auto &preLaneSet = GetPreLaneSetByLaneUid(curLaneUid);
        for (const auto &preLaneUid : preLaneSet) {
          deadend_laneuid_queue.push(preLaneUid);
        }
      }
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
}

txLaneInfoInterfacePtr HadmapCacheConCurrent::RegisterGeomtryInfoSyncRunTime(const Base::Info_Lane_t &_lane_loc_info)
    TX_NOEXCEPT {
  if (CallSucc(_lane_loc_info.IsOnLane())) {
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      return RegisterLaneInfoSyncRunTime(lanePtr);
    } else {
      LOG(FATAL) << "GetTxLanePtr failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
      return nullptr;
    }
  } else {
    hadmap::txLaneLinkPtr lanelinkPtr =
        GetTxLaneLinkPtr(_lane_loc_info.onLinkFromLaneUid, _lane_loc_info.onLinkToLaneUid);
    if (NonNull_Pointer(lanelinkPtr)) {
      return RegisterLaneLinkInfoSyncRunTime(lanelinkPtr);
    } else {
      LOG(FATAL) << "GetTxLaneLinkPtr failure. "
                 << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(_lane_loc_info.onLinkFromLaneUid))
                        TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(_lane_loc_info.onLinkToLaneUid));
      return nullptr;
    }
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetDir(const Base::Info_Lane_t &_lane_loc_info,
                                                            const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();

    txLaneInfoInterfacePtr laneInfoPtr = RegisterGeomtryInfoSyncRunTime(_lane_loc_info);
    if (NonNull_Pointer(laneInfoPtr)) {
      return (laneInfoPtr->GetLaneDir(_s));
    } else {
      LOG(FATAL) << "RegisterGeomInfoSync failure. Info_Lane_t = " << _lane_loc_info;
      return txVec3(1.0, 0.0, 0.0);
    }
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

Base::txFloat HadmapCacheConCurrent::GetHdMapAlt(const Base::Info_Lane_t &_lane_loc_info,
                                                 const txFloat _s) TX_NOEXCEPT {
  if (_lane_loc_info.IsOnLane()) {
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      const txFloat laneLen = lanePtr->getLength();
      if (_s >= 0.0 && _s < laneLen) {
        return (lanePtr->getGeometry()->getPoint(_s)).z;
      } else {
        return lanePtr->getGeometry()->getEnd().z;
      }
    }
  } else if (_lane_loc_info.IsOnLaneLink()) {
    hadmap::txLaneLinkPtr linkPtr = GetTxLaneLinkPtr(_lane_loc_info.onLinkFromLaneUid, _lane_loc_info.onLinkToLaneUid);
    if (NonNull_Pointer(linkPtr)) {
      const txFloat linkLen = linkPtr->getGeometry()->getLength();
      if (_s >= 0.0 && _s < linkLen) {
        return linkPtr->getGeometry()->getPoint(_s).z;
      } else {
        return linkPtr->getGeometry()->getEnd().z;
      }
    }
  }
  return 0.0;
}

Coord::txWGS84 HadmapCacheConCurrent::GetPos(const Base::Info_Lane_t &_lane_loc_info, const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    txLaneInfoInterfacePtr laneInfoPtr = RegisterGeomtryInfoSyncRunTime(_lane_loc_info);
    if (NonNull_Pointer(laneInfoPtr)) {
      return txENU(laneInfoPtr->GetLocalPos(_s)).ToWGS84();
    } else {
      LOG(FATAL) << "RegisterGeomInfoSync failure. Info_Lane_t = " << _lane_loc_info;
      return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
    }
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetLaneDir(const Base::Info_Lane_t &_lane_loc_info,
                                                                const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return (laneInfoPtr->GetLaneDir(_s));
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txVec3(1.0, 0.0, 0.0);
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetLaneDir(const Base::txLaneUId &_lane_uid,
                                                                const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(_lane_uid);
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return (laneInfoPtr->GetLaneDir(_s));
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txVec3(1.0, 0.0, 0.0);
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetLaneDir(const hadmap::txLanePtr lanePtr,
                                                                const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(lanePtr->getTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return (laneInfoPtr->GetLaneDir(_s));
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txVec3(1.0, 0.0, 0.0);
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetLaneLinkDir(const Base::Info_Lane_t &_lane_loc_info,
                                                                    const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLaneLinkPtr lanelinkPtr =
        GetTxLaneLinkPtr(_lane_loc_info.onLinkFromLaneUid, _lane_loc_info.onLinkToLaneUid);
    if (NonNull_Pointer(lanelinkPtr)) {
      txLaneInfoInterfacePtr linkInfoPtr = RegisterLaneLinkInfoSyncRunTime(lanelinkPtr);
      if (NonNull_Pointer(linkInfoPtr)) {
        return (linkInfoPtr->GetLaneDir(_s));
      }
    }
    LOG(FATAL) << "RegisterLaneLinkSync failure. _linkId = "
               << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(_lane_loc_info.onLinkFromLaneUid))
                      TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(_lane_loc_info.onLinkToLaneUid));
    return txVec3(1.0, 0.0, 0.0);
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::GetLaneLinkDir(const hadmap::txLaneLinkPtr lanelinkPtr,
                                                                    const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(lanelinkPtr->getId(), lanelinkPtr->fromTxLaneId(), lanelinkPtr->toTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    if (NonNull_Pointer(lanelinkPtr)) {
      txLaneInfoInterfacePtr linkInfoPtr = RegisterLaneLinkInfoSyncRunTime(lanelinkPtr);
      if (NonNull_Pointer(linkInfoPtr)) {
        return (linkInfoPtr->GetLaneDir(_s));
      }
    }
    LOG(FATAL) << "RegisterLaneLinkSync failure. _linkId = "
               << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(_lane_loc_info.onLinkFromLaneUid))
                      TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(_lane_loc_info.onLinkToLaneUid));
    return txVec3(1.0, 0.0, 0.0);
  } else {
    return (ca->second->GetLaneDir(_s));
  }
}

Base::txBool HadmapCacheConCurrent::GetLanePos(const Base::Info_Lane_t &_lane_loc_info, const txFloat _s,
                                               txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLanePos(_lane_loc_info, _s);
  return true;
}

Coord::txWGS84 HadmapCacheConCurrent::GetLanePos(const Base::Info_Lane_t &_lane_loc_info,
                                                 const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return txENU(laneInfoPtr->GetLocalPos(_s)).ToWGS84();
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

Base::txBool HadmapCacheConCurrent::GetLanePos(const Base::txLaneUId &_laneUid, const txFloat _s,
                                               txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLanePos(_laneUid, _s);
  return true;
}

Coord::txWGS84 HadmapCacheConCurrent::GetLanePos(const Base::txLaneUId &_laneUid, const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(_laneUid);
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLanePtr lanePtr = GetTxLanePtr(_lane_loc_info.onLaneUid);
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return txENU(laneInfoPtr->GetLocalPos(_s)).ToWGS84();
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

Base::txBool HadmapCacheConCurrent::GetLanePos(const hadmap::txLanePtr lanePtr, const txFloat _s,
                                               txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLanePos(lanePtr, _s);
  return true;
}

Coord::txWGS84 HadmapCacheConCurrent::GetLanePos(const hadmap::txLanePtr lanePtr, const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(lanePtr->getTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    if (NonNull_Pointer(lanePtr)) {
      txLaneInfoInterfacePtr laneInfoPtr = RegisterLaneInfoSyncRunTime(lanePtr);
      if (NonNull_Pointer(laneInfoPtr)) {
        return txENU(laneInfoPtr->GetLocalPos(_s)).ToWGS84();
      }
    }

    LOG(FATAL) << "RegisterLaneSync failure. laneUid = " << Utils::ToString(_lane_loc_info.onLaneUid);
    return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

Base::txBool HadmapCacheConCurrent::GetLaneLinkPos(const hadmap::txLaneLinkPtr &_lanelinkPtr, const txFloat _s,
                                                   txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLaneLinkPos(_lanelinkPtr, _s);
  return true;
}

Coord::txWGS84 HadmapCacheConCurrent::GetLaneLinkPos(const hadmap::txLaneLinkPtr &_lanelinkPtr,
                                                     const txFloat _s) TX_NOEXCEPT {
  const Base::Info_Lane_t _lane_loc_info(_lanelinkPtr->getId(), _lanelinkPtr->fromTxLaneId(),
                                         _lanelinkPtr->toTxLaneId());
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    if (NonNull_Pointer(_lanelinkPtr)) {
      txLaneInfoInterfacePtr linkInfoPtr = RegisterLaneLinkInfoSyncRunTime(_lanelinkPtr);
      if (NonNull_Pointer(linkInfoPtr)) {
        return txENU(linkInfoPtr->GetLocalPos(_s)).ToWGS84();
      }
    }
    LOG(FATAL) << "RegisterLaneSync failure. _linkId = "
               << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(_lane_loc_info.onLinkFromLaneUid))
                      TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(_lane_loc_info.onLinkToLaneUid));
    return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

Base::txBool HadmapCacheConCurrent::GetLaneLinkPos(const Base::Info_Lane_t &_lane_loc_info, const txFloat _s,
                                                   txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLaneLinkPos(_lane_loc_info, _s);
  return true;
}

Coord::txWGS84 HadmapCacheConCurrent::GetLaneLinkPos(const Base::Info_Lane_t &_lane_loc_info,
                                                     const txFloat _s) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallFail(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    ca.release();
    hadmap::txLaneLinkPtr _lanelinkPtr =
        GetTxLaneLinkPtr(_lane_loc_info.onLinkFromLaneUid, _lane_loc_info.onLinkToLaneUid);
    if (NonNull_Pointer(_lanelinkPtr)) {
      txLaneInfoInterfacePtr linkInfoPtr = RegisterLaneLinkInfoSyncRunTime(_lanelinkPtr);
      if (NonNull_Pointer(linkInfoPtr)) {
        return txENU(linkInfoPtr->GetLocalPos(_s)).ToWGS84();
      }
    }
    LOG(FATAL) << "RegisterLaneSync failure. _linkId = "
               << TX_VARS_NAME(onLinkFromLaneUid, Utils::ToString(_lane_loc_info.onLinkFromLaneUid))
                      TX_VARS_NAME(onLinkToLaneUid, Utils::ToString(_lane_loc_info.onLinkToLaneUid));
    return txENU(txVec3(0.0, 0.0, 0.0)).ToWGS84();
  } else {
    return txENU(ca->second->GetLocalPos(_s)).ToWGS84();
  }
}

HadmapCacheConCurrent::txVec3 HadmapCacheConCurrent::OffsetLocalPosWithLaneTangent(const txVec3 &vPos,
                                                                                   const txVec3 &vLaneDir,
                                                                                   const txFloat _offset) TX_NOEXCEPT {
  txVec3 vTargetDir = Utils::VetRotVecByDegree(vLaneDir, Unit::txDegree::MakeDegree(90.0), Utils::Axis_Enu_Up());
  return vPos + vTargetDir * _offset;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::GetLonLatFromSTonLaneWithOffset(const Base::txLaneUId _laneUid,
                                                                                     const txFloat _s, const txFloat _t,
                                                                                     txWGS84 &resPos) TX_NOEXCEPT {
  const txVec3 laneDir = GetLaneDir(_laneUid, _s);
  resPos = GetLanePos(_laneUid, _s);
  const txVec3 offsetPos = OffsetLocalPosWithLaneTangent(resPos.ToENU().ENU(), laneDir, _t);
  resPos.FromENU(offsetPos);
  return true;
}

const std::set<Base::txRoadID> HadmapCacheConCurrent::GetPreRoadByRoadId(const Base::txRoadID _roadId) TX_NOEXCEPT {
  road2RoadSetType::const_accessor ca;

  if (CallFail(s_RoadId2PreRoadSet.find(ca, _roadId))) {
    return std::set<Base::txRoadID>();
  } else {
    return (ca->second);
  }
}

const std::set<Base::txRoadID> HadmapCacheConCurrent::GetNextRoadByRoadId(const Base::txRoadID _roadId) TX_NOEXCEPT {
  road2RoadSetType::const_accessor ca;
  if (CallFail(s_RoadId2NextRoadSet.find(ca, _roadId))) {
    return std::set<Base::txRoadID>();
  } else {
    return (ca->second);
  }
}

Base::LocInfoSet HadmapCacheConCurrent::GetLaneLinkByFromToRoadId4Route(const Base::txRoadID _fromRoadId,
                                                                        const Base::txRoadID _toRoadId) TX_NOEXCEPT {
  roadIdPair2LaneLinkSetType::const_accessor ca;
  if (CallFail(s_fromRoadIdToRoadId2LaneLinkIdSet4Route.find(ca, std::make_pair(_fromRoadId, _toRoadId)))) {
    hadmap::txLaneLinks resLinks;
    hadmap::getLaneLinks(pMapHandle, _fromRoadId, _toRoadId, resLinks);

    Base::LocInfoSet resLinksId;
    for (const auto &refLaneLink : resLinks) {
      // if (Utils::IsLaneLinkValid(refLaneLink))
      {
        resLinksId.insert(
            Base::Info_Lane_t(refLaneLink->getId(), refLaneLink->fromTxLaneId(), refLaneLink->toTxLaneId()));
      }
    }
    {
      roadIdPair2LaneLinkSetType::accessor a;
      s_fromRoadIdToRoadId2LaneLinkIdSet4Route.insert(a, std::make_pair(_fromRoadId, _toRoadId));
      a->second = resLinksId;
      a.release();
    }
    return resLinksId;
  } else {
    return (ca->second);
  }
}

Base::LocInfoSet HadmapCacheConCurrent::GetLaneLinkByFromToRoadId(const Base::txRoadID _fromRoadId,
                                                                  const Base::txRoadID _toRoadId) TX_NOEXCEPT {
  roadIdPair2LaneLinkSetType::const_accessor ca;
  if (CallFail(s_fromRoadIdToRoadId2LaneLinkIdSet.find(ca, std::make_pair(_fromRoadId, _toRoadId)))) {
    hadmap::txLaneLinks resLinks;
    hadmap::getLaneLinks(pMapHandle, _fromRoadId, _toRoadId, resLinks);

    Base::LocInfoSet resLinksId;
    for (const auto &refLaneLink : resLinks) {
      if (Utils::IsLaneLinkValid(refLaneLink)) {
        resLinksId.insert(
            Base::Info_Lane_t(refLaneLink->getId(), refLaneLink->fromTxLaneId(), refLaneLink->toTxLaneId()));
      }
    }
    {
      roadIdPair2LaneLinkSetType::accessor a;
      s_fromRoadIdToRoadId2LaneLinkIdSet.insert(a, std::make_pair(_fromRoadId, _toRoadId));
      a->second = resLinksId;
      a.release();
    }
    return resLinksId;
  } else {
    return (ca->second);
  }
}

void HadmapCacheConCurrent::init_black_list() TX_NOEXCEPT { s_locInfo_blacklist.clear(); }

void HadmapCacheConCurrent::add_link_black_list(const Base::Info_Lane_t &link_locInfo) TX_NOEXCEPT {
  s_locInfo_blacklist.insert(link_locInfo);
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::IsLinkIdInBlackList(const Base::Info_Lane_t &link_locInfo)
    TX_NOEXCEPT {
  if (s_locInfo_blacklist.end() != s_locInfo_blacklist.find(link_locInfo)) {
    return true;
  } else {
    return false;
  }
}

hadmap::txLanePtr HadmapCacheConCurrent::GetLaneForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                        Base::txFloat &distanceCurve,
                                                        Base::txFloat &distancePedal) TX_NOEXCEPT {
  static Base::txFloat unuse = 0.0;
  return GetLaneForInit(lon, lat, distanceCurve, distancePedal, unuse);
}

hadmap::txLanePtr HadmapCacheConCurrent::GetLaneForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                        Base::txFloat &distanceCurve, Base::txFloat &distancePedal,
                                                        Base::txFloat &_lOffset) TX_NOEXCEPT {
  hadmap::txPoint txPt(lon, lat, FLAGS_HdMap_3D_Alt), pedal;
  hadmap::txLanePtr pLane = nullptr;
  int nRet = hadmap::getLane(pMapHandle, txPt, pLane);

  if (!pLane) {
    distanceCurve = 0.0;
    distancePedal = 0.0;
    _lOffset = 0.0;
    LOG(WARNING) << "get lane error!" << Utils::ToString(txPt);
    return nullptr;
  } else {
    const Base::txVec2 pt_local = txENU().FromWGS84(lon, lat, 0.0).ENU2D();
    const Base::txVec2 ptEnd_local = txENU().FromWGS84(pLane->getGeometry()->getEnd()).ENU2D();
    const Base::txFloat dDist = (pt_local - ptEnd_local).norm();
    const Base::txFloat lane_len = pLane->getLength();
    // LOG(WARNING) << TX_VARS(lon) << TX_VARS(lat) << TX_VARS(dDist) << TX_VARS(lane_len) <<
    // TX_VARS(pLane->getTxLaneId());
    if ((2.0 * dDist) < lane_len) { /*if (dDist < 0.1)*/
      hadmap::txLanes nextLanes;
      hadmap::getNextLanes(pMapHandle, pLane, nextLanes);
      if (nextLanes.size() > 0) {
        for (int i = 0; i < nextLanes.size(); ++i) {
          if (nextLanes[i]->getLength() > 0) {
            const Base::txVec2 ptStart_local = txENU().FromWGS84(nextLanes[i]->getGeometry()->getStart()).ENU2D();
            const Base::txFloat dStartDist = (ptStart_local - pt_local).norm();
            LOG(WARNING) << TX_VARS(nextLanes[i]->getTxLaneId()) << TX_VARS(dStartDist);
            if (dStartDist < 1.0) {
              pLane = nextLanes[i];
              distanceCurve = 0.0;

              LOG(INFO) << "hadmap::getLane() return the previous lane, the real next lane is : txLaneId : "
                        << Utils::ToString(pLane->getTxLaneId());
              break;
            } else {
              LOG(WARNING) << TX_VARS(dStartDist);
            }
          }
        }
      }
    } else {
      // LOG(WARNING) << TX_VARS(dDist);
    }
    double yaw = 0.0;
    distancePedal = 0.0;
    distanceCurve = 0.0;

    /*pLane->getGeometry()->xy2sl(lon, lat, distanceCurve, distancePedal, yaw);
    hadmap::txPoint pt2 = pLane->getGeometry()->getPoint(distanceCurve);
    LOG(INFO) << "pLane->getGeometry() " << TX_VARS(distanceCurve) << TX_VARS_NAME(distPt, Utils::ToString(pt2));*/

    pLane->xy2sl(lon, lat, distanceCurve, distancePedal, yaw);
    hadmap::txPoint pt1 = pLane->getGeometry()->getPoint(distanceCurve);
    // LOG(INFO) << "plane " << TX_VARS(distanceCurve) << TX_VARS_NAME(distPt, Utils::ToString(pt1));
  }
  _lOffset = distancePedal;
  distancePedal = std::fabs(distancePedal);
  return pLane;
}

hadmap::txLaneLinkPtr HadmapCacheConCurrent::GetLaneLinkForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                                Base::txFloat &distanceCurve,
                                                                Base::txFloat &distancePedal) TX_NOEXCEPT {
  static Base::txFloat unuse = 0.0;
  return GetLaneLinkForInit(lon, lat, distanceCurve, distancePedal, unuse);
}

hadmap::txLaneLinkPtr HadmapCacheConCurrent::GetLaneLinkForInit(const Base::txFloat lon, const Base::txFloat lat,
                                                                Base::txFloat &distanceCurve,
                                                                Base::txFloat &distancePedal,
                                                                Base::txFloat &_lOffset) TX_NOEXCEPT {
  _lOffset = 0.0;
  hadmap::txPoint loc(lon, lat, FLAGS_HdMap_3D_Alt), pedal;
  hadmap::txLaneLinkPtr pLaneLink = nullptr;
  int nRet = hadmap::getLaneLink(pMapHandle, loc, pLaneLink);
  if (TX_HADMAP_DATA_OK != nRet || !pLaneLink) {
    LOG(WARNING) << "get lane error!";
    return nullptr;
  }

  if (pLaneLink) {
    distanceCurve = (dynamic_cast<const hadmap::txLineCurve *>(pLaneLink->getGeometry()))
                        ->getPassedDistance(loc, pedal, hadmap::CoordType::COORD_WGS84);

    double s = 0.0, yaw = 0.0;
    distancePedal = 0.0;
    pLaneLink->getGeometry()->xy2sl(lon, lat, s, distancePedal, yaw);
    LOG(WARNING) << TX_VARS_NAME(route_start, Utils::ToString(loc)) << TX_VARS_NAME(st_t, distancePedal);
  } else {
    distanceCurve = 0.0;
    distancePedal = 0.0;
  }
  _lOffset = distancePedal;
  distancePedal = std::fabs(distancePedal);
  return pLaneLink;
}

HadmapCacheConCurrent::roadId2LaneLinkSetType::value_type::second_type HadmapCacheConCurrent::GetLaneLinksFromRoad(
    const Base::txRoadID _from_roadId) TX_NOEXCEPT {
  roadId2LaneLinkSetType::const_accessor ca;
  if (CallFail(s_fromRoadId2LaneLinkSet.find(ca, _from_roadId))) {
    ca.release();
    hadmap::txLaneLinks retLinks;
    hadmap::getLaneLinks(pMapHandle, _from_roadId, ROAD_PKID_INVALID, retLinks);
    roadId2LaneLinkSetType::value_type::second_type retLinkIdSet;
    for (const auto linkPtr : retLinks) {
      retLinkIdSet.insert(Base::Info_Lane_t(linkPtr->getId(), linkPtr->fromTxLaneId(), linkPtr->toTxLaneId()));
    }
    {
      roadId2LaneLinkSetType::accessor a;
      s_fromRoadId2LaneLinkSet.insert(a, _from_roadId);
      a->second = retLinkIdSet;
      a.release();
    }
    return retLinkIdSet;
  } else {
    return ca->second;
  }
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::Get_LAL_Lane_By_S(const Base::Info_Lane_t &_lane_loc_info,
                                                                       const txFloat _s, txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLanePos(_lane_loc_info, _s);
  return true;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::Get_LAL_Lane_By_S(const Base::txLaneUId &_lane_uid,
                                                                       const txFloat _s, txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLanePos(_lane_uid, _s);
  return true;
}

HadmapCacheConCurrent::txBool HadmapCacheConCurrent::Get_LAL_LaneLink_By_S(const Base::Info_Lane_t &_lane_loc_info,
                                                                           const txFloat _s,
                                                                           txWGS84 &resPos) TX_NOEXCEPT {
  resPos = GetLaneLinkPos(_lane_loc_info, _s);
  return true;
}

Base::txBool HadmapCacheConCurrent::AdsorbLaneInfo(Coord::txWGS84 aPos, Base::Info_Lane_t &locInfo, Base::txFloat &_S,
                                                   Base::txFloat &_TT) TX_NOEXCEPT {
  const hadmap::txPoint gpsPt = aPos.WGS84_Adsorb();
  hadmap::txLanePtr initLane = nullptr;
  hadmap::txLaneLinkPtr initLaneLink = nullptr;
  hadmap::getLane(txMapHdr, gpsPt, initLane);
  hadmap::getLaneLink(txMapHdr, gpsPt, initLaneLink);

  /*if (NonNull_Pointer(initLaneLink) && NonNull_Pointer(initLaneLink->getGeometry())) {
      Base::txFloat _yaw = 0.0;
      locInfo.FromLaneLink(initLaneLink->getId(), initLaneLink->fromTxLaneId(), initLaneLink->toTxLaneId());
      initLaneLink->getGeometry()->xy2sl(aPos.Lon(), aPos.Lat(), _S, _TT, _yaw);
      return true;
  } else*/
  if (NonNull_Pointer(initLane) && NonNull_Pointer(initLane->getGeometry())) {
    Base::txFloat _yaw = 0.0;
    locInfo.FromLane(initLane->getTxLaneId());
    initLane->getGeometry()->xy2sl(aPos.Lon(), aPos.Lat(), _S, _TT, _yaw);
    return true;
  } else {
    return false;
  }
}

Base::txBool HadmapCacheConCurrent::IsRoadNearJunction(const txLaneUId _laneUid) TX_NOEXCEPT {
  hadmap::txLaneLinks nextLaneLinks = GetLaneNextLinks(_laneUid);

  if (_NonEmpty_(nextLaneLinks)) {
    for (auto curLaneLinkPtr : nextLaneLinks) {
      if (curLaneLinkPtr->getGeometry()->getLength() > 0.5) {
        return true;
      }
    }
  }
  return false;
}

Base::txString HadmapCacheConCurrent::GetLaneLinkControlPhase(const hadmap::txLaneLinkPtr _linkptr) TX_NOEXCEPT {
  Base::txString phase = "";
  if (_linkptr->getGeometry() && !_linkptr->getGeometry()->empty()) {
    const hadmap::txLineCurve *pLinecurve = dynamic_cast<const hadmap::txLineCurve *>(_linkptr->getGeometry());
    double startYaw = pLinecurve->yaw(0);
    int nSize = pLinecurve->size();
    double endYaw = pLinecurve->yaw(pLinecurve->size() - 1);
    double yawDiff = (endYaw - startYaw);
    if (yawDiff < -180.0) {
      yawDiff += 360.0;
    }
    if (yawDiff > 180) {
      yawDiff -= 360.0;
    }
    if (std::abs(yawDiff) < 45.0) {
      phase = "T";
    } else if (yawDiff >= 45 && yawDiff < 150) {
      phase = "L";
    } else if (yawDiff >= 150 && yawDiff <= 180) {
      phase = "L0";
    } else if (yawDiff > (-150) && yawDiff <= (-45)) {
      phase = "R";
    } else {
      phase = "R0";
    }
  }
  return phase;
}

#if 0
txLaneInfoInterfacePtr HadmapCacheConCurrent::GetLaneLinkInfoById(const txLaneLinkID& _lanelinkId) TX_NOEXCEPT {
  return GetGeomInfoById(MakeLaneLocInfoByLinkId(_lanelinkId));
    /*uid2LaneLinkInfoType::const_accessor ca;
    if (CallSucc(s_uid2LaneLinkInfo.find(ca, _lanelinkId))) {
        return ca->second;
    } else {
        return nullptr;
    }*/
}
#endif

txLaneInfoInterfacePtr HadmapCacheConCurrent::GetLaneInfoByUid(const txLaneUId &_laneUid) TX_NOEXCEPT {
  return GetGeomInfoById(Base::Info_Lane_t(_laneUid));
}

txLaneInfoInterfacePtr HadmapCacheConCurrent::GetGeomInfoById(const Base::Info_Lane_t &_lane_loc_info) TX_NOEXCEPT {
  laneLocInfo2LaneInfoType::const_accessor ca;
  if (CallSucc(s_lanelocinfo2GeomInfo.find(ca, _lane_loc_info))) {
    return ca->second;
  } else {
    return nullptr;
  }
}

txLaneInfoInterface::LaneShape HadmapCacheConCurrent::GetGeomShape(const Base::Info_Lane_t &_lane_loc_info)
    TX_NOEXCEPT {
  laneLocInfo2LaneShapeType::const_accessor ca;
  if (s_laneLocInfo2LaneShapeType.find(ca, _lane_loc_info)) {
    return ca->second;
  } else {
    return txLaneInfoInterface::LaneShape::sLine;
  }
}

txLaneInfoInterfacePtr HadmapCacheConCurrent::GetRoadGeomInfoById(const Base::Info_Road_t &_road_loc_info) TX_NOEXCEPT {
  RoadLocInfo2GeomInfoType::const_accessor ca;
  if (CallSucc(s_roadlocinfo2GeomInfo.find(ca, _road_loc_info))) {
    return ca->second;
  } else {
    return nullptr;
  }
}

Base::Info_Lane_t HadmapCacheConCurrent::MakeLaneLocInfoByLinkId_uncheck(const hadmap::txLaneLinkPtr _linkptr)
    TX_NOEXCEPT {
  return Base::Info_Lane_t(_linkptr->getId(), _linkptr->fromTxLaneId(), _linkptr->toTxLaneId());
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(HadmapCacheConCurrent::HadmapFilter::link_from_to, fromLaneUid,
                                                toLaneUid);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(HadmapCacheConCurrent::HadmapFilter::link_from_to_road, fromRoadId,
                                                toRoadId);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(HadmapCacheConCurrent::HadmapFilter::link_from_to_distance, fromLaneUid,
                                                toLaneUid, distance);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    HadmapCacheConCurrent::HadmapFilter::cfg_special_hdmap::expand_vision_node, source_link, expand_links);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(HadmapCacheConCurrent::HadmapFilter::cfg_special_hdmap,
                                                HDMapNameWithExtern, blacklist_from_to_laneUid,
                                                blacklist_from_to_roadId, refuse_switch_dual_direction_lane,
                                                refuse_switch_left_lane, refuse_switch_right_lane,
                                                refuse_switch_dual_direction_road, refuse_switch_left_road,
                                                refuse_switch_right_road, expand_vision);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(HadmapCacheConCurrent::HadmapFilter, hdmap_advanced_cfg);

void HadmapCacheConCurrent::HadmapFilter::Parse(const Base::txString map_name_with_extern) TX_NOEXCEPT {
  LogInfo << TX_VARS_NAME(HadmapFilter.cfg_size, hdmap_advanced_cfg.size()) << Str();

  for (auto itr = std::begin(hdmap_advanced_cfg); itr != std::end(hdmap_advanced_cfg);) {
    if (map_name_with_extern != (*itr).HDMapNameWithExtern) {
      itr = hdmap_advanced_cfg.erase(itr);
    } else {
      ++itr;
    }
  }

  if (hdmap_advanced_cfg.size() > 0) {
    LogInfo << "has cfg info. " << hdmap_advanced_cfg.front();
  } else {
    LOG(WARNING) << "cfg do not have map configure. " << map_name_with_extern;
  }
}

Base::txString HadmapCacheConCurrent::HadmapFilter::HdMapName() const TX_NOEXCEPT {
  if (IsValid()) {
    return hdmap_advanced_cfg.front().HDMapNameWithExtern;
  } else {
    return "HadmapFilter is invalid.";
  }
}

/*std::vector< std::tuple<Base::txLaneUId, Base::txLaneUId> >
 * HadmapCacheConCurrent::HadmapFilter::BlacklistFromToLaneUid() const TX_NOEXCEPT*/
std::vector<Base::Info_Lane_t> HadmapCacheConCurrent::HadmapFilter::BlacklistLocInfo() const TX_NOEXCEPT {
  std::vector<Base::Info_Lane_t> res;
  if (IsValid()) {
    const auto &refblacklist_from_to_laneUid = hdmap_advanced_cfg.front().blacklist_from_to_laneUid;
    // LOG(WARNING) << TX_VARS(refblacklist_from_to_laneUid.size());
    for (const auto &ref_from_to : refblacklist_from_to_laneUid) {
      // LOG(WARNING) << TX_VARS_NAME(fromLaneUid, ref_from_to.fromLaneUid) << TX_VARS_NAME(toLaneUid,
      // ref_from_to.toLaneUid);
      res.emplace_back(Base::Info_Lane_t(0, Utils::Str2LaneUid(ref_from_to.fromLaneUid, "."),
                                         Utils::Str2LaneUid(ref_from_to.toLaneUid, ".")));
    }
    return res;
  } else {
    return res;
  }
}

std::vector<HadmapCacheConCurrent::HadmapFilter::link_from_to_road>
HadmapCacheConCurrent::HadmapFilter::BlacklistFromToRoadId() const TX_NOEXCEPT {
  std::vector<HadmapCacheConCurrent::HadmapFilter::link_from_to_road> res;
  if (IsValid()) {
    return hdmap_advanced_cfg.front().blacklist_from_to_roadId;
  } else {
    return res;
  }
}

std::vector<Base::txLaneUId> HadmapCacheConCurrent::HadmapFilter::RefuseSwitchLeftLane() const TX_NOEXCEPT {
  std::vector<Base::txLaneUId> res;
  if (IsValid()) {
    for (const auto &refLane : hdmap_advanced_cfg.front().refuse_switch_dual_direction_lane) {
      res.emplace_back(Utils::Str2LaneUid(refLane, "."));
    }

    for (const auto &refLane : hdmap_advanced_cfg.front().refuse_switch_left_lane) {
      res.emplace_back(Utils::Str2LaneUid(refLane, "."));
    }

    for (const auto &refRoadId : hdmap_advanced_cfg.front().refuse_switch_dual_direction_road) {
      hadmap::txRoadPtr cur_road_ptr = HadmapCacheConCurrent::GetTxRoadPtr(refRoadId);
      if (NonNull_Pointer(cur_road_ptr)) {
        const auto &sections = cur_road_ptr->getSections();
        for (const auto &cur_section_ptr : sections) {
          if (NonNull_Pointer(cur_section_ptr)) {
            const auto &lanes = cur_section_ptr->getLanes();
            for (const auto &cur_lane_ptr : lanes) {
              if (NonNull_Pointer(cur_lane_ptr)) {
                res.emplace_back(cur_lane_ptr->getTxLaneId());
              }
            }
          }
        }
      }
    }

    for (const auto &refRoadId : hdmap_advanced_cfg.front().refuse_switch_left_road) {
      hadmap::txRoadPtr cur_road_ptr = HadmapCacheConCurrent::GetTxRoadPtr(refRoadId);
      if (NonNull_Pointer(cur_road_ptr)) {
        const auto &sections = cur_road_ptr->getSections();
        for (const auto &cur_section_ptr : sections) {
          if (NonNull_Pointer(cur_section_ptr)) {
            const auto &lanes = cur_section_ptr->getLanes();
            for (const auto &cur_lane_ptr : lanes) {
              if (NonNull_Pointer(cur_lane_ptr)) {
                res.emplace_back(cur_lane_ptr->getTxLaneId());
              }
            }
          }
        }
      }
    }
    return res;
  } else {
    return res;
  }
}

std::vector<Base::txLaneUId> HadmapCacheConCurrent::HadmapFilter::RefuseSwitchRightLane() const TX_NOEXCEPT {
  std::vector<Base::txLaneUId> res;
  if (IsValid()) {
    for (const auto &refLane : hdmap_advanced_cfg.front().refuse_switch_dual_direction_lane) {
      res.emplace_back(Utils::Str2LaneUid(refLane, "."));
    }

    for (const auto &refLane : hdmap_advanced_cfg.front().refuse_switch_right_lane) {
      res.emplace_back(Utils::Str2LaneUid(refLane, "."));
    }

    for (const auto &refRoadId : hdmap_advanced_cfg.front().refuse_switch_dual_direction_road) {
      hadmap::txRoadPtr cur_road_ptr = HadmapCacheConCurrent::GetTxRoadPtr(refRoadId);
      if (NonNull_Pointer(cur_road_ptr)) {
        const auto &sections = cur_road_ptr->getSections();
        for (const auto &cur_section_ptr : sections) {
          if (NonNull_Pointer(cur_section_ptr)) {
            const auto &lanes = cur_section_ptr->getLanes();
            for (const auto &cur_lane_ptr : lanes) {
              if (NonNull_Pointer(cur_lane_ptr)) {
                res.emplace_back(cur_lane_ptr->getTxLaneId());
              }
            }
          }
        }
      }
    }

    for (const auto &refRoadId : hdmap_advanced_cfg.front().refuse_switch_right_road) {
      hadmap::txRoadPtr cur_road_ptr = HadmapCacheConCurrent::GetTxRoadPtr(refRoadId);
      if (NonNull_Pointer(cur_road_ptr)) {
        const auto &sections = cur_road_ptr->getSections();
        for (const auto &cur_section_ptr : sections) {
          if (NonNull_Pointer(cur_section_ptr)) {
            const auto &lanes = cur_section_ptr->getLanes();
            for (const auto &cur_lane_ptr : lanes) {
              if (NonNull_Pointer(cur_lane_ptr)) {
                res.emplace_back(cur_lane_ptr->getTxLaneId());
              }
            }
          }
        }
      }
    }
    return res;
  } else {
    return res;
  }
}

std::vector<std::tuple<std::tuple<Base::txLaneUId, Base::txLaneUId>,
                       std::vector<std::tuple<Base::txLaneUId, Base::txLaneUId, Base::txFloat>>>>
HadmapCacheConCurrent::HadmapFilter::ExpandVision() const TX_NOEXCEPT {
  std::vector<std::tuple<std::tuple<Base::txLaneUId, Base::txLaneUId>,
                         std::vector<std::tuple<Base::txLaneUId, Base::txLaneUId, Base::txFloat>>>>
      res;
  if (IsValid()) {
    for (const auto &ref_vision_node : hdmap_advanced_cfg.front().expand_vision) {
      std::tuple<Base::txLaneUId, Base::txLaneUId> source_info =
          std::make_tuple(Utils::Str2LaneUid(ref_vision_node.source_link.fromLaneUid, "."),
                          Utils::Str2LaneUid(ref_vision_node.source_link.toLaneUid, "."));

      std::vector<std::tuple<Base::txLaneUId, Base::txLaneUId, Base::txFloat>> expand_info;
      for (const auto &ref_expand : ref_vision_node.expand_links) {
        expand_info.emplace_back(std::make_tuple(Utils::Str2LaneUid(ref_expand.fromLaneUid, "."),
                                                 Utils::Str2LaneUid(ref_expand.toLaneUid, "."), ref_expand.distance));
      }
      res.emplace_back(std::make_tuple(source_info, expand_info));
    }
    return res;
  } else {
    return res;
  }
}

Base::txBool HadmapCacheConCurrent::InitializeHadmapFilter(const Base::txString cfg_path,
                                                           const Base::txString map_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  if (exists(FilePath(cfg_path))) {
    std::ifstream in_json(cfg_path);
    nlohmann::json re_parse_json = nlohmann::json::parse(in_json);
    from_json(re_parse_json, s_hadmap_filer);
    s_hadmap_filer.Parse(map_path);
    LogInfo << "[HadmapFilter][find param configure file] " << std::endl << s_hadmap_filer;
    return true;
  } else {
    LOG(WARNING) << "loading cfg file failure : " << cfg_path;
    return false;
  }
}

Base::txString HadmapCacheConCurrent::HadmapFilter::link_from_to::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS(fromLaneUid) << TX_VARS(toLaneUid) << "},";
  return oss.str();
}

Base::txString HadmapCacheConCurrent::HadmapFilter::link_from_to_road::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS(fromRoadId) << TX_VARS(toRoadId) << "},";
  return oss.str();
}

Base::txString HadmapCacheConCurrent::HadmapFilter::link_from_to_distance::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS(fromLaneUid) << TX_VARS(toLaneUid) << TX_VARS(distance) << "},";
  return oss.str();
}

Base::txString HadmapCacheConCurrent::HadmapFilter::cfg_special_hdmap::expand_vision_node::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS_NAME(source_link, source_link.Str()) << " expand_links : ";
  for (const auto &refDst : expand_links) {
    oss << refDst.Str();
  }
  oss << "}" << std::endl;
  return oss.str();
}

Base::txString HadmapCacheConCurrent::HadmapFilter::cfg_special_hdmap::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "{" << TX_VARS(HDMapNameWithExtern) << std::endl;

  oss << "blacklist_from_to_laneUid : ";
  for (const auto &ref : blacklist_from_to_laneUid) {
    oss << ref << ",";
  }
  oss << std::endl;

  oss << "blacklist_from_to_roadId : ";
  for (const auto &ref : blacklist_from_to_roadId) {
    oss << ref << ",";
  }
  oss << std::endl;

  oss << "refuse_switch_dual_direction_lane : ";
  std::copy(refuse_switch_dual_direction_lane.begin(), refuse_switch_dual_direction_lane.end(),
            std::ostream_iterator<txString>(oss, ","));
  oss << std::endl;

  oss << "refuse_switch_left_lane : ";
  std::copy(refuse_switch_left_lane.begin(), refuse_switch_left_lane.end(), std::ostream_iterator<txString>(oss, ","));
  oss << std::endl;

  oss << "refuse_switch_right_lane : ";
  std::copy(refuse_switch_right_lane.begin(), refuse_switch_right_lane.end(),
            std::ostream_iterator<txString>(oss, ","));
  oss << std::endl;

  oss << "refuse_switch_dual_direction_road : ";
  std::copy(refuse_switch_dual_direction_road.begin(), refuse_switch_dual_direction_road.end(),
            std::ostream_iterator<Base::txRoadID>(oss, ","));
  oss << std::endl;

  oss << "refuse_switch_left_road : ";
  std::copy(refuse_switch_left_road.begin(), refuse_switch_left_road.end(),
            std::ostream_iterator<Base::txRoadID>(oss, ","));
  oss << std::endl;

  oss << "refuse_switch_right_road : ";
  std::copy(refuse_switch_right_road.begin(), refuse_switch_right_road.end(),
            std::ostream_iterator<Base::txRoadID>(oss, ","));
  oss << std::endl;

  oss << "expand_vision : ";
  for (const auto &ref_expand_vision : expand_vision) {
    oss << ref_expand_vision.Str() << ", ";
  }
  oss << std::endl;

  return oss.str();
}

Base::txString HadmapCacheConCurrent::HadmapFilter::Str() const TX_NOEXCEPT {
  std::ostringstream oss;
  for (const auto &ref : hdmap_advanced_cfg) {
    oss << ref.Str() << std::endl;
  }
  return oss.str();
}

void HadmapCacheConCurrent::Hadmap_Filter(hadmap::txLanes &lanePtrVec, hadmap::txLaneLinks &lanelinkPtrVec,
                                          hadmap::txRoads &roadPtrVec) TX_NOEXCEPT {
  std::map<txLaneUId, std::map<txLaneUId, txLaneLinkID>> cache_fromto_lanelinkid;
  for (const auto linkPtr : lanelinkPtrVec) {
    const auto fromUid = (linkPtr)->fromTxLaneId();
    const auto toUid = (linkPtr)->toTxLaneId();
    bool find_delete_link = false;
    for (const Base::Info_Lane_t from_to_uid : s_hadmap_filer.BlacklistLocInfo()) {
      if (((from_to_uid.onLinkFromLaneUid) == fromUid) && ((from_to_uid.onLinkToLaneUid) == toUid)) {
        find_delete_link = true;
        break;
      }
    }

    for (const HadmapFilter::link_from_to_road &from_to_roadId : s_hadmap_filer.BlacklistFromToRoadId()) {
      if (fromUid.roadId == from_to_roadId.fromRoadId && toUid.roadId == from_to_roadId.toRoadId) {
        find_delete_link = true;
        break;
      }
    }

    auto to_lane_ptr = GetTxLanePtr(toUid);
    // LOG(WARNING) << TX_COND(NonNull_Pointer(to_lane_ptr)) << TX_VARS(to_lane_ptr->getLaneType());
    if (NonNull_Pointer(to_lane_ptr) && CallFail(Utils::IsDrivingLane(to_lane_ptr->getLaneType()))) {
      find_delete_link = true;
      /*LOG(WARNING) << "[lane_type_none]" << TX_VARS(Utils::ToString(toUid));*/
    }
    if (find_delete_link) {
      LogInfo << "[BlacklistFromToLaneUid]" << TX_VARS_NAME(from, Utils::ToString(linkPtr->fromTxLaneId()))
              << TX_VARS_NAME(to, Utils::ToString(linkPtr->toTxLaneId()));
      add_link_black_list(Base::Info_Lane_t(linkPtr->getId(), linkPtr->fromTxLaneId(), linkPtr->toTxLaneId()));
    }

    cache_fromto_lanelinkid[fromUid][toUid] = (linkPtr->getId());
  }

  /*for (const auto blackLinkId : s_hadmap_filer.BlacklistLaneLinkId()) {
      add_link_black_list(blackLinkId);
  }*/

  AddRefuseChangeLane(s_hadmap_filer.RefuseSwitchLeftLane(), SwitchDir::eLeft);
  AddRefuseChangeLane(s_hadmap_filer.RefuseSwitchRightLane(), SwitchDir::eRight);

  std::ostringstream oss;
  if (FLAGS_LogLevel_MapSDK) {
    oss << "[RefuseSwitchLeftLane] :";
    for (const auto &refLaneUid : s_hadmap_filer.RefuseSwitchLeftLane()) {
      oss << Utils::ToString(refLaneUid) << std::endl;
    }

    oss << "[RefuseSwitchRightLane] :";
    for (const auto &refLaneUid : s_hadmap_filer.RefuseSwitchRightLane()) {
      oss << Utils::ToString(refLaneUid) << std::endl;
    }
    LOG(INFO) << oss.str();
  }

  const std::vector<
      std::tuple<std::tuple<txLaneUId, txLaneUId>, std::vector<std::tuple<txLaneUId, txLaneUId, txFloat>>>>
      expandVision = s_hadmap_filer.ExpandVision();

  oss.str("");
  oss << "[expand_vision] " << std::endl;
  for (const auto &expandNode : expandVision) {
    const auto &source = std::get<0>(expandNode);
    const txLaneLinkID source_linkId = cache_fromto_lanelinkid[std::get<0>(source)][std::get<1>(source)];
    oss << TX_VARS(source_linkId) << ": ";

    lanelinkExpandVisionType::accessor a;
    s_lanelink_expand_vision.insert(a, Base::Info_Lane_t(source_linkId, std::get<0>(source), std::get<1>(source)));
    for (const auto &expandVec : std::get<1>(expandNode)) {
      const txLaneLinkID expand_linkId = cache_fromto_lanelinkid[std::get<0>(expandVec)][std::get<1>(expandVec)];
      const Base::Info_Lane_t expand_locInfo(expand_linkId, std::get<0>(expandVec), std::get<1>(expandVec));
      const txFloat expand_dist = std::get<2>(expandVec);
      const txFloat lanelinkLength = GetLaneLinkLength(expand_locInfo);
      (a->second)[expand_locInfo] = std::make_tuple(expand_dist, lanelinkLength);

      oss << TX_VARS(expand_locInfo) << TX_VARS(expand_dist) << TX_VARS(lanelinkLength);
    }
    oss << std::endl;
  }
  if (FLAGS_LogLevel_MapSDK) {
    LOG(INFO) << oss.str();
    {
      std::ostringstream oss;
      oss << "[expand_vision] " << std::endl;
      auto range = s_lanelink_expand_vision.range();
      for (auto itr = range.begin(); itr != range.end(); ++itr) {
        oss << TX_VARS_NAME(source_link_id, (*itr).first) << ": ";
        for (const auto &ref_tuple : (*itr).second) {
          oss << TX_VARS_NAME(expand_link_id, std::get<0>(ref_tuple))
              << TX_VARS_NAME(expand_dist, std::get<0>(std::get<1>(ref_tuple)))
              << TX_VARS_NAME(lanelink_length, std::get<1>(std::get<1>(ref_tuple)));
        }
        oss << std::endl;
      }
      LOG(INFO) << oss.str();
    }
  }
  return;
}

Base::txBool HadmapCacheConCurrent::CheckExpandVision(
    const Base::Info_Lane_t &link_locInfo, lanelinkExpandVisionType::mapped_type &res_expand_vision) TX_NOEXCEPT {
  lanelinkExpandVisionType::const_accessor ca;
  if (CallSucc(s_lanelink_expand_vision.find(ca, link_locInfo))) {
    res_expand_vision = ca->second;
    return true;
  } else {
    return false;
  }
}

void HadmapCacheConCurrent::ClearAddDstRefuseLaneChange() TX_NOEXCEPT {
  s_dst_refuse_lane_change_left.clear();
  s_dst_refuse_lane_change_right.clear();
}

void HadmapCacheConCurrent::AddDstRefuseLaneChange_Left(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  DstReRefuseLaneChangeSet::accessor a;
  s_dst_refuse_lane_change_left.insert(a, _laneUid);
}

void HadmapCacheConCurrent::AddDstRefuseLaneChange_Right(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  DstReRefuseLaneChangeSet::accessor a;
  s_dst_refuse_lane_change_right.insert(a, _laneUid);
}

Base::txBool HadmapCacheConCurrent::IsDstRefuseLaneChange_Left(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  DstReRefuseLaneChangeSet::const_accessor ca;
  return CallSucc(s_dst_refuse_lane_change_left.find(ca, _laneUid));
}

Base::txBool HadmapCacheConCurrent::IsDstRefuseLaneChange_Right(const Base::txLaneUId _laneUid) TX_NOEXCEPT {
  DstReRefuseLaneChangeSet::const_accessor ca;
  return CallSucc(s_dst_refuse_lane_change_right.find(ca, _laneUid));
}

Base::txBool HadmapCacheConCurrent::InitializeHdMapTrick(const Base::txString map_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  FilePath file_name = FilePath(map_path).filename();
  LOG(INFO) << "[HdMapTrick][LR]" << TX_VARS(map_path) << TX_VARS(file_name);
  s_hdmapTrick_LaneUidLeftRightType.clear();
  if (Base::txString("re_project_mannul_0414.xodr") == file_name) {
    {
      hdmapTrick_LaneUidLeftRightType::accessor a;
      s_hdmapTrick_LaneUidLeftRightType.insert(a, Base::txLaneUId(160, 0, -3));
      a->second = {Base::txLaneUId(160, 0, -1), Base::txLaneUId(160, 0, -4)};
      a.release();
    }
    {
      hdmapTrick_LaneUidLeftRightType::accessor a;
      s_hdmapTrick_LaneUidLeftRightType.insert(a, Base::txLaneUId(96, 0, -3));
      a->second = {Base::txLaneUId(96, 0, -1), Base::txLaneUId(96, 0, -4)};
      a.release();
    }
    {
      hdmapTrick_LaneUidLeftRightType::accessor a;
      s_hdmapTrick_LaneUidLeftRightType.insert(a, Base::txLaneUId(171, 0, -3));
      a->second = {Base::txLaneUId(171, 0, -1), Base::txLaneUId(171, 0, -4)};
      a.release();
    }
    AddRefuseChangeLane({Base::txLaneUId(58, 0, -2), Base::txLaneUId(142, 0, -2), Base::txLaneUId(5, 0, -3)},
                        SwitchDir::eLeft);

    {
      hdmapTrick_RoadLaneKeepTimeType::accessor a;
      s_hdmapTrick_RoadLaneKeepTimeType.insert(a, 142);
      a->second = 0.0;
      a.release();
    }
    {
      hdmapTrick_RoadLaneKeepTimeType::accessor a;
      s_hdmapTrick_RoadLaneKeepTimeType.insert(a, 5);
      a->second = 0.0;
      a.release();
    }
    {
      hdmapTrick_RoadLaneKeepTimeType::accessor a;
      s_hdmapTrick_RoadLaneKeepTimeType.insert(a, 171);
      a->second = 0.0;
      a.release();
    }
    {
      hdmapTrick_RoadLaneKeepTimeType::accessor a;
      s_hdmapTrick_RoadLaneKeepTimeType.insert(a, 96);
      a->second = 0.0;
      a.release();
    }
    {
      hdmapTrick_RoadLaneKeepTimeType::accessor a;
      s_hdmapTrick_RoadLaneKeepTimeType.insert(a, 160);
      a->second = 0.0;
      a.release();
    }

    LOG(INFO) << "[HdMapTrick][LR][success]" << TX_VARS(file_name) << TX_VARS(s_hdmapTrick_LaneUidLeftRightType.size());
    return true;
  }
  return false;
}

Base::txBool HadmapCacheConCurrent::QueryTrick_LR(const Base::txLaneUId &srcLaneUid, Base::txLaneUId &leftUid,
                                                  Base::txLaneUId &rightUid) TX_NOEXCEPT {
  hdmapTrick_LaneUidLeftRightType::const_accessor ca;
  if (CallSucc(s_hdmapTrick_LaneUidLeftRightType.find(ca, srcLaneUid))) {
    const auto &refArrayLR = ca->second;
    leftUid = refArrayLR.front();
    rightUid = refArrayLR.back();
    return true;
  }
  return false;
}

Base::txBool HadmapCacheConCurrent::QueryTrick_L(const Base::txLaneUId &srcLaneUid,
                                                 Base::txLaneUId &leftUid) TX_NOEXCEPT {
  Base::txLaneUId unuse;
  return QueryTrick_LR(srcLaneUid, leftUid, unuse);
}

Base::txBool HadmapCacheConCurrent::QueryTrick_R(const Base::txLaneUId &srcLaneUid,
                                                 Base::txLaneUId &rightUid) TX_NOEXCEPT {
  Base::txLaneUId unuse;
  return QueryTrick_LR(srcLaneUid, unuse, rightUid);
}

Base::txBool HadmapCacheConCurrent::QueryTrick_LaneKeepTime(const txRoadID rid,
                                                            Base::txFloat &spLaneKeepTime) TX_NOEXCEPT {
  hdmapTrick_RoadLaneKeepTimeType::const_accessor ca;
  if (CallSucc(s_hdmapTrick_RoadLaneKeepTimeType.find(ca, rid))) {
    spLaneKeepTime = ca->second;
    ca.release();
    return true;
  } else {
    return false;
  }
}

void HadmapCacheConCurrent::MakeConsistency() TX_NOEXCEPT {}

TX_NAMESPACE_CLOSE(HdMap)

#undef LogInfo
