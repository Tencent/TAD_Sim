// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_protobuf_utils.h"
#include "tx_map_tracker.h"
#include "tx_math.h"
TX_NAMESPACE_OPEN(Utils)

Base::txString ProtobufDebugString(sim_msg::Car* _carMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_carMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_carMsg, &retStr)) {
      return retStr;
    } else {
      return (_carMsg->ShortDebugString());
      /*std::cerr << "Message not valid (partial content: "
          << _carMsg.ShortDebugString() << ")\n";*/
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugString(const sim_msg::Car& _carMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (google::protobuf::TextFormat::PrintToString(_carMsg, &retStr)) {
    return retStr;
  } else {
    return (_carMsg.ShortDebugString());
  }
}

Base::txString ProtobufDebugJson(sim_msg::Car* _carMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_carMsg) {
    if (google::protobuf::util::MessageToJsonString(*_carMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_carMsg->ShortDebugString());
      /*std::cerr << "Message not valid (partial content: "
          << _carMsg.ShortDebugString() << ")\n";*/
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugLogString(const sim_msg::Car& refCar) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << _StreamPrecision_ << TX_VARS_NAME(vehId, refCar.id()) << TX_VARS_NAME(type, refCar.type())
      << TX_VARS_NAME(lng, refCar.x()) << TX_VARS_NAME(lat, refCar.y()) << TX_VARS_NAME(alt, refCar.z())
      << TX_VARS_NAME(length, refCar.length()) << TX_VARS_NAME(width, refCar.width())
      << TX_VARS_NAME(height, refCar.height()) << TX_VARS_NAME(fromRoad, refCar.tx_road_id())
      << TX_VARS_NAME(fromSection, refCar.tx_section_id()) << TX_VARS_NAME(fromLane, refCar.tx_lane_id())
      << TX_VARS_NAME(toRoad, refCar.to_tx_road_id()) << TX_VARS_NAME(toSection, refCar.to_tx_section_id())
      << TX_VARS_NAME(toLane, refCar.to_tx_lane_id()) << TX_VARS_NAME(v, refCar.v()) << TX_VARS_NAME(v_l, refCar.vl())
      << TX_VARS_NAME(heading, refCar.heading());
  return oss.str();
}

Base::txString ProtobufDebugLogString(const sim_msg::StaticObstacle& refObs) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << _StreamPrecision_ << TX_VARS_NAME(obsId, refObs.id()) << TX_VARS_NAME(type, refObs.type())
      << TX_VARS_NAME(lng, refObs.x()) << TX_VARS_NAME(lat, refObs.y()) << TX_VARS_NAME(alt, refObs.z());
  return oss.str();
}

Base::txString ProtobufDebugLogString(const sim_msg::DynamicObstacle& refPed) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << _StreamPrecision_ << TX_VARS_NAME(pedId, refPed.id()) << TX_VARS_NAME(type, refPed.type())
      << TX_VARS_NAME(lng, refPed.x()) << TX_VARS_NAME(lat, refPed.y()) << TX_VARS_NAME(alt, refPed.z())
      << TX_VARS_NAME(length, refPed.length()) << TX_VARS_NAME(width, refPed.width())
      << TX_VARS_NAME(height, refPed.height()) << TX_VARS_NAME(v, refPed.v())
      << TX_VARS_NAME(heading, refPed.heading());
  return oss.str();
}

Base::txString ProtobufDebugLogString(const sim_msg::TrafficLight& refSignal) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << _StreamPrecision_ << TX_VARS_NAME(sigId, refSignal.id()) << TX_VARS_NAME(color, refSignal.color())
      << TX_VARS_NAME(lng, refSignal.x()) << TX_VARS_NAME(lat, refSignal.y()) << TX_VARS_NAME(alt, refSignal.z())
      << TX_VARS_NAME(age, refSignal.age()) << TX_VARS_NAME(plan, refSignal.plan())
      << TX_VARS_NAME(junction, refSignal.junction()) << TX_VARS_NAME(phasenumber, refSignal.phasenumber())
      << TX_VARS_NAME(signalhead, refSignal.signalhead());
  return oss.str();
}

Base::txString ProtobufDebugLogString(const its::txVehicle& _vehicleMsg) TX_NOEXCEPT {
  std::ostringstream oss;
  oss << _StreamPrecision_ << TX_VARS_NAME(vehId, _vehicleMsg.vehicle_id())
      << TX_VARS_NAME(type, _vehicleMsg.vehicle_type()) << TX_VARS_NAME(lng, _vehicleMsg.lng())
      << TX_VARS_NAME(lat, _vehicleMsg.lat())
      << TX_VARS_NAME(length, _vehicleMsg.length()) /*<< TX_VARS_NAME(width, _vehicleMsg.vehicle_width()) <<
                                                       TX_VARS_NAME(height, _vehicleMsg.vehicle_height())*/
      << TX_VARS_NAME(fromRoad, _vehicleMsg.road_id()) << TX_VARS_NAME(fromSection, _vehicleMsg.section_id())
      << TX_VARS_NAME(fromLane, _vehicleMsg.lane_id()) << TX_VARS_NAME(toRoad, _vehicleMsg.to_road_id())
      << TX_VARS_NAME(toSection, _vehicleMsg.to_section_id()) << TX_VARS_NAME(toLane, _vehicleMsg.to_lane_id())
      << TX_VARS_NAME(v, _vehicleMsg.speed()) << TX_VARS_NAME(heading, _vehicleMsg.yaw());
  return oss.str();
}

