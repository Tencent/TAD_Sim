/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "pedestrianV2.h"
#include <boost/algorithm/string.hpp>
#include "common/xml_parser/entity/vehicle.h"

CPedestrianV2::CPedestrianV2() {}

void CPedestrianV2::Reset() {
  CPedestrian::Reset();
  m_strBehavior = "";
  m_strStartVelocity = "";
  m_strMaxVelocity = "";

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
  } else {
    assert(false);
  }
}

void CPedestrianV2::ConvertToStr() {
  CPedestrian::ConvertToStr();

  m_strStartVelocity = std::to_string(m_fStartVelocity);
  m_strMaxVelocity = std::to_string(m_fMaxVelocity);

  if (m_nBehaviorType == CVehicle::VEHICLE_BEHAVIOR_USERDEFINE) {
    m_strBehavior = "UserDefine";
  } else if (m_nBehaviorType == CVehicle::VEHICLE_BEHAVIOR_TRAFFIC) {
    m_strBehavior = "TrafficVehicle";
  } else {
    assert(false);
  }
}
