// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "parallel_simulation/tx_tc_ps_traffic_record.h"
#include <ctime>
#include <fstream>
#include <iomanip>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "nlohmann/json.hpp"
#include "tx_header.h"

TX_NAMESPACE_OPEN(TrafficFlow)

TrafficRecord::TrafficRecord() {}
TrafficRecord::~TrafficRecord() {}

std::map<int, TrafficRecord::RoadTravelTime> TrafficRecord::UpdateRoadTravelTime(
    int roadId, int64_t singleVehicleTravelTime, const std::map<int, RoadTravelTime>& allRoadTravelTime) const {
  std::map<int, TrafficRecord::RoadTravelTime> tmpAllRoadTravelTime = allRoadTravelTime;
  if (tmpAllRoadTravelTime.find(roadId) == tmpAllRoadTravelTime.end()) {
    RoadTravelTime roadTravelTime;
    hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(roadId);
    if (NonNull_Pointer(curRoadPtr) && NonNull_Pointer(curRoadPtr->getGeometry())) {
      roadTravelTime.roadLength = curRoadPtr->getLength();
      roadTravelTime.passVehicle = 1;
      roadTravelTime.averageTime = singleVehicleTravelTime;
      tmpAllRoadTravelTime[roadId] = roadTravelTime;
    }
  } else {
    tmpAllRoadTravelTime[roadId].averageTime =
        (tmpAllRoadTravelTime[roadId].averageTime * tmpAllRoadTravelTime[roadId].passVehicle +
         singleVehicleTravelTime) /
        (tmpAllRoadTravelTime[roadId].passVehicle + 1);
    tmpAllRoadTravelTime[roadId].passVehicle += 1;
  }
  return tmpAllRoadTravelTime;
}

void TrafficRecord::UpdateRoadTravelTimeWithCurrentVehicle(std::map<int, RoadTravelTime> roadsTravelTime,
                                                           const sim_msg::Traffic& originTraffic,
                                                           int64_t currentTime) const {
  double free_flow_speed = 18;  // 18m/s = 64.8km/h
  for (const sim_msg::Car& car : originTraffic.cars()) {
    if (car.tx_road_id() != ROAD_PKID_INVALID &&
        allVehicleArriveRoadTime.find(car.tx_road_id()) != allVehicleArriveRoadTime.end()) {
      hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(car.tx_road_id());
      if (NonNull_Pointer(curRoadPtr) && NonNull_Pointer(curRoadPtr->getGeometry())) {
        hadmap::txPoint curPoint(car.x(), car.y(), car.z());
        hadmap::txPoint endPoint = curRoadPtr->getGeometry()->getEnd();
        double distanceToEnd = curRoadPtr->getGeometry()->getPassedDistance(curPoint, endPoint, hadmap::COORD_WGS84);
        double estimateTravelTime = currentTime - allVehicleArriveRoadTime.at(car.tx_road_id()).arriveTime +
                                    distanceToEnd * 1000 / free_flow_speed;
        roadsTravelTime = UpdateRoadTravelTime(car.tx_road_id(), estimateTravelTime, roadsTravelTime);
      } else {
        LOG(WARNING) << TX_COND_NAME(NonNull_Pointer(curRoadPtr), NonNull_Pointer(curRoadPtr));
        continue;
      }
    }
  }
}

std::map<int, TrafficRecord::RoadTravelTime> TrafficRecord::CalculateFinalRoadTravelTime(
    const sim_msg::Traffic& originTraffic, int64_t currentTime) const {
  auto fanalAllRoadTravelTime = allRoadTravelTime;

  UpdateRoadTravelTimeWithCurrentVehicle(fanalAllRoadTravelTime, originTraffic, currentTime);

  // Update vehicle final road travel time
  auto allVehicleStartEndTimeWithoutNotReach = RemoveVehicleNotReachEnd(originTraffic, allVehicleStartEndTime);
  for (auto iter = allVehicleStartEndTimeWithoutNotReach.begin(); iter != allVehicleStartEndTimeWithoutNotReach.end();
       iter++) {
    if (allVehicleArriveRoadTime.find(iter->first) == allVehicleArriveRoadTime.end()) {
      continue;
    }
    if (allVehicleArriveRoadTime.at(iter->first).curRoadId != -1) {
      fanalAllRoadTravelTime = UpdateRoadTravelTime(
          allVehicleArriveRoadTime.at(iter->first).curRoadId,
          iter->second.arriveTime - allVehicleArriveRoadTime.at(iter->first).arriveTime, fanalAllRoadTravelTime);
    }
  }
  return fanalAllRoadTravelTime;
}

