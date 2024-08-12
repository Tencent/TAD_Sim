/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include <cassert>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/vehicle.h"

CVehicle::CVehicle() {}

CVehicle::CVehicle(CVehicleParam vp) {
  /*
  m_strID = vp.m_strID;
  m_strRouteID = vp.m_strRouteID;
  m_strLaneID = vp.m_strLaneID;
  m_strStartShift = vp.m_strStartShift;
  m_strStartTime = vp.m_strStartTime;
  m_strStartVelocity = vp.m_strStartVelocity;
  m_strMaxVelocity = vp.m_strMaxVelocity;
  m_strOffset = vp.m_strOffset;
  m_strLength = vp.m_strLength;
  m_strWidth = vp.m_strWidth;
  m_strHeight = vp.m_strHeight;
  m_strType = vp.m_strType;
  m_strAccID = vp.m_strAccID;
  m_strMergeID = vp.m_strMergeID;
  m_strAggress = vp.m_strAggress;
  m_strBehavior = vp.m_strBehavior;
  */
  Copy(&vp);
}

void CVehicle::Reset() {
  m_strName = "";

  m_strID = "";
  m_strRouteID = "";
  m_strLaneID = "";
  m_strStartShift = "";
  m_strStartTime = "";
  m_strStartVelocity = "";
  m_strMaxVelocity = "-1";
  m_strOffset = "";
  // m_strLength = "";
  // m_strWidth = "";
  // m_strHeight = "";
  m_strType = "";
  m_strAccID = "";
  m_strMergeID = "";
  m_strAggress = "-1";
  m_strBehavior = "UserDefine";
  m_strFollow = "";
  m_strObuStauts = "0";
  m_ID = 0;
  m_routeID = 0;
  m_laneID = 0;
  m_accID = 0;
  m_mergeID = 0;
  m_type = 0;
  m_behaviorType = VEHICLE_BEHAVIOR_USERDEFINE;
  m_fStartShift = 0;
  m_fStartVelocity = 0;
  m_fMaxVelocity = -1.0f;
  m_fStartTime = 0;
  m_fStartOffset = 0;
  // m_fVehicleLength = 0;
  // m_fVehicleWidth = 0;
  // m_fVehicleHeight = 0;
  m_dAggress = -1;
}

void CVehicle::Copy(CVehicle* pV) {
  if (!pV) {
    assert(false);
    return;
  }

  m_strName = pV->m_strName;

  m_strID = pV->m_strID;
  m_strRouteID = pV->m_strRouteID;
  m_strLaneID = pV->m_strLaneID;
  m_strStartShift = pV->m_strStartShift;
  m_strStartTime = pV->m_strStartTime;
  m_strStartVelocity = pV->m_strStartVelocity;
  m_strMaxVelocity = pV->m_strMaxVelocity;
  m_strOffset = pV->m_strOffset;
  // m_strLength = pV->m_strLength;
  // m_strWidth = pV->m_strWidth;
  // m_strHeight = pV->m_strHeight;
  m_strType = pV->m_strType;
  m_strAccID = pV->m_strAccID;
  m_strMergeID = pV->m_strMergeID;
  m_strAggress = pV->m_strAggress;
  m_strBehavior = pV->m_strBehavior;
  m_strFollow = pV->m_strFollow;
  m_strObuStauts = pV->m_strObuStauts;
  m_strStartAngle = pV->m_strStartAngle;
  m_strSensorGroup = pV->m_strSensorGroup;

  m_ID = pV->m_ID;
  m_routeID = pV->m_routeID;
  m_laneID = pV->m_laneID;
  m_accID = pV->m_accID;
  m_mergeID = pV->m_mergeID;
  m_type = pV->m_type;
  m_behaviorType = pV->m_behaviorType;
  m_fStartShift = pV->m_fStartShift;
  m_fStartVelocity = pV->m_fStartVelocity;
  m_fMaxVelocity = pV->m_fMaxVelocity;
  m_fStartTime = pV->m_fStartTime;
  m_fStartOffset = pV->m_fStartOffset;
  // m_fVehicleLength = pV->m_fVehicleLength;
  // m_fVehicleWidth = pV->m_fVehicleWidth;
  // m_fVehicleHeight = pV->m_fVehicleHeight;
  m_dAggress = pV->m_dAggress;
  m_boundingBox = pV->m_boundingBox;
}

