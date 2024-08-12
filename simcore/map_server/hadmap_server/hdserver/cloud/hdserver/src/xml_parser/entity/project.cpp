/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "project.h"

#include <tinyxml.h>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <cstring>
#include <iostream>
#include <utility>
#include "../../engine/config.h"
#include "common/log/xml_logger.h"

CProject::CProject() {
  m_strProjectPath = "";
  m_strSimulation = "";
  m_strTraffic = "";
  m_strSensor = "";
  m_strEnvironment = "";
  m_strControlPath = "";
  m_strGrading = "";
}

int CProject::SetProjectPath(const char* strProjectPath) {
  m_strProjectPath = strProjectPath;
  return 0;
}

int CProject::SetSimulationFile(const char* strSimu) {
  m_strSimulation = strSimu;
  return 0;
}

int CProject::SetTrafficFile(const char* strTrafficFileName) {
  if (!strTrafficFileName) {
    return -1;
  }

  m_strTraffic = strTrafficFileName;
  return 0;
}

int CProject::SetSensorFile(const char* strSensorFileName) {
  if (!strSensorFileName) {
    return -1;
  }

  m_strSensor = strSensorFileName;
  return 0;
}

int CProject::SetEnvironmentFile(const char* strEnvironmentFileName) {
  if (!strEnvironmentFileName) {
    return -1;
  }

  m_strEnvironment = strEnvironmentFileName;
  return 0;
}

int CProject::SetControlPathFile(const char* strControlPathFileName) {
  if (!strControlPathFileName) {
    return -1;
  }

  m_strControlPath = strControlPathFileName;
  return 0;
}

int CProject::SetGradingFile(const char* strGradingFileName) {
  if (!strGradingFileName) {
    return -1;
  }

  m_strGrading = strGradingFileName;
  return 0;
}

int CProject::Parse(const char* strFileName, bool bLoadHadmap /* = true*/) {
  if (!strFileName) {
    XML_LOGGER_ERROR("project file name null");
    return -1;
  }

  TiXmlDocument doc;
  bool bRet = doc.LoadFile(strFileName);

  if (!bRet) {
    XML_LOGGER_ERROR("project format error!");
    return -1;
  }

  TiXmlElement* xmlRoot = doc.RootElement();

  if (!xmlRoot) return -1;

  std::string strName = xmlRoot->Value();
  if (!boost::algorithm::iequals(strName, "project")) return -1;

  TiXmlElement* elemFiles = xmlRoot->FirstChildElement("files");
  if (!elemFiles) {
    return -1;
  }

  TiXmlElement* elemSimulation = elemFiles->FirstChildElement("simulation");
  int nRet = ParseSimulation(elemSimulation, m_strSimulation);
  if (nRet != 0) {
    return -1;
  }

  TiXmlElement* elemTraffic = elemFiles->FirstChildElement("traffic");
  nRet = ParseTraffic(elemTraffic, m_strTraffic);
  if (nRet != 0) {
    return -1;
  }

  // 解析传感器文件路径
  TiXmlElement* elemSensor = elemFiles->FirstChildElement("sensor");
  if (elemSensor) {
    nRet = ParseSensor(elemSensor, m_strSensor);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemControlPath = elemFiles->FirstChildElement("controlPath");
  if (elemControlPath) {
    nRet = ParseControlPath(elemControlPath, m_strControlPath);
    if (nRet != 0) {
      return -1;
    }
  }

  // 解析环境文件路径
  TiXmlElement* elemEnvironment = elemFiles->FirstChildElement("environment");
  if (elemEnvironment) {
    nRet = ParseEnvironment(elemEnvironment, m_strEnvironment);
    if (nRet != 0) {
      return -1;
    }
  }

  // 解析grading文件
  TiXmlElement* elemGrading = elemFiles->FirstChildElement("grading");
  if (elemGrading) {
    nRet = ParseGrading(elemGrading, m_strGrading);
    if (nRet != 0) {
      return -1;
    }
  }

  TiXmlElement* elemData = xmlRoot->FirstChildElement("data");
  if (!elemData) {
    return -1;
  }

  TiXmlElement* elemMeasurement = elemData->FirstChildElement("measurements");
  if (elemMeasurement) {
    nRet = ParseMeasurement(elemMeasurement, m_mapMeasurement);
    if (nRet != 0) {
      return -1;
    }
  }

  XML_LOGGER_INFO("Parse project file : %s succeed!", strFileName);

  return 0;
}

int CProject::ParseSimulation(TiXmlElement* elemSimulation, std::string& strSimulation) {
  if (!elemSimulation) return -1;

  const char* p = elemSimulation->GetText();
  if (p) strSimulation = p;

  return 0;
}

int CProject::ParseTraffic(TiXmlElement* elemTraffic, std::string& strTraffic) {
  if (!elemTraffic) return -1;

  const char* p = elemTraffic->GetText();
  if (p) strTraffic = p;

  return 0;
}

int CProject::ParseSensor(TiXmlElement* elemSensor, std::string& strSensor) {
  if (!elemSensor) return -1;

  const char* p = elemSensor->GetText();
  if (p) strSensor = p;

  return 0;
}

int CProject::ParseEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment) {
  if (!elemEnvironment) return -1;

  const char* p = elemEnvironment->GetText();
  if (p) strEnvironment = p;

  return 0;
}

