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
const wchar_t* allocStr(std::wstring& str) {
  static std::wstring strEmpty = L"";

  int nLen = str.length();
  if (nLen > 0) {
    wchar_t* pRes = new wchar_t[nLen + 1];
    // wcscpy should copy terminate '\0'
    wcscpy(pRes, str.c_str());
    // pRes[nLen] = '\0';
    return pRes;
  }

  return L"";
}

extern "C" SCENE_WRAPPER_API int init(const wchar_t* wstrResourcePath, const wchar_t* wstrAppInstallDir) {
  std::string strResourcePath = CEngineConfig::Instance().WStr2MBStr(wstrResourcePath);
  std::string strAppInstallDir = CEngineConfig::Instance().WStr2MBStr(wstrAppInstallDir);
  xercesc::XMLPlatformUtils::Initialize();
  CLog::Initialize(strResourcePath.c_str());
  CEngineConfig::Instance().Init(strResourcePath.c_str(), strAppInstallDir.c_str());
  CMapDataCache::Instance().Init();
  SYSTEM_LOGGER_INFO("Load data cache finshed");
  CDatabase::Instance().Init();
  SYSTEM_LOGGER_INFO("Load data base finshed");
  CMapEdit::Instance().Init(strResourcePath.c_str());
  SYSTEM_LOGGER_INFO("Load mapEngine finshed");
  return 0;
}

extern "C" SCENE_WRAPPER_API int testInfo(const wchar_t* str) {
  char tmp[512] = {0};
  wcstombs(tmp, str, 511);
  SYSTEM_LOGGER_INFO("%s", tmp);
  return 0;
}

extern "C" SCENE_WRAPPER_API int deinit() {
  CLog::Finialize();
  return 0;
}

extern "C" SCENE_WRAPPER_API const wchar_t* getConfiguration() { return CMapDataCache::Instance().Configuration(); }

extern "C" SCENE_WRAPPER_API const wchar_t* getL3StateMachine() { return CMapDataCache::Instance().L3StateMachine(); }

extern "C" SCENE_WRAPPER_API const wchar_t* loadHadmap(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("load hadmap file name is null!");
    return L"false";
  }
  return CMapDataCache::Instance().LoadHadMap(wstrMapName);
}