void CVehicle::ConvertToValue() {
  try {
    m_ID = std::stoi(m_strID);
    m_routeID = std::stoi(m_strRouteID);
  } catch (const std::exception& ex) {
    char err[512] = {0};
    snprintf(err, sizeof(err), "V:%s, ID:%s or RID:%s must be non-empty", m_strName.c_str(), m_strID.c_str(),
             m_strRouteID.c_str());
    SYSTEM_LOGGER_ERROR("%s", err);
    throw std::invalid_argument(err);
  }
  if (m_strLaneID.empty()) {
    m_laneID = 0;
  } else {
    m_laneID = std::atoi(m_strLaneID.c_str());
  }
  m_accID = std::atoi(m_strAccID.c_str());
  m_mergeID = std::atoi(m_strMergeID.c_str());

  m_type = GetVehicleType(m_strType);

  if (boost::algorithm::iequals(m_strBehavior, "UserDefine")) {
    m_behaviorType = VEHICLE_BEHAVIOR_USERDEFINE;
  } else if (boost::algorithm::iequals(m_strBehavior, "TrafficVehicle")) {
    m_behaviorType = VEHICLE_BEHAVIOR_TRAFFIC;
  } else if (boost::algorithm::iequals(m_strBehavior, "TrafficVehicleArterial")) {
    m_behaviorType = VEHICLE_BEHAVIOR_TRAFFICARTERIAL;
  } else if (boost::algorithm::iequals(m_strBehavior, "TrajectoryFollow")) {
    m_behaviorType = VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW;
  } else {
    // assert(false);
    SYSTEM_LOGGER_INFO("vechicle behavior error=%s", m_strBehavior.c_str());
  }

  m_fStartShift = atof(m_strStartShift.c_str());
  m_fStartVelocity = atof(m_strStartVelocity.c_str());
  if (m_strMaxVelocity.size() > 0) {
    m_fMaxVelocity = atof(m_strMaxVelocity.c_str());
  } else {
    m_fMaxVelocity = -1.0f;
  }
  m_fStartTime = atof(m_strStartTime.c_str());
  m_fStartOffset = atof(m_strOffset.c_str());
  m_dAggress = atof(m_strAggress.c_str());
}

