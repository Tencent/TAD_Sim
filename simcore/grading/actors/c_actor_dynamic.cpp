// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_dynamic.h"
#include "traffic.pb.h"

namespace eval {
void CDynamicActorBuilder::Build(const EvalMsg& msg, ActorReposity& actor_repo) {
  if (msg.GetTopic() == topic::TRAFFIC) {
    CDynamicActorBuilder::BuildFrom(msg, actor_repo);
  }
}
void CDynamicActorBuilder::BuildFrom(const EvalMsg& msg, ActorReposity& actor_repo) {
  // get map ptr
  auto map_ptr = CMapManager::GetInstance();

  if (map_ptr && msg.GetPayload().size() > 0) {
    // get sim time
    double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(msg.GetPayload());
    ActorAgentsPtr actors = actor_repo[Actor_Dynamic];
    size_t actorIndex = 0;

    for (auto i = 0; i < traffic.dynamicobstacles().size() && actorIndex < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CDynamicActorPtr>(actors->at(actorIndex)->GetActorPtr());
      const sim_msg::DynamicObstacle& fellow = traffic.dynamicobstacles().at(i);

      if (actor_ptr == nullptr) throw "dynamic fellow actor is nullptr.\n";

      // set lane id, default is invalid
      actor_ptr->SetLaneID();

      if (601 <= fellow.type() && fellow.type() <= 650) {
        continue;
      }

      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto raw_pos_ptr = actor_ptr->MutableRawLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
      actor_ptr->SetType(Actor_Dynamic);
      actor_ptr->SetID(fellow.id());
      actor_ptr->SetTypeID(fellow.type());
      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      raw_pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      map_ptr->WGS84ToENU(*pos_ptr);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      actor_ptr->MutableShape()->SetValues(fellow.length(), fellow.width(), fellow.height());

      auto vx = fellow.v() * std::cos(fellow.heading());
      auto vy = fellow.v() * std::sin(fellow.heading());
      actor_ptr->MutableSpeed()->SetValues(vx, vy, 0.0);

      actors->at(actorIndex)->SetState(Actor_Valid);

      actorIndex++;
    }
  } else {
    LOG_ERROR << "hadmap ptr is empty.\n";
  }
}
}  // namespace eval