SCENE_WRAPPER_API const wchar_t* getRoadData(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }

  std::string strRes = CMapDataCache::Instance().RoadDataCache(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());

  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getLaneData(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }

  std::string strRes = CMapDataCache::Instance().LaneDataCache(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());

  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getLaneBoundaryData(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }

  std::string strRes = CMapDataCache::Instance().LaneBoundaryDataCache(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());

  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getLaneLinkData(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }

  std::string strRes = CMapDataCache::Instance().LaneLinkDataCache(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());

  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getMapObjectData(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }

  std::string strRes = CMapDataCache::Instance().ObjectDataCache(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());

  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getTrafficlights(const wchar_t* wstrMapName) {
  if (!wstrMapName) {
    SYSTEM_LOGGER_ERROR("hadmap file name is null!");
    return L"";
  }
  std::string strRes = CMapDataCache::Instance().GetTrafficLights(wstrMapName);
  std::wstring wstrRes = CEngineConfig::Instance().MBStr2WStr(strRes.c_str());
  return allocStr(wstrRes);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryNearbyInfo(const wchar_t* wstrQueryString) {
  if (!wstrQueryString) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryNearbyInfo(wstrQueryString);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryNearbyLane(const wchar_t* strMapName, wchar_t* wstrLon,
                                                            wchar_t* wstrLat) {
  if (!strMapName || !wstrLon || !wstrLat) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryNearbyLane(strMapName, wstrLon, wstrLat);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryInfobyPt(const wchar_t* strMapName, wchar_t* wstrLon, wchar_t* wstrLat,
                                                          wchar_t* wstrSearchDistance /*, wchar_t * wstrAlt*/) {
  if (!strMapName || !wstrLon || !wstrLat || !wstrSearchDistance) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryInfoByPt(strMapName, wstrLon, wstrLat, wstrSearchDistance);
}

extern "C" SCENE_WRAPPER_API const wchar_t* batchQueryInfobyPt(const wchar_t* wstrMapName, const wchar_t* wstrParam) {
  if (!wstrMapName || !wstrParam) {
    SystemLogger::Error("parameter is null");
    return L"[]";
  }

  std::string param = CEngineConfig::Instance().WStr2MBStr(wstrParam);

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

      std::wstring wstrRes =
          CMapDataCache::Instance().QueryInfoByPt(wstrMapName, wstrLon.c_str(), wstrLat.c_str(), wstrDistance.c_str());
      std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
      result.append(StringToJson(strRes));
    }
  } catch (const std::exception& ex) {
  } catch (...) {
  }
  std::string data = JsonToString(result);
  std::wstring wdata = CEngineConfig::Instance().MBStr2WStr(data.c_str());
  return allocStr(wdata);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryInfobyLocalPt(const wchar_t* strMapName, wchar_t* wstrX,
                                                               wchar_t* wstrY, wchar_t* wstrZ) {
  if (!strMapName || !wstrX || !wstrY || !wstrZ) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryInfoByLocalPt(strMapName, wstrX, wstrY, wstrZ);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryNextLane(const wchar_t* strMapName, wchar_t* wstrRoadID,
                                                          wchar_t* wstrSecID, wchar_t* wstrLaneID) {
  if (!strMapName || !wstrRoadID || !wstrSecID || !wstrLaneID) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryNextLane(strMapName, wstrRoadID, wstrSecID, wstrLaneID);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryLonLat(const wchar_t* strMapName, wchar_t* wstrRoadID,
                                                        wchar_t* wstrSecID, wchar_t* wstrLaneID, wchar_t* wstrShift,
                                                        wchar_t* wstrOffset) {
  if (!strMapName || !wstrRoadID || !wstrSecID || !wstrLaneID || !wstrShift || !wstrOffset) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryLonLat(strMapName, wstrRoadID, wstrSecID, wstrLaneID, wstrShift, wstrOffset);
}

extern "C" SCENE_WRAPPER_API const wchar_t* queryLonLatByPoint(const wchar_t* strMapName, wchar_t* wstrLon,
                                                               wchar_t* wstrLat /*, wchar_t * wstrAlt*/,
                                                               wchar_t* wstrElemType, wchar_t* wstrElemID,
                                                               wchar_t* wstrShift, wchar_t* wstrOffset) {
  if (!strMapName || !wstrLon || !wstrLat || !wstrElemType || !wstrElemID || !wstrShift || !wstrOffset) {
    SYSTEM_LOGGER_ERROR("parameter is null");
    return L"";
  }

  return CMapDataCache::Instance().QueryLonLatByPoint(strMapName, wstrLon, wstrLat, wstrElemType, wstrElemID, wstrShift,
                                                      wstrOffset, true);
}

extern "C" SCENE_WRAPPER_API const wchar_t* batchQueryLonLatByPoint(const wchar_t* wstrMapName,
                                                                    const wchar_t* wstrParam) {
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
  if (!wstrMapName || !wstrParam) {
    SystemLogger::Error("parameter is null");
    return L"[]";
  }

  std::string param = CEngineConfig::Instance().WStr2MBStr(wstrParam);

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

      std::wstring wstrRes = CMapDataCache::Instance().QueryLonLatByPoint(wstrMapName, wstrLon.c_str(), wstrLat.c_str(),
                                                                          wstrElemType.c_str(), wstrElemID.c_str(),
                                                                          wstrShift.c_str(), wstrOffset.c_str(), true);
      std::string strRes = CEngineConfig::Instance().WStr2MBStr(wstrRes.c_str());
      result.append(StringToJson(strRes));
    }
  } catch (const std::exception& ex) {
  } catch (...) {
  }
  std::string data = JsonToString(result);
  std::wstring wdata = CEngineConfig::Instance().MBStr2WStr(data.c_str());
  return allocStr(wdata);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getMapList() { return CMapDataCache::Instance().HadmapList(); }

extern "C" SCENE_WRAPPER_API const wchar_t* getMapinfoList() { return CMapDataCache::Instance().HadmapinfoList(); }

extern "C" SCENE_WRAPPER_API const wchar_t* deleteHadmaps(const wchar_t* strParams) {
  return CMapDataCache::Instance().DeleteHadmaps(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* downloadHadmaps(const wchar_t* strParams) {
  return CMapDataCache::Instance().DownloadHadmaps(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* uploadHadmaps(const wchar_t* strParams) {
  return CMapDataCache::Instance().UploadHadmaps(strParams);
}
// 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapsProgress() {
  return CMapDataCache::Instance().GetHadmapsProgessVal();
}
// 终止进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopHadmapsProgress() {
  return CMapDataCache::Instance().StopHadmapsProgess();
}

extern "C" SCENE_WRAPPER_API const wchar_t* importSceneTemplate(const wchar_t* strTemplateName) {
  return CSceneData::Instance().GetTemplateData(strTemplateName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneData(const wchar_t* strSceneName) {
  return CSceneData::Instance().GetSceneData(strSceneName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneData(const wchar_t* strSceneName, const wchar_t* strSceneContent) {
  return CSceneData::Instance().SaveSceneData(strSceneName, strSceneContent);
}

extern "C" SCENE_WRAPPER_API const wchar_t* paramScene(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  // return CSceneData::Instance().ParamScene(wstrSceneName, wstrParam);
  return CSceneData::Instance().ParamSceneV2(wstrSceneName, wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* paramSceneCount(const wchar_t* wstrSceneName, const wchar_t* wstrParam) {
  // return CSceneData::Instance().ParamSceneCount(wstrSceneName, wstrParam);
  return CSceneData::Instance().ParamSceneCountV2(wstrSceneName, wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneDataV2(const wchar_t* wstrParam) {
  return CSceneData::Instance().GetSceneDataByParam(wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneDataV2(const wchar_t* wstrParam) {
  return CSceneData::Instance().SaveSceneDataByParam(wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveSceneDataV3(const char* strParam) {
  return CSceneData::Instance().SaveSceneDataByParam(strParam);
  return nullptr;
}

extern "C" SCENE_WRAPPER_API const wchar_t* loadGlobalSensor() {
  return CSceneData::Instance().LoadGlobalSensorConfig();
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveGlobalSensor(const wchar_t* wstrConfigContent) {
  return CSceneData::Instance().SaveGlobalSensorConfig(wstrConfigContent);
}

extern "C" SCENE_WRAPPER_API const wchar_t* loadGlobalEnvironment() {
  return CSceneData::Instance().LoadGlobalEnvironmentConfig();
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveGlobalEnvironment(const wchar_t* wstrConfigContent) {
  return CSceneData::Instance().SaveGlobalEnvironmentConfig(wstrConfigContent);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getCatalogList() { return CSceneData::Instance().GetCatalogList(); }

extern "C" SCENE_WRAPPER_API const wchar_t* addCatalog(const wchar_t* strVechileCatalog) {
  return CSceneData::Instance().AddCatalogList(strVechileCatalog);
}

extern "C" SCENE_WRAPPER_API const wchar_t* updateCatalog(const wchar_t* strVechileCatalog) {
  return CSceneData::Instance().UpdateCatalog(strVechileCatalog);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteCatalog(const wchar_t* strVechileCatalog) {
  return CSceneData::Instance().DeleteCatalog(strVechileCatalog);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorLoad() { return CSceneData::Instance().SensorLoad(); }

extern "C" SCENE_WRAPPER_API const wchar_t* sensorAdd(const wchar_t* strSensor) {
  return CSceneData::Instance().SensorAdd(strSensor);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorUpdate(const wchar_t* strSensor) {
  return CSceneData::Instance().SensorUpdate(strSensor);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorDelete(const wchar_t* strSensor) {
  return CSceneData::Instance().SensorDelete(strSensor);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorSave(const wchar_t* strSensorInfo) {
  return CSceneData::Instance().SensorSave(strSensorInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupLoad() { return CSceneData::Instance().SensorGroupLoad(); }

extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupAdd(const wchar_t* strAddInfo) {
  return CSceneData::Instance().SensorGroupSave(strAddInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sensorGroupDelete(const wchar_t* strDeleteInfo) {
  return CSceneData::Instance().SensorGroupDelete(strDeleteInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* dynamicSave(const wchar_t* strDynamicInfo) {
  return CSceneData::Instance().DynamicSave(strDynamicInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* dynamicDelete(const wchar_t* strDynamicInfo) {
  return CSceneData::Instance().DynamicDelete(strDynamicInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* dynamicLoad() { return CSceneData::Instance().DynamicLoad(); }

extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneList(const int offset, const int limit) {
  return CSceneData::Instance().GetHadmapSceneList(offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* searchHadmapScene(const wchar_t* strHadmapSceneName, const int offset,
                                                              const int limit) {
  return CSceneData::Instance().SearchHadmapScene(strHadmapSceneName, offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneData(const wchar_t* strHadmapSceneName) {
  return CHadmapSceneData::Instance().GetHadmapSceneDataV2(strHadmapSceneName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getHadmapSceneData3(const wchar_t* strHadmapSceneName) {
  return CHadmapSceneData::Instance().GetHadmapSceneDataV3(strHadmapSceneName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData(const wchar_t* strHadmapSceneName,
                                                                const wchar_t* strHadmapSceneContent) {
  return CHadmapSceneData::Instance().SaveHadmapSceneData(strHadmapSceneName, strHadmapSceneContent);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData2(const wchar_t* strHadmapSceneContent) {
  return CHadmapSceneData::Instance().SaveHadmapSceneDataV2(strHadmapSceneContent);
}

extern "C" SCENE_WRAPPER_API const wchar_t* saveHadmapSceneData3(const wchar_t* strHadmapSceneContent) {
  return CHadmapSceneData::Instance().SaveHadmapSceneDataV3(strHadmapSceneContent);
}
/********************     mapedit    start    ********************************/
extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateSession() { return CMapEdit::Instance().CreateSession(); }

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditReleaseSession(const wchar_t* strmapinfo) {
  return CMapEdit::Instance().ReleaseSession(strmapinfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditOpenHadmap(const wchar_t* strmapinfo) {
  return CMapEdit::Instance().OpenHadmap(strmapinfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateString(const wchar_t* strmapinfo) {
  const char* data = CMapEdit::Instance().CreateHadmapString(strmapinfo);
  int len = MultiByteToWideChar(CP_ACP, 0, data, strlen(data), NULL, 0);
  wchar_t* m_wchar = new wchar_t[len + 1];
  MultiByteToWideChar(CP_ACP, 0, data, strlen(data), m_wchar, len);
  m_wchar[len] = '\0';
  delete[] data;
  data = NULL;
  return m_wchar;
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditCreateMap(const wchar_t* strmapinfo) {
  return CMapEdit::Instance().CreateNewHadmap(strmapinfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditModifyMap(const wchar_t* strmapinfo) {
  return CMapEdit::Instance().ModifyHadmap(strmapinfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapEditSaveMap(const wchar_t* strmapinfo) {
  return CMapEdit::Instance().SaveHadmap(strmapinfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* modelUpload(const wchar_t* zippath, const wchar_t* strid) {
  return CSceneData::Instance().ModelUpload(zippath, strid);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapModelSave(const wchar_t* strModelInfo) {
  return CSceneData::Instance().MapModelSave(strModelInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* mapModelDelete(const wchar_t* strModelInfo) {
  return CSceneData::Instance().MapModelDelete(strModelInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getMapModelList() { return CSceneData::Instance().GetMapModelList(); }

extern "C" SCENE_WRAPPER_API const wchar_t* sceneModelSave(const wchar_t* strModelInfo) {
  return CSceneData::Instance().SceneModelSave(strModelInfo);
}

extern "C" SCENE_WRAPPER_API const wchar_t* sceneModelDelete(const wchar_t* strModelInfo) {
  return CSceneData::Instance().SceneModelDelete(strModelInfo);
}

/*******************   mapedit   end     ********************************/
extern "C" SCENE_WRAPPER_API const wchar_t* getGISImageList(const int offset, const int limit) {
  return CSceneData::Instance().GetGISImageList(offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* uploadGISImage(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().uploadGISImage(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteGISImage(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().deleteGISImage(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* renameGISImage(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().renameGISImage(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getGISModelList(const int offset, const int limit) {
  return CSceneData::Instance().GetGISModelList(offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* uploadGISModel(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().uploadGISModel(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteGISModel(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().deleteGISModel(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* renameGISModel(const wchar_t* wstrParams) {
  return CHadmapSceneData::Instance().renameGISModel(wstrParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneList(const int offset, const int limit) {
  return CSceneData::Instance().GetSceneList(offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneInfo(const wchar_t* strSceneName) {
  return CSceneData::Instance().GetSceneInfo(strSceneName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* searchScene(const wchar_t* strSceneName, const int offset,
                                                        const int limit) {
  return CSceneData::Instance().SearchScene(strSceneName, offset, limit);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteScene(const wchar_t* strSceneName) {
  return CSceneData::Instance().DeleteScene(strSceneName);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getScenesInfo(const wchar_t* strSceneNames) {
  return CSceneData::Instance().GetScenesInfo(strSceneNames);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteScenes(const wchar_t* strParams) {
  return CSceneData::Instance().DeleteScenes(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* downloadScenes(const wchar_t* strParams) {
  return CSceneData::Instance().DownloadScenesV2(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* uploadScenes(const wchar_t* strParams) {
  return CSceneData::Instance().UploadScenes(strParams);
}
// 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* getScenesProgress() { return CSceneData::Instance().GetScenesProgessVal(); }
// 终止进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopScenesProgress() { return CSceneData::Instance().StopScenesProgess(); }
// 导入导出进度
extern "C" SCENE_WRAPPER_API const wchar_t* getGenScenesProgress() {
  return CSceneData::Instance().GetGenScenesProgessVal();
}
// 终止进度
extern "C" SCENE_WRAPPER_API const wchar_t* stopGenScenesProgress() {
  return CSceneData::Instance().StopGenScenesProgess();
}

extern "C" SCENE_WRAPPER_API const wchar_t* copyScenes(const wchar_t* strParams) {
  return CSceneData::Instance().UploadScenes(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* renameScene(const wchar_t* strParams) {
  return CSceneData::Instance().RenameScenes(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* syncScenarioDBFromDisk() {
  return CSceneData::Instance().SyncScenarioDBFromDisk();
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneListV2(const wchar_t* strParams) {
  return CSceneData::Instance().GetSceneListByParam(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* searchSceneV2(const wchar_t* strParams) {
  return CSceneData::Instance().SearchSceneByParam(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getSceneInfoV2(const wchar_t* wstrID) {
  return CSceneData::Instance().GetSceneInfoByParam(wstrID);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getScenesInfoV2(const wchar_t* strParams) {
  return CSceneData::Instance().GetScenesInfoByParam(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* getScenarioSetList(const wchar_t* strParams) {
  return CScenarioSetData::Instance().GetScenarioSetList(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* createScenarioSet(const wchar_t* strParams) {
  return CScenarioSetData::Instance().CreateScenarioSet(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* updateScenarioSet(const wchar_t* strParams) {
  return CScenarioSetData::Instance().UpdateScenarioSet(strParams);
}

extern "C" SCENE_WRAPPER_API const wchar_t* deleteScenarioSet(const wchar_t* strParams) {
  return CScenarioSetData::Instance().DeleteScenarioSet(strParams);
}

extern "C" SCENE_WRAPPER_API int releaseStr(const wchar_t* strContent) {
  if (strContent && wcslen(strContent) > 0) {
    delete[] strContent;
    strContent = nullptr;
  }
  return 0;
  // return CSceneData::Instance().ReleaseStr(strContent);
}

extern "C" SCENE_WRAPPER_API int recordLog(const wchar_t* strContent) {
  std::string strLog = CEngineConfig::Instance().WStr2MBStr(strContent);
  SYSTEM_LOGGER_INFO("%s", strLog.c_str());
  return 0;
}

extern "C" SCENE_WRAPPER_API const bool json2xml(const char* xmlfilePath, const char* sceneName,
                                                 const char* jsoncontent, int xmlType) {
  if (!jsoncontent) {
    SYSTEM_LOGGER_INFO("jsoncontent is null");
    return false;
  }

  return true;
}

extern "C" SCENE_WRAPPER_API const char* xml2json(const char* xmlfilePath, const char* sceneName, int xmlType) {
  return "";
}

extern "C" SCENE_WRAPPER_API const wchar_t* startGenerateSemantic(const wchar_t* wstrParam) {
  return CSceneData::Instance().StartExe(wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* generateSceneSet() {
  return CSceneData::Instance().CreateCurrentSceneSet();
}

extern "C" SCENE_WRAPPER_API const wchar_t* insertMap(const wchar_t* wstrParam) {
  return CSceneData::Instance().InsertOneMap(wstrParam);
}

extern "C" SCENE_WRAPPER_API const wchar_t* insertScene(const wchar_t* wstrParam) {
  return CSceneData::Instance().InsertScenes(wstrParam);
}
