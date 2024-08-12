/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "simulation.h"

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <chrono>
#include <cstring>
#include <iostream>
#include <utility>
#include "../../engine/config.h"
#include "common/log/xml_logger.h"
#include "common/xml_parser/entity/catalog.h"
#include "common/xml_parser/entity/traffic.h"
#include "simulation_param.h"
using namespace std::chrono;

CSimulation::CSimulation() {
  m_strInfo = "";
  m_strSensor = "";
  m_strEnvironment = "";
  m_strTraffic = "";
  m_strControlPath = "";
  m_strGrading = "";
  m_strGeoFence = "";
}

CSimulation::CSimulation(CSimulationParam& sp) {
  m_strInfo = sp.m_strInfo;
  m_mapFile = sp.m_mapFile;
  m_strTraffic = sp.m_strTraffic;
  m_strSensor = sp.m_strSensor;
  m_strEnvironment = sp.m_strEnvironment;
  m_strControlPath = sp.m_strControlPath;
  m_strGrading = sp.m_strGrading;
  m_strGeoFence = sp.m_strGeoFence;
  m_planner = sp.m_planner;
  m_l3States = sp.m_l3States;
  m_generateInfo = sp.m_generateInfo;
}

int CSimulation::SetScenePath(const char* strScenePath) {
  m_strScenePath = strScenePath;
  return 0;
}

int CSimulation::SetMapFile(const CMapFile& mapfile) {
  m_mapFile = mapfile;
  return 0;
}

int CSimulation::SetTrafficFile(const char* strTrafficFileName) {
  if (!strTrafficFileName) {
    return -1;
  }

  m_strTraffic = strTrafficFileName;
  return 0;
}

int CSimulation::SetSensorFile(const char* strSensorFileName) {
  if (!strSensorFileName) {
    return -1;
  }

  m_strSensor = strSensorFileName;
  return 0;
}

int CSimulation::SetEnvironmentFile(const char* strEnvironmentFileName) {
  if (!strEnvironmentFileName) {
    return -1;
  }

  m_strEnvironment = strEnvironmentFileName;
  return 0;
}

int CSimulation::SetControlPathFile(const char* strControlPathFileName) {
  if (!strControlPathFileName) {
    return -1;
  }

  m_strControlPath = strControlPathFileName;
  return 0;
}

int CSimulation::SetGradingFile(const char* strGradingFileName) {
  if (!strGradingFileName) {
    return -1;
  }

  m_strGrading = strGradingFileName;
  return 0;
}

int CSimulation::SetPlanner(const CPlanner& planner) {
  m_planner = planner;

  return 0;
}

int CSimulation::SetRosbag(const CRosbag& rosbag) {
  m_rosbag = rosbag;

  return 0;
}

int CSimulation::ChangeTrafficName(const char* strFileName, const char* strTrafficFileName) {
  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFileName);

  if (!bRet) {
    XML_LOGGER_ERROR("sim format error!");
    return -1;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  // 获得指定节点。
  TiXmlElement* First = xmlRoot->FirstChildElement("traffic");
  if (First != nullptr) {
    First->FirstChild()->SetValue(strTrafficFileName);
    doc.SaveFile(strFileName);

    return 0;
  }

  return -1;
}

int CSimulation::Parse(const char* strFileName, bool bLoadHadmap /* = true*/) {
  if (!strFileName) {
    XML_LOGGER_ERROR("sim file name null");
    return -1;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFileName);

  if (!bRet) {
    XML_LOGGER_ERROR("sim format error!");
    return -2;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -3;

  std::string strName = xmlRoot->Value();

  if (!boost::algorithm::iequals(strName, "simulation")) return -4;

  // 解析地图参数数据
  TiXmlElement* elemMapFile = xmlRoot->FirstChildElement("mapfile");
  int nRet = ParseMapfile(elemMapFile, m_mapFile);
  if (nRet != 0) {
    return -5;
  }

  // 载入实际的地图数据
  if (bLoadHadmap) {
    if (m_mapFile.m_strMapfile.length() > 0) {
      nRet = m_mapFile.ParseMapV2(m_mapFile.m_strMapfile.c_str());
      if (nRet != 0) {
        return -6;
      }
    }
  }

  TiXmlElement* elemTraffic = xmlRoot->FirstChildElement("traffic");
  nRet = ParseTraffic(elemTraffic, m_strTraffic);
  if (nRet != 0) {
    return -7;
  }

  // 解析传感器文件路径
  TiXmlElement* elemSensor = xmlRoot->FirstChildElement("sensor");
  if (elemSensor) {
    nRet = ParseSensor(elemSensor, m_strSensor);
    if (nRet != 0) {
      return -8;
    }
  }

  // 解析环境文件路径
  TiXmlElement* elemEnvironment = xmlRoot->FirstChildElement("environment");
  if (elemEnvironment) {
    nRet = ParseEnvironment(elemEnvironment, m_strEnvironment);
    if (nRet != 0) {
      return -9;
    }
  }

  // 解析grading文件
  TiXmlElement* elemGrading = xmlRoot->FirstChildElement("grading");
  if (elemGrading) {
    nRet = ParseGrading(elemGrading, m_strGrading);
    if (nRet != 0) {
      return -10;
    }
  }

  /* controlPath 路径不用解析，给control模块使用，场景编辑器不需要再解析回来。
          controlPath的关键信息存放在planner中

  */
  TiXmlElement* elemPlanner = xmlRoot->FirstChildElement("planner");
  nRet = ParsePlanner(elemPlanner, m_planner);
  if (nRet != 0) {
    return -11;
  }
  m_planner.m_strUnrealLevel = m_mapFile.m_strUnrealLevelIndex;

  // rosbag
  TiXmlElement* elemRosbag = xmlRoot->FirstChildElement("rosbag");
  if (elemRosbag) {
    nRet = ParseRosbag(elemRosbag, m_rosbag);
    if (nRet != 0) {
      return -12;
    }
  }

  // l3 state machine
  TiXmlElement* elemL3StateMachine = xmlRoot->FirstChildElement("l3_states");
  if (elemL3StateMachine) {
    nRet = ParseL3States(elemL3StateMachine, m_l3States);
    if (nRet != 0) {
      return -13;
    }
  }

  // geofence
  TiXmlElement* elemGeoFence = xmlRoot->FirstChildElement("geofence");
  if (elemGeoFence) {
    nRet = ParseGeoFence(elemGeoFence, m_strGeoFence);
    if (nRet != 0) {
      return -14;
    }
  }

  // generate info
  TiXmlElement* elemGenerateInfo = xmlRoot->FirstChildElement("generateinfo");
  if (elemGenerateInfo) {
    nRet = ParseGenerateInfo(elemGenerateInfo, m_generateInfo);
    if (nRet != 0) {
      return -15;
    }
  }

  // info
  TiXmlElement* elemInfo = xmlRoot->FirstChildElement("info");
  if (elemInfo) {
    nRet = ParseInfo(elemInfo, m_strInfo);
    if (nRet != 0) {
      return -16;
    }
  }
  TiXmlElement* elemSimrec = xmlRoot->FirstChildElement("simrec");
  if (elemSimrec) {
    nRet = ParseSimrecDataSource(elemSimrec, m_strDataSource);
    XML_LOGGER_INFO("find simrec...,source=%s,nret=%d", m_strDataSource.c_str(), nRet);
    if (nRet != 0) {
      return -17;
    }
  } else {
    XML_LOGGER_INFO("no simrec");
  }
  XML_LOGGER_INFO("Parse sim file : %s succeed!", strFileName);
  return 0;
}

int CSimulation::ParseMapfile(TiXmlElement* elemMapfile, CMapFile& mapfile) {
  if (!elemMapfile) return -1;

  const char* p = elemMapfile->Attribute("lon");
  if (p) {
    mapfile.m_strLon = p;
    mapfile.m_fLon = std::atof(p);
  }

  p = elemMapfile->Attribute("lat");
  if (p) {
    mapfile.m_strLat = p;
    mapfile.m_fLat = std::atof(p);
  }

  p = elemMapfile->Attribute("alt");
  if (p) {
    mapfile.m_strAlt = p;
    mapfile.m_fAlt = std::atof(p);
  }

  p = elemMapfile->Attribute("unrealLevelIndex");
  if (p) mapfile.m_strUnrealLevelIndex = p;

  p = elemMapfile->GetText();
  if (p) {
    mapfile.m_strMapfile = p;
  }

  return 0;
}

int CSimulation::ParseTraffic(TiXmlElement* elemTraffic, std::string& strTraffic) {
  if (!elemTraffic) return -1;

  const char* p = elemTraffic->GetText();
  if (p) strTraffic = p;

  return 0;
}

int CSimulation::ParseSensor(TiXmlElement* elemSensor, std::string& strSensor) {
  if (!elemSensor) return -1;

  const char* p = elemSensor->GetText();
  if (p) strSensor = p;

  return 0;
}

int CSimulation::ParseEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment) {
  if (!elemEnvironment) return -1;

  const char* p = elemEnvironment->GetText();
  if (p) strEnvironment = p;

  return 0;
}

