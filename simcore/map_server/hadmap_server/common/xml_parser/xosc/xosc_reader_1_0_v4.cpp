/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc/xosc_reader_1_0_v4.h"
#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include "IXMLValidation.h"
#include "common/engine/math/utils.h"
#include "common/log/system_logger.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/entity/traffic.h"
#include "tinyxml2.h"
#ifdef __CLOUD_HADMAP_SERVER__
#  include "common/utils/xsd_validator.h"
#endif
#include "common/xml_parser/entity/catalogbase.h"
#include "engine/config.h"
#include "xml_parser/entity/parser.h"
#include "xml_parser/entity/simulation.h"
XOSCReader_1_0_v4::XOSCReader_1_0_v4() {}

XOSCReader_1_0_v4::~XOSCReader_1_0_v4() {}

int XOSCReader_1_0_v4::ParseXOSC(const char* strFileName, sTagSimuTraffic& infos) {
  std::string strFilePath = strFileName;

  ParameterDeclarationPreprocess(strFilePath);

  SYSTEM_LOGGER_INFO("start to parse xosc file %s", strFileName);

  Reset();
  m_filePath = strFileName;
  infos.m_confPath = CEngineConfig::Instance().getConfPath();
  infos.m_defaultXosc = CEngineConfig::Instance().getCataLogDir();
  infos.m_vehicleXosc = CEngineConfig::Instance().getVehicle();
  if (!infos.m_confPath.empty()) {
    this->LoadConf(infos.m_confPath.c_str());
  }
  try {
    tinyxml2::XMLDocument xml_doc;
    tinyxml2::XMLError eResult = xml_doc.LoadFile(strFileName);
    if (eResult != tinyxml2::XML_SUCCESS) return -1;
    auto OpenSCENARIO = xml_doc.FirstChildElement("OpenSCENARIO");
    if (OpenSCENARIO == nullptr) return -2;
    osc::__parse__OpenScenario(OpenSCENARIO, pScene);
    auto& fht = pScene.sub_FileHeader;
    SYSTEM_LOGGER_INFO("ParseFileHeader");
    ParseFileHeader(*fht, infos);
    // parameter declaration
    SYSTEM_LOGGER_INFO("ParameterDeclarations");
    auto& pdo = pScene.sub_ParameterDeclarations;
    if (pdo != NULL) {
      ParseParameterDeclaration(*pdo);
      for (auto pd : pdo->sub_ParameterDeclaration) {
        CSimulation::ParameterDeclaration one;
        one.name = *pd._name;
        one.parameterType = *pd._parameterType;
        one.value = *pd._value;
        infos.m_simulation.m_parameterDeclarations.emplace_back(one);
      }
    }
    // catalog location
    // SYSTEM_LOGGER_INFO("CatalogLocations");
    // auto& clo =  pScene.sub_CatalogLocations;
    // if (clo!=NULL) {
    //  ParseCatalogLocations(*clo, infos);
    // }
    SYSTEM_LOGGER_INFO("RoadNetwork");
    // road network
    auto& rno = pScene.sub_RoadNetwork;
    if (rno != NULL) {
      ParseRoadNetWork(*rno, infos);
    }
    SYSTEM_LOGGER_INFO("Entities");
    // entities
    auto& entities = pScene.sub_Entities;
    if (entities != NULL) {
      ParseEntities(*entities, infos);
    }
    // storyboard
    SYSTEM_LOGGER_INFO("Storyboard");
    auto& sbo = pScene.sub_Storyboard;
    if (sbo != NULL) {
      ParseStoryboard(*sbo, infos);
    }
    // StrName2ID(infos);
    FlushData(infos);
  } catch (std::exception& e) {
#ifdef __CLOUD_HADMAP_SERVER__
    auto status =
        utils::XsdValidator::Instance().Validate(strFilePath, utils::XsdValidator::XsdType::kOpenScenario_1_0);
    if (!status.ok()) {
      SYSTEM_LOGGER_ERROR("invalid osc, %s", status.ToString().c_str());
    }
#else
    if (!infos.m_oscXsdPath.empty()) {
      if (!CheckXosc(infos.m_oscXsdPath.c_str(), strFilePath.c_str())) {
        return -1;
      }
    }
#endif
    SYSTEM_LOGGER_ERROR("parse openscenario file error: %s", e.what());
    return -1;
  }
  return 0;
}

int XOSCReader_1_0_v4::CopyXOSC(const char* srcFileName, const char* dstFileName) {
  boost::filesystem::copy_file(srcFileName, dstFileName, BOOST_COPY_OPTION);
  return true;
}

int XOSCReader_1_0_v4::getXoscFilenames(const std::string& dir, std::vector<std::string>& filenames) {
  using namespace boost::filesystem;
  boost::filesystem::path path(dir);
  if (!boost::filesystem::exists(path)) {
    return -1;
  }
  boost::filesystem::directory_iterator end_iter;
  for (boost::filesystem::directory_iterator iter(path); iter != end_iter; ++iter) {
    if (boost::filesystem::is_regular_file(iter->status())) {
      if (iter->path().extension().string() == ".xosc") {
        filenames.push_back(iter->path().string());
      }
    }
  }
  return filenames.size();
}

int XOSCReader_1_0_v4::ParseManeuversCatalogXosc(const std::vector<std::string> maneuverName,
                                                 osc::xsd::Vector<osc::Maneuver>& maneuver) {
  boost::filesystem::path maneuverDir = this->m_catalogPath.strManeuverCatalog;
  if (!boost::filesystem::is_directory(maneuverDir)) {
    SYSTEM_LOGGER_ERROR("Maneuver Catalog File is not exiest");
    return -1;
  }
  try {
    std::vector<std::string> filenames;
    getXoscFilenames(maneuverDir.string(), filenames);
    for (auto it : filenames) {
      std::unique_ptr<osc::OpenScenario> pScene;
      // osc::__parse(it.c_str(), *pScene);
      /*OpenScenario::Catalog_optional& catalogOpt =  pScene.Catalog();
      if (catalogOpt) {
        Catalog::Maneuver_sequence& manuerVec = catalogOpt.Maneuver();
        for (auto itname : maneuverName) {
          bool flag = false;
          for (auto iter : manuerVec) {
            if (std::string(iter.name()) == itname) {
              maneuver.push_back(iter);
              flag = true;
              return 0;
            }
          }
          if (flag == false) {
            SYSTEM_LOGGER_ERROR("parse maneuver catalog error, not find maneuver name :%s", itname);
            return -1;
          }
        }
      }*/
    }
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_ERROR("parse maneuver openscenario file error: %s", e.what());
    return -1;
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseEnvironmentCatalogXosc(const std::string environmentName, osc::Environment& environment) {
  boost::filesystem::path environmentpath = this->m_catalogPath.strEnvironmentCatalog;
  if (!boost::filesystem::is_directory(environmentpath)) {
    SYSTEM_LOGGER_ERROR("Environment Catalog File is not exiest");
    return -1;
  }
  try {
    std::vector<std::string> filenames;
    getXoscFilenames(environmentpath.string(), filenames);
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_ERROR("parse maneuver openscenario file error: %s", e.what());
    return -1;
  }
}

int XOSCReader_1_0_v4::ParseRouteCatalogXosc(const std::string RouteName, osc::Route& route) {
  boost::filesystem::path RouteCatalogPath = this->m_catalogPath.strRouteCatalog;
  if (!boost::filesystem::is_directory(RouteCatalogPath)) {
    SYSTEM_LOGGER_ERROR("RouteCatalg File is not exiest");
    return -1;
  }
  try {
    std::vector<std::string> filenames;
    getXoscFilenames(RouteCatalogPath.string(), filenames);
    std::cout << "filenames size " << filenames.size();
    SYSTEM_LOGGER_DEBUG("find route catalog %d", filenames.size());
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_ERROR("parse route openscenario file error: %s", e.what());
    return -1;
  }
}

int XOSCReader_1_0_v4::ParseTrajectoryCataloggXosc(const std::string TrajectoryName, osc::Trajectory& tra) {
  boost::filesystem::path TrajectoryCatalogPath = this->m_catalogPath.strTrajectoryCatalog;
  if (!boost::filesystem::is_directory(TrajectoryCatalogPath)) {
    SYSTEM_LOGGER_ERROR("ParseTrajectoryCatalog File is not exiest");
    return -1;
  }
  try {
    std::vector<std::string> filenames;
    getXoscFilenames(TrajectoryCatalogPath.string(), filenames);
  } catch (const std::exception& e) {
    SYSTEM_LOGGER_ERROR("parse maneuver openscenario file error: %s", e.what());
    return -1;
  }
}
void XOSCReader_1_0_v4::Reset() {
  m_nVehiclIDGeneartor = 1;
  m_nObstacleIDGenerator = 1;
  m_nPedestrianIDGenerator = 1;
  m_nRouteIDGenerator = 0;
  m_nAccIDGeneator = 1;
  m_nMergeIDGenerator = 1;
  m_nPedVIDGenerator = 1;
  m_nVIDGenerator = 1;

  m_mapIniPos.clear();

  m_ParameterDeclarations.clear();

  m_mapObstacles.clear();
  m_mapVehicles.clear();
  m_mapRoutes.clear();
  m_mapAccs.clear();
  m_mapMerges.clear();
  m_mapPedestrians.clear();
  m_mapPedestrianVelocitys.clear();
  m_mapVelocitys.clear();
  m_mapTrafficSingals.clear();
  m_mapSceneEvents.clear();
  m_mapSimPlanerEvents.clear();
  m_isRouteAction = false;
  // default acc
  if (m_mapAccs.size() == 0) {
    CAcceleration acc;
    acc.m_strID = std::to_string(m_nAccIDGeneator++);
    acc.SetToDefault();
    m_mapAccs.insert(std::make_pair(acc.m_strID, acc));
  }
  if (m_mapMerges.size() == 0) {
    CMerge m;
    m.m_strID = std::to_string(m_nMergeIDGenerator++);
    m.SetToDefault();
    m_mapMerges.insert(std::make_pair(m.m_strID, m));
  }
}
void XOSCReader_1_0_v4::LoadConf(const char* fileName) {
  SYSTEM_LOGGER_INFO("start to load Conf");
  m_conLoadPtr = std::make_shared<CConfLoad>();
  bool ret = m_conLoadPtr->ReadConfig(fileName);
  if (ret == false) {
    m_conLoadPtr.reset();
    m_conLoadPtr = nullptr;
    SYSTEM_LOGGER_ERROR("ReadConfig is Error,cfg = %s", fileName);
    return;
  }
}
bool XOSCReader_1_0_v4::CheckXosc(const char* xsdPath, const char* xoscPath) {
  // get xsd string
  std::ifstream inf;
  std::string s;
  std::string xsdStr;
  inf.open(xsdPath);
  while (std::getline(inf, s)) {
    xsdStr += s;
    xsdStr += '\n';
  }
  IXMLValidation xmlValid(xsdStr);
  // get xosc string
  std::ifstream of2(xoscPath);
  std::string xmlStr;
  while (getline(of2, s)) {
    xmlStr += s;
    xmlStr += '\n';
  }
  of2.close();
  std::string errMsg;
  int ret = xmlValid.validation(xmlStr, errMsg);
  if (ret) {
    std::cout << errMsg.c_str();
    SYSTEM_LOGGER_ERROR("validation fail: %s", errMsg.c_str());
    return false;
  } else {
    SYSTEM_LOGGER_INFO("validation success.");
    return true;
  }
}
std::string XOSCReader_1_0_v4::MatchName(std::string type, std::string name) {
  if (m_conLoadPtr != NULL) {
    if (type == "Ego") {
      std::string strname;
      if (EgoTypeStr2Value(name) != ET_INVALID) {
        strname = name;
      } else {
        strname = m_conLoadPtr->ReadString("Ego", name.c_str(), "suv");
      }
      return strname;
    } else if (type == "VehicleCatalog") {
      std::string strname;
      if (CVehicle::IsVehicleType(name)) {
        strname = name;
      } else {
        strname = m_conLoadPtr->ReadString("Veh", name.c_str(), "SUV");
      }
      return strname;
    } else if (type == "MiscObjectCatalog") {
      CObstacle tmp;
      return m_conLoadPtr->ReadString("MiscObj", name.c_str(), "Stob_001");
    } else {
      if (MakeSure_P(name)) {
        return name;
      }
      return m_conLoadPtr->ReadString("Ped", name.c_str(), "human");
    }
  } else {
    if (type == "Ego") {
      return "suv";
    } else if (type == "VehicleCatalog") {
      return "SUV";
    } else if (type == "MiscObjectCatalog") {
      return "Stob_001";
    } else {
      return "human";
    }
  }
}

void XOSCReader_1_0_v4::GetCatalogReference(osc::ScenarioObject& scenrioObject, sTagSimuTraffic& infos) {
  std::string strName = *scenrioObject._name;
  std::string catalogName;  //= scenrioObject->sub_CatalogReference.catalogName();
  std::string entryName;    // = scenrioObject->sub_CatalogReference._entryName;
  std::string strMaxV = "0.0";
  if (catalogName == "VehicleCatalog") {
    std::string strType = this->MatchName(catalogName, entryName);
    bool isPedestrianType = MakeSure_P(strType);
    // bicycle
    if (isPedestrianType) {
      // this->m_name2standard[std::string(scenrioObject.name())] = "P_"
      CPedestrianV2 pa;
      pa.m_strName = *scenrioObject._name;
      pa.m_strID = std::to_string(m_nPedestrianIDGenerator++);
      pa.m_strBehavior = "UserDefine";
      pa.m_strType = this->MatchName(catalogName, entryName);
      pa.m_strStartTime = "0";
      pa.m_strEndTime = "1000";
      strMaxV = "1.0";
      pa.m_strStartVelocity = "0.0";
      pa.m_strMaxVelocity = strMaxV;
      m_mapPedestrians.insert(make_pair(pa.m_strName, pa));
    } else {
      strMaxV = "12.0";
      string strAggress = "-1.000", strFollow = "", strEventID = "", strTrajectoryAngle = "0",
             strTrajectoryTracking = "";
      string strBehavior = "UserDefine";
      CVehicle v;
      v.m_strName = *scenrioObject._name;
      v.m_strID = std::to_string(m_nVehiclIDGeneartor++);
      v.m_strType = strType;
      v.m_strStartTime = "0";
      v.m_strStartVelocity = "0";
      v.m_strMaxVelocity = strMaxV;
      v.m_strBehavior = strBehavior;
      if (!infos.m_vehicleXosc.empty()) {
        CCatalogModelPtr single;
        if (CCataLog::getCatalogName(infos.getCatalogDir().c_str(), CATALOG_TYPE_VEHICLE, v.m_strName, single) == 0) {
          v.m_strMaxVelocity = single->getPeformance().getMaxSpeed();
        }
        if (strAggress.size() > 0) v.m_strAggress = strAggress;
        if (strFollow.size() > 0) v.m_strFollow = strFollow;
        // if (strEventID.size() > 0) v.m_strEventId = strEventID;
        if (strTrajectoryAngle.size() > 0) v.m_strAngle = strTrajectoryAngle;
        if (strTrajectoryTracking.size() > 0)
          m_mapXoscTrajectoryTracking.insert(make_pair(strName, strTrajectoryTracking));
        m_mapVehicles.insert(std::make_pair(v.m_strName, v));
      }
    }
  } else if (catalogName == "PedestrianCatalog") {
    CPedestrianV2 pa;
    pa.m_strName = *scenrioObject._name;
    pa.m_strID = std::to_string(m_nPedestrianIDGenerator++);
    pa.m_strBehavior = "UserDefine";
    pa.m_strType = this->MatchName(catalogName, entryName);
    pa.m_strStartTime = "0";
    pa.m_strEndTime = "1000";
    strMaxV = "1.0";
    pa.m_strStartVelocity = "0.0";
    pa.m_strMaxVelocity = strMaxV;
    m_mapPedestrians.insert(make_pair(pa.m_strName, pa));
  } else if (catalogName == "MiscObjectCatalog") {
    CObstacle O;
    O.m_strName = *scenrioObject._name;
    O.m_strID = std::to_string(m_nObstacleIDGenerator++);
    if (!O.GetDefaultObstacle(entryName, infos.getMiscObjectXoscPath())) {
      O.m_strType = this->MatchName(catalogName, entryName);
    }
    m_mapObstacles.insert(make_pair(O.m_strName, O));
  }
}

void XOSCReader_1_0_v4::FlushData(sTagSimuTraffic& infos) {
  for (auto& i : m_mapVehicles) {
    if (i.second.m_strAccID.size() < 1) {
      i.second.m_strAccID = "1";
    }

    if (i.second.m_strMergeID.size() < 1) {
      i.second.m_strMergeID = "1";
    }

    // i.second.m_strAggress = "0.5";
  }
  for (auto& it : m_egoInfos) {
    infos.m_simulation.m_planner.m_egos[it.first] = it.second;
    if (m_mapSimPlanerEvents.find(it.first) != m_mapSimPlanerEvents.end())
      infos.m_simulation.m_planner.m_egos[it.first].m_scenceEvents = m_mapSimPlanerEvents[it.first];
  }
  infos.m_traffic.Accs() = m_mapAccs;
  infos.m_traffic.Merges() = m_mapMerges;
  infos.m_traffic.Routes() = m_mapRoutes;
  infos.m_traffic.Velocities() = m_mapVelocitys;
  infos.m_traffic.PedestrianVelocities() = m_mapPedestrianVelocitys;

  infos.m_traffic.Vehicles() = m_mapVehicles;
  infos.m_traffic.PedestriansV2() = m_mapPedestrians;
  infos.m_traffic.Obstacles() = m_mapObstacles;
  infos.m_traffic.Signlights() = m_mapTrafficSingals;
  infos.m_traffic.SceneEvents() = m_mapSceneEvents;

  CTraffic::VehicleMap m_mapTempVehicles;
  for (auto itr = m_mapVehicles.begin(); itr != m_mapVehicles.end(); itr++) {
    m_mapTempVehicles.insert(make_pair(itr->second.m_strID, itr->second));
  }
  infos.m_traffic.Vehicles() = m_mapTempVehicles;

  CTraffic::PedestrianV2Map m_mapTempPedestrians;
  for (auto itr = m_mapPedestrians.begin(); itr != m_mapPedestrians.end(); itr++) {
    m_mapTempPedestrians.insert(make_pair(itr->second.m_strID, itr->second));
  }
  infos.m_traffic.PedestriansV2() = m_mapTempPedestrians;

  CTraffic::ObstacleMap m_mapTempObstacles;
  for (auto itr = m_mapObstacles.begin(); itr != m_mapObstacles.end(); itr++) {
    m_mapTempObstacles.insert(make_pair(itr->second.m_strID, itr->second));
  }
  infos.m_traffic.Obstacles() = m_mapTempObstacles;
}

void XOSCReader_1_0_v4::StrName2ID(sTagSimuTraffic& infos) {
  for (auto iter = infos.m_traffic.m_mapVehicles.begin(); iter != infos.m_traffic.m_mapVehicles.end(); iter++) {
    iter->second.m_strID = iter->first;
  }
  for (auto iter = infos.m_traffic.m_mapObstacles.begin(); iter != infos.m_traffic.m_mapObstacles.end(); iter++) {
    iter->second.m_strID = iter->first;
  }
  for (auto iter = infos.m_traffic.m_mapPedestriansV2.begin(); iter != infos.m_traffic.m_mapPedestriansV2.end();
       iter++) {
    iter->second.m_strID = iter->first;
  }
  for (auto iter = infos.m_traffic.m_mapRoutes.begin(); iter != infos.m_traffic.m_mapRoutes.end(); iter++) {
    iter->second.m_strID = iter->first;
  }
  for (auto iter = infos.m_traffic.m_mapAccs.begin(); iter != infos.m_traffic.m_mapAccs.end(); iter++) {
    iter->second.m_strID = iter->first;
  }
  for (auto iter = infos.m_traffic.m_mapMerges.begin(); iter != infos.m_traffic.m_mapMerges.end(); iter++) {
    iter->second.m_strID = iter->first;
  }
}

int XOSCReader_1_0_v4::ParseStartTrigger(osc::Trigger& st, string& strEntityRef, int& nType, string& strTrigerValue,
                                         string& strDistanceType, string& strAlongRoute) {
  auto& cgs = st.sub_ConditionGroup;
  for (auto cg : cgs) {
    auto& cs = cg.sub_Condition;
    for (auto c : cs) {
      std::string strConditionName = *c._name;
      std::string strDelay = *c._delay;
      std::string strEdge = *c._conditionEdge;

      // by value condition time absolute
      if (c.sub_ByValueCondition) {
        if (c.sub_ByValueCondition->sub_SimulationTimeCondition) {
          strTrigerValue = *c.sub_ByValueCondition->sub_SimulationTimeCondition->_value;
          if (strTrigerValue[0] == '$') FindParameterDeclarationsValude(strTrigerValue);
          nType = TrigConditionType::TCT_TIME_ABSOLUTE;
        }
      }

      // by entity condition event
      if (c.sub_ByEntityCondition) {
        // ttc time
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition) {
          strTrigerValue = *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_value;

          if (strTrigerValue[0] == '$') FindParameterDeclarationsValude(strTrigerValue);

          strAlongRoute = *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_alongRoute;

          nType = TrigConditionType::TCT_TIME_RELATIVE;
        }

        // ttc relative distance
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition) {
          strTrigerValue = *c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition->_value;
          strDistanceType =
              *c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition->_relativeDistanceType;

          if (strTrigerValue[0] == '$') FindParameterDeclarationsValude(strTrigerValue);

          if (strDistanceType[0] == '$') FindParameterDeclarationsValude(strDistanceType);

          nType = TrigConditionType::TCT_POSITION_RELATIVE;
        }

        // ttc distance
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_DistanceCondition) {
          strTrigerValue = *c.sub_ByEntityCondition->sub_EntityCondition->sub_DistanceCondition->_value;
          strDistanceType = "cartesianDistance";
          nType = TrigConditionType::TCT_POSITION_ABSOLUTE;
        }
      }
    }
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseStartTrigger(osc::Trigger& st, int& nType, string& strTrigerValue, string& strDistanceType,
                                         string& strAlongRoute, SceneEvent& singleSceneEvent, string& ManeuverName) {
  auto cgs = st.sub_ConditionGroup;
  //
  for (auto cg : cgs) {
    auto cs = cg.sub_Condition;
    for (auto c : cs) {
      SceneCondition condition;
      std::string strConditionName = *c._name;
      std::string strDelay = *c._delay;
      condition.strBoundary = c._conditionEdge.get() ? *c._conditionEdge : "none";
      // by value condition time absolute
      if (c.sub_ByValueCondition) {
        if (c.sub_ByValueCondition->sub_SimulationTimeCondition) {
          condition.strType = "time_trigger";
          strTrigerValue = *c.sub_ByValueCondition->sub_SimulationTimeCondition->_value;
          if (strTrigerValue[0] == '$') FindParameterDeclarationsValude(strTrigerValue);
          condition.strTriggerTime = *c.sub_ByValueCondition->sub_SimulationTimeCondition->_value;
          condition.strEquationOp = formatOptRule(*c.sub_ByValueCondition->sub_SimulationTimeCondition->_rule);
          nType = TrigConditionType::TCT_TIME_ABSOLUTE;
        }
        // 自定义
        if (c.sub_ByValueCondition->sub_ParameterCondition) {
          condition.strType = "ego_attach_laneid_custom";
          string strvalue = *c.sub_ByValueCondition->sub_ParameterCondition->_value;
          std::vector<std::string> strVec;
          boost::algorithm::split(strVec, strvalue, boost::algorithm::is_any_of(";"));
          if (strVec.size() == 3) {
            condition.strRoadId = strVec[0];
            condition.strSectionId = strVec[1];
            condition.strLaneId = strVec[2];
          }
          condition.strVariate = *c.sub_ByValueCondition->sub_ParameterCondition->_parameterRef;
          condition.strEquationOp = formatOptRule(*c.sub_ByValueCondition->sub_ParameterCondition->_rule);
        }
        // 故事板
        if (c.sub_ByValueCondition->sub_StoryboardElementStateCondition) {
          condition.strType = "element_state";
          condition.strElementType =
              *c.sub_ByValueCondition->sub_StoryboardElementStateCondition->_storyboardElementType;
          condition.strElementRef = *c.sub_ByValueCondition->sub_StoryboardElementStateCondition->_storyboardElementRef;
          condition.strState = *c.sub_ByValueCondition->sub_StoryboardElementStateCondition->_state;
        }
      }
      // by entity condition event
      if (c.sub_ByEntityCondition) {
        const auto& ers = c.sub_ByEntityCondition->sub_TriggeringEntities->sub_EntityRef;
        for (auto er : ers) {
          condition.strTargetElement = getEntryRef(*er._entityRef);
        }
        // ttc
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition) {
          strTrigerValue = *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_value;
          condition.strType = "ttc_trigger";
          condition.strThreshold = *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_value;

          string strAlongRoute =
              *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_alongRoute;
          if (strAlongRoute == "false") {
            condition.strDisType = "euclideandistance";
          } else {
            condition.strDisType = "laneprojection";
          }
          nType = TrigConditionType::TCT_TIME_RELATIVE;

          condition.strEquationOp =
              formatOptRule(*c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeToCollisionCondition->_rule);
        }

        // 相对距离触发
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition) {
          condition.strType = "distance_trigger";
          condition.strThreshold = *c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition->_value;
          condition.strEquationOp =
              formatOptRule(*c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition->_rule);
          string strAlongRoute =
              *c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeDistanceCondition->_relativeDistanceType;
          if (strAlongRoute == "cartesianDistance") {
            condition.strDisType = "euclideandistance";
          } else {
            condition.strDisType = "laneprojection";
          }
          strTrigerValue = condition.strThreshold;
          strDistanceType = condition.strDisType;
          nType = TrigConditionType::TCT_POSITION_RELATIVE;
        }

        // 距离触发
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_DistanceCondition) {
          double distance =
              std::atof((*c.sub_ByEntityCondition->sub_EntityCondition->sub_DistanceCondition->_value).c_str());
          const auto& wpo =
              c.sub_ByEntityCondition->sub_EntityCondition->sub_DistanceCondition->sub_Position->sub_WorldPosition;
          if (wpo) {
            double x = std::atof((*wpo->_x).c_str());
            double y = std::atof((*wpo->_y).c_str());
            ConvertXODR2LonLat(x, y);
            sTagQueryInfoResult result;
            ConvertLonLat2SimPosition(x, y, result);
            ConvertSimPosition2LonLat(result.dsrcLon, result.dsrcLat, result.laneID, result.dShift + distance, 0, x, y);
            std::stringstream ssX;
            ssX << std::setprecision(10) << x;
            std::stringstream ssY;
            ssY << std::setprecision(10) << y;
            condition.strLon = ssX.str();
            condition.strLat = ssY.str();
            condition.strPostionType = "absolute_position";
            condition.strType = "reach_abs_position";
            condition.strRadius = "20";
            if (wpo->_z) condition.strAlt = (*wpo->_y);
          }
        }

        // 到达位置
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_ReachPositionCondition) {
          condition.strRadius = *c.sub_ByEntityCondition->sub_EntityCondition->sub_ReachPositionCondition->_tolerance;
          // WorldPosition
          const auto& wpo =
              c.sub_ByEntityCondition->sub_EntityCondition->sub_ReachPositionCondition->sub_Position->sub_WorldPosition;
          if (wpo) {
            condition.strType = "reach_abs_position";
            condition.strPostionType = "absolute_position";

            condition.strLon = *wpo->_x;
            condition.strLat = *wpo->_y;
            if (wpo->_z) condition.strAlt = *(wpo->_z);
            if (wpo->_z) condition.strRadius = *(wpo->_r);
          }

          // LanePosition
          const auto& lpo =
              c.sub_ByEntityCondition->sub_EntityCondition->sub_ReachPositionCondition->sub_Position->sub_LanePosition;
          if (lpo) {
            condition.strType = "reach_abs_lane";
            condition.strRoadId = *lpo->_roadId;
            condition.strLaneId = *lpo->_laneId;
            condition.strLongitudinalOffset = *lpo->_s;
            if (lpo->_offset) condition.strLateralOffset = *lpo->_offset;

            if (lpo->sub_Orientation && lpo->sub_Orientation->_type) {
              condition.strPostionType = *(lpo->sub_Orientation->_type);
            }
          }
        }

        // 绝对速度
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_SpeedCondition) {
          condition.strType = "velocity_trigger";
          condition.strSpeedType = "absolute";
          condition.strThreshold = *c.sub_ByEntityCondition->sub_EntityCondition->sub_SpeedCondition->_value;
          condition.strEquationOp =
              formatOptRule(*c.sub_ByEntityCondition->sub_EntityCondition->sub_SpeedCondition->_rule);
          condition.strSourceElement = ManeuverName;
          condition.strSpeedUnit = "m_s";
          if (c.sub_ByEntityCondition->sub_TriggeringEntities->sub_EntityRef.size() > 0) {
            condition.strTargetElement =
                *c.sub_ByEntityCondition->sub_TriggeringEntities->sub_EntityRef.at(0)._entityRef;
          }
          condition.strDisType = "euclideandistance";
        }

        // 相对速度
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeSpeedCondition) {
          condition.strType = "velocity_trigger";
          condition.strSpeedType = "relative";
          condition.strThreshold = *c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeSpeedCondition->_value;
          condition.strEquationOp =
              formatOptRule(*c.sub_ByEntityCondition->sub_EntityCondition->sub_RelativeSpeedCondition->_rule);
          condition.strSourceElement = ManeuverName;
          condition.strSpeedUnit = "m_s";
          if (c.sub_ByEntityCondition->sub_TriggeringEntities->sub_EntityRef.size() > 0) {
            condition.strTargetElement =
                *c.sub_ByEntityCondition->sub_TriggeringEntities->sub_EntityRef.at(0)._entityRef;
          }
          condition.strDisType = "euclideandistance";
        }

        // 车头时距
        if (c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeHeadwayCondition) {
          condition.strType = "timeheadway_trigger";
          condition.strTargetElement =
              *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeHeadwayCondition->_entityRef;
          condition.strThreshold = *c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeHeadwayCondition->_value;
          condition.strEquationOp =
              formatOptRule(*c.sub_ByEntityCondition->sub_EntityCondition->sub_TimeHeadwayCondition->_rule);
          condition.strSpeedUnit = "m_s";
        }
      }
      // conditions
      singleSceneEvent.ConditionVec.push_back(condition);
    }
  }
  return 0;
}

