// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "ibeo_data.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "traffic_ga.h"
#include "boost/foreach.hpp"
#include "common/coord_trans.h"
#include "hadmap.h"
#include "tx_math.h"

#include <iostream>
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

Ibeo_Data g_curIbeoData;
std::vector<Ibeo_Data> g_ibeoDataList;
std::vector<Ibeo_GPS> g_ibeoGpsDataList;
uint64_t g_iMaxPlayingUpdateIndex = 1;
uint64_t g_iCurPlayingUpdateIndex = 0;
uint64_t g_iIbeoCurProcIndex = 0;
uint64_t g_iIbeoStartProcIndex = 0;
uint64_t g_iIbeoEndProcIndex = 0;
uint64_t g_iIbeoCurPlayingIndex = 0;
uint64_t g_iIbeoStartPlayingIndex = 0;
uint64_t g_iIbeoEndPlayingIndex = 0;
uint64_t g_iLastStartGpsDataIndex = 0;
double g_startIbeoTimeStep = 0;
double g_curIbeoTimeStep = 0;
bool g_bCurIbeoDataPlayState = false;
unsigned int g_iCurPlayImageIndex = 0;
const char *g_szCurentIbeoDataPath;
std::wstring g_szCurentIbeoPlayImageName;
Ibeo_TradfficInfo g_IbeoTrafficInfo;

void LoadIbeoDataFromJsonFile(const char *pszFilename);
void LoadIbeoDataFromPcdFile(const char *pszFilename, unsigned int index);
void AddIbeoGpsData(Ibeo_GPS gpsData);
Ibeo_GPS GetCurrentGpsData(double duration);
void UpdateCurGpsData(double duration);
Ibeo_GPS GetCurGpsData(double curtime, int lastIndex, int nextIndex);
bool ConvMutiGpsFilesToSingle(const char *pszGpsPath);
void SaveJson(bool bLonLocation);
void SavePCL();
hadmap::txPoint ibeoPointToFVector(Vertex3 vPos, double lon, double lat, double alt);
void UpdateCurentIbeoPlayImageName(double curTimeStep);
void SaveBoostData(boost::property_tree::ptree pt, unsigned int index, bool bLonLocation);
void SavePlayInfoData();
uint64_t GetIbeoCurPlayIndex(double timestamp);
Vertex3 GetFixedLonLatAlt(double x, double y, double z, double refLon, double refLat, double refAlt,
                          double fCenterAngle, bool bLonLocation);
unsigned int GetNextTheSomeBboxDataIndex(uint64_t dataIndex, int32_t id);
Ibeo_FrontVehicle GetFrontVehicleDistance(unsigned int index);
bool CheckIbeoTrafficInfo(unsigned int index, unsigned int laneIndex);
void AddIbeoTrafficInfo(unsigned int index, int parentIndex);

#include <math.h>
#include <cmath>
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif
const double EARTH_RADIUS = 6378137;
static double rad(double d) { return d * M_PI / 180.0; }
double GetDistance(double lat1, double lng1, double lat2, double lng2) {
  double radLat1 = rad(lat1);
  double radLat2 = rad(lat2);
  double a = radLat1 - radLat2;
  double b = rad(lng1) - rad(lng2);
  double s = 2 * std::asin(std::sqrt(std::pow(std::sin(a / 2), 2) +
                                     std::cos(radLat1) * std::cos(radLat2) * std::pow(std::sin(b / 2), 2)));
  s = s * EARTH_RADIUS;
  s = std::round(s * 10000) / 10000;
  return s;
}
Vertex3 txPointToFVector_YZ(hadmap::txPoint point) {
  Vertex3 vPos = Vertex3(point.x, point.y, point.z);
  coord_trans_api::lonlat2local(vPos.x, vPos.y, vPos.z, point.x, point.y, point.z);
  return Vertex3(vPos.x, 0.0, vPos.y * -1);
}

Vertex3 txPointToFVector(double x, double y, double z, double yOffset) {
  hadmap::txPoint tmpTXPoint(x, y, z);
  Vertex3 tmpVector = txPointToFVector_YZ(tmpTXPoint);
  tmpVector.y += yOffset;
  return tmpVector;
}

