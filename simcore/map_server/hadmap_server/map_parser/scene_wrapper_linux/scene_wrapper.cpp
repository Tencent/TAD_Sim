/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "scene_wrapper.h"
#include <json/value.h>
#include <json/writer.h>
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <string>
#include <xercesc/util/PlatformUtils.hpp>
#include "../src/database/db.h"
#include "../src/engine/config.h"
#include "../src/map_data/lane_link.h"
#include "../src/server_hadmap/hadmap_scene_data.h"
#include "../src/server_map_cache/map_data_cache.h"
#include "../src/server_scene/scenario_set_data.h"
#include "../src/server_scene/scene_data.h"
#include "../src/xml_parser/entity/CMapEdit.h"
#include "common/engine/entity/hadmap.h"
#include "common/log/system_logger.h"
#include "common/map_data/map_object.h"
#include "common/utils/misc.h"
#include "common/xml_parser/entity/mapfile.h"
const char* allocStr(const std::string& str);

extern "C" SCENE_WRAPPER_API int init(const char* strPath, const char* strAppInstallPath) {
  CLog::Initialize(strPath);
  CEngineConfig::Instance().Init(strPath, strAppInstallPath);
  CMapDataCache::Instance().Init();
  xercesc::XMLPlatformUtils::Initialize();
  CDatabase::Instance().Init();
  CMapEdit::Instance().Init(strPath);
  return 0;
}

extern "C" SCENE_WRAPPER_API int testInfo(const char* str) { return 0; }

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

