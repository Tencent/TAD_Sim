/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <json/value.h>
#include <map>
#include <mutex>
#include <string>
#include <thread>

class CHadmap;
namespace Json {
class Value;
}

// typedef int64_t uint64;

class CMapDataCache {
 public:
  typedef std::map<std::wstring, std::wstring> MapDataCaches;
  typedef std::map<std::wstring, Json::Value> MapDataSimpleStringCaches;

  typedef std::map<std::wstring, CHadmap*> HadmapCaches;

  static CMapDataCache& Instance();

  static const std::wstring m_wstrSuccess;  //= L"true";;
  static const std::wstring m_wstrFailed;   //= L"false";

  void Init();

  const wchar_t* Configuration();

  const wchar_t* L3StateMachine();

  const wchar_t* CloudHadmapUploadCallback(const wchar_t* wstrParams, std::wstring&);

  const wchar_t* GetForwardPoints(const wchar_t* wstrParams, std::wstring&);

  const wchar_t* GetLaneRel(const wchar_t* wstrParams, std::wstring& res);
  const wchar_t* GetRoadRel(const wchar_t* wstrParams, std::wstring& res);

  const wchar_t* LoadHadMap(const wchar_t* strMapName, const std::string& cosBucket = {},
                            const std::string& cosKey = {});

  const std::wstring LoadHadMap(const wchar_t* strMapName, bool fineRsp, const std::string& cosBucket = {},
                                const std::string& cosKey = {});

  const wchar_t* HadmapList();

  const wchar_t* HadmapinfoList();

  const wchar_t* DeleteHadmaps(const wchar_t* wstrParams);
  const wchar_t* DownloadHadmaps(const wchar_t* wstrParams);
  const wchar_t* UploadHadmaps(const wchar_t* wstrParams);

  const wchar_t* QueryNearbyInfo(const wchar_t* wstrQueryString);
  const wchar_t* QueryNearbyLane(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat);
  const wchar_t* QueryInfoByPt(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat);
  const wchar_t* QueryInfoByLocalPt(const wchar_t* wstrMapName, const wchar_t* wstrX, const wchar_t* wstrY,
                                    const wchar_t* wstrZ);
  const wchar_t* QueryNextLane(const wchar_t* strMapName, const wchar_t* wstrRoadID, const wchar_t* wstrSecID,
                               const wchar_t* wstrLaneID);
  const wchar_t* QueryLonLat(const wchar_t* wstrMapName, const wchar_t* wstrRoadID, const wchar_t* wstrSecID,
                             const wchar_t* wstrLaneID, const wchar_t* wstrShift, const wchar_t* wstrOffset);
  const wchar_t* QueryLonLatByPoint(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat,
                                    const wchar_t* wstrElemType, const wchar_t* wstrLaneID, const wchar_t* wstrShift,
                                    const wchar_t* wstrOffset, bool isStart = false);

  const std::wstring SearchPathImpl(const std::string& param);

  void RemoveHadmapCacheAll(const wchar_t* wstrMapName, bool lock = true);
  void RemoveHadmapCache(const wchar_t* wstrMapName, bool lock = true);

  CHadmap* HadmapCache(const wchar_t* wstrMapName);
  std::string HadmapEtagCache(const wchar_t* wstrMapName);
  void PushHadmapCache(const wchar_t* wstrMapName, CHadmap* pHadmap);

  void RemoveLaneRelDataCache(const wchar_t* strMapName);
  Json::Value LaneRelDataCache(const wchar_t* strMapName);
  Json::Value LanePrevRelDataCache(const wchar_t* strMapName);
  void PushLaneRelDataCache(const wchar_t* strMapName, Json::Value, Json::Value);

  void RemoveRoadRelDataCache(const wchar_t* strMapName);
  Json::Value RoadRelDataCache(const wchar_t* strMapName);
  Json::Value RoadPrevRelDataCache(const wchar_t* strMapName);
  void PushRoadRelDataCache(const wchar_t* strMapName, Json::Value, Json::Value);

  void RemoveRoadCache(const wchar_t* wstrMapName);
  const wchar_t* RoadDataCache(const wchar_t* wstrMapName);
  void PushRoadCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  void RemoveLaneCache(const wchar_t* wstrMapName);
  const wchar_t* LaneDataCache(const wchar_t* wstrMapName);
  void PushLaneCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  void RemoveLaneBoundaryCache(const wchar_t* wstrMapName);
  const wchar_t* LaneBoundaryDataCache(const wchar_t* wstrMapName);
  void PushLaneBoundaryCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  void RemoveLaneLinkCache(const wchar_t* wstrMapName);
  const wchar_t* LaneLinkDataCache(const wchar_t* wstrMapName);
  void PushLaneLinkCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  void RemoveObjectCache(const wchar_t* wstrMapName);
  const wchar_t* ObjectDataCache(const wchar_t* wstrMapName);
  void PushObjectCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  const wchar_t* WMapDataCache(const wchar_t* wstrMapName);
  void PushWDataCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  void RemoveTrafficLightCache(const wchar_t* wstrMapName);
  const wchar_t* TrafficLightDataCache(const wchar_t* wstrMapName);
  void PushTrafficLightCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  int CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir);
  int UpdateHadmapConfig(const char* strFullName, const char* strFileName);

 protected:
  CMapDataCache();

  int DeleteSingleHadmap(const char* strFileName);

  void ComposeCacheData(const wchar_t* wstrHadmapFile, CHadmap* pHadmap);
  std::wstring ConvertToJson(Json::Value& root);

  Json::Value buildHdserverDefaultRetStr();

 protected:
  HadmapCaches m_hadmapCaches;

  MapDataCaches m_mapWDataCache;

  MapDataCaches m_mapRoadCaches;
  MapDataCaches m_mapLaneCaches;
  MapDataCaches m_mapLaneBoundaryCaches;
  MapDataCaches m_mapLanelinkCaches;
  MapDataCaches m_mapObjCaches;
  MapDataCaches m_mapTrafficLightCaches;
  MapDataSimpleStringCaches m_laneRelM;
  MapDataSimpleStringCaches mRoadRelM;

  MapDataSimpleStringCaches m_lanePrevRelM;
  MapDataSimpleStringCaches mRoadPrevRelM;

  std::wstring m_mapLists;

  std::wstring m_wstrConvertToJsonSucceed;
  std::wstring m_wstrConvertToJsonFailed;
  std::wstring m_wstrMapFileFormatError;

  std::mutex m_mutexQueryNearByInfo;
  std::mutex m_mutexHadmapMutex;
  std::mutex m_mutexQueryNearBy;
  std::mutex m_mutexQueryNearbyPt;
  std::mutex m_mutexQueryNextLane;
  std::mutex m_mutexQueryLonLat;
  // make it thread safe
  std::map<std::string, std::thread> bg_threads;
};
