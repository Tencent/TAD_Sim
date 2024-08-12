// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/variant.hpp>
#include "scene.pb.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_sim.h"
#include "tx_element_manager_base.h"
#include "tx_enum_def.h"
#include "tx_optional.h"
#include "tx_planning_flags.h"
#include "tx_planning_scene_loader.h"
TX_NAMESPACE_OPEN(SceneLoader)

class PlanningSceneEvent_SceneLoader : public Planning_SceneLoader {
 public:
  using txFloat = Base::txFloat;

#if __TX_Mark__("Position")
  struct IPositionWorld {
    using value_type = sim_msg::PositionWorld;
    virtual txBool gps(hadmap::txPoint& pos) const TX_NOEXCEPT = 0;
    virtual txBool rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionRelativeWorld {
    using value_type = sim_msg::PositionRelativeWorld;
    virtual txBool relative_world_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool relative_world_step(Base::txVec3& step) const TX_NOEXCEPT = 0;
    virtual txBool relative_world_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionRelativeObject {
    using value_type = sim_msg::PositionRelativeObject;
    virtual txBool relative_object_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool relative_object_step(Base::txVec3& step) const TX_NOEXCEPT = 0;
    virtual txBool relative_object_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionRoad {
    using value_type = sim_msg::PositionRoad;
    virtual txBool road_id(uint64_t& rid) const TX_NOEXCEPT = 0;
    virtual txBool road_st(Base::txVec2& _st) const TX_NOEXCEPT = 0;
    virtual txBool road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionRelativeRoad {
    using value_type = sim_msg::PositionRelativeRoad;
    virtual txBool relative_road_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool relative_road_st(Base::txVec2& _st) const TX_NOEXCEPT = 0;
    virtual txBool relative_road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionLane {
    using value_type = sim_msg::PositionLane;
    virtual txBool lane_road_id(uint64_t& rid) const TX_NOEXCEPT = 0;
    virtual txBool lane_lane_id(int64_t& lid) const TX_NOEXCEPT = 0;
    virtual txBool lane_st(Base::txVec2& _st) const TX_NOEXCEPT = 0;
    virtual txBool lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct IPositionRelativeLane {
    using value_type = sim_msg::PositionRelativeLane;
    virtual txBool relative_lane_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool relative_lane_step(int32_t& _s) const TX_NOEXCEPT = 0;
    virtual txBool relative_lane_st(Base::txVec2& _st) const TX_NOEXCEPT = 0;
    virtual txBool relative_lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT = 0;
  };

  struct Position : public IPositionWorld,
                    public IPositionRelativeWorld,
                    public IPositionRelativeObject,
                    public IPositionRoad,
                    public IPositionRelativeRoad,
                    public IPositionLane,
                    public IPositionRelativeLane {
    struct NonePosition {};
    enum class PositionType : Base::txInt {
      none = 0,
      world = 1,
      relative_world = 2,
      relative_object = 3,
      road = 4,
      relative_road = 5,
      lane = 6,
      relative_lane = 7
    };
    using PositionVariant =
        boost::variant<NonePosition, sim_msg::PositionWorld, sim_msg::PositionRelativeWorld,
                       sim_msg::PositionRelativeObject, sim_msg::PositionRoad, sim_msg::PositionRelativeRoad,
                       sim_msg::PositionLane, sim_msg::PositionRelativeLane>;

    void Clear() TX_NOEXCEPT;
    txBool Initialize(const sim_msg::Position& refPosition) TX_NOEXCEPT;
    txBool IsValid(const PositionType _type) const TX_NOEXCEPT { return (_type == position_type); }

    virtual txBool gps(hadmap::txPoint& pos) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool relative_world_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_world_step(Base::txVec3& step) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_world_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool relative_object_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_object_step(Base::txVec3& step) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_object_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool road_id(uint64_t& rid) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool road_st(Base::txVec2& _st) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool relative_road_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_road_st(Base::txVec2& _st) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_road_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool lane_road_id(uint64_t& rid) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool lane_lane_id(int64_t& lid) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool lane_st(Base::txVec2& _st) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool relative_lane_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_lane_step(int32_t& _s) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_lane_st(Base::txVec2& _st) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool relative_lane_rpy_radian(Base::txVec3& rpy) const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    PositionType position_type = PositionType::none;
    PositionVariant position_data = NonePosition();
  };
#endif

  struct Assign {
    using value_type = sim_msg::Assign;
    struct Cruise {
      txBool Initialize(const sim_msg::Assign_Cruise& _cruise) TX_NOEXCEPT {
        m_cruise.CopyFrom(_cruise);
        return true;
      }
      txBool IsResumeSW_ON() const TX_NOEXCEPT { return (sim_msg::OnOff::ONOFF_ON == m_cruise.resume_sw()); }
      txBool IsCancelSW_ON() const TX_NOEXCEPT { return (sim_msg::OnOff::ONOFF_ON == m_cruise.cancel_sw()); }
      txBool IsSpeedIncSW_ON() const TX_NOEXCEPT { return (sim_msg::OnOff::ONOFF_ON == m_cruise.speed_inc_sw()); }
      txBool IsSpeedDecSW_ON() const TX_NOEXCEPT { return (sim_msg::OnOff::ONOFF_ON == m_cruise.speed_dec_sw()); }
      txFloat set_timegap() const TX_NOEXCEPT { return m_cruise.set_timegap(); }
      txFloat set_speed() const TX_NOEXCEPT { return m_cruise.set_speed(); }

     protected:
      sim_msg::Assign_Cruise m_cruise;
    };

    txBool Initialize(const value_type& _activate) TX_NOEXCEPT {
      m_Assign.CopyFrom(_activate);
      if (m_Assign.has_cruise()) {
        m_op_cruise = Cruise();
        (*m_op_cruise).Initialize(m_Assign.cruise());
      } else {
        m_op_cruise.clear_value();
      }
      return true;
    }
    txString ControllerName() const TX_NOEXCEPT { return m_Assign.controller_name(); }
    txBool HasCruise() const TX_NOEXCEPT { return m_op_cruise.has_value(); }
    Cruise GetCruise() const TX_NOEXCEPT { return (*m_op_cruise); }
    txString Str() const TX_NOEXCEPT { return m_Assign.DebugString(); }
    sim_msg::Assign Raw() const TX_NOEXCEPT { return m_Assign; }

   protected:
    Base::txOptional<Cruise> m_op_cruise;
    sim_msg::Assign m_Assign;
  };

  struct Activate {
    using value_type = sim_msg::Activate;
    txBool Initialize(const sim_msg::Activate& _activate) TX_NOEXCEPT {
      m_activate.CopyFrom(_activate);
      return true;
    }
    sim_msg::Activate::Automode automode() const TX_NOEXCEPT { return m_activate.automode(); }
    txString Str() const TX_NOEXCEPT { return m_activate.ShortDebugString(); }
    sim_msg::Activate Raw() const TX_NOEXCEPT { return m_activate; }

   protected:
    sim_msg::Activate m_activate;
  };

  struct Override {
    using value_type = sim_msg::Override;
    txBool Initialize(const value_type& _override) TX_NOEXCEPT {
      m_override.CopyFrom(_override);
      return true;
    }
    txString Str() const TX_NOEXCEPT { return m_override.ShortDebugString(); }
    sim_msg::Override m_override;
  };

  struct Command {
    using value_type = sim_msg::Command;
    txBool Initialize(const value_type& _command) TX_NOEXCEPT {
      m_command.CopyFrom(_command);
      return true;
    }
    txString Str() const TX_NOEXCEPT { return m_command.ShortDebugString(); }
    sim_msg::Command m_command;
  };

  struct Status {
    using value_type = sim_msg::Status;
    txBool Initialize(const value_type& _status) TX_NOEXCEPT {
      m_status.CopyFrom(_status);
      return true;
    }
    txString Str() const TX_NOEXCEPT { return m_status.ShortDebugString(); }
    sim_msg::Status m_status;
  };

  struct Setting {
    txBool Initialize(const sim_msg::Setting& _setting) TX_NOEXCEPT {
      m_setting.CopyFrom(_setting);
      return true;
    }
    txString ScenarioPath() const TX_NOEXCEPT { return m_setting.scenario_path(); }
    txString HadmapPath() const TX_NOEXCEPT { return m_setting.hadmap_path(); }
    txString Version() const TX_NOEXCEPT { return m_setting.version(); }

   protected:
    sim_msg::Setting m_setting;
  };
#if __TX_Mark__("Condition")
  struct IConditionTimeToCollision {
    using value_type = sim_msg::ConditionTimeToCollision;
    virtual txBool TimeToCollision_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool TimeToCollision_distance_type(Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT = 0;
    virtual txBool TimeToCollision_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool TimeToCollision_value(Base::txFloat& _v) const TX_NOEXCEPT = 0;
  };

  struct IConditionTimeHeadWay {
    using value_type = sim_msg::ConditionTimeHeadway;
    virtual txBool TimeHeadWay_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool TimeHeadWay_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool TimeHeadWay_value(Base::txFloat& _v) const TX_NOEXCEPT = 0;
    virtual txBool TimeHeadWay_freespace(Base::txBool& _freespace) const TX_NOEXCEPT = 0;
    virtual txBool TimeHeadWay_along_route(Base::txBool& _along_route) const TX_NOEXCEPT = 0;
  };

  struct IConditionStoryboardElementState {
    using value_type = sim_msg::ConditionStoryboardElementState;
    virtual txBool ElementState_evId_actionId(std::tuple<Base::txSysId, Base::txInt>& evId_actId) const TX_NOEXCEPT = 0;
    virtual txBool ElementState_state(TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType& _rule) const
        TX_NOEXCEPT = 0;
  };

  struct IConditionSpeed {
    using value_type = sim_msg::ConditionSpeed;
    virtual txBool Speed_direction(Base::Enums::ConditionDirDimension& _dir) const TX_NOEXCEPT = 0;
    virtual txBool Speed_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool Speed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT = 0;
  };

  struct IConditionRelativeSpeed {
    using value_type = sim_msg::ConditionRelativeSpeed;
    virtual txBool RelativeSpeed_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool RelativeSpeed_direction(Base::Enums::ConditionDirDimension& _dir) const TX_NOEXCEPT = 0;
    virtual txBool RelativeSpeed_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool RelativeSpeed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT = 0;
  };

  struct IConditionReachPosition {
    using value_type = sim_msg::ConditionReachPosition;
    virtual txBool ReachPosition_radius_m(Base::txFloat& _r) const TX_NOEXCEPT = 0;
    virtual txBool ReachPosition_position(Position& _pos) const TX_NOEXCEPT = 0;
  };

  struct IConditionRelativeDistance {
    using value_type = sim_msg::ConditionRelativeDistance;
    virtual txBool RelativeDistance_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT = 0;
    virtual txBool RelativeDistance_distance_type(Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT = 0;
    virtual txBool RelativeDistance_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool RelativeDistance_value_m(Base::txFloat& _v) const TX_NOEXCEPT = 0;
  };

  struct IConditionEgoAttachLaneidCustom {
    using value_type = sim_msg::ConditionEgoAttachLaneidCustom;
    virtual txBool EgoAttachLaneidCustom_road_id(Base::txLaneUId& _laneUid) const TX_NOEXCEPT = 0;
  };

  struct IConditionSimulationTime {
    using value_type = sim_msg::ConditionSimulationTime;
    virtual txBool SimulationTime_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT = 0;
    virtual txBool SimulationTime_value_s(Base::txFloat& _v) const TX_NOEXCEPT = 0;
  };

  struct Condition : public IConditionTimeToCollision,
                     public IConditionSpeed,
                     public IConditionRelativeSpeed,
                     public IConditionReachPosition,
                     public IConditionRelativeDistance,
                     public IConditionEgoAttachLaneidCustom,
                     public IConditionSimulationTime,
                     public IConditionTimeHeadWay,
                     public IConditionStoryboardElementState {
    struct NoneCondition {};
    enum class ConditionType : Base::txInt {
      none = 0,
      ttc = 1,
      speed = 2,
      relative_speed = 3,
      reach_position = 4,
      distance = 5,
      ego_attach_laneid_custom = 6,
      simulation_time = 7,
      time_headway = 8,
      storyboard_element_state = 9
    };
    using ConditionVariant = boost::variant<
        NoneCondition, sim_msg::ConditionTimeToCollision, sim_msg::ConditionSpeed, sim_msg::ConditionRelativeSpeed,
        sim_msg::ConditionReachPosition, sim_msg::ConditionRelativeDistance, sim_msg::ConditionEgoAttachLaneidCustom,
        sim_msg::ConditionSimulationTime, sim_msg::ConditionTimeHeadway, sim_msg::ConditionStoryboardElementState>;

    virtual void Clear() TX_NOEXCEPT;
    virtual txBool Initialize(const sim_msg::Condition& refCondition) TX_NOEXCEPT;
    virtual txBool IsValid(const ConditionType _type) const TX_NOEXCEPT { return (_type == condition_type); }
    virtual ISceneEventViewer::kvMap KeyValueMap() const TX_NOEXCEPT;
    Base::Enums::ConditionBoundaryType edge() const TX_NOEXCEPT { return m_condition_boundary; }
    Base::txFloat delay() const TX_NOEXCEPT { return m_delay; }
    Base::txInt count() const TX_NOEXCEPT { return m_trigger_count; }

    virtual txBool TimeToCollision_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeToCollision_distance_type(Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual txBool TimeToCollision_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeToCollision_value(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool Speed_direction(Base::Enums::ConditionDirDimension& _dir) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool Speed_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool Speed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool RelativeSpeed_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool RelativeSpeed_direction(Base::Enums::ConditionDirDimension& _dir) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool RelativeSpeed_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool RelativeSpeed_value_m_s(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool ReachPosition_radius_m(Base::txFloat& _r) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool ReachPosition_position(Position& _pos) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool RelativeDistance_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool RelativeDistance_distance_type(Base::Enums::DistanceProjectionType& _type) const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual txBool RelativeDistance_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool RelativeDistance_value_m(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool EgoAttachLaneidCustom_road_id(Base::txLaneUId& _laneUid) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool SimulationTime_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool SimulationTime_value_s(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool TimeHeadWay_type_sysId(std::tuple<Base::Enums::ElementType, Base::txSysId>& type_sysId) const
        TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeHeadWay_rule(Base::Enums::ConditionEquationOp& _rule) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeHeadWay_value(Base::txFloat& _v) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeHeadWay_freespace(Base::txBool& _freespace) const TX_NOEXCEPT TX_OVERRIDE;
    virtual txBool TimeHeadWay_along_route(Base::txBool& _along_route) const TX_NOEXCEPT TX_OVERRIDE;

    virtual txBool ElementState_evId_actionId(std::tuple<Base::txSysId, Base::txInt>& evId_actId) const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual txBool ElementState_state(TrafficFlow::TAD_SceneEvent_1_0_0_0::SceneEventElementStatusType& _rule) const
        TX_NOEXCEPT TX_OVERRIDE;

   protected:
    ConditionType condition_type = ConditionType::none;
    ConditionVariant condition_data = NoneCondition();
    Base::Enums::ConditionBoundaryType m_condition_boundary = _plus_(Base::Enums::ConditionBoundaryType::none);
    Base::txFloat m_delay;
    Base::txInt m_trigger_count = 0;
  };
#endif
#if __TX_Mark__("Trigger")
  struct Trigger {
    txBool Initialize(const sim_msg::Trigger& refTrigger) TX_NOEXCEPT;
    Base::txString name() const TX_NOEXCEPT { return m_name; }
    std::vector<Condition> conditions() const TX_NOEXCEPT { return m_vec_condition; }

   protected:
    Base::txString m_name;
    std::vector<Condition> m_vec_condition;
  };
#endif
#if __TX_Mark__("ActionEgo")
  struct action_ego {
   public:
    struct NoneAction {};
    enum class ActionType : Base::txInt { none = 0, activate = 1, assign = 2, override = 3, command = 4, status = 5 };
    using ActionVariant = boost::variant<NoneAction, sim_msg::Activate, sim_msg::Assign, sim_msg::Override,
                                         sim_msg::Command, sim_msg::Status>;

    txBool Initialize(const sim_msg::Action& refActionEgo) TX_NOEXCEPT;
    void Clear() TX_NOEXCEPT;
    txBool IsValid(const ActionType _type) const TX_NOEXCEPT { return (_type == action_type); }

    txBool activate(Activate& _a) const TX_NOEXCEPT;
    txBool assign(Assign& _a) const TX_NOEXCEPT;
    txBool override_(Override& _a) const TX_NOEXCEPT;
    txBool command(Command& _a) const TX_NOEXCEPT;
    txBool status(Status& _a) const TX_NOEXCEPT;
    txString Str() const TX_NOEXCEPT;
    Base::txInt action_id() const TX_NOEXCEPT { return m_action_id; }
    void set_action_id(const Base::txInt _id) TX_NOEXCEPT { m_action_id = _id; }

   protected:
    ActionType action_type = ActionType::none;
    ActionVariant action_data = NoneAction();
    Base::txInt m_action_id;
  };

  struct IDummyActionViewer : public Base::ISceneLoader::ISceneEventViewer::IActionViewer {
    virtual ~IDummyActionViewer() TX_DEFAULT;
    virtual txBool initialize(const std::vector<action_ego>& action_ego_vec) TX_NOEXCEPT;
    virtual std::vector<action_ego> actions() const TX_NOEXCEPT { return m_action_vec; }

   protected:
    std::vector<action_ego> m_action_vec;
  };
  using IDummyActionViewerPtr = std::shared_ptr<IDummyActionViewer>;
#endif

#if __TX_Mark__("EventEgo")
  struct event_ego {
    txBool Initialize(const sim_msg::Event& _eventEgo) TX_NOEXCEPT;
    int32_t id() const TX_NOEXCEPT { return m_id; }
    Base::txString name() const TX_NOEXCEPT { return m_name; }
    Base::txString info() const TX_NOEXCEPT { return m_info; }
    Trigger trigger() const TX_NOEXCEPT { return m_trigger; }
    std::vector<action_ego> actions() const TX_NOEXCEPT { return m_action; }

   protected:
    int32_t m_id;
    Base::txString m_name;
    Base::txString m_info;
    Trigger m_trigger;
    std::vector<action_ego> m_action;
  };
  using DynamicEgo = std::vector<event_ego>;
#endif

#if __TX_Mark__("EgoPhysicleCommon")
  struct bounding_box {
    txBool Initialize(const sim_msg::BoundingBox& _bx) TX_NOEXCEPT {
      m_bbx.CopyFrom(_bx);
      return true;
    }
    hadmap::txPoint Center() const TX_NOEXCEPT {
      return hadmap::txPoint(m_bbx.center().x(), m_bbx.center().y(), m_bbx.center().z());
    }
    txFloat Length() const TX_NOEXCEPT { return m_bbx.length(); }
    txFloat Width() const TX_NOEXCEPT { return m_bbx.width(); }
    txFloat Higth() const TX_NOEXCEPT { return m_bbx.higth(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::BoundingBox m_bbx;
  };

  struct performance {
    txBool Initialize(const sim_msg::Performance& _p) TX_NOEXCEPT {
      m_perf.CopyFrom(_p);
      return true;
    }
    txFloat max_speed_m_s() const TX_NOEXCEPT { return m_perf.max_speed(); }
    txFloat max_accel_m_ss() const TX_NOEXCEPT { return m_perf.max_accel(); }
    txFloat max_decel_m_ss() const TX_NOEXCEPT { return m_perf.max_decel(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::Performance m_perf;
  };

  struct ego_physicle_common {
    txBool Initialize(const sim_msg::PhysicleCommon& _common) TX_NOEXCEPT {
      m_common.CopyFrom(_common);
      m_bounding_box.Initialize(m_common.bounding_box());
      return true;
    }
    bounding_box BoundingBox() const TX_NOEXCEPT { return m_bounding_box; }
    sim_msg::Color Color() const TX_NOEXCEPT { return m_common.color(); }
    Base::txBool Mass() const TX_NOEXCEPT { return m_common.mass(); }
    // Base::txString GeometryReference() const TX_NOEXCEPT { return m_common.geometry_reference(); }
    // sim_msg::ReferencePoint ReferencePoint() const TX_NOEXCEPT { return m_common.reference_point(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::PhysicleCommon m_common;
    bounding_box m_bounding_box;
  };

  // struct alxe
  // {
  //  txBool Initialize(const sim_msg::Alxe& _alxe) TX_NOEXCEPT { m_alxe.CopyFrom(_alxe); return true; }
  //  txFloat MaxSteering() const TX_NOEXCEPT { return m_alxe.max_steering(); }
  //  txFloat TrackWidth_m() const TX_NOEXCEPT { return m_alxe.track_width(); }
  //  txFloat WheelDiameter_m() const TX_NOEXCEPT { return m_alxe.wheel_diameter(); }
  //  txFloat PositionX() const TX_NOEXCEPT { return m_alxe.position_x(); }
  //  txFloat PositionZ() const TX_NOEXCEPT { return m_alxe.position_z(); }
  //  txString Str() const TX_NOEXCEPT;
  // protected:
  //  sim_msg::Alxe m_alxe;
  // };

  struct vehicle_geometory {
    txBool Initialize(const sim_msg::VehicleGeometory& _veh_geometory) TX_NOEXCEPT {
      m_geometory.CopyFrom(_veh_geometory);
      return true;
    };

   protected:
    sim_msg::VehicleGeometory m_geometory;
  };

  struct physicle_ego {
    txBool Initialize(const sim_msg::PhysicleEgo& _phy_ego) TX_NOEXCEPT {
      m_phy_ego.CopyFrom(_phy_ego);
      return true;
    }
    // alxe FrontAxle() const TX_NOEXCEPT { alxe retv; retv.Initialize(m_phy_ego.front_axle()); return std::move(retv);
    // } alxe RearAxle() const TX_NOEXCEPT { alxe retv; retv.Initialize(m_phy_ego.rear_axle()); return std::move(retv);
    // }
    ego_physicle_common Common() const TX_NOEXCEPT {
      ego_physicle_common retv;
      retv.Initialize(m_phy_ego.common());
      return std::move(retv);
    }
    performance Performance() const TX_NOEXCEPT {
      performance retv;
      retv.Initialize(m_phy_ego.performance());
      return std::move(retv);
    }

    // txFloat Offset_X()const TX_NOEXCEPT { return m_phy_ego.geometory(); }
    // txFloat Offset_Z()const TX_NOEXCEPT { return m_phy_ego.offset_z(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::PhysicleEgo m_phy_ego;
  };

  struct Waypoint {
    txBool Initialize(const sim_msg::Waypoint& _waypoint) TX_NOEXCEPT {
      m_waypoint.CopyFrom(_waypoint);
      return true;
    }
    // txString LaneId() const TX_NOEXCEPT { return m_waypoint.lane_id(); }
    // txFloat Start_S() const TX_NOEXCEPT { return m_waypoint.start_s(); }
    // txFloat LOffset() const TX_NOEXCEPT { return m_waypoint.l_offset(); }
    Position GetPosition() const TX_NOEXCEPT {
      Position _p;
      _p.Initialize(m_waypoint.position());
      return std::move(_p);
    }
    txFloat Speed_m_s() const TX_NOEXCEPT { return m_waypoint.speed().value(); }
    txFloat Acc_m_ss() const TX_NOEXCEPT { return m_waypoint.accel().value(); }
    txFloat HeadingAngle_Radian() const TX_NOEXCEPT {
      return Math::Degrees2Radians(m_waypoint.heading_angle().value());
    }
    txFloat FrontWheelSteerAngle_Radian() const TX_NOEXCEPT {
      return Math::Degrees2Radians(m_waypoint.front_wheel_steer_angle().value());
    }
    sim_msg::GearSts Gear() const TX_NOEXCEPT { return m_waypoint.gear(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::Waypoint m_waypoint;
  };
  using WaypointVec = std::vector<Waypoint>;

  struct initial_ego {
    txBool Initialize(const sim_msg::InitialEgo& _initial) TX_NOEXCEPT {
      m_initial.CopyFrom(_initial);
      return true;
    }
    txFloat Speed() const TX_NOEXCEPT { return m_initial.common().speed(); }
    Assign assign() const TX_NOEXCEPT {
      Assign _assign;
      _assign.Initialize(m_initial.assign());
      return std::move(_assign);
    }
    Activate activate() const TX_NOEXCEPT {
      Activate _activate;
      _activate.Initialize(m_initial.activate());
      return std::move(_activate);
    }
    WaypointVec Waypoints() const TX_NOEXCEPT {
      WaypointVec retV;
      for (const auto& refwp : m_initial.common().waypoints()) {
        Waypoint wp;
        wp.Initialize(refwp);
        retV.emplace_back(std::move(wp));
      }
      return retV;
    }
    txBool trajectory_enabled() const TX_NOEXCEPT { return m_initial.trajectory_enabled(); }
    txString Str() const TX_NOEXCEPT;

   protected:
    sim_msg::InitialEgo m_initial;
  };

  struct extrinsic {
    txBool Initialize(const sim_msg::Sensor_Extrinsic& _extrinsic) TX_NOEXCEPT {
      m_extrinsic.CopyFrom(_extrinsic);
      return true;
    }
    int64_t ID() const TX_NOEXCEPT { return m_extrinsic.id(); }
    txString InstallSlot() const TX_NOEXCEPT { return m_extrinsic.installslot(); }
    txString Device() const TX_NOEXCEPT { return m_extrinsic.device(); }
    txFloat LocationX() const TX_NOEXCEPT { return m_extrinsic.locationx(); }
    txFloat LocationY() const TX_NOEXCEPT { return m_extrinsic.locationy(); }
    txFloat LocationZ() const TX_NOEXCEPT { return m_extrinsic.locationz(); }
    txFloat RotationX() const TX_NOEXCEPT { return m_extrinsic.rotationx(); }
    txFloat RotationY() const TX_NOEXCEPT { return m_extrinsic.rotationy(); }
    txFloat RotationZ() const TX_NOEXCEPT { return m_extrinsic.rotationz(); }

   protected:
    sim_msg::Sensor_Extrinsic m_extrinsic;
  };

  struct Sensor {
    txBool Initialize(const sim_msg::Sensor& _sensor) TX_NOEXCEPT {
      m_sensor.CopyFrom(_sensor);
      return true;
    }
    sim_msg::SensorType Type() const TX_NOEXCEPT { return m_sensor.type(); }
    std::map<txString, txString> Intrinsic() const TX_NOEXCEPT {
      std::map<txString, txString> retM;
      for (const auto& refPair : m_sensor.intrinsic().params()) {
        retM[refPair.first] = refPair.second;
      }
      return std::move(retM);
    }
    extrinsic Extrinsic() const TX_NOEXCEPT {
      extrinsic _e;
      _e.Initialize(m_sensor.extrinsic());
      return std::move(_e);
    }

   protected:
    sim_msg::Sensor m_sensor;
  };
  using SensorVec = std::vector<Sensor>;
  struct sensor_group {
    txBool Initialize(const sim_msg::SensorGroup& _sensor_group) TX_NOEXCEPT {
      m_sensor_group.CopyFrom(_sensor_group);
      return true;
    }
    txString Name() const TX_NOEXCEPT { return m_sensor_group.name(); }
    sim_msg::SensorGroup Raw() const TX_NOEXCEPT { return m_sensor_group; }
    SensorVec&& sensors(const sim_msg::SensorType _type) const TX_NOEXCEPT {
      SensorVec retV;
      for (const auto& ref : m_sensor_group.sensors()) {
        if (_type == ref.type()) {
          Sensor s;
          s.Initialize(ref);
          retV.emplace_back(std::move(s));
        }
      }
      return std::move(retV);
    }

    SensorVec&& Cameras() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_CAMERA); }
    SensorVec&& Lidars() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_TRADITIONAL_LIDAR); }
    SensorVec&& Radars() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_RADAR); }
    SensorVec&& Ultrasonics() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_ULTRASONIC); }
    SensorVec&& Gpss() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_GPS); }
    SensorVec&& Imus() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_IMU); }
    SensorVec&& Obus() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_OBU); }
    SensorVec&& Truth() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_TRUTH); }
    SensorVec&& FishEyes() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_FISHEYE); }
    SensorVec&& Semantics() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_SEMANTIC); }
    SensorVec&& Depths() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_DEPTH); }
    SensorVec&& UserDefine() const TX_NOEXCEPT { return sensors(sim_msg::SensorType::SENSOR_TYPE_USER_DEFINE); }

   protected:
    sim_msg::SensorGroup m_sensor_group;
  };
