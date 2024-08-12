// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <sstream>
#include <string>
#include <vector>
#include "tx_math.h"

#define MAX_SPEED 27.8
#define MAX_DISTANCE 999999

struct Ibeo_Vertex {
  Ibeo_Vertex() { SetData(0, 0); }

  Ibeo_Vertex(float _x, float _y) { SetData(_x, _y); }

  void SetData(float _x, float _y) {
    x = _x;
    y = _y;
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{Ibeo_Vertex|x:" << x << ",y:" << y << "}";
    return ss.str();
  }
  float x, y;
};

struct Ibeo_Vertex3 {
  Ibeo_Vertex3() { SetData(0, 0, 0); }

  Ibeo_Vertex3(float _x, float _y, float _z) { SetData(_x, _y, _z); }

  void SetData(float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{Ibeo_Vertex3|x:" << x << ",y:" << y << ",z:" << z << "}";
    return ss.str();
  }
  float x, y, z;
};

struct Ibeo_LaneVertex {
  Ibeo_LaneVertex(float _x, float _y, float _z, unsigned int _laneIndex, bool _bChangeState) {
    x = _x;
    y = _y;
    z = _z;
    laneIndex = _laneIndex;
    bChangeState = _bChangeState;
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{Ibeo_LaneVertex|x:" << x << ",y:" << y << ",z:" << z << ",laneIndex:" << laneIndex
       << ",bChangeState:" << bChangeState << "}";
    return ss.str();
  }

  float x, y, z;
  unsigned int laneIndex;
  bool bChangeState;
};

struct Ibeo_BBox {
  Ibeo_BBox() {
    id = 0;
    lon = 0;
    lat = 0;
    alt = 0;
    rot = 0;
    duration = 0;
    velocity = 5;
    startMapLaneIndex = 0;
    startMapPointIndex = 0;
    distance = MAX_DISTANCE;
    parentIndex = -1;
    moveDistance = 0.0;
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{Ibeo_BBox|id:" << id << ",type:" << type << ",lon:" << lon << ",lat:" << lat << ",alt:" << alt
       << ",rot:" << rot << ",duration:" << duration << ",velocity:" << velocity
       << ",startMapLaneIndex:" << startMapLaneIndex << ",startMapPointIndex:" << startMapPointIndex
       << ",distance:" << distance << ",parentIndex:" << parentIndex << ",moveDistance:" << moveDistance;
    ss << ",vCenter:{Vertex3|x:" << vCenter.x << ",y:" << vCenter.y << ",z:" << vCenter.z << "}";
    ss << ",vTempPos:{Vertex3|x:" << vTempPos.x << ",y:" << vTempPos.y << ",z:" << vTempPos.z << "}";
    for (int i = 0; i < 4; i++) {
      ss << ",vPos[" << i << "]:" << vPos[i].tostring();
    }
    int j = 0;
    for (auto &i : vNextPosList) {
      ss << ",vNextPosList[" << j++ << "]:" << i.tostring();
    }
    ss << "}";
    return ss.str();
  }
  int32_t id;
  std::string type;
  double rot;
  Ibeo_Vertex vPos[4];
  Vertex3 vCenter;
  double lon, lat, alt;
  double duration;
  Vertex3 vTempPos;
  double velocity;
  double moveDistance;
  std::vector<Ibeo_LaneVertex> vNextPosList;
  uint64_t startMapLaneIndex;
  uint64_t startMapPointIndex;
  int parentIndex;
  double distance;
};

struct Ibeo_TradfficLaneInfoData {
  Ibeo_TradfficLaneInfoData(uint64_t _startMapLaneIndex, uint64_t _startMapPointIndex, unsigned int _value) {
    startMapLaneIndex = _startMapLaneIndex;
    startMapPointIndex = _startMapPointIndex;
    value = _value;
  }

  uint64_t startMapLaneIndex;
  uint64_t startMapPointIndex;
  unsigned int value;
};

struct Ibeo_TradfficLaneInfo {
  void AddData(uint64_t startMapLaneIndex, uint64_t startMapPointIndex, unsigned int value, unsigned int laneIndex);
  std::vector<unsigned int> laneInfoData;
};

struct Ibeo_TradfficInfo {
  void Clear() { laneList.clear(); }

  std::vector<Ibeo_TradfficLaneInfo> laneList;
};

struct Ibeo_FrontVehicle {
  Ibeo_FrontVehicle() {
    parentIndex = -1;
    distance = MAX_DISTANCE;
  }

  int parentIndex;
  double distance;
};

struct Ibeo_GPS {
  double utc_time, lon, lat, height;
  double qx, qy, qz, qw;
  double roll, pitch, yaw, velocity_east, velocity_north, velocity_up, roll_rate, pitch_rate, yaw_rate;  // unused
  bool operator<(const Ibeo_GPS &rhs) const { return utc_time < rhs.utc_time; }
};

struct Ibeo_Data {
  void Clear() {
    bboxList.clear();
    vertex3List.clear();
  }
  std::string tostring() {
    std::stringstream ss;
    ss << "{Ibeo_Data|";
    int j = 0;
    for (auto &i : bboxList) {
      ss << ",bboxList[" << j++ << "]:" << i.tostring();
    }
    j = 0;
    for (auto &i : vertex3List) {
      ss << ",vertex3List[" << j++ << "]:" << i.tostring();
    }
    ss << "}";
    return ss.str();
  }
  std::vector<Ibeo_BBox> bboxList;
  std::vector<Ibeo_Vertex3> vertex3List;
};

extern std::vector<Ibeo_Data> g_ibeoDataList;
extern Ibeo_Data g_curIbeoData;
extern uint64_t g_iIbeoCurPlayingIndex;
extern uint64_t g_iMaxPlayingUpdateIndex;
extern uint64_t g_iIbeoCurProcIndex;
extern uint64_t g_iIbeoStartProcIndex;
extern uint64_t g_iIbeoEndProcIndex;
extern uint64_t g_iIbeoStartPlayingIndex;
extern uint64_t g_iIbeoEndPlayingIndex;
extern std::wstring g_szCurentIbeoPlayImageName;
extern Ibeo_TradfficInfo g_IbeoTrafficInfo;

double GetDistance(double lat1, double lng1, double lat2, double lng2);

void LoadIbeoData(const char *pszFilePath, int start, int end, bool bLonLocation);

void UpdateIbeoDataPlay(double time);
void GetCurrentIbeoTimestepData(char *pszText);
bool GetCurrentIbeoPlayState();
void ChangeCurrentIbeoPlayState();
void GetCurrentIbeoPlayPrecntRate(char *pszText);
bool GetCurrentIbeoCarLonLatAlt(double &lon, double &lat, double &alt);
void ChangeCurrentIbeoTimeStamp(int index);
void GetCurrentIbeoPlayTimeStamp(char *pszText);
void UpdateIbeoImageForPlay(double timeStamp);
void UpdateIbeoData();
void PlayIbeoDataByIndex(uint64_t index);

void GetInitialGapAndVelocity(double &gap, double &velocity);
void IbeoOutputDataForGA(struct TT_Feature &tt_feature, double TT_ObsDistance, const std::string &outdir);
void IbeoDataStop();
void IbeoDataClear();
