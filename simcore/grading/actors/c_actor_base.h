// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"
#include "utils/eval_math.h"
#include "utils/eval_shape.h"
#include "utils/eval_unit.h"

#include "manager/map_manager.h"

namespace eval {
// actor reposity
class ActorAgent;
class ActorAgents;
using ActorAgentPtr = std::shared_ptr<ActorAgent>;
using ActorAgentsPtr = std::shared_ptr<ActorAgents>;
using ActorReposity =
    std::unordered_map<eval::ActorType, ActorAgentsPtr, std::hash<uint32_t>, std::equal_to<ActorType>>;

// specific actor list
template <typename Ty>
using ActorList = std::vector<Ty>;
using StaticActorList = ActorList<CStaticActorPtr>;
using DynamicActorList = ActorList<CDynamicActorPtr>;
using DynamicActorFLUList = ActorList<CDynamicActorFLUPtr>;
using VehilceActorList = ActorList<CVehicleActorPtr>;
using SignalLightActorList = ActorList<CTrafficLightActorPtr>;

// actor builder
class CActorBuilder;
using CActorBuilderPtr = std::shared_ptr<CActorBuilder>;

/**
 * @brief "交通参与者基类", actor base type.
 */
class CActorBase {
 public:
  CShape _shape;

  CActorBase();
  virtual ~CActorBase() {}

  inline const std::string &GetUUID() const { return _uuid; }
  inline const ActorType &GetType() const { return _type; }

  inline const CSimTime &GetSimTime() const { return _simtime; }
  inline const CLocation &GetLocation() const { return _loc; }

  inline const CSpeed &GetSpeed() const { return _speed; }
  inline const CAcceleration &GetAcc() const { return _accel; }
  inline const CJerk &GetJerk() const { return _jerk; }
  inline const CAngularVelocity &GetAngularV() const { return _angular_v; }

  // for circle is radius, for point is 0, for rect is triangle line
  virtual double GetBaseLength() const { return 0.0; }

  // get shape of actor
  virtual inline const CShape &GetShape() const { return _shape; }

  inline const CLocation &GetRawLocation() const { return _raw_location; }

 public:
  void SetUUID(const std::string &uuid) { _uuid = uuid; }
  void SetSimTime(const CSimTime &t) { _simtime.FromSecond(t.GetSecond()); }
  void SetType(ActorType type) { _type = type; }

  inline CSimTime *MutableSimTime() { return &_simtime; }
  inline CLocation *MutableLocation() { return &_loc; }
  inline CSpeed *MutableSpeed() { return &_speed; }
  inline CAcceleration *MutableAcc() { return &_accel; }
  inline CJerk *MutableJerk() { return &_jerk; }
  inline CAngularVelocity *MutableAngularV() { return &_angular_v; }

  virtual inline CShape *MutableShape() { return &_shape; }

  inline CLocation *MutableRawLocation() { return &_raw_location; }

 public:
  /*
          return actor of b expressed in x, convert location, orientation and velocity for now
  */
  static CActorBase CalXB(const CActorBase &GX, const CActorBase &GB);

 protected:
  const std::string _name = "CActorBase";
  std::string _uuid;
  ActorType _type;
  CLocation _loc;
  CSimTime _simtime;
  CSpeed _speed;
  CAcceleration _accel;
  CJerk _jerk;
  CAngularVelocity _angular_v;
  MapManagerPtr _map_mgr;
  CLocation _raw_location;
};

/**
 * @brief "交通参与者基类", actor builder, which build the actors from the map and the msg.
 */
class CActorBuilder {
 public:
  virtual void Build(const EvalMsg &msg, ActorReposity &actor_repo) = 0;
};

/**
 * @brief "交通参与者代理", actor agent, used to manage actor‘s state.
 */
class ActorAgent {
 private:
  std::shared_ptr<CActorBase> _ptr;
  ActorState _state = Actor_Invalid;

 public:
  ActorAgent(ActorBasePtr ptr, ActorState state) : _state(state) { _ptr.reset(ptr); }
  ActorAgent() : ActorAgent(nullptr, Actor_Invalid) {}
  virtual ~ActorAgent() {}

  ActorAgent(const ActorAgent &) = delete;
  ActorAgent &operator=(const ActorAgent &) = delete;

  ActorAgent(ActorAgent &&rval) {
    _ptr = rval.GetActorSharedPtr();
    _state = rval.GetActorState();
  }
  ActorAgent &operator=(ActorAgent &&rval) {
    _ptr = rval.GetActorSharedPtr();
    _state = rval.GetActorState();
    return *this;
  }

  inline void SetState(ActorState state) { _state = state; }
  inline void ResetState() { _state = Actor_Invalid; }
  inline void SetActorAgent(ActorBasePtr ptr, ActorState state) {
    if (_ptr) _ptr.reset(ptr);
    _state = state;
  }
  inline void Release() { _ptr.reset(); }

  inline ActorState GetActorState() const { return _state; }
  inline ActorBasePtr GetActorPtr() const { return _ptr.get(); }
  inline std::shared_ptr<CActorBase> GetActorSharedPtr() const { return _ptr; }
};

/**
 * @brief "交通参与者代理商", actor agents, which is a vector of ActorAgentPtr.
 */
class ActorAgents : public std::vector<ActorAgentPtr> {
 public:
  explicit ActorAgents(const size_t &max_capacity = 1) { reserve(max_capacity); }
  virtual ~ActorAgents() {}

 protected:
};
}  // namespace eval