void Ibeo_TradfficLaneInfo::AddData(uint64_t startMapLaneIndex, uint64_t startMapPointIndex, unsigned int value,
                                    unsigned int laneIndex) {}

void LoadIbeoData(const char *pszFilePath, int start, int end, bool bLonLocation) {
  if (strlen(pszFilePath) <= 0 || start < 0 || end < start) return;

  g_bCurIbeoDataPlayState = false;
  g_curIbeoTimeStep = 0;
  g_iCurPlayingUpdateIndex = 0;
  g_iIbeoCurProcIndex = start;
  g_iIbeoStartProcIndex = start;
  g_iIbeoEndProcIndex = end;
  g_iIbeoStartPlayingIndex = 0;
  g_iIbeoEndPlayingIndex = 0;
  g_iIbeoCurPlayingIndex = 0;
  g_ibeoDataList.clear();

  g_szCurentIbeoDataPath = pszFilePath;
  g_ibeoDataList.reserve(end - start + 1);
  for (unsigned int i = start; i <= end; i++) {
    char szFilename[1024];
    sprintf(szFilename, "%s/%s/data_%d.json", std::string(g_szCurentIbeoDataPath).c_str(),
            ((bLonLocation) ? "lonlat" : "local"), i + 1);
    LoadIbeoDataFromJsonFile(szFilename);
    continue;
    if (!bLonLocation) {
      sprintf(szFilename, "%s/%s/data_%d.pcd", std::string(g_szCurentIbeoDataPath).c_str(),
              ((bLonLocation) ? "lonlat" : "local"), i + 1);
      LoadIbeoDataFromPcdFile(szFilename, i - start);
    }

    g_iIbeoCurProcIndex = i;
    // g_pWorldManager->AddPercentBarProcess(1);
  }

  for (unsigned int j = 0; j < g_ibeoDataList.size() - 1; j++) {
    // LOG(WARNING) << g_ibeoDataList.size() << std::endl; >> 21
    float fPassedTime = (g_ibeoDataList[j + 1].bboxList[0].duration - g_ibeoDataList[j].bboxList[0].duration);
    for (unsigned int i = 0; i < g_ibeoDataList[j].bboxList.size(); i++) {
      g_ibeoDataList[j].bboxList[i].velocity = 0.0;
      int idx = -1;
      if (j) {
        if (g_ibeoDataList[j].bboxList[i].type == "Car" || g_ibeoDataList[j].bboxList[i].type == "Truck")
          idx = GetNextTheSomeBboxDataIndex(j + 1, g_ibeoDataList[j].bboxList[i].id);
      } else {
        idx = 0;
      }
      if (idx >= 0) {
        Vertex3 vNext =
            txPointToFVector(g_ibeoDataList[j + 1].bboxList[idx].lon, g_ibeoDataList[j + 1].bboxList[idx].lat,
                             g_ibeoDataList[j + 1].bboxList[idx].alt, 0);
        g_ibeoDataList[j].bboxList[i].vCenter = txPointToFVector(
            g_ibeoDataList[j].bboxList[i].lon, g_ibeoDataList[j].bboxList[i].lat, g_ibeoDataList[j].bboxList[i].alt, 0);
        // Vertex3 vNext = g_pWorldManager->txPointToFVector(g_ibeoDataList[j + 1].bboxList[idx].lon, g_ibeoDataList[j +
        // 1].bboxList[idx].lat, g_ibeoDataList[j + 1].bboxList[idx].alt, 0); g_ibeoDataList[j].bboxList[i].vCenter =
        // g_pWorldManager->txPointToFVector(g_ibeoDataList[j].bboxList[i].lon, g_ibeoDataList[j].bboxList[i].lat,
        // g_ibeoDataList[j].bboxList[i].alt, 0); g_ibeoDataList[j].bboxList[i].moveDistance = sqrt((vNext.x -
        // g_ibeoDataList[j].bboxList[i].vCenter.x) * (vNext.x - g_ibeoDataList[j].bboxList[i].vCenter.x) + (vNext.z -
        // g_ibeoDataList[j].bboxList[i].vCenter.z) * (vNext.z - g_ibeoDataList[j].bboxList[i].vCenter.z));
        g_ibeoDataList[j].bboxList[i].moveDistance =
            GetDistance(g_ibeoDataList[j + 1].bboxList[idx].lat, g_ibeoDataList[j + 1].bboxList[idx].lon,
                        g_ibeoDataList[j].bboxList[i].lat, g_ibeoDataList[j].bboxList[i].lon);
        g_ibeoDataList[j].bboxList[i].velocity = g_ibeoDataList[j].bboxList[i].moveDistance / fPassedTime;
      }
    }
  }
  g_iIbeoStartPlayingIndex = start;
  g_iIbeoEndPlayingIndex = end;
  g_iIbeoCurPlayingIndex = g_iIbeoStartPlayingIndex;
  g_startIbeoTimeStep = g_ibeoDataList[0].bboxList[0].duration;
  g_curIbeoTimeStep = g_startIbeoTimeStep;
}

