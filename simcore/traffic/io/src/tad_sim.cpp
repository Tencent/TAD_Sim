// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_sim.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "tx_path_utils.h"
#include "tx_string_utils.h"
#include "tx_xml_utils.h"
#include "xml/intros_ptree.hpp"
#include "xml/util_traits.hpp"
using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Sim)

BEGIN_INTROS_TYPE_USER_NAME(mapfile_t, "mapfile")
ADD_INTROS_ITEM_USER_NAME(lon, MAKE_USER_NAME("lon", "", true))
ADD_INTROS_ITEM_USER_NAME(lat, MAKE_USER_NAME("lat", "", true))
ADD_INTROS_ITEM_USER_NAME(alt, MAKE_USER_NAME("alt", "", true))
ADD_INTROS_ITEM_USER_NAME(unrealLevelIndex, MAKE_USER_NAME("unrealLevelIndex", "", true))
ADD_INTROS_ITEM_USER_NAME(mapfile, MAKE_USER_NAME("", "", false))
END_INTROS_TYPE(mapfile_t)

// ADD_INTROS_ITEM_USER_NAME(_map_path, MAKE_USER_NAME("", "", false))

BEGIN_INTROS_TYPE_USER_NAME(Ego, "ego")
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(group, MAKE_USER_NAME("group", "", true))
END_INTROS_TYPE(Ego)

BEGIN_INTROS_TYPE_USER_NAME(ControlPath, "ControlPath")
ADD_INTROS_ITEM_USER_NAME(points, MAKE_USER_NAME("points", "", true))
ADD_INTROS_ITEM_USER_NAME(sampleInterval, MAKE_USER_NAME("sampleInterval", "", true))
END_INTROS_TYPE(ControlPath)

BEGIN_INTROS_TYPE_USER_NAME(Altitude, "altitude")
ADD_INTROS_ITEM_USER_NAME(start, MAKE_USER_NAME("start", "", true))
ADD_INTROS_ITEM_USER_NAME(end, MAKE_USER_NAME("end", "", true))
END_INTROS_TYPE(Altitude)

BEGIN_INTROS_TYPE_USER_NAME(Route, "route")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(start, MAKE_USER_NAME("start", "", true))
ADD_INTROS_ITEM_USER_NAME(end, MAKE_USER_NAME("end", "", true))
ADD_INTROS_ITEM_USER_NAME(mid, MAKE_USER_NAME("mid", "", true))
END_INTROS_TYPE(Route)

BEGIN_INTROS_TYPE_USER_NAME(Scene_event, "scene_event")
ADD_INTROS_ITEM_USER_NAME(version, MAKE_USER_NAME("version", "", true))
END_INTROS_TYPE(Scene_event)

BEGIN_INTROS_TYPE_USER_NAME(Input_Path, "InputPath")
ADD_INTROS_ITEM_USER_NAME(points, MAKE_USER_NAME("points", "", true))
END_INTROS_TYPE(Input_Path)

BEGIN_INTROS_TYPE_USER_NAME(EgoInfo, "egoinfo")
ADD_INTROS_ITEM_USER_NAME(ego, "ego")
ADD_INTROS_ITEM_USER_NAME(altitude, "altitude")
ADD_INTROS_ITEM_USER_NAME(route, "route")
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", false))
ADD_INTROS_ITEM_USER_NAME(theta, MAKE_USER_NAME("theta", "", false))
ADD_INTROS_ITEM_USER_NAME(Velocity_Max, MAKE_USER_NAME("Velocity_Max", "", false))

ADD_INTROS_ITEM_USER_NAME(control_longitudinal, MAKE_USER_NAME("control_longitudinal", "", false))
ADD_INTROS_ITEM_USER_NAME(control_lateral, MAKE_USER_NAME("control_lateral", "", false))
ADD_INTROS_ITEM_USER_NAME(acceleration_max, MAKE_USER_NAME("acceleration_max", "", false))
ADD_INTROS_ITEM_USER_NAME(deceleration_max, MAKE_USER_NAME("deceleration_max", "", false))

