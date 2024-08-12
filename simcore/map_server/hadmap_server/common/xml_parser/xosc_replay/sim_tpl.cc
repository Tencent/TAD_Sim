/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/sim_tpl.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "intros_ptree.hpp"
#include "util_traits.hpp"

using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(ENG)

// clang-format off
BEGIN_INTROS_TYPE_USER_NAME(xmlMapfile, "mapfile")
  ADD_INTROS_ITEM_USER_NAME(lon, MAKE_USER_NAME("lon", "", true))
  ADD_INTROS_ITEM_USER_NAME(lat, MAKE_USER_NAME("lat", "", true))
  ADD_INTROS_ITEM_USER_NAME(alt, MAKE_USER_NAME("alt", "", true))
  ADD_INTROS_ITEM_USER_NAME(unrealLevelIndex, MAKE_USER_NAME("unrealLevelIndex", "", true))
  ADD_INTROS_ITEM_USER_NAME(path, MAKE_USER_NAME("<xmltext>", "", false))
END_INTROS_TYPE(xmlMapfile)

BEGIN_INTROS_TYPE_USER_NAME(xmlEgo, "ego")
  ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
  ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
END_INTROS_TYPE(xmlEgo)

BEGIN_INTROS_TYPE_USER_NAME(xmlAltitude, "altitude")
  ADD_INTROS_ITEM_USER_NAME(start, MAKE_USER_NAME("start", "", true))
  ADD_INTROS_ITEM_USER_NAME(end, MAKE_USER_NAME("end", "", true))
END_INTROS_TYPE(xmlAltitude)

BEGIN_INTROS_TYPE_USER_NAME(xmlRoute, "route")
  ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
  ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
  ADD_INTROS_ITEM_USER_NAME(start, MAKE_USER_NAME("start", "", true))
  ADD_INTROS_ITEM_USER_NAME(end, MAKE_USER_NAME("end", "", true))
  ADD_INTROS_ITEM_USER_NAME(mid, MAKE_USER_NAME("mid", "", true))
END_INTROS_TYPE(xmlRoute)

BEGIN_INTROS_TYPE_USER_NAME(xmlSceneEvent, "scene_event")
  ADD_INTROS_ITEM_USER_NAME(version, MAKE_USER_NAME("version", "", true))
END_INTROS_TYPE(xmlSceneEvent)

BEGIN_INTROS_TYPE_USER_NAME(xmlInputPath, "InputPath")
  ADD_INTROS_ITEM_USER_NAME(points, MAKE_USER_NAME("points", "", true))
END_INTROS_TYPE(xmlInputPath)

BEGIN_INTROS_TYPE_USER_NAME(xmlControlPath, "ControlPath")
  ADD_INTROS_ITEM_USER_NAME(points, MAKE_USER_NAME("points", "", true))
  ADD_INTROS_ITEM_USER_NAME(sampleInterval, MAKE_USER_NAME("sampleInterval", "", true))
END_INTROS_TYPE(xmlControlPath)

BEGIN_INTROS_TYPE_USER_NAME(xmlPlanner, "planner")
  ADD_INTROS_ITEM_USER_NAME(ego, "ego")
  ADD_INTROS_ITEM_USER_NAME(altitude, "altitude")
  ADD_INTROS_ITEM_USER_NAME(route, "route")
  ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(theta, MAKE_USER_NAME("theta.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(Velocity_Max, MAKE_USER_NAME("Velocity_Max.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(control_longitudinal, MAKE_USER_NAME("control_longitudinal.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(control_lateral, MAKE_USER_NAME("control_lateral.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(acceleration_max, MAKE_USER_NAME("acceleration_max.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(deceleration_max, MAKE_USER_NAME("deceleration_max.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(trajectory_enabled, MAKE_USER_NAME("trajectory_enabled.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(scene_event, "scene_event")
  ADD_INTROS_ITEM_USER_NAME(InputPath, "InputPath")
  ADD_INTROS_ITEM_USER_NAME(ControlPath, "ControlPath")
END_INTROS_TYPE(xmlPlanner)

BEGIN_INTROS_TYPE_USER_NAME(xmlSimulation, "simulation")
  ADD_INTROS_ITEM_USER_NAME(version, MAKE_USER_NAME("version", "", true))
  ADD_INTROS_ITEM_USER_NAME(date_version, MAKE_USER_NAME("date_version", "", true))
  ADD_INTROS_ITEM_USER_NAME(sim_id, MAKE_USER_NAME("sim_id", "", true))
  ADD_INTROS_ITEM_USER_NAME(mapfile, MAKE_USER_NAME("mapfile", "", false))
  ADD_INTROS_ITEM_USER_NAME(traffic, MAKE_USER_NAME("traffic.<xmltext>", "", false))
  ADD_INTROS_ITEM_USER_NAME(planner, MAKE_USER_NAME("planner", "", false))
END_INTROS_TYPE(xmlSimulation)
// clang-format on

SimulationPtr Load(std::istream& stream) {
  if (!stream) {
    return nullptr;
  }
  ptree tree;
  int flags = 0;
  flags |= xml_parser::no_concat_text;
  flags |= xml_parser::trim_whitespace;
  read_xml(stream, tree, flags);
  return std::make_shared<xmlSimulation>(make_intros_object<xmlSimulation>(tree));
}

SimulationPtr Load(const std::string& file) {
  std::ifstream ifs(file);
  auto ptr = Load(ifs);
  ifs.close();
  return ptr;
}

std::string Dump(const SimulationPtr& xosc) TX_NOEXCEPT {
  if (!xosc) {
    return "";
  }

  std::ostringstream oss;
  ptree tree = make_ptree<xmlSimulation>(*xosc);
  write_xml(oss, tree, xml_writer_settings<std::string>(' ', 2));
  return oss.str();
}

TX_NAMESPACE_CLOSE(ENG)
TX_NAMESPACE_CLOSE(SIM)