#endif

 public:
  class IDummySceneEventViewer : public ISceneEventViewer {
   public:
    using ParentClass = ISceneEventViewer;
#if __TX_Mark__("ConditionSpeed")
    struct IDummySpeedTriggerConditionViewer : public IConditionViewer {
     public:
      using SceneEventType = Base::Enums::SceneEventType;
      using ConditionDirDimension = Base::Enums::ConditionDirDimension;
      using txSize = Base::txSize;

      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_speed);
      }
      virtual ConditionDirDimension direction() const TX_NOEXCEPT { return mDirection; }
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }
      virtual txFloat speed_threshold() const TX_NOEXCEPT { return m_speed_threshold; }
      virtual ConditionSpeedType speed_type() const TX_NOEXCEPT { return mSpeedType; }
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }
      virtual txFloat delay_threshold() const TX_NOEXCEPT { return m_delay_threshold; }
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

     protected:
      const ConditionSpeedType mSpeedType = _plus_(ConditionSpeedType::absolute);
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txFloat m_speed_threshold = 0.0;
      ConditionDirDimension mDirection;
      txSize m_trigger_count = 1;
      txFloat m_delay_threshold = 0.0;
    };
    using IDummySpeedTriggerConditionViewerPtr = std::shared_ptr<IDummySpeedTriggerConditionViewer>;
