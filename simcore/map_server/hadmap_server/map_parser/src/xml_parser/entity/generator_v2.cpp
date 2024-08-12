/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "generator_v2.h"
#include <json/value.h>
#include <json/writer.h>
#include <boost/algorithm/string.hpp>
#include <boost/thread.hpp>
#include "../../engine/config.h"
#include "../../engine/util/scene_util.h"
#include "../../engine/util/scene_validator.h"
#include "../database/db.h"
#include "../database/entity/scenario.h"
#include "../database/entity/scenario_set.h"
#include "batch_scene_param.h"
#include "common/engine/constant.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/log/xml_logger.h"
#include "common/utils/stringhelper.h"
#include "common/xml_parser/entity/parser_tools.h"
#include "common/xml_parser/xosc/xosc_writer_1_0_v4.h"
// #include "../../engine/engine.h"

bool CGeneratorV2::isStopGenScenesProgess = false;
double CGeneratorV2::dGenScenesProgessVal = 0.0;

int CGeneratorV2::Parse(const char* strSimuParamFile, const char* strTrafficParamFile) {
  int nRet = m_simParam.Parse(strSimuParamFile);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strSimuParamFile);
    return -1;
  }

  nRet = m_traParam.Parse(strTrafficParamFile);
  if (nRet) {
    XML_LOGGER_ERROR("traffic file %s parse failed!", strTrafficParamFile);
    return -1;
  }

  return 0;
}

