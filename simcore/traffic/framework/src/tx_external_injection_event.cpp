// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_external_injection_event.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>
#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hashed_road.h"
#include "nlohmann/json.hpp"
#include "tx_component.h"
#include "tx_parallel_def.h"
#include "tx_path_utils.h"
#include "tx_spatial_query.h"
#include "tx_traffic_element_base.h"
#if USE_DITW_Event
TX_NAMESPACE_OPEN(Base)

txBool txInjectionEvent::CheckTime(const txFloat curTimeStamp) TX_NOEXCEPT {
  // 如果当前时间戳大于等于事件的开始时间且小于事件的结束时间
  if (StartTime() <= curTimeStamp && curTimeStamp < EndTime()) {
    // LOG(WARNING) << "[InjectionEventTime]" << TX_VARS(StartTime()) << TX_VARS(curTimeStamp) << TX_VARS(EndTime());
    return true;
  } else {  // 如果当前时间戳不在事件的开始时间和结束时间之间
    LOG(WARNING) << "[InjectionEventTimeOut]" << TX_VARS(StartTime()) << TX_VARS(curTimeStamp) << TX_VARS(EndTime());
    return false;
  }
}

txBool txInjectionEvent::UpdateSimulationEvent(const txFloat curTimeInSecond,
                                               std::vector<Base::ITrafficElementPtr> vecVehiclePtr) TX_NOEXCEPT {
  // 如果事件有效
  if (IsValid()) {
    // 如果当前时间戳在事件的时间范围内
    if (CheckTime(curTimeInSecond)) {
      // 如果事件类型为速度限制
      if (EventType::eSpeedLimitRoad == GetEventType()) {
        // 并行遍历车辆
        tbb::parallel_for(static_cast<std::size_t>(0), vecVehiclePtr.size(), [&](const std::size_t idx) {
          auto vehiclePtr = vecVehiclePtr[idx];
          const auto& curLaneInfo = vehiclePtr->StableLaneInfo();
          // 如果车辆存活且支持速度控制且当前车辆不在link上
          if (CallSucc(vehiclePtr->IsAlive()) && CallSucc(vehiclePtr->SupportVelocityControled()) &&
              CallFail(curLaneInfo.isOnLaneLink)) {
            // 如果当前车道所在的道路ID在受影响的道路ID集合中
            if (mAffectedRoads.roadIdSet.count(curLaneInfo.onLaneUid.roadId) > 0) {
              // 如果车辆的速度不受控制
              if (CallFail(vehiclePtr->IsVelocityControled())) {
                // 设置车辆的速度受控制
                vehiclePtr->SetVelocityControled(true);
                // 计算随机速度比例
                const Base::txFloat rnd_ratio =
                    mAffectedRoads.affected_speed_ratio +
                    (vehiclePtr->GetRnd_0_1()) *
                        (mAffectedRoads.affected_speed_ratio_upper - mAffectedRoads.affected_speed_ratio);
                // 设置车辆的期望速度
                vehiclePtr->SetVelocityDesired(vehiclePtr->GetRawVelocityDesired() * rnd_ratio);
              }
            } else {
              // 设置车辆的速度不受控制
              vehiclePtr->SetVelocityControled(false);
              vehiclePtr->ResetVelocityDesired();
            }
          }
#  if 0
          if (CallSucc(vehiclePtr->IsAlive()) && CallFail(curLaneInfo.isOnLaneLink) &&
              mAffectedRoads.roadIdSet.count(curLaneInfo.onLaneUid.roadId) > 0 &&
              CallSucc(vehiclePtr->SupportVelocityControled()) && CallFail(vehiclePtr->IsVelocityControled())) {
            vecVehiclePtr[idx]->SetVelocityControled(true);
            const Base::txFloat rnd_ratio = mAffectedRoads.affected_speed_ratio +
                                            (vehiclePtr->GetRnd_0_1()) * (mAffectedRoads.affected_speed_ratio_upper -
                                                                          mAffectedRoads.affected_speed_ratio);
            vehiclePtr->SetVelocityDesired(vehiclePtr->GetRawVelocityDesired() * rnd_ratio);
            // LOG(WARNING) << TX_VARS(vehiclePtr->Id()) << TX_VARS(rnd_ratio);
          }
#  endif
        }); /*lamda function*/
            /* parallel_for */
      }
      return true;
    } else if (curTimeInSecond >= EndTime()) {  // 如果当前时间大于等于事件的结束时间
      // 如果事件类型为速度限制道路
      if (EventType::eSpeedLimitRoad == GetEventType()) {
        // 使用TBB库的并行for循环遍历所有车辆
        tbb::parallel_for(static_cast<std::size_t>(0), vecVehiclePtr.size(), [&](const std::size_t idx) {
          auto vehiclePtr = vecVehiclePtr[idx];
          const auto& curLaneInfo = vehiclePtr->StableLaneInfo();
          // 如果车辆存活且支持速度控制且速度受控制
          if (CallSucc(vehiclePtr->IsAlive()) && CallSucc(vehiclePtr->SupportVelocityControled()) &&
              CallSucc(vehiclePtr->IsVelocityControled())) {
            // 设置车辆的速度不受控制
            vehiclePtr->SetVelocityControled(false);
            vehiclePtr->ResetVelocityDesired();
          }
        }); /*lamda function*/
            /* parallel_for */
      }
      // 设置事件无效
      mValid = false;
      LOG(WARNING) << "[log2world] event finish." << TX_VARS(curTimeInSecond);
      return true;
    } else {
      return false;
    }
  } else {
    return false;
  }
  return false;
}