int XOSCReader_1_0_v4::CheckTriggerType(SceneEvent& singleSceneEvent, std::string entryname) { return 0; }

int XOSCReader_1_0_v4::ParseAction(std::string entryName, osc::Action& st, SceneEvent& singleSceneEvent,
                                   sTagSimuTraffic& infos) {
  std::string strActionName = *st._name;
  const auto& privateAct = st.sub_PrivateAction;
  Scene_Action Scene_ActionTemp;
  Scene_EndCondition Scene_EndConditionTemp;
  Scene_ActionTemp.strName = strActionName;
  if (privateAct) {
    // velocity and acc
    if (privateAct->sub_LongitudinalAction) {
      if (m_egoInfos.find(entryName) == m_egoInfos.end() && privateAct->sub_LongitudinalAction->sub_SpeedAction) {
        const auto& speedAction = privateAct->sub_LongitudinalAction->sub_SpeedAction;
        if (speedAction->sub_SpeedActionTarget->sub_AbsoluteTargetSpeed) {
          string strSpeedValue = *speedAction->sub_SpeedActionTarget->sub_AbsoluteTargetSpeed->_value;
          if (strSpeedValue[0] == '$') FindParameterDeclarationsValude(strSpeedValue);
          string strDynamicsShape = *speedAction->sub_SpeedActionDynamics->_dynamicsShape;
          string strValue = *speedAction->sub_SpeedActionDynamics->_value;
          if (strValue[0] == '$') FindParameterDeclarationsValude(strValue);
          string strDynamicsDimension = *speedAction->sub_SpeedActionDynamics->_dynamicsDimension;
          if (strDynamicsShape == "linear" || strDynamicsShape == "sinusoidal") {
            // 加速度（速度终止）
            Scene_ActionTemp.strChecked = "true";
            Scene_ActionTemp.strType = "acc";
            Scene_ActionTemp.strValue = strValue;
            Scene_ActionTemp.strSubType = "none";
            Scene_ActionTemp.strLaneOffset = "0";
            if (strSpeedValue != "-1.0000") {
              Scene_EndConditionTemp.strType = "velocity";
              Scene_EndConditionTemp.strValue = strSpeedValue;
            } else {
              Scene_EndConditionTemp.strType = "none";
              Scene_EndConditionTemp.strValue = "0";
            }
          } else if (strDynamicsShape == "step") {
            // 速度触发
            Scene_ActionTemp.strChecked = "true";
            Scene_ActionTemp.strType = "velocity";
            Scene_ActionTemp.strValue = strSpeedValue;
            Scene_ActionTemp.strSubType = "none";
            Scene_ActionTemp.strLaneOffset = "0.0";
            Scene_EndConditionTemp.strType = "none";
            Scene_EndConditionTemp.strValue = "0.0";
          }
          singleSceneEvent.isUserDefined = true;
          singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
          singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
        } else if (speedAction->sub_SpeedActionTarget->sub_RelativeTargetSpeed) {
          string strSpeedValue = *speedAction->sub_SpeedActionTarget->sub_RelativeTargetSpeed->_value;
          if (strSpeedValue[0] == '$') {
            FindParameterDeclarationsValude(strSpeedValue);
          }
          string strDynamicsShape = *speedAction->sub_SpeedActionDynamics->_dynamicsShape;
          string strValue = *speedAction->sub_SpeedActionDynamics->_value;
          if (strValue[0] == '$') FindParameterDeclarationsValude(strValue);
          string strDynamicsDimension = *speedAction->sub_SpeedActionDynamics->_dynamicsDimension;
          if (strDynamicsShape == "linear") {
            // 以时间终止
            string strAccValue = strValue;
            double T = stod(strSpeedValue) / stod(strAccValue);
            string strAccOverValue = std::to_string(T);
            Scene_ActionTemp.strChecked = "true";
            Scene_ActionTemp.strType = "acc";
            Scene_ActionTemp.strValue = strValue;
            Scene_ActionTemp.strSubType = "none";
            Scene_ActionTemp.strLaneOffset = "0.0";
            Scene_EndConditionTemp.strType = "time";
            Scene_EndConditionTemp.strValue = strAccOverValue;
          }
          singleSceneEvent.isUserDefined = true;
          singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
          singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
        }
      }
    }
    // lateralAction
    if (privateAct->sub_LateralAction) {
      if (privateAct->sub_LateralAction->sub_LaneChangeAction) {
        SYSTEM_LOGGER_INFO("LaneChangeAction");
        string strTargetLane, strV, strDynamicsShape, strDynamicsDimension;
        std::string targetLane;
        std::string strMergeOffset;
        ReadLateralAction_LaneChangeAction(*privateAct, strDynamicsShape, strV, strDynamicsDimension, strTargetLane,
                                           strMergeOffset);
        std::string strMergeOverTime = strV;
        std::string strMergeValue = strTargetLane;
        Scene_ActionTemp.strChecked = "true";
        Scene_ActionTemp.strType = "merge";
        Scene_ActionTemp.strValue = strV;
        Scene_ActionTemp.strLaneOffset = strMergeOffset;
        if (m_version == XOSC_1_0_VERSION_TAD_SIM) {
          if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) > 0) {
            Scene_ActionTemp.strSubType = "right_in_lane";
          }
          if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) < 0) {
            Scene_ActionTemp.strSubType = "left_in_lane";
            if (!strMergeOffset.empty()) Scene_ActionTemp.strLaneOffset = strMergeOffset.substr(1);
          }
          if (std::atof(strTargetLane.c_str()) > 0) {
            Scene_ActionTemp.strSubType = "right";
          }
          if (std::atof(strTargetLane.c_str()) < 0) {
            Scene_ActionTemp.strSubType = "left";
          }
          if (Scene_ActionTemp.strLaneOffset.empty()) {
            Scene_ActionTemp.strLaneOffset = "0";
          }
        } else {
          if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) < 0) {
            Scene_ActionTemp.strSubType = "right_in_lane";
          }
          if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) > 0) {
            Scene_ActionTemp.strSubType = "left_in_lane";
          }
          if (std::atof(strTargetLane.c_str()) < 0) {
            Scene_ActionTemp.strSubType = "right";
          }
          if (std::atof(strTargetLane.c_str()) > 0) {
            Scene_ActionTemp.strSubType = "left";
          }
          if (Scene_ActionTemp.strLaneOffset.empty()) {
            Scene_ActionTemp.strLaneOffset = "0";
          }
        }
        Scene_EndConditionTemp.strType = "none";
        Scene_EndConditionTemp.strValue = "0.0";
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      } else if (privateAct->sub_LateralAction->sub_LateralDistanceAction) {
        SYSTEM_LOGGER_INFO("LateralDistanceAction");
        Scene_ActionTemp.strChecked = "true";
        Scene_ActionTemp.strType = "lateralDistance";
        Scene_ActionTemp.strChecked = "true";
        Scene_ActionTemp.strValue = *privateAct->sub_LateralAction->sub_LateralDistanceAction->_distance;
        Scene_ActionTemp.sActionMulti.sLateralDistance.s_continuous =
            *privateAct->sub_LateralAction->sub_LateralDistanceAction->_continuous;
        Scene_ActionTemp.sActionMulti.sLateralDistance.s_target_element =
            *privateAct->sub_LateralAction->sub_LateralDistanceAction->_entityRef;
        if (privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints) {
          if (privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxAcceleration) {
            Scene_ActionTemp.sActionMulti.sLateralDistance.s_maxAcc =
                *privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxAcceleration;
          }
          if (privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxDeceleration) {
            Scene_ActionTemp.sActionMulti.sLateralDistance.s_maxDec =
                *privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxDeceleration;
          }
          if (privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxSpeed) {
            Scene_ActionTemp.sActionMulti.sLateralDistance.s_maxSpeed =
                *privateAct->sub_LateralAction->sub_LateralDistanceAction->sub_DynamicConstraints->_maxSpeed;
          }
        }
        Scene_EndConditionTemp.strType = "none";
        Scene_EndConditionTemp.strValue = "0.0";
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      }
    }
    // activate
    if (privateAct->sub_ActivateControllerAction && (privateAct->sub_ActivateControllerAction->_lateral ||
                                                     privateAct->sub_ActivateControllerAction->_longitudinal)) {
      SYSTEM_LOGGER_INFO("pa.ActivateControllerAction");
      int nValue = 0;
      const auto& strlateral = privateAct->sub_ActivateControllerAction->_lateral;
      const auto& strlongitudinal = privateAct->sub_ActivateControllerAction->_longitudinal;
      if ((strlateral) != NULL && (*strlateral == string("true"))) {
        nValue += 1;
      }
      if (strlongitudinal != NULL && (*strlongitudinal == string("true"))) {
        nValue += 2;
      }
      string strValue;
      switch (nValue) {
        case 0:
          strValue = "off";
          break;
        case 1:
          strValue = "lateral";
          break;
        case 2:
          strValue = "longitudinal";
          break;
        case 3:
          strValue = "autopilot";
          break;
        default:
          break;
      }
      Scene_ActionTemp.strType = "activate";
      Scene_ActionTemp.strValue = "0.0";
      Scene_ActionTemp.strSubType = strValue;
      Scene_ActionTemp.strLaneOffset = "0.0";
      Scene_EndConditionTemp.strType = "none";
      Scene_EndConditionTemp.strValue = "0.0";
      singleSceneEvent.isUserDefined = true;
      singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
      singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
    }
    // control
    if (privateAct->sub_ControllerAction) {
      string controlName = *privateAct->sub_ControllerAction->sub_AssignControllerAction->sub_Controller->_name;
      if (!controlName.empty() && controlName != "none") {
        const auto& sProperties =
            privateAct->sub_ControllerAction->sub_AssignControllerAction->sub_Controller->sub_Properties;
        const auto& PropertSeq = sProperties->sub_Property;
        for (auto itPro : PropertSeq) {
          if (*itPro._name == "resume_sw") {
            Scene_ActionTemp.sActionMulti.sAssign.s_resume_sw = *itPro._value;
          } else if (*itPro._name == "cancel_sw") {
            Scene_ActionTemp.sActionMulti.sAssign.s_cancel_sw = *itPro._value;
          } else if (*itPro._name == "speed_inc_sw") {
            Scene_ActionTemp.sActionMulti.sAssign.s_speed_inc_sw = *itPro._value;
          } else if (*itPro._name == "speed_dec_sw") {
            Scene_ActionTemp.sActionMulti.sAssign.s_speed_dec_sw = *itPro._value;
          } else if (*itPro._name == "set_timegap") {
            Scene_ActionTemp.sActionMulti.sAssign.s_set_timegap = *itPro._value;
          } else if (*itPro._name == "set_speed") {
            Scene_ActionTemp.sActionMulti.sAssign.s_set_speed = *itPro._value;
          }
        }
        Scene_ActionTemp.sActionMulti.flag = true;
        Scene_ActionTemp.strType = "assign";
        Scene_ActionTemp.strValue = controlName;
        Scene_ActionTemp.strSubType = "none";
        Scene_ActionTemp.strLaneOffset = "0.0";
        Scene_EndConditionTemp.strType = "none";
        Scene_EndConditionTemp.strValue = "0.0";
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      } else {
        const auto& overide = privateAct->sub_ControllerAction->sub_OverrideControllerValueAction;
        string tmpActivate = *overide->sub_Throttle->_active;
        string tmpValue = *overide->sub_Throttle->_value;
        Scene_ActionTemp.sActionMulti.sOveride.s_throttle = (tmpActivate) + ";" + tmpValue;
        tmpActivate = *overide->sub_Brake->_active;
        tmpValue = *overide->sub_Brake->_value;
        Scene_ActionTemp.sActionMulti.sOveride.s_brake = (tmpActivate) + ";" + tmpValue;
        tmpActivate = *overide->sub_Clutch->_active;
        tmpValue = *overide->sub_Clutch->_value;
        Scene_ActionTemp.sActionMulti.sOveride.s_clutch = (tmpActivate) + ";" + tmpValue;

        tmpActivate = *overide->sub_ParkingBrake->_active;
        tmpValue = *overide->sub_ParkingBrake->_value;
        Scene_ActionTemp.sActionMulti.sOveride.s_parking_brake = (tmpActivate) + ";" + tmpValue;

        tmpActivate = *overide->sub_SteeringWheel->_active;
        tmpValue = *overide->sub_SteeringWheel->_value;
        Scene_ActionTemp.sActionMulti.sOveride.s_steering_wheel = (tmpActivate) + ";" + tmpValue;

        tmpActivate = *overide->sub_Gear->_active;
        tmpValue = *overide->sub_Gear->_number;
        Scene_ActionTemp.sActionMulti.sOveride.s_gear = (tmpActivate) + ";" + tmpValue;

        Scene_ActionTemp.sActionMulti.flag = true;
        Scene_ActionTemp.strType = "override";
        Scene_ActionTemp.strValue = "none";
        Scene_ActionTemp.strSubType = "none";
        Scene_ActionTemp.strLaneOffset = "0.0";
        Scene_EndConditionTemp.strType = "none";
        Scene_EndConditionTemp.strValue = "0.0";
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      }
    }
  }
  const auto& userdefineAct = st.sub_UserDefinedAction;
  if (userdefineAct) {
    std::string stype = *userdefineAct->sub_CustomCommandAction->_type;
    if (stype == "Command") {
      std::string strText = *userdefineAct->sub_CustomCommandAction->_text;
      std::string strSubType;
      std::string strValue;
      std::string strOffset;
      GetCommandValue(strText, strSubType, strValue, strOffset);
      Scene_ActionTemp.strType = "command";
      if (strSubType == "LaneChange") {
        Scene_ActionTemp.strSubType = "lane_change";
        Scene_ActionTemp.strValue = strValue;
        Scene_ActionTemp.strLaneOffset = strOffset;
      } else if (strSubType == "Overtaking") {
        Scene_ActionTemp.strSubType = "overtaking";
        Scene_ActionTemp.strValue = strValue;
        Scene_ActionTemp.strLaneOffset = strOffset;
      } else if (strSubType == "PullOver") {
        Scene_ActionTemp.strSubType = "pull_over";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "EmergencyStop") {
        Scene_ActionTemp.strSubType = "emergency_stop";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "LateralSpeedToLeft") {
        Scene_ActionTemp.strSubType = "lateral_speed_to_left";
        Scene_ActionTemp.strValue = strOffset;
      } else if (strSubType == "LateralSpeedToRight") {
        Scene_ActionTemp.strSubType = "lateral_speed_to_right";
        Scene_ActionTemp.strValue = strOffset;
      } else if (strSubType == "LateralAccelToLeft") {
        Scene_ActionTemp.strSubType = "lateral_accel_to_left";
        Scene_ActionTemp.strValue = strOffset;
      } else if (strSubType == "LateralAccelToRight") {
        Scene_ActionTemp.strSubType = "lateral_accel_to_right";
        Scene_ActionTemp.strValue = strOffset;
      } else if (strSubType == "UserDefine") {
        Scene_ActionTemp.strSubType = "userdefine";
        Scene_ActionTemp.strValue = strValue;
      } else {
        return 0;
      }
    } else if (stype == "Status") {
      std::string strText = *userdefineAct->sub_CustomCommandAction->_text;
      std::string strSubType;
      std::string strValue;
      std::string strOffset;
      GetCommandValue(strText, strSubType, strValue, strOffset);
      Scene_ActionTemp.strType = "status";
      if (strSubType == "EmergencyBrake") {
        Scene_ActionTemp.strSubType = "emergency_brake";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "VehicleLostControl") {
        Scene_ActionTemp.strSubType = "vehicle_lost_control";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "HandsOnSteeringwheel") {
        Scene_ActionTemp.strSubType = "hands_on_steeringwheel";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "EyesOnRoad") {
        Scene_ActionTemp.strSubType = "eyes_on_road";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Lidar") {
        Scene_ActionTemp.strSubType = "lidar";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Lidar") {
        Scene_ActionTemp.strSubType = "lidar";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Radar") {
        Scene_ActionTemp.strSubType = "radar";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Ultrasonic") {
        Scene_ActionTemp.strSubType = "ultrasonic";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Camera") {
        Scene_ActionTemp.strSubType = "camera";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Gnss") {
        Scene_ActionTemp.strSubType = "gnss";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Imu") {
        Scene_ActionTemp.strSubType = "imu";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Localization") {
        Scene_ActionTemp.strSubType = "localization";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Beam") {
        Scene_ActionTemp.strSubType = "beam";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "BrakeLight") {
        Scene_ActionTemp.strSubType = "brake_light";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "HazardLight") {
        Scene_ActionTemp.strSubType = "hazard_light";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "LeftTurnLight") {
        Scene_ActionTemp.strSubType = "left_turn_light";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "RightTurnLight") {
        Scene_ActionTemp.strSubType = "right_turn_light";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "DriverSeatbelt") {
        Scene_ActionTemp.strSubType = "driver_seatbelt";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "PassengerSeatbelt") {
        Scene_ActionTemp.strSubType = "passenger_seatbelt";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "DriverDoor") {
        Scene_ActionTemp.strSubType = "driver_door";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "PassengerDoor") {
        Scene_ActionTemp.strSubType = "passenger_door";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Hood") {
        Scene_ActionTemp.strSubType = "hood";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Trunk") {
        Scene_ActionTemp.strSubType = "trunk";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "ParkingBrake") {
        Scene_ActionTemp.strSubType = "parkingbrake";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Wiper") {
        Scene_ActionTemp.strSubType = "wiper";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Trunk") {
        Scene_ActionTemp.strSubType = "trunk";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Gear") {
        Scene_ActionTemp.strSubType = "gear";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "Key") {
        Scene_ActionTemp.strSubType = "key";
        Scene_ActionTemp.strValue = strValue;
      } else if (strSubType == "UserDefine") {
        Scene_ActionTemp.strSubType = "userdefine";
        Scene_ActionTemp.strValue = strValue;
      } else {
        return -1;
      }
    }
    Scene_ActionTemp.sActionMulti.flag = true;
    Scene_EndConditionTemp.strType = "none";
    Scene_EndConditionTemp.strValue = "0.0";
    singleSceneEvent.isUserDefined = true;
    singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
    singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
  }
  //
  if (st.sub_PrivateAction && st.sub_PrivateAction->sub_RoutingAction) {
    SYSTEM_LOGGER_INFO("pa.sub_RoutingAction");
    const auto& privateAct = st.sub_PrivateAction;
    // planner and vehicle
    vector<sPathPoint> TempPoints;
    std::string strOrientation = "";
    if (privateAct->sub_RoutingAction->sub_AssignRouteAction) {
      ReadRoutingAction(*privateAct, AssignRouteAction_Type, TempPoints, strOrientation);
      AddRoute(entryName, infos, TempPoints);
    } else if (privateAct->sub_RoutingAction->sub_FollowTrajectoryAction) {
      ReadRoutingAction(*privateAct, FollowTrajectoryAction_Type, TempPoints, strOrientation);
      AddRoute(entryName, infos, TempPoints);
    } else if (privateAct->sub_RoutingAction->sub_AcquirePositionAction) {
      // only end position
      TempPoints = infos.m_simulation.m_planner.m_route.m_path.m_vPoints;
      ReadRoutingAction(*privateAct, AcquirePositionAction_Type, TempPoints, strOrientation);
      AddRoute(entryName, infos, TempPoints);
    }
    return 0;
  }

  return 0;
}
void XOSCReader_1_0_v4::GetCommandValue(std::string strText, std::string& strSubType, std::string& strValue,
                                        std::string& strOffset) {
  std::vector<std::string> strValueVec;
  boost::algorithm::split(strValueVec, strText, boost::algorithm::is_any_of(":"));
  if (strValueVec.size() < 2) {
    return;
  }
  strSubType = strValueVec.at(0);
  if (strSubType == "UserDefine") {
    auto f = strText.find("=");
    auto t = strText.rfind(",value=0.0");
    if (f != std::string::npos || t > f) {
      strValue = strText.substr(f + 1, t == std::string::npos ? t : t - f - 1);
    }
    return;
  }
  std::vector<std::string> valueAndOffset;
  boost::algorithm::split(valueAndOffset, strValueVec.at(1), boost::algorithm::is_any_of(","));
  if (valueAndOffset.size() == 0) {
    return;
  } else if (valueAndOffset.size() == 1) {
    std::vector<std::string> tmp;
    boost::algorithm::split(tmp, valueAndOffset.at(0), boost::algorithm::is_any_of("="));
    strValue = tmp.at(1);
  } else if (valueAndOffset.size() == 2) {
    std::vector<std::string> tmp;
    boost::algorithm::split(tmp, valueAndOffset.at(0), boost::algorithm::is_any_of("="));
    strValue = tmp.at(1);
    tmp.clear();
    boost::algorithm::split(tmp, valueAndOffset.at(1), boost::algorithm::is_any_of("="));
    strOffset = tmp.at(1);
  }
  return;
}
void XOSCReader_1_0_v4::ParseEnvironmentAction(osc::EnvironmentAction& environment, sTagSimuTraffic& infos) {
  osc::xsd::Attribute<osc::Environment> tmp;
  if (environment.sub_CatalogReference) {
    std::string name = *environment.sub_CatalogReference->_entryName;
    ParseEnvironmentCatalogXosc(name, *tmp);
  } else {
    tmp = environment.sub_Environment;
  }
  CWeatherFrame tmpEnvironment;
  std::string dateTime = *tmp->sub_TimeOfDay->_dateTime;
  tmpEnvironment.setDateTime(dateTime);

  //
  CWeatherV2 tmpWeather;
  tmpWeather.m_strVisualRange = std::to_string(atof((*tmp->sub_Weather->sub_Fog->_visualRange).c_str()) / 1000.0);
  //
  std::map<std::string, std::string> key2valueMap;
  // 时间
  int64_t sTimeStamp = 0;
  std::string swind_speed = "1.0";
  std::string stmperature = "30.0";
  const auto& Declarations = tmp->sub_ParameterDeclarations;
  if (Declarations) {
    for (auto& it : Declarations->sub_ParameterDeclaration) {
      key2valueMap[*it._name] = *it._value;
    }
    if (key2valueMap.find("TimeStamp") != key2valueMap.end()) {
      sTimeStamp = std::atol(key2valueMap["TimeStamp"].c_str());
    }
    if (key2valueMap.find("wind_speed") != key2valueMap.end()) {
      swind_speed = key2valueMap["wind_speed"];
    }
    if (key2valueMap.find("tmperature") != key2valueMap.end()) {
      stmperature = std::to_string(std::atof(key2valueMap["tmperature"].c_str()) - 273.15);
    }
  }
  tmpEnvironment.m_date.m_lTimeStamp = sTimeStamp;
  tmpEnvironment.m_time.m_lTimeStamp = sTimeStamp;
  tmpEnvironment.m_timestamp = sTimeStamp;
  // 雨和雪
  double srain = 0.0;
  double ssnow = 0.0;
  std::string sPrecipitationType = *tmp->sub_Weather->sub_Precipitation->_precipitationType;
  std::string sPrecipitationInstencity = *tmp->sub_Weather->sub_Precipitation->_intensity;
  std::string sCloudState = *tmp->sub_Weather->_cloudState;
  tmpWeather.m_lTimeStamp = sTimeStamp;
  // tmpWeather.m_strRainFall = std::to_string(srain);
  // tmpWeather.m_strSnowFall = std::to_string(ssnow);
  tmpWeather.m_strPrecipitationType = sPrecipitationType;
  tmpWeather.m_strPrecipitationIntensity = sPrecipitationInstencity;
  tmpWeather.m_strWindSpeed = swind_speed;
  tmpWeather.m_strCloudState = sCloudState;
  tmpWeather.m_strTemperature = stmperature;
  tmpEnvironment.setWeather(tmpWeather);
  infos.m_environment.WeatherData()[sTimeStamp] = tmpEnvironment;
  infos.m_bIncludeEnvironment = true;
}
void XOSCReader_1_0_v4::ReadLateralAction_LaneChangeAction(osc::PrivateAction& pa, string& strDynamicsShape,
                                                           string& strValue, string& strDynamicsDimension,
                                                           string& strTargetLane, string& strTargetOffset) {
  const auto& targetLaneOffset = pa.sub_LateralAction->sub_LaneChangeAction->_targetLaneOffset;
  if (targetLaneOffset) {
    strTargetOffset = *targetLaneOffset;
  } else {
    strTargetOffset = "";
  }
  if (pa.sub_LateralAction->sub_LaneChangeAction->sub_LaneChangeTarget) {
    strTargetLane = *pa.sub_LateralAction->sub_LaneChangeAction->sub_LaneChangeTarget->sub_RelativeTargetLane->_value;
    if (strTargetLane[0] == '$') FindParameterDeclarationsValude(strTargetLane);
  }

  strDynamicsShape = *pa.sub_LateralAction->sub_LaneChangeAction->sub_LaneChangeActionDynamics->_dynamicsShape;

  strValue = *pa.sub_LateralAction->sub_LaneChangeAction->sub_LaneChangeActionDynamics->_value;

  if (strValue[0] == '$') FindParameterDeclarationsValude(strValue);

  strDynamicsDimension = *pa.sub_LateralAction->sub_LaneChangeAction->sub_LaneChangeActionDynamics->_dynamicsDimension;
}