std::map<int, TrafficRecord::VehicleStartEndlTime> TrafficRecord::RemoveVehicleNotReachEnd(
    const sim_msg::Traffic& originTraffic, std::map<int, VehicleStartEndlTime> allVehicleStartEndTime) const {
  auto tmpAllVehicleStartEndTime = allVehicleStartEndTime;

  // Remove all vehicles still on the road (not reaching the end)
  for (const sim_msg::Car& car : originTraffic.cars()) {
    auto it = tmpAllVehicleStartEndTime.find(car.id());
    if (it != tmpAllVehicleStartEndTime.end()) {
      tmpAllVehicleStartEndTime.erase(it);
    }
  }
  return tmpAllVehicleStartEndTime;
}

bool TrafficRecord::getInputVehicle(uint64_t vehicleId, its::txVehicle& vehicle) const {
  if (inputVehicles_.find(vehicleId) == inputVehicles_.end()) {
    return false;
  } else {
    vehicle = inputVehicles_.at(vehicleId);
    return true;
  }
}

its::txSimStat TrafficRecord::getSimStatMsg(const sim_msg::Traffic& originTraffic, int64_t currentTime) const {
  its::txSimStat simStat;
  for (auto iter = allVehicleStartEndTime.begin(); iter != allVehicleStartEndTime.end(); iter++) {
    its::txVehicle vehicle;
    if (!getInputVehicle(iter->first, vehicle)) continue;

    LOG(WARNING) << "if equal vehicle:" << vehicle.DebugString();
    LOG(WARNING) << "actualPath.size():" << iter->second.actualPath.size();
    if (iter->second.actualPath.empty() ||
        !std::equal(vehicle.route().road_id().begin() + vehicle.road_idx(), vehicle.route().road_id().end(),
                    iter->second.actualPath.begin())) {
      LOG(WARNING) << "input route and actual route is not equal";
      LOG(WARNING) << "input route: ";
      for (auto it : vehicle.route().road_id()) {
        LOG(WARNING) << it << " ";
      }

      LOG(WARNING) << "actual route: ";
      for (auto it : iter->second.actualPath) {
        LOG(WARNING) << it << " ";
      }
      continue;
    }
    vehicle.set_start_time(iter->second.startTime);
    vehicle.set_arrive_time(iter->second.arriveTime);
    simStat.add_vehicle()->CopyFrom(vehicle);
  }

  auto finalAllRoadTravelTime = CalculateFinalRoadTravelTime(originTraffic, currentTime);
  its::txRoadsStatPeriod* roadStatMap = simStat.add_roads_stat_period();
  for (auto iter = finalAllRoadTravelTime.begin(); iter != finalAllRoadTravelTime.end(); iter++) {
    auto& roadStat = *roadStatMap->mutable_stats();
    auto& connStatDict = *roadStat[iter->first].mutable_conn_stat_dict();
    connStatDict[0].set_vehicle_count(iter->second.passVehicle);
    connStatDict[0].set_avg_time_cost(iter->second.averageTime);
    connStatDict[0].set_avg_speed(iter->second.roadLength * 1000 / iter->second.averageTime);
  }
  return simStat;
}