extern "C" SCENE_WRAPPER_API const char* loadHadmap(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("load hadmap file name is null!");
    return "false";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::wstring wstrRes = CMapDataCache::Instance().LoadHadMap(wstrMapName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return strRes.c_str();
}

extern "C" SCENE_WRAPPER_API const char* getRoadData(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().RoadDataCache(wstrMapName.c_str());
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());

  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneData(const char* strMapName) {
  SystemLogger::Info("getLaneData %s", strMapName);
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().LaneDataCache(wstrMapName.c_str());

  SystemLogger::Info("getLaneData %s done", strMapName);
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getTrafficlights(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().GetTrafficLights(wstrMapName.c_str());
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneBoundaryData(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().LaneBoundaryDataCache(wstrMapName.c_str());
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getLaneLinkData(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().LaneLinkDataCache(wstrMapName.c_str());
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CMapDataCache::Instance().LaneLinkDataCache(wstrMapName);
}

extern "C" SCENE_WRAPPER_API const char* getMapObjectData(const char* strMapName) {
  if (!strMapName) {
    SystemLogger::Error("hadmap file name is null!");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::string strRes = CMapDataCache::Instance().ObjectDataCache(wstrMapName.c_str());
  // std::string strRes =
  // CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
  // return CMapDataCache::Instance().ObjectDataCache(wstrMapName);
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

extern "C" SCENE_WRAPPER_API const char* queryNearbyLane(const char* strMapName, char* strLon, char* strLat) {
  if (!strMapName || !strLon || !strLat) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
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
                                                       char* strSearchDistance) {
  if (!strMapName || !strLon || !strLat) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon);
  std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat);
  std::wstring wstrDistance = CEngineConfig::Instance().MBStr2WStr(strSearchDistance);

  std::wstring wstrRes = CMapDataCache::Instance().QueryInfoByPt(wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str(),
                                                                 wstrDistance.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
extern "C" SCENE_WRAPPER_API const char* batchQueryInfobyPt(const char* strMapName, const char* param) {
  if (!strMapName || !param) {
    SystemLogger::Error("parameter is null");
    return "[]";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);

  Json::Value result(Json::arrayValue);
  try {
    Json::Value body = StringToJson(param);
    for (auto& point : body.get("points", Json::Value(Json::arrayValue))) {
      std::string strLon = point.get("startLon", "0.0").asString();
      std::string strLat = point.get("startLat", "0.0").asString();
      std::string strSearchDistance = point.get("searchDistance", "5.0").asString();
      std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon.c_str());
      std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat.c_str());
      std::wstring wstrDistance = CEngineConfig::Instance().MBStr2WStr(strSearchDistance.c_str());

      std::wstring wstrRes = CMapDataCache::Instance().QueryInfoByPt(wstrMapName.c_str(), wstrLon.c_str(),
                                                                     wstrLat.c_str(), wstrDistance.c_str());
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
                                                       char* strLaneID) {
  if (!strMapName || !strRoadID || !strSecID || !strLaneID) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::wstring wstrRoadID = CEngineConfig::Instance().MBStr2WStr(strRoadID);
  std::wstring wstrSecID = CEngineConfig::Instance().MBStr2WStr(strSecID);
  std::wstring wstrLaneID = CEngineConfig::Instance().MBStr2WStr(strLaneID);

  std::wstring wstrRes = CMapDataCache::Instance().QueryNextLane(wstrMapName.c_str(), wstrRoadID.c_str(),
                                                                 wstrSecID.c_str(), wstrLaneID.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* queryLonLat(const char* strMapName, char* strRoadID, char* strSecID,
                                                     char* strLaneID, char* strShift, char* strOffset) {
  if (!strMapName || !strRoadID || !strSecID || !strLaneID || !strShift || !strOffset) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
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
                                                            char* strOffset) {
  if (!strMapName || !strLon || !strLat || !strElemID || !strElemID || !strShift || !strOffset) {
    SystemLogger::Error("parameter is null");
    return "";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);
  std::wstring wstrLon = CEngineConfig::Instance().MBStr2WStr(strLon);
  std::wstring wstrLat = CEngineConfig::Instance().MBStr2WStr(strLat);
  std::wstring wstrElemType = CEngineConfig::Instance().MBStr2WStr(strElemType);
  std::wstring wstrElemID = CEngineConfig::Instance().MBStr2WStr(strElemID);
  std::wstring wstrShift = CEngineConfig::Instance().MBStr2WStr(strShift);
  std::wstring wstrOffset = CEngineConfig::Instance().MBStr2WStr(strOffset);

  std::wstring wstrRes = CMapDataCache::Instance().QueryLonLatByPoint(
      wstrMapName.c_str(), wstrLon.c_str(), wstrLat.c_str(), wstrElemType.c_str(), wstrElemID.c_str(),
      wstrShift.c_str(), wstrOffset.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* batchQueryLonLatByPoint(const char* strMapName, const char* param) {
  /*
  curl -X GET 'http://127.0.0.1:9000/hadmapdata?cmd=loadhadmap&mapname=d2d_20190726.xodr' -v -w'\n'
  curl -X POST -H'Content-Type: application/json' 'http://127.0.0.1:9000/hadmapdata'
  -d'{"cmd":"batchquerylonlatbypoint","mapname":"d2d_20190726.xodr","points":[{"startLon":"121.17650158692382",
  "startLat":"31.279939661607216","type":"lane","id":"-1","shift":"450.000","offset":"0.000"}]}
  ' -v -w'\n'
  {
    "cmd": "batchquerylonlatbypoint",
    "mapname": "d2d_20190726.xodr",
    "points": [
        {
            "startLon": "121.174911",
            "startLat": "31.269941",
            "type": "lane",
            "id": "-1",
            "shift": "0",
            "offset": "-3.447691"
        }
    ],
  }
  */
  if (!strMapName || !param) {
    SystemLogger::Error("parameter is null");
    return "[]";
  }

  std::wstring wstrMapName = CEngineConfig::Instance().MBStr2WStr(strMapName);

  Json::Value result(Json::arrayValue);
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
// 导入导出进度
extern "C" SCENE_WRAPPER_API const char* getHadmapsProgress() {
  std::wstring wstrRes = CMapDataCache::Instance().GetHadmapsProgessVal();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
// 终止进度
extern "C" SCENE_WRAPPER_API const char* stopHadmapsProgress() {
  std::wstring wstrRes = CMapDataCache::Instance().StopHadmapsProgess();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* uploadHadmaps(const char* strParams) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strParams);
  std::wstring wstrRes = CMapDataCache::Instance().UploadHadmaps(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* importSceneTemplate(const char* strTemplateName) {
  std::wstring wstrTemplateName = CEngineConfig::Instance().MBStr2WStr(strTemplateName);
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
  std::wstring wstrRes = CSceneData::Instance().ParamSceneV2(wstrSceneName.c_str(), wstrSceneParam.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* paramSceneCount(const char* strSceneName, const char* strParam) {
  std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrSceneParam = CEngineConfig::Instance().MBStr2WStr(strParam);
  std::wstring wstrRes = CSceneData::Instance().ParamSceneCountV2(wstrSceneName.c_str(), wstrSceneParam.c_str());
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

extern "C" SCENE_WRAPPER_API const char* getHadmapSceneData3(const char* strHadmapSceneName) {
  std::wstring wstrHadmapSceneName = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneName);
  std::wstring wstrRes = CHadmapSceneData::Instance().GetHadmapSceneDataV3(wstrHadmapSceneName.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
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
  std::wstring wstrHadmapSceneContent = CEngineConfig::Instance().MBStr2WStr(strHadmapSceneContent);
  std::wstring wstrRes = CHadmapSceneData::Instance().SaveHadmapSceneDataV3(wstrHadmapSceneContent.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
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

/********************     mapedit    start    ********************************/
extern "C" SCENE_WRAPPER_API const char* mapEditCreateSession() {
  // std::wstring wstrSceneName = CEngineConfig::Instance().MBStr2WStr(strSceneName);
  std::wstring wstrRes = CMapEdit::Instance().CreateSession();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapEditReleaseSession(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  std::wstring wstrRes = CMapEdit::Instance().ReleaseSession(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapEditOpenHadmap(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  std::wstring wstrRes = CMapEdit::Instance().OpenHadmap(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapEditCreateString(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  const char* data = CMapEdit::Instance().CreateHadmapString(wstrInput.c_str());
  return data;
}

extern "C" SCENE_WRAPPER_API const char* mapEditCreateMap(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  std::wstring wstrRes = CMapEdit::Instance().CreateNewHadmap(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapEditModifyMap(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  std::wstring wstrRes = CMapEdit::Instance().ModifyHadmap(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapEditSaveMap(const char* strmapinfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strmapinfo);
  std::wstring wstrRes = CMapEdit::Instance().SaveHadmap(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* modelUpload(const char* zippath, const char* strid) {
  std::wstring wstrInput_zippath = CEngineConfig::Instance().MBStr2WStr(zippath);
  std::wstring wstrInput_strid = CEngineConfig::Instance().MBStr2WStr(strid);
  std::wstring wstrRes = CSceneData::Instance().ModelUpload(wstrInput_zippath.c_str(), wstrInput_strid.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapModelSave(const char* strModelInfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strModelInfo);
  std::wstring wstrRes = CSceneData::Instance().MapModelSave(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* mapModelDelete(const char* strModelInfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strModelInfo);
  std::wstring wstrRes = CSceneData::Instance().MapModelDelete(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* getMapModelList() {
  std::wstring wstrRes = CSceneData::Instance().GetMapModelList();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sceneModelSave(const char* SceneModelSave) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(SceneModelSave);
  std::wstring wstrRes = CSceneData::Instance().SceneModelSave(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sceneModelDelete(const char* strModelInfo) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(strModelInfo);
  std::wstring wstrRes = CSceneData::Instance().SceneModelDelete(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

/*******************   mapedit   end     ********************************/

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
// 导入导出进度
extern "C" SCENE_WRAPPER_API const char* getScenesProgress() {
  std::wstring wstrRes = CSceneData::Instance().GetScenesProgessVal();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
// 终止进度
extern "C" SCENE_WRAPPER_API const char* stopScenesProgress() {
  std::wstring wstrRes = CSceneData::Instance().StopScenesProgess();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
// 导入导出进度
extern "C" SCENE_WRAPPER_API const char* getGenScenesProgress() {
  std::wstring wstrRes = CSceneData::Instance().GetGenScenesProgessVal();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}
// 终止进度
extern "C" SCENE_WRAPPER_API const char* stopGenScenesProgress() {
  std::wstring wstrRes = CSceneData::Instance().StopGenScenesProgess();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
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

extern "C" SCENE_WRAPPER_API const char* getCatalogList() {
  std::wstring wstrRes = CSceneData::Instance().GetCatalogList();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* addCatalog(const char* strVechileCatalog) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strVechileCatalog);
  std::wstring wstrRes = CSceneData::Instance().AddCatalogList(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* updateCatalog(const char* strVechileCatalog) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strVechileCatalog);
  std::wstring wstrRes = CSceneData::Instance().UpdateCatalog(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* deleteCatalog(const char* strVechileCatalog) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strVechileCatalog);
  std::wstring wstrRes = CSceneData::Instance().DeleteCatalog(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorLoad() {
  std::wstring wstrRes = CSceneData::Instance().SensorLoad();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorAdd(const char* strSensor) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensor);
  std::wstring wstrRes = CSceneData::Instance().SensorAdd(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorUpdate(const char* strSensor) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensor);
  std::wstring wstrRes = CSceneData::Instance().SensorUpdate(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorDelete(const char* strSensor) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensor);
  std::wstring wstrRes = CSceneData::Instance().SensorDelete(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorSave(const char* strSensorInfo) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensorInfo);
  std::wstring wstrRes = CSceneData::Instance().SensorSave(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorGroupLoad() {
  std::wstring wstrRes = CSceneData::Instance().SensorGroupLoad();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorGroupAdd(const char* strSensorInfo) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensorInfo);
  std::wstring wstrRes = CSceneData::Instance().SensorGroupSave(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* sensorGroupDelete(const char* strSensorInfo) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strSensorInfo);
  std::wstring wstrRes = CSceneData::Instance().SensorGroupDelete(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* dynamicSave(const char* strDynamicInfo) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strDynamicInfo);
  std::wstring wstrRes = CSceneData::Instance().DynamicSave(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* dynamicDelete(const char* strDynamicInfo) {
  std::wstring wstrParams = CEngineConfig::Instance().MBStr2WStr(strDynamicInfo);
  std::wstring wstrRes = CSceneData::Instance().DynamicDelete(wstrParams.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* dynamicLoad() {
  std::wstring wstrRes = CSceneData::Instance().DynamicLoad();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* startGenerateSemantic(const char* wstrParam) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(wstrParam);
  std::wstring wstrRes = CSceneData::Instance().StartExe(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* generateSceneSet() {
  std::wstring wstrRes = CSceneData::Instance().CreateCurrentSceneSet();
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* insertMap(const char* wstrParam) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(wstrParam);
  std::wstring wstrRes = CSceneData::Instance().InsertOneMap(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

extern "C" SCENE_WRAPPER_API const char* insertScene(const char* wstrParam) {
  std::wstring wstrInput = CEngineConfig::Instance().MBStr2WStr(wstrParam);
  std::wstring wstrRes = CSceneData::Instance().InsertScenes(wstrInput.c_str());
  std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
  return allocStr(strRes);
}

const char* allocStr(const std::string& str) {
  static std::string strEmpty = "";

  int nLen = str.length();
  if (nLen > 0) {
    char* pRes = new char[nLen + 1];
    strcpy(pRes, str.c_str());
    pRes[nLen] = '\0';
    return pRes;
  }

  return strEmpty.c_str();
}

extern "C" SCENE_WRAPPER_API int recordLog(const char* strContent) {
  SystemLogger::Info(strContent);
  return 0;
}