#endif /*__TX_Mark__("ConditionSpeed")*/

#if __TX_Mark__("ConditionRelativeSpeed")
    struct IDummyRelativeSpeedTriggerConditionViewer : public IConditionViewer {
     public:
      using SceneEventType = Base::Enums::SceneEventType;
      using ConditionDirDimension = Base::Enums::ConditionDirDimension;
      using txSize = Base::txSize;

      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_relative_speed);
      }
      virtual ConditionDirDimension direction() const TX_NOEXCEPT { return mDirection; }
      virtual ConditionEquationOp op() const TX_NOEXCEPT { return mEquOp; }
      virtual txFloat speed_threshold() const TX_NOEXCEPT { return m_speed_threshold; }
      virtual txFloat delay_threshold() const TX_NOEXCEPT { return m_delay_threshold; }
      virtual ConditionSpeedType speed_type() const TX_NOEXCEPT { return mSpeedType; }
      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }
      virtual ElementType target_type() const TX_NOEXCEPT { return m_target_element_type; }
      virtual Base::txSysId target_id() const TX_NOEXCEPT { return m_target_element_id; }
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

     protected:
      const ConditionSpeedType mSpeedType = _plus_(ConditionSpeedType::relative);
      ConditionEquationOp mEquOp = _plus_(ConditionEquationOp::eq);
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txFloat m_speed_threshold = 0.0;
      ConditionDirDimension mDirection;
      ElementType m_target_element_type = _plus_(ElementType::Unknown);
      Base::txSysId m_target_element_id = 0;
      txSize m_trigger_count = 1;
      txFloat m_delay_threshold = 0.0;
    };
    using IDummyRelativeSpeedTriggerConditionViewerPtr = std::shared_ptr<IDummyRelativeSpeedTriggerConditionViewer>;