Base::txString ProtobufDebugString(sim_msg::StaticObstacle* _obsMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_obsMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_obsMsg, &retStr)) {
      return retStr;
    } else {
      return (_obsMsg->ShortDebugString());
      /*std::cerr << "Message not valid (partial content: "
          << _carMsg.ShortDebugString() << ")\n";*/
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugJson(sim_msg::StaticObstacle* _obsMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_obsMsg) {
    if (google::protobuf::util::MessageToJsonString(*_obsMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_obsMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugString(sim_msg::DynamicObstacle* _obsMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_obsMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_obsMsg, &retStr)) {
      return retStr;
    } else {
      return (_obsMsg->ShortDebugString());
      /*std::cerr << "Message not valid (partial content: "
          << _carMsg.ShortDebugString() << ")\n";*/
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugJson(sim_msg::DynamicObstacle* _obsMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_obsMsg) {
    if (google::protobuf::util::MessageToJsonString(*_obsMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_obsMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugString(sim_msg::TrafficLight* _tlMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_tlMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_tlMsg, &retStr)) {
      return retStr;
    } else {
      return (_tlMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugJson(sim_msg::TrafficLight* _tlMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_tlMsg) {
    if (google::protobuf::util::MessageToJsonString(*_tlMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_tlMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugString(sim_msg::Location* _egoMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_egoMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_egoMsg, &retStr)) {
      return retStr;
    } else {
      return (_egoMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugJson(sim_msg::Location* _egoMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_egoMsg) {
    if (google::protobuf::util::MessageToJsonString(*_egoMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_egoMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugString(sim_msg::SimMessage* _outputMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_outputMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_outputMsg, &retStr)) {
      return retStr;
    } else {
      return (_outputMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString txLaneLinksToString(const hadmap::txLaneLinks& refLaneLinks) TX_NOEXCEPT {
  std::stringstream ss;
  ss << "txLaneLinks:{";
  for (const auto& link : refLaneLinks) {
    if (link) {
      ss << TX_VARS_NAME(Id, link->getId()) << TX_VARS_NAME(LinkType, link->getLinkType())
         << TX_VARS_NAME(fromRoadId, link->fromRoadId()) << TX_VARS_NAME(fromSectionId, link->fromSectionId())
         << TX_VARS_NAME(fromLaneId, link->fromLaneId()) << TX_VARS_NAME(fromTxLaneId, link->fromTxLaneId())
         << TX_VARS_NAME(toRoadId, link->toRoadId()) << TX_VARS_NAME(toSectionId, link->toSectionId())
         << TX_VARS_NAME(toLaneId, link->toLaneId()) << TX_VARS_NAME(toTxLaneId, link->toTxLaneId())
         << TX_VARS_NAME(UniqueId, link->getUniqueId());
    }
  }
  ss << "}";
  return ss.str();
}

Base::txString ProtobufDebugString(sim_msg::DITW_Event* _outputMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_outputMsg) {
    if (google::protobuf::TextFormat::PrintToString(*_outputMsg, &retStr)) {
      return retStr;
    } else {
      return (_outputMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txString ProtobufDebugJson(sim_msg::DITW_Event* _outputMsg) TX_NOEXCEPT {
  Base::txString retStr;
  if (_outputMsg) {
    if (google::protobuf::util::MessageToJsonString(*_outputMsg, &retStr).ok()) {
      return retStr;
    } else {
      return (_outputMsg->ShortDebugString());
    }
  } else {
    return retStr;
  }
}

Base::txBool IsEqual(const sim_msg::Car& leftVehicle, const sim_msg::Car& rightVehicle) TX_NOEXCEPT {
  return (leftVehicle.id() == rightVehicle.id()) &&
         // (Math::isEqual(leftVehicle.t(), rightVehicle.t())) &&
         (Math::isZero(leftVehicle.x() - rightVehicle.x(), 1e-8)) &&
         (Math::isZero(leftVehicle.y() - rightVehicle.y(), 1e-8)) &&
         (Math::isZero(leftVehicle.v() - rightVehicle.v(), 1e-8)) &&
         (Math::isZero(leftVehicle.heading() - rightVehicle.heading(), 0.05));
#if 0
    message Car {
  int64 id = 1;
  double t = 2;
  double x = 3;
  double y = 4;
  double heading = 5;
  double v = 6;
  double vl = 7;
  double theta = 8;
  double length = 9;
  double width = 10;
  double height = 11;
  int32 type = 12;
  int32 laneId = 13;
  double acc = 14;
  int32 age = 15;
  double z = 16;
  uint64 tx_road_id = 17;
  uint64 tx_section_id = 18;
  int64 tx_lane_id = 19;
  uint64 tx_lanelink_id = 20;
  Trajectory planning_line = 21;
  double show_abs_velocity = 22;
  double show_abs_acc = 23;
  double show_relative_velocity = 24; /*vertical along to reference line*/
    double show_relative_acc = 25; /*vertical along to reference line*/
    double show_relative_velocity_horizontal = 26; /*horizontal along to reference line*/
    double show_relative_acc_horizontal = 27; /*horizontal along to reference line*/
    double show_relative_dist_vertical = 28;
    double show_relative_dist_horizontal = 29;
    ControlState control_state = 30;
    /* note: 1. fixed size 8*/ /*std::vector< NearestObject > show_NearestObjectInfo; */
    repeated NearestObject show_NearestObjectInfo = 31;
    double angle_along_centerline_degree = 32;
    string strid = 33;
    TwinInfo twin_data = 34;
}
#endif
}

TX_NAMESPACE_CLOSE(Utils)
