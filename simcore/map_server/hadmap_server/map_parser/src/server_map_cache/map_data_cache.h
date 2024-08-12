/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <mutex>
#include <string>

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

class CHadmap;
namespace Json {
class Value;
}

static bool isStopHadmapsProgess = false;
static double dHadmapsProgessVal = 0.0;
class CMapDataCache {
 public:
  typedef std::map<std::wstring, std::string> MapDataCaches;

  typedef std::map<std::wstring, std::wstring> MapDataCachesW;

  typedef std::map<std::wstring, CHadmap*> HadmapCaches;

  static CMapDataCache& Instance();

  void Init();

  const wchar_t* Configuration();

  const wchar_t* L3StateMachine();

  const wchar_t* LoadHadMap(const wchar_t* strMapName);

  const wchar_t* LoadHadMapCloud(const wchar_t* strMapName);

  const wchar_t* HadmapList();

  const wchar_t* HadmapinfoList();

  const wchar_t* DeleteHadmaps(const wchar_t* wstrParams);
  const wchar_t* DownloadHadmaps(const wchar_t* wstrParams);
  const wchar_t* UploadHadmaps(const wchar_t* wstrParams);

  const wchar_t* QueryNearbyInfo(const wchar_t* wstrQueryString);
  const wchar_t* QueryNearbyLane(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat);
  const wchar_t* QueryInfoByPt(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat,
                               const wchar_t* wstrSearchDistance = L"5.0");
  const wchar_t* QueryInfoByLocalPt(const wchar_t* wstrMapName, const wchar_t* wstrX, const wchar_t* wstrY,
                                    const wchar_t* wstrZ);
  const wchar_t* QueryNextLane(const wchar_t* strMapName, const wchar_t* wstrRoadID, const wchar_t* wstrSecID,
                               const wchar_t* wstrLaneID);
  const wchar_t* QueryLonLat(const wchar_t* wstrMapName, const wchar_t* wstrRoadID, const wchar_t* wstrSecID,
                             const wchar_t* wstrLaneID, const wchar_t* wstrShift, const wchar_t* wstrOffset);
  const wchar_t* QueryLonLatByPoint(const wchar_t* wstrMapName, const wchar_t* wstrLon, const wchar_t* wstrLat,
                                    const wchar_t* wstrElemType, const wchar_t* wstrLaneID, const wchar_t* wstrShift,
                                    const wchar_t* wstrOffset, bool isStart = false);
  void ProcessMapData();

  void RemoveHadmapCacheAll(const wchar_t* wstrMapName);
  void RemoveHadmapCache(const wchar_t* wstrMapName);
  CHadmap* HadmapCache(const wchar_t* wstrMapName);
  void PushHadmapCache(const wchar_t* wstrMapName, CHadmap* pHadmap);

  void RemoveRoadCache(const wchar_t* wstrMapName);
  const char* RoadDataCache(const wchar_t* wstrMapName);
  void PushRoadCache(const wchar_t* wstrMapName, const char* wstrContent);

  void RemoveLaneCache(const wchar_t* wstrMapName);
  const char* LaneDataCache(const wchar_t* wstrMapName);
  void PushLaneCache(const wchar_t* wstrMapName, const char* wstrContent);

  void RemoveLaneBoundaryCache(const wchar_t* wstrMapName);
  const char* LaneBoundaryDataCache(const wchar_t* wstrMapName);
  void PushLaneBoundaryCache(const wchar_t* wstrMapName, const char* strContent);

  void RemoveLaneLinkCache(const wchar_t* wstrMapName);
  const char* LaneLinkDataCache(const wchar_t* wstrMapName);
  void PushLaneLinkCache(const wchar_t* wstrMapName, const char* wstrContent);

  void RemoveObjectCache(const wchar_t* wstrMapName);
  const char* ObjectDataCache(const wchar_t* wstrMapName);
  void PushObjectCache(const wchar_t* wstrMapName, const char* wstrContent);

  const wchar_t* WMapDataCache(const wchar_t* wstrMapName);
  void PushWDataCache(const wchar_t* wstrMapName, const wchar_t* wstrContent);

  std::string GetTrafficLights(const wchar_t* wstrMapName);

  const wchar_t* StopHadmapsProgess();
  const wchar_t* GetHadmapsProgessVal();

 protected:
  CMapDataCache();

  int DeleteSingleHadmap(const char* strFileName);
  int CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir);
  int CopySingleHadmap(const char* strFileName, const char* strSrcDir, const char* strDestDir, const char* strExistType,
                       std::string& newname);
  void ComposeCacheData(const wchar_t* wstrHadmapFile, CHadmap* pHadmap);
  std::wstring ConvertToJson(Json::Value& root);

  bool IncludeChinese(const char* str);

  double transform_angle(double dvalue, bool isAngle) {
    double radians = dvalue;
    if (isAngle) radians = dvalue * (M_PI / 180.0);
    while (radians < 0) {
      radians += 2 * M_PI;
    }
    while (radians > 2 * M_PI) {
      radians -= 2 * M_PI;
    }
    return radians;
  }

 protected:
  HadmapCaches m_hadmapCaches;

  MapDataCachesW m_mapWDataCache;

  MapDataCaches m_mapRoadCaches;
  MapDataCaches m_mapLaneCaches;
  MapDataCaches m_mapLaneBoundaryCaches;
  MapDataCaches m_mapLanelinkCaches;
  MapDataCaches m_mapObjCaches;

  std::wstring m_mapLists;

  std::wstring m_wstrSuccess;
  std::wstring m_wstrFailed;
  std::wstring m_wstrOkJson;
  std::wstring m_wstrConvertToJsonSucceed;
  std::wstring m_wstrConvertToJsonFailed;
  std::wstring m_wstrMapFileFormatError;

  std::wstring m_wstrDownloadHadmapResp;
  std::wstring m_wstrUploadHadmapResp;

  std::mutex m_mutexQueryNearByInfo;
  std::mutex m_mutexHadmapMutex;
  std::mutex m_mutexQueryNearBy;
  std::mutex m_mutexQueryNearbyPt;
  std::mutex m_mutexQueryNextLane;
  std::mutex m_mutexQueryLonLat;

  std::wstring strWMessageProgess;
};