its::txSimStatus TrafficRecord::getCurrentTrafficState(const sim_msg::Traffic& originTraffic) const {
  LOG(INFO) << "[20210204] call getCurrentTrafficState" << TX_VARS(originTraffic.cars_size());

  // std::clock_t start = clock();
  its::txSimStatus startStatus;
  // LOG(WARNING) << _FootPrint_ << " originTraffic:" << originTraffic.DebugString();
  for (const sim_msg::Car& car : originTraffic.cars()) {
    // std::clock_t start1 = clock();
    // LOG(INFO) << "[20210204] car " << car.id();
    its::txVehicle vehicle;
    if (!getInputVehicle(car.id(), vehicle)) continue;
    // LOG(INFO) << "getInputVehicle cost:" << (double)(clock() - start1) * 1000 / CLOCKS_PER_SEC << "ms";
    if (allVehicleStartEndTime.find(car.id()) == allVehicleStartEndTime.end()) continue;
    // LOG(INFO) << "[20210204] getInputVehicle " << TXST_TRACE_VARIABLES(car.id());
    if (!std::equal(allVehicleStartEndTime.at(car.id()).actualPath.begin(),
                    allVehicleStartEndTime.at(car.id()).actualPath.end(), vehicle.route().road_id().begin())) {
      // LOG(WARNING) << car.id() << ": input route and actual route is not equal";
      // LOG(WARNING) << "input route: ";
      //       for(auto it: vehicle.route().road_id()) {
      //   LOG(WARNING) << it <<" ";
      //       }

      //       LOG(WARNING) << "actual route: ";
      //       for(auto it: allVehicleStartEndTime.at(car.id()).actualPath) {
      //   LOG(WARNING) << it <<" ";
      //       }
      continue;
    }
    ModifyVehicleForMeso(car, vehicle);

    auto it = std::find(vehicle.route().road_id().begin(), vehicle.route().road_id().end(), vehicle.road_id());
    int road_idx = it - vehicle.route().road_id().begin();

    vehicle.set_road_idx(road_idx);
    vehicle.set_speed(car.v());
    vehicle.set_lng(car.x());
    vehicle.set_lat(car.y());
    vehicle.set_lane_id(car.tx_lane_id());

    startStatus.add_vehicle()->CopyFrom(vehicle);
    // LOG(INFO) << "add_vehicle cost:" << (double)(clock() - start1) * 1000 / CLOCKS_PER_SEC << "ms";
  }
  startStatus.set_time(originTraffic.time());
  LOG(INFO) << "[20210204] call getCurrentTrafficState finish";
  // LOG(WARNING) << _FootPrint_ << " getCurrentTrafficState:" << startStatus.DebugString();
  // LOG(INFO) << "getCurrentTrafficState cost:" << (double)(clock() - start) * 1000 / CLOCKS_PER_SEC << "ms";
  return startStatus;
}

