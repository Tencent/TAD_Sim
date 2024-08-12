// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_static.h"

namespace eval {
/**
 * @brief "动态障碍物-主车坐标系", dynamic actor FLU type, which is from static actor.
 */
class CDynamicActorFLU : public CStaticActor {
 public:
  const std::string _name = "CDynamicActorFLU";
  CDynamicActorFLU() { _type = Actor_Dynamic_FLU; }
  virtual ~CDynamicActorFLU() {}
};

/**
 * @brief "动态障碍物-主车坐标系建造者", dynamic actor FLU builder, which build the actors from the map and the msg.
 */
class CDynamicActorFLUBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg& msg, ActorReposity& actor_repo) override;

 protected:
  static void BuildFromTraffic(const EvalMsg& msg, ActorReposity& actor_repo);
  static void BuildFromLocationUnion(const EvalMsg& msg, ActorReposity& actor_repo);
};
}  // namespace eval
