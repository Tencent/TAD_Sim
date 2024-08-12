/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "scene_wrapper.h"
#include <json/value.h>
#include <json/writer.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <string>
#include <xercesc/util/PlatformUtils.hpp>
#include "../src/S3ClientWrap.h"
#include "../src/database/db.h"
#include "../src/engine/config.h"
#include "../src/map_data/lane_link.h"
#include "../src/server_hadmap/hadmap_scene_data.h"
#include "../src/server_map_cache/map_data_cache.h"
#include "../src/server_map_cache/map_syncer.h"
#include "../src/server_scene/scenario_set_data.h"
#include "../src/server_scene/scene_catalog_data.h"
#include "../src/server_scene/scene_data.h"
#include "common/drone_adaptor/drone_adaptor.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"
#include "common/map_data/map_object.h"
#include "common/recast_detour/recast_detour.h"
#include "common/utils/misc.h"
#include "common/utils/xsd_validator.h"
#include "common/xml_parser/entity/mapfile.h"
#include "param_adapter.h"

#ifdef UPGRADE_MAP_EDITOR
#  include "common/tx_od_log.h"
#endif

StringSlice* NewStringSlice(const char* data, size_t size) {
  StringSlice* p = new StringSlice();
  p->data = new char[size];
  memcpy(p->data, data, size);
  p->size = size;
  return p;
}

void DeleteStringSlice(StringSlice* p) {
  if (p->data) {
    delete[] p->data;
  }
  if (p) {
    delete p;
  }
}

const char* StringSliceData(StringSlice* p) { return p->data; }

size_t StringSliceSize(StringSlice* p) { return p->size; }

const char* allocStr(const std::string& str);

inline std::wstring composeWCompleteMapName(const std::string& version, const std::string& map) {
  std::string whole_name = computeMapCompleteName(map, version);
  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(whole_name.c_str());
  return wstrMapName;
}

extern "C" SCENE_WRAPPER_API int init(const char* strPath, const char* strAppInstallPath, int argc, char* argv[]) {
  google::AllowCommandLineReparsing();
  google::ParseCommandLineFlags(&argc, &argv, true);

  xercesc::XMLPlatformUtils::Initialize();

  CLog::Initialize(strPath);
  CEngineConfig::Instance().Init(strPath, strAppInstallPath);
  CMapDataCache::Instance().Init();
  CMapDataProxy::Instance().Init();
  MapSyncer::Create()->Start();

  CDatabase::Instance().Init();
  utils::XsdValidator::Instance().Initialize(std::string(strPath) + "/OpenX");
  utils::S3ClientWrap::Instance().Initialize();
  SceneCatalogData::Initialize();
#ifdef UPGRADE_MAP_EDITOR
  Base::Logger::Initialize(strPath);
#endif
  return 0;
}

extern "C" SCENE_WRAPPER_API int testInfo(const char* str) {
  // char tmp[128] = { 0 };
  // wcstombs(tmp, str, 8);
  // SystemLogger::Info(tmp);
  return 0;
}

extern "C" SCENE_WRAPPER_API int deinit() {
  CLog::Finialize();
  return 0;
}

extern "C" SCENE_WRAPPER_API const char* getConfiguration() {
  std::wstring wstrRes = CMapDataCache::Instance().Configuration();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getL3StateMachine() {
  std::wstring wstrRes = CMapDataCache::Instance().L3StateMachine();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* loadHadmap(const char* strMapName, const char* version, const char* fineRsp,
                                                    const char* cosBucket, const char* cosKey) {
  std::string mapName = adapter::HadmapOptionsAdapter(strMapName, cosKey);
  if (mapName.empty()) {
    SystemLogger::Error("load hadmap file name is null!");
    return "false";
  }

  auto conv_fine_rsp = [](const char* fine_rsp) {
    bool _fine_rsp = false;
    std::istringstream(fine_rsp) >> std::boolalpha >> _fine_rsp;
    return _fine_rsp || atoi(fine_rsp) == 1;
  };

  std::wstring wstrMapName = composeWCompleteMapName(version, mapName.c_str());
  std::wstring wstrRes =
      CMapDataCache::Instance().LoadHadMap(wstrMapName.c_str(), conv_fine_rsp(fineRsp), cosBucket, cosKey);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  SystemLogger::Info("scene wrapper load hadmap done ");
  return allocStr(strRes);

  // return strRes.c_str();
}

SCENE_WRAPPER_API const char* getEtag(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::string etag = CMapDataCache::Instance().HadmapEtagCache(wstrMapName.c_str());
  return allocStr(etag);
}

extern "C" SCENE_WRAPPER_API const char* getRoadData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().RoadDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());

  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().LaneDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneBoundaryData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().LaneBoundaryDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneLinkData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().LaneLinkDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CMapDataCache::Instance().LaneLinkDataCache(wstrMapName);
}

