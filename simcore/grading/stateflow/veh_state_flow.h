// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "manager/map_manager.h"
#include "stateflow/state_flow.h"
#include "utils/detection.h"
#include "utils/eval_common.h"

namespace eval {
// vehicle behavior type
typedef std::string VehBehavType;

// vehicle state type
namespace VehStateType {
const StateType Driving("Driving");
const StateType Reversing("Reversing");
const StateType Stopped("Stopped");
const StateType InJunction("InJunction");
const StateType LaneChanging("LaneChaning");
};  // namespace VehStateType

// vehicle behavior
namespace VehicleBehavior {
const VehBehavType Driving("Driving");
const VehBehavType LaneChaning_Left("LaneChaning_Left");
const VehBehavType LaneChaning_Right("LaneChaning_Right");
const VehBehavType TurnLeft("TurnLeft");
const VehBehavType TurnRight("TurnRight");
const VehBehavType U_TurnLeft("U_TurnLeft");
const VehBehavType U_TurnRight("U_TurnRight");
const VehBehavType StartUp("StartUp");
const VehBehavType Stopping("Stopping");
const VehBehavType Stopped("Stopped");
const VehBehavType Crawling("Crawling");
const VehBehavType Reversing("Reversing");
};  // namespace VehicleBehavior

// I/O and parameters of state-flow
struct VehInput : public FLowInput {
  double m_t_s;                // current simulation time, unit s
  LaneID m_lane_id;            // current lane id
  ActorMapInfoPtr m_map_info;  // actor map info
  CEgoAssemblePtr m_ego;       // ego assembly

  VehInput() {
    m_t_s = 0.0;
    m_lane_id.Clear();
    m_ego = nullptr;
    m_map_info.reset();
  }
};
struct VehOutput : public FlowOutput {
  VehBehavType m_veh_behav;      // vehicle behavior
  bool m_duration_valid;         // if lane change duration is valid
  double m_start_t, m_duration;  // lane change duration

  VehOutput(const VehBehavType &veh_behav = VehicleBehavior::Driving, bool duration_valid = false,
            double duration = 0.0)
      : m_veh_behav(veh_behav), m_duration(duration), m_duration_valid(duration_valid) {}
  inline void SetValue(VehBehavType veh_behav = VehicleBehavior::Driving, bool duration_valid = false,
                       double duration = 0.0) {
    m_veh_behav = veh_behav;
    m_duration_valid = duration_valid;
    m_duration = duration;
  }
  void DebugShow() const {
    VLOG_2 << "vehicle behavior:" << m_veh_behav << ", duration valid:" << m_duration_valid
           << ", duration:" << m_duration << "\n";
  }
};
struct VehParams : public FlowParams {
  // params for turn judgement
  const double m_left_turn_thresh = 45.0 / 180.0 * M_PI;         // unit, rad
  const double m_U_turn_left_thresh = 135.0 / 180.0 * M_PI;      // unit, rad
  const double m_right_turn_thresh = -45.0 / 180.0 * M_PI;       // unit, rad
  const double m_U_turn_right_thresh = -135.0 / 180.0 * M_PI;    // unit, rad
  const double m_lane_change_turn_thresh = 20.0 / 180.0 * M_PI;  // unit, rad

  // params for stop judgement
  const double m_stop_speed_thresh = 1.39;    // unit m/s, 5 km/h
  const double m_stop_speed_thresh_A = 0.56;  // unit m/s, 2 km/h
  const int64_t m_order_couner_thresh = 5;
};

// lane change state-flow
class VehFlow;      // lane change state-flow
class CLaneChange;  // lane changing state
class CDrive;       // driving state
class CStopped;     // stopped state
class CReverse;     // reverse state
class CInJunction;  // in junction state
class TurnDrive;    // driving state

// states and its map
using Veh_StatePtr = I_StatePtr<VehFlow>;
typedef std::unordered_map<StateType, Veh_StatePtr> VehFlowMap;

// vehicle behavior state-flow
class VehFlow final : public I_Flow<VehInput, VehOutput, VehParams>, public NoneCopyable {
 public:
  VehFlow() { m_flow_map.clear(); }
  virtual ~VehFlow() { m_flow_map.clear(); }

 public:
  // operation
  virtual bool Switch(const StateType &state_type) override;
  virtual bool Init() override;
  virtual bool Update(const VehInput &input) override;
  virtual bool Stop() override;