#endif /*__TX_Mark__("ConditionRelativeSpeed")*/

#if __TX_Mark__("ConditionReachPosition")
    struct IDummyReachAbsPositionConditionViewer : public IConditionViewer {
      using ConditionPositionType = Base::Enums::ConditionPositionType;
      using txSize = Base::txSize;
      using txSysId = Base::txSysId;

      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_reach_position);
      }
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;

      virtual hadmap::txPoint point_gps() const TX_NOEXCEPT { return m_position; }
      virtual txFloat radius() const TX_NOEXCEPT { return m_radius; }

      virtual ConditionBoundaryType boundaryType() const TX_NOEXCEPT { return mBoundaryType; }
      virtual txSize trigger_count() const TX_NOEXCEPT { return m_trigger_count; }
      virtual txFloat delay_threshold() const TX_NOEXCEPT { return m_delay_threshold; }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

     protected:
      hadmap::txPoint m_position;
      txFloat m_radius = 0.0;
      ConditionBoundaryType mBoundaryType = _plus_(ConditionBoundaryType::none);
      txSize m_trigger_count = 1;
      txFloat m_delay_threshold = 0.0;
    };
    using IDummyReachAbsPositionConditionViewerPtr = std::shared_ptr<IDummyReachAbsPositionConditionViewer>;