void LoadIbeoDataFromJsonFile(const char *pszFilename) {
  boost::property_tree::ptree pt;
  boost::property_tree::ptree items;
  boost::property_tree::read_json<boost::property_tree::ptree>(pszFilename, pt);

  items = pt.get_child("objects");
  Ibeo_Data data;
  for (boost::property_tree::ptree::iterator it = items.begin(); it != items.end(); ++it) {
    Ibeo_BBox bbox;
    bbox.id = it->second.get<uint32_t>("id");
    bbox.type = it->second.get<std::string>("type");
    bbox.rot = it->second.get<double>("rotation");
    bbox.duration = it->second.get<double>("duration");
    bbox.lon = it->second.get<double>("longitude");
    bbox.lat = it->second.get<double>("latitude");
    bbox.alt = it->second.get<double>("altitude");
    bbox.velocity = it->second.get<double>("velocity");

    int index = 0;
    boost::property_tree::ptree vertexe_array = it->second.get_child("vertexes");
    for (boost::property_tree::ptree::iterator v = vertexe_array.begin(); v != vertexe_array.end(); ++v) {
      std::string xString = v->second.get<std::string>("x");
      std::string yString = v->second.get<std::string>("y");
      sscanf(xString.c_str(), "%f", &bbox.vPos[index].x);
      sscanf(yString.c_str(), "%f", &bbox.vPos[index].y);
      index += 1;
    }

    data.bboxList.push_back(bbox);
  }

  g_ibeoDataList.push_back(data);
}

void LoadIbeoDataFromPcdFile(const char *pszFilename, unsigned int index) {}

void IbeoDataStop() {
  g_iMaxPlayingUpdateIndex = 1;
  g_iCurPlayingUpdateIndex = 0;
  g_iIbeoCurProcIndex = 0;
  g_iIbeoStartProcIndex = 0;
  g_iIbeoEndProcIndex = 0;
  g_iIbeoCurPlayingIndex = 0;
  g_iIbeoStartPlayingIndex = 0;
  g_iIbeoEndPlayingIndex = 0;
  g_iLastStartGpsDataIndex = 0;
  g_startIbeoTimeStep = 0;
  g_curIbeoTimeStep = 0;
  g_bCurIbeoDataPlayState = false;
  g_iCurPlayImageIndex = 0;
}

void IbeoDataClear() {
  IbeoDataStop();
  g_curIbeoData.Clear();
  g_ibeoDataList.clear();
  g_ibeoGpsDataList.clear();
}

void UpdateIbeoDataPlay(double time) {
  if (!g_bCurIbeoDataPlayState) return;

  if (g_iIbeoCurPlayingIndex < g_iIbeoEndPlayingIndex) {
    g_curIbeoTimeStep += time;
    g_iIbeoCurPlayingIndex = g_iIbeoStartPlayingIndex + GetIbeoCurPlayIndex(g_curIbeoTimeStep);
    // UpdateCurrentIbeoData();
  } else {
    g_bCurIbeoDataPlayState = false;
  }
}