extern "C" SCENE_WRAPPER_API const char* getMapObjectData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().ObjectDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CMapDataCache::Instance().ObjectDataCache(wstrMapName);
}

extern "C" SCENE_WRAPPER_API const char* getTrafficLightData(const char* strMapName, const char* version) {
  std::string mapName = adapter::HadmapOptionsAdapter(computeMapCompleteName(strMapName, version));
  if (mapName.empty()) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(mapName.c_str());
  std::wstring wstrRes = CMapDataCache::Instance().TrafficLightDataCache(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryNearbyInfo(const char* strQueryString) {
  if (!strQueryString) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrQueryString = CEngineConfig::Instance().MBStr2WStr(strQueryString);
  std::wstring wstrRes = CMapDataCache::Instance().QueryNearbyInfo(wstrQueryString.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryNearbyLane(const char* strMapName, char* strLon, char* strLat,
                                                         const char* version) {
  if (!strMapName || !strLon || !strLat) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);
  std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon);
  std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat);
  std::wstring wstrRes =
      CMapDataCache::Instance().QueryNearbyLane(wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CMapDataCache::Instance().QeruyNearbyLane(strMapName, wstrLon,
  // wstrLat);
}

extern "C" SCENE_WRAPPER_API const char* queryInfobyPt(const char* strMapName, char* strLon, char* strLat,
                                                       const char* version) {
  if (!strMapName || !strLon || !strLat) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);
  std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon);
  std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat);

  std::wstring wstrRes = CMapDataCache::Instance().QueryInfoByPt(wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* batchQueryInfobyPt(const char* strMapName, const char* version,
                                                            const char* param) {
  if (!strMapName || !version || !param) {
    SystemLogger::Error("parameter is null");
    return "[]";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);

  Json::Value result(Json::Value(Json::arrayValue));
  try {
    Json::Value body = StringToJson(param);
    for (auto& point : body.get("points", Json::Value(Json::arrayValue))) {
      std::string strLon = point.get("startLon", "0.0").asString();
      std::string strLat = point.get("startLat", "0.0").asString();
      std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon.c_str());
      std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat.c_str());

      std::wstring wstrRes =
          CMapDataCache::Instance().QueryInfoByPt(wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str());
      std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
      result.append(StringToJson(strRes));
    }
  } catch (const std::exception& ex) {
  } catch (...) {
  }
  return allocStr(JsonToString(result));
}