#endif /*__TX_Mark__("ConditionReachPosition")*/

#if __TX_Mark__("ConditionTimeToCollision")
    struct IDummyTTCTriggerViewer : public ITTCTriggerViewer {
      using ParentClass = ITTCTriggerViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_ttc_trigger);
      }
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;
      virtual txFloat delay_threshold() const TX_NOEXCEPT { return m_delay_threshold; }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

     protected:
      txFloat m_delay_threshold = 0.0;
    };
    using IDummyTTCTriggerViewerPtr = std::shared_ptr<IDummyTTCTriggerViewer>;
#endif /*__TX_Mark__("ConditionTimeToCollision")*/

#if __TX_Mark__("ConditionRelativeDistance")
    struct IDummyDistanceTriggerViewer : public IDistanceTriggerViewer {
      using ParentClass = IDistanceTriggerViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_distance_trigger);
      }
      virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txBool initialize(const kvMap& ref_kv_map) TX_NOEXCEPT TX_OVERRIDE;
      virtual txFloat delay_threshold() const TX_NOEXCEPT { return m_delay_threshold; }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;

     protected:
      txFloat m_delay_threshold = 0.0;
    };
    using IDummyDistanceTriggerViewerPtr = std::shared_ptr<IDummyDistanceTriggerViewer>;
