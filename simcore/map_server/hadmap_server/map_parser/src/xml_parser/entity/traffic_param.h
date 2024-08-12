/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "acceleration_param.h"
#include "common/xml_parser/entity/merge_param.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/entity/velocity_param.h"
#include "single_vehicle_info.h"

struct sTagBatchSceneParam;
struct sTagBatchSceneParamV2;

class CTrafficParam : public CTraffic {
 public:
  typedef std::map<std::string, CVehicleParam> VechileParamMap;
  typedef std::map<std::string, CTraffic> TrafficMap;

  typedef std::map<std::string, CAccelerationParam> AccelerationParamMap;
  typedef std::map<std::string, CVelocityParam> VelocityParamMap;
  typedef std::map<std::string, CMergeParam> MergeParamMap;

  typedef std::list<CTraffic> TrafficList;

  typedef std::list<CSingleVehicleInfo> SingleVehicleRelatedInfos;
  typedef std::list<SingleVehicleRelatedInfos> AllVehiclesRelatedInfos;
  typedef std::map<std::string, SingleVehicleRelatedInfos> VehiclesRelatedInfoMap;

 public:
  CTrafficParam();
  explicit CTrafficParam(CTraffic& traffic);
  CTrafficParam& operator=(CTraffic& traffic);

  void SetToDefault();
  virtual int Parse(const char* strFile);
  int ParseTemplate(const char* strFile);
  virtual int Save(const char* strFile);
  void GenerateAllSinglsFactors();
  void CalculateAllSinglesFactors(sTagBatchSceneParam& param, uint64_t& ullCount);
  void CalculateAllSinglesFactorsV2(sTagBatchSceneParamV2& param, uint64_t& ullCount);
  void Generate();
  void GenerateV2();
  int CalculateGeneratedTraffCount(int64_t& count, int64_t& size);
  VechileParamMap& VehicleParams() { return m_mapVehicleParams; }
  TrafficMap& GeneratedTraffics() { return m_mapTraffics; }
  TrafficList& GeneratedTrafficsV2() { return m_generatedAllTraffics; }

  void SetParam(sTagBatchSceneParam& param);
  void SetParamV2(sTagBatchSceneParamV2& param);

  virtual int GetOneAccID();
  virtual int GetOneMergeID();

 protected:
  int ParseVehicles(TiXmlElement* VehiclesElem, std::map<std::string, CVehicleParam>& mapVehicles);
  virtual int ParseOneVehicle(TiXmlElement* VehcileElem, CVehicleParam& v);

  int ComposeTraffic(AllVehiclesRelatedInfos& allVehilclesInfos, TrafficList& outTraffics);

 protected:
  std::string m_strFileName;
  VechileParamMap m_mapVehicleParams;
  AccelerationParamMap m_mapAccelerationParams;
  MergeParamMap m_mapMergeParams;
  VelocityParamMap m_mapVelocityParams;
  TrafficMap m_mapTraffics;

  TrafficList m_generatedAllTraffics;

  VehiclesRelatedInfoMap m_mapVehiclesRelatedInfo;
};