void AddIbeoTrafficInfo(unsigned int index, int parentIndex) {
  for (unsigned int i = 0; i < g_IbeoTrafficInfo.laneList.size(); i++) {
    for (unsigned int j = 0; j < g_IbeoTrafficInfo.laneList[i].laneInfoData.size(); j++) {
      if (g_IbeoTrafficInfo.laneList[i].laneInfoData[j] == index) {
        if (parentIndex >= 0) {
          std::vector<unsigned int>::iterator it = g_IbeoTrafficInfo.laneList[i].laneInfoData.begin() + j;
          g_IbeoTrafficInfo.laneList[i].laneInfoData.insert(it, parentIndex);
        }

        return;
      }
    }
  }

  Ibeo_TradfficLaneInfo info;
  if (parentIndex >= 0) info.laneInfoData.push_back(parentIndex);

  info.laneInfoData.push_back(index);
  g_IbeoTrafficInfo.laneList.push_back(info);
}

bool CheckIbeoTrafficInfo(unsigned int index, unsigned int laneIndex) {
  if (g_curIbeoData.bboxList[index].vNextPosList.size()) {
    if (g_curIbeoData.bboxList[index].type != "Car" && g_curIbeoData.bboxList[index].type != "Truck") return false;

    if (g_curIbeoData.bboxList[index].vNextPosList[0].laneIndex == laneIndex) return true;

    if (g_curIbeoData.bboxList[index].vNextPosList[0].bChangeState) {
      unsigned int count = g_curIbeoData.bboxList[index].vNextPosList.size();
      return (g_curIbeoData.bboxList[index].vNextPosList[count - 1].laneIndex == laneIndex);
    }
  }

  return false;
}

Ibeo_FrontVehicle GetFrontVehicleDistance(unsigned int index) {
  Ibeo_FrontVehicle data;
  if (g_curIbeoData.bboxList[index].vNextPosList.size()) {
    unsigned int laneIndex = g_curIbeoData.bboxList[index].vNextPosList[0].laneIndex;
    uint64_t startMapLaneIndex = g_curIbeoData.bboxList[index].startMapLaneIndex;
    uint64_t startMapPointIndex = g_curIbeoData.bboxList[index].startMapPointIndex;
    Vertex3 vPos = g_curIbeoData.bboxList[index].vCenter;
    for (unsigned int i = 0; i < g_curIbeoData.bboxList.size(); i++) {
      if (i == index || !g_curIbeoData.bboxList[i].vNextPosList.size()) continue;

      bool bFindResult = false;
      if (g_curIbeoData.bboxList[i].startMapLaneIndex == startMapLaneIndex) {
        if (g_curIbeoData.bboxList[i].startMapPointIndex > startMapPointIndex) bFindResult = true;
      } else if (g_curIbeoData.bboxList[i].startMapLaneIndex > (startMapLaneIndex - laneIndex)) {
        bFindResult = true;
      }

      if (bFindResult) {
        bFindResult = (g_curIbeoData.bboxList[i].vNextPosList[0].laneIndex == laneIndex);
        if (!bFindResult && g_curIbeoData.bboxList[i].vNextPosList[0].bChangeState) {
          unsigned int count = g_curIbeoData.bboxList[i].vNextPosList.size();
          bFindResult = (g_curIbeoData.bboxList[i].vNextPosList[count - 1].laneIndex == laneIndex);
        }

        if (bFindResult) {
          double curDistance = VectorSub(g_curIbeoData.bboxList[i].vCenter, vPos).Magnitude();
          if (curDistance < data.distance) {
            data.distance = curDistance;
            data.parentIndex = i;
          }
        }
      }
    }
  }

  return data;
}

unsigned int GetNextTheSomeBboxDataIndex(uint64_t dataIndex, int32_t id) {
  if (dataIndex < g_ibeoDataList.size()) {
    for (int j = 0; j < g_ibeoDataList[dataIndex].bboxList.size(); j++) {
      if (id == g_ibeoDataList[dataIndex].bboxList[j].id) return j;
    }
  }

  return -1;
}

