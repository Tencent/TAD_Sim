// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#define USE_PB_LOGSIM (1)
#include <forward_list>
#include <map>
#include <unordered_map>
#include "location.pb.h"
#include "traffic.pb.h"
#include "trajectory.pb.h"
#include "tx_simulation_loop.h"
#include "txsim_module.h"
#if !USE_PB_LOGSIM
#  include "player/player.h"
#endif

#include "tx_sim_time.h"
#include "tx_tadsim_flags.h"
#define _Pedestrian_ (1)
#define _Trajectory_ (1)
class InterpolatePlayerModule : public Base::txSimulationTemplate {
 public:
  InterpolatePlayerModule() = default;
  virtual ~InterpolatePlayerModule() = default;

  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT;
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT;
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT;
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT;

  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE {}
  virtual Base::TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  virtual void PreSimulation(tx_sim::StepHelper& helper,
                             const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {};
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {};
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {};
  virtual Base::TimeParamManager MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT TX_OVERRIDE {
    return Base::TimeParamManager(0.0, 0.0, 0.0, 0.0);
  }
  virtual Base::txBool IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual void ResetVars() TX_NOEXCEPT {}

 private:
  std::vector<std::string> ParseStrings(const std::string& concat, char delim = ' ');

  sim_msg::Traffic InterpTraffic(double time_ms);
  sim_msg::Location InterpLocation(double time_ms);
  sim_msg::Trajectory InterpTrajectory(double time_ms);

#if _Pedestrian_

 private:
  void InterpPedestrians(sim_msg::Traffic& traffic, double time_ms);
  void InterpObstacles(sim_msg::Traffic& traffic, double time_ms);
  void InterpLights(sim_msg::Traffic& traffic, double time_ms);
#endif /*_Pedestrian_*/

  double ComputeFrameTime(const sim_msg::Traffic& ref_traffic);
  sim_msg::Traffic SearchTraffic(double time_ms);
  sim_msg::Location SearchLocation(double time_ms);
  sim_msg::Trajectory SearchTrajectory(double time_ms);

 public:
  static Base::txBool Frame_Interpolation() TX_NOEXCEPT { return FLAGS_Logsim_FrameInterpolation; }
  static Base::txBool Heading_Interpolation() TX_NOEXCEPT { return FLAGS_Logsim_HeadingInterpolation; }

 protected:
  std::unordered_map<int64_t, std::map<double, sim_msg::Car>> cars_;
#if _Pedestrian_
  std::unordered_map<int64_t, std::map<double, sim_msg::DynamicObstacle>> pedestrians_;
  std::unordered_map<int64_t, std::map<double, sim_msg::StaticObstacle>> obstacles_;
  std::unordered_map<int64_t, std::map<double, sim_msg::TrafficLight>> lights_;
#endif /*_Pedestrian_*/
  std::map<double, sim_msg::Location> locations_;

#if USE_PB_LOGSIM
  bool Finished(const double time_stamp) const noexcept;
  sim_msg::TrafficRecords4Logsim trafficRecords_;
  Base::txFloat begin_time = 0.0;
#else
  std::forward_list<tx_sim::record::proto::SingleMessage> other_msgs_;
  std::unique_ptr<tx_sim::record::Player> player_ = nullptr;
#endif
  static const std::string TRAFFIC_TOPIC;
  static const std::string LOCATION_TOPIC;
  static const double cache_time_ms_;
  Base::txFloat m_time_diff_ns = 0.0;

  std::vector<std::tuple<double, sim_msg::Traffic>> m_vec_Traffic;
  std::vector<std::tuple<double, sim_msg::Location>> m_vec_Location;
  hadmap::txPoint m_originPoint;
};