#  if USE_HashedRoadNetwork
Base::txBool txInjectionEventCrashInflunce::Initialize(const txSysId _eId, const Coord::txWGS84 _crash_location,
                                                       const std::vector<Base::txLaneID>& influnceLaneIdVec,
                                                       const Base::txFloat pre_dist, const Base::txFloat post_dist,
                                                       const txFloat startTimeStamp,
                                                       const txFloat endTimeStamp) TX_NOEXCEPT {
  // 设置事件有效
  mValid = (true);
  mStartingTime = (startTimeStamp);
  mEndTime = (endTimeStamp);
  // 设置事件ID
  mEventId = (_eId);
  mEventLocation = _crash_location;
  mEventType = txInjectionEvent::EventType::eCrashInfluneLane;

  // 获取事件位置对应的S坐标
  Base::txFloat event_s = 0.0;
  if (Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_S_Coord_By_Enu_Pt(
          mEventLocation, mEventLaneInfo, event_s)) {
    // 如果事件位置在车道链上
    if (mEventLaneInfo.isOnLaneLink) {
      LOG(WARNING) << TX_VARS_NAME(Event, _eId) << " initialize failure.  location on lanelink, "
                   << TX_VARS(mEventLocation) << TX_VARS(mEventLaneInfo);
      mValid = false;
      return false;
    } else {
      // 遍历影响车道ID集合
      for (const auto laneId : influnceLaneIdVec) {
        // 获取当前车道信息
        Base::Info_Lane_t curLaneInfo;
        curLaneInfo.FromLane(
            Base::txLaneUId(mEventLaneInfo.onLaneUid.roadId, mEventLaneInfo.onLaneUid.sectionId, laneId));
        const auto geom_ptr = HdMap::HadmapCacheConCurrent::GetLaneInfoByUid(curLaneInfo.onLaneUid);
        // 计算当前车道的起始哈希信息
        const Geometry::SpatialQuery::HashedLaneInfo curLaneStartHashPt =
            Geometry::SpatialQuery::GenerateHashedLaneInfo(geom_ptr, event_s, curLaneInfo);

        // 初始化哈希信息向量
        std::vector<Geometry::SpatialQuery::HashedLaneInfo> vecHashedInfo;
        vecHashedInfo.emplace_back(curLaneStartHashPt);
        // 计算事件位置与当前车道起始位置之间的距离
        Base::txFloat dist_between_post = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(
            curLaneStartHashPt.subSectionStartWGS84, _crash_location);
        Base::txBool bFlag = true;
        // 查找事件位置后方的哈希信息
        for (Base::txInt i = 0; i < 100 && bFlag && (dist_between_post < std::abs(post_dist)); ++i) {
          Geometry::SpatialQuery::HashedLaneInfo newHashedInfo;
          bFlag = Geometry::SpatialQuery::GetPostHashedLaneInfo(vecHashedInfo.back(), newHashedInfo);
          if (bFlag) {
            vecHashedInfo.emplace_back(newHashedInfo);
          }
          dist_between_post =
              Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(newHashedInfo.subSectionStartWGS84, _crash_location);
        }

        // 注册事件影响的哈希信息
        HdMap::HashedRoadCacheConCurrent::RegisterCloseLaneEvent(mEventId, vecHashedInfo);
        // 将哈希信息添加到事件影响哈希信息向量中
        mInflunceHashedLaneInfo.insert(mInflunceHashedLaneInfo.end(), vecHashedInfo.begin(), vecHashedInfo.end());
      }
      std::ostringstream oss;
      std::copy(mInflunceHashedLaneInfo.begin(), mInflunceHashedLaneInfo.end(),
                std::ostream_iterator<Geometry::SpatialQuery::HashedLaneInfo>(oss, "\n"));
      LOG(WARNING) << TX_VARS_NAME(Event, _eId) << oss.str();
      return true;
    }
  } else {
    LOG(WARNING) << TX_VARS_NAME(Event, _eId) << " initialize failure.  Get_S_Coord_By_Enu_Pt error, "
                 << mEventLocation;
    mValid = false;
    return false;
  }
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(txInjectionEventCrashInflunce::InjectionEventInfo, event_id, event_type,
                                   event_start_time_in_second, event_end_time_in_second, event_location_lon,
                                   event_location_lat, event_influnce_lane, event_influnce_range_alone_lane,
                                   record_file_path, l2w_switch_time);

Base::txBool txInjectionEventCrashInflunce::LoadEventCfg(const Base::txString& _cfg_full_path,
                                                         InjectionEventInfo& ret_cfg) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 如果配置文件存在
  if (exists(FilePath(_cfg_full_path))) {
    std::ifstream in_json(_cfg_full_path);
    // 解析配置文件为JSON对象
    nlohmann::json re_parse_json = nlohmann::json::parse(in_json);
    // 将JSON对象转换为事件配置信息
    from_json(re_parse_json, ret_cfg);
    LOG(INFO) << "[txInjectionEventCrashInflunce][find param configure file] " << std::endl << ret_cfg;
    return true;
  } else {
    // 输出警告信息
    LOG(WARNING) << "loading cfg file failure : " << _cfg_full_path;
    return false;
  }
}