void ChangeCurrentIbeoTimeStamp(int index) {
  if (g_iIbeoEndPlayingIndex) {
    int64_t nextIndex = g_iIbeoCurPlayingIndex + index;
    if (nextIndex < g_iIbeoStartPlayingIndex)
      nextIndex = g_iIbeoStartPlayingIndex;
    else if (nextIndex > g_iIbeoEndPlayingIndex)
      nextIndex = g_iIbeoEndPlayingIndex;

    g_iIbeoCurPlayingIndex = nextIndex;
    UpdateIbeoData();
  }
}

void UpdateIbeoData(double timeStamp) {
  if (g_ibeoDataList.size()) {
    if (g_iIbeoCurPlayingIndex > g_iIbeoEndPlayingIndex) return;

    g_curIbeoTimeStep += timeStamp;
    g_iCurPlayImageIndex = g_iIbeoCurPlayingIndex;
    UpdateCurentIbeoPlayImageName(g_curIbeoTimeStep);
    // UpdateCurrentIbeoData();
  } else {
    g_startIbeoTimeStep = g_curIbeoTimeStep = 0;
  }
}

void UpdateIbeoData() {
  if (g_ibeoDataList.size()) {
    if (g_iIbeoCurPlayingIndex > g_iIbeoEndPlayingIndex) return;

    g_startIbeoTimeStep = g_ibeoDataList[0].bboxList[0].duration;
    g_curIbeoTimeStep = g_ibeoDataList[g_iIbeoCurPlayingIndex - g_iIbeoStartPlayingIndex].bboxList[0].duration;
    UpdateIbeoData(0.0);
  } else {
    g_startIbeoTimeStep = g_curIbeoTimeStep = 0;
  }
}

void UpdateIbeoImageForPlay(double timeStamp) { UpdateCurentIbeoPlayImageName(timeStamp); }

void GetInitialGapAndVelocity(double &gap, double &velocity) {
  gap = 0.0;
  velocity = 0.0;
  unsigned int gapCount = 0;
  unsigned int velocityCount = 0;
  for (unsigned int i = 0; i < g_curIbeoData.bboxList.size(); i++) {
    if (g_curIbeoData.bboxList[i].type == "Car" || g_curIbeoData.bboxList[i].type == "Truck") {
      velocity += g_curIbeoData.bboxList[i].velocity;
      if (g_curIbeoData.bboxList[i].distance < 100) gap += g_curIbeoData.bboxList[i].distance;

      gapCount += 1;
      velocityCount += 1;
    }
  }

  if (gapCount) gap /= gapCount;

  if (velocityCount) velocity /= velocityCount;
}

void SaveJson(bool bLonLocation) {
  for (int m = 0; m < g_ibeoDataList.size(); m++) {
    if (g_ibeoDataList[m].bboxList[0].lon <= 0 || g_ibeoDataList[m].bboxList[0].lat <= 0) continue;

    boost::property_tree::ptree pt, children;
    for (int i = 0; i < g_ibeoDataList[m].bboxList.size(); i++) {
      boost::property_tree::ptree objectPT, vertexPT;
      objectPT.put("shape", "Box");
      objectPT.put("id", g_ibeoDataList[m].bboxList[i].id);
      objectPT.put("type", g_ibeoDataList[m].bboxList[i].type);
      objectPT.put("rotation", g_ibeoDataList[m].bboxList[i].rot);
      objectPT.put("duration", g_ibeoDataList[m].bboxList[i].duration);
      objectPT.put("longitude", g_ibeoDataList[m].bboxList[i].lon);
      objectPT.put("latitude", g_ibeoDataList[m].bboxList[i].lat);
      objectPT.put("altitude", g_ibeoDataList[m].bboxList[i].alt);
      objectPT.put("velocity", g_ibeoDataList[m].bboxList[i].velocity);

      for (int j = 0; j < 4; j++) {
        boost::property_tree::ptree child;
        child.put("x", g_ibeoDataList[m].bboxList[i].vPos[j].x);
        child.put("y", g_ibeoDataList[m].bboxList[i].vPos[j].y);
        vertexPT.push_back(std::make_pair("", child));
      }

      objectPT.push_back(std::make_pair("vertexes", vertexPT));
      children.push_back(std::make_pair("", objectPT));
    }

    pt.add_child("objects", children);
    pt.put("ground", -1.68843);
    SaveBoostData(pt, m + 1, bLonLocation);
    // g_pWorldManager->AddPercentBarProcess(1);
  }

  SavePlayInfoData();
}

