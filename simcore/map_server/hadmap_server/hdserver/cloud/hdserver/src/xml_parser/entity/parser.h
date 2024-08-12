/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "common/xml_parser/entity/environments/environment.h"
#include "common/xml_parser/entity/traffic.h"
#include "grading/grading.h"
#include "project.h"
#include "sensors/sensors.h"
#include "simulation.h"

struct sTagSimuTraffic {
  sTagSimuTraffic() {
    m_bIncludeSensors = false;
    m_bIncludeEnvironment = false;
    m_bIncludeGrading = false;
    m_strType = "sim";
    m_confPath = "";
    m_defaultXosc = "";
    m_vehicleXosc = "";
  }

  CProject m_project;
  CSimulation m_simulation;
  CTraffic m_traffic;
  CSensors m_sensors;
  CEnvironment m_environment;
  CGrading m_grading;
  bool m_bIncludeSensors;
  bool m_bIncludeEnvironment;
  bool m_bIncludeGrading;
  std::string m_strType;
  // the record id this scene belongs to
  int64_t m_nID;
  // add because to paramscene
  std::string m_path = "";
  // confPath
  std::string m_confPath = "";
  // default.xosc path
  std::string m_defaultXosc = "";
  // vehicle.xosc path
  std::string m_vehicleXosc = "";
  // xsd path
  std::string m_oscXsdPath = "";
  // Catalogs\\ Miscobjects \\ MiscObjectCatalog.xosc
  std::string getMiscObjectXoscPath() {
    boost::filesystem ::path path_conf(m_confPath);
    return (std::string(path_conf.parent_path().string()) + "/MiscObjects/MiscObjectCatalog.xosc");
  }
};

class CParser {
 public:
  typedef std::vector<sTagSimuTraffic> SimuTrafficVec;

  virtual int Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, bool bLoadHadmap = true);

  virtual int Parse(sTagSimuTraffic& simTrac, const char* strSimuFile, const char* strDirScene,
                    bool bLoadHadmap = true);

  static bool DirectoryExist(const char* strPath);
  static bool CreateDir(const char* strPath);

 protected:
  int ParseSim(sTagSimuTraffic& simTrac, const char* strSimuFile, const char* strDirScene, bool bLoadHadmap = true);
  int ParseXOSC(sTagSimuTraffic& simTrac, const char* strXoscFile, const char* strDirScene, bool bLoadHadmap = true);
};