int CSimulation::ParseControlPath(TiXmlElement* elemControlPath, std::string& strControlPath) {
  if (!elemControlPath) return -1;

  const char* p = elemControlPath->GetText();
  if (p) strControlPath = p;

  return 0;
}

int CSimulation::ParseSimrecDataSource(TiXmlElement* elemInfo, std::string& strSource) {
  if (!elemInfo) return -1;
  TiXmlElement* elemSource = elemInfo->FirstChildElement("source");
  if (elemSource) {
    const char* p = elemSource->GetText();
    if (p) {
      strSource = p;
    }
  }
  return 0;
}

int CSimulation::ParseGrading(TiXmlElement* elemGrading, std::string& strGrading) {
  if (!elemGrading) return -1;

  const char* p = elemGrading->GetText();
  if (p) strGrading = p;

  return 0;
}

int CSimulation::ParsePlanner(TiXmlElement* elemPlanner, CPlanner& planner) {
  if (!elemPlanner) return -1;
  if (elemPlanner->Attribute("version") && std::string(elemPlanner->Attribute("version")) == "2.0") {
    return ParsePlannerV2(elemPlanner, planner);
  }
  TiXmlElement* elemEgo = elemPlanner->FirstChildElement("ego");
  CEgoInfo _ego;
  if (elemEgo) {
    const char* p = elemEgo->Attribute("type");
    std::string pType = p;
    if (p) {
      if (pType == "suv" || pType == "") {
        _ego.m_strType = "car";
      } else if (pType == "truck") {
        _ego.m_strType = "combination";
      } else {
        _ego.m_strType = pType;
      }
    } else {
      _ego.m_strType = "car";
    }
    const char* egoName = elemEgo->Attribute("name");

    std::string strCatalogPath = CEngineConfig::Instance().getDataCatalogsDir();
    std::string strName;
    std::cout << "_ego.m_strType" << _ego.m_strType;
    if (_ego.m_strType == "car") {
      if (!egoName) {
        _ego.m_strName = "suv";
      } else if (CCataLog{}.findEgoModelFromName(strCatalogPath.c_str(), std::string(egoName), VEHICLE_CAR)) {
        _ego.m_strName = egoName;
      } else {
        _ego.m_strName = "suv";
      }
    } else if (_ego.m_strType == "AIV_FullLoad_001" || _ego.m_strType == "AIV_Empty_001" ||
               _ego.m_strType == "MPV_001" || _ego.m_strType == "MiningDumpTruck_001" ||
               _ego.m_strType == "AIV_Empty_002" || _ego.m_strType == "MIFA_01" || _ego.m_strType == "MIFA_01" ||
               _ego.m_strType == "AIV_V5_FullLoad_001" || _ego.m_strType == "AIV_V5_Empty_001" ||
               _ego.m_strType == "AIV_FullLoad_002") {
      _ego.m_strName = _ego.m_strType;
      _ego.m_strType = "car";
    } else {
      if (!egoName) {
        _ego.m_strName = "truck";
      } else if (CCataLog{}.findEgoModelFromName(strCatalogPath.c_str(), std::string(egoName), VEHICLE_TRUCK)) {
        _ego.m_strName = egoName;
      } else {
        _ego.m_strName = "truck";
      }
    }
  } else {
    _ego.m_strType = "car";
    _ego.m_strName = "suv";
  }
  TiXmlElement* elemRoute = elemPlanner->FirstChildElement("route");
  int nRet = CTraffic::ParseOneRoute(elemRoute, _ego.m_route);
  if (nRet != 0) {
    XML_LOGGER_ERROR("simfile planner route can not be null!");
    return -1;
  }

  TiXmlElement* elemStartV = elemPlanner->FirstChildElement("start_v");
  if (!elemStartV) {
    XML_LOGGER_ERROR("simfile planner start_v can not be null!");
    return -1;
  }

  const char* p = elemStartV->GetText();
  if (p) _ego.m_strStartVelocity = p;

  TiXmlElement* elemTheta = elemPlanner->FirstChildElement("theta");
  if (!elemTheta) {
    XML_LOGGER_ERROR("simfile planner theta can not be null!");
    return -1;
  }

  p = elemTheta->GetText();
  if (p) _ego.m_strTheta = p;

  TiXmlElement* elemVelocityMax = elemPlanner->FirstChildElement("Velocity_Max");
  if (!elemVelocityMax) {
    XML_LOGGER_ERROR("simfile planner Velocity_Max can not be null!");
    return -1;
  }

  p = elemVelocityMax->GetText();
  if (p) _ego.m_strVelocityMax = p;

  TiXmlElement* elemcontrol_longitudinal = elemPlanner->FirstChildElement("control_longitudinal");
  if (elemcontrol_longitudinal) {
    p = elemcontrol_longitudinal->GetText();
    if (p)
      _ego.m_controlLongitudinal = p;
    else
      _ego.m_controlLongitudinal = "true";
  }

  TiXmlElement* elemcontrol_lateral = elemPlanner->FirstChildElement("control_lateral");
  if (elemcontrol_lateral) {
    p = elemcontrol_lateral->GetText();
    if (p)
      _ego.m_controlLateral = p;
    else
      _ego.m_controlLateral = "true";
  }
  TiXmlElement* trajectory_enabled = elemPlanner->FirstChildElement("trajectory_enabled");
  if (trajectory_enabled) {
    p = trajectory_enabled->GetText();
    if (p)
      _ego.m_trajectoryEnabled = p;
    else
      _ego.m_trajectoryEnabled = "false";
  }

  TiXmlElement* elemcontrol_acceleration_max = elemPlanner->FirstChildElement("acceleration_max");
  if (elemcontrol_acceleration_max) {
    p = elemcontrol_acceleration_max->GetText();
    if (p)
      _ego.m_accelerationMax = p;
    else
      _ego.m_accelerationMax = "12.0";
  }

  TiXmlElement* elemcontrol_deceleration_max = elemPlanner->FirstChildElement("deceleration_max");
  if (elemcontrol_deceleration_max) {
    p = elemcontrol_deceleration_max->GetText();
    if (p)
      _ego.m_decelerationMax = p;
    else
      _ego.m_decelerationMax = "10.0";
  }

  TiXmlElement* elemSceneEvents = elemPlanner->FirstChildElement("scene_event");
  if (elemSceneEvents) {
    int nRet = ParseSceneEvents(elemSceneEvents, _ego.m_scenceEvents);
  }
  TiXmlElement* elemAltitude = elemPlanner->FirstChildElement("altitude");
  if (elemAltitude) {
    const char* p = elemAltitude->Attribute("start");
    if (p) {
      _ego.m_strStartAlt = p;
      _ego.m_dStartAlt = std::atof(p);
    } else {
      _ego.m_strStartAlt = "0";
      _ego.m_dStartAlt = 0;
    }

    p = elemAltitude->Attribute("end");
    if (p) {
      _ego.m_strEndAlt = p;
      _ego.m_dEndAlt = std::atof(p);
    } else {
      _ego.m_strEndAlt = "0";
      _ego.m_dEndAlt = 0;
    }
  } else {
    _ego.m_strStartAlt = "0";
    _ego.m_dStartAlt = 0;
    _ego.m_strEndAlt = "0";
    _ego.m_dEndAlt = 0;
  }

  TiXmlElement* elemInputPath = elemPlanner->FirstChildElement("InputPath");
  if (elemInputPath) {
    p = elemInputPath->Attribute("points");
    if (p) {
      _ego.m_inputPath.m_strPoints = p;
    } else {
      // assert(false);
      XML_LOGGER_ERROR("simfile planner inputpath can not be null!");
      return -1;
    }
  } else {
    CRoute& route = _ego.m_route;
    route.FromStr();
    _ego.m_inputPath.m_strPoints = route.m_strStartLon + "," + route.m_strStartLat + "," + _ego.m_strStartAlt + ";" +
                                   route.m_strEndLon + "," + route.m_strEndLat + "," + _ego.m_strEndAlt;
  }
  if (_ego.m_trajectoryEnabled == "true") {
    TiXmlElement* elemControlPath = elemPlanner->FirstChildElement("ControlPath");
    if (elemControlPath) {
      p = elemControlPath->Attribute("points");
      if (p) {
        _ego.m_controlPath.m_strPoints = p;
        _ego.m_controlPath.ConvertToValue();
      } else {
        assert(false);
        XML_LOGGER_ERROR("simfile planner control path can not be null!");
        return -1;
      }

      p = elemControlPath->Attribute("sampleInterval");
      if (p) {
        _ego.m_controlPath.m_strSampleInterval = p;
      } else {
        _ego.m_controlPath.m_strSampleInterval = "10";
        XML_LOGGER_WARN("simfile planner control path sample interval is null!");
      }
    } else {
      CRoute& route = _ego.m_route;
      route.FromStr();
      _ego.m_controlPath.m_strPoints = route.m_strStartLon + "," + route.m_strStartLat + "," + _ego.m_strStartAlt +
                                       "," + _ego.m_strStartVelocity + ", drive;" + route.m_strEndLon + "," +
                                       route.m_strEndLat + "," + _ego.m_strEndAlt + ",0,drive";
      _ego.m_controlPath.m_strSampleInterval = "10";
    }
  }
  _ego.m_strGroupName = "Ego_001";
  planner.m_egos.insert(std::make_pair("Ego_001", _ego));
  return 0;
}