#endif /*__TX_Mark__("ConditionRelativeDistance")*/

#if __TX_Mark__("ConditionEgoAttachLaneidCustom")
    struct IDummyEgoAttachLaneUidCustomViewer : public IEgoAttachLaneUidCustomViewer {
      using ParentClass = IEgoAttachLaneUidCustomViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_ego_attach_laneid_custom);
      }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;
    };
    using IDummyEgoAttachLaneUidCustomViewerPtr = std::shared_ptr<IDummyEgoAttachLaneUidCustomViewer>;
#endif /*__TX_Mark__("ConditionEgoAttachLaneidCustom")*/

#if __TX_Mark__("ConditionSimulationTime")
    struct IDummyTimeTriggerViewer : public ITimeTriggerViewer {
      using ParentClass = ITimeTriggerViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_time_trigger);
      }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;
    };
    using IDummyTimeTriggerViewerPtr = std::shared_ptr<IDummyTimeTriggerViewer>;
#endif /*__TX_Mark__("ConditionSimulationTime")*/

#if __TX_Mark__("ConditionTimeHeadWay")
    struct IDummyTimeHeadwayTriggerViewer : public ITimeHeadwayTriggerViewer {
      using ParentClass = ITimeHeadwayTriggerViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_time_head_way);
      }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr, Base::ITrafficElementPtr curElemPtr,
                                  Base::IElementManagerPtr _elemMgrPtr) TX_NOEXCEPT;
    };
    using IDummyTimeHeadwayTriggerViewerPtr = std::shared_ptr<IDummyTimeHeadwayTriggerViewer>;