ADD_INTROS_ITEM_USER_NAME(trajectory_enabled, MAKE_USER_NAME("trajectory_enabled", "", false))
ADD_INTROS_ITEM_USER_NAME(scene_event, "scene_event")
ADD_INTROS_ITEM_USER_NAME(InputPath, "InputPath")
ADD_INTROS_ITEM_USER_NAME(control_path, "ControlPath")
END_INTROS_TYPE(EgoInfo)

BEGIN_INTROS_TYPE_USER_NAME(Planner, "planner")
ADD_INTROS_ITEM_USER_NAME(version, MAKE_USER_NAME("version", "", false))
ADD_INTROS_ITEM_USER_NAME(ego, "ego")
ADD_INTROS_ITEM_USER_NAME(altitude, "altitude")
ADD_INTROS_ITEM_USER_NAME(route, "route")
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", false))
ADD_INTROS_ITEM_USER_NAME(theta, MAKE_USER_NAME("theta", "", false))
ADD_INTROS_ITEM_USER_NAME(Velocity_Max, MAKE_USER_NAME("Velocity_Max", "", false))

ADD_INTROS_ITEM_USER_NAME(control_longitudinal, MAKE_USER_NAME("control_longitudinal", "", false))
ADD_INTROS_ITEM_USER_NAME(control_lateral, MAKE_USER_NAME("control_lateral", "", false))
ADD_INTROS_ITEM_USER_NAME(acceleration_max, MAKE_USER_NAME("acceleration_max", "", false))
ADD_INTROS_ITEM_USER_NAME(deceleration_max, MAKE_USER_NAME("deceleration_max", "", false))

ADD_INTROS_ITEM_USER_NAME(trajectory_enabled, MAKE_USER_NAME("trajectory_enabled", "", false))
ADD_INTROS_ITEM_USER_NAME(scene_event, "scene_event")
ADD_INTROS_ITEM_USER_NAME(InputPath, "InputPath")
ADD_INTROS_ITEM_USER_NAME(control_path, "ControlPath")

ADD_INTROS_ITEM_USER_NAME(ego_list, MAKE_USER_NAME("egoinfo", "egolist", false))
END_INTROS_TYPE(Planner)

BEGIN_INTROS_TYPE_USER_NAME(simulation, "simulation")
ADD_INTROS_ITEM_USER_NAME(traffic_file_path, MAKE_USER_NAME("traffic", "", false))
ADD_INTROS_ITEM_USER_NAME(_mapfile, "mapfile")
ADD_INTROS_ITEM_USER_NAME(_planner, "planner")
ADD_INTROS_ITEM_USER_NAME(_version, MAKE_USER_NAME("version", "", true))
ADD_INTROS_ITEM_USER_NAME(_date_version, MAKE_USER_NAME("date_version", "", true))
ADD_INTROS_ITEM_USER_NAME(_sim_id, MAKE_USER_NAME("sim_id", "", true))
END_INTROS_TYPE(simulation)

simulation_ptr load_scene_sim(const Base::txString& _sim_path) TX_NOEXCEPT {
  std::ifstream infile(_sim_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    return std::make_shared<simulation>(make_intros_object<simulation>(tree));
  } else {
    LOG(WARNING) << "Load Xml File Failure. " << _sim_path;
    return nullptr;
  }
}

void save_scene_sim(const Base::txString& _sim_path, simulation_ptr sim_ptr) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  if (NonNull_Pointer(sim_ptr)) {
    ptree tree = make_ptree(*sim_ptr);
    xml_writer_settings<Base::txString> settings(' ', 4);
    std::stringstream fileStream;
    write_xml(fileStream, tree, settings);
    Base::txString s = fileStream.str();
    Utils::StringReplace(s, "<>", "");
    Utils::StringReplace(s, "</>", "");
    fileStream.str(s);

    std::ofstream outfile(_sim_path);
    outfile << fileStream.str();
    outfile.close();
    LOG(INFO) << "[save_sim] " << TX_COND(exists(FilePath(_sim_path)));
  } else {
    LOG(WARNING) << "sim ptr is nullptr";
  }
}

TX_NAMESPACE_CLOSE(Sim)
TX_NAMESPACE_CLOSE(SceneLoader)