void CVehicle::ConvertToStr() {
  m_strID = std::to_string(m_ID);
  m_strRouteID = std::to_string(m_routeID);
  m_strLaneID = std::to_string(m_laneID);
  m_strStartShift = CUtils::doubleToStringDot3(m_fStartShift);
  m_strStartTime = CUtils::doubleToStringDot3(m_fStartTime);
  m_strStartVelocity = CUtils::doubleToStringDot3(m_fStartVelocity);
  m_strMaxVelocity = CUtils::doubleToStringDot3(m_fMaxVelocity);
  m_strOffset = CUtils::doubleToStringDot3(m_fStartOffset);
  m_strAccID = std::to_string(m_accID);
  m_strMergeID = std::to_string(m_mergeID);
  m_strAggress = CUtils::doubleToStringDot3(m_dAggress);
  if (m_type == VEHICLE_TYPE_SEDAN) {
    m_strType = "Sedan";
  } else if (m_type == VEHICLE_TYPE_SUV) {
    m_strType = "SUV";
  } else if (m_type == VEHICLE_TYPE_BUS) {
    m_strType = "Bus";
  } else if (m_type == VEHICLE_TYPE_TRUNK) {
    m_strType = "Truck";
  } else if (m_type == VEHICLE_TYPE_TRUNK_1) {
    m_strType = "Truck_1";
  } else if (m_type == VEHICLE_TYPE_SEDAN_001) {
    m_strType = "Sedan_001";
  } else if (m_type == VEHICLE_TYPE_SEDAN_002) {
    m_strType = "Sedan_002";
  } else if (m_type == VEHICLE_TYPE_SEDAN_003) {
    m_strType = "Sedan_003";
  } else if (m_type == VEHICLE_TYPE_SEDAN_004) {
    m_strType = "Sedan_004";
  } else if (m_type == VEHICLE_TYPE_SUV_001) {
    m_strType = "SUV_001";
  } else if (m_type == VEHICLE_TYPE_SUV_002) {
    m_strType = "SUV_002";
  } else if (m_type == VEHICLE_TYPE_SUV_003) {
    m_strType = "SUV_003";
  } else if (m_type == VEHICLE_TYPE_SUV_004) {
    m_strType = "SUV_004";
  } else if (m_type == VEHICLE_TYPE_SUV_005) {
    m_strType = "SUV_005";
  } else if (m_type == VEHICLE_TYPE_TRUCK_002) {
    m_strType = "Truck_002";
  } else if (m_type == VEHICLE_TYPE_TRUCK_003) {
    m_strType = "Truck_003";
  } else if (m_type == VEHICLE_TYPE_BUS_001) {
    m_strType = "Bus_001";
  } else if (m_type == VEHICLE_TYPE_SEMI_TRAILER_TRUCK_001) {
    m_strType = "Semi_Trailer_Truck_001";
  } else if (m_type == VEHICLE_TYPE_AMBULANCE) {
    m_strType = "Ambulance";
  } else if (m_type == VEHICLE_TYPE_AMBULANCE_001) {
    m_strType = "Ambulance_001";
  } else if (m_type == VEHICLE_TYPE_TRUCK_004) {
    m_strType = "Truck_004";
  } else if (m_type == VEHICLE_TYPE_SEDAN_005) {
    m_strType = "Sedan_005";
  } else if (m_type == VEHICLE_TYPE_AdminVehicle_001) {
    m_strType = "AdminVehicle_01";
  } else if (m_type == VEHICLE_TYPE_AdminVehicle_002) {
    m_strType = "AdminVehicle_02";
  } else if (m_type == VEHICLE_TYPE_BUS_005) {
    m_strType = "Bus_005";
  } else if (m_type == VEHICLE_TYPE_BUS_004) {
    m_strType = "Bus_004";
  } else if (m_type == VEHICLE_TYPE_BUS_003) {
    m_strType = "Bus_003";
  } else if (m_type == VEHICLE_TYPE_BUS_002) {
    m_strType = "Bus_002";
  } else if (m_type == VEHICLE_TYPE_TRICLE_002) {
    m_strType = "tricycle_002";
  } else if (m_type == VEHICLE_TYPE_AIV_FullLoad_001) {
    m_strType = "AIV_FullLoad_001";
  } else if (m_type == VEHICLE_TYPE_AIV_Empty_001) {
    m_strType = "AIV_Empty_001";
  } else if (m_type == VEHICLE_TYPE_MPV_001) {
    m_strType = "MPV_001";
  } else if (m_type == VEHICLE_TYPE_AIV_FullLoad_002) {
    m_strType = "AIV_FullLoad_002";
  } else if (m_type == VEHICLE_TYPE_AIV_Empty_002) {
    m_strType = "AIV_Empty_002";
  } else if (m_type == VEHICLE_TYPE_MIFA_01) {
    m_strType = "MIFA_01";
  } else if (m_type == VEHICLE_TYPE_Truck_with_cargobox) {
    m_strType = "Truck_with_cargobox";
  } else if (m_type == VEHICLE_TYPE_Truck_without_cargobox) {
    m_strType = "Truck_without_cargobox";
  } else if (m_type == VEHICLE_TYPE_MPV_002) {
    m_strType = "MPV_002";
  } else if (m_type == VEHICLE_TYPE_MiningDumpTruck_001) {
    m_strType = "MiningDumpTruck_001";
  } else if (m_type == VEHICLE_TYPE_AIV_V5_FullLoad_001) {
    m_strType = "AIV_V5_FullLoad_001";
  } else if (m_type == VEHICLE_TYPE_AIV_V5_Empty_001) {
    m_strType = "AIV_V5_Empty_001";
  } else if (m_type == VEHICLE_TYPE_LandRover_RangeRover_2010) {
    m_strType = "LandRover_RangeRover_2010";
  } else if (m_type == VEHICLE_TYPE_BMW_MINI_CooperS_2010) {
    m_strType = "BMW_MINI_CooperS_2010";
  } else if (m_type == VEHICLE_TYPE_Opel_Corsa_2009) {
    m_strType = "Opel_Corsa_2009";
  } else if (m_type == VEHICLE_TYPE_Lexus_IS_2009) {
    m_strType = "Lexus_IS_2009";
  } else if (m_type == VEHICLE_TYPE_Suzuki_Celerio_2011) {
    m_strType = "Suzuki_Celerio_2011";
  } else if (m_type == VEHICLE_TYPE_Lancia_DeltaIntegrale_Evo1) {
    m_strType = "Lancia_DeltaIntegrale_Evo1";
  } else if (m_type == VEHICLE_TYPE_Chevrolet_ExpressVan1500_2001) {
    m_strType = "Chevrolet_ExpressVan1500_2001";
  } else if (m_type == VEHICLE_TYPE_MercedesBenz_SL_2005) {
    m_strType = "MercedesBenz_SL_2005";
  } else if (m_type == VEHICLE_TYPE_Dodge_Charger_2006) {
    m_strType = "Dodge_Charger_2006";
  } else if (m_type == VEHICLE_TYPE_Porsche_911_TurboS_2020) {
    m_strType = "Porsche_911_TurboS_2020";
  } else if (m_type == VEHICLE_TYPE_Nissan_GTR_2009) {
    m_strType = "Nissan_GTR_2009";
  } else if (m_type == VEHICLE_TYPE_Opel_Ampera_2009) {
    m_strType = "Opel_Ampera_2009";
  } else if (m_type == VEHICLE_TYPE_Dodge_ViperMambaEdition_RCH680) {
    m_strType = "Dodge_ViperMambaEdition_RCH680";
  } else if (m_type == VEHICLE_TYPE_Mitsubishi_iMiEV_2018) {
    m_strType = "Mitsubishi_iMiEV_2018";
  } else if (m_type == VEHICLE_TYPE_Audi_A8_2015) {
    m_strType = "Audi_A8_2015";
  } else if (m_type == VEHICLE_TYPE_Toyota_Tacoma_2017) {
    m_strType = "Toyota_Tacoma_2017";
  } else if (m_type == VEHICLE_TYPE_Lamborghini_Gallardo_2011) {
    m_strType = "Lamborghini_Gallardo_2011";
  } else if (m_type == VEHICLE_TYPE_MercedesBenz_SLSAMG_2014) {
    m_strType = "MercedesBenz_SLSAMG_2014";
  } else if (m_type == VEHICLE_TYPE_Scania_R620_2012) {
    m_strType = "Scania_R620_2012";
  } else if (m_type == VEHICLE_TYPE_MAN_LionsCoach_2003) {
    m_strType = "MAN_LionsCoach_2003";
  } else if (m_type == VEHICLE_TYPE_Cadillac_XTSLimousine_2018) {
    m_strType = "Cadillac_XTSLimousine_2018";
  } else if (m_type == VEHICLE_TYPE_MercedesBenz_Arocs_3240) {
    m_strType = "MercedesBenz_Arocs_3240";
  } else if (m_type == VEHICLE_TYPE_BMW_X6_2014) {
    m_strType = "BMW_X6_2014";
  } else if (m_type == VEHICLE_TYPE_Truck_004) {
    m_strType = "Truck_004";
  } else if (m_type == VEHICLE_TYPE_Half_loaded_Trailer) {
    m_strType = "Half_loaded_Trailer";
  } else if (m_type == VEHICLE_TYPE_Full_loaded_Trailer) {
    m_strType = "Full_loaded_Trailer";
  } else if (m_type == VEHICLE_TYPE_Unloaded_Trailer) {
    m_strType = "Unloaded_Trailer";
  } else {
    // assert(false);
    SYSTEM_LOGGER_INFO("11vechicle type error=%d", m_type);
  }

  if (m_behaviorType == VEHICLE_BEHAVIOR_USERDEFINE) {
    m_strBehavior = "UserDefine";
  } else if (m_behaviorType == VEHICLE_BEHAVIOR_TRAFFIC) {
    m_strBehavior = "TrafficVehicle";
  } else if (m_behaviorType == VEHICLE_BEHAVIOR_TRAFFICARTERIAL) {
    m_strBehavior = "TrafficVehicleArterial";
  } else if (m_behaviorType == VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW) {
    m_strBehavior = "TrajectoryFollow";
  } else {
    // assert(false);
    SYSTEM_LOGGER_INFO("11vechicle behavior error=%d", m_behaviorType);
  }
}

