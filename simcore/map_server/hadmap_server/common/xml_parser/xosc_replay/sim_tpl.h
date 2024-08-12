/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "common/xml_parser/xosc_replay/tpl_macros.h"

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(ENG)

struct xmlMapfile {
  int unrealLevelIndex = 0;
  double lon = 0.0;
  double lat = 0.0;
  double alt = 0.0;
  std::string path;
  friend std::ostream& operator<<(std::ostream& os, const xmlMapfile& v) TX_NOEXCEPT {
    os << "<mapfile " << TX_VARS_NAME(lon, std::quoted(std::to_string(v.lon)))
       << TX_VARS_NAME(lat, std::quoted(std::to_string(v.lat))) << TX_VARS_NAME(alt, std::quoted(std::to_string(v.alt)))
       << TX_VARS_NAME(unrealLevelIndex, std::quoted(std::to_string(v.unrealLevelIndex))) << ">" << v.path
       << "</mapfile>";
    return os;
  }
};

struct xmlEgo {
  std::string type;
  std::string name;
  friend std::ostream& operator<<(std::ostream& os, const xmlEgo& v) TX_NOEXCEPT { return os; }
};

struct xmlAltitude {
  double start = 0.0;
  double end = 0.0;
  friend std::ostream& operator<<(std::ostream& os, const xmlAltitude& v) TX_NOEXCEPT { return os; }
};

struct xmlRoute {
  int id = 0;
  std::string type;
  std::string start;
  std::string end;
  std::string mid;
  friend std::ostream& operator<<(std::ostream& os, const xmlRoute& v) TX_NOEXCEPT { return os; }
};

struct xmlSceneEvent {
  std::string version = "1.2.0.0";
  friend std::ostream& operator<<(std::ostream& os, const xmlSceneEvent& v) TX_NOEXCEPT { return os; }
};

struct xmlInputPath {
  std::string points;
  friend std::ostream& operator<<(std::ostream& os, const xmlInputPath& v) TX_NOEXCEPT { return os; }
};

struct xmlControlPath {
  std::string points;
  std::string sampleInterval;
  friend std::ostream& operator<<(std::ostream& os, const xmlControlPath& v) TX_NOEXCEPT { return os; }
};

struct xmlPlanner {
  xmlEgo ego;
  xmlAltitude altitude;
  xmlRoute route;
  double start_v = 0.0;
  double theta = 0.0;
  double Velocity_Max = 0.0;
  std::string control_longitudinal;
  std::string control_lateral;
  double acceleration_max = 0.0;
  double deceleration_max = 0.0;
  std::string trajectory_enabled;
  double altitude_start = 0.0;
  double altitude_end = 0.0;
  xmlSceneEvent scene_event;
  xmlInputPath InputPath;
  xmlControlPath ControlPath;
  friend std::ostream& operator<<(std::ostream& os, const xmlPlanner& v) TX_NOEXCEPT { return os; }
};

struct xmlSimulation {
  std::string version;
  std::string date_version;
  std::string sim_id;
  xmlMapfile mapfile;
  std::string traffic;
  xmlPlanner planner;
  friend std::ostream& operator<<(std::ostream& os, const xmlSimulation& v) TX_NOEXCEPT {
    os << "<simulation " << TX_VARS_NAME(version, std::quoted(v.version))
       << TX_VARS_NAME(date_version, std::quoted(v.date_version)) << TX_VARS_NAME(sim_id, std::quoted(v.sim_id)) << ">"
       << std::endl;
    os << v.mapfile << std::endl << "</simulation>";
    return os;
  }
};

using SimulationPtr = std::shared_ptr<xmlSimulation>;

extern SimulationPtr Load(std::istream& stream);

extern SimulationPtr Load(const std::string& file);

extern std::string Dump(const SimulationPtr& sim) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(ENG)
TX_NAMESPACE_CLOSE(SIM)