void XOSCReader_1_0_v4::ReadLongitudinalSpeedAbsolute(osc::PrivateAction& pa, string& strDynamicsShape,
                                                      string& strValue, string& strDynamicsDimension,
                                                      string& strSpeedValue) {
  // 速度值
  strSpeedValue = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionTarget->sub_AbsoluteTargetSpeed->_value;

  if (strSpeedValue[0] == '$') FindParameterDeclarationsValude(strSpeedValue);

  strDynamicsShape = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_dynamicsShape;

  strValue = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_value;

  if (strValue[0] == '$') FindParameterDeclarationsValude(strValue);

  strDynamicsDimension = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_dynamicsDimension;
}
void XOSCReader_1_0_v4::ReadLongitudinalSpeedRelative(osc::PrivateAction& pa, string& strDynamicsShape,
                                                      string& strValue, string& strDynamicsDimension,
                                                      string& strSpeedValue) {
  // 相对速度值  算加速时间用
  strSpeedValue = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionTarget->sub_RelativeTargetSpeed->_value;

  if (strSpeedValue[0] == '$') {
    FindParameterDeclarationsValude(strSpeedValue);
  }

  strDynamicsShape = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_dynamicsShape;

  strValue = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_value;
  if (strValue[0] == '$') FindParameterDeclarationsValude(strValue);

  strDynamicsDimension = *pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionDynamics->_dynamicsDimension;
}