extern "C" SCENE_WRAPPER_API const char* queryInfobyLocalPt(const char* strMapName, char* strX, char* strY,
                                                            char* strZ) {
  if (!strMapName || !strX || !strY || !strZ) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::wstring wstrX = CEngineConfig::Instance().MBStr2WStr(strX);
  std::wstring wstrY = CEngineConfig::Instance().MBStr2WStr(strY);
  std::wstring wstrZ = CEngineConfig::Instance().MBStr2WStr(strZ);

  std::wstring wstrRes =
      CMapDataCache::Instance().QueryInfoByLocalPt(wstrMapName.c_str(), wstrX.c_str(), wstrY.c_str(), wstrZ.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryNextLane(const char* strMapName, char* strRoadID, char* strSecID,
                                                       char* strLaneID, const char* version) {
  if (!strMapName || !strRoadID || !strSecID || !strLaneID) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);
  std::wstring wstrRoadID = CEngineConfig::Instance().MBStr2WStr(strRoadID);
  std::wstring wstrSecID = CEngineConfig::Instance().MBStr2WStr(strSecID);
  std::wstring wstrLaneID = CEngineConfig::Instance().MBStr2WStr(strLaneID);

  std::wstring wstrRes = CMapDataCache::Instance().QueryNextLane(wstrMapName.c_str(), wstrRoadID.c_str(),
                                                                 wstrSecID.c_str(), wstrLaneID.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryLonLat(const char* strMapName, char* strRoadID, char* strSecID,
                                                     char* strLaneID, char* strShift, char* strOffset,
                                                     const char* version) {
  if (!strMapName || !strRoadID || !strSecID || !strLaneID || !strShift || !strOffset) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);
  std::wstring wstrRoadID = CEngineConfig::Instance().MBStr2WStr(strRoadID);
  std::wstring wstrSecID = CEngineConfig::Instance().MBStr2WStr(strSecID);
  std::wstring wstrLaneID = CEngineConfig::Instance().MBStr2WStr(strLaneID);
  std::wstring wstrShift = CEngineConfig::Instance().MBStr2WStr(strShift);
  std::wstring wstrOffset = CEngineConfig::Instance().MBStr2WStr(strOffset);

  std::wstring wstrRes =
      CMapDataCache::Instance().QueryLonLat(wstrMapName.c_str(), wstrRoadID.c_str(), wstrSecID.c_str(),
                                            wstrLaneID.c_str(), wstrShift.c_str(), wstrOffset.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryLonLatByPoint(const char* strMapName, char* strLon, char* strLat,
                                                            char* strElemType, char* strElemID, char* strShift,
                                                            char* strOffset, const char* version) {
  if (!strMapName || !strLon || !strLat || !strElemID || !strElemID || !strShift || !strOffset) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);
  std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon);
  std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat);
  std::wstring wstrElemType = CEngineConfig::Instance().MBStr2WStr(strElemType);
  std::wstring wstrElemID = CEngineConfig::Instance().MBStr2WStr(strElemID);
  std::wstring wstrShift = CEngineConfig::Instance().MBStr2WStr(strShift);
  std::wstring wstrOffset = CEngineConfig::Instance().MBStr2WStr(strOffset);

  std::wstring wstrRes = CMapDataCache::Instance().QueryLonLatByPoint(
      wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str(), wstrElemType.c_str(), wstrElemID.c_str(),
      wstrShift.c_str(), wstrOffset.c_str(), true);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* batchQueryLonLatByPoint(const char* strMapName, const char* version,
                                                                 const char* param) {
  if (!strMapName || !version || !param) {
    SystemLogger::Error("parameter is null");
    return "[]";
  }

  std::wstring wstrMapName = composeWCompleteMapName(version, strMapName);

  Json::Value result(Json::Value(Json::arrayValue));
  try {
    Json::Value body = StringToJson(param);
    for (auto& point : body.get("points", Json::Value(Json::arrayValue))) {
      std::string strLon = point.get("startLon", "0.0").asString();
      std::string strLat = point.get("startLat", "0.0").asString();
      std::string strElemType = point.get("type", "").asString();
      std::string strElemID = point.get("id", "").asString();
      std::string strShift = point.get("shift", "").asString();
      std::string strOffset = point.get("offset", "").asString();
      std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon.c_str());
      std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat.c_str());
      std::wstring wstrElemType = CEngineConfig::Instance().MBStr2WStr(strElemType.c_str());
      std::wstring wstrElemID = CEngineConfig::Instance().MBStr2WStr(strElemID.c_str());
      std::wstring wstrShift = CEngineConfig::Instance().MBStr2WStr(strShift.c_str());
      std::wstring wstrOffset = CEngineConfig::Instance().MBStr2WStr(strOffset.c_str());

      std::wstring wstrRes = CMapDataCache::Instance().QueryLonLatByPoint(
          wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str(), wstrElemType.c_str(), wstrElemID.c_str(),
          wstrShift.c_str(), wstrOffset.c_str(), true);
      std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
      result.append(StringToJson(strRes));
    }
  } catch (const std::exception& ex) {
  } catch (...) {
  }
  return allocStr(JsonToString(result));
}

