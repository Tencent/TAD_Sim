/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "generator.h"
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <map>
#include "../../engine/config.h"
#include "../../engine/util/scene_util.h"
#include "../../engine/util/scene_validator.h"
#include "batch_scene_param.h"
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"
#include "common/utils/stringhelper.h"
#include "common/xml_parser/entity/parser_tools.h"
// #include "../../engine/engine.h"

int CGenerator::Parse(const char* strSimuParamFile, const char* strTrafficParamFile) {
  int nRet = m_simParam.Parse(strSimuParamFile);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed:%d!", strSimuParamFile, nRet);
    return -1;
  }

  nRet = m_traParam.Parse(strTrafficParamFile);
  if (nRet) {
    XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficParamFile);
    return -1;
  }

  return 0;
}

int CGenerator::Generate() {
  XML_LOGGER_INFO("start generate from template!");

  int nFile = 0;
  m_simTraffics.clear();

  typedef std::vector<CTraffic> TrafficVec;
  TrafficVec vecTraf;

  m_traParam.GenerateAllSinglsFactors();
  m_traParam.Generate();
  CTrafficParam::TrafficMap& traffics = m_traParam.GeneratedTraffics();
  CTrafficParam::TrafficMap::iterator itr = traffics.begin();
  for (; itr != traffics.end(); ++itr) {
    vecTraf.push_back(itr->second);
  }

  typedef std::vector<CSimulation> SimulationVec;
  SimulationVec vecSimu;
  CPlannerParam pp = m_simParam.m_plannerparam;
  pp.GenerateSingleEgoParam();
  int nCount = pp.m_paramsVec.size();

  for (size_t i = 0; i < nCount; i++) {
    if (pp.m_paramsVec.at(i).size() > 0) {
      CSimulation s(m_simParam);
      s.m_planner = pp;
      int nIndexEgo = 0;
      s.m_planner.m_strStartVelocity = pp.m_paramsVec.at(i).at(0);
      for (auto& it : s.m_planner.m_egos) {
        it.second.m_strStartVelocity = pp.m_paramsVec.at(i).at(nIndexEgo);
      }
      vecSimu.push_back(s);
    }
  }

  SimulationVec::iterator simuItr = vecSimu.begin();
  for (; simuItr != vecSimu.end(); ++simuItr) {
    TrafficVec::iterator trafItr = vecTraf.begin();
    for (; trafItr != vecTraf.end(); ++trafItr) {
      sTagSimuTraffic st;
      st.m_simulation = (*simuItr);
      st.m_traffic = (*trafItr);
      if (st.m_simulation.m_strSensor.size() > 0) {
        st.m_sensors = m_sensors;
      }
      if (st.m_simulation.m_strEnvironment.size() > 0) {
        st.m_environment = m_environment;
      }
      if (st.m_simulation.m_strGrading.size() > 0) {
        st.m_grading = m_grading;
      }

      m_simTraffics.push_back(st);
    }
  }

  return 0;
}

int CGenerator::CalculateCount(int64_t& count, int64_t& size) {
  XML_LOGGER_INFO("start calculate!");

  int nFile = 0;
  m_simTraffics.clear();

  m_traParam.GenerateAllSinglsFactors();

  int64_t llCount, llSize = 0;
  m_traParam.CalculateGeneratedTraffCount(llCount, llSize);

  typedef std::vector<CSimulation> SimulationVec;
  SimulationVec vecSimu;

  CPlannerParam pp = m_simParam.m_plannerparam;

  int nVelocity = pp.GenerateSingleEgoParam();

  count = nVelocity * llCount;
  size = llSize;

  return 0;
}

int CGenerator::Save(const char* strSimuParamFile, const char* strTrafficParamFile) {
  if (!DirectoryExist("result")) {
    CParser::CreateDir("result");
  }

  XML_LOGGER_INFO("start save generated files!");

  std::string strSaveSimuName = strSimuParamFile;
  std::vector<std::string> strs;
  std::string strSep = ".";
  CParseTools::SplitString(strSaveSimuName, strs, strSep);
  strSaveSimuName = strs[0];

  strs.clear();
  std::string strSaveTrafficName = strTrafficParamFile;
  CParseTools::SplitString(strSaveTrafficName, strs, strSep);
  strSaveTrafficName = strs[0];

  int nFile = 0;

  SimuTrafficVec::iterator itr = m_simTraffics.begin();
  for (; itr != m_simTraffics.end(); ++itr) {
    char strID[16] = {0};
    sprintf(strID, "%d", nFile);
    nFile++;

    std::string strFileNameNew = strSaveTrafficName;

    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append(".xml");

    std::string strSaveName = "result/" + strFileNameNew;

    itr->m_traffic.Save(strSaveName.c_str());

    itr->m_simulation.m_strTraffic = strFileNameNew;

    strFileNameNew = strSaveSimuName;
    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append(".sim");

    strSaveName = "result/" + strFileNameNew;

    itr->m_simulation.Save(strSaveName.c_str());
  }

  return 0;
}

