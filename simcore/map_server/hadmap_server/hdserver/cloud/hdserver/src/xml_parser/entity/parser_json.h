/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <cstring>

#include "common/log/system_logger.h"
#include "common/utils/misc.h"

#include <string>

struct sTagSimuTraffic;
struct sTagBatchSceneParam;
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
struct sTagCloudHadmapUploadCallbackParam;
struct sTagGetForwardPointsParam;
struct sTagGetLaneRelParam;
struct sTagGetRoadRelParam;

class CSensors;
class CTraffic;
class CEnvironment;

namespace Json {
class Value;
};

class CParserJson {
 public:
  CParserJson();
  ~CParserJson();

  std::string XmlToJson(const char* pPath, const char* pDir, sTagEntityScenario& scenario);
  std::string XoscToSim(const char* pPath, const char* pDir, sTagEntityScenario& scenario);

  int JsonToXml(const char* strJson, sTagSimuTraffic& scene);
  int SaveToFile(const char* pPath, const char* strFileName, sTagSimuTraffic& scene, std::string& strPath);
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
  // scenes
  int ParseScenesDeleteParams(const char* strJson, sTagScenesDeleteParams& param);
  int ParseScenesDownloadParams(const char* strJson, sTagScenesDownloadParams& param);
  int ParseScenesUploadParams(const char* strJson, sTagScenesUploadParams& param);
  int ParseScenesCopyParams(const char* strJson, sTagScenesCopyParams& param);
  int ParseSceneRenameParams(const char* strJson, sTagSceneRenameParams& param);
  int ParseHadmapsDeleteParams(const char* strJson, sTagHadmapsDeleteParams& param);
  int ParseHadmapsDownloadParams(const char* strJson, sTagHadmapsDownloadParams& param);
  int ParseHadmapsUploadParams(const char* strJson, sTagHadmapsUploadParams& param);

  int ParseGetForwardPointsParam(const char* strJson, sTagGetForwardPointsParam& param);

  int ParseJsonToSim(const Json::Value& sim, CSimulation& simulation, CTraffic& traffic);
  // int ParseCloudHadmapUploadCallbackParam( const char* strJson,
  // sTagCloudHadmapUploadCallbackParam& param);
  int ParseCloudHadmapUploadCallbackParam(const char* strJson, sTagCloudHadmapUploadCallbackParam& param);

  template <typename T>
  int ParseGetRelParam(const char* strJson, T& param) {
    Json::CharReaderBuilder builder;
    builder["collectComments"] = false;
    JSONCPP_STRING errs;

    Json::CharReader* reader = builder.newCharReader();
    Json::Value root;
    int nLen = strlen(strJson);
    if (!reader->parse(strJson, strJson + nLen, &root, &errs)) {
      SYSTEM_LOGGER_ERROR("parse get lane rel callback param  json failed: ", strJson);
      return -1;
    }

    // files
    Json::Value& jFiles = root["hadmaps"];

    if (!jFiles.isNull()) {
      Json::ValueIterator fItr = jFiles.begin();
      for (; fItr != jFiles.end(); ++fItr) {
        std::string strFileName = (*fItr)["name"].asString();
        std::string strVersion = "";
        if (!(*fItr)["version"].isNull()) strVersion = (*fItr)["version"].asString();
        std::string strMapName = computeMapCompleteName(strFileName, strVersion);
        param.m_hadmapNames.push_back(strMapName);
      }
    } else {
      SYSTEM_LOGGER_ERROR(
          "The hadmap names is empty/no-exist fro "
          "getRel param.");
      return -1;
    }

    return 0;
  }

  int ParseGetLaneRelParam(const char* strJson, sTagGetLaneRelParam& param);
  // int ParseGetLaneRelParam(const char* strJson, sTagGetLaneRelParam&
  // param);
  int ParseGetRoadRelParam(const char* strJson, sTagGetRoadRelParam& param);
  int ParseQueryNearbyInfoParams(const char* strJson, sTagQueryNearbyInfoParams& param);

  int SaveGlobalSensorConfig(const char* strJson);
  std::string LoadGlobalSensorConfig();
  int SaveGlobalEnvironmentConfig(const char* strJson);
  std::string LoadGlobalEnvironmentConfig();

  static std::string ToJsonString(const Json::Value& jRoot, bool bUseUnstyled = false);
  // trafficlights
  int InsertTrafficLights(const std::string& strJson, sTagSimuTraffic& scene);

 protected:
  bool basicParser(const char* strJson, Json::Value*);

  int Pedestrians2Json(Json::Value& pedestrians, CTraffic* pTraffic);
  int PedestriansV22Json(Json::Value& pedestrians, CTraffic* pTraffic);

  int ParsePedestrians(Json::Value& jPedestrians, CTraffic& traffic);
  int ParsePedestriansV2(Json::Value& jPedestrians, CTraffic& traffic);

  int ParseSceneParamEvents(Json::Value& jEvents, sTagBatchSceneParam& param);
  int ParseSceneParamOneEvent(Json::Value& jOneEvent, sTagBatchSceneParam& param);

  bool JsonToXmlSensorsInfo(Json::Value& jSensors, CSensors& sensors);
  bool XmlToJsonSensorsInfo(CSensors& sensors, Json::Value& jSensors);

  bool JsonToXmlEnvironmentInfo(Json::Value& jSensors, CEnvironment& environment);
  bool XmlToJsonEnvironmentInfo(CEnvironment& environtment, Json::Value& jSensors);

  std::string Save(sTagSimuTraffic* pSceneData, sTagEntityScenario& scenario, const char* pFileName);
};