#endif /*__TX_Mark__("ConditionTimeHeadWay")*/

#if __TX_Mark__("ConditionElementState")
    struct IDummyElementStateTriggerViewer : public IElementStateTriggerViewer {
      using ParentClass = IElementStateTriggerViewer;
      virtual SceneEventType conditionType() const TX_NOEXCEPT TX_OVERRIDE {
        return _plus_(SceneEventType::dummy_element_state);
      }
      virtual txBool TriggerEvent(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
    };
    using IDummyElementStateTriggerViewerPtr = std::shared_ptr<IDummyElementStateTriggerViewer>;
#endif /*__TX_Mark__("ConditionElementState")*/

   public:
    virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return ""; }
    virtual txBool initialize(const Base::txSysId _evId, const kvMapVec& _condition_vec,
                              const std::vector<action_ego> _action_vec) TX_NOEXCEPT;
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE {
      return _NonEmpty_(mCondPtrVec) && NonNull_Pointer(mCondPtrVec.front()) && _NonEmpty_(m_action_vec);
    }
    virtual std::vector<action_ego> actions() const TX_NOEXCEPT { return m_action_vec; }

   protected:
    std::vector<action_ego> m_action_vec;
    /*IConditionViewerPtrVec mCondPtrVec;*/
  };

  using IDummySceneEventViewerPtr = std::shared_ptr<IDummySceneEventViewer>;
  using IDummySceneEventViewerPtrVec = std::vector<IDummySceneEventViewerPtr>;

 public:
  struct DummyDriverViewer : public IViewer {
   public:
    virtual hadmap::txPoint startGPS() const TX_NOEXCEPT;
    virtual hadmap::txPoint endGPS() const TX_NOEXCEPT;
    virtual std::vector<hadmap::txPoint> midGPS() const TX_NOEXCEPT;
    // virtual sim_msg::Location startLocation() const TX_NOEXCEPT { return m_location; }

    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }
    void Init(const std::vector<hadmap::txPoint>& vec_ego_path) TX_NOEXCEPT;
    virtual txBool Initialize(const sim_msg::Ego& _ego) TX_NOEXCEPT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return m_dummyId; }
    virtual txString Name() const TX_NOEXCEPT { return m_name; }
    virtual txString Type() const TX_NOEXCEPT { return m_type; }
    virtual txString Group() const TX_NOEXCEPT { return m_group_name; }
    virtual DynamicEgo dynamic() const TX_NOEXCEPT { return m_dynamic; }
    virtual IDummySceneEventViewerPtrVec DummySceneEventViewerPtrVec() const TX_NOEXCEPT {
      return m_scene_event_viewer_ptr_vec;
    }
    virtual txString Str() const TX_NOEXCEPT;
    virtual physicle_ego phyEgoLeader() const TX_NOEXCEPT { return m_physicles.front(); }
    virtual Base::txSize phyEgoSize() const TX_NOEXCEPT { return m_physicles.size(); }
    virtual physicle_ego phyEgo(const Base::txSize idx) const TX_NOEXCEPT { return m_physicles.at(idx); }
    virtual initial_ego initEgo() const TX_NOEXCEPT { return m_initial; }
    virtual sensor_group sensorGroup() const TX_NOEXCEPT { return m_sensor_group; }

   protected:
    Base::txBool m_inited = false;
    // std::vector< hadmap::txPoint > m_vec_ego_path;
    Base::txSysId m_dummyId;
    txString m_name;
    txString m_type;
    txString m_group_name;
    Base::Enums::VEHICLE_BEHAVIOR m_behavior;
    // sim_msg::Location m_location;
    std::vector<physicle_ego> m_physicles;
    initial_ego m_initial;
    DynamicEgo m_dynamic;
    sensor_group m_sensor_group;
    IDummySceneEventViewerPtrVec m_scene_event_viewer_ptr_vec;
  };
  using DummyDriverViewerPtr = std::shared_ptr<DummyDriverViewer>;
  using DummyDriverViewerPtrVec = std::vector<DummyDriverViewerPtr>;

 public:
  PlanningSceneEvent_SceneLoader() TX_DEFAULT;
  virtual ~PlanningSceneEvent_SceneLoader() TX_DEFAULT;
  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eTAD_DummyDriver; }
  virtual Base::txBool LoadSceneEvent(sim_msg::Scene& ref_scene_event) TX_NOEXCEPT;
  virtual Base::txBool ParseSceneEvent() TX_NOEXCEPT TX_OVERRIDE;
  virtual void ClearSceneEvent() TX_NOEXCEPT {
    m_driver_scene_event.Clear();
    _seceneEventVec.clear();
    m_dummy_viewer_vec.clear();
  }
  virtual Base::txBool HasEgoId(const Base::txSysId egoId) const TX_NOEXCEPT;
  virtual sim_msg::Scene GetScenePb() const TX_NOEXCEPT { return m_driver_scene_event; }
  virtual Setting GetSetting() const TX_NOEXCEPT {
    Setting _s;
    _s.Initialize(m_driver_scene_event.setting());
    return _s;
  }
  virtual DummyDriverViewerPtrVec GetDummyDriverViewerVec() const TX_NOEXCEPT { return m_dummy_viewer_vec; }
  virtual DummyDriverViewerPtr GetDummyDriverViewer(const Base::txSysId _id) const TX_NOEXCEPT;
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE;
  Base::txString GetSimSimulationTraffic() const TX_NOEXCEPT;
  Base::txString GetSimSimulationMapFile() const TX_NOEXCEPT;
  Base::txBool GetSimSimulationPlannerRouteStart(std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT;
  Base::txFloat GetSimSimulationMapfileLongitude() const TX_NOEXCEPT;
  Base::txFloat GetSimSimulationMapfileLatitude() const TX_NOEXCEPT;
  Base::txFloat GetSimSimulationMapfileAltitude() const TX_NOEXCEPT;
  virtual Base::txString GetGroupName() const TX_NOEXCEPT { return m_groupname; }
  virtual void SetGroupName(Base::txString groupName) TX_NOEXCEPT { m_groupname = groupName; }

 public:
  static Base::Enums::VEHICLE_BEHAVIOR Str2Behavior(const Base::txString strEgoType) TX_NOEXCEPT;
  static Base::Enums::ConditionBoundaryType Enum2BoundaryEdge(const sim_msg::ConditionEdge eEdgeType) TX_NOEXCEPT;
  static Base::Enums::DistanceProjectionType Enum2DistanceType(const sim_msg::DistanceType eDistType) TX_NOEXCEPT;
  static Base::Enums::ConditionEquationOp Enum2OpType(const sim_msg::Rule eRule) TX_NOEXCEPT;
  static Base::Enums::ConditionDirDimension Enum2DirDimension(const sim_msg::DirDimension dd) TX_NOEXCEPT;

 public:
  using kvMap = ISceneEventViewer::kvMap;
  using kvMapVec = ISceneEventViewer::kvMapVec;

 protected:
  SceneLoader::Sim::simulation_ptr m_DataSource_Scene = nullptr;
  sim_msg::Scene m_driver_scene_event;
  DummyDriverViewerPtrVec m_dummy_viewer_vec;

  // std::map< Base::txSysId, DummyDriverViewerPtr > m_id2viewer;
};
using PlanningSceneEvent_SceneLoaderPtr = std::shared_ptr<PlanningSceneEvent_SceneLoader>;

TX_NAMESPACE_CLOSE(SceneLoader)