int CGenerator::BatchSave(const char* strPrefix, std::string& strGeneratedDirName) {
  boost::filesystem::path p = CEngineConfig::Instance().ResourceDir();
  // boost::filesystem::path p = "";
  // 批量场景的生成目录
  p.append("/batch_scene");
  if (!boost::filesystem::exists(p)) {
    boost::filesystem::create_directory(p);
  }

  // 该次批量场景的生成目录
  std::string strPrefixDate = strPrefix;
  strPrefixDate.append("_");
  std::string strDate = CUtils::getTime();
  strPrefixDate.append(strDate);

  p /= strPrefixDate;
  if (!boost::filesystem::exists(p)) {
    boost::filesystem::create_directory(p);
  }

  // strGeneratedDirName = p.string();
  strGeneratedDirName = strPrefixDate;

  XML_LOGGER_INFO("start save generated files!");

  boost::filesystem::path file = strPrefixDate;
  file = file.stem();

  // 清空文件列表
  m_filesnew.clear();

  int nFile = 0;
  SimuTrafficVec::iterator itr = m_simTraffics.begin();
  for (; itr != m_simTraffics.end(); ++itr) {
    char strID[16] = {0};
    sprintf(strID, "%d", nFile);
    nFile++;

    // 保存sensor文件
    if (itr->m_simulation.m_strSensor.size() > 0) {
      std::string strFileNameNew = file.string();

      strFileNameNew.append("_");
      strFileNameNew.append(strID);
      strFileNameNew.append("_sensor.xml");

      boost::filesystem::path sensorPath = p;
      sensorPath.append(strFileNameNew);
      itr->m_sensors.Save(sensorPath.string().c_str());
      itr->m_simulation.m_strSensor = strFileNameNew;
    }

    // 保存environment文件
    if (itr->m_simulation.m_strEnvironment.size() > 0) {
      std::string strFileNameNew = file.string();

      strFileNameNew.append("_");
      strFileNameNew.append(strID);
      strFileNameNew.append("_environment.xml");

      boost::filesystem::path environmentPath = p;
      environmentPath.append(strFileNameNew);
      itr->m_environment.Save(environmentPath.string().c_str());
      itr->m_simulation.m_strEnvironment = strFileNameNew;
    }

    // 保存grading文件
    if (itr->m_simulation.m_strGrading.size() > 0) {
      std::string strFileNameNew = file.string();

      strFileNameNew.append("_");
      strFileNameNew.append(strID);
      strFileNameNew.append("_grading.xml");

      boost::filesystem::path gradingPath = p;
      gradingPath /= strFileNameNew;
      itr->m_grading.Save(gradingPath.string().c_str());
      itr->m_simulation.m_strGrading = strFileNameNew;
    }

    // 保存traffic文件
    std::string strFileNameNew = file.string();

    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append("_traffic.xml");

    boost::filesystem::path traPath = p;
    traPath.append(strFileNameNew);
    itr->m_traffic.Save(traPath.string().c_str());

    // 保存sim文件
    itr->m_simulation.m_strTraffic = strFileNameNew;

    strFileNameNew = file.string();
    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append(".sim");

    boost::filesystem::path simPath = p;
    simPath.append(strFileNameNew);

    // 判断文件是否已经存在
    bool bExist = false;
    if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
      bExist = true;
    }

    if (!bExist) {
      m_filesnew.push_back(simPath.string());
    }

    itr->m_simulation.Save(simPath.string().c_str());
  }

  return 0;
}