CVehicle::VEHICLE_TYPE CVehicle::GetVehicleType(std::string type) {
  VEHICLE_TYPE nType;
  if (boost::algorithm::iequals(type, "SUV")) {
    nType = VEHICLE_TYPE_SUV;
  } else if (boost::algorithm::iequals(type, "Sedan")) {
    nType = VEHICLE_TYPE_SEDAN;
  } else if (boost::algorithm::iequals(type, "Bus")) {
    nType = VEHICLE_TYPE_BUS;
  } else if (boost::algorithm::iequals(type, "Trunk") || boost::algorithm::iequals(type, "Truck")) {
    nType = VEHICLE_TYPE_TRUNK;
  } else if (boost::algorithm::iequals(type, "Trunk_1") || boost::algorithm::iequals(type, "Truck_1")) {
    nType = VEHICLE_TYPE_TRUNK_1;
  } else if (boost::algorithm::iequals(type, "Sedan_001")) {
    nType = VEHICLE_TYPE_SEDAN_001;
  } else if (boost::algorithm::iequals(type, "Sedan_002")) {
    nType = VEHICLE_TYPE_SEDAN_002;
  } else if (boost::algorithm::iequals(type, "Sedan_003")) {
    nType = VEHICLE_TYPE_SEDAN_003;
  } else if (boost::algorithm::iequals(type, "Sedan_004")) {
    nType = VEHICLE_TYPE_SEDAN_004;
  } else if (boost::algorithm::iequals(type, "SUV_001")) {
    nType = VEHICLE_TYPE_SUV_001;
  } else if (boost::algorithm::iequals(type, "SUV_002")) {
    nType = VEHICLE_TYPE_SUV_002;
  } else if (boost::algorithm::iequals(type, "SUV_003")) {
    nType = VEHICLE_TYPE_SUV_003;
  } else if (boost::algorithm::iequals(type, "SUV_004")) {
    nType = VEHICLE_TYPE_SUV_004;
  } else if (boost::algorithm::iequals(type, "SUV_005")) {
    nType = VEHICLE_TYPE_SUV_005;
  } else if (boost::algorithm::iequals(type, "Truck_002")) {
    nType = VEHICLE_TYPE_TRUCK_002;
  } else if (boost::algorithm::iequals(type, "Truck_003")) {
    nType = VEHICLE_TYPE_TRUCK_003;
  } else if (boost::algorithm::iequals(type, "Bus_001")) {
    nType = VEHICLE_TYPE_BUS_001;
  } else if (boost::algorithm::iequals(type, "Semi_Trailer_Truck_001")) {
    nType = VEHICLE_TYPE_SEMI_TRAILER_TRUCK_001;
  } else if (boost::algorithm::iequals(type, "Ambulance")) {
    nType = VEHICLE_TYPE_AMBULANCE;
  } else if (boost::algorithm::iequals(type, "Ambulance_001")) {
    nType = VEHICLE_TYPE_AMBULANCE_001;
  } else if (boost::algorithm::iequals(type, "Truck_004")) {
    nType = VEHICLE_TYPE_TRUCK_004;
  } else if (boost::algorithm::iequals(type, "Sedan_005")) {
    nType = VEHICLE_TYPE_SEDAN_005;
  } else if (boost::algorithm::iequals(type, "AdminVehicle_01")) {
    nType = VEHICLE_TYPE_AdminVehicle_001;
  } else if (boost::algorithm::iequals(type, "AdminVehicle_02")) {
    nType = VEHICLE_TYPE_AdminVehicle_002;
  } else if (boost::algorithm::iequals(type, "Bus_005")) {
    nType = VEHICLE_TYPE_BUS_005;
  } else if (boost::algorithm::iequals(type, "Bus_004")) {
    nType = VEHICLE_TYPE_BUS_004;
  } else if (boost::algorithm::iequals(type, "Bus_003")) {
    nType = VEHICLE_TYPE_BUS_003;
  } else if (boost::algorithm::iequals(type, "Bus_002")) {
    nType = VEHICLE_TYPE_BUS_002;
  } else if (boost::algorithm::iequals(type, "tricycle_002")) {
    nType = VEHICLE_TYPE_TRICLE_002;
  } else if (boost::algorithm::iequals(type, "AIV_FullLoad_001")) {
    nType = VEHICLE_TYPE_AIV_FullLoad_001;
  } else if (boost::algorithm::iequals(type, "AIV_Empty_001")) {
    nType = VEHICLE_TYPE_AIV_Empty_001;
  } else if (boost::algorithm::iequals(type, "MPV_001")) {
    nType = VEHICLE_TYPE_MPV_001;
  } else if (boost::algorithm::iequals(type, "Truck_with_cargobox")) {
    nType = VEHICLE_TYPE_Truck_with_cargobox;
  } else if (boost::algorithm::iequals(type, "Truck_without_cargobox")) {
    nType = VEHICLE_TYPE_Truck_without_cargobox;
  } else if (boost::algorithm::iequals(type, "MPV_002")) {
    nType = VEHICLE_TYPE_MPV_002;
  } else if (boost::algorithm::iequals(type, "MIFA_01")) {
    nType = VEHICLE_TYPE_MIFA_01;
  } else if (boost::algorithm::iequals(type, "AIV_Empty_002")) {
    nType = VEHICLE_TYPE_AIV_Empty_002;
  } else if (boost::algorithm::iequals(type, "AIV_FullLoad_002")) {
    nType = VEHICLE_TYPE_AIV_FullLoad_002;
  } else if (boost::algorithm::iequals(type, "MiningDumpTruck_001")) {
    nType = VEHICLE_TYPE_MiningDumpTruck_001;
  } else if (boost::algorithm::iequals(type, "AIV_V5_FullLoad_001")) {
    nType = VEHICLE_TYPE_AIV_V5_FullLoad_001;
  } else if (boost::algorithm::iequals(type, "AIV_V5_Empty_001")) {
    nType = VEHICLE_TYPE_AIV_V5_Empty_001;
  } else if (boost::algorithm::iequals(type, "LandRover_RangeRover_2010")) {
    nType = VEHICLE_TYPE_LandRover_RangeRover_2010;
  } else if (boost::algorithm::iequals(type, "BMW_MINI_CooperS_2010")) {
    nType = VEHICLE_TYPE_BMW_MINI_CooperS_2010;
  } else if (boost::algorithm::iequals(type, "Opel_Corsa_2009")) {
    nType = VEHICLE_TYPE_Opel_Corsa_2009;
  } else if (boost::algorithm::iequals(type, "Lexus_IS_2009")) {
    nType = VEHICLE_TYPE_Lexus_IS_2009;
  } else if (boost::algorithm::iequals(type, "Suzuki_Celerio_2011")) {
    nType = VEHICLE_TYPE_Suzuki_Celerio_2011;
  } else if (boost::algorithm::iequals(type, "Lancia_DeltaIntegrale_Evo1")) {
    nType = VEHICLE_TYPE_Lancia_DeltaIntegrale_Evo1;
  } else if (boost::algorithm::iequals(type, "Chevrolet_ExpressVan1500_2001")) {
    nType = VEHICLE_TYPE_Chevrolet_ExpressVan1500_2001;
  } else if (boost::algorithm::iequals(type, "MercedesBenz_SL_2005")) {
    nType = VEHICLE_TYPE_MercedesBenz_SL_2005;
  } else if (boost::algorithm::iequals(type, "Dodge_Charger_2006")) {
    nType = VEHICLE_TYPE_Dodge_Charger_2006;
  } else if (boost::algorithm::iequals(type, "Porsche_911_TurboS_2020")) {
    nType = VEHICLE_TYPE_Porsche_911_TurboS_2020;
  } else if (boost::algorithm::iequals(type, "Nissan_GTR_2009")) {
    nType = VEHICLE_TYPE_Nissan_GTR_2009;
  } else if (boost::algorithm::iequals(type, "Opel_Ampera_2009")) {
    nType = VEHICLE_TYPE_Opel_Ampera_2009;
  } else if (boost::algorithm::iequals(type, "Dodge_ViperMambaEdition_RCH680")) {
    nType = VEHICLE_TYPE_Dodge_ViperMambaEdition_RCH680;
  } else if (boost::algorithm::iequals(type, "Mitsubishi_iMiEV_2018")) {
    nType = VEHICLE_TYPE_Mitsubishi_iMiEV_2018;
  } else if (boost::algorithm::iequals(type, "Audi_A8_2015")) {
    nType = VEHICLE_TYPE_Audi_A8_2015;
  } else if (boost::algorithm::iequals(type, "Toyota_Tacoma_2017")) {
    nType = VEHICLE_TYPE_Toyota_Tacoma_2017;
  } else if (boost::algorithm::iequals(type, "Lamborghini_Gallardo_2011")) {
    nType = VEHICLE_TYPE_Lamborghini_Gallardo_2011;
  } else if (boost::algorithm::iequals(type, "MercedesBenz_SLSAMG_2014")) {
    nType = VEHICLE_TYPE_MercedesBenz_SLSAMG_2014;
  } else if (boost::algorithm::iequals(type, "Scania_R620_2012")) {
    nType = VEHICLE_TYPE_Scania_R620_2012;
  } else if (boost::algorithm::iequals(type, "MAN_LionsCoach_2003")) {
    nType = VEHICLE_TYPE_MAN_LionsCoach_2003;
  } else if (boost::algorithm::iequals(type, "Cadillac_XTSLimousine_2018")) {
    nType = VEHICLE_TYPE_Cadillac_XTSLimousine_2018;
  } else if (boost::algorithm::iequals(type, "MercedesBenz_Arocs_3240")) {
    nType = VEHICLE_TYPE_MercedesBenz_Arocs_3240;
  } else if (boost::algorithm::iequals(type, "BMW_X6_2014")) {
    nType = VEHICLE_TYPE_BMW_X6_2014;
  } else if (boost::algorithm::iequals(type, "Truck_004")) {
    nType = VEHICLE_TYPE_Truck_004;
  } else if (boost::algorithm::iequals(type, "Half_loaded_Trailer")) {
    nType = VEHICLE_TYPE_Half_loaded_Trailer;
  } else if (boost::algorithm::iequals(type, "Full_loaded_Trailer")) {
    nType = VEHICLE_TYPE_Full_loaded_Trailer;
  } else if (boost::algorithm::iequals(type, "Unloaded_Trailer")) {
    nType = VEHICLE_TYPE_Unloaded_Trailer;
  } else {
    nType = VEHICLE_TYPE_OTHER;
    // assert(false);
    SYSTEM_LOGGER_INFO("vechicle type error=%s", type.c_str());
  }
  return nType;
}