std::ostream& operator<<(std::ostream& os, const txInjectionEventCrashInflunce::InjectionEventInfo& v) TX_NOEXCEPT {
  // 将影响车道ID集合转换为字符串
  std::ostringstream oss_influnce_lane;
  std::copy(v.event_influnce_lane.begin(), v.event_influnce_lane.end(),
            std::ostream_iterator<Base::txInt>(oss_influnce_lane, ";"));
  // 将影响范围转换为字符串
  std::ostringstream oss_influnce_range;
  std::copy(v.event_influnce_range_alone_lane.begin(), v.event_influnce_range_alone_lane.end(),
            std::ostream_iterator<Base::txFloat>(oss_influnce_range, ","));

  os << TX_VARS_NAME(event_id, v.event_id) << TX_VARS_NAME(event_type, v.event_type)
     << TX_VARS_NAME(event_start_time_in_second, v.event_start_time_in_second)
     << TX_VARS_NAME(event_end_time_in_second, v.event_end_time_in_second)
     << TX_VARS_NAME(event_location_lon, v.event_location_lon) << TX_VARS_NAME(event_location_lat, v.event_location_lat)
     << TX_VARS_NAME(event_influnce_lane, oss_influnce_lane.str())
     << TX_VARS_NAME(event_influnce_range_alone_lane, oss_influnce_range.str())
     << TX_VARS_NAME(record_file_path, v.record_file_path) << TX_VARS_NAME(l2w_switch_time, v.l2w_switch_time);
  return os;
}

Base::txBool txInjectionEventCrashInflunce::CheckTime(const txFloat curTimeStamp) TX_NOEXCEPT {
  // 如果当前时间大于等于事件的开始时间且小于事件的结束时间
  if (StartTime() <= curTimeStamp && curTimeStamp < EndTime()) {
    // LOG(WARNING) << "[InjectionEventTime]" << TX_VARS(StartTime()) << TX_VARS(curTimeStamp) << TX_VARS(EndTime());
    return true;
  } else {
    LOG(WARNING) << "[InjectionEventTimeOut]" << TX_VARS(StartTime()) << TX_VARS(curTimeStamp) << TX_VARS(EndTime());
    return false;
  }
}

Base::txBool txInjectionEventCrashInflunce::UpdateSimulationEvent(
    const txFloat curTimeStamp, std::vector<Base::ITrafficElementPtr> vecVehiclePtr) TX_NOEXCEPT {
  // 如果事件有效
  if (IsValid()) {
    if (curTimeStamp >= EndTime()) {
      // 注销事件影响的哈希信息
      HdMap::HashedRoadCacheConCurrent::UnRegisterCloseLaneEvent(mEventId);
      // 设置事件无效
      mValid = false;
      return true;
    }
  }
  return false;
}

#  endif /*USE_HashedRoadNetwork*/
TX_NAMESPACE_CLOSE(Base)
#endif /*USE_DITW_Event*/
