/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/entity/pedestrianV2.h"

#include <boost/algorithm/string.hpp>
#include "common/engine/math/utils.h"
#include "common/xml_parser/entity/vehicle.h"

CPedestrianV2::CPedestrianV2() {}
bool CPedestrianV2::isDynamicObsctale() {
  if (this->m_strType == std::string("Port_Crane_001")) {
    return true;
  }
  if (this->m_strType == "Port_Crane_002") {
    return true;
  }
  if (this->m_strType == "Port_Crane_002_0_0") {
    return true;
  }
  if (this->m_strType == "Port_Crane_002_0_2") {
    return true;
  }
  if (this->m_strType == "Port_Crane_002_0_5") {
    return true;
  }
  if (this->m_strType == "Port_Crane_002_1_0") {
    return true;
  }
  if (this->m_strType == "Port_Crane_002_5_0") {
    return true;
  }
  if (this->m_strType == "Port_Crane_004") {
    return true;
  }
  if (this->m_strType == "Port_Crane_003") {
    return true;
  }
  return false;
}

void CPedestrianV2::Reset() {
  CPedestrian::Reset();
  m_strBehavior = "";
  m_strStartVelocity = "";
  m_strMaxVelocity = "";
  m_strAngle = "";
  m_strStartAngle = "0.0";

  m_nBehaviorType = 0;
  m_fStartVelocity = 0;
  m_fMaxVelocity = 0;
}

void CPedestrianV2::ConvertToValue() {
  CPedestrian::ConvertToValue();

  m_fStartVelocity = atof(m_strStartVelocity.c_str());
  m_fMaxVelocity = atof(m_strMaxVelocity.c_str());

  if (boost::algorithm::iequals(m_strBehavior, "UserDefine")) {
    m_nBehaviorType = CVehicle::VEHICLE_BEHAVIOR_USERDEFINE;
  } else if (boost::algorithm::iequals(m_strBehavior, "TrafficVehicle")) {
    m_nBehaviorType = CVehicle::VEHICLE_BEHAVIOR_TRAFFIC;
  } else if (boost::algorithm::iequals(m_strBehavior, "TrajectoryFollow")) {
    m_nBehaviorType = CVehicle::VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW;
  } else {
    assert(false);
  }
}

void CPedestrianV2::ConvertToStr() {
  CPedestrian::ConvertToStr();

  m_strStartVelocity = CUtils::doubleToStringDot3(m_fStartVelocity);
  m_strMaxVelocity = CUtils::doubleToStringDot3(m_fMaxVelocity);

  if (m_nBehaviorType == CVehicle::VEHICLE_BEHAVIOR_USERDEFINE) {
    m_strBehavior = "UserDefine";
  } else if (m_nBehaviorType == CVehicle::VEHICLE_BEHAVIOR_TRAFFIC) {
    m_strBehavior = "TrafficVehicle";
  } else if (m_nBehaviorType == CVehicle::VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW) {
    m_strBehavior = "TrajectoryFollow";
  } else {
    assert(false);
  }
}