bool CVehicle::IsVehicleType(std::string name) {
  return (GetVehicleType(name) != CVehicle::VEHICLE_TYPE::VEHICLE_TYPE_OTHER);
}

CVehicleParam::CVehicleParam() {
  m_fStartShiftStart = 0;
  m_fStartShiftEnd = 0;
  m_fStartShiftSep = 0;

  m_fStartOffsetStart = 0;
  m_fStartOffsetEnd = 0;
  m_fStartOffsetSep = 0;

  m_fStartVelocityStart = 0;
  m_fStartVelocityEnd = 0;
  m_fStartVelocitySep = 0;
}

CVehicleParam::CVehicleParam(CVehicle& other) { (*this) = other; }

CVehicleParam& CVehicleParam::operator=(const CVehicle& v) {
  CVehicle::operator=(v);

  /*
  m_strID = v.m_strID;
  m_strRouteID = v.m_strRouteID;
  m_strLaneID = v.m_strLaneID;
  m_strStartShift = v.m_strStartShift;
  m_strStartTime = v.m_strStartTime;
  m_strStartVelocity = v.m_strStartVelocity;
  m_strMaxVelocity = v.m_strMaxVelocity;
  m_strOffset = v.m_strOffset;
  m_strLength = v.m_strLength;
  m_strWidth = v.m_strWidth;
  m_strHeight = v.m_strHeight;
  m_strType = v.m_strType;
  m_strAccID = v.m_strAccID;
  m_strMergeID = v.m_strMergeID;
  m_strAggress = v.m_strAggress;
  m_strBehavior = v.m_strBehavior;
  */

  return (*this);
}

