// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <sstream>
#include "tx_header.h"

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Sim)

#if __TX_Mark__("mapfile")
struct mapfile_t {
  Base::txFloat lon;
  Base::txFloat lat;
  Base::txFloat alt;
  Base::txString unrealLevelIndex;
  Base::txString mapfile;
  friend std::ostream& operator<<(std::ostream& os, const mapfile_t& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "{" << TX_VARS_NAME(lon, v.lon) << TX_VARS_NAME(lat, v.lat) << TX_VARS_NAME(alt, v.alt)
       << TX_VARS_NAME(unrealLevelIndex, v.unrealLevelIndex) << TX_VARS_NAME(map_path, v.mapfile) << "}";
    return os;
  }
};

#endif /*__TX_Mark__("mapfile")*/

#if __TX_Mark__("planner")

struct Ego {
  /*<ego type="suv"/>*/
  Base::txString type;
  Base::txString name;
  Base::txString group;
  friend std::ostream& operator<<(std::ostream& os, const Ego& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "ego : " << TX_VARS_NAME(type, v.type) << TX_VARS_NAME(name, v.name)
       << TX_VARS_NAME(group, v.group);
    return os;
  }
};

struct Altitude {
  /*<altitude start="41.78395658452064" end="41.499654094688594" />*/
  Base::txFloat start;
  Base::txFloat end;
  friend std::ostream& operator<<(std::ostream& os, const Altitude& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "altitude : " << TX_VARS_NAME(start, v.start) << TX_VARS_NAME(end, v.end);
    return os;
  }
};

struct Route {
  /*<route id="0" type="start_end" start="0.07803117451036837,0.034735825231097105"
   * end="0.09071559112964764,0.03464611556498887" />*/
  Base::txSysId id;
  Base::txString type;
  Base::txString start;
  Base::txString end;
  Base::txString mid;
  friend std::ostream& operator<<(std::ostream& os, const Route& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "route : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(type, v.type)
       << TX_VARS_NAME(start, v.start) << TX_VARS_NAME(end, v.end) << TX_VARS_NAME(mid, v.mid);
    return os;
  }
};

struct ControlPath {
  Base::txString points, sampleInterval;
  friend std::ostream& operator<<(std::ostream& os, const ControlPath& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "ControlPath : " << TX_VARS_NAME(start, v.points) << TX_VARS_NAME(end, v.sampleInterval);
    return os;
  }
};

struct Scene_event {
  Base::txString version;
  friend std::ostream& operator<<(std::ostream& os, const Scene_event& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "Scene_event : " << TX_VARS_NAME(version, v.version);
    return os;
  }
};

struct Input_Path {
  Base::txString points;
  friend std::ostream& operator<<(std::ostream& os, const Input_Path& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "InputPath : " << TX_VARS_NAME(points, v.points);
    return os;
  }
};

struct EgoInfo {
  Ego ego;
  Altitude altitude;
  Route route;
  Base::txFloat start_v;
  Base::txFloat theta;
  Base::txFloat Velocity_Max;
  Base::txString control_longitudinal;
  Base::txString control_lateral;
  Base::txFloat acceleration_max;
  Base::txFloat deceleration_max;
  Base::txString trajectory_enabled;
  Scene_event scene_event;
  Input_Path InputPath;
  ControlPath control_path;

  friend std::ostream& operator<<(std::ostream& os, const EgoInfo& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << v.ego << std::endl
       << v.altitude << std::endl
       << v.route << std::endl
       << TX_VARS_NAME(start_v, v.start_v) << std::endl
       << TX_VARS_NAME(theta, v.theta) << std::endl
       << TX_VARS_NAME(Velocity_Max, v.Velocity_Max) << std::endl

       << TX_VARS_NAME(control_longitudinal, v.control_longitudinal) << std::endl
       << TX_VARS_NAME(control_lateral, v.control_lateral) << std::endl
       << TX_VARS_NAME(acceleration_max, v.acceleration_max) << std::endl
       << TX_VARS_NAME(deceleration_max, v.deceleration_max) << std::endl

       << TX_VARS_NAME(trajectory_enabled, v.trajectory_enabled) << std::endl

       << TX_VARS_NAME(scene_event, v.scene_event) << std::endl
       << TX_VARS_NAME(InputPath, v.InputPath) << std::endl

       << TX_VARS_NAME(ControlPath, v.control_path) << std::endl;
    return os;
  }
};

struct Planner {
  Ego ego;
  Altitude altitude;
  Route route;
  Base::txFloat start_v;
  Base::txFloat theta;
  Base::txFloat Velocity_Max;
  Base::txString control_longitudinal;
  Base::txString control_lateral;
  Base::txFloat acceleration_max;
  Base::txFloat deceleration_max;
  Base::txString trajectory_enabled;
  Scene_event scene_event;
  Input_Path InputPath;
  ControlPath control_path;
  Base::txString version;
  std::vector<EgoInfo> ego_list;

  friend std::ostream& operator<<(std::ostream& os, const Planner& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << v.ego << std::endl
       << v.altitude << std::endl
       << v.route << std::endl
       << TX_VARS_NAME(start_v, v.start_v) << std::endl
       << TX_VARS_NAME(theta, v.theta) << std::endl
       << TX_VARS_NAME(Velocity_Max, v.Velocity_Max) << std::endl

       << TX_VARS_NAME(control_longitudinal, v.control_longitudinal) << std::endl
       << TX_VARS_NAME(control_lateral, v.control_lateral) << std::endl
       << TX_VARS_NAME(acceleration_max, v.acceleration_max) << std::endl
       << TX_VARS_NAME(deceleration_max, v.deceleration_max) << std::endl

       << TX_VARS_NAME(trajectory_enabled, v.trajectory_enabled) << std::endl

       << TX_VARS_NAME(scene_event, v.scene_event) << std::endl
       << TX_VARS_NAME(InputPath, v.InputPath) << std::endl

       << TX_VARS_NAME(ControlPath, v.control_path) << std::endl
       << TX_VARS_NAME(version, v.version) << std::endl;
    os << "ego_list : " << std::endl;
    for (const auto& refEgo : v.ego_list) {
      os << refEgo << std::endl;
    }
    return os;
  }
};

#endif /*__TX_Mark__("planner")*/
struct simulation {
  Base::txString traffic_file_path;

  mapfile_t _mapfile;
  Planner _planner;
  Base::txString _version;
  Base::txString _date_version;
  Base::txString _sim_id;
  friend std::ostream& operator<<(std::ostream& os, const simulation& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "simulation : " << TX_VARS_NAME(sim_id, v._sim_id) << TX_VARS_NAME(version, v._version)
       << TX_VARS_NAME(date_version, v._date_version) << std::endl
       << TX_VARS_NAME(map_info, v._mapfile)
       << std::endl
       /*<< TX_VARS_NAME(map_path, v._map_path) << std::endl*/
       << TX_VARS_NAME(traffic_file_path, v.traffic_file_path) << TX_VARS_NAME(planner, v._planner);
    return os;
  }
};

using simulation_ptr = std::shared_ptr<simulation>;

extern simulation_ptr load_scene_sim(const Base::txString& _sim_path) TX_NOEXCEPT;
extern void save_scene_sim(const Base::txString& _sim_path, simulation_ptr sim_ptr) TX_NOEXCEPT;
TX_NAMESPACE_CLOSE(Sim)
TX_NAMESPACE_CLOSE(SceneLoader)
