/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <map>
#include "camera.h"
#include "fisheye.h"
#include "gps.h"
#include "imu.h"
#include "obu.h"
#include "radar.h"
#include "sensor_truth.h"
#include "traditional_lindar.h"
#include "ultrasonic.h"

class TiXmlElement;

class CSensors {
 public:
  typedef std::map<std::string, CSensorCamera> CameraMap;                       // 摄像头
  typedef std::map<std::string, CSensorRadar> RadarMap;                         // 毫米波雷达
  typedef std::map<std::string, CSensorTraditionalLindar> TraditionalLidarMap;  // 激光雷达
  typedef std::map<std::string, CSensorTruth> SensorTruthMap;                   // 真值传感器
  typedef std::map<std::string, CSensorGPS> GPSMap;                             // GPS
  typedef std::map<std::string, CSensorIMU> IMUMap;                             // IMU

  typedef std::map<std::string, CSensorFisheye> FisheyeMap;        // fisheye
  typedef std::map<std::string, CSensorCamera> SemanticMap;        // 语义相机
  typedef std::map<std::string, CSensorCamera> DepthMap;           // 深度相机
  typedef std::map<std::string, CSensorUltrasonic> UltrasonicMap;  // 超声波雷达

  typedef std::map<std::string, COBU> OBUMap;  // 车载单元

  CameraMap& Cameras() { return m_mapCameras; }
  RadarMap& Radars() { return m_mapRadars; }
  TraditionalLidarMap& TraditionLidars() { return m_mapTraditionalLidars; }
  SensorTruthMap& SensorTruths() { return m_mapSensorTruths; }
  GPSMap& GPSs() { return m_mapGPSes; }
  IMUMap& IMUs() { return m_mapIMUs; }

  FisheyeMap& Fisheyes() { return m_mapFisheyes; }
  CameraMap& Semantics() { return m_mapSemantics; }
  CameraMap& Depths() { return m_mapDepths; }
  UltrasonicMap& Ultrasonics() { return m_mapUltrasonics; }

  OBUMap& OBUS() { return m_mapOBUs; }

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

  int ParseFisheyes(TiXmlElement* elemFisheyes, FisheyeMap& fisheyes);
  int ParseOneFisheye(TiXmlElement* elemConfig, CSensorFisheye& fisheye);

  int ParseSemantics(TiXmlElement* elemSemantics, SemanticMap& semantics);
  int ParseOneSemantics(TiXmlElement* elemConfig, CSensorCamera& semantic);

  int ParseDepths(TiXmlElement* elemDepths, DepthMap& depths);
  int ParseOneDepth(TiXmlElement* elemConfig, CSensorCamera& depth);

  int ParseUltrasonics(TiXmlElement* elemUltrasonics, UltrasonicMap& ultrasonics);
  int ParseOneUltrasonic(TiXmlElement* elemConfig, CSensorUltrasonic& ultrasonic);

  int ParseOBUs(TiXmlElement* elemOBUs, OBUMap& obus);
  int ParseOneOBU(TiXmlElement* elemConfig, COBU& obu);

  int SaveOneBaseInfo(TiXmlElement* elemSensor, CSensorBase& baseInfo);

  int SaveCameras(TiXmlElement* elemParent, CameraMap& cameras);
  int SaveOneCamera(TiXmlElement* elemCamera, CSensorCamera& camera);

  int SaveSemantics(TiXmlElement* elemParent, SemanticMap& semantics);

  int SaveDepths(TiXmlElement* elemParent, DepthMap& depths);

  int SaveFisheyes(TiXmlElement* elemFisheyes, FisheyeMap& fisheyes);
  int SaveOneFisheye(TiXmlElement* elemConfig, CSensorFisheye& fisheye);

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

  int SaveUltrasonics(TiXmlElement* elemUltrasonics, UltrasonicMap& ultrasonics);
  int SaveOneUltrasonic(TiXmlElement* elemConfig, CSensorUltrasonic& ultrasonic);

  int SaveOBUs(TiXmlElement* elemUltrasonics, OBUMap& obus);
  int SaveOneOBU(TiXmlElement* elemConfig, COBU& obu);

  CameraMap m_mapCameras;
  RadarMap m_mapRadars;
  TraditionalLidarMap m_mapTraditionalLidars;
  SensorTruthMap m_mapSensorTruths;
  GPSMap m_mapGPSes;
  IMUMap m_mapIMUs;

  FisheyeMap m_mapFisheyes;
  SemanticMap m_mapSemantics;
  DepthMap m_mapDepths;
  UltrasonicMap m_mapUltrasonics;

  OBUMap m_mapOBUs;
};
