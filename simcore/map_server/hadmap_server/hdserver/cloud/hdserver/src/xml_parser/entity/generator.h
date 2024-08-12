/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "common/xml_parser/entity/environments/environment.h"
#include "parser.h"
#include "sensors/sensors.h"
#include "simulation_param.h"
#include "traffic_param.h"

struct sTagBatchSceneParam;

class CGenerator : public CParser {
 public:
  int Parse(const char* strSimuParamFile, const char* strTrafficParamFile);

  int Parse(const char* strSimName, sTagBatchSceneParam& param, const char* strParams);

  int GetSceneFileSize(const char* strSimName, uint64_t& ullFileSize);

  int Generate();

  int CalculateCount(int64_t& count, int64_t& size);

  int Generate(const char* strSimName, sTagBatchSceneParam& param, const char* strParams,
               std::string& strGeneratedDirName);
  int GenerateCount(const char* strSimName, sTagBatchSceneParam& param, uint64_t& llCount, uint64_t& llSize,
                    uint64_t& ullTime);

  int Save(const char* strSimuParamFile, const char* strTrafficParamFile);

  int BatchSave(const char* strPrefix, std::string& strGeneratedDirName);

 public:
  CSimulationParam m_simParam;
  CTrafficParam m_traParam;
  CSensors m_sensors;
  CEnvironment m_environment;
  CGrading m_grading;

  SimuTrafficVec m_simTraffics;

  std::vector<std::string> m_filesnew;
};
