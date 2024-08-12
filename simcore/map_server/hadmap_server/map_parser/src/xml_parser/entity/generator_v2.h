/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <boost/filesystem.hpp>

#include "common/xml_parser/entity/environments/environment.h"
#include "parser.h"
#include "sensors/sensors.h"
#include "simulation_param.h"
#include "traffic_param.h"

struct sTagBatchSceneParamV2;

class CGeneratorV2 : public CParser {
 public:
  static bool isStopGenScenesProgess;
  static double dGenScenesProgessVal;

  int Parse(const char* strSimuParamFile, const char* strTrafficParamFile);

  int Parse(const char* strSimName, sTagBatchSceneParamV2& param, const char* strParams);

  int ParseXosc(const char* strSimName, sTagBatchSceneParamV2& param, const char* strParams);

  int GetSceneFileSize(int nType, const char* strSimName, uint64_t& ullFileSize);

  int Generate();

  int CalculateCount(int64_t& count, int64_t& size);

  int Generate(const char* strSimName, const char* strSourceType, sTagBatchSceneParamV2& param, const char* strParams,
               std::string& strGeneratedDirName);
  int GenerateCount(const char* strSimName, sTagBatchSceneParamV2& param, uint64_t& llCount, uint64_t& llSize,
                    uint64_t& ullTime);

  int Save(const char* strSimuParamFile, const char* strTrafficParamFile);

  int BatchSave(sTagBatchSceneParamV2& param, std::string& strGeneratedDirName);

  int BatchSaveOne(sTagBatchSceneParamV2& param, std::string& strGeneratedDirName, int simTrafficsIndex);

  int SaveOneSim(sTagSimuTraffic& scene, std::string strFileName, const std::string& strID,
                 boost::filesystem::path& pParent);

  int SaveOneXosc(sTagSimuTraffic& scene, std::string strFileName, const std::string& strID,
                  boost::filesystem::path& pParent);

  void setFilePathName(std::string strName) { m_strFilePathName = strName; }
  std::string getFilePathName() { return m_strFilePathName; }

 public:
  CSimulationParam m_simParam;
  CTrafficParam m_traParam;
  CSensors m_sensors;
  CEnvironment m_environment;
  CGrading m_grading;

  SimuTrafficVec m_simTraffics;

  std::vector<std::string> m_filesnew;

 private:
  std::string m_strFilePathName;
};