int CSimulation::ParsePlannerV2(TiXmlElement* elemPlanner, CPlanner& planner) {
  if (!elemPlanner->FirstChildElement("egolist")) {
    return -1;
  }
  TiXmlElement* _egoinfo = elemPlanner->FirstChildElement("egolist")->FirstChildElement("egoinfo");
  while (_egoinfo) {
    TiXmlElement* elemEgo = _egoinfo->FirstChildElement("ego");
    CEgoInfo _tmpEgo;
    if (elemEgo) {
      const char* p = elemEgo->Attribute("type");
      std::string pType = p;
      if (p) {
        if (pType == "suv" || pType == "") {
          _tmpEgo.m_strType = "car";
        } else if (pType == "truck") {
          _tmpEgo.m_strType = "combination";
        } else {
          _tmpEgo.m_strType = pType;
        }
      } else {
        _tmpEgo.m_strType = "car";
      }
      const char* egoName = elemEgo->Attribute("name");
      const char* _group = elemEgo->Attribute("group");
      if (p) {
        _tmpEgo.m_strGroupName = _group;
      }
      std::string strCatalogPath = CEngineConfig::Instance().getDataCatalogsDir();
      std::string strName;

      if (_tmpEgo.m_strType == "car") {
        if (!egoName) {
          _tmpEgo.m_strName = "suv";
        } else if (CCataLog{}.findEgoModelFromName(strCatalogPath.c_str(), std::string(egoName), VEHICLE_CAR)) {
          _tmpEgo.m_strName = egoName;
        } else {
          _tmpEgo.m_strName = "suv";
        }
      } else if (_tmpEgo.m_strType == "AIV_FullLoad_001" || _tmpEgo.m_strType == "AIV_Empty_001" ||
                 _tmpEgo.m_strType == "MPV_001" || _tmpEgo.m_strType == "MiningDumpTruck_001" ||
                 _tmpEgo.m_strType == "AIV_Empty_002" || _tmpEgo.m_strType == "MIFA_01" ||
                 _tmpEgo.m_strType == "MIFA_01" || _tmpEgo.m_strType == "AIV_V5_FullLoad_001" ||
                 _tmpEgo.m_strType == "AIV_V5_Empty_001" || _tmpEgo.m_strType == "AIV_FullLoad_002") {
        _tmpEgo.m_strName = _tmpEgo.m_strType;
        _tmpEgo.m_strType = "car";
      } else {
        if (!egoName) {
          _tmpEgo.m_strName = "truck";
        } else if (CCataLog{}.findEgoModelFromName(strCatalogPath.c_str(), std::string(egoName), VEHICLE_TRUCK)) {
          _tmpEgo.m_strName = egoName;
        } else {
          _tmpEgo.m_strName = "truck";
        }
      }
    } else {
      _tmpEgo.m_strType = "car";
      _tmpEgo.m_strName = "suv";
    }
    TiXmlElement* elemRoute = _egoinfo->FirstChildElement("route");
    int nRet = CTraffic::ParseOneRoute(elemRoute, _tmpEgo.m_route);
    if (nRet != 0) {
      XML_LOGGER_ERROR("simfile planner route can not be null!");
      return -1;
    }

    TiXmlElement* elemStartV = _egoinfo->FirstChildElement("start_v");
    if (!elemStartV) {
      XML_LOGGER_ERROR("simfile planner start_v can not be null!");
      return -1;
    }

    const char* p = elemStartV->GetText();
    if (p) _tmpEgo.m_strStartVelocity = p;

    TiXmlElement* elemTheta = _egoinfo->FirstChildElement("theta");
    if (!elemTheta) {
      XML_LOGGER_ERROR("simfile planner theta can not be null!");
      return -1;
    }

    p = elemTheta->GetText();
    if (p) _tmpEgo.m_strTheta = p;

    TiXmlElement* elemVelocityMax = _egoinfo->FirstChildElement("Velocity_Max");
    if (!elemVelocityMax) {
      XML_LOGGER_ERROR("simfile planner Velocity_Max can not be null!");
      return -1;
    }

    p = elemVelocityMax->GetText();
    if (p) _tmpEgo.m_strVelocityMax = p;

    TiXmlElement* elemcontrol_longitudinal = _egoinfo->FirstChildElement("control_longitudinal");
    if (elemcontrol_longitudinal) {
      p = elemcontrol_longitudinal->GetText();
      if (p)
        _tmpEgo.m_controlLongitudinal = p;
      else
        _tmpEgo.m_controlLongitudinal = "true";
    }

    TiXmlElement* elemcontrol_lateral = _egoinfo->FirstChildElement("control_lateral");
    if (elemcontrol_lateral) {
      p = elemcontrol_lateral->GetText();
      if (p)
        _tmpEgo.m_controlLateral = p;
      else
        _tmpEgo.m_controlLateral = "true";
    }
    TiXmlElement* trajectory_enabled = _egoinfo->FirstChildElement("trajectory_enabled");
    if (trajectory_enabled) {
      p = trajectory_enabled->GetText();
      if (p)
        _tmpEgo.m_trajectoryEnabled = p;
      else
        _tmpEgo.m_trajectoryEnabled = "false";
    }

    TiXmlElement* elemcontrol_acceleration_max = _egoinfo->FirstChildElement("acceleration_max");
    if (elemcontrol_acceleration_max) {
      p = elemcontrol_acceleration_max->GetText();
      if (p)
        _tmpEgo.m_accelerationMax = p;
      else
        _tmpEgo.m_accelerationMax = "12.0";
    }

    TiXmlElement* elemcontrol_deceleration_max = _egoinfo->FirstChildElement("deceleration_max");
    if (elemcontrol_deceleration_max) {
      p = elemcontrol_deceleration_max->GetText();
      if (p)
        _tmpEgo.m_decelerationMax = p;
      else
        _tmpEgo.m_decelerationMax = "10.0";
    }

    TiXmlElement* elemSceneEvents = _egoinfo->FirstChildElement("scene_event");
    if (elemSceneEvents) {
      int nRet = ParseSceneEvents(elemSceneEvents, _tmpEgo.m_scenceEvents);
    }

    TiXmlElement* elemAltitude = _egoinfo->FirstChildElement("altitude");
    if (elemAltitude) {
      const char* p = elemAltitude->Attribute("start");
      if (p) {
        _tmpEgo.m_strStartAlt = p;
        _tmpEgo.m_dStartAlt = std::atof(p);
      } else {
        _tmpEgo.m_strStartAlt = "0";
        _tmpEgo.m_dStartAlt = 0;
      }

      p = elemAltitude->Attribute("end");
      if (p) {
        _tmpEgo.m_strEndAlt = p;
        _tmpEgo.m_dEndAlt = std::atof(p);
      } else {
        _tmpEgo.m_strEndAlt = "0";
        _tmpEgo.m_dEndAlt = 0;
      }
    } else {
      _tmpEgo.m_strStartAlt = "0";
      _tmpEgo.m_dStartAlt = 0;
      _tmpEgo.m_strEndAlt = "0";
      _tmpEgo.m_dEndAlt = 0;
    }

    TiXmlElement* elemInputPath = _egoinfo->FirstChildElement("InputPath");
    if (elemInputPath) {
      p = elemInputPath->Attribute("points");
      if (p) {
        _tmpEgo.m_inputPath.m_strPoints = p;
      } else {
        // assert(false);
        XML_LOGGER_ERROR("simfile planner inputpath can not be null!");
        return -1;
      }
    } else {
      CRoute& route = _tmpEgo.m_route;
      route.FromStr();
      _tmpEgo.m_inputPath.m_strPoints = route.m_strStartLon + "," + route.m_strStartLat + "," + _tmpEgo.m_strStartAlt +
                                        ";" + route.m_strEndLon + "," + route.m_strEndLat + "," + _tmpEgo.m_strEndAlt;
    }
    if (_tmpEgo.m_trajectoryEnabled == "true") {
      TiXmlElement* elemControlPath = _egoinfo->FirstChildElement("ControlPath");
      if (elemControlPath) {
        p = elemControlPath->Attribute("points");
        if (p) {
          _tmpEgo.m_controlPath.m_strPoints = p;
          _tmpEgo.m_controlPath.ConvertToValue();
        } else {
          assert(false);
          XML_LOGGER_ERROR("simfile planner control path can not be null!");
          return -1;
        }

        p = elemControlPath->Attribute("sampleInterval");
        if (p) {
          _tmpEgo.m_controlPath.m_strSampleInterval = p;
        } else {
          _tmpEgo.m_controlPath.m_strSampleInterval = "10";
          XML_LOGGER_WARN("simfile planner control path sample interval is null!");
        }
      } else {
        CRoute& route = _tmpEgo.m_route;
        route.FromStr();
        _tmpEgo.m_controlPath.m_strPoints = route.m_strStartLon + "," + route.m_strStartLat + "," +
                                            _tmpEgo.m_strStartAlt + "," + _tmpEgo.m_strStartVelocity + ", drive;" +
                                            route.m_strEndLon + "," + route.m_strEndLat + "," + _tmpEgo.m_strEndAlt +
                                            ",0,drive";
        _tmpEgo.m_controlPath.m_strSampleInterval = "10";
      }
    }
    _egoinfo = _egoinfo->NextSiblingElement("egoinfo");
    planner.m_egos.insert(std::make_pair(_tmpEgo.m_strGroupName, _tmpEgo));
  }
  return 0;
}