void SavePCL() {}

void SavePlayInfoData() {
  char szFileName[1024];
  sprintf(szFileName, "%s/Info.txt", std::string(g_szCurentIbeoDataPath).c_str());
  FILE *pFile = fopen(szFileName, "w+");
  fprintf(pFile, "count: %zd\n", g_ibeoDataList.size());
  for (int i = 0; i < g_ibeoDataList.size(); i++) fprintf(pFile, "%lf\n", g_ibeoDataList[i].bboxList[0].duration);
  fclose(pFile);
}

void SaveBoostData(boost::property_tree::ptree pt, unsigned int index, bool bLonLocation) {
  char szFileName[1024], szFileFormat[24];
  strcpy(szFileName, std::string(g_szCurentIbeoDataPath).c_str());
  sprintf(szFileFormat, "/%s/data_%d.json", (bLonLocation) ? "/lonlat" : "/local", index);
  strcat(szFileName, szFileFormat);
  boost::property_tree::json_parser::write_json(szFileName, pt);
}

hadmap::txPoint ibeoPointToFVector(Vertex3 vPos, double lon, double lat, double alt) {
  hadmap::txPoint point(vPos.x, vPos.y, vPos.z);
  coord_trans_api::local2lonlat(point.x, point.y, point.z, lon, lat, alt);
  return point;
}

void GetCurrentIbeoTimestepData(char *pszText) {
  if (!g_ibeoDataList.size() || g_curIbeoTimeStep < g_startIbeoTimeStep) {
    strcpy(pszText, "");
  } else {
    time_t t = g_curIbeoTimeStep - g_startIbeoTimeStep;
    tm *local = localtime(&t);
    sprintf(pszText, "%d:%d:%d", local->tm_hour - 8, local->tm_min, local->tm_sec);
  }
}

bool GetCurrentIbeoPlayState() { return g_bCurIbeoDataPlayState; }

void ChangeCurrentIbeoPlayState() {
  if (g_iIbeoEndPlayingIndex <= 0) {
    g_bCurIbeoDataPlayState = false;
  } else {
    g_bCurIbeoDataPlayState = !g_bCurIbeoDataPlayState;
    if (g_bCurIbeoDataPlayState && g_iIbeoCurPlayingIndex >= g_iIbeoEndPlayingIndex)
      g_iIbeoCurPlayingIndex = g_iIbeoStartPlayingIndex;
  }
}

void GetCurrentIbeoPlayPrecntRate(char *pszText) {
  if (g_iIbeoEndPlayingIndex <= 0) {
    strcpy(pszText, "");
  } else {
    int percent =
        (g_iIbeoCurPlayingIndex - g_iIbeoStartPlayingIndex) * 100 / (g_iIbeoEndPlayingIndex - g_iIbeoStartPlayingIndex);
    sprintf(pszText, "%d %%", percent);
  }
}

void GetCurrentIbeoPlayTimeStamp(char *pszText) {
  if (g_iIbeoEndPlayingIndex <= 0 || g_curIbeoTimeStep == 0)
    strcpy(pszText, "");
  else
    sprintf(pszText, "UTC: %lf", g_curIbeoTimeStep);
}

void UpdateCurentIbeoPlayImageName(double curTimeStep) {
  char szNumber[128], szTempName[128], szName[1024];

  sprintf(szNumber, "%lf", curTimeStep);
  char *pStr = strstr(szNumber, ".");
  int index = strlen(pStr);

  int len = strlen(szNumber) - index + 2;
  memcpy(szTempName, szNumber, len);
  szTempName[len] = '\0';
  strcat(szTempName, "00000");

  sprintf(szName, "%s/images/%s.jpg", std::string(g_szCurentIbeoDataPath).c_str(), szTempName);
  g_szCurentIbeoPlayImageName = std::wstring(&szName[0], &szName[512]);
}

