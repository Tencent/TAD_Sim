// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "actors/c_actor_dynamic.h"

namespace eval {
/**
 * @brief "交通车", vehicle actor type, which is a dynamic actor.
 */
class CVehicleActor : public CDynamicActor {
 public:
  const std::string _name = "CVehicleActor";
  CVehicleActor();
  virtual ~CVehicleActor() {}
};

/**
 * @brief "交通车建造者", vehicle actor builder, which build the actors from the map and the msg.
 */
class CVehicleActorBuilder : public CActorBuilder {
 public:
  virtual void Build(const EvalMsg& msg, ActorReposity& actor_repo) override;

 protected:
  static void BuildFromTraffic(const EvalMsg& msg, ActorReposity& actor_repo);
  static void BuildFromLocationUnion(const EvalMsg& msg, ActorReposity& actor_repo);
};
}  // namespace eval