bool TrafficRecord::ModifyVehicleForMeso(const sim_msg::Car& car, its::txVehicle& vehicle) const {
  if (car.tx_road_id() != ROAD_PKID_INVALID) {
    // LOG(INFO) << "[20210305]" << TXST_TRACE_VARIABLES(car.tx_road_id());
    hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(car.tx_road_id());
    if (NonNull_Pointer(curRoadPtr) && NonNull_Pointer(curRoadPtr->getGeometry())) {
      hadmap::txPoint curPoint(car.x(), car.y(), car.z());
      hadmap::txPoint startPoint = curRoadPtr->getGeometry()->getStart();
      double distanceToStart = curRoadPtr->getGeometry()->getPassedDistance(curPoint, startPoint, hadmap::COORD_WGS84);
      vehicle.set_road_pos(distanceToStart);
      vehicle.set_road_id(car.tx_road_id());
    } else {
      LOG(WARNING) << "[20210305]" << TX_COND_NAME(NonNull_Pointer(curRoadPtr), NonNull_Pointer(curRoadPtr));
      return false;
    }
  } else {
    const Base::txLaneUId fromUid(car.tx_road_id(), car.tx_section_id(), car.tx_lane_id());
    const Base::txLaneUId toUid(car.to_tx_road_id(), car.to_tx_section_id(), car.to_tx_lane_id());
    hadmap::txLaneLinkPtr curLaneLinkPtr = HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(fromUid, toUid);
    if (NonNull_Pointer(curLaneLinkPtr)) {
      vehicle.set_road_pos(0.0);
      vehicle.set_road_id(curLaneLinkPtr->toRoadId());
    } else {
      LOG(WARNING) << "[20210305] link " << TX_VARS(car.tx_lanelink_id());
    }
  }
  // car.set_show_abs_velocity(car.start_position());
  // LOG(INFO) << "mutable_cars cost:" << (double)(clock() - start1) * 1000 / CLOCKS_PER_SEC << "ms";
  return true;
}
// sim_msg::Traffic TrafficRecord::RecordMesoInfo(const sim_msg::Traffic& originTraffic) const{
//     LOG(INFO) << "[20210204] call_RecordMesoInfo";
//     // std::clock_t start = clock();
//   sim_msg::Traffic mesoTraffic = originTraffic;
//   // LOG(INFO) << "CopyFrom(originTraffic cost:" << (double)(clock() - start) * 1000 / CLOCKS_PER_SEC << "ms";
//     // LOG(INFO) << "[20210204] CopyFrom finish." << TXST_TRACE_VARIABLES(mesoTraffic->cars_size());
//   for (sim_msg::Car& car : *(mesoTraffic.mutable_cars())) {
//         // LOG(INFO) << "[20210204] set car " << car.id();
//       // std::clock_t start1 = clock();
//     if(car.tx_road_id() != ROAD_PKID_INVALID) {
//             // LOG(INFO) << "[20210305]" << TXST_TRACE_VARIABLES(car.tx_road_id());
//             hadmap::txRoadPtr curRoadPtr = HdMap::HadmapCacheConCurrent::GetTxRoadPtr(car.tx_road_id());
//               if (NonNull_Pointer(curRoadPtr) && NonNull_Pointer(curRoadPtr->getGeometry()))
//               {
//                 hadmap::txPoint curPoint(car.x(), car.y(), car.z());
//               hadmap::txPoint startPoint = curRoadPtr->getGeometry()->getStart();
//               double distanceToStart = curRoadPtr->getGeometry()->getPassedDistance(curPoint,
// startPoint, hadmap::COORD_WGS84);               car.set_start_position(distanceToStart);
// car.set_show_abs_velocity(distanceToStart);//show distanceToStart in Tadsim for debug
//               }
//               else
//               {
//                   LOG(WARNING) << "[20210305]"
//                       << TXST_TRACE_COND_NAME(NonNull_Pointer(curRoadPtr), NonNull_Pointer(curRoadPtr));
//                   continue;
//               }
//       }
//       else
//         {
//           hadmap::txLaneLinkPtr curLaneLinkPtr =
//         HdMap::HadmapCacheConCurrent::GetTxLaneLinkPtr(car.tx_lanelink_id());
//             if (NonNull_Pointer(curLaneLinkPtr))
//             {
//                 car.set_start_position(0.0);
//                 car.set_tx_road_id(curLaneLinkPtr->toRoadId());
//             }
//             else
//             {
//                 LOG(WARNING) << "[20210305] link " << TXST_TRACE_VARIABLES(car.tx_lanelink_id());
//             }
//       }
//       // car.set_show_abs_velocity(car.start_position());
//     // LOG(INFO) << "mutable_cars cost:" << (double)(clock() - start1) * 1000 / CLOCKS_PER_SEC << "ms";
//   }
//   // LOG(INFO) << "RecordMesoInfo cost:" << (double)(clock() - start) * 1000 / CLOCKS_PER_SEC << "ms";
//   return mesoTraffic;
// }