int CSimulation::ParseRosbag(TiXmlElement* elemRosbag, CRosbag& rosbag) {
  if (!elemRosbag) return -1;

  const char* p = elemRosbag->GetText();
  if (p) rosbag.SetPath(p);

  return 0;
}

int CSimulation::ParseL3States(TiXmlElement* elemL3States, CL3States& l3States) {
  if (!elemL3States) return -1;

  TiXmlElement* elemState = elemL3States->FirstChildElement("l3_state");
  while (elemState) {
    tagStateItem si;

    TiXmlAttribute* attr = elemState->FirstAttribute();
    std::string strName = attr->Name();
    std::vector<std::string> names;
    boost::algorithm::split(names, strName, boost::algorithm::is_any_of("."));

    si.strStateType = names[0];
    si.strStateName = names[1];
    si.nValue = attr->IntValue();
    const char* p = elemState->GetText();
    if (p) {
      si.dTriggerTime = std::atof(p);
      si.dTriggerTime /= 1000;
    }
    l3States.AddState(si);
    elemState = elemState->NextSiblingElement("l3_state");
  }

  return 0;
}

int CSimulation::ParseGeoFence(TiXmlElement* elemGeoFence, std::string& strGeoFence) {
  if (elemGeoFence) {
    const char* p = elemGeoFence->GetText();
    if (p) {
      strGeoFence = p;
    }
  }

  return 0;
}