uint64_t CVehicleParam::Generate(CVehicleParam::Vehicles& vehicles, bool onlyCalcualte) {
  int nShift = CUtils::is_zero(m_fStartShiftSep)
                   ? 0
                   : (m_fStartShiftEnd * 100 - m_fStartShiftStart * 100) / (m_fStartShiftSep * 100);
  int nOffset = CUtils::is_zero(m_fStartOffsetSep)
                    ? 0
                    : (m_fStartOffsetEnd * 100 - m_fStartOffsetStart * 100) / (m_fStartOffsetSep * 100);
  int nVelocity = CUtils::is_zero(m_fStartVelocitySep)
                      ? 0
                      : (m_fStartVelocityEnd * 100 - m_fStartVelocityStart * 100) / (m_fStartVelocitySep * 100);

  if (onlyCalcualte) {
    nShift++;
    nOffset++;
    nVelocity++;

    uint64_t llCount = nShift * nOffset * nVelocity;

    return llCount;
  }

  for (size_t i = 0; i <= nShift; i++) {
    float fShift = m_fStartShiftStart + i * m_fStartShiftSep;
    char strShiftBuf[256] = {0};
    sprintf(strShiftBuf, "%4f", fShift);

    for (size_t k = 0; k <= nOffset; k++) {
      float fOffset = m_fStartOffsetStart + k * m_fStartOffsetSep;
      char strOffsetBuf[256] = {0};
      sprintf(strOffsetBuf, "%4f", fOffset);
      for (size_t j = 0; j <= nVelocity; ++j) {
        float fVelocity = m_fStartVelocityStart + j * m_fStartVelocitySep;
        char strVelocityBuf[256] = {0};
        sprintf(strVelocityBuf, "%4f", fVelocity);

        CVehicle v(*this);
        v.m_strOffset = strOffsetBuf;
        v.m_strStartShift = strShiftBuf;
        v.m_strStartVelocity = strVelocityBuf;

        vehicles.push_back(v);
      }
    }
  }

  return 1;
}