void XOSCReader_1_0_v4::AddIniSpeed(string& strEntityRef, sTagSimuTraffic& infos, string& strSpeedV) {
  if (m_egoInfos.find(strEntityRef) != m_egoInfos.end()) {
    CEgoInfo& egoinfo = m_egoInfos.find(strEntityRef)->second;
    egoinfo.m_strStartVelocity = strSpeedV;
    if (atof(egoinfo.m_strVelocityMax.c_str()) < atof(egoinfo.m_strStartVelocity.c_str())) {
      egoinfo.m_strVelocityMax = egoinfo.m_strStartVelocity;
    }
  }
  {
    auto itr = m_mapVehicles.find(strEntityRef);
    if (itr != m_mapVehicles.end()) {
      itr->second.m_strStartVelocity = strSpeedV;
      if (atof(itr->second.m_strMaxVelocity.c_str()) < atof(itr->second.m_strStartVelocity.c_str())) {
        itr->second.m_strMaxVelocity = itr->second.m_strStartVelocity;
      }
      return;
    }
  }
  {
    auto itr = m_mapPedestrians.find(strEntityRef);
    if (itr != m_mapPedestrians.end()) {
      itr->second.m_strStartVelocity = strSpeedV;
      if (atof(itr->second.m_strMaxVelocity.c_str()) < atof(itr->second.m_strStartVelocity.c_str())) {
        itr->second.m_strMaxVelocity = itr->second.m_strStartVelocity;
      }
      return;
    }
  }
}
void XOSCReader_1_0_v4::ReadTeleportAction_LanePosition(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid,
                                                        hadmap::lanepkid& iLaneid, double& dS, double& dOffset,
                                                        double& yaw) {
  string strRoadid = *pa.sub_TeleportAction->sub_Position->sub_LanePosition->_roadId;
  iRoadid = std::atoi(strRoadid.c_str());
  string strLaneid = *pa.sub_TeleportAction->sub_Position->sub_LanePosition->_laneId;
  iLaneid = std::atoi(strLaneid.c_str());
  string strS = *pa.sub_TeleportAction->sub_Position->sub_LanePosition->_s;
  dS = stod(strS);
  const auto& Offset_o = pa.sub_TeleportAction->sub_Position->sub_LanePosition->_offset;
  if (Offset_o) {
    string strOffset = *Offset_o;
    dOffset = stod(strOffset);
  }
  if (pa.sub_TeleportAction->sub_Position->sub_LanePosition->sub_Orientation) {
    string strYaw = *pa.sub_TeleportAction->sub_Position->sub_LanePosition->sub_Orientation->_h;
    yaw = std::atof(strYaw.c_str());
  }
}

void XOSCReader_1_0_v4::ReadTeleActionRelLanePos(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid,
                                                 hadmap::lanepkid& iLaneid, double& dS, double& dOffset) {
  string strName = *pa.sub_TeleportAction->sub_Position->sub_RelativeLanePosition->_entityRef;
  auto itr = m_mapIniPos.find(strName);
  if (itr != m_mapIniPos.end()) {
    iRoadid = itr->second.rid;
    iLaneid =
        itr->second.lid + std::atoi((*pa.sub_TeleportAction->sub_Position->sub_RelativeLanePosition->_dLane).c_str());
    dS = itr->second.s + std::stod(*(pa.sub_TeleportAction->sub_Position->sub_RelativeLanePosition->_ds));
    auto& offset = pa.sub_TeleportAction->sub_Position->sub_RelativeLanePosition->_offset;
    if (offset) {
      dOffset = itr->second.t + std::stod((*offset).c_str());
    } else {
      dOffset = 0;
    }
  }
}

void XOSCReader_1_0_v4::ReadTeleportAction_RoadPosition(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, double& dS,
                                                        double& dT) {
  string strRoadid = *pa.sub_TeleportAction->sub_Position->sub_RoadPosition->_roadId;
  iRoadid = std::atoi(strRoadid.c_str());
  string strS = *pa.sub_TeleportAction->sub_Position->sub_RoadPosition->_s;
  dS = std::stod(strS);
  string strT = *pa.sub_TeleportAction->sub_Position->sub_RoadPosition->_t;
  dT = std::stod(strT);
}

void XOSCReader_1_0_v4::ReadTeleActionRelRoadPos(osc::PrivateAction& pa, hadmap::roadpkid& iRoadid, double& dS,
                                                 double& dT) {
  string strName = *pa.sub_TeleportAction->sub_Position->sub_RelativeRoadPosition->_entityRef;
  auto itr = m_mapIniPos.find(strName);
  if (itr != m_mapIniPos.end()) {
    iRoadid = itr->second.rid;
    string strS = *pa.sub_TeleportAction->sub_Position->sub_RelativeRoadPosition->_ds;
    dS = stod(strS) + itr->second.s;
    string strT = *pa.sub_TeleportAction->sub_Position->sub_RelativeRoadPosition->_dt;
    dT = stod(strT) + itr->second.t;
  }
}

void XOSCReader_1_0_v4::ReadTeleportAction_WorldPosition(osc::PrivateAction& pa, double& dX, double& dY, double& dZ,
                                                         double& dH) {
  string strX = *pa.sub_TeleportAction->sub_Position->sub_WorldPosition->_x;

  string strY = *pa.sub_TeleportAction->sub_Position->sub_WorldPosition->_y;

  const auto& z_o = pa.sub_TeleportAction->sub_Position->sub_WorldPosition->_z;
  string strZ;
  if (z_o) {
    strZ = *z_o;
  } else {
    strZ = "0";
  }

  const auto& A = pa.sub_TeleportAction->sub_Position->sub_WorldPosition->_h;
  string strH;
  if (A) {
    strH = *A;
  } else {
    strH = "0";
  }

  // 要在ParameterDeclarations中找到确切值
  if (strX[0] == '$') FindParameterDeclarationsValude(strX);

  // 要在ParameterDeclarations中找到确切值
  if (strY[0] == '$') FindParameterDeclarationsValude(strY);

  // 要在ParameterDeclarations中找到确切值
  if (strZ[0] == '$') FindParameterDeclarationsValude(strZ);

  // 要在ParameterDeclarations中找到确切值
  if (strH[0] == '$') FindParameterDeclarationsValude(strH);

  dX = stod(strX);
  dY = stod(strY);
  dZ = stod(strZ);
  dH = stod(strH);
}

void XOSCReader_1_0_v4::ReadTeleActionRelWorldPos(osc::PrivateAction& pa, double& dX, double& dY, double& dZ,
                                                  double& dH) {
  string strName = *pa.sub_TeleportAction->sub_Position->sub_RelativeWorldPosition->_entityRef;
  auto itr = m_mapIniPos.find(strName);
  if (itr != m_mapIniPos.end()) {
    string XX = *pa.sub_TeleportAction->sub_Position->sub_RelativeWorldPosition->_dx;
    dX = std::stod(XX.c_str()) + itr->second.x;
    string YY = *pa.sub_TeleportAction->sub_Position->sub_RelativeWorldPosition->_dy;
    dY = std::stod(YY.c_str()) + itr->second.y;
    auto ZZ = pa.sub_TeleportAction->sub_Position->sub_RelativeWorldPosition->_dz;
    if (ZZ) {
      dZ = std::stod((*ZZ).c_str()) + itr->second.y;
    } else {
      dZ = 0;
    }
    dH = 0;
  }
}

void XOSCReader_1_0_v4::IniWordPos(string& strEntityRef, sTagSimuTraffic& infos, double& dX, double& dY, double& dZ,
                                   double& dH) {
  CRoute TempRoute;
  TempRoute.m_strType = "start_end";

  double dLon = dX;
  double dLat = dY;
  double dAlt = dZ;

  if (this->m_version != XOSC_VERSION::XOSC_1_0_VERSION_TAD_SIM_2_0) ConvertXODR2LonLat(dLon, dLat);
  sTagQueryInfoResult result;
  ConvertLonLat2SimPosition(dLon, dLat, result);
  // 添加route
  TempRoute.m_strStartLon = DoubleToString(dLon);
  TempRoute.m_strStartLat = DoubleToString(dLat);
  TempRoute.m_strStartAlt = DoubleToString(dH, 6);
  TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
  AddIniPos(strEntityRef, TempRoute, infos, result, dX, dY, dZ, dH, true);
  IniPos TempIniPos;
  TempIniPos.x = dLon;
  TempIniPos.y = dLat;
  TempIniPos.z = dAlt;
  TempIniPos.o = dH;
  TempIniPos.rid = result.roadID;
  TempIniPos.lid = result.laneID;
  TempIniPos.s = result.dShift;
  TempIniPos.t = result.dOffset;
  m_mapIniPos.insert(std::make_pair(strEntityRef, TempIniPos));
}

void XOSCReader_1_0_v4::AddIniPos(string& strEntityRef, CRoute& TempRoute, sTagSimuTraffic& infos,
                                  sTagQueryInfoResult& result, double& dX, double& dY, double& dZ, double& dH,
                                  bool isUse) {
  // 通过位置 确定routeID 和 laneID
  if (m_egoInfos.find(strEntityRef) != m_egoInfos.end()) {
    CEgoInfo& egoinfo = m_egoInfos.find(strEntityRef)->second;
    if (isUse) {
      egoinfo.m_strTheta = DoubleToString(dH, 6);
      TempRoute.m_strStartLon = DoubleToString(result.dsrcLon);
      TempRoute.m_strStartLat = DoubleToString(result.dsrcLat);
      TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
    } else {
      TempRoute.m_strStartLon = DoubleToString(result.dsrcLon);
      TempRoute.m_strStartLat = DoubleToString(result.dsrcLat);
      TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
      egoinfo.m_strTheta = DoubleToString(dH, 6);
    }
    if (false == m_isRouteAction) {
      CRoute& r = TempRoute;
      r.m_strStartLon = DoubleToString(result.dsrcLon);
      r.m_strStartLat = DoubleToString(result.dsrcLat);
      r.m_strStart = r.m_strStartLon + "," + r.m_strStartLat;
      r.ExtractPath();
      r.m_path.ConvertToStr();
      TempRoute.m_strID = std::to_string(m_nRouteIDGenerator++);
      TempRoute.ConvertToValue();
      m_mapRoutes.insert(std::make_pair(TempRoute.m_strID, TempRoute));
      egoinfo.m_strTheta = DoubleToString(dH, 6);
      egoinfo.m_route = r;
      egoinfo.m_inputPath = r.m_path;
      egoinfo.m_strStartAlt = r.m_strStartAlt;
      egoinfo.m_strEndAlt = r.m_strEndAlt;
    }
    return;
  }
  // vehicle
  {
    auto itr = m_mapVehicles.find(strEntityRef);
    if (itr != m_mapVehicles.end()) {
      TempRoute.m_strID = std::to_string(m_nRouteIDGenerator++);
      itr->second.m_strLaneID = std::to_string(result.laneID);
      itr->second.m_strStartShift = std::to_string(result.dShift);
      itr->second.m_strOffset = std::to_string(result.dOffset);
      // set start hdg
      if (itr->second.m_strBehavior == "TrajectoryFollow" && m_version == XOSC_1_0_VERSION_TAD_SIM_2_1) {
        double deg = formatRad2Deg(dH);
        itr->second.m_strStartAngle = DoubleToString(deg, 6);
      } else if (itr->second.m_strBehavior == "TrajectoryFollow") {
        double deg = formatRad2Deg(std::atof(itr->second.m_strAngle.c_str()));
        itr->second.m_strStartAngle = DoubleToString(deg, 6);
      } else {
        itr->second.m_strStartAngle = "";
      }
      if (itr->second.m_strRouteID.size() < 1) {
        itr->second.m_strRouteID = TempRoute.m_strID;
      } else {
        SYSTEM_LOGGER_INFO("vehicle %s route %s already exist!", strEntityRef, itr->second.m_strRouteID.c_str());
      }
      TempRoute.m_strStartLon = DoubleToString(result.dLon);
      TempRoute.m_strStartLat = DoubleToString(result.dLat);
      TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
      m_mapRoutes.insert(std::make_pair(TempRoute.m_strID, TempRoute));
      return;
    }
  }

  {
    auto itr = m_mapPedestrians.find(strEntityRef);
    if (itr != m_mapPedestrians.end()) {
      TempRoute.m_strID = std::to_string(m_nRouteIDGenerator++);
      itr->second.m_strLaneID = std::to_string(result.laneID);
      itr->second.m_strStartShift = std::to_string(result.dShift);
      itr->second.m_strOffset = std::to_string(result.dOffset);
      // set start hdg
      if (itr->second.m_strBehavior == "TrajectoryFollow" && m_version == XOSC_1_0_VERSION_TAD_SIM_2_1) {
        double deg = formatRad2Deg(dH);
        itr->second.m_strStartAngle = DoubleToString(deg, 6);
      } else if (itr->second.m_strBehavior == "TrajectoryFollow") {
        double deg = formatRad2Deg(std::atof(itr->second.m_strAngle.c_str()));
        itr->second.m_strStartAngle = DoubleToString(deg, 6);
      } else {
        itr->second.m_strStartAngle = "";
      }
      if (itr->second.m_strRouteID.size() < 1) {
        itr->second.m_strRouteID = TempRoute.m_strID;
      } else {
        SYSTEM_LOGGER_INFO("pedestrian %s route %s already exist!", strEntityRef, itr->second.m_strRouteID.c_str());
      }
      TempRoute.m_strStartLon = DoubleToString(result.dLon);
      TempRoute.m_strStartLat = DoubleToString(result.dLat);
      TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
      m_mapRoutes.insert(std::make_pair(TempRoute.m_strID, TempRoute));
      return;
    }
  }

  {
    auto itr = m_mapObstacles.find(strEntityRef);
    if (itr != m_mapObstacles.end()) {
      TempRoute.m_strID = std::to_string(m_nRouteIDGenerator++);
      itr->second.m_strLaneID = std::to_string(result.laneID);
      itr->second.m_strStartShift = std::to_string(result.dShift);
      itr->second.m_strOffset = std::to_string(result.dOffset);
      double deg = formatRad2Deg(dH);
      itr->second.m_strStartAngle = DoubleToString(deg, 6);
      roadpkid roadp = result.roadID;
      lanepkid lanep = result.laneID;
      double fShift = result.dShift;
      double fOffset = result.dOffset;
      itr->second.m_strDirection = "0";
      if (itr->second.m_strRouteID.size() < 1) {
        itr->second.m_strRouteID = TempRoute.m_strID;
      } else {
        SYSTEM_LOGGER_INFO("obstacle %s route %s already exist!", strEntityRef, itr->second.m_strRouteID.c_str());
      }
      TempRoute.m_strStartLon = DoubleToString(result.dLon);
      TempRoute.m_strStartLat = DoubleToString(result.dLat);
      TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;
      m_mapRoutes.insert(std::make_pair(TempRoute.m_strID, TempRoute));
      return;
    }
  }

  SYSTEM_LOGGER_ERROR("Wrong entity %s", strEntityRef.c_str());
}

void XOSCReader_1_0_v4::IniLanePos(string& strEntityRef, sTagSimuTraffic& infos, hadmap::roadpkid& iRoadid,
                                   hadmap::lanepkid& iLaneid, double& dS, double& dT, double yaw) {
  CRoute TempRoute;
  TempRoute.m_strType = "start_end";

  sTagQueryInfoResult result;
  sTagQueryInfoResult resultTmp;

  hadmap::txLaneId _tmpLaneId;
  ConvertLonLat2SimPosition(iRoadid, iLaneid, dS, dT, result);
  result.dsrcLon = result.dLon;
  result.dsrcLat = result.dLat;
  ConvertLonLat2SimPosition(iRoadid, result.sectionID, iLaneid, -9999, 0, resultTmp);
  result.dLon = resultTmp.dLon;
  result.dLat = resultTmp.dLat;

  TempRoute.m_strStartLon = DoubleToString(result.dLon);
  TempRoute.m_strStartLat = DoubleToString(result.dLat);
  TempRoute.m_strStartAlt = DoubleToString(result.dAlt);
  TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;

  double TempdLon = result.dLon;
  double TempdLat = result.dLat;
  ConvertLonLat2XODR(TempdLon, TempdLat);  // 将经纬度坐标 转换成 投影平面坐标
  IniPos TempIniPos;
  TempIniPos.x = TempdLon;
  TempIniPos.y = TempdLat;
  TempIniPos.z = result.dAlt;
  TempIniPos.o = 0;
  TempIniPos.rid = iRoadid;
  TempIniPos.lid = iLaneid;
  TempIniPos.s = result.dShift;
  TempIniPos.t = result.dOffset;
  m_mapIniPos.insert(std::make_pair(strEntityRef, TempIniPos));
  double dH = result.yaw / 180.0 * PI;
  if (abs(yaw + 9999.0) > 1e-3) {
    dH = yaw;
  }
  AddIniPos(strEntityRef, TempRoute, infos, result, result.dLon, result.dLat, result.dAlt, dH, false);
}