int CSimulation::ParseGenerateInfo(TiXmlElement* elemGenerateInfo, CGenerateInfo& generateInfo) {
  if (!elemGenerateInfo) return -1;

  const char* p = elemGenerateInfo->GetText();
  if (p) generateInfo.m_strGenerateInfo = p;

  p = elemGenerateInfo->Attribute("origin");
  if (p) generateInfo.m_strOriginFile = p;

  return 0;
}

int CSimulation::ParseInfo(TiXmlElement* elemInfo, std::string& strInfo) {
  if (elemInfo) {
    const char* p = elemInfo->GetText();
    if (p) {
      strInfo = p;
    }
  }

  return 0;
}

int CSimulation::Save(const char* strFile) {
  if (!strFile) return -1;
  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemSimulation = new TiXmlElement("simulation");
  elemSimulation->SetAttribute("version", "1.0");

  auto ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
  elemSimulation->SetAttribute("date_version", std::to_string(ms.count()));
  elemSimulation->SetAttribute("sim_id", std::to_string(m_nID));

  // m_mapFile.m_strUnrealLevelIndex = m_planner.m_strUnrealLevel;
  TiXmlElement* elemMapfile = new TiXmlElement("mapfile");
  int nRet = SaveMapfile(elemMapfile, m_mapFile);
  if (nRet) {
    assert(false);
  }

  TiXmlElement* elemTraffic = new TiXmlElement("traffic");
  nRet = SaveTraffic(elemTraffic, m_strTraffic);
  if (nRet) {
    assert(false);
  }

  if (m_strSensor.length() > 0) {
    TiXmlElement* elemSensor = new TiXmlElement("sensor");
    nRet = SaveSensor(elemSensor, m_strSensor);
    if (nRet) {
      assert(false);
    }

    elemSimulation->LinkEndChild(elemSensor);
  }

  if (m_strEnvironment.length() > 0) {
    TiXmlElement* elemEnvironment = new TiXmlElement("environment");
    nRet = SaveEnvironment(elemEnvironment, m_strEnvironment);
    if (nRet) {
      assert(false);
    }

    elemSimulation->LinkEndChild(elemEnvironment);
  }

  // if (CEngineConfig::Instance().EditorConfigure().controlPathEnable == 1)
  {
    if (m_strControlPath.length() > 0) {
      TiXmlElement* elemControlPath = new TiXmlElement("controlPath");
      nRet = SaveControlPath(elemControlPath, m_strControlPath);
      if (nRet) {
        assert(false);
      }

      elemSimulation->LinkEndChild(elemControlPath);
    }
  }

  if (m_strGrading.length() > 0) {
    TiXmlElement* elemGrading = new TiXmlElement("grading");
    nRet = SaveGrading(elemGrading, m_strGrading);
    if (nRet) {
      assert(false);
    }

    elemSimulation->LinkEndChild(elemGrading);
  }

  TiXmlElement* elemPlanner = new TiXmlElement("planner");
  nRet = SavePlanner(elemPlanner, m_planner);
  if (nRet) {
    assert(false);
  }

  if (!m_rosbag.IsEmpty()) {
    TiXmlElement* elemRosbag = new TiXmlElement("rosbag");
    nRet = SaveRosbag(elemRosbag, m_rosbag);
    if (nRet) {
      assert(false);
    }

    elemSimulation->LinkEndChild(elemRosbag);
  }

  if (!m_l3States.IsEmpty()) {
    TiXmlElement* elemL3StateMachine = new TiXmlElement("l3_states");
    nRet = SaveL3States(elemL3StateMachine, m_l3States);
    if (nRet) {
      assert(false);
    }

    elemSimulation->LinkEndChild(elemL3StateMachine);
  }

  elemSimulation->LinkEndChild(elemMapfile);
  elemSimulation->LinkEndChild(elemTraffic);
  elemSimulation->LinkEndChild(elemPlanner);

  if (m_strGeoFence.size() > 0) {
    TiXmlElement* elemGeoFence = new TiXmlElement("geofence");
    nRet = SaveGeoFence(elemGeoFence, m_strGeoFence);
    if (nRet) {
      assert(false);
    }
    elemSimulation->LinkEndChild(elemGeoFence);
  }

  if (m_generateInfo.m_strOriginFile.size() > 0) {
    TiXmlElement* elemGenerateInfo = new TiXmlElement("generateinfo");
    nRet = SaveGenerateInfo(elemGenerateInfo, m_generateInfo);
    if (nRet) {
      assert(false);
    }
    elemSimulation->LinkEndChild(elemGenerateInfo);
  }

  if (m_strInfo.size() > 0) {
    TiXmlElement* elemInfo = new TiXmlElement("info");
    nRet = SaveInfo(elemInfo, m_strInfo);
    if (nRet) {
      assert(false);
    }
    elemSimulation->LinkEndChild(elemInfo);
  }

  if (m_parameterDeclarations.size() > 0) {
    TiXmlElement* elem = new TiXmlElement("ParameterDeclarations");
    for (auto&& parameter_declaration : m_parameterDeclarations) {
      TiXmlElement* one = new TiXmlElement("ParameterDeclaration");
      one->SetAttribute("name", parameter_declaration.name);
      one->SetAttribute("parameterType", parameter_declaration.parameterType);
      one->SetAttribute("value", parameter_declaration.value);
      elem->LinkEndChild(one);
    }
    elemSimulation->LinkEndChild(elem);
  }

  doc.LinkEndChild(dec);
  doc.LinkEndChild(elemSimulation);

  doc.SaveFile(strFile);
  return 0;
}

