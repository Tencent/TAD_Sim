// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "manager/actor_manager.h"

namespace eval {
ActorManager::ActorManager() {
  // init actor reposity buffer
  InitActorReposity<CEgoActor>(m_actors_reposity, Actor_Ego_Front, 1);
  InitActorReposity<CEgoActor>(m_actors_reposity, Actor_Ego_Trailer, 1);
  InitActorReposity<CEnvironmentActor>(m_actors_reposity, Actor_Environment, 1);
  InitActorReposity<CStaticActor>(m_actors_reposity, Actor_Static, const_max_actor_size);
  InitActorReposity<CDynamicActor>(m_actors_reposity, Actor_Dynamic, const_max_actor_size);
  InitActorReposity<CVehicleActor>(m_actors_reposity, Actor_Vehicle, const_max_actor_size);
  InitActorReposity<CTrafficLightActor>(m_actors_reposity, Actor_SignalLight, const_max_actor_size);
  InitActorReposity<CDynamicActorFLU>(m_actors_reposity, Actor_Dynamic_FLU, const_max_actor_size);

  VLOG_2 << "all actors ptr buffer created.\n";

  // create actor builders
  m_actor_builders.reserve(64);
  m_actor_builders.emplace_back(std::make_shared<CStaticActorBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CDynamicActorBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CDynamicActorFLUBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CVehicleActorBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CTrafficLightActorBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CEgoActorBuilder>());
  m_actor_builders.emplace_back(std::make_shared<CEnvironmentActorBuilder>());

  VLOG_2 << "all actor builders ptr buffer created.\n";
}

ActorManager::~ActorManager() {
  // release all actor memory
  for (auto pair = m_actors_reposity.begin(); pair != m_actors_reposity.end(); ++pair) {
    pair->second.reset();
  }

  VLOG_2 << "all actors ptr deleted.\n";
}

void ActorManager::Init(const ActorManagerParam &params) {
  m_parameters = params;
  ResetActorState();
}

const ActorManagerParam &ActorManager::getParameters() { return m_parameters; }

void ActorManager::ResetActorState() {
  for (auto iter = m_actors_reposity.begin(); iter != m_actors_reposity.end(); ++iter) {
    ActorAgentsPtr repo = iter->second;
    for (auto i = 0; i < repo->size(); ++i) {
      repo->at(i)->ResetState();
    }
  }
}

void ActorManager::FilterActorsByTime(double cur_t_ms, double t_ms_thresh) {
  double delta_t_ms = 0.0;

  // only keep new actor
  for (auto pair = m_actors_reposity.begin(); pair != m_actors_reposity.end(); ++pair) {
    ActorAgentsPtr repo = pair->second;
    for (auto i = 0; i < repo->size(); ++i) {
      ActorAgentPtr p_actor = repo->at(i);

      // continue if actor is not valid
      if (p_actor->GetActorState() != ActorState::Actor_Valid) {
        continue;
      }

      // set actor state to invalid if too old
      delta_t_ms = cur_t_ms - p_actor->GetActorPtr()->GetSimTime().GetMilliseond();
      if (delta_t_ms > t_ms_thresh) {
        if (Actor_Ego_Front == pair->first) {  // Actor_Ego_Front no reset, only warning
          LOG_WARNING << "Actor_Ego_Front's last simtime is too old, " << delta_t_ms << " ms earlier.\n";
        } else {
          p_actor->ResetState();
        }
      }
    }
  }
}

void ActorManager::FilterActorsByDist(double dist_thresh) {
  double delta_dist = 0.0;
  auto ego_front = GetEgoFrontActorPtr();

  // only keep new actor
  if (ego_front == nullptr) {
    return;
  }

  const CPosition &ego_pos = ego_front->GetLocation().GetPosition();
  for (auto pair = m_actors_reposity.begin(); pair != m_actors_reposity.end(); ++pair) {
    ActorAgentsPtr repo = pair->second;
    for (auto i = 0; i < repo->size(); ++i) {
      ActorAgentPtr p_actor = repo->at(i);

      // continue if actor is not valid
      if (p_actor->GetActorState() != ActorState::Actor_Valid) {
        continue;
      }

      // set actor state to invalid if too farway
      const CPosition &fellow_pos = p_actor->GetActorPtr()->GetLocation().GetPosition();
      delta_dist = CEvalMath::AbsoluteDistance2D(ego_pos, fellow_pos);
      if (pair->first == Actor_Dynamic_FLU) {
        if (fellow_pos.GetNormal2D() > dist_thresh) {
          p_actor->ResetState();
        }
      } else if (delta_dist > dist_thresh) {
        p_actor->ResetState();
      }
    }
  }
}

// build actors imp
void ActorManager::BuildActorsImp(double cur_t_ms, const EvalMsg &msg) {
  // build actors
  for (auto iter = m_actor_builders.begin(); iter != m_actor_builders.end(); ++iter) {
    const CActorBuilderPtr &builder = *iter;
    builder->Build(msg, m_actors_reposity);
  }
}

void ActorManager::BuildActors(double cur_t_ms, const EvalMsg &msg) {
  // build actors
  BuildActorsImp(cur_t_ms, msg);

  // filter actors by time
  FilterActorsByTime(cur_t_ms);

  // filter actors by distance
  FilterActorsByDist();
}

CEgoActorPtr ActorManager::GetEgoFrontActorPtr() {
  auto p_actor = m_actors_reposity[Actor_Ego_Front]->at(0);
  if (p_actor && p_actor->GetActorState() == ActorState::Actor_Valid) {
    return dynamic_cast<CEgoActorPtr>(p_actor->GetActorPtr());
  }
  return nullptr;
}

CEgoActorPtr ActorManager::GetEgoTrailerActorPtr() {
  auto p_actor = m_actors_reposity[Actor_Ego_Trailer]->at(0);
  if (p_actor && p_actor->GetActorState() == ActorState::Actor_Valid) {
    return dynamic_cast<CEgoActorPtr>(p_actor->GetActorPtr());
  }
  return nullptr;
}

CEnvironmentActorPtr ActorManager::GetEnvironmentActorPtr() {
  auto p_actor = m_actors_reposity[Actor_Environment]->at(0);
  if (p_actor && p_actor->GetActorState() == ActorState::Actor_Valid) {
    return dynamic_cast<CEnvironmentActorPtr>(p_actor->GetActorPtr());
  }
  return nullptr;
}
}  // namespace eval
