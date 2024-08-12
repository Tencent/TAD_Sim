/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include <list>
#include <string>
#include "catalogbase.h"
#include "common/map_data/map_data_predef.h"

class CVehicleParam;
class CVehicle {
 public:
  enum VEHICLE_TYPE {
    VEHICLE_TYPE_SEDAN = 0,
    VEHICLE_TYPE_SUV,
    VEHICLE_TYPE_BUS,
    VEHICLE_TYPE_TRUNK,
    VEHICLE_TYPE_AMBULANCE,

    VEHICLE_TYPE_TRUNK_1,
    VEHICLE_TYPE_SEDAN_001,
    VEHICLE_TYPE_SEDAN_002,
    VEHICLE_TYPE_SEDAN_003,
    VEHICLE_TYPE_SEDAN_004,
    VEHICLE_TYPE_SUV_001,
    VEHICLE_TYPE_SUV_002,
    VEHICLE_TYPE_SUV_003,
    VEHICLE_TYPE_SUV_004,
    VEHICLE_TYPE_SUV_005,
    VEHICLE_TYPE_TRUCK_002,
    VEHICLE_TYPE_TRUCK_003,
    VEHICLE_TYPE_BUS_001,
    VEHICLE_TYPE_SEMI_TRAILER_TRUCK_001,
    VEHICLE_TYPE_AMBULANCE_001,
    VEHICLE_TYPE_TRUCK_004,
    VEHICLE_TYPE_SEDAN_005,
    VEHICLE_TYPE_AdminVehicle_001,
    VEHICLE_TYPE_AdminVehicle_002,
    VEHICLE_TYPE_BUS_005,
    VEHICLE_TYPE_BUS_004,
    VEHICLE_TYPE_BUS_003,
    VEHICLE_TYPE_BUS_002,
    VEHICLE_TYPE_TRICLE_002,

    VEHICLE_TYPE_AIV_FullLoad_001 = 503,
    VEHICLE_TYPE_AIV_Empty_001 = 504,
    VEHICLE_TYPE_MPV_001 = 505,
    VEHICLE_TYPE_AIV_FullLoad_002 = 506,
    VEHICLE_TYPE_AIV_Empty_002 = 507,
    VEHICLE_TYPE_MIFA_01 = 508,
    VEHICLE_TYPE_Truck_with_cargobox = 509,
    VEHICLE_TYPE_Truck_without_cargobox = 510,
    VEHICLE_TYPE_MPV_002 = 511,
    VEHICLE_TYPE_MiningDumpTruck_001 = 512,
    VEHICLE_TYPE_AIV_V5_FullLoad_001 = 513,
    VEHICLE_TYPE_AIV_V5_Empty_001 = 514,
    VEHICLE_TYPE_LandRover_RangeRover_2010,
    VEHICLE_TYPE_BMW_MINI_CooperS_2010,
    VEHICLE_TYPE_Opel_Corsa_2009,
    VEHICLE_TYPE_Lexus_IS_2009,
    VEHICLE_TYPE_Suzuki_Celerio_2011,
    VEHICLE_TYPE_Lancia_DeltaIntegrale_Evo1,
    VEHICLE_TYPE_Chevrolet_ExpressVan1500_2001,
    VEHICLE_TYPE_MercedesBenz_SL_2005,
    VEHICLE_TYPE_Dodge_Charger_2006,
    VEHICLE_TYPE_Porsche_911_TurboS_2020,
    VEHICLE_TYPE_Nissan_GTR_2009,
    VEHICLE_TYPE_Opel_Ampera_2009,
    VEHICLE_TYPE_Dodge_ViperMambaEdition_RCH680,
    VEHICLE_TYPE_Mitsubishi_iMiEV_2018,
    VEHICLE_TYPE_Audi_A8_2015,
    VEHICLE_TYPE_Toyota_Tacoma_2017,
    VEHICLE_TYPE_Lamborghini_Gallardo_2011,
    VEHICLE_TYPE_MercedesBenz_SLSAMG_2014,
    VEHICLE_TYPE_Scania_R620_2012,
    VEHICLE_TYPE_MAN_LionsCoach_2003,
    VEHICLE_TYPE_Cadillac_XTSLimousine_2018,
    VEHICLE_TYPE_MercedesBenz_Arocs_3240,
    VEHICLE_TYPE_BMW_X6_2014,
    VEHICLE_TYPE_Truck_004,
    VEHICLE_TYPE_Half_loaded_Trailer,
    VEHICLE_TYPE_Full_loaded_Trailer,
    VEHICLE_TYPE_Unloaded_Trailer,
    VEHICLE_TYPE_OTHER = 999,
  };

  enum {
    VEHICLE_BEHAVIOR_TRAFFIC = 0,
    VEHICLE_BEHAVIOR_USERDEFINE,
    VEHICLE_BEHAVIOR_TRAFFICARTERIAL,
    VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW,
  };

  CVehicle();
  explicit CVehicle(CVehicleParam vp);
  void Reset();
  void Copy(CVehicle* pV);
  void ConvertToValue();
  void ConvertToStr();

  static VEHICLE_TYPE GetVehicleType(std::string type);
  static bool IsVehicleType(std::string name);

 public:
  std::string m_strName;
  std::string m_strID;
  std::string m_strRouteID;
  std::string m_strLaneID;
  std::string m_strStartShift;
  std::string m_strStartTime;
  std::string m_strStartVelocity;
  std::string m_strMaxVelocity;
  std::string m_strOffset;

  std::string m_strType;
  std::string m_strAccID;
  std::string m_strMergeID;
  std::string m_strBehavior;
  std::string m_strAggress;
  std::string m_strFollow;
  std::string m_strEventId;
  std::string m_strAngle;
  std::string m_strSensorGroup;
  std::string m_strObuStauts;
  std::string m_strStartAngle;
  CBoundingBox m_boundingBox;
  // std::string m_strLength;
  // std::string m_strWidth;
  // std::string m_strHeight;

  uint64_t m_ID;
  uint64_t m_routeID;
  lanepkid m_laneID;
  uint64_t m_accID;
  uint64_t m_mergeID;
  int m_type;
  int m_behaviorType;
  float m_fStartShift;
  float m_fStartVelocity;
  float m_fMaxVelocity;
  float m_fStartTime;
  float m_fStartOffset;
  // float m_fVehicleLength;
  // float m_fVehicleWidth;
  // float m_fVehicleHeight;
  double m_dAggress;
};

class CVehicleParam : public CVehicle {
 public:
  typedef std::list<CVehicle> Vehicles;

  CVehicleParam();
  explicit CVehicleParam(CVehicle& other);
  CVehicleParam& operator=(const CVehicle& other);

  float m_fStartShiftStart;
  float m_fStartShiftEnd;
  float m_fStartShiftSep;

  float m_fStartOffsetStart;
  float m_fStartOffsetEnd;
  float m_fStartOffsetSep;

  float m_fStartVelocityStart;
  float m_fStartVelocityEnd;
  float m_fStartVelocitySep;

  Vehicles& GeneratedVehicles() { return m_GeneratedVehicles; }

  uint64_t Generate(Vehicles& vehicles, bool onlyCalculate = false);

  Vehicles m_GeneratedVehicles;
};