void XOSCReader_1_0_v4::IniRoadPos(string& strEntityRef, sTagSimuTraffic& infos, hadmap::roadpkid& iRoadid, double& dS,
                                   double& dT) {
  CRoute TempRoute;
  TempRoute.m_strType = "start_end";

  sTagQueryInfoResult result;
  ConvertLonLat2SimPosition(iRoadid, dS, dT, result);

  TempRoute.m_strStartLon = DoubleToString(result.dLon);
  TempRoute.m_strStartLat = DoubleToString(result.dLat);
  TempRoute.m_strStartAlt = DoubleToString(result.dAlt);
  TempRoute.m_strStart = TempRoute.m_strStartLon + "," + TempRoute.m_strStartLat;

  double TempdLon = result.dLon;
  double TempdLat = result.dLat;
  ConvertLonLat2XODR(TempdLon, TempdLat);
  IniPos TempIniPos;
  TempIniPos.x = TempdLon;
  TempIniPos.y = TempdLat;
  TempIniPos.z = result.dAlt;
  TempIniPos.o = 0;
  TempIniPos.rid = iRoadid;
  TempIniPos.lid = result.laneID;
  TempIniPos.s = result.dShift;
  TempIniPos.t = result.dOffset;
  m_mapIniPos.insert(std::make_pair(strEntityRef, TempIniPos));

  double dH = 0;
  AddIniPos(strEntityRef, TempRoute, infos, result, result.dLon, result.dLat, result.dAlt, dH, false);
}

void XOSCReader_1_0_v4::ReadRoutingAction(osc::PrivateAction& pa, RoutingActionType type, vector<sPathPoint>& Vpoints,
                                          string& Orientation) {
  if (type == AssignRouteAction_Type) {
    if (pa.sub_RoutingAction->sub_AssignRouteAction) {
      osc::xsd::Attribute<osc::Route> route;
      const auto& routeAct = pa.sub_RoutingAction->sub_AssignRouteAction;
      if (routeAct->sub_CatalogReference) {
        ParseRouteCatalogXosc(*routeAct->sub_CatalogReference->_entryName, *route);
      } else if (routeAct->sub_Route) {
        route = routeAct->sub_Route;
      }
      if (route && route->sub_Waypoint.size() > 0) {
        // At least two waypoints are needed to define a route.
        const auto& wps = route->sub_Waypoint;
        for (auto iter = wps.begin(); iter != wps.end(); iter++) {
          sPathPoint TempsPathPoint;
          if (iter->sub_Position->sub_RoadPosition) {
            string strRoadid = *iter->sub_Position->sub_RoadPosition->_roadId;
            int iRoadid = std::atoi(strRoadid.c_str());
            string strS = *iter->sub_Position->sub_RoadPosition->_s;
            double dS = stod(strS);
            string strT = *iter->sub_Position->sub_RoadPosition->_t;
            double dT = stod(strT);
            sTagQueryInfoResult result;
            ConvertLonLat2SimPosition(iRoadid, dS, dT, result);
            double TempdLon = result.dLon;
            double TempdLat = result.dLat;
            Orientation = std::to_string(formatDeg2Rad(result.yaw));
            TempsPathPoint.m_strLon = std::to_string(TempdLon);
            TempsPathPoint.m_strLat = std::to_string(TempdLat);
            TempsPathPoint.m_strAlt = std::to_string(result.dAlt);
            TempsPathPoint.m_strHead = std::to_string(formatDeg2Rad(result.yaw));
          } else if (iter->sub_Position->sub_WorldPosition) {
            double _dLon = std::atof((*iter->sub_Position->sub_WorldPosition->_x).c_str());
            double _dLat = std::atof((*iter->sub_Position->sub_WorldPosition->_y).c_str());
            if (this->m_version != XOSC_VERSION::XOSC_1_0_VERSION_TAD_SIM_2_0) ConvertXODR2LonLat(_dLon, _dLat);
            TempsPathPoint.m_strLon = DoubleToString(_dLon);
            TempsPathPoint.m_strLat = DoubleToString(_dLat);
            const auto& z_o = iter->sub_Position->sub_WorldPosition->_z;
            if (z_o) {
              TempsPathPoint.m_strAlt = *z_o;
            }
          } else if (iter->sub_Position->sub_LanePosition) {
            string strRoadid = *iter->sub_Position->sub_LanePosition->_roadId;
            int iRoadid = std::atoi(strRoadid.c_str());
            string strS = *iter->sub_Position->sub_LanePosition->_s;
            double dS = stof(strS);
            string strLaneId = *iter->sub_Position->sub_LanePosition->_laneId;
            int laneid = atoi(strLaneId.c_str());
            string strOffset = *iter->sub_Position->sub_LanePosition->_offset;
            double dT = stof(strOffset);
            sTagQueryInfoResult result;
            ConvertLonLat2SimPosition(iRoadid, laneid, dS, dT, result);
            double TempdLon = result.dLon;
            double TempdLat = result.dLat;
            Orientation = std::to_string(formatDeg2Rad(result.yaw));
            TempsPathPoint.m_strLon = std::to_string(TempdLon);
            TempsPathPoint.m_strLat = std::to_string(TempdLat);
            TempsPathPoint.m_strAlt = std::to_string(result.dAlt);
            TempsPathPoint.m_strHead = std::to_string(formatDeg2Rad(result.yaw));
          }
          Vpoints.push_back(TempsPathPoint);
        }
      }
    }
    SYSTEM_LOGGER_INFO("pb.sub_RoutingAction");
  }

  if (type == FollowTrajectoryAction_Type) {
    const auto& follow = pa.sub_RoutingAction->sub_FollowTrajectoryAction;
    if (follow) {
      osc::xsd::Attribute<osc::Trajectory> tra;
      if (follow->sub_CatalogReference) {
        int ret = ParseTrajectoryCataloggXosc(*follow->sub_CatalogReference->_entryName, *tra);
        if (ret == -1) return;
      } else if (follow->sub_Trajectory) {
        tra = pa.sub_RoutingAction->sub_FollowTrajectoryAction->sub_Trajectory;
      }
      if (tra->sub_Shape->sub_Polyline) {
        const auto& vs = tra->sub_Shape->sub_Polyline->sub_Vertex;
        auto iter = vs.begin();
        if (iter->sub_Position->sub_WorldPosition) {
          for (; iter != vs.end(); iter++) {
            sPathPoint TempsPathPoint;
            double _dLon = std::atof((*iter->sub_Position->sub_WorldPosition->_x).c_str());
            double _dLat = std::atof((*iter->sub_Position->sub_WorldPosition->_y).c_str());
            if (this->m_version != XOSC_VERSION::XOSC_1_0_VERSION_TAD_SIM_2_0) ConvertXODR2LonLat(_dLon, _dLat);
            TempsPathPoint.m_strLon = DoubleToString(_dLon);
            TempsPathPoint.m_strLat = DoubleToString(_dLat);
            const auto& z_o = iter->sub_Position->sub_WorldPosition->_z;
            if (z_o) {
              TempsPathPoint.m_strAlt = *z_o;
            }
            // 要在ParameterDeclarations中找到确切值
            if (TempsPathPoint.m_strLon[0] == '$') FindParameterDeclarationsValude(TempsPathPoint.m_strLon);

            // 要在ParameterDeclarations中找到确切值
            if (TempsPathPoint.m_strLat[0] == '$') FindParameterDeclarationsValude(TempsPathPoint.m_strLat);

            // 要在ParameterDeclarations中找到确切值
            if (TempsPathPoint.m_strAlt[0] == '$') FindParameterDeclarationsValude(TempsPathPoint.m_strAlt);

            Vpoints.push_back(TempsPathPoint);
          }

          // pedestrian velocity direction
        } else if (iter->sub_Position->sub_RelativeObjectPosition) {
          for (; iter != vs.end(); iter++) {
            const auto& orientation_h = iter->sub_Position->sub_RelativeObjectPosition->sub_Orientation->_h;
            if (orientation_h) {
              Orientation = (*orientation_h);
              if (this->m_version != XOSC_1_0_VERSION_TAD_SIM)
                Orientation = std::to_string(std::atof(Orientation.c_str()) * 180 / 3.141);
            }
          }
        } else if (iter->sub_Position->sub_LanePosition) {
          for (; iter != vs.end(); iter++) {
            sPathPoint TempsPathPoint;
            string strRoadid = *iter->sub_Position->sub_LanePosition->_roadId;
            int iRoadid = std::atoi(strRoadid.c_str());
            string strLaneid = *iter->sub_Position->sub_LanePosition->_laneId;
            int ilane = std::atoi(strLaneid.c_str());
            string strS = *iter->sub_Position->sub_LanePosition->_s;
            double dS = std::atof(strS.c_str());
            string strT = *iter->sub_Position->sub_LanePosition->_offset;
            double dT = std::atof(strT.c_str());
            sTagQueryInfoResult result;
            ConvertLonLat2SimPosition(iRoadid, ilane, dS, dT, result);
            double TempdLon = result.dLon;
            double TempdLat = result.dLat;
            TempsPathPoint.m_strLon = std::to_string(TempdLon);
            TempsPathPoint.m_strLat = std::to_string(TempdLat);
            TempsPathPoint.m_strAlt = std::to_string(result.dAlt);
            Vpoints.push_back(TempsPathPoint);
          }
        }
      }
    }
  }

  if (type == AcquirePositionAction_Type) {
    const auto& action = pa.sub_RoutingAction->sub_AcquirePositionAction;
    if (!action) {
      return;
    }
    sPathPoint TempsPathPoint;
    if (action->sub_Position->sub_RoadPosition) {
    } else if (action->sub_Position->sub_WorldPosition) {
      double _dLon = std::atof((*action->sub_Position->sub_WorldPosition->_x).c_str());
      double _dLat = std::atof((*action->sub_Position->sub_WorldPosition->_y).c_str());
      if (this->m_version != XOSC_VERSION::XOSC_1_0_VERSION_TAD_SIM_2_0) ConvertXODR2LonLat(_dLon, _dLat);
      TempsPathPoint.m_strLon = DoubleToString(_dLon);
      TempsPathPoint.m_strLat = DoubleToString(_dLat);
      const auto& z_o = action->sub_Position->sub_WorldPosition->_z;
      if (z_o) {
        TempsPathPoint.m_strAlt = *z_o;
      }
    }
    Vpoints.push_back(TempsPathPoint);
  }
}

void XOSCReader_1_0_v4::AddRoute(string& strEntityRef, sTagSimuTraffic& infos, vector<sPathPoint>& Vpoints) {
  if (Vpoints.size() < 1) {
    SYSTEM_LOGGER_ERROR("route point is zero!");
    return;
  }

  CRoute r;
  for (auto& p : Vpoints) {
    p.ConvertToValue();
    r.m_path.pushPoint(p);
  }

  r.m_path.ConvertToStr();
  //
  r.m_strID = std::to_string(m_nRouteIDGenerator);
  r.m_strType = "start_end";
  int nLen = r.m_path.m_vPoints.size();
  r.m_strStartLon = r.m_path.m_vPoints[0].m_strLon;
  r.m_strStartLat = r.m_path.m_vPoints[0].m_strLat;
  r.m_strStartAlt = r.m_path.m_vPoints[0].m_strAlt;
  r.m_strStart = r.m_strStartLon + "," + r.m_strStartLat + "," + r.m_strStartAlt;

  if (nLen > 1) {
    r.m_strEndLon = r.m_path.m_vPoints[nLen - 1].m_strLon;
    r.m_strEndLat = r.m_path.m_vPoints[nLen - 1].m_strLat;
    r.m_strEndAlt = r.m_path.m_vPoints[nLen - 1].m_strAlt;
    r.m_strEnd = r.m_strEndLon + "," + r.m_strEndLat + "," + r.m_strEndAlt;
  }

  for (int i = 1; i < nLen - 1; ++i) {
    std::string strOneP =
        r.m_path.m_vPoints[i].m_strLon + "," + r.m_path.m_vPoints[i].m_strLat + "," + r.m_path.m_vPoints[i].m_strAlt;
    if (i < nLen - 2) {
      strOneP += ";";
    }
    r.m_strMids += strOneP;
  }

  r.ExtractPath();
  r.m_path.ConvertToStr();
  // ego
  if (m_egoInfos.find(strEntityRef) != m_egoInfos.end()) {
    m_egoInfos.find(strEntityRef)->second.m_route = r;
    m_egoInfos.find(strEntityRef)->second.m_inputPath = r.m_path;
    m_egoInfos.find(strEntityRef)->second.m_strStartAlt = r.m_strStartAlt;
    m_egoInfos.find(strEntityRef)->second.m_strEndAlt = r.m_strEndAlt;

    m_nRouteIDGenerator++;
  } else {
    sTagQueryInfoResult result;
    ConvertLonLat2SimPosition(r.m_path.m_vPoints[0].m_dLon, r.m_path.m_vPoints[0].m_dLat, result);

    r.m_strStartLon = DoubleToString(result.dLon);
    r.m_strStartLat = DoubleToString(result.dLat);
    r.m_strStart = r.m_strStartLon + "," + r.m_strStartLat + "," + r.m_strStartAlt;
    // vehicle
    if (strEntityRef[0] == 'V' || m_mapVehicles.find(strEntityRef) != m_mapVehicles.end()) {
      auto itr = m_mapVehicles.find(strEntityRef);
      if (itr != m_mapVehicles.end()) {
        if (itr->second.m_strRouteID.size() < 1) {
          itr->second.m_strRouteID = std::to_string(m_nRouteIDGenerator++);
        }
        r.m_strID = itr->second.m_strRouteID;
        itr->second.m_strStartShift = std::to_string(result.dShift);
        itr->second.m_strOffset = std::to_string(result.dOffset);
        itr->second.m_strLaneID = std::to_string(result.laneID);
      }
    } else if (m_mapPedestrians.find(strEntityRef) != m_mapPedestrians.end() || strEntityRef[0] == 'P' ||
               strEntityRef[0] == 'A' || strEntityRef[0] == 'B' || strEntityRef[0] == 'M') {
      auto itr = m_mapPedestrians.find(strEntityRef);
      if (itr != m_mapPedestrians.end()) {
        if (itr->second.m_strRouteID.size() < 1) {
          itr->second.m_strRouteID = std::to_string(m_nRouteIDGenerator++);
        }
        r.m_strID = itr->second.m_strRouteID;
        itr->second.m_strStartShift = std::to_string(result.dShift);
        itr->second.m_strOffset = std::to_string(result.dOffset);
        itr->second.m_strLaneID = std::to_string(result.laneID);

      } else {
        SYSTEM_LOGGER_ERROR("pedestrian %s not exist", strEntityRef.c_str());
      }
    }
  }
  r.ConvertToValue();
  auto TTitr = m_mapXoscTrajectoryTracking.find(strEntityRef);
  if (TTitr != m_mapXoscTrajectoryTracking.end()) {
    r.m_strTrajectoryTracking_XOSC = TTitr->second;
    r.MergeXOSCString();
  }
  m_mapRoutes[r.m_strID] = r;
}

double XOSCReader_1_0_v4::formatRad2Deg(double rad) {
  double deg = rad * 180.0 / 3.1415;
  if (deg > 360) {
    deg -= 360;
  }
  if (deg < 0) {
    deg += 360;
  }
  return deg;
}
double XOSCReader_1_0_v4::formatDeg2Rad(double deg) { return deg * 3.14159 / 180.0; }

std::string XOSCReader_1_0_v4::formatOptRule(std::string rule) {
  if (rule == "eq" || rule == "equalTo") {
    return "eq";
  } else if (rule == "lt" || rule == "lessThan") {
    return "lt";
  } else if (rule == "gt" || rule == "greaterThan") {
    return "gt";
  } else if (rule == "lte") {
    return "lte";
  } else if (rule == "gte") {
    return "gte";
  } else {
    SYSTEM_LOGGER_ERROR("Not Support Opt rule :%s", rule);
    return "eq";
  }
}

std::string XOSCReader_1_0_v4::getEntryRef(const std::string name) {
  int index = 1;
  if (CompareString(name, "ego")) return "ego_1";
  for (auto it : this->m_egoInfos) {
    if (CompareString(it.first, name)) {
      return std::string("ego_") + std::to_string(index);
    }
  }
  index = 1;
  for (auto it : this->m_mapVehicles) {
    if (CompareString(it.first, name)) {
      return std::string("v_") + std::to_string(index);
    }
    index++;
  }
  for (auto it : this->m_mapPedestrians) {
    if (CompareString(it.first, name)) {
      return std::string("p_") + std::to_string(index);
    }
    index++;
  }
  for (auto it : this->m_mapObstacles) {
    if (CompareString(it.first, name)) {
      return std::string("o_") + std::to_string(index);
    }
    index++;
  }
  return "v_1";
}

bool XOSCReader_1_0_v4::CompareString(std::string str1, std::string str2) {
  std::transform(str1.begin(), str1.end(), str1.begin(), [](unsigned char c) { return std::tolower(c); });
  std::transform(str2.begin(), str2.end(), str2.begin(), [](unsigned char c) { return std::tolower(c); });
  return (str1 == str2);
}