 protected:
  VehFlowMap m_flow_map;
  Veh_StatePtr m_active_state;
};

// lane changing state
class CLaneChange final : public I_State<VehFlow> {
 private:
  CLaneChange(VehFlow *veh_flow, StateType state_type) {
    m_flow = veh_flow;
    m_state_type = state_type;
  }

 public:
  static Veh_StatePtr Build(VehFlow *veh_flow) {
    Veh_StatePtr sh_ptr;
    sh_ptr.reset(new CLaneChange(veh_flow, VehStateType::LaneChanging));
    return sh_ptr;
  }

 public:
  virtual ~CLaneChange() {}

  // operation
  virtual bool OnEntry() override;
  virtual bool Update() override;
  virtual bool OnExit() override;

 protected:
  LaneID m_pre_lane_id;
  LaneID m_cur_lane_id;
  double m_t1, m_t2;
};

// driving state
class CDrive final : public I_State<VehFlow> {
 private:
  CDrive(VehFlow *veh_flow, StateType state_type) {
    m_t1 = 0.0;
    m_t2 = 0.0;
    m_flow = veh_flow;
    m_state_type = state_type;
  }

 public:
  static Veh_StatePtr Build(VehFlow *veh_flow) {
    Veh_StatePtr sh_ptr;
    sh_ptr.reset(new CDrive(veh_flow, VehStateType::Driving));
    return sh_ptr;
  }

 public:
  virtual ~CDrive() {}

  // operation
  virtual bool OnEntry() override;
  virtual bool Update() override;
  virtual bool OnExit() override;

 private:
  CLocation m_pre_lane_loc;
  CLocation m_cur_lane_loc;
  LaneID m_pre_lane_id;
  LaneID m_cur_lane_id;
  double m_t1, m_t2;
  RiseUpDetection<int> m_junc_detector = MakeRiseUpDetection<int>(0);
  RiseUpDetection<int> m_dotline_detector = MakeRiseUpDetection<int>(0);
};

// turn in junction state
class CInJunction final : public I_State<VehFlow> {
 private:
  CInJunction(VehFlow *veh_flow, StateType state_type) {
    m_flow = veh_flow;
    m_state_type = state_type;
  }

 public:
  static Veh_StatePtr Build(VehFlow *veh_flow) {
    Veh_StatePtr sh_ptr;
    sh_ptr.reset(new CInJunction(veh_flow, VehStateType::InJunction));
    return sh_ptr;
  }

 public:
  virtual ~CInJunction() {}

  // operation
  virtual bool OnEntry() override;
  virtual bool Update() override;
  virtual bool OnExit() override;

 protected:
  CLocation m_pre_lane_loc, m_cur_lane_loc;
  LaneID m_pre_laneid, m_cur_laneid;
  double m_t1, m_t2;
};

// stopped sate
class CStopped final : public I_State<VehFlow> {
 private:
  CStopped(VehFlow *veh_flow, StateType state_type) {
    m_flow = veh_flow;
    m_state_type = state_type;
  }

 public:
  static Veh_StatePtr Build(VehFlow *veh_flow) {
    Veh_StatePtr sh_ptr;
    sh_ptr.reset(new CStopped(veh_flow, VehStateType::Stopped));
    return sh_ptr;
  }

 public:
  virtual ~CStopped() {}

  // operation
  virtual bool OnEntry() override;
  virtual bool Update() override;
  virtual bool OnExit() override;

 protected:
  eval::RisingCheck<double> m_rise_check = RisingCheck<double>(static_cast<double>(INT32_MIN));
  eval::FallingCheck<double> m_fall_check = FallingCheck<double>(static_cast<double>(INT32_MAX));
};

// reverse state
class CReverse final : public I_State<VehFlow> {
 private:
  CReverse(VehFlow *veh_flow, StateType state_type) {
    m_flow = veh_flow;
    m_state_type = state_type;
  }

 public:
  static Veh_StatePtr Build(VehFlow *veh_flow) {
    Veh_StatePtr sh_ptr;
    sh_ptr.reset(new CReverse(veh_flow, VehStateType::Reversing));
    return sh_ptr;
  }

 public:
  virtual ~CReverse() {}

  // operation
  virtual bool OnEntry() override;
  virtual bool Update() override;
  virtual bool OnExit() override;
};
}  // namespace eval
