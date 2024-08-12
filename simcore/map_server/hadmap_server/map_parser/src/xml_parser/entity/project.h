/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <map>
#include <string>
#include "measurement.h"

class TiXmlElement;

class CProject {
 public:
  typedef std::map<std::string, CMeasurement> MeasurementMap;

  CProject();
  virtual int Parse(const char* strFileName, bool bLoadHadmap = true);
  int Save(const char* strFileName);

 public:
  int SetProjectPath(const char* strProjectPath);
  int SetSimulationFile(const char* strSimu);
  int SetTrafficFile(const char* strTrafficFileName);
  int SetSensorFile(const char* strSensorFileName);
  int SetEnvironmentFile(const char* strEnvironmentFileName);
  int SetControlPathFile(const char* strControlPathFileName);
  int SetGradingFile(const char* strGradingFileName);

  MeasurementMap& Measurements() { return m_mapMeasurement; }

 protected:
  int ParseSimulation(TiXmlElement* elemSimulation, std::string& strSimu);
  int ParseTraffic(TiXmlElement* elemTraffic, std::string& strTraffic);
  int ParseSensor(TiXmlElement* elemSensor, std::string& strSensor);
  int ParseEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment);
  int ParseControlPath(TiXmlElement* elemControlPath, std::string& strControlPath);
  int ParseGrading(TiXmlElement* elemGrading, std::string& strGrading);

  int ParseMeasurement(TiXmlElement* elemMeasurement, MeasurementMap& measurements);
  int ParseOneMeasurement(TiXmlElement* elemMeasurement, CMeasurement& m);

  int SaveSimulation(TiXmlElement* elemSimulation, std::string& strSimu);
  int SaveTraffic(TiXmlElement* elemTraffic, std::string& strTraffic);
  int SaveSensor(TiXmlElement* elemSensor, std::string& strSensor);
  int SaveEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment);
  int SaveControlPath(TiXmlElement* elemControlPath, std::string& strControlPath);
  int SaveGrading(TiXmlElement* elemGrading, std::string& strGrading);

  int SaveMeasurement(TiXmlElement* elemMeasurement, MeasurementMap& measurements);
  int SaveOneMeasurement(TiXmlElement* elemMeasurement, CMeasurement& m);

 public:
  std::string m_strProjectPath;
  std::string m_strSimulation;
  std::string m_strTraffic;
  std::string m_strSensor;
  std::string m_strEnvironment;
  std::string m_strControlPath;
  std::string m_strGrading;
  MeasurementMap m_mapMeasurement;
};