int CGenerator::Parse(const char* strFileName, sTagBatchSceneParam& param, const char* strParams) {
  if (!CSceneValidator::SceneExist(strFileName)) {
    SYSTEM_LOGGER_ERROR("file %s not exist!", strFileName);
    return -1;
  }

  std::string strFullPath = CSceneUtil::SceneFullPath(strFileName);

  // simulation
  int nRet = m_simParam.Parse(strFullPath.c_str());
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strFileName);
    return -1;
  }

  m_simParam.m_plannerparam = m_simParam.m_planner;
  // vecloity
  for (const auto& it : param.m_ego) {
    sEgoVelocity _vecloity;
    _vecloity.m_fStartVelocityStart = it.second.m_ego_velocity_start;
    _vecloity.m_fStartVelocityEnd = it.second.m_ego_velocity_end;
    _vecloity.m_fStartVelocitySep = it.second.m_ego_velocity_sep;
    std::string str_key = utils::FromatNumberToString(std::atoi(it.first.c_str()), "Ego_");
    m_simParam.m_plannerparam.m_mapEgoGroup2Velocity[str_key] = _vecloity;
  }

  // generate info
  m_simParam.m_generateInfo.m_strOriginFile = strFileName;
  m_simParam.m_generateInfo.m_strGenerateInfo = strParams;

  // info
  m_simParam.m_strInfo = param.m_strInfo;

  // traffic
  if (m_simParam.m_strTraffic.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strTraffic;

    std::string strTrafficFile = p.lexically_normal().make_preferred().string();
    // 解析原始文件
    nRet = m_traParam.Parse(strTrafficFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficFile.c_str());
      return -1;
    }

    // 移除源文件的条件
    m_traParam.SetToDefault();

    // 设置参数
    m_traParam.SetParam(param);
  }

  // sensors
  if (m_simParam.m_strSensor.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strSensor;

    std::string strSensorFile = p.lexically_normal().make_preferred().string();
    nRet = m_sensors.Parse(strSensorFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("sensor file %s parse failed!", strSensorFile.c_str());
      return -1;
    }
  }

  // environment
  if (m_simParam.m_strEnvironment.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strEnvironment;

    std::string strEnvironmentFile = p.lexically_normal().make_preferred().string();
    nRet = m_environment.Parse(strEnvironmentFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("environment file %s parse failed!", strEnvironmentFile.c_str());
      return -1;
    }
  }

  // grading
  if (m_simParam.m_strGrading.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strGrading;

    std::string strGradingFile = p.lexically_normal().make_preferred().string();
    nRet = m_grading.Parse(strGradingFile.c_str());
    if (nRet) {
      XML_LOGGER_ERROR("grading file %s parse failed!", strGradingFile.c_str());
      return -1;
    }
  }

  return 0;
}

int CGenerator::GetSceneFileSize(const char* strFileName, uint64_t& ullSingleSize) {
  if (!CSceneValidator::SceneExist(strFileName)) {
    SYSTEM_LOGGER_ERROR("file %s not exist!", strFileName);
    return -1;
  }

  ullSingleSize = 0;
  std::string strFullPath = CSceneUtil::SceneFullPath(strFileName);
  ullSingleSize = CSceneUtil::FileSize(strFullPath.c_str());

  // simulation
  int nRet = m_simParam.Parse(strFullPath.c_str());
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strFileName);
    return -1;
  }

  // traffic
  if (m_simParam.m_strTraffic.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strTraffic;

    std::string strTrafficFile = p.lexically_normal().make_preferred().string();
    ullSingleSize += CSceneUtil::FileSize(strTrafficFile.c_str());
  }

  // sensors
  if (m_simParam.m_strSensor.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strSensor;

    std::string strSensorFile = p.lexically_normal().make_preferred().string();
    ullSingleSize += CSceneUtil::FileSize(strSensorFile.c_str());
  }

  // environment
  if (m_simParam.m_strEnvironment.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strEnvironment;

    std::string strEnvironmentFile = p.lexically_normal().make_preferred().string();
    ullSingleSize += CSceneUtil::FileSize(strEnvironmentFile.c_str());
  }

  // grading
  if (m_simParam.m_strGrading.size() > 0) {
    boost::filesystem::path p = CEngineConfig::Instance().SceneDir();
    p /= m_simParam.m_strGrading;

    std::string strGradingFile = p.lexically_normal().make_preferred().string();
    ullSingleSize += CSceneUtil::FileSize(strGradingFile.c_str());
  }

  return 0;
}

int CGenerator::Generate(const char* strSimName, sTagBatchSceneParam& param, const char* strParams,
                         std::string& strGeneratedDirName) {
  int nRet = Parse(strSimName, param, strParams);

  if (nRet != 0) {
    return nRet;
  }

  nRet = Generate();
  if (nRet != 0) {
    return nRet;
  }

  nRet = BatchSave(param.m_strPrefix.c_str(), strGeneratedDirName);

  return nRet;
}

int CGenerator::GenerateCount(const char* strSimName, sTagBatchSceneParam& param, uint64_t& ullCount, uint64_t& ullSize,
                              uint64_t& ullTime) {
  ullCount = 0;
  ullSize = 0;
  ullTime = 0;

  uint64_t ullSingleSize = 0;
  int nRet = GetSceneFileSize(strSimName, ullSingleSize);
  if (nRet != 0) {
    return -1;
  }

  nRet = Parse(strSimName, param, "calculate");
  if (nRet != 0) {
    return -1;
  }

  uint64_t ullTrafficCount = 0;

  m_traParam.CalculateAllSinglesFactors(param, ullTrafficCount);

  CPlannerParam pp = m_simParam.m_plannerparam;

  int nVelocity = pp.GenerateSingleEgoParam();

  uint64_t ullSingleTime = 3;
  ullCount = nVelocity * ullTrafficCount;
  ullSize = ullCount * ullSingleSize;
  ullTime = ullCount * ullSingleTime;

  return nRet;
}
