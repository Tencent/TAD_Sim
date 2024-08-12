// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_static.h"

namespace eval {
/**
 * @brief "动态障碍物", dynamic actor type, which is from static actor.
 */
class CDynamicActor : public CStaticActor {
 public:
  const std::string _name = "CDynamicActor";
  CDynamicActor() { _type = Actor_Dynamic; }
  virtual ~CDynamicActor() {}
};

/**
 * @brief "动态障碍物建造者", dynamic actor builder, which build the actors from the map and the msg.
 */
class CDynamicActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg& msg, ActorReposity& actor_repo) override;

 protected:
  static void BuildFrom(const EvalMsg& msg, ActorReposity& actor_repo);
};
}  // namespace eval
