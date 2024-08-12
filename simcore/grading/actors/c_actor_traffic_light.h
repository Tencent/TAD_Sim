// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_static.h"

namespace eval {
typedef std::vector<LaneID> ControlLanes;
typedef std::vector<SignalLightPhase> ControlPhase;

/**
 * @brief "交通信号灯", traffic light actor type, which is a static actor.
 */
class CTrafficLightActor final : public CStaticActor {
 private:
  SignalLightColor _color;
  ControlLanes _control_lane;
  ControlPhase _control_phase;

 public:
  inline const SignalLightColor& GetSignalLightColor() const { return _color; }
  inline const ControlLanes& GetControlLanes() const { return _control_lane; }
  inline const ControlPhase& GetControlPhase() const { return _control_phase; }

 public:
  void Reset() {
    _color = GREEN;
    _control_lane.clear();
    _control_phase.clear();
  }
  void SetSignalLightColor(SignalLightColor color);
  void AddControlLane(const LaneID& lane_id);
  void AddControlPhase(SignalLightPhase phase);
};

/**
 * @brief "交通信号灯建造者", traffic light builder, which build the actors from the map and the msg.
 */
class CTrafficLightActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg& msg, ActorReposity& actor_repo) override;

 protected:
  static void BuildFrom(const EvalMsg& msg, ActorReposity& actor_repo);
};
}  // namespace eval