int CProject::ParseControlPath(TiXmlElement* elemControlPath, std::string& strControlPath) {
  if (!elemControlPath) return -1;

  const char* p = elemControlPath->GetText();
  if (p) strControlPath = p;

  return 0;
}

int CProject::ParseGrading(TiXmlElement* elemGrading, std::string& strGrading) {
  if (!elemGrading) return -1;

  const char* p = elemGrading->GetText();
  if (p) strGrading = p;

  return 0;
}

int CProject::ParseMeasurement(TiXmlElement* elemMeasurements, MeasurementMap& mapMeasurement) {
  if (!elemMeasurements) return -1;

  mapMeasurement.clear();

  TiXmlElement* elemMeasurement = elemMeasurements->FirstChildElement("measurement");
  while (elemMeasurement) {
    CMeasurement m;

    int nRet = ParseOneMeasurement(elemMeasurement, m);
    if (nRet) {
      std::cout << "measurement format error!" << std::endl;
      assert(false);

      elemMeasurement = elemMeasurement->NextSiblingElement("measurement");
      continue;
    }

    mapMeasurement.insert(std::make_pair(m.m_strID, m));

    elemMeasurement = elemMeasurement->NextSiblingElement("measurement");
  }

  return 0;
}

int CProject::ParseOneMeasurement(TiXmlElement* elemMeasurment, CMeasurement& m) {
  if (!elemMeasurment) return -1;

  const char* p = elemMeasurment->Attribute("id");
  if (p) m.m_strID = p;

  p = elemMeasurment->Attribute("position");
  if (p) {
    m.m_strPositions = p;
  }

  return 0;
}

int CProject::Save(const char* strFile) {
  if (!strFile) return -1;
  TiXmlDocument doc;

  TiXmlDeclaration* dec = new TiXmlDeclaration("1.0", "utf-8", "yes");
  TiXmlElement* elemProject = new TiXmlElement("project");
  elemProject->SetAttribute("version", "1.0");

  TiXmlElement* elemFiles = new TiXmlElement("files");

  TiXmlElement* elemSimulation = new TiXmlElement("simulation");
  int nRet = SaveSimulation(elemSimulation, m_strSimulation);
  if (nRet) {
    assert(false);
  }
  elemFiles->LinkEndChild(elemSimulation);

  TiXmlElement* elemTraffic = new TiXmlElement("traffic");
  nRet = SaveTraffic(elemTraffic, m_strTraffic);
  if (nRet) {
    assert(false);
  }
  elemFiles->LinkEndChild(elemTraffic);

  if (m_strSensor.length() > 0) {
    TiXmlElement* elemSensor = new TiXmlElement("sensor");
    nRet = SaveSensor(elemSensor, m_strSensor);
    if (nRet) {
      assert(false);
    }

    elemFiles->LinkEndChild(elemSensor);
  }

  if (m_strEnvironment.length() > 0) {
    TiXmlElement* elemEnvironment = new TiXmlElement("environment");
    nRet = SaveEnvironment(elemEnvironment, m_strEnvironment);
    if (nRet) {
      assert(false);
    }

    elemFiles->LinkEndChild(elemEnvironment);
  }

  // if (CEngineConfig::Instance().EditorConfigure().controlPathEnable == 1)
  {
    if (m_strControlPath.length() > 0) {
      TiXmlElement* elemControlPath = new TiXmlElement("controlPath");
      nRet = SaveControlPath(elemControlPath, m_strControlPath);
      if (nRet) {
        assert(false);
      }

      elemControlPath->LinkEndChild(elemControlPath);
    }
  }

  if (m_strGrading.length() > 0) {
    TiXmlElement* elemGrading = new TiXmlElement("grading");
    nRet = SaveGrading(elemGrading, m_strGrading);
    if (nRet) {
      assert(false);
    }

    elemFiles->LinkEndChild(elemGrading);
  }

  //------ data
  TiXmlElement* elemData = new TiXmlElement("data");

  if (m_mapMeasurement.size() > 0) {
    TiXmlElement* elemMeasurements = new TiXmlElement("measurements");
    nRet = SaveMeasurement(elemMeasurements, m_mapMeasurement);
    if (nRet) {
      assert(false);
    }

    elemData->LinkEndChild(elemMeasurements);
  }

  doc.LinkEndChild(dec);
  elemProject->LinkEndChild(elemFiles);
  elemProject->LinkEndChild(elemData);
  doc.LinkEndChild(elemProject);

  doc.SaveFile(strFile);

  return 0;
}