int XOSCReader_1_0_v4::ParsePrivateAction(osc::PrivateAction& pa, sTagSimuTraffic& infos, string& strEntityRef,
                                          string& strvelocity, string& strAccValue, bool& isOver_V,
                                          string& strAccOverValue, string& strMergeValue, string& strMergeOffset,
                                          string& strMergeOverTime, std::string& strOrientation,
                                          SceneEvent& singleSceneEvent, std::string actionName, bool isInit) {
  Scene_Action Scene_ActionTemp;
  Scene_ActionTemp.strName = actionName;
  Scene_EndCondition Scene_EndConditionTemp;
  // 初始速度
  if (pa.sub_LongitudinalAction) {
    if (pa.sub_LongitudinalAction->sub_SpeedAction) {
      if (pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionTarget->sub_AbsoluteTargetSpeed) {
        string strSpeedV, strV, strDynamicsShape, strDynamicsDimension;
        ReadLongitudinalSpeedAbsolute(pa, strDynamicsShape, strV, strDynamicsDimension, strSpeedV);
        if (isInit) {
          AddIniSpeed(strEntityRef, infos, strSpeedV);
          return 0;
        }
        if (strDynamicsShape == "linear") {
          strAccValue = strV;
          strAccOverValue = strSpeedV;
          isOver_V = true;  // 以速度终止
          Scene_ActionTemp.strChecked = "true";
          Scene_ActionTemp.strType = "acc";
          Scene_ActionTemp.strValue = strV;
          Scene_ActionTemp.strSubType = "none";
          Scene_ActionTemp.strLaneOffset = "0";
          if (strAccOverValue != "-1.0000") {
            Scene_EndConditionTemp.strType = "velocity";
            Scene_EndConditionTemp.strValue = strSpeedV;
          } else {
            Scene_EndConditionTemp.strType = "none";
            Scene_EndConditionTemp.strValue = "0";
          }
        } else if (strDynamicsShape == "step") {
          strvelocity = strSpeedV;  // 跳转到这个速度
          Scene_ActionTemp.strChecked = "true";
          Scene_ActionTemp.strType = "velocity";
          Scene_ActionTemp.strValue = strSpeedV;
          Scene_ActionTemp.strSubType = "none";
          Scene_ActionTemp.strLaneOffset = "0";
          Scene_EndConditionTemp.strType = "none";
          Scene_EndConditionTemp.strValue = "0";
        }
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      } else if (pa.sub_LongitudinalAction->sub_SpeedAction->sub_SpeedActionTarget->sub_RelativeTargetSpeed) {
        string strSpeedV, strV, strDynamicsShape, strDynamicsDimension;
        ReadLongitudinalSpeedRelative(pa, strDynamicsShape, strV, strDynamicsDimension, strSpeedV);
        if (strDynamicsShape == "linear") {
          strAccValue = strV;
          double T = stod(strSpeedV) / stod(strAccValue);
          strAccOverValue = std::to_string(T);  // 以时间终止
          isOver_V = false;
          Scene_ActionTemp.strChecked = "true";
          Scene_ActionTemp.strType = "acc";
          Scene_ActionTemp.strValue = strV;
          Scene_ActionTemp.strSubType = "none";
          Scene_ActionTemp.strLaneOffset = "0.0";
          Scene_EndConditionTemp.strType = "time";
          Scene_EndConditionTemp.strValue = strAccOverValue;
        }
        singleSceneEvent.isUserDefined = true;
        singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
        singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
      }
    }
    return 0;
  }
  // 初始位置
  if (pa.sub_TeleportAction) {
    if (pa.sub_TeleportAction->sub_Position->sub_WorldPosition) {
      double dH, dX, dY, dZ;
      ReadTeleportAction_WorldPosition(pa, dX, dY, dZ, dH);
      if (isInit) {
        IniWordPos(strEntityRef, infos, dX, dY, dZ, dH);
      }
    } else if (pa.sub_TeleportAction->sub_Position->sub_RelativeWorldPosition) {
      double dH, dX, dY, dZ;
      ReadTeleActionRelWorldPos(pa, dX, dY, dZ, dH);
      if (isInit) {
        IniWordPos(strEntityRef, infos, dX, dY, dZ, dH);
      }
    } else if (pa.sub_TeleportAction->sub_Position->sub_LanePosition) {
      hadmap::roadpkid iRoadid;
      hadmap::lanepkid iLaneid;
      double dS, dOffset, yaw = -9999;
      ReadTeleportAction_LanePosition(pa, iRoadid, iLaneid, dS, dOffset, yaw);
      if (isInit) {
        IniLanePos(strEntityRef, infos, iRoadid, iLaneid, dS, dOffset, yaw);
      }
    } else if (pa.sub_TeleportAction->sub_Position->sub_RelativeLanePosition) {
      hadmap::roadpkid iRoadid;
      hadmap::lanepkid iLaneid;
      double dS, dOffset;
      ReadTeleActionRelLanePos(pa, iRoadid, iLaneid, dS, dOffset);
      if (isInit) {
        IniLanePos(strEntityRef, infos, iRoadid, iLaneid, dS, dOffset);
      }
    } else if (pa.sub_TeleportAction->sub_Position->sub_RoadPosition) {
      hadmap::roadpkid iRoadid;
      double dS, dT;
      ReadTeleportAction_RoadPosition(pa, iRoadid, dS, dT);
      if (isInit) {
        IniRoadPos(strEntityRef, infos, iRoadid, dS, dT);
      }
    } else if (pa.sub_TeleportAction->sub_Position->sub_RelativeRoadPosition) {
      hadmap::roadpkid iRoadid;
      double dS, dT;
      ReadTeleActionRelRoadPos(pa, iRoadid, dS, dT);
      if (isInit) {
        IniRoadPos(strEntityRef, infos, iRoadid, dS, dT);
      }
    }

    return 0;
  }
  // 换道动作
  if (pa.sub_LateralAction) {
    if (pa.sub_LateralAction->sub_LaneChangeAction) {
      string strTargetLane, strV, strDynamicsShape, strDynamicsDimension;
      ReadLateralAction_LaneChangeAction(pa, strDynamicsShape, strV, strDynamicsDimension, strTargetLane,
                                         strMergeOffset);
      strMergeOverTime = strV;
      strMergeValue = strTargetLane;
      Scene_ActionTemp.strChecked = "true";
      Scene_ActionTemp.strType = "merge";
      Scene_ActionTemp.strValue = strV;
      Scene_ActionTemp.strLaneOffset = strMergeOffset;
      if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) > 0) {
        Scene_ActionTemp.strSubType = "right_in_lane";
      }
      if (strTargetLane == "0" && std::atof(strMergeOffset.c_str()) < 0) {
        Scene_ActionTemp.strSubType = "left_in_lane";
        if (!strMergeOffset.empty()) Scene_ActionTemp.strLaneOffset = strMergeOffset.substr(1);
      }
      if (std::atoi(strTargetLane.c_str()) > 0) {
        Scene_ActionTemp.strSubType = "right";
      }
      if (std::atoi(strTargetLane.c_str()) < 0) {
        Scene_ActionTemp.strSubType = "left";
      }
      if (Scene_ActionTemp.strLaneOffset.empty()) {
        Scene_ActionTemp.strLaneOffset = "0";
      }
      Scene_EndConditionTemp.strType = "none";
      Scene_EndConditionTemp.strValue = "0.0";
      singleSceneEvent.isUserDefined = true;
      singleSceneEvent.ActionVer.push_back(Scene_ActionTemp);
      singleSceneEvent.EndConditionVer.push_back(Scene_EndConditionTemp);
    }
    return 0;
  }
  // 路径
  if (pa.sub_RoutingAction) {
    SYSTEM_LOGGER_INFO("pa.sub_RoutingAction");
    // planner and vehicle
    if (pa.sub_RoutingAction->sub_AssignRouteAction) {
      vector<sPathPoint> TempPoints;
      ReadRoutingAction(pa, AssignRouteAction_Type, TempPoints, strOrientation);
      AddRoute(strEntityRef, infos, TempPoints);
    } else if (pa.sub_RoutingAction->sub_FollowTrajectoryAction) {
      vector<sPathPoint> TempPoints;
      ReadRoutingAction(pa, FollowTrajectoryAction_Type, TempPoints, strOrientation);
      if (TempPoints.size() > 0) {
        AddRoute(strEntityRef, infos, TempPoints);
      }
    }
    return 0;
  }
  // 激活控制器
  if (pa.sub_ActivateControllerAction) {
    if (isInit) {
      if (*pa.sub_ActivateControllerAction->_lateral == ("true")) {
        infos.m_simulation.m_planner.m_controlLateral = "true";
      } else {
        infos.m_simulation.m_planner.m_controlLateral = "false";
      }
      if (*pa.sub_ActivateControllerAction->_longitudinal == string("true")) {
        infos.m_simulation.m_planner.m_controlLongitudinal = "true";
      } else {
        infos.m_simulation.m_planner.m_controlLongitudinal = "false";
      }
    }
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseParameterDeclaration(osc::ParameterDeclarations& pas) {
  m_ParameterDeclarations.clear();

  const auto& pds = pas.sub_ParameterDeclaration;
  for (auto pd : pds) {
    std::string strName = *pd._name;
    std::string strValue = *pd._value;
    std::string strType = *pd._parameterType;
    Type3_Value temp;
    temp._parameterType = strType;
    temp._vaule = strValue;
    m_ParameterDeclarations.insert(make_pair(strName, temp));
  }
  if (m_ParameterDeclarations.find("ActualMap") != m_ParameterDeclarations.end()) {
    m_actualMap = m_ParameterDeclarations.find("ActualMap")->second._vaule;
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseRoadNetWork(osc::RoadNetwork& rn, sTagSimuTraffic& infos) {
  const auto& lfo = rn.sub_LogicFile;
  if (infos.m_simulation.m_mapFile.m_strMapfile.empty() && lfo) {
    std::string strRoadPath = *lfo->_filepath;
    if (strRoadPath[0] == '$') FindParameterDeclarationsValude(strRoadPath);
    boost::filesystem::path p = strRoadPath;
    std::string strMapName = p.filename().string();
    infos.m_simulation.m_mapFile.m_strMapfile = "../hadmap/" + strMapName;
  }
  if (!infos.m_simulation.m_mapFile.m_strMapfile.empty()) {
    int nRet = InitMapQuery(infos.m_simulation);
    if (nRet != 0) {
      SYSTEM_LOGGER_INFO("fail to init map query %s", infos.m_simulation.m_mapFile.m_strMapfile.c_str());
      return nRet;
    }
    XODRCoordinator(infos.m_simulation);
  }
  const auto& tso = rn.sub_TrafficSignals;
  if (tso) {
    const auto& tscs = tso->sub_TrafficSignalController;
    if (m_ParameterDeclarations.find("$ActivePlan") != m_ParameterDeclarations.end()) {
      infos.m_traffic.m_ActivePlan = m_ParameterDeclarations.find("$ActivePlan")->second._vaule;
      infos.m_traffic.isConfigTraffic = true;
      for (auto itr : tscs) {
        string strid = *itr._name;
        const auto& ps = itr.sub_Phase;
        CSignLight temp;
        temp.m_strID = strid;
        CRoute route;
        route.m_strType = "start_end";
        std::string RouteStr = "$TrafficInfo_" + temp.m_strID;

        if (m_ParameterDeclarations.find(RouteStr) != m_ParameterDeclarations.end()) {
          std::string value = m_ParameterDeclarations.find(RouteStr)->second._vaule;
          int size = value.size();
          std::string::size_type pos;
          std::vector<std::string> result;
          for (int i = 0; i < size; i++) {
            pos = value.find(";", i);
            if (pos < size) {
              std::string s = value.substr(i, pos - i);
              if (!s.empty()) result.push_back(s);
              i = pos;
            }
          }
          std::map<std::string, std::string> dataMap;
          for (auto it : result) {
            int pos = it.find(":");
            std::string key = it.substr(0, pos);
            std::string value = it.substr(pos + 1, it.size() - pos - 1);
            dataMap[key] = value;
          }
          if (dataMap.find("routeid") != dataMap.end()) {
            route.m_strID = dataMap.find("routeid")->second;
            temp.m_strRouteID = route.m_strID;
          }
          if (dataMap.find("route") != dataMap.end()) {
            route.m_strStart = dataMap.find("route")->second;
          }
          if (dataMap.find("junctionid") != dataMap.end()) {
            temp.m_strJunction = dataMap.find("junctionid")->second;
          }
          if (dataMap.find("l_offset") != dataMap.end()) {
            temp.m_strOffset = dataMap.find("l_offset")->second;
          }
          if (dataMap.find("phase") != dataMap.end()) {
            string phase = dataMap.find("phase")->second;
            for (string::size_type pos(0); pos != string::npos; pos++) {
              if ((pos = phase.find(",", pos)) != string::npos) {
                phase.replace(pos, 1, ";");
              } else {
                break;
              }
            }
            temp.m_strPhase = phase;
          }
          if (dataMap.find("status") != dataMap.end()) {
            temp.m_strStatus = dataMap.find("status")->second;
          }
          if (dataMap.find("plan") != dataMap.end()) {
            temp.m_strPlan = dataMap.find("plan")->second;
          }
          if (dataMap.find("phaseNumber") != dataMap.end()) {
            temp.m_strPhasenumber = dataMap.find("phaseNumber")->second;
          }
          if (dataMap.find("start_t") != dataMap.end()) {
            temp.m_strStartTime = dataMap.find("start_t")->second;
          }
          if (dataMap.find("lane") != dataMap.end()) {
            string lane = dataMap.find("lane")->second;
            for (string::size_type pos(0); pos != string::npos; pos++) {
              if ((pos = lane.find(",", pos)) != string::npos) {
                lane.replace(pos, 1, ";");
              } else {
                break;
              }
            }
            temp.m_strLane = lane;
          }
          temp.m_strStartShift = "0";
          infos.m_traffic.m_mapSignlights[strid] = temp;
          if (m_mapRoutes.find(route.m_strID) == m_mapRoutes.end())
            m_mapRoutes.insert(std::make_pair(route.m_strID, route));
        }
        int index = 0;
        for (auto itp : ps) {
          if (*itp._name == "stop") temp.m_strTimeRed = *itp._duration;
          if (*itp._name == "go") temp.m_strTimeGreen = *itp._duration;
          if (*itp._name == "attention") temp.m_strTimeYellow = *itp._duration;
          if (index == 0) {
            for (auto itState : itp.sub_TrafficSignalState) {
              temp.m_strSignalheadVec.push_back(*itState._trafficSignalId);
            }
          }
          index++;
        }
        m_mapTrafficSingals.insert(make_pair(strid, temp));
      }
    } else {
      std::map<std::string, CSignLight> m_id2Light;
      const auto& tscs = tso->sub_TrafficSignalController;
      if (tso) {
        infos.m_traffic.m_ActivePlan = "1";
        infos.m_traffic.isConfigTraffic = true;

        for (auto itr : tscs) {
          CSignLight temp;
          temp.Reset();
          string strid = *itr._name;

          const auto& ps = itr.sub_Phase;
          // get phase time
          for (auto itp : ps) {
            if (*itp._name == "red") {
              temp.m_strTimeRed = *itp._duration;
            }
            if (*itp._name == "green" || *itp._name == "go") {
              temp.m_strTimeGreen = *itp._duration;
            }
            if (*itp._name == "yellow" || *itp._name == "blink") {
              temp.m_strTimeYellow = *itp._duration;
            }
          }
          auto phase = ps.begin();
          // get control road and junction
          std::map<std::string, std::vector<std::string>> m_roadId2trafficLights;
          std::map<std::string, hadmap::txPoint> m_roadEndPoint;
          std::map<std::string, std::string> m_roadRouteId;
          std::string junctionid;
          for (auto itState : phase->sub_TrafficSignalState) {
            std::string id = *itState._trafficSignalId;
            std::string phase;
            std::string lane;
            std::string junction;
            std::string strRoadId;
            std::string routeId;
            hadmap::txPoint _point;
            if (GetTrafficLightInfo(id, routeId, phase, lane, junction, strRoadId, _point) == -1) continue;
            junctionid = junction;
            if (m_roadId2trafficLights.find(strRoadId) != m_roadId2trafficLights.end()) {
              m_roadEndPoint.find(strRoadId)->second = _point;
              m_roadId2trafficLights.find(strRoadId)->second.push_back(id);
              m_roadRouteId[strRoadId] = routeId;
            } else {
              std::vector<std::string> tmp;
              tmp.push_back(id);
              m_roadId2trafficLights[strRoadId] = tmp;
              m_roadEndPoint[strRoadId] = _point;
              m_roadRouteId[strRoadId] = routeId;
            }
          }
          int nPhase = 0;
          for (auto itSignalLight : m_roadId2trafficLights) {
            temp.m_strID = m_roadRouteId[itSignalLight.first];
            temp.m_strRouteID = m_roadRouteId[itSignalLight.first];
            hadmap::txPoint _point = m_roadEndPoint[itSignalLight.first];
            // route
            CRoute route;
            route.m_strType = "start_end";
            route.m_strID = m_roadRouteId[itSignalLight.first];
            route.m_strStart = std::to_string(_point.x) + "," + std::to_string(_point.y) + ",0";
            route.ConvertToValue();

            // junction
            temp.m_strJunction = junctionid;
            temp.m_strOffset = "0";
            temp.m_strPlan = "1";
            temp.m_strStartShift = "0";
            temp.m_strPhasenumber = std::to_string(nPhase++);
            temp.m_strStartTime = "0";
            for (auto itll : itSignalLight.second) {
              temp.m_strSignalheadVec.push_back(itll);
            }

            infos.m_traffic.m_mapSignlights[route.m_strID] = temp;

            // routes
            if (m_mapRoutes.find(route.m_strID) == m_mapRoutes.end())
              m_mapRoutes.insert(std::make_pair(route.m_strID, route));

            m_mapTrafficSingals.insert(make_pair(temp.m_strID, temp));
          }
        }
      }
      return 0;
    }
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseRoadNetWorkCloud(osc::RoadNetwork& rn, const char* strMapename, sTagSimuTraffic& infos) {
  const auto& lfo = rn.sub_LogicFile;
  if (lfo) {
    std::string strRoadPath = *lfo->_filepath;

    if (strRoadPath[0] == '$') FindParameterDeclarationsValude(strRoadPath);

    boost::filesystem::path p = strRoadPath;
    std::string strMapName = p.filename().string();

    infos.m_simulation.m_mapFile.m_strMapfile = "../hadmap/" + strMapName;

    int nRet = InitMapQueryCloud(infos.m_simulation, strMapename);
    if (nRet != 0) {
      return nRet;
    }
  }

  const auto& tso = rn.sub_TrafficSignals;
  if (tso) {
    const auto& tscs = tso->sub_TrafficSignalController;
    if (m_ParameterDeclarations.find("$ActivePlan") != m_ParameterDeclarations.end()) {
      infos.m_traffic.m_ActivePlan = m_ParameterDeclarations.find("$ActivePlan")->second._vaule;
    }
    for (auto itr : tscs) {
      string strid = *itr._name;
      const auto& ps = itr.sub_Phase;
      CSignLight temp;
      temp.m_strID = strid;
      std::string RouteStr = "$RouteInfo_" + temp.m_strID;
      if (m_ParameterDeclarations.find("RouteStr") != m_ParameterDeclarations.end()) {
        std::string value = m_ParameterDeclarations.find("RouteStr")->second._vaule;
        int size = value.size();
        std::string::size_type pos;
        std::vector<std::string> result;
        for (int i = 0; i < size; i++) {
          pos = value.find(";", i);
          if (pos < size) {
            std::string s = value.substr(i, pos - i);
            if (!s.empty()) result.push_back(s);
            i = pos;
          }
        }
        std::map<std::string, std::string> dataMap;
        for (auto it : result) {
          int pos = it.find(":");
          std::string key = it.substr(0, pos);
          std::string value = it.substr(pos + 1, it.size() - pos - 1);
          dataMap[key] = value;
        }
        if (dataMap.find("route") != dataMap.end()) {
          CRoute route;
          route.m_strID = strid;
          route.m_strType = "start_end";
          route.m_strStart = dataMap.find("route")->second;
          m_mapRoutes.insert(std::make_pair(route.m_strID, route));
        }
        if (dataMap.find("junctionid") != dataMap.end()) {
          temp.m_strJunction = dataMap.find("junctionid")->second;
        }
        if (dataMap.find("l_offset") != dataMap.end()) {
          temp.m_strOffset = dataMap.find("l_offset")->second;
        }
        if (dataMap.find("phase") != dataMap.end()) {
          temp.m_strPhase = dataMap.find("phase")->second;
        }
        if (dataMap.find("status") != dataMap.end()) {
          temp.m_strStatus = dataMap.find("status")->second;
        }
        if (dataMap.find("plan") != dataMap.end()) {
          temp.m_strPlan = dataMap.find("plan")->second;
        }
        if (dataMap.find("phaseNumber") != dataMap.end()) {
          temp.m_strPhasenumber = dataMap.find("phaseNumber")->second;
        }
        temp.m_strStartShift = "0";
        infos.m_traffic.m_mapSignlights[strid] = temp;
      }
      int index = 0;
      for (auto itp : ps) {
        if (*itp._name == "stop") temp.m_strTimeRed = *itp._duration;
        if (*itp._name == "go") temp.m_strTimeGreen = *itp._duration;
        if (*itp._name == "attention") temp.m_strTimeYellow = *itp._duration;
        if (index == 0) {
          for (auto itState : itp.sub_TrafficSignalState) {
            temp.m_strSignalheadVec.push_back(*itState._trafficSignalId);
          }
        }
        index++;
      }
      m_mapTrafficSingals.insert(make_pair(strid, temp));
    }
  }

  return 0;
}

bool XOSCReader_1_0_v4::MakeSure_P(string& strType) {
  if (m_set_PedestrianType.find(strType) != m_set_PedestrianType.end()) {
    return true;
  }
  return false;
}

int XOSCReader_1_0_v4::ParseEntities(osc::Entities& entities, sTagSimuTraffic& infos) {
  for (auto& scenarioObject : entities.sub_ScenarioObject) {
    std::string strName = *scenarioObject._name;
    string strStartV = "0", strMaxV = "0", strGrading = "", strBehavior = "UserDefine", strAngle = "0.0";
    std::string strMaxAcc = "0";
    std::string strMaxDec = " 0";
    string strOriginFile = "", strGenerateInfo = "";
    const auto& objV = scenarioObject.sub_Vehicle;
    // check entity type
    bool bIsEgoFromProperty = false;
    if (objV) {
      strMaxV = *objV->sub_Performance->_maxSpeed;
      strMaxAcc = *objV->sub_Performance->_maxAcceleration;
      strMaxDec = *objV->sub_Performance->_maxDeceleration;
      const auto& pts = objV->sub_Properties->sub_Property;
      for (auto itr : pts) {
        if (*itr._name == "control" && *itr._value == "external") {
          bIsEgoFromProperty = true;
          break;
        }
      }
    }
    // 主车特殊处理下
    if (strName == "Ego" || strName == "ego" || bIsEgoFromProperty == true) {
      ParseOneEgoInfo(scenarioObject, infos.getCatalogDir());
    } else {
      // vechile
      if (scenarioObject.sub_CatalogReference) {
        GetCatalogReference(scenarioObject, infos);
        continue;
      }
      const auto& objV = scenarioObject.sub_Vehicle;
      if (objV) {
        strMaxV = *objV->sub_Performance->_maxSpeed;
        std::string strName = *objV->_name;
        string strType = strName;
        bool isPedestrianType = MakeSure_P(strType);
        string strMathPeName = this->MatchName("VehicleCatalog", strName);
        if (isPedestrianType || MakeSure_P(strMathPeName)) {
          strType = strMathPeName;
          const auto& pts = objV->sub_Properties->sub_Property;
          std::string strTrajectoryTracking;
          for (auto itr : pts) {
            if (*itr._name == "Behavior") {
              strBehavior = *itr._value;
            }
            if (*itr._name == "TrajectoryTracking") {
              strTrajectoryTracking = *itr._value;
            }
          }
          strType = this->MatchName("PedestrianCatalog", strName);
          CPedestrianV2 pa;
          pa.m_strName = *scenarioObject._name;
          pa.m_strID = std::to_string(m_nPedestrianIDGenerator++);
          pa.m_strBehavior = strBehavior;
          pa.m_strType = strType;

          pa.m_strStartTime = "0";
          pa.m_strEndTime = "1000";

          pa.m_strStartVelocity = strStartV;
          pa.m_strMaxVelocity = strMaxV;
          if (strTrajectoryTracking.size() > 0)
            m_mapXoscTrajectoryTracking.insert(make_pair(pa.m_strName, strTrajectoryTracking));

          m_mapPedestrians.insert(make_pair(pa.m_strName, pa));
        } else {
          CCatalogModelPtr single;
          if (CCataLog::getCatalogName(infos.getCatalogDir().c_str(), CATALOG_TYPE_VEHICLE, strType, single) != 0) {
            strType = this->MatchName("VehicleCatalog", strName);
            if (CCataLog::getCatalogName(infos.getCatalogDir().c_str(), CATALOG_TYPE_VEHICLE, strType, single) == 0)
              strMaxV = single->getPeformance().getMaxSpeed();
          }
          const auto& pts = objV->sub_Properties->sub_Property;
          string strAggress = "", strFollow = "", strEventID = "", strTrajectoryAngle = "", strTrajectoryTracking = "";
          CVehicle v;
          for (auto itr : pts) {
            if (*itr._name == "Behavior") {
              strBehavior = *itr._value;
            }
            if (*itr._name == "Aggress") {
              strAggress = *itr._value;
            }
            if (*itr._name == "Follow") {
              strFollow = *itr._value;
            }
            if (*itr._name == "EventID") {
              strEventID = *itr._value;
            }
            if (*itr._name == "TrajectoryAngle") {
              strTrajectoryAngle = *itr._value;
            }
            if (*itr._name == "TrajectoryTracking") {
              strTrajectoryTracking = *itr._value;
            }
            if (*itr._name == "SensorGroup") {
              v.m_strSensorGroup = *itr._value;
            }
            if (*itr._name == "ObuStatus") {
              v.m_strObuStauts = *itr._value;
            }
          }

          v.m_strName = *scenarioObject._name;
          v.m_strID = std::to_string(m_nVehiclIDGeneartor++);
          v.m_strType = strType;
          v.m_strStartTime = "0";
          v.m_strStartVelocity = strStartV;
          if (!strMaxV.empty()) v.m_strMaxVelocity = strMaxV;
          v.m_strBehavior = strBehavior;

          if (strAggress.size() > 0) v.m_strAggress = strAggress;
          if (strFollow.size() > 0) v.m_strFollow = strFollow;
          // if (strEventID.size() > 0) v.m_strEventId = strEventID;
          if (strTrajectoryAngle.size() > 0) v.m_strAngle = strTrajectoryAngle;
          if (strTrajectoryTracking.size() > 0)
            m_mapXoscTrajectoryTracking.insert(make_pair(v.m_strName, strTrajectoryTracking));

          const auto& vop = scenarioObject.sub_Vehicle;
          if (vop) {
            v.m_boundingBox.m_strHeight = *vop->sub_BoundingBox->sub_Dimensions->_height;
            v.m_boundingBox.m_strWidth = *vop->sub_BoundingBox->sub_Dimensions->_width;
            v.m_boundingBox.m_strLength = *vop->sub_BoundingBox->sub_Dimensions->_length;
          } else {
            v.m_boundingBox.m_strHeight = "1.44";
            v.m_boundingBox.m_strWidth = "2.05";
            v.m_boundingBox.m_strLength = "4.76";
          }
          m_mapVehicles.insert(std::make_pair(v.m_strName, v));
        }
      }

      // misobject
      const auto& objM = scenarioObject.sub_MiscObject;
      if (objM) {
        CObstacle O;
        O.m_strName = *scenarioObject._name;
        O.m_strID = std::to_string(m_nObstacleIDGenerator++);
        if (!O.GetDefaultObstacle(*objM->_name, infos.getCatalogDir())) {
          O.m_strType = this->MatchName("MiscObjectCatalog", *objM->_name);
        }
        const auto& mop = scenarioObject.sub_MiscObject;
        if (mop) {
          O.m_boundingBox.m_strHeight = *mop->sub_BoundingBox->sub_Dimensions->_height;
          O.m_boundingBox.m_strWidth = *mop->sub_BoundingBox->sub_Dimensions->_width;
          O.m_boundingBox.m_strLength = *mop->sub_BoundingBox->sub_Dimensions->_length;
        } else {
          O.m_boundingBox.m_strHeight = "0.75";
          O.m_boundingBox.m_strWidth = "0.75";
          O.m_boundingBox.m_strLength = "0.75";
        }
        m_mapObstacles.insert(make_pair(O.m_strName, O));
      }

      // pedestrian
      const auto& objP = scenarioObject.sub_Pedestrian;
      if (objP) {
        strMaxV = "-1";
        std::string strTrajectoryTracking;
        strBehavior = "TrajectoryFollow";
        const auto& pts = objP->sub_Properties->sub_Property;
        for (auto itr : pts) {
          if (*itr._name == "StartV") {
            strStartV = *itr._value;
          }
          if (*itr._name == "MaxV") {
            strMaxV = *itr._value;
          }
          if (*itr._name == "Behavior") {
            strBehavior = *itr._value;
          }
          if (*itr._name == "Angle") {
            strAngle = *itr._value;
          }
          if (*itr._name == "TrajectoryTracking") {
            strTrajectoryTracking = *itr._value;
          }
        }
        if (strMaxV == "-1") {
          strMaxV = "10";
        }
        CPedestrianV2 pa;
        pa.m_strName = *scenarioObject._name;
        pa.m_strID = std::to_string(m_nPedestrianIDGenerator++);
        pa.m_strBehavior = strBehavior;
        pa.m_strBehavior = "UserDefine";
        pa.m_strType = this->MatchName("PedestrianCatalog", *objP->_name);
        pa.m_strBehavior = strBehavior;
        pa.m_strStartTime = "0";
        pa.m_strEndTime = "1000";
        if (strTrajectoryTracking.size() > 0)
          m_mapXoscTrajectoryTracking.insert(make_pair(pa.m_strName, strTrajectoryTracking));

        pa.m_strStartVelocity = strStartV;
        pa.m_strMaxVelocity = strMaxV;
        pa.m_strAngle = strAngle;
        m_mapPedestrians.insert(make_pair(pa.m_strName, pa));
      }
    }
  }
  return 0;
}

int XOSCReader_1_0_v4::ParseStoryboard(osc::Storyboard& sb, sTagSimuTraffic& infos) {
  // 1) init
  const auto& i = sb.sub_Init;
  const auto& ias = i->sub_Actions;
  const auto& gba = ias->sub_GlobalAction;
  for (auto p : gba) {
    // trafficlight
    const auto& iao = p.sub_InfrastructureAction;
    if (iao) {
      const auto& tsa = iao->sub_TrafficSignalAction;
      if (!tsa->sub_TrafficSignalControllerAction) continue;
      string strid = *tsa->sub_TrafficSignalControllerAction->_trafficSignalControllerRef;
      auto itf = m_mapTrafficSingals.find(strid);
      if (itf != m_mapTrafficSingals.end()) {
        itf->second.m_strStartTime = *tsa->sub_TrafficSignalControllerAction->_phase;
      }
    }

    // environment
    const auto& environment = p.sub_EnvironmentAction;
    if (environment) {
      ParseEnvironmentAction(*environment, infos);
    }
  }

  const auto& ps = ias->sub_Private;
  bool isInit = true;
  for (auto p : ps) {
    std::string strEntityRef = *p._entityRef;
    string strvelocity = "null", str2 = "null", str3 = "null", str4 = "null", str5 = "null", str6 = "null",
           str7 = "null";
    string strOffset;
    string actionName;
    bool is1, is2, is3;
    const auto& pas = p.sub_PrivateAction;
    SceneEvent singleSceneEventNull;
    m_isRouteAction = false;
    for (auto pa : pas) {
      if (pa.sub_RoutingAction) {
        m_isRouteAction = true;
      }
    }
    for (auto pa : pas) {
      ParsePrivateAction(pa, infos, strEntityRef, strvelocity, str2, is1, str3, str4, str5, str6, str7,
                         singleSceneEventNull, actionName, isInit);
    }
    // if ego finish read, set route to TrajectoryTracking
    if (m_egoInfos.find(strEntityRef) != m_egoInfos.end()) {
      CEgoInfo& egoinfo = m_egoInfos.find(strEntityRef)->second;
      if (egoinfo.m_trajectoryEnabled == "true") {
        egoinfo.m_controlPath.setControlPointLonLat(egoinfo.m_inputPath);
      }
    }
  }
  isInit = false;

  // parseDeclarations
  std::map<std::string, std::string> name2Value;
  // 2)story
  const auto& ss = sb.sub_Story;
  for (auto s : ss) {
    if (s.sub_ParameterDeclarations) {
      for (auto itParam : s.sub_ParameterDeclarations->sub_ParameterDeclaration) {
        std::string name = *itParam._name;
        std::string value = *itParam._value;
        if (value != "") {
          name2Value.insert(std::make_pair(name, value));
        }
      }
    }
    std::string strStoryName = *s._name;
    const auto& as = s.sub_Act;
    for (auto a : as) {
      int nindexSceneEvent = 0;
      int nSimIndexEvent = 0;
      std::string strActName = *a._name;
      const auto& mgs = a.sub_ManeuverGroup;
      for (auto mg : mgs) {
        std::string strManuverName = *mg._name;
        std::string strExeTimes = *mg._maximumExecutionCount;
        const auto& at = mg.sub_Actors;
        std::string strEntityCount = *at->_selectTriggeringEntities;

        std::string strEntityName;
        const auto& ers = at->sub_EntityRef;
        for (auto er : ers) {
          strEntityName = *er._entityRef;
        }

        osc::xsd::Vector<osc::Maneuver> ms = mg.sub_Maneuver;
        if (mg.sub_CatalogReference.size() > 0) {
          std::vector<std::string> ManuerNames;
          for (auto itRefer : mg.sub_CatalogReference) {
            ManuerNames.push_back(*itRefer._entryName);
          }
          osc::xsd::Vector<osc::Maneuver> maneuvers;
          this->ParseManeuversCatalogXosc(ManuerNames, maneuvers);
          for (auto itMauver : maneuvers) {
            ms.push_back(itMauver);
          }
        }
        for (auto m : ms) {
          std::string strManeuverName = *m._name;
          const auto& es = m.sub_Event;
          for (auto e : es) {
            SceneEvent singleSceneEventTemp;
            singleSceneEventTemp.strName = *e._name;
            const auto& macCount = *e._maximumExecutionCount;
            std::string strEventPriority = *e._priority;
            const auto& as = e.sub_Action;
            const auto& st = e.sub_StartTrigger;
            int ntype = -7;
            string TrigerValue = "";
            string DistanceType = "";
            string strAlongRoute = "";

            // ParseStartTrigge
            std::string _entry = strEntityName;
            if (!_entry.empty()) {
              std::transform(_entry.begin(), _entry.begin() + 1, _entry.begin(), ::tolower);
            }
            int ret =
                ParseStartTrigger(*st, ntype, TrigerValue, DistanceType, strAlongRoute, singleSceneEventTemp, _entry);
            if (ret == -1) {
              SYSTEM_LOGGER_ERROR("Find tadsim not support trigger type");
              continue;
            }
            int index = 0;
            for (auto& it : singleSceneEventTemp.ConditionVec) {
              // if is ego
              std::string key = "";
              if (m_egoInfos.find(strEntityName) != m_egoInfos.end()) {
                CEgoInfo& egoinfo = m_egoInfos.find(strEntityName)->second;
                key.append("$ego_triggercount$" + singleSceneEventTemp.strName + "$Condition_" + std::to_string(index));
              } else {
                key.append("$" + strEntityName + "_triggercount$" + singleSceneEventTemp.strName + "$Condition_" +
                           std::to_string(index));
              }
              if (name2Value.find(key) != name2Value.end()) {
                it.strCount = name2Value[key].c_str();
              }
              index++;
            }

            string strvelocity = "";
            string strAccValue = "";
            bool isOver_V;
            string strAccOverValue = "";
            string strMergeValue = "", strMergeOffset = "", strMergeOverTime = "";
            string strOrientation = "";
            isOver_V = false;
            // private parse action
            for (auto a : as) {
              std::string strActionName = *a._name;
              const auto& pao = a.sub_PrivateAction;
              if (pao) {
                if (m_mapPedestrians.find(strEntityName) != m_mapPedestrians.end()) {
                  ParsePrivateAction((*pao), infos, strEntityName, strvelocity, strAccValue, isOver_V, strAccOverValue,
                                     strMergeValue, strMergeOffset, strMergeOverTime, strOrientation,
                                     singleSceneEventTemp, strActionName, false);
                } else {
                  ParseAction(strEntityName, a, singleSceneEventTemp, infos);
                }
              }
              const auto& UserDefineOpt = a.sub_UserDefinedAction;
              if (UserDefineOpt) {
                ParseAction(strEntityName, a, singleSceneEventTemp, infos);
              }
            }
            AddPedestrianVelocity(strEntityName, infos, strvelocity, ntype, TrigerValue, DistanceType, strOrientation,
                                  strAlongRoute);

            if (singleSceneEventTemp.isUserDefined) {
              if (m_mapVehicles.find(strEntityName) != m_mapVehicles.end()) {
                auto pItr = m_mapVehicles.find(strEntityName);
                string strN = std::to_string(nindexSceneEvent);
                if (pItr->second.m_strEventId.find(strN) == pItr->second.m_strEventId.npos) {
                  if (pItr->second.m_strEventId.size() > 0) {
                    strN = ("," + strN);
                  }
                  pItr->second.m_strEventId.append(strN);
                }
              } else if (m_mapPedestrians.find(strEntityName) != m_mapPedestrians.end()) {
                auto pItr1 = m_mapPedestrians.find(strEntityName);
                string strN = std::to_string(nindexSceneEvent);
                if (!pItr1->second.isDynamicObsctale()) continue;
                if (pItr1->second.m_strEventId.find(strN) == pItr1->second.m_strEventId.npos) {
                  if (pItr1->second.m_strEventId.size() > 0) {
                    strN = "," + strN;
                  }
                  pItr1->second.m_strEventId.append(strN);
                }
                pItr1->second.m_strBehavior = "TrajectoryFollow";
                pItr1->second.m_nBehaviorType = CVehicle::VEHICLE_BEHAVIOR_TRAJECTORYFOLLOW;
              }
              if (m_egoInfos.find(strEntityName) == m_egoInfos.end()) {
                singleSceneEventTemp.m_id = std::to_string(nindexSceneEvent);
                m_mapSceneEvents.insert(std::make_pair(std::to_string(nindexSceneEvent++), singleSceneEventTemp));
              } else {
                CEgoInfo& egoinfo = m_egoInfos.find(strEntityName)->second;
                egoinfo.m_scenceEvents.insert(std::make_pair(std::to_string(nSimIndexEvent++), singleSceneEventTemp));
                singleSceneEventTemp.m_id = std::to_string(nSimIndexEvent);
                if (m_mapSimPlanerEvents.find(strEntityName) != m_mapSimPlanerEvents.end()) {
                  m_mapSimPlanerEvents.find(strEntityName)
                      ->second.insert(std::make_pair(std::to_string(nSimIndexEvent++), singleSceneEventTemp));
                } else {
                  CTraffic::SceneEventMap tmp;
                  tmp[std::to_string(nSimIndexEvent++)] = singleSceneEventTemp;
                  m_mapSimPlanerEvents[strEntityName] = tmp;
                }
              }
            }
          }
        }
      }
      const auto& st = a.sub_StartTrigger;
      int ntype;
      string TrigerValue = "";
      string DistanceType = "";
      string strAlongRoute = "";
      ParseStartTrigger(*st, strActName, ntype, TrigerValue, DistanceType, strAlongRoute);
    }
  }

  // 3) sort data
  std::map<std::string, std::string> actionName2ActionId;
  int eventid = 0;
  for (auto& it : m_mapSceneEvents) {
    int actionid = 0;
    for (auto& itAction : it.second.ActionVer) {
      itAction.strId = std::to_string(actionid);
      std::string actionId = std::to_string(eventid) + "_" + std::to_string(actionid++);
      actionName2ActionId[itAction.strName] = actionId;
    }
    eventid++;
  }
  for (auto& it : m_mapSceneEvents) {
    int actionid = 0;
    for (auto& itCondition : it.second.ConditionVec) {
      if (itCondition.strType == "element_state") {
        if (actionName2ActionId.find(itCondition.strElementRef) != actionName2ActionId.end()) {
          std::string actionId = actionName2ActionId.find(itCondition.strElementRef)->second;
          itCondition.strElementRef = actionId;
        }
      }
    }
  }
  eventid = 0;
  for (auto& tmp : m_mapSimPlanerEvents) {
    int actionid = 0;
    for (auto& it : tmp.second) {
      for (auto& itAction : it.second.ActionVer) {
        itAction.strId = std::to_string(actionid);
        std::string actionId = std::to_string(eventid) + "_" + std::to_string(actionid++);
        actionName2ActionId[itAction.strName] = actionId;
      }
      eventid++;
    }
  }
  for (auto& tmp : m_mapSimPlanerEvents) {
    int actionid = 0;
    for (auto& it : tmp.second) {
      for (auto& itCondition : it.second.ConditionVec) {
        if (itCondition.strType == "element_state") {
          if (actionName2ActionId.find(itCondition.strElementRef) != actionName2ActionId.end()) {
            std::string actionId = actionName2ActionId.find(itCondition.strElementRef)->second;
            itCondition.strElementRef = actionId;
          }
        }
      }
    }
  }

  return 0;
}

int XOSCReader_1_0_v4::ParseOneEgoInfo(osc::ScenarioObject& scenarioObject, std::string catalogDir) {
  // 1) in catalogreference
  CEgoInfo egoinfo;
  if (scenarioObject.sub_CatalogReference && !scenarioObject.sub_CatalogReference->_entryName &&
      !scenarioObject.sub_CatalogReference->_catalogName) {
    std::string catalogName = *scenarioObject.sub_CatalogReference->_catalogName;
    std::string entryName = *scenarioObject.sub_CatalogReference->_entryName;
    std::string name = this->MatchName("Ego", entryName);
    egoinfo.Reset();
    egoinfo.m_strName = name;
    std::ostringstream oss;
    oss << std::setw(3) << std::setfill('0') << m_nEgoGenerator++;
    egoinfo.m_strGroupName = "Ego_" + oss.str();
    if (name != "truck")
      egoinfo.m_strType = "car";
    else
      egoinfo.m_strType = "combination";
    // get Catalog from default.xosc
    CDriveCatalog dirveCataLog;
    if (0 == CCataLog{}.getEgoModelFromName(catalogDir.c_str(), name, dirveCataLog)) {
      egoinfo.m_strVelocityMax = dirveCataLog.m_objPer.getMaxSpeed();
      egoinfo.m_accelerationMax = dirveCataLog.m_objPer.getMaxAcceleration();
      egoinfo.m_decelerationMax = dirveCataLog.m_objPer.getMaxDeceleration();
    }
    m_egoInfos[*scenarioObject._name] = egoinfo;
    return 0;
  } else {
    if (!scenarioObject.sub_Vehicle) {
      return -1;
    }
  }
  // 2) xosc(no catalogreference)
  const auto& objV = scenarioObject.sub_Vehicle;
  if (objV) {
    egoinfo.m_strVelocityMax = *objV->sub_Performance->_maxSpeed;
    egoinfo.m_accelerationMax = *objV->sub_Performance->_maxAcceleration;
    egoinfo.m_decelerationMax = *objV->sub_Performance->_maxDeceleration;
    const auto& pts = objV->sub_Properties->sub_Property;
    for (auto itr : pts) {
      if (*itr._name == "Behavior") {
        std::string strBehavior = *itr._value;
        if (strBehavior == "TrajectoryFollow") {
          egoinfo.m_trajectoryEnabled = "true";
        }
      }
      if (*itr._name == "SensorGroup") {
        egoinfo.m_sensorId = *itr._value;
      }
      if (*itr._name == "TrajectoryTracking" && *itr._value != "") {
        egoinfo.m_controlPath.setControlPathPoint(*itr._value);
      }
    }
  }
  if (scenarioObject.sub_ObjectController) {
    const auto& control = scenarioObject.sub_ObjectController->sub_Controller;
    egoinfo.m_controlType = *control->_name;
    if (control->sub_Properties->sub_Property.size() > 0) {
      auto& sProperties = control->sub_Properties->sub_Property;
    }
  }
  // find name in xosc
  std::string name = *objV->_name;
  bool bIsNameInXosc = false;
  if (!CCataLog{}.findEgoModelFromName(catalogDir.c_str(), name)) {
    name = this->MatchName("Ego", *objV->_name);
    bIsNameInXosc = false;
  } else {
    bIsNameInXosc = true;
  }
  if (name == "suv" || name == "") {
    egoinfo.m_strName = "suv";
  } else if (name == "truck") {
    egoinfo.m_strName = "truck";
  } else {
    egoinfo.m_strName = name;
  }
  const auto& strVehicleCategory = objV->_vehicleCategory;
  egoinfo.m_strType = *(strVehicleCategory);
  if (egoinfo.m_strType.empty()) {
    egoinfo.m_strType = "car";
  }
  // if type is not in xosc
  if (!bIsNameInXosc) {
    CDriveCatalog dirveCataLog;
    if (0 == CCataLog{}.getEgoModelFromName(catalogDir.c_str(), name, dirveCataLog)) {
      egoinfo.m_strVelocityMax = dirveCataLog.m_objPer.getMaxSpeed();
      egoinfo.m_accelerationMax = dirveCataLog.m_objPer.getMaxAcceleration();
      egoinfo.m_decelerationMax = dirveCataLog.m_objPer.getMaxDeceleration();
    }
  }
  string strtemp = "$UnrealLevelIndex";
  FindParameterDeclarationsValude(strtemp);
  egoinfo.m_strUnrealLevel = strtemp;
  egoinfo.m_strName = name;
  std::ostringstream oss;
  oss << std::setw(3) << std::setfill('0') << m_nEgoGenerator++;
  egoinfo.m_strGroupName = "Ego_" + oss.str();
  m_egoInfos[*scenarioObject._name] = egoinfo;
  return 0;
}

void XOSCReader_1_0_v4::ParseFileHeader(osc::FileHeader& fht, sTagSimuTraffic& infos) {
  string str_author = *fht._author;
  string str_date = *fht._date;
  string str_description = *fht._description;
  string str_revMajor = *fht._revMajor;
  string srt_revMinor = *fht._revMinor;
  if (str_author == "TAD Sim" || str_author == "VIRES OpenDRIVE Scenario Editor 2.1.0.495") {
    this->m_version = XOSC_1_0_VERSION_TAD_SIM;
  } else if (str_author == XOSC_TADSIM_2_0 || str_author == XOSC_TADSIM_2_1) {  // tadsim v2.1 == tadsim v2.0
    this->m_version = XOSC_1_0_VERSION_TAD_SIM_2_0;
  } else if (str_author == XOSC_TADSIM_2_2) {
    this->m_version = XOSC_1_0_VERSION_TAD_SIM_2_1;
  } else {
    this->m_version = XOSC_1_0_VERSION_OTHER;
  }
  infos.m_simulation.m_strInfo = str_description;
}

void XOSCReader_1_0_v4::FindParameterDeclarationsValude(string& str) {
  auto iter = m_ParameterDeclarations.find(str);
  if (iter != m_ParameterDeclarations.end()) {
    str = iter->second._vaule;
  } else {
    // assert(false);
    SYSTEM_LOGGER_ERROR("%s is not in parameter declaration", str.c_str());
  }
}

void XOSCReader_1_0_v4::AddVelocity(string& strEntityRef, sTagSimuTraffic& infos, string& strTargetValue, int& nType,
                                    string& strTrigerValue, string& strDistanceType, string& strAlongRoute) {
  if (strTargetValue.size() == 0) return;

  auto pItr = m_mapVehicles.find(strEntityRef);
  if (pItr == m_mapVehicles.end()) return;

  int nDistanceMode = TDM_NONE;
  if (nType == TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SYSTEM_LOGGER_ERROR("distance type error!");
    }
  } else if (nType == TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SYSTEM_LOGGER_ERROR("distance type error!");
    }
  } else {
    SYSTEM_LOGGER_ERROR("event type error!");
  }

  VelocityTimeNode TempVelocityTimeNode;
  TempVelocityTimeNode.dTriggerValue = atof(strTrigerValue.c_str());
  TempVelocityTimeNode.dVelocity = atof(strTargetValue.c_str());
  TempVelocityTimeNode.nCount = 1;
  TempVelocityTimeNode.nDistanceMode = nDistanceMode;
  TempVelocityTimeNode.nTriggerType = nType;

  auto vItr = m_mapVelocitys.find(pItr->second.m_strID);
  if (vItr == m_mapVelocitys.end()) {
    CVelocity TempCVelocity;
    TempCVelocity.m_ID = std::atoi(pItr->second.m_strID.c_str());
    TempCVelocity.m_nodes.push_back(TempVelocityTimeNode);
    TempCVelocity.ConvertToStr();
    TempCVelocity.ConvertToEventStr();
    TempCVelocity.ConvertToTimeStr();

    m_mapVelocitys.insert(std::make_pair(TempCVelocity.m_strID, TempCVelocity));
  } else {
    CVelocity& pv = vItr->second;
    pv.m_nodes.push_back(TempVelocityTimeNode);
    pv.ConvertToStr();
    pv.ConvertToEventStr();
    pv.ConvertToTimeStr();
  }
}

void XOSCReader_1_0_v4::AddPedestrianVelocity(string& strEntityRef, sTagSimuTraffic& infos, string& strTargetValue,
                                              int& nType, string& strTrigerValue, string& strDistanceType,
                                              string& strOrientation, string& strAlongRoute) {
  // if (strTargetValue.size() == 0) return;
  auto pItr = m_mapPedestrians.find(strEntityRef);
  if (pItr == m_mapPedestrians.end()) return;
  int nDistanceMode = TDM_NONE;
  if (nType == TCT_TIME_ABSOLUTE) {
  } else if (nType == TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SystemLogger::Error("distance type error!");
    }
  } else if (nType == TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SystemLogger::Error("distance type error!");
    }
  } else {
    SystemLogger::Error("event type error!");
    return;
  }
  if (pItr->second.isDynamicObsctale()) {
    return;
  } else if (std::atof(strTrigerValue.c_str()) < 1e-3 && std::atof(strTargetValue.c_str()) < 1e-3) {
    return;
  }
  PedestrianVelocityNode TempPedestrianVelocityNode;
  TempPedestrianVelocityNode.dTriggerValue = atof(strTrigerValue.c_str());
  TempPedestrianVelocityNode.dDirection = atof(strOrientation.c_str());
  TempPedestrianVelocityNode.dVelocity = atof(strTargetValue.c_str());
  TempPedestrianVelocityNode.nCount = 1;
  TempPedestrianVelocityNode.nDistanceMode = nDistanceMode;
  TempPedestrianVelocityNode.nTriggerType = nType;

  auto vItr = m_mapPedestrianVelocitys.find(pItr->second.m_strID);
  if (vItr == m_mapPedestrianVelocitys.end()) {
    CPedestrianVelocity TempCPedestrianVelocity;
    TempCPedestrianVelocity.m_ID = std::atoi(pItr->second.m_strID.c_str());
    TempCPedestrianVelocity.m_nodes.push_back(TempPedestrianVelocityNode);
    TempCPedestrianVelocity.ConvertToStr();
    TempCPedestrianVelocity.ConvertToEventStr();
    TempCPedestrianVelocity.ConvertToTimeStr();
    m_mapPedestrianVelocitys.insert(std::make_pair(TempCPedestrianVelocity.m_strID, TempCPedestrianVelocity));

  } else {
    CPedestrianVelocity& pv = vItr->second;
    pv.m_nodes.push_back(TempPedestrianVelocityNode);
    pv.ConvertToStr();
    pv.ConvertToEventStr();
    pv.ConvertToTimeStr();
  }

  // pItr->second.m_strBehavior = "UserDefine";
}

void XOSCReader_1_0_v4::AddAccOrMerge(string& strEntityRef, sTagSimuTraffic& infos, string& strAccValue, bool& isOver_V,
                                      string& strAccOverValue, string& strMergeValue, string& strMergeOffset,
                                      string& strMergeOverTime, int& nType, string& strTrigerValue,
                                      string& strDistanceType, string& strAlongRoute) {
  auto vItr = m_mapVehicles.find(strEntityRef);
  if (vItr == m_mapVehicles.end()) return;

  int nDistanceMode = TDM_NONE;
  if (nType == TCT_POSITION_RELATIVE) {
    if (boost::iequals("longitudinal", strDistanceType)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("cartesianDistance", strDistanceType)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SYSTEM_LOGGER_ERROR("distance type error!");
    }
  } else if (nType == TCT_TIME_RELATIVE) {
    if (boost::iequals("true", strAlongRoute)) {
      nDistanceMode = TDM_LANE_PROJECTION;
    } else if (boost::iequals("false", strAlongRoute)) {
      nDistanceMode = TDM_EUCLIDEAN_DISTANCE;
    } else {
      SYSTEM_LOGGER_ERROR("distance type error!");
    }
  } else {
    SYSTEM_LOGGER_ERROR("event type error!");
  }

  {
    TimeNode TempTimeNode;
    TempTimeNode.dTriggerValue = atof(strTrigerValue.c_str());

    if (strAccValue.size() > 0) {
      TempTimeNode.dAcc = atof(strAccValue.c_str());
    } else {
      TempTimeNode.dAcc = 0;
    }
    TempTimeNode.nTriggerType = nType;
    TempTimeNode.nDistanceMode = nDistanceMode;
    TempTimeNode.nCount = 1;
    if (isOver_V) {
      TempTimeNode.nStopType = AECT_VELOCITY;
    } else {
      TempTimeNode.nStopType = AECT_TIME;
    }
    if (strAccOverValue.size() > 0) {
      TempTimeNode.dStopValue = stod(strAccOverValue);
    } else {
      TempTimeNode.dStopValue = 3;
    }

    bool isNew = true;

    std::string strID = vItr->second.m_strAccID;
    if (strID.size() > 0) {
      auto aItr = m_mapAccs.find(strID);
      if (aItr == m_mapAccs.end()) {
        SYSTEM_LOGGER_ERROR("acc %s is not exist", strID.c_str());
        return;
      }
      aItr->second.m_nodes.push_back(TempTimeNode);
      isNew = false;
      aItr->second.ConvertToStr();
      aItr->second.ConvertToTimeStr();
      aItr->second.ConvertToEventStr();
    }

    if (isNew) {
      CAcceleration TempAcceleration;
      TempAcceleration.m_ID = m_nAccIDGeneator++;
      TempAcceleration.m_nodes.push_back(TempTimeNode);

      TempAcceleration.ConvertToStr();
      TempAcceleration.ConvertToTimeStr();
      TempAcceleration.ConvertToEventStr();

      m_mapAccs.insert(std::make_pair(TempAcceleration.m_strID, TempAcceleration));
      vItr->second.m_strAccID = TempAcceleration.m_strID;
    }
  }

  {
    tagMergeUnit TemptagMergeUnit;
    TemptagMergeUnit.dTrigValue = stod(strTrigerValue);
    if (strMergeValue.size() > 0) {
      if (strMergeOffset.size() > 0) {
        double dOffset = stod(strMergeOffset);
        if (dOffset < 0) {
          TemptagMergeUnit.nDir = MM_LANE_RIGHT;
        } else {
          TemptagMergeUnit.nDir = MM_LANE_LEFT;
        }
        TemptagMergeUnit.dOffset = dOffset > 0 ? dOffset : -1 * dOffset;
      } else {
        TemptagMergeUnit.nDir = std::atoi(strMergeValue.c_str());  // mergemodle
        TemptagMergeUnit.dOffset = 0;
      }
      TemptagMergeUnit.dDuration = stod(strMergeOverTime);
      TemptagMergeUnit.nDistanceMode = nDistanceMode;
    } else {
      TemptagMergeUnit.nDir = 0;
      TemptagMergeUnit.dDuration = 4.5;
      TemptagMergeUnit.nDistanceMode = nDistanceMode;
      TemptagMergeUnit.dOffset = 0;
    }

    TemptagMergeUnit.nTrigType = nType;
    TemptagMergeUnit.nCount = 1;

    bool isNew = true;
    std::string strID = vItr->second.m_strMergeID;
    if (strID.size() > 0) {
      auto mItr = m_mapMerges.find(strID);
      if (mItr == m_mapMerges.end()) {
        SYSTEM_LOGGER_ERROR("merge %s is not exist", strID.c_str());
        return;
      }
      mItr->second.m_merges.push_back(TemptagMergeUnit);
      isNew = false;
      mItr->second.ConvertToStr();
      mItr->second.ConvertToTimeStr();
      mItr->second.ConvertToEventStr();
    }

    if (isNew) {
      CMerge TempMerge;
      TempMerge.m_ID = m_nMergeIDGenerator++;
      TempMerge.m_merges.push_back(TemptagMergeUnit);
      TempMerge.ConvertToTimeStr();
      TempMerge.ConvertToEventStr();

      m_mapMerges.insert(std::make_pair(TempMerge.m_strID, TempMerge));
      vItr->second.m_strMergeID = TempMerge.m_strID;
    }
  }
}