int CGeneratorV2::Generate() {
  XML_LOGGER_INFO("start generate from template!");

  int nFile = 0;
  m_simTraffics.clear();

  typedef std::vector<CTraffic> TrafficVec;
  TrafficVec vecTraf;

  m_traParam.GenerateAllSinglsFactors();
  m_traParam.GenerateV2();
  CTrafficParam::TrafficList& traffics = m_traParam.GeneratedTrafficsV2();
  for (auto gt : traffics) {
    vecTraf.push_back(gt);
  }

  typedef std::vector<CSimulation> SimulationVec;
  SimulationVec vecSimu;

  CPlannerParam pp = m_simParam.m_plannerparam;
  pp.m_egos = m_simParam.m_planner.m_egos;
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
        nIndexEgo++;
      }
      vecSimu.push_back(s);
    }
  }

  SimulationVec::iterator simuItr = vecSimu.begin();
  for (; simuItr != vecSimu.end(); ++simuItr) {
    if (vecTraf.size() > 0) {
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
    } else {
      sTagSimuTraffic st;
      st.m_simulation = (*simuItr);
      st.m_traffic = m_traParam;
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

int CGeneratorV2::CalculateCount(int64_t& count, int64_t& size) {
  XML_LOGGER_INFO("start calculate!");

  int nFile = 0;
  m_simTraffics.clear();

  m_traParam.GenerateAllSinglsFactors();

  int64_t llCount, llSize = 0;

  m_traParam.CalculateGeneratedTraffCount(llCount, llSize);

  typedef std::vector<CSimulation> SimulationVec;
  SimulationVec vecSimu;

  CPlannerParam pp = m_simParam.m_plannerparam;
  pp.GenerateSingleEgoParam();
  int nVelocity = pp.m_paramsVec.size();
  count = nVelocity * llCount;
  size = llSize;
  return 0;
}

int CGeneratorV2::Save(const char* strSimuParamFile, const char* strTrafficParamFile) {
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

int CGeneratorV2::SaveOneSim(sTagSimuTraffic& scene, std::string strFileName, const std::string& strID,
                             boost::filesystem::path& pParent) {
  // 保存sensor文件
  if (scene.m_simulation.m_strSensor.size() > 0) {
    std::string strFileNameNew = strFileName;

    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append("_sensor.xml");

    boost::filesystem::path sensorPath = pParent;
    sensorPath.append(strFileNameNew);
    scene.m_sensors.Save(sensorPath.string().c_str());
    scene.m_simulation.m_strSensor = strFileNameNew;
  }

  // 保存environment文件
  if (scene.m_simulation.m_strEnvironment.size() > 0) {
    std::string strFileNameNew = strFileName;

    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append("_environment.xml");

    boost::filesystem::path environmentPath = pParent;
    environmentPath.append(strFileNameNew);
    scene.m_environment.Save(environmentPath.string().c_str());
    scene.m_simulation.m_strEnvironment = strFileNameNew;
  }

  // 保存grading文件
  if (scene.m_simulation.m_strGrading.size() > 0) {
    std::string strFileNameNew = strFileName;

    strFileNameNew.append("_");
    strFileNameNew.append(strID);
    strFileNameNew.append("_grading.xml");

    boost::filesystem::path gradingPath = pParent;
    gradingPath /= strFileNameNew;
    scene.m_grading.Save(gradingPath.string().c_str());
    scene.m_simulation.m_strGrading = strFileNameNew;
  }

  // 保存traffic文件
  std::string strFileNameNew = strFileName;

  strFileNameNew.append("_");
  strFileNameNew.append(strID);
  strFileNameNew.append("_traffic.xml");

  boost::filesystem::path traPath = pParent;
  traPath.append(strFileNameNew);
  scene.m_traffic.Save(traPath.string().c_str());
  //

  // 保存sim文件
  scene.m_simulation.m_strTraffic = strFileNameNew;

  strFileNameNew = strFileName;
  strFileNameNew.append("_");
  strFileNameNew.append(strID);
  strFileNameNew.append(".sim");

  boost::filesystem::path simPath = pParent;
  simPath.append(strFileNameNew);

  // 判断文件是否已经存在
  bool bExist = false;
  if (boost::filesystem::exists(simPath) && boost::filesystem::is_regular_file(simPath)) {
    bExist = true;
  }

  if (!bExist) {
    m_filesnew.push_back(simPath.string());
  }
  scene.m_path = simPath.string().c_str();
  scene.m_simulation.Save(simPath.string().c_str());

  return 0;
}

int CGeneratorV2::SaveOneXosc(sTagSimuTraffic& scene, std::string strFileName, const std::string& strID,
                              boost::filesystem::path& pParent) {
  XOSCWriter_1_0_v4 writer;
  boost::filesystem::path xoscPath = pParent;
  std::string strXoscName = strFileName;
  strXoscName.append("_");
  strXoscName.append(strID);
  strXoscName.append(".xosc");
  xoscPath /= (strXoscName);

  boost::filesystem::path mapPath = scene.m_simulation.m_mapFile.m_strMapfile;
  scene.m_simulation.m_mapFile.m_strMapfile = mapPath.filename().string();
  scene.m_path = xoscPath.string().c_str();
  scene.m_defaultXosc = CEngineConfig::Instance().getCataLogDir();
  return writer.Save2XOSC(xoscPath.string().c_str(), scene);
}

int CGeneratorV2::BatchSave(sTagBatchSceneParamV2& param, std::string& strGeneratedDirName) {
  boost::filesystem::path p = CEngineConfig::Instance().getSceneDataDir();  // CEngineConfig::Instance().ResourceDir();
  // boost::filesystem::path p = "";
  // 批量场景的生成目录
  p.append("/batch_scene");
  if (!boost::filesystem::exists(p)) {
    boost::filesystem::create_directory(p);
  }

  // 该次批量场景的生成目录
  std::string strPrefixDate = param.m_strPrefix;
  strPrefixDate.append("_");
  std::string strDate = CUtils::getTime();
  strPrefixDate.append(strDate);

  p /= strPrefixDate;
  if (!boost::filesystem::exists(p)) {
    boost::filesystem::create_directory(p);
  }
  // 获取场景集名称
  std::string strScenarioSetName = p.filename().string();
  strGeneratedDirName = strPrefixDate;

  XML_LOGGER_INFO("start save generated files!");

  boost::filesystem::path file = strPrefixDate;
  file = file.stem();

  // 清空文件列表
  m_filesnew.clear();
  isStopGenScenesProgess = false;
  dGenScenesProgessVal = 0.0;

  return 0;
}

int CGeneratorV2::BatchSaveOne(sTagBatchSceneParamV2& param, std::string& strGeneratedDirName, int simTrafficsIndex) {
  // 获取源目录
  boost::filesystem::path p = CEngineConfig::Instance().getSceneDataDir();  // CEngineConfig::Instance().ResourceDir();
  // 批量场景的生成目录
  p.append("/batch_scene");
  p /= strGeneratedDirName;

  sTagSimuTraffic& simuTraffic = m_simTraffics.at(simTrafficsIndex);
  if (isStopGenScenesProgess) {
    return -2;
  }
  char strID[16] = {0};
  sprintf(strID, "%.1f", static_cast<double>(simTrafficsIndex));
  SYSTEM_LOGGER_DEBUG("strID", strID);
  int numall = m_simTraffics.size();
  if (param.m_file_type == SCENE_TYPE_SIM) {
    SaveOneSim(simuTraffic, strGeneratedDirName, strID, p);
  } else if (param.m_file_type == SCENE_TYPE_XOSC) {
    SaveOneXosc(simuTraffic, strGeneratedDirName, strID, p);
  }
  dGenScenesProgessVal = simTrafficsIndex / static_cast<double>(numall);
  SYSTEM_LOGGER_DEBUG("dGenScenesProgessVal: %.1f", dGenScenesProgessVal);
  return 0;
}

int CGeneratorV2::Parse(const char* strFileName, sTagBatchSceneParamV2& param, const char* strParams) {
  std::string strFullPath = CSceneUtil::GetSceneFullPath(strFileName, CSceneUtil::ALL_PATH);

  if (!m_strFilePathName.empty()) strFullPath = m_strFilePathName;
  if (!CSceneValidator::SceneExist(strFullPath.c_str())) {
    SYSTEM_LOGGER_ERROR("File %s not exist!", strFullPath.c_str());
    return -1;
  }
  // simulation
  int nRet = m_simParam.Parse(strFullPath.c_str(), false);
  if (nRet) {
    XML_LOGGER_ERROR("simulation file %s parse failed!", strFileName);
    return -1;
  }

  boost::filesystem::path parentPath = strFullPath;
  parentPath = parentPath.parent_path();
  m_simParam.m_plannerparam = m_simParam.m_planner;

  // vecloity
  for (auto it : param.m_ego) {
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
    boost::filesystem::path p = parentPath.string();
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
    m_traParam.SetParamV2(param);
  }

  // sensors
  if (m_simParam.m_strSensor.size() > 0) {
    boost::filesystem::path p = parentPath.string();
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
    boost::filesystem::path p = parentPath.string();
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
    boost::filesystem::path p = parentPath.string();
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

int CGeneratorV2::ParseXosc(const char* strFileName, sTagBatchSceneParamV2& param, const char* strParams) {
  std::string strFullPath = CSceneUtil::GetSceneFullPath(strFileName, CSceneUtil::ALL_PATH);
  if (!m_strFilePathName.empty()) {
    strFullPath = m_strFilePathName;
  }
  bool flag_exist = CSceneValidator::SceneExist(strFullPath.c_str());
  if (false == flag_exist) {
    SYSTEM_LOGGER_ERROR("file %s not exist!", strFullPath.c_str());
    return -1;
  }
  CParser parser;
  sTagSimuTraffic sst;
  boost::filesystem::path p = strFullPath;
  boost::filesystem::path parent = p.parent_path();
  // set parse xosc depend on path
  sst.m_confPath = CEngineConfig::Instance().getConfPath();
  sst.m_defaultXosc = CEngineConfig::Instance().getCataLogDir();
  sst.m_vehicleXosc = CEngineConfig::Instance().getVehicle();
  int nRes = parser.Parse(sst, strFullPath.c_str(), parent.string().c_str(), false);

  // simulation
  m_simParam = sst.m_simulation;
  for (auto it : param.m_ego) {
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
  // if (m_simParam.m_strTraffic.size() > 0)
  {
    m_traParam = sst.m_traffic;

    // 移除源文件的条件
    m_traParam.SetToDefault();

    // 设置参数
    m_traParam.SetParamV2(param);
  }

  return 0;
}

int CGeneratorV2::GetSceneFileSize(int nType, const char* strFileName, uint64_t& ullSingleSize) {
  ullSingleSize = 0;
  std::string strFullPath = CSceneUtil::GetSceneFullPath(strFileName, CSceneUtil::ALL_PATH);
  if (!m_strFilePathName.empty()) {
    strFullPath = m_strFilePathName;
    SYSTEM_LOGGER_INFO("GetSceneFileSize fullpath = %s", strFullPath.c_str());
  }
  if (!CSceneValidator::SceneExist(strFullPath.c_str())) {
    SYSTEM_LOGGER_ERROR("File %s not exist!", strFullPath.c_str());
    return -1;
  }
  ullSingleSize = CSceneUtil::FileSize(strFullPath.c_str());

  if (nType == SCENE_TYPE_XOSC) {
  } else if (nType == SCENE_TYPE_SIM) {
    // simulation
    int nRet = m_simParam.Parse(strFullPath.c_str(), false);
    if (nRet) {
      XML_LOGGER_ERROR("Simulation file %s parse failed!", strFileName);
      return -2;
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

  } else {
  }
  return 0;
}

int CGeneratorV2::Generate(const char* strSimName, const char* strSourceType, sTagBatchSceneParamV2& param,
                           const char* strParams, std::string& strGeneratedDirName) {
  int nRet = 0;
  if (boost::iequals(strSourceType, "xosc")) {
    std::string strName = strSimName;
    strName.append(".xosc");
    nRet = ParseXosc(strName.c_str(), param, strParams);

    if (nRet != 0) {
      return nRet;
    }

  } else {
    std::string strName = strSimName;
    if (strName.find(".sim") == std::string::npos) strName.append(".sim");
    nRet = Parse(strName.c_str(), param, strParams);

    if (nRet != 0) {
      return nRet;
    }
  }

  nRet = Generate();
  if (nRet != 0) {
    return nRet;
  }

  nRet = BatchSave(param, strGeneratedDirName);

  return nRet;
}

int CGeneratorV2::GenerateCount(const char* strSimName, sTagBatchSceneParamV2& param, uint64_t& ullCount,
                                uint64_t& ullSize, uint64_t& ullTime) {
  ullCount = 0;
  ullSize = 0;
  ullTime = 0;

  uint64_t ullSingleSize = 0;
  int nRet = 0;
  boost::filesystem::path p = strSimName;
  std::string strExt = p.extension().string();
  int nSourceType = SCENE_TYPE_SIM;
  if (boost::iequals(strExt, ".sim")) {
    nSourceType = SCENE_TYPE_SIM;
  } else {
    nSourceType = SCENE_TYPE_XOSC;
  }

  nRet = GetSceneFileSize(nSourceType, strSimName, ullSingleSize);

  if (nRet != 0) {
    SYSTEM_LOGGER_INFO("GetSceneFileSize nRet = %d", nRet);
    return -1;
  }

  if (nSourceType == SCENE_TYPE_XOSC) {
    nRet = ParseXosc(strSimName, param, "calculate");
    if (nRet != 0) {
      return -2;
    }
  } else {
    nRet = Parse(strSimName, param, "calculate");
    if (nRet != 0) {
      return -3;
    }
  }

  uint64_t ullTrafficCount = 0;

  m_traParam.CalculateAllSinglesFactorsV2(param, ullTrafficCount);

  CPlannerParam pp = m_simParam.m_plannerparam;
  int nVelocity = pp.GenerateCount();

  uint64_t ullSingleTime = 4;
  ullCount = nVelocity * ullTrafficCount;
  ullSize = ullCount * ullSingleSize;
  ullTime = ullCount * ullSingleTime;

  return nRet;
}