void TrafficRecord::RecordTravelTime(const sim_msg::Traffic& originTraffic, int64_t currentTime) {
  // printf("RecordTravelTime\n");
  for (const sim_msg::Car& car : originTraffic.cars()) {
    // printf("id:%d ,t:%lf\n", car.id(), car.t());

    // Record vehicle start&end time
    if (allVehicleStartEndTime.find(car.id()) == allVehicleStartEndTime.end()) {
      VehicleStartEndlTime vehicleStartEndlTime;
      vehicleStartEndlTime.startTime = currentTime;
      allVehicleStartEndTime[car.id()] = vehicleStartEndlTime;
      if (car.tx_road_id() != -1) vehicleStartEndlTime.actualPath.push_back(car.tx_road_id());
    } else {
      allVehicleStartEndTime[car.id()].arriveTime = currentTime;
      if (car.tx_road_id() != -1 && (allVehicleStartEndTime[car.id()].actualPath.empty() ||
                                     car.tx_road_id() != allVehicleStartEndTime[car.id()].actualPath.back())) {
        allVehicleStartEndTime[car.id()].actualPath.push_back(car.tx_road_id());
      }
    }

    // Record road travel time
    if (allVehicleArriveRoadTime.find(car.id()) == allVehicleArriveRoadTime.end()) {
      VehicleArriveRoadTime vehicleArriveRoadTime;
      vehicleArriveRoadTime.arriveTime = currentTime;
      vehicleArriveRoadTime.curRoadId = car.tx_road_id();

      allVehicleArriveRoadTime[car.id()] = vehicleArriveRoadTime;
      // printf("creat:curRoadId:%ld ,arriveTime:%ld\n", car.tx_road_id(),
      // allVehicleArriveRoadTime[car.id()].curRoadId);
    } else {
      if (car.tx_road_id() != allVehicleArriveRoadTime[car.id()].curRoadId) {
        if (allVehicleArriveRoadTime[car.id()].curRoadId != -1) {
          // printf("update:tx_road_id:%d ,curRoadId:%d\n", car.tx_road_id(),
          // allVehicleArriveRoadTime[car.id()].curRoadId); printf("update:t:%lf ,arriveTime:%ld\n", car.t(),
          // allVehicleArriveRoadTime[car.id()].arriveTime);
          allRoadTravelTime =
              UpdateRoadTravelTime(allVehicleArriveRoadTime[car.id()].curRoadId,
                                   currentTime - allVehicleArriveRoadTime[car.id()].arriveTime, allRoadTravelTime);
        }
        allVehicleArriveRoadTime[car.id()].curRoadId = car.tx_road_id();
        allVehicleArriveRoadTime[car.id()].arriveTime = currentTime;
      }
    }
  }
}

void TrafficRecord::OutputTrafficRecordsPBFile(std::string pbFilePath) {
  // printf("OutputTrafficRecordsPBFile\n");
  // LOG(WARNING) << _FootPrint_ << traffic_records.DebugString();
  std::fstream output(pbFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!traffic_records.SerializeToOstream(&output)) {
    // std::cerr << "Failed to write traffic_records." << std::endl;
  }
  traffic_records.Clear();
}

void TrafficRecord::Clear() {
  allVehicleStartEndTime.clear();
  allRoadTravelTime.clear();
  allVehicleArriveRoadTime.clear();
  traffic_records.Clear();
}

void TrafficRecord::PartialClear() { traffic_records.Clear(); }

void TrafficRecord::RecordInputVehicles(const its::txVehicles& vehiclesMsg) {
  for (auto vehicleMsg : vehiclesMsg.vehicle()) {
    inputVehicles_[vehicleMsg.vehicle_id()].CopyFrom(vehicleMsg);
  }
}

void TrafficRecord::RecordInputVehicle(const its::txVehicle& vehicleMsg) {
  inputVehicles_[vehicleMsg.vehicle_id()].CopyFrom(vehicleMsg);
}

void TrafficRecord::RecordTrafficMsg(const sim_msg::Traffic& originTraffic) {
  traffic_records.add_traffic_record()->CopyFrom(originTraffic);
}

TX_NAMESPACE_CLOSE(TrafficFlow)
