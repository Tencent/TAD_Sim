// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"
#include "utils/eval_math.h"
#include "utils/eval_unit.h"
#include "utils/eval_utils.h"

#include "actors/c_actor_base.h"
#include "actors/c_actor_dynamic.h"
#include "actors/c_actor_dynamic_flu.h"
#include "actors/c_actor_ego.h"
#include "actors/c_actor_environment.h"
#include "actors/c_actor_static.h"
#include "actors/c_actor_traffic_light.h"
#include "actors/c_actor_vehicle.h"

#include "scene.pb.h"

namespace eval {
struct ActorManagerParam {
  sim_msg::VehicleGeometoryList egoGeometry;
  sim_msg::Scene sceneDescription;

  ActorManagerParam() {
    egoGeometry.Clear();
    sceneDescription.Clear();
  }
};

/**
 * @brief "交通参与者管理者", manage all actors‘s reposity and builder. It can't be copied.
 */
class ActorManager : public NoneCopyable {
 private:
  ActorManager();

  void FilterActorsByTime(double cur_t_ms, double t_ms_thresh = 1000.0);
  void FilterActorsByDist(double dist_thresh = const_max_dist);

  // init actor reposity buffer
  template <typename ActorTy>
  void InitActorReposity(ActorReposity& actor_repo, const ActorType& actor_type, const size_t& capacity) {
    ActorAgentsPtr actors = std::make_shared<ActorAgents>(capacity);
    actor_repo[actor_type] = actors;
    for (size_t i = 0; i < actors->capacity(); ++i) {
      actors->push_back(std::make_shared<ActorAgent>(new ActorTy(), Actor_Invalid));
    }
  }

  // build actors imp
  void BuildActorsImp(double cur_t_ms, const EvalMsg& msg);

 public:
  friend CActorManager;
  virtual ~ActorManager();

  void ResetActorState();
  void Init(const ActorManagerParam& params);

  /**
   * @brief call all builers to build actors by message and current time
   */
  void BuildActors(double cur_t_ms, const EvalMsg& msg);

  const ActorManagerParam& getParameters();

 public:
  /**
   * @brief get actorlist by actor type
   */
  template <class ActorPtrTy>
  ActorList<ActorPtrTy> GetFellowActorsByType(ActorType actor_type) {
    ActorList<ActorPtrTy> actor_depot;
    actor_depot.reserve(const_max_actor_size);

    auto iter = m_actors_reposity.find(actor_type);

    if (iter != m_actors_reposity.end()) {
      ActorAgentsPtr actors = m_actors_reposity[actor_type];
      for (auto i = 0; i < actors->size(); ++i) {
        ActorAgentPtr actor = actors->at(i);
        // continue if actor not invalid
        if (actor->GetActorState() != Actor_Valid) {
          continue;
        }
        // place actor pointer into return list
        ActorPtrTy actor_ptr = dynamic_cast<ActorPtrTy>(actor->GetActorPtr());
        if (actor_ptr != nullptr) actor_depot.emplace_back(actor_ptr);
      }
    }

    return actor_depot;
  }

  /**
   * @brief get first ego actor, aka main ego actor
   */
  CEgoActorPtr GetEgoFrontActorPtr();

  /**
   * @brief get first ego trailer actor, aka main ego trailer actor (if any)
   */
  CEgoActorPtr GetEgoTrailerActorPtr();

  CEnvironmentActorPtr GetEnvironmentActorPtr();

 private:
  // actor reposity
  ActorReposity m_actors_reposity;

  // actor builders
  std::vector<CActorBuilderPtr> m_actor_builders;

  // actor manager parameters
  ActorManagerParam m_parameters;
};
}  // namespace eval