int CProject::SaveSimulation(TiXmlElement* elemSimulation, std::string& strSimulation) {
  if (!elemSimulation) return -1;

  TiXmlText* txt = new TiXmlText(strSimulation.c_str());
  elemSimulation->LinkEndChild(txt);

  return 0;
}

int CProject::SaveTraffic(TiXmlElement* elemTraffic, std::string& strTraffic) {
  if (!elemTraffic) return -1;

  TiXmlText* txt = new TiXmlText(strTraffic.c_str());
  elemTraffic->LinkEndChild(txt);

  return 0;
}

int CProject::SaveSensor(TiXmlElement* elemSensor, std::string& strSensor) {
  if (!elemSensor) return -1;

  TiXmlText* txt = new TiXmlText(strSensor.c_str());
  elemSensor->LinkEndChild(txt);

  return 0;
}

int CProject::SaveEnvironment(TiXmlElement* elemEnvironment, std::string& strEnvironment) {
  if (!elemEnvironment) return -1;

  TiXmlText* txt = new TiXmlText(strEnvironment.c_str());
  elemEnvironment->LinkEndChild(txt);

  return 0;
}

int CProject::SaveControlPath(TiXmlElement* elemControlPath, std::string& strControlPath) {
  if (!elemControlPath) return -1;

  TiXmlText* txt = new TiXmlText(strControlPath.c_str());
  elemControlPath->LinkEndChild(txt);

  return 0;
}

int CProject::SaveGrading(TiXmlElement* elemGrading, std::string& strGrading) {
  if (!elemGrading) return -1;

  TiXmlText* txt = new TiXmlText(strGrading.c_str());
  elemGrading->LinkEndChild(txt);

  return 0;
}

int CProject::SaveMeasurement(TiXmlElement* elemMeasurements, MeasurementMap& mapMeasurements) {
  if (!elemMeasurements) return -1;

  MeasurementMap::iterator itr = mapMeasurements.begin();
  for (; itr != mapMeasurements.end(); ++itr) {
    TiXmlElement* elemMeasurement = new TiXmlElement("measurement");

    int nRet = SaveOneMeasurement(elemMeasurement, itr->second);
    if (nRet) {
      std::cout << "measurement save error!" << std::endl;
      assert(false);
    }

    elemMeasurements->LinkEndChild(elemMeasurement);
  }
  return 0;
}

int CProject::SaveOneMeasurement(TiXmlElement* elemMeasurement, CMeasurement& m) {
  if (!elemMeasurement) return -1;

  elemMeasurement->SetAttribute("id", m.m_strID);
  if (m.m_strPositions.size() > 0) {
    elemMeasurement->SetAttribute("position", m.m_strPositions);
  } else {
    elemMeasurement->SetAttribute("position", "");
  }
  return 0;
}