int CSimulation::SaveGenerateInfo(TiXmlElement* elemGenerateInfo, CGenerateInfo& generateInfo) {
  if (!elemGenerateInfo) return -1;

  elemGenerateInfo->SetAttribute("origin", generateInfo.m_strOriginFile);

  TiXmlText* txt = new TiXmlText(generateInfo.m_strGenerateInfo.c_str());
  elemGenerateInfo->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveMapfile(TiXmlElement* elemMapfile, CMapFile& mapfile) {
  if (!elemMapfile) return -1;

  elemMapfile->SetAttribute("lon", mapfile.m_strLon);
  elemMapfile->SetAttribute("lat", mapfile.m_strLat);
  elemMapfile->SetAttribute("alt", mapfile.m_strAlt);
  elemMapfile->SetAttribute("unrealLevelIndex", mapfile.m_strUnrealLevelIndex);

  TiXmlText* txt = new TiXmlText(mapfile.m_strMapfile.c_str());
  elemMapfile->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveTraffic(TiXmlElement* elemTraffic, std::string& strTraffic) {
  if (!elemTraffic) return -1;

  TiXmlText* txt = new TiXmlText(strTraffic.c_str());
  elemTraffic->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveSensor(TiXmlElement* elemSensor, std::string& strSensor) {
  if (!elemSensor) return -1;

  TiXmlText* txt = new TiXmlText(strSensor.c_str());
  elemSensor->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment) {
  if (!elemEnvironment) return -1;

  TiXmlText* txt = new TiXmlText(strEnvironment.c_str());
  elemEnvironment->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveControlPath(TiXmlElement* elemControlPath, std::string& strControlPath) {
  if (!elemControlPath) return -1;

  TiXmlText* txt = new TiXmlText(strControlPath.c_str());
  elemControlPath->LinkEndChild(txt);

  return 0;
}

int CSimulation::SaveGrading(TiXmlElement* elemGrading, std::string& strGrading) {
  if (!elemGrading) return -1;

  TiXmlText* txt = new TiXmlText(strGrading.c_str());
  elemGrading->LinkEndChild(txt);

  return 0;
}

int CSimulation::SavePlanner(TiXmlElement* elemPlanner, CPlanner& planner) {
  elemPlanner->SetAttribute("version", "2.0");
  // save egolist v2.0 param
  TiXmlElement* elemEgoList = new TiXmlElement("egolist");
  for (auto& it : planner.m_egos) {
    TiXmlElement* elemEgoInfo = new TiXmlElement("egoinfo");
    TiXmlElement* elemEgo = new TiXmlElement("ego");
    elemEgo->SetAttribute("type", it.second.m_strType);
    elemEgo->SetAttribute("name", it.second.m_strName);
    elemEgo->SetAttribute("group", it.second.m_strGroupName);

    TiXmlElement* elemBoundingBoxCenter = new TiXmlElement("boundingBoxCenter");
    elemBoundingBoxCenter->SetAttribute("x", it.second.m_boundingBox.m_strX);
    elemBoundingBoxCenter->SetAttribute("y", it.second.m_boundingBox.m_strY);
    elemBoundingBoxCenter->SetAttribute("z", it.second.m_boundingBox.m_strZ);

    TiXmlElement* elemDimensions = new TiXmlElement("dimensions");
    elemDimensions->SetAttribute("height", it.second.m_boundingBox.m_strX);
    elemDimensions->SetAttribute("length", it.second.m_boundingBox.m_strY);
    elemDimensions->SetAttribute("width", it.second.m_boundingBox.m_strZ);

    TiXmlElement* elemRoute = new TiXmlElement("route");
    int nRet = CTraffic::SaveOneRoute(elemRoute, it.second.m_route);
    if (nRet) {
      assert(false);
    }

    TiXmlElement* elemStartV = new TiXmlElement("start_v");
    TiXmlText* txt = new TiXmlText(it.second.m_strStartVelocity.c_str());
    elemStartV->LinkEndChild(txt);

    TiXmlElement* elemTheta = new TiXmlElement("theta");
    txt = new TiXmlText(it.second.m_strTheta.c_str());
    elemTheta->LinkEndChild(txt);

    TiXmlElement* elemVelocityMax = new TiXmlElement("Velocity_Max");
    txt = new TiXmlText(it.second.m_strVelocityMax.c_str());
    elemVelocityMax->LinkEndChild(txt);
    // acceleration_max
    TiXmlElement* elemAccelerationMax = new TiXmlElement("acceleration_max");
    txt = new TiXmlText(it.second.m_accelerationMax.c_str());
    elemAccelerationMax->LinkEndChild(txt);
    // deceleration_max
    TiXmlElement* elemDecelerationMax = new TiXmlElement("deceleration_max");
    txt = new TiXmlText(it.second.m_decelerationMax.c_str());
    elemDecelerationMax->LinkEndChild(txt);
    // control_longitudinal
    TiXmlElement* elemControlLongitudinal = new TiXmlElement("control_longitudinal");
    txt = new TiXmlText(it.second.m_controlLongitudinal.c_str());
    elemControlLongitudinal->LinkEndChild(txt);
    // control_lateral
    TiXmlElement* elemControlLateral = new TiXmlElement("control_lateral");
    txt = new TiXmlText(it.second.m_controlLateral.c_str());
    elemControlLateral->LinkEndChild(txt);
    TiXmlElement* elemTrajectoryEnabled = new TiXmlElement("trajectory_enabled");
    txt = new TiXmlText(it.second.m_trajectoryEnabled.c_str());
    elemTrajectoryEnabled->LinkEndChild(txt);
    // SensorV2
    TiXmlElement* sensorgroup = new TiXmlElement("sensorGroup");
    txt = new TiXmlText(it.second.m_sensorId.c_str());
    sensorgroup->LinkEndChild(txt);
    // scene_event
    TiXmlElement* elemSceneEvents = new TiXmlElement("scene_event");
    elemSceneEvents->SetAttribute("version", SCENE_EVENT_VERSION);  // 设定 scene_event 的版本
    nRet = SaveSceneEvents(elemSceneEvents, it.second.m_scenceEvents);
    if (nRet) {
      return -1;
    }
    // ego end track
    TiXmlElement* elemInputPath = new TiXmlElement("InputPath");
    elemInputPath->SetAttribute("points", it.second.m_inputPath.m_strPoints);

    elemEgoInfo->LinkEndChild(elemEgo);
    elemEgoInfo->LinkEndChild(elemBoundingBoxCenter);
    elemEgoInfo->LinkEndChild(elemDimensions);
    elemEgoInfo->LinkEndChild(elemRoute);
    elemEgoInfo->LinkEndChild(elemStartV);
    elemEgoInfo->LinkEndChild(elemTheta);
    elemEgoInfo->LinkEndChild(elemVelocityMax);
    elemEgoInfo->LinkEndChild(elemControlLongitudinal);
    elemEgoInfo->LinkEndChild(elemControlLateral);
    elemEgoInfo->LinkEndChild(elemAccelerationMax);
    elemEgoInfo->LinkEndChild(elemDecelerationMax);
    elemEgoInfo->LinkEndChild(elemDecelerationMax);
    elemEgoInfo->LinkEndChild(sensorgroup);
    elemEgoInfo->LinkEndChild(elemTrajectoryEnabled);
    elemEgoInfo->LinkEndChild(elemSceneEvents);

    if (!it.second.m_strStartAlt.empty()) {
      TiXmlElement* elemAltitude = new TiXmlElement("altitude");
      elemAltitude->SetAttribute("start", it.second.m_strStartAlt);
      elemAltitude->SetAttribute("end", it.second.m_strEndAlt);
      elemEgoInfo->LinkEndChild(elemAltitude);
    }
    elemEgoInfo->LinkEndChild(elemInputPath);
    // save control track
    if (it.second.m_trajectoryEnabled == "true") {
      // control track
      TiXmlElement* elemControlPath = new TiXmlElement("ControlPath");
      elemControlPath->SetAttribute("points", it.second.m_controlPath.getControlPath());
      elemControlPath->SetAttribute("sampleInterval", it.second.m_controlPath.m_strSampleInterval);
      elemEgoInfo->LinkEndChild(elemControlPath);
    }
    elemEgoList->LinkEndChild(elemEgoInfo);
  }
  elemPlanner->LinkEndChild(elemEgoList);
  // save egolist v1.0 param
  planner.ConvertFirstEgoParam();
  TiXmlElement* elemEgo = new TiXmlElement("ego");
  elemEgo->SetAttribute("type", planner.m_strType);
  elemEgo->SetAttribute("name", planner.m_strName);

  TiXmlElement* elemRoute = new TiXmlElement("route");
  int nRet = CTraffic::SaveOneRoute(elemRoute, planner.m_route);
  if (nRet) {
    assert(false);
  }

  TiXmlElement* elemStartV = new TiXmlElement("start_v");
  TiXmlText* txt = new TiXmlText(planner.m_strStartVelocity.c_str());
  elemStartV->LinkEndChild(txt);

  TiXmlElement* elemTheta = new TiXmlElement("theta");
  txt = new TiXmlText(planner.m_strTheta.c_str());
  elemTheta->LinkEndChild(txt);

  TiXmlElement* elemVelocityMax = new TiXmlElement("Velocity_Max");
  txt = new TiXmlText(planner.m_strVelocityMax.c_str());
  elemVelocityMax->LinkEndChild(txt);
  // acceleration_max
  TiXmlElement* elemAccelerationMax = new TiXmlElement("acceleration_max");
  txt = new TiXmlText(planner.m_accelerationMax.c_str());
  elemAccelerationMax->LinkEndChild(txt);
  // deceleration_max
  TiXmlElement* elemDecelerationMax = new TiXmlElement("deceleration_max");
  txt = new TiXmlText(planner.m_decelerationMax.c_str());
  elemDecelerationMax->LinkEndChild(txt);
  // control_longitudinal
  TiXmlElement* elemControlLongitudinal = new TiXmlElement("control_longitudinal");
  txt = new TiXmlText(planner.m_controlLongitudinal.c_str());
  elemControlLongitudinal->LinkEndChild(txt);
  // control_lateral
  TiXmlElement* elemControlLateral = new TiXmlElement("control_lateral");
  txt = new TiXmlText(planner.m_controlLateral.c_str());
  elemControlLateral->LinkEndChild(txt);
  TiXmlElement* elemTrajectoryEnabled = new TiXmlElement("trajectory_enabled");
  txt = new TiXmlText(planner.m_trajectoryEnabled.c_str());
  elemTrajectoryEnabled->LinkEndChild(txt);
  // SensorV2
  TiXmlElement* sensorgroup = new TiXmlElement("sensorGroup");
  txt = new TiXmlText(planner.m_sensorId.c_str());
  sensorgroup->LinkEndChild(txt);
  // scene_event
  TiXmlElement* elemSceneEvents = new TiXmlElement("scene_event");
  elemSceneEvents->SetAttribute("version", SCENE_EVENT_VERSION);  // 设定 scene_event 的版本
  nRet = SaveSceneEvents(elemSceneEvents, planner.m_scenceEvents);
  if (nRet) {
    // assert(false);
    return -1;
  }
  // ego end track
  TiXmlElement* elemInputPath = new TiXmlElement("InputPath");
  elemInputPath->SetAttribute("points", planner.m_inputPath.m_strPoints);

  elemPlanner->LinkEndChild(elemEgo);
  elemPlanner->LinkEndChild(elemRoute);
  elemPlanner->LinkEndChild(elemStartV);
  elemPlanner->LinkEndChild(elemTheta);
  elemPlanner->LinkEndChild(elemVelocityMax);
  elemPlanner->LinkEndChild(elemControlLongitudinal);
  elemPlanner->LinkEndChild(elemControlLateral);
  elemPlanner->LinkEndChild(elemAccelerationMax);
  elemPlanner->LinkEndChild(elemDecelerationMax);
  elemPlanner->LinkEndChild(elemDecelerationMax);
  elemPlanner->LinkEndChild(sensorgroup);

  elemPlanner->LinkEndChild(elemTrajectoryEnabled);
  elemPlanner->LinkEndChild(elemSceneEvents);

  if (!planner.m_strStartAlt.empty()) {
    TiXmlElement* elemAltitude = new TiXmlElement("altitude");
    elemAltitude->SetAttribute("start", planner.m_strStartAlt);
    elemAltitude->SetAttribute("end", planner.m_strEndAlt);
    elemPlanner->LinkEndChild(elemAltitude);
  }
  elemPlanner->LinkEndChild(elemInputPath);

  // save control track
  if (planner.m_trajectoryEnabled == "true") {
    // control track
    TiXmlElement* elemControlPath = new TiXmlElement("ControlPath");
    elemControlPath->SetAttribute("points", planner.m_controlPath.getControlPath());
    elemControlPath->SetAttribute("sampleInterval", planner.m_controlPath.m_strSampleInterval);
    elemPlanner->LinkEndChild(elemControlPath);
  }
  //
  return 0;
}

int CSimulation::SaveRosbag(TiXmlElement* elemRosbag, CRosbag& rosbag) {
  TiXmlText* path = new TiXmlText(rosbag.Path());
  elemRosbag->LinkEndChild(path);

  return 0;
}

int CSimulation::SaveL3States(TiXmlElement* elemL3States, CL3States& l3States) {
  CL3States::StateItems& states = l3States.States();
  for (int i = 0; i < states.size(); ++i) {
    TiXmlElement* elemState = new TiXmlElement("l3_state");
    std::string stateType = states[i].strStateType;
    std::string stateName = states[i].strStateName;
    std::string attrName = stateType + "." + stateName;
    elemState->SetAttribute(attrName.c_str(), states[i].nValue);
    int64_t lTriggerTime = states[i].dTriggerTime * 1000;
    TiXmlText* triggerTime = new TiXmlText(std::to_string(lTriggerTime));
    elemState->LinkEndChild(triggerTime);

    elemL3States->LinkEndChild(elemState);
  }

  return 0;
}

int CSimulation::SaveGeoFence(TiXmlElement* elemGeoFence, std::string& strGeoFence) {
  if (elemGeoFence) {
    TiXmlText* txt = new TiXmlText(strGeoFence.c_str());
    elemGeoFence->LinkEndChild(txt);
    return 0;
  }

  return -1;
}

int CSimulation::SaveInfo(TiXmlElement* elemInfo, std::string& strInfo) {
  if (elemInfo) {
    TiXmlText* txt = new TiXmlText(strInfo.c_str());
    elemInfo->LinkEndChild(txt);
    return 0;
  }

  return -1;
}

int CSimulation::ParseHadmapAttr(const char* strFileName) {
  if (!strFileName) {
    XML_LOGGER_ERROR("sim file name null");
    return -1;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFileName);

  if (!bRet) {
    XML_LOGGER_ERROR("sim format error!");
    return -1;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  // if (_stricmp(strName.c_str(), "simulation") != 0)
  if (!boost::algorithm::iequals(strName, "simulation")) return -1;

  // 解析地图参数数据
  TiXmlElement* elemMapFile = xmlRoot->FirstChildElement("mapfile");
  int nRet = ParseMapfile(elemMapFile, m_mapFile);
  if (nRet != 0) {
    return -1;
  }

  XML_LOGGER_INFO("Parse sim file : %s succeed!", strFileName);

  return 0;
}

int CSimulation::SaveSceneEvents(TiXmlElement* SceneEventsElem, SceneEventMap& mapSceneEvent) {
  if (!SceneEventsElem) return -1;
  auto itr = mapSceneEvent.begin();
  for (; itr != mapSceneEvent.end(); ++itr) {
    TiXmlElement* elemSceneEvent = new TiXmlElement("event");
    int nRet = SaveOneSceneEvent(elemSceneEvent, itr->second);
    if (nRet) {
      XML_LOGGER_ERROR("sceneEvent save error!");
      assert(false);
    }
    SceneEventsElem->LinkEndChild(elemSceneEvent);
  }
  return 0;
}

int CSimulation::SaveOneSceneEvent(TiXmlElement* SceneEventElem, SceneEvent& se) {
  if (!SceneEventElem) return -1;
  SceneEventElem->SetAttribute("id", se.m_id);
  SceneEventElem->SetAttribute("name", se.strName);
#ifdef SCENE_EVENTS_UPGRAD
  se.MergeString();
#else
  se.MergeString1_0();
#endif
  if (SCENE_EVENT_VERSION == "1.1.0.0") {
    if (se.ConditionVec.size() > 0) {
      std::string _typename = std::string("type");
      SceneEventElem->SetAttribute(_typename, se.ConditionVec.at(0).strType.c_str());
      std::string _conditonname = std::string("condition");
      SceneEventElem->SetAttribute(_conditonname, se.ConditionVec.at(0).mergeCondition().c_str());
    }
  } else {
    int index = 1;
    for (auto it : se.ConditionVec) {
      std::string _typename = std::string("type") + std::to_string(index);
      SceneEventElem->SetAttribute(_typename, it.strType.c_str());
      std::string _conditonname = std::string("condition") + std::to_string(index++);
      SceneEventElem->SetAttribute(_conditonname, it.mergeCondition().c_str());
    }
    SceneEventElem->SetAttribute("endCondition", se.strEndCondition);
    SceneEventElem->SetAttribute("action", se.strAction);
    SceneEventElem->SetAttribute("info", se.strInfo);
  }
  SceneEventElem->SetAttribute("conditionNumber", se.ConditionVec.size());
  SceneEventElem->SetAttribute("endCondition", se.strEndCondition);
  SceneEventElem->SetAttribute("action", se.strAction);
  SceneEventElem->SetAttribute("info", se.strInfo);
  return 0;
}

int CSimulation::ParseSceneEvents(TiXmlElement* SceneEventsElem, std::map<std::string, SceneEvent>& mapSceneEvents) {
  if (!SceneEventsElem) return -1;
  mapSceneEvents.clear();
  TiXmlElement* elemSceneEvent = SceneEventsElem->FirstChildElement("event");
  while (elemSceneEvent) {
    SceneEvent se;
    std::string version = SceneEventsElem->Attribute("version");
    int nRet = ParseOneSceneEvent(elemSceneEvent, se, version);
    if (nRet) {
      elemSceneEvent = elemSceneEvent->NextSiblingElement("event");
      continue;
    }
    mapSceneEvents.insert(std::make_pair(se.m_id, se));
    elemSceneEvent = elemSceneEvent->NextSiblingElement("event");
  }

  return 0;
}

int CSimulation::ParseOneSceneEvent(TiXmlElement* elemSceneEvent, SceneEvent& se, std::string version) {
  if (!elemSceneEvent) return -1;
  const char* p = elemSceneEvent->Attribute("id");
  if (p) se.m_id = p;

  if (version == "1.0.0.0" || version == "1.1.0.0") {
    SceneCondition condition;
    p = elemSceneEvent->Attribute("type");
    if (p) condition.strType = p;
    p = elemSceneEvent->Attribute("condition");
    std::string strCondition;
    if (p) strCondition = p;
    if (condition.loadCondition(strCondition)) se.ConditionVec.push_back(condition);
  } else {
    SceneCondition condition;
    // the most support num is 9
    for (int i = 1; i < 10; i++) {
      std::string _Type = std::string("type") + std::to_string(i);
      std::string _Conditon = std::string("condition") + std::to_string(i);
      if (elemSceneEvent->Attribute(_Type.c_str()) && elemSceneEvent->Attribute(_Conditon.c_str())) {
        SceneCondition condition;
        p = elemSceneEvent->Attribute(_Type.c_str());
        if (p) condition.strType = p;
        p = elemSceneEvent->Attribute(_Conditon.c_str());
        std::string strCondition;
        if (p) strCondition = p;
        if (condition.loadCondition(strCondition)) se.ConditionVec.push_back(condition);
      }
    }
  }

  p = elemSceneEvent->Attribute("name");
  if (p) se.strName = p;

  p = elemSceneEvent->Attribute("endCondition");
  if (p) se.strEndCondition = p;

  p = elemSceneEvent->Attribute("action");
  if (p) se.strAction = p;

  p = elemSceneEvent->Attribute("info");
  if (p) se.strInfo = p;
  if (version == "1.0.0.0") {
    se.SegmentString1_0();
  } else if (version == "1.1.0.0" || version == "1.2.0.0") {
    se.SegmentString();
  } else {
    SYSTEM_LOGGER_ERROR("Simplan ParseOneSceneEvent verion is error");
    return -1;
  }
  return 0;
}
