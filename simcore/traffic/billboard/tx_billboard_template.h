// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <list>
#include <numeric>
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "tbb/concurrent_hash_map.h"
#include "traffic.pb.h"
#include "traffic_billboard.pb.h"
#include "tx_enum_def.h"
#include "tx_locate_info.h"
#include "tx_simulation_loop.h"
#include "tx_time_utils.h"
#include "tx_billboard_lane_info_aver_speed.h"
#include "tx_billboard_ego.h"
#include "tx_billboard_flags.h"
TX_NAMESPACE_OPEN(TrafficFlow)
#if 0
class LaneInfoAverSpeed {
 public:
  struct SliceInfo {
    void Init(const Base::txFloat _pass_time_s) {
      pass_time_s = _pass_time_s;
      pass_time_ms = Utils::SecondToMillisecond(pass_time_s);
      sum_speed_ms = 0.0;
      sum_veh_cnt = 0;
    }

    Base::txFloat pass_time_s = 0.0;
    Base::txInt pass_time_ms = 0.0;
    Base::txFloat sum_speed_ms = 0.0;
    Base::txInt sum_veh_cnt = 0;
  };

 public:
  LaneInfoAverSpeed() TX_DEFAULT;
  ~LaneInfoAverSpeed() TX_DEFAULT;

 public:
  void ClearOldInfo(const Base::txFloat pass_time_s,
                    const Base::txInt buffer_time_ms /*5min = 5 * 60s*1000 = 300000*/) TX_NOEXCEPT;
  void RegisterVehInfo(const Base::txFloat pass_time_s, const Base::txFloat speed_ms) TX_NOEXCEPT;
  Base::txInt VehicleCountInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT;
  void ComputeAverSpeedMS(const Base::txFloat pass_time_s) TX_NOEXCEPT;
  Base::txFloat AverSpeedMSInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_aver_speed_ms; }
  Base::txFloat SumSpeedMSInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_sum_speed_ms; }
  Base::txInt SumVehCntInDuration(const Base::txFloat pass_time_s) const TX_NOEXCEPT { return cur_sum_veh_cnt; }

 protected:
  std::list<SliceInfo> list_info;
  Base::txFloat cur_aver_speed_ms = 0.0;
  Base::txInt cur_sum_veh_cnt = 0;
  Base::txFloat cur_sum_speed_ms = 0.0;
};
#endif
class BillboardTemplate : public Base::txSimulationTemplate {
 public:
  using ParentClass = Base::txSimulationTemplate;
  using RoadNetworkStatisticsDict =
      tbb::concurrent_hash_map<Base::Info_Lane_t, LaneInfoAverSpeed /*sim_msg::BillboardLaneInfo*/,
                               Utils::Info_Lane_t_HashCompare>;

 public:
  virtual void Init(tx_sim::InitHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Step(tx_sim::StepHelper& helper) TX_NOEXCEPT TX_OVERRIDE;
  virtual void Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT TX_OVERRIDE;

  virtual void CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void PreSimulation(tx_sim::StepHelper& helper,
                             const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void PostSimulation(tx_sim::StepHelper& helper,
                              const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid; }

 protected:
  virtual void ResetVars() TX_NOEXCEPT TX_OVERRIDE;
  virtual void CreateSystem() TX_NOEXCEPT TX_OVERRIDE {}
  virtual Base::TrafficElementSystemPtr TrafficSystemPtr() TX_NOEXCEPT TX_OVERRIDE { return nullptr; }
  static sim_msg::BillboardLaneInfo InfoLane2BillboardLaneInfo(const Base::Info_Lane_t& info_lane) TX_NOEXCEPT;
  virtual Base::TimeParamManager MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT TX_OVERRIDE;
  static Base::txBool ParseMapInitParams(tx_sim::ResetHelper& helper,
                                         HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT;
  const Billboard_Ego& Ego() const TX_NOEXCEPT { return m_in_ego; }
  Billboard_Ego& Ego() TX_NOEXCEPT { return m_in_ego; }
  virtual Base::txBool InitializeHdMapObject(hadmap::PointVec envelope) TX_NOEXCEPT;

 protected:
  Base::txFloat m_lastStepTime = 0.0;
  Base::txFloat m_passTime = 0.0;
  sim_msg::Traffic m_in_Traffic;
  sim_msg::Billboard m_out_Billboard;

  RoadNetworkStatisticsDict m_road_network_dict;
  Base::txBool m_isValid = false;
  Billboard_Ego m_in_ego;
};

using BillboardTemplatePtr = std::shared_ptr<BillboardTemplate>;

TX_NAMESPACE_CLOSE(TrafficFlow)