extern "C" SCENE_WRAPPER_API const char* getMapList() {
  std::wstring wstrRes = CMapDataCache::Instance().HadmapList();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getMapinfoList() {
  std::wstring wstrRes = CMapDataCache::Instance().HadmapinfoList();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
extern "C" SCENE_WRAPPER_API const char* deleteHadmaps(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CMapDataCache::Instance().DeleteHadmaps(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* downloadHadmaps(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CMapDataCache::Instance().DownloadHadmaps(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* uploadHadmaps(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CMapDataCache::Instance().UploadHadmaps(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* importSceneTemplate(const char* strTemplateName, const char* strMapName) {
  std::string tpl_name = strTemplateName;
  /// find tpl by map name
  std::string map_name = strMapName;
  if (tpl_name.empty() && !map_name.empty()) {
    auto* attr = CEngineConfig::Instance().HadmapAttr(map_name);
    if (attr) {
      tpl_name = attr->m_strTemplateName;
    }
  }
  if (tpl_name.empty()) {
    return allocStr(R"({"err":-1,"message":"Not default map or not found tpl"})");
  }

  std::wstring wstrTemplateName = CEngineConfig::Instance().MBStr2WStr(tpl_name.c_str());
  std::wstring wstrRes = CSceneData::Instance().GetTemplateData(wstrTemplateName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().GetSceneData(strSceneName);
}

extern "C" SCENE_WRAPPER_API const char* getSceneData(const char* strSceneName) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrRes = CSceneData::Instance().GetSceneData(wstrSceneName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().GetSceneData(strSceneName);
}

extern "C" SCENE_WRAPPER_API const char* saveSceneData(const char* strSceneName, const char* strSceneContent) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrSceneContent = CEngineConfig::Instance().MBStr2WStr(strSceneContent);
  std::wstring wstrRes = CSceneData::Instance().SaveSceneData(wstrSceneName.c_str(), wstrSceneContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().SaveSceneData(strSceneName,
  // strSceneContent);
}
extern "C" SCENE_WRAPPER_API const char* paramScene(const char* strSceneName, const char* strParam) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrSceneParam = CEngineConfig::Instance().MBStr2WStr(strParam);
  std::wstring wstrRes = CSceneData::Instance().ParamScene(wstrSceneName.c_str(), wstrSceneParam.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* paramSceneCount(const char* strSceneName, const char* strParam) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrSceneParam = CEngineConfig::Instance().MBStr2WStr(strParam);
  std::wstring wstrRes = CSceneData::Instance().ParamSceneCount(wstrSceneName.c_str(), wstrSceneParam.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getSceneDataV2(const char* strParam) {
  std::wstring wstrParam = CEngineConfig::Instance().MBStr2WStr(strParam);
  std::wstring wstrRes = CSceneData::Instance().GetSceneDataByParam(wstrParam.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* saveSceneDataV2(const char* strParam) {
  std::wstring wstrParam = CEngineConfig::Instance().MBStr2WStr(strParam);
  std::wstring wstrRes = CSceneData::Instance().SaveSceneDataByParam(wstrParam.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* loadGlobalSensor() {
  std::wstring wstrRes = CSceneData::Instance().LoadGlobalSensorConfig();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* saveGlobalSensor(const char* strConfigContent) {
  std::wstring wstrConfigContent = CEngineConfig::Instance().MBStr2WStr(strConfigContent);
  std::wstring wstrRes = CSceneData::Instance().SaveGlobalSensorConfig(wstrConfigContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
extern "C" SCENE_WRAPPER_API const char* loadGlobalEnvironment() {
  std::wstring wstrRes = CSceneData::Instance().LoadGlobalEnvironmentConfig();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* saveGlobalEnvironment(const char* strConfigContent) {
  std::wstring wstrConfigContent = CEngineConfig::Instance().MBStr2WStr(strConfigContent);
  std::wstring wstrRes = CSceneData::Instance().SaveGlobalEnvironmentConfig(wstrConfigContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getHadmapSceneList(const int offset, const int limit) {
  std::wstring wstrRes = CSceneData::Instance().GetHadmapSceneList(offset, limit);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* searchHadmapScene(const char* strHadmapSceneName, const int offset,
                                                           const int limit) {
  std::wstring wstrHadmapSceneName = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneName);
  std::wstring wstrRes = CSceneData::Instance().SearchHadmapScene(wstrHadmapSceneName.c_str(), offset, limit);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getHadmapSceneData(const char* strHadmapSceneName) {
  std::wstring wstrHadmapSceneName = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneName);
  std::wstring wstrRes = CHadmapSceneData::Instance().GetHadmapSceneDataV2(wstrHadmapSceneName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getHadmapSceneData3(const char* strHadmapSceneName, const char* version,
                                                             const char* bucket, const char* mapKey,
                                                             const char* jsonKey) {
  std::wstring wstrMapName = composeWCompleteMapName(version, strHadmapSceneName);
  std::string real_map_key = mapKey;
  std::string real_json_key = jsonKey;
  /// 适配交通仿真新架构，通过版本号生成COS路径
  if (real_map_key.empty()) {
    real_map_key = GenMapEditorCosKeyByVersion(strHadmapSceneName, version);
    real_json_key = GenMapEditorJsonKeyByVersion(strHadmapSceneName, version);
  }
  /////////////////////////////////////////
  std::string strRes =
      CHadmapSceneData::Instance().GetHadmapSceneDataV3(wstrMapName.c_str(), bucket, real_map_key, real_json_key);

  // std::string strRes = "";
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* saveHadmapSceneData(const char* strHadmapSceneName,
                                                             const char* strHadmapSceneContent) {
  std::wstring wstrHadmapSceneName = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneName);
  std::wstring wstrHadmapSceneContent = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneContent);
  std::wstring wstrRes =
      CHadmapSceneData::Instance().SaveHadmapSceneData(wstrHadmapSceneName.c_str(), wstrHadmapSceneContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
extern "C" SCENE_WRAPPER_API const char* saveHadmapSceneData2(const char* strHadmapSceneContent) {
  std::wstring wstrHadmapSceneContent = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneContent);
  std::wstring wstrRes = CHadmapSceneData::Instance().SaveHadmapSceneDataV2(wstrHadmapSceneContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* saveHadmapSceneData3(const char* strHadmapSceneContent) {
  std::string content = strHadmapSceneContent;
  /// 适配交通仿真新架构，通过版本号生成COS路径
  Json::Value root;
  misc::ParseJson(content.c_str(), &root, "");
  if (root["jsonKey"].isNull() && root["name"].isString() && root["version"].isString()) {
    root["jsonKey"] = GenMapEditorJsonKeyByVersion(root["name"].asString(), root["version"].asString());
    content = root.toStyledString();
  }
  /////////////////////////////////////////

  std::wstring wstrHadmapSceneContent = CEngineConfig::Instance().MBStr2WStr(content.c_str());
  std::string strRes = CHadmapSceneData::Instance().SaveHadmapSceneDataV3(wstrHadmapSceneContent.c_str());

  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getGISImageList(const int offset, const int limit) {
  std::wstring wstrRes = CSceneData::Instance().GetGISImageList(offset, limit);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* uploadGISImage(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CHadmapSceneData::Instance().uploadGISImage(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* deleteGISImage(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CHadmapSceneData::Instance().deleteGISImage(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* renameGISImage(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CHadmapSceneData::Instance().renameGISImage(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getSceneList(const int offset, const int limit) {
  std::wstring wstrRes = CSceneData::Instance().GetSceneList(offset, limit);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().GetSceneList(offset, limit);
}

extern "C" SCENE_WRAPPER_API const char* getSceneInfo(const char* strSceneName) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrRes = CSceneData::Instance().GetSceneInfo(wstrSceneName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().GetSceneInfo(strSceneName);
}

extern "C" SCENE_WRAPPER_API const char* searchScene(const char* strSceneName, const int offset, const int limit) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrRes = CSceneData::Instance().SearchScene(wstrSceneName.c_str(), offset, limit);
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().SearchScene(strSceneName, offset, limit);
}

extern "C" SCENE_WRAPPER_API const char* deleteScene(const char* strSceneName) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrRes = CSceneData::Instance().DeleteScene(wstrSceneName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().DeleteScene(strSceneName);
}

extern "C" SCENE_WRAPPER_API const char* getScenesInfo(const char* strSceneNames) {
  std::wstring wstrSceneNames = CEngineConfig::Instance().MBStr2WStr(strSceneNames);
  std::wstring wstrRes = CSceneData::Instance().GetScenesInfo(wstrSceneNames.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CSceneData::Instance().GetScenesInfo(strSceneNames);
}

extern "C" SCENE_WRAPPER_API const char* deleteScenes(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().DeleteScenes(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* downloadScenes(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().DownloadScenesV2(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* uploadScenes(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().UploadScenes(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* copyScenes(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().CopyScenes(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* renameScene(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().RenameScenes(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* syncScenarioDBFromDisk() {
  std::wstring wstrRes = CSceneData::Instance().SyncScenarioDBFromDisk();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneRel(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes;
  const wchar_t* status = CMapDataCache::Instance().GetLaneRel(wstrParams.c_str(), wstrRes);
  if (wcscmp(status, CMapDataCache::m_wstrSuccess.c_str()) != 0) {
    SystemLogger::Error("scene_wrapper: get lane rel failed.");
  }
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getRoadRel(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes;
  const wchar_t* status = CMapDataCache::Instance().GetRoadRel(wstrParams.c_str(), wstrRes);
  if (wcscmp(status, CMapDataCache::m_wstrSuccess.c_str()) != 0) {
    SystemLogger::Error("scene_wrapper: get road rel failed.");
  }
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* cloudHadmapUploadCallback(const char* strParams) {
  std::string real_param = adapter::HadmapOptionsAdapter(StringToJson(strParams));
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(real_param.c_str());
  std::wstring wstrRes;
  const wchar_t* status = CMapDataCache::Instance().CloudHadmapUploadCallback(wstrParams.c_str(), wstrRes);
  if (wcscmp(status, CMapDataCache::m_wstrSuccess.c_str()) != 0) {
    SystemLogger::Error("scene_wrapper: cloud hadmap callback failed.");
  }
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getForwardPoints(const char* strParams) {
  Json::Value root;
  if (misc::ParseJson(strParams, &root, "get forward points")) {
    return allocStr(R"({"code":-1,"message":"parse param failed, maybe invalid json"})");
  }
  root["hadmapName"] =
      computeMapCompleteName(root.get("hadmapName", "").asString(), root.get("version", "").asString());

  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(root.toStyledString().c_str());
  std::wstring wstrRes;
  const wchar_t* status = CMapDataCache::Instance().GetForwardPoints(wstrParams.c_str(), wstrRes);
  if (wcscmp(status, CMapDataCache::m_wstrSuccess.c_str()) != 0) {
    SystemLogger::Error("[scene_wrapper]scene_wrapper: get forward points failed.");
  }
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getSceneListV2(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().GetSceneListByParam(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* searchSceneV2(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().SearchSceneByParam(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getSceneInfoV2(const char* strID) {
  std::wstring wstrID = CEngineConfig::Instance().MBStr2WStr(strID);
  std::wstring wstrRes = CSceneData::Instance().GetSceneInfoByParam(wstrID.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getScenesInfoV2(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CSceneData::Instance().GetScenesInfoByParam(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getScenarioSetList(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CScenarioSetData::Instance().GetScenarioSetList(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* createScenarioSet(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CScenarioSetData::Instance().CreateScenarioSet(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* updateScenarioSet(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CScenarioSetData::Instance().UpdateScenarioSet(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* deleteScenarioSet(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CScenarioSetData::Instance().DeleteScenarioSet(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API int releaseStr(char* strContent) {
  if (strContent && strlen(strContent) > 0) {
    delete[] strContent;
    strContent = nullptr;
  }
  return 0;
  // return CSceneData::Instance().ReleaseStr(strContent);
}

const char* allocStr(const std::string& str) {
  static std::string strEmpty = "";

  int nLen = str.length();
  if (nLen > 0) {
    char* pRes = new char[nLen + 1];
    strncpy(pRes, str.c_str(), nLen);
    pRes[nLen] = '\0';
    return pRes;
  }

  return strEmpty.c_str();
}

extern "C" SCENE_WRAPPER_API int recordLog(const char* strContent) {
  SystemLogger::Info(strContent);
  return 0;
}

extern "C" SCENE_WRAPPER_API const char* OpenScenario(const char* param) {
  std::string real_param = adapter::ScenarioOptionsAdapter(param);

  auto w_param = CEngineConfig::Instance().MBStr2WStr(real_param.c_str());
  auto w_res = CSceneData::Instance().OpenScenarioImpl(w_param.c_str());
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* SaveScenario(const char* param) {
  std::string real_param = adapter::ScenarioOptionsAdapter(param);

  auto w_param = CEngineConfig::Instance().MBStr2WStr(real_param.c_str());
  auto w_res = CSceneData::Instance().SaveScenarioImpl(w_param.c_str());
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* Xosc2Sim(const char* param) {
  std::string real_param = adapter::ScenarioOptionsAdapter(param);

  auto w_param = CEngineConfig::Instance().MBStr2WStr(real_param.c_str());
  auto w_res = CSceneData::Instance().Xosc2SimImpl(w_param.c_str());
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* Xosc2Simrec(const char* param) {
  std::string real_param = adapter::ScenarioOptionsAdapter(param);

  auto w_param = CEngineConfig::Instance().MBStr2WStr(real_param.c_str());
  auto w_res = CSceneData::Instance().Xosc2SimrecImpl(w_param.c_str());
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* GetCatalog(const char* param) {
  auto w_res = CSceneData::Instance().GetCatalogImpl(param);
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* SetCatalog(const char* param) {
  auto w_res = CSceneData::Instance().SetCatalogImpl(param);
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* SearchPath(const char* param) {
  auto w_res = CMapDataCache::Instance().SearchPathImpl(param);
  auto res = CEngineConfig::Instance().WStr2MBStr(w_res.c_str());
  return allocStr(res);
}

extern "C" SCENE_WRAPPER_API const char* BuildNavMesh(const char* obj, const char* settings, const char* events,
                                                      const char* coskey) {
  bool build_ret = recast_detour::BuildNavMesh(obj, settings, events);
  if (!build_ret) {
    return "";
  }
  auto bin_path = boost::filesystem::path(obj).replace_extension(recast_detour::kNavMeshFileExt);

  if (coskey && strlen(coskey) > 0) {
    const auto& bucket = CEngineConfig::Instance().EnvVar().m_strDefaultBucket;
    auto remote_path = boost::filesystem::path(coskey).parent_path();
    // upload navMesh.bin

    bool ret =
        utils::S3ClientWrap::Instance().Upload(bucket, (remote_path / "navMesh.bin").string(), bin_path.string());
    std::string rsp = ret ? "success" : "error";
    // upload navMesh.gset
    auto gset_path = boost::filesystem::path(obj).replace_extension(recast_detour::kGeomSetFileExt);
    if (boost::filesystem::exists(gset_path)) {
      ret = utils::S3ClientWrap::Instance().Upload(bucket, (remote_path / "navMesh.gset").string(), gset_path.string());
      rsp += (ret ? ",success" : ",error");
    }
    return allocStr(rsp);
  }
  return allocStr(bin_path.string());
}

extern "C" {

SCENE_WRAPPER_API const StringSlice* TransDroneToSimrec(const char* drone, const char* capsule, int interval) {
  if (!drone || !capsule) {
    return nullptr;
  }

  std::string simrec = drone_adaptor::TransDroneToSimrec(drone, capsule, interval);
  return NewStringSlice(simrec.c_str(), simrec.size());
}
}
