/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <set>
#include <string>

#ifdef PROJ_NEW_API
#  include <proj.h>
#else
#  ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#    define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#  endif
#  include "proj_api.h"
#endif
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "common/utils/base64.h"
#include "common/utils/status.h"
#include "common/xml_parser/xosc_replay/xosc_replay.h"
#include "traffic.pb.h"

class XOSCReplayConverter {
 public:
  struct Options {
    double frame_interval = 0.01;
    double write_debug_json = true;
    std::string map_filename;
  };

  using RepeatedPtrLocation = google::protobuf::RepeatedPtrField<sim_msg::Location>;

  using RepeatedPtrTraffic = google::protobuf::RepeatedPtrField<sim_msg::Traffic>;

  XOSCReplayConverter();

  ~XOSCReplayConverter();

  void ExpandCatalogReference();

  void ContractCatalogReference();

  // osc file to simrec
  utils::Status SerializeToSimrec(const std::string& xosc);

  utils::Status SerializeToSimrec(const std::string& xosc, const Options& opts);

  utils::Status SerializeToPoints(const std::string& xosc);

  // simlog to osc file
  utils::Status ParseFromSimlog(const std::string& simlog, const std::string& map_filename);

  utils::Status ParseFromSimlog(const std::string& simlog, const Options& opts);

 private:
  void ResetMapFilename(const std::string& xosc_filename, const Options& opts);

  /// parse
  /////////////////////////////////////////////////////////////////////////////////////////////////
  template <typename T>
  void PartialParse(const std::string& s, const char* ext_info_s, double t) {
    T msg;
    msg.ParseFromString(utils::Base64Decode(std::begin(s), std::end(s)));
    // std::cout << msg.ShortDebugString() << std::endl;
    PartialParse(msg, ext_info_s, t);
  }

  void PartialParse(const sim_msg::Location& loc, const char* ext_info_s, double t);

  void PartialParse(const sim_msg::Traffic& tfc, const char* ext_info_s, double t);

  void ParseHeader(const std::string& map_filename);

  void ParseVehicle(const sim_msg::Car& car, double t);

  void ParseSpeedAndTrajectory(const std::string& name, double t, double h, double x, double y, double z = 0.0,
                               double v = 0.0);
  /////////////////////////////////////////////////////////////////////////////////////////////////

  /// serialize
  /////////////////////////////////////////////////////////////////////////////////////////////////
  utils::Status GenSimContent(const char* simrec_filename, std::string& content);

  void PartialSerialize(RepeatedPtrLocation* locs, uint64_t& duration_ms, double frame_interval);

  void PartialSerialize(RepeatedPtrTraffic* tfcs, uint64_t& duration_ms, double frame_interval);

  /// 计算速度信息
  void CalcSpeed(const std::string& name, SIM::OSC::TrajectoryPtr);
  /// 设置静态信息
  template <typename T>
  void SerializeCommon(const SIM::OSC::ScenarioObjectPtr input, T*);
  /// 设置轨迹信息
  template <typename T>
  void SerializeTrajectory(const std::string& name, double t, SIM::OSC::TrajectoryPtr, T*);

  void SerializeTrajectory(const std::string& name, double t, SIM::OSC::TrajectoryPtr, sim_msg::StaticObstacle*);
  /////////////////////////////////////////////////////////////////////////////////////////////////

  std::string GetEgoEntityName() const { return ego_entity_name_; }

  void SetEgoEntityName(const std::string& ego_car_name) { ego_entity_name_ = ego_car_name; }

  template <typename T>
  void TransformXY(T& object);
  void TransformXY(sim_msg::Location& loc);
  void TransformXY(SIM::OSC::xmlWorldPosition& pos);
  void TransformXYImpl(double& x, double& y);

  // return osc date
  int64_t CurTime();

  // simulation time condition
  double RefTime(const std::string& name);

 private:
  int64_t cur_time_ms_;
  std::string ego_entity_name_;
  SIM::ODR::OpenDrivePtr odr_;
  SIM::OSC::OpenScenarioPtr osc_;
#ifdef PROJ_NEW_API
  // xodr to lon lat cs2cs
  PJ* m_PJ = NULL;
  // lonlat to xodr cs2cs
  PJ* m_PJ2 = NULL;
  PJ_CONTEXT* m_C = NULL;
#else
  std::map<std::string, projPJ> pj_map_;
#endif
  enum CatalogType { kMiscObjectCatalog = 0, kVehicleCatalog = 1, kPedestrianCatalog = 2, kDefaultCatalog = 3, kAll };

  static std::atomic_int catalog_init_flag_;
  static boost::shared_mutex catalog_rw_mutex_;
  static std::unordered_map<std::string, int> signal_state_map_;
  static std::unordered_map<std::string, SIM::OSC::OpenCatalogPtr> catalog_map_;
  static constexpr const char* catalog_name_[] = {"MiscObjectCatalog", "VehicleCatalog", "PedestrianCatalog",
                                                  "default"};
};
