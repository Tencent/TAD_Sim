/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include "common/xml_parser/entity/SensorV2.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/xosc/xosc_base_1_0.h"
struct sTagSimuTraffic;
struct sTagBatchSceneParam;
struct sTagBatchSceneParamV2;
struct sTagVehicleParam;
struct sTagHadmapsDeleteParams;
struct sTagHadmapsDownloadParams;
struct sTagHadmapsUploadParams;
struct sTagScenesDeleteParams;
struct sTagScenesDownloadParams;
struct sTagScenesUploadParams;
struct sTagScenesCopyParams;
struct sTagSceneRenameParams;
struct sTagQueryNearbyInfoParams;

struct sTagEntityScenario;

struct sTagGetScenarioListParam;
struct sTagGetScenarioInfoListParam;
struct sTagSearchScenarioParam;
struct sTagGetScenarioSetListParam;
struct sTagCreateScenarioSetParam;
struct sTagUpdateScenarioSetParam;
struct sTagDeleteScenarioSetParam;

class CSensors;
class CTraffic;
class CEnvironment;

namespace Json {
class Value;
};

class CParserJson : public XOSCBase_1_0 {
 public:
  CParserJson();
  ~CParserJson();

  // std::string XmlToJson(const char*pPath, const char* pDir, const char*
  // pFileName);
  std::string XmlToJson(const char* pPath, const char* pDir, sTagEntityScenario& scenario);

  std::string XmlToTraffic(const char* pPath, const char* pDir, sTagEntityScenario& scenario, sTagSimuTraffic& sim);

  int JsonToXml(const char* strJson, sTagSimuTraffic& scene);
  int SaveToFile(const char* pPath, const char* strFileName, sTagSimuTraffic& scene, std::string& strPath);
  int SaveToDefaultXosc(sTagSimuTraffic& scene);
  int SaveToSim(const char* pPath, const char* strFileName, sTagSimuTraffic& scene, std::string& strPath);
  int SaveToXOSC(const char* pPath, const char* strFileName, sTagSimuTraffic& scene, std::string& strXoscPath);

  // scenario set
  int ParseScenarioSetListParams(const char* strJson, sTagGetScenarioSetListParam& param);
  int ParseScenarioSetCreateParams(const char* strJson, sTagCreateScenarioSetParam& param);
  int ParseScenarioSetUpdateParams(const char* strJson, sTagUpdateScenarioSetParam& param);
  int ParseScenarioSetDeleteParams(const char* strJson, sTagDeleteScenarioSetParam& param);

  // scenes
  int ParseScenarioGetParams(const char* strJson, sTagEntityScenario& param);
  int ParseScenarioCreateParams(const char* strJson, sTagEntityScenario& param);
  int ParseScenarioSearchParams(const char* strJson, sTagSearchScenarioParam& param);
  int ParseScenarioListParams(const char* strJson, sTagGetScenarioListParam& param);
  int ParseScenarioInfoListParams(const char* strJson, sTagGetScenarioInfoListParam& param);

  // batch scene
  int ParseSceneParam(const char* strJson, sTagBatchSceneParam& param);
  int ParseSceneParamV2(const char* strJson, sTagBatchSceneParamV2& param);
  // scenes
  int ParseScenesDeleteParams(const char* strJson, sTagScenesDeleteParams& param);
  int ParseScenesDownloadParams(const char* strJson, sTagScenesDownloadParams& param);
  int ParseScenesUploadParams(const char* strJson, sTagScenesUploadParams& param);
  int ParseScenesCopyParams(const char* strJson, sTagScenesCopyParams& param);
  int ParseSceneRenameParams(const char* strJson, sTagSceneRenameParams& param);
  int ParseHadmapsDeleteParams(const char* strJson, sTagHadmapsDeleteParams& param);
  int ParseHadmapsDownloadParams(const char* strJson, sTagHadmapsDownloadParams& param);
  int ParseHadmapsUploadParams(const char* strJson, sTagHadmapsUploadParams& param);

  int ParseQueryNearbyInfoParams(const char* strJson, sTagQueryNearbyInfoParams& param);
  int SaveGlobalSensorConfig(const char* strJson);
  std::string LoadGlobalSensorConfig();
  int SaveGlobalEnvironmentConfig(const char* strJson);
  std::string LoadGlobalEnvironmentConfig();
  //
  void ParseJsonToSim(const Json::Value& sim, CSimulation& simulation, CTraffic& traffic,std::string catalogdir);

  static std::string ToJsonString(const Json::Value& jRoot, bool bUseUnstyled = false);

  // sensor
  int ParseSensor(const char* strJson, SensorV2& sensorv2);
  int ParseSingleSensor(const char* strJson, SensorBaseV2& sensorv2);
  int ParseSingleSensor(const Json::Value& json, SensorBaseV2& sensorv2);
  int ParseDeleteInfo(const char* strJson, std::string& sensoridx);
  // trafficlights
  int InsertTrafficLights(std::string strJson, sTagSimuTraffic& scene);
  // gettrafficLights
 protected:
  int Pedestrians2Json(Json::Value& pedestrians, CTraffic* pTraffic);
  int PedestriansV22Json(Json::Value& pedestrians, CTraffic* pTraffic);

  int ParsePedestrians(Json::Value& jPedestrians, CTraffic& traffic);
  int ParsePedestriansV2(Json::Value& jPedestrians, CTraffic& traffic, const char* catalogDirPath);

  int ParseSceneParamEvents(Json::Value& jEvents, sTagBatchSceneParam& param);
  int ParseSceneParamOneEvent(Json::Value& jOneEvent, sTagBatchSceneParam& param);
  int ParseSceneParamEventsV2(Json::Value& jEvents, sTagVehicleParam& param);
  int ParseSceneParamOneEventV2(Json::Value& jOneEvent, sTagVehicleParam& param);

  bool JsonToXmlSensorsInfo(Json::Value& jSensors, CSensors& sensors);
  bool XmlToJsonSensorsInfo(CSensors& sensors, Json::Value& jSensors);

  bool JsonToXmlEnvironmentInfo(Json::Value& jSensors, CEnvironment& environment);
  bool XmlToJsonEnvironmentInfo(CEnvironment& environtment, Json::Value& jSensors);

  void AdjustData(sTagSimuTraffic& scene);

  void AdjustTrafficData(CTraffic& traffic);

  std::string Save(sTagSimuTraffic* pSceneData, sTagEntityScenario& scenario, const char* pFileName);
};
