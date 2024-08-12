/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <map>
#include "camera.h"
#include "gps.h"
#include "imu.h"
#include "radar.h"
#include "sensor_truth.h"
#include "traditional_lindar.h"

class TiXmlElement;

class CSensors {
 public:
  typedef std::map<std::string, CSensorCamera> CameraMap;
  typedef std::map<std::string, CSensorRadar> RadarMap;
  typedef std::map<std::string, CSensorTraditionalLindar> TraditionalLidarMap;
  typedef std::map<std::string, CSensorTruth> SensorTruthMap;
  typedef std::map<std::string, CSensorGPS> GPSMap;
  typedef std::map<std::string, CSensorIMU> IMUMap;

  CameraMap& Cameras() { return m_mapCameras; }
  RadarMap& Radars() { return m_mapRadars; }
  TraditionalLidarMap& TraditionLidars() { return m_mapTraditionalLidars; }
  SensorTruthMap& SensorTruths() { return m_mapSensorTruths; }
  GPSMap& GPSs() { return m_mapGPSes; }
  IMUMap& IMUs() { return m_mapIMUs; }

  int Parse(const char* strSensorFile);
  int Save(const char* strSensorFile);

 protected:
  int ParseOneBaseInfo(TiXmlElement* elemSensor, CSensorBase& baseInfo);

  int ParseCameras(TiXmlElement* elemCamera, CameraMap& cameras);
  int ParseOneCamera(TiXmlElement* elemConfig, CSensorCamera& camera);

  int ParseRadars(TiXmlElement* elemRadar, RadarMap& radars);
  int ParseOneRadar(TiXmlElement* elemConfig, CSensorRadar& radar);

  int ParseTraditionalLidars(TiXmlElement* elemTraditionalLidar, TraditionalLidarMap& traditionalLidars);
  int ParseOneTraditionalLidar(TiXmlElement* elemConfig, CSensorTraditionalLindar& traditionalLidar);

  int ParseTruths(TiXmlElement* elemTruths, SensorTruthMap& truths);
  int ParseOneTruth(TiXmlElement* elemConfig, CSensorTruth& truth);

  int ParseIMUs(TiXmlElement* elemIMUs, IMUMap& imus);
  int ParseOneIMU(TiXmlElement* elemConfig, CSensorIMU& imu);

  int ParseGPSes(TiXmlElement* elemGPSs, GPSMap& gpses);
  int ParseOneGPS(TiXmlElement* elemConfig, CSensorGPS& gps);

  int SaveOneBaseInfo(TiXmlElement* elemSensor, CSensorBase& baseInfo);

  int SaveCameras(TiXmlElement* elemParent, CameraMap& cameras);
  int SaveOneCamera(TiXmlElement* elemCamera, CSensorCamera& camera);

  int SaveRadars(TiXmlElement* elemRadar, RadarMap& radars);
  int SaveOneRadar(TiXmlElement* elemConfig, CSensorRadar& radar);

  int SaveTraditionalLidars(TiXmlElement* elemTraditionLidar, TraditionalLidarMap& lidars);
  int SaveOneTraditionalLidar(TiXmlElement* elemConfig, CSensorTraditionalLindar& lidar);

  int SaveTruths(TiXmlElement* elemTruths, SensorTruthMap& truths);
  int SaveOneTruth(TiXmlElement* elemConfig, CSensorTruth& truth);

  int SaveIMUs(TiXmlElement* elemIMUs, IMUMap& imus);
  int SaveOneIMU(TiXmlElement* elemConfig, CSensorIMU& imu);

  int SaveGPSes(TiXmlElement* elemGPSs, GPSMap& gpses);
  int SaveOneGPS(TiXmlElement* elemConfig, CSensorGPS& gps);

  CameraMap m_mapCameras;
  RadarMap m_mapRadars;
  TraditionalLidarMap m_mapTraditionalLidars;
  SensorTruthMap m_mapSensorTruths;
  GPSMap m_mapGPSes;
  IMUMap m_mapIMUs;
};