bool GetCurrentIbeoCarLonLatAlt(double &lon, double &lat, double &alt) {
  if (g_iIbeoCurPlayingIndex < g_iIbeoEndPlayingIndex - 1) {
    int index = g_iIbeoCurPlayingIndex - g_iIbeoStartPlayingIndex;
    lon = g_ibeoDataList[index].bboxList[0].lon;
    lat = g_ibeoDataList[index].bboxList[0].lat;
    alt = g_ibeoDataList[index].bboxList[0].alt;
    return true;
  }

  return false;
}

void PlayIbeoDataByIndex(uint64_t index) {
  if (index < g_iIbeoStartPlayingIndex || index > g_iIbeoEndPlayingIndex) return;

  g_iIbeoCurPlayingIndex = index;
}

uint64_t GetIbeoCurPlayIndex(double timestamp) {
  uint64_t index = g_iIbeoCurPlayingIndex - g_iIbeoStartPlayingIndex;
  for (uint64_t i = index; i < g_ibeoDataList.size() - 1; i++) {
    if (timestamp < g_ibeoDataList[i].bboxList[0].duration) return i - 1;
  }

  return g_ibeoDataList.size() - 1;
}

Vertex3 GetFixedLonLatAlt(double x, double y, double z, double refLon, double refLat, double refAlt,
                          double fCenterAngle, bool bLonLocation) {
  Vertex3 vPos;
  double v2x = x * cos(fCenterAngle) - y * sin(fCenterAngle);
  double v2y = x * sin(fCenterAngle) + y * cos(fCenterAngle);
  vPos.x = v2x;
  vPos.y = v2y;
  vPos.z = z;

  if (bLonLocation) {
    hadmap::txPoint point = ibeoPointToFVector(vPos, refLon, refLat, refAlt);
    return Vertex3(point.x, point.y, point.z);
  }

  return Vertex3(vPos.x + refLon, vPos.y + refLat, vPos.z + refAlt);
}

// IbeoOutputDataForGA(g_TT_Obs, g_V_Obs, g_TT_ObsCount, OBS_DISTANCE);

void IbeoOutputDataForGA(TT_Feature &tt_feature, double TT_ObsDistance, const std::string &outdir) {
  if (g_ibeoDataList.size() <= 0) return;

  tt_feature.v_obs_.clear();
  tt_feature.tt_obs_ = 0.0;
  // TT_V_OutputCount = 0;
  double curMovedDistance = 0.0;
  // TT_Obs = 0.0;
  float totalVelocity = 0.0;
  uint64_t velocityCount = 0;
  for (unsigned int j = 0; j < g_ibeoDataList.size(); j++) {
    // TT_Obs += (j) ? (g_ibeoDataList[j].bboxList[0].duration - g_ibeoDataList[j - 1].bboxList[0].duration) : 0.0;
    tt_feature.tt_obs_ +=
        (j) ? (g_ibeoDataList[j].bboxList[0].duration - g_ibeoDataList[j - 1].bboxList[0].duration) : 0.0;
    curMovedDistance += g_ibeoDataList[j].bboxList[0].moveDistance;

    for (unsigned int i = 0; i < g_ibeoDataList[j].bboxList.size(); i++) {
      if (g_ibeoDataList[j].bboxList[i].type == "Car" || g_ibeoDataList[j].bboxList[i].type == "Truck") {
        if (g_ibeoDataList[j].bboxList[i].velocity < 20) {
          totalVelocity += g_ibeoDataList[j].bboxList[i].velocity;
          velocityCount += 1;
        }
      }
    }

    if (curMovedDistance >= TT_ObsDistance) {
      auto vel = (velocityCount) ? (totalVelocity / velocityCount) : 0.0f;
      tt_feature.v_obs_.emplace_back(vel);
      curMovedDistance = 0;
    }
  }

  std::string filename = outdir + "/TrafficInputData.txt";
  tt_feature.tofile(filename);

  LOG(INFO) << "IbeoOutputDataForGA:" << filename << " saved!";
}
