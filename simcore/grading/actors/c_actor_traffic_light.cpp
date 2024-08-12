// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_traffic_light.h"
#include "traffic.pb.h"

namespace eval {
void CTrafficLightActor::SetSignalLightColor(SignalLightColor color) { _color = color; }
void CTrafficLightActor::AddControlLane(const LaneID& lane_id) { _control_lane.push_back(lane_id); }
void CTrafficLightActor::AddControlPhase(SignalLightPhase phase) { _control_phase.push_back(phase); }

void CTrafficLightActorBuilder::Build(const EvalMsg& msg, ActorReposity& actor_repo) {
  if (msg.GetTopic() == topic::TRAFFIC) {
    CTrafficLightActorBuilder::BuildFrom(msg, actor_repo);
  }
}

void CTrafficLightActorBuilder::BuildFrom(const EvalMsg& msg, ActorReposity& actor_repo) {
  // get map ptr
  auto map_ptr = CMapManager::GetInstance();

  if (map_ptr && msg.GetPayload().size() > 0) {
    // get sim time
    double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(msg.GetPayload());
    ActorAgentsPtr actors = actor_repo[Actor_SignalLight];

    for (auto i = 0; i < traffic.trafficlights().size() && i < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CTrafficLightActor*>(actors->at(i)->GetActorPtr());
      auto fellow = traffic.trafficlights().at(i);

      if (actor_ptr == nullptr) throw "traffic light actor is nullptr.\n";

      // reset signal light
      actor_ptr->Reset();

      actor_ptr->SetType(Actor_SignalLight);
      actor_ptr->SetID(fellow.id());
      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);

      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      map_ptr->WGS84ToENU(*pos_ptr);

      // set color
      actor_ptr->SetSignalLightColor((SignalLightColor)fellow.color());

      // get control lanes
      LaneID lane_id;
      for (auto k = 0; k < fellow.control_lanes().size(); ++k) {
        auto cl = fellow.control_lanes().at(k);

        lane_id.tx_road_id = cl.tx_road_id();
        lane_id.tx_section_id = cl.tx_section_id();
        lane_id.tx_lane_id = cl.tx_lane_id();
        lane_id.tx_lanelink_id = 0;

        actor_ptr->AddControlLane(lane_id);
      }

      // get control phase
      for (auto k = 0; k < fellow.control_phases().size(); ++k) {
        actor_ptr->AddControlPhase((SignalLightPhase)fellow.control_phases().at(k));
      }

      actors->at(i)->SetState(Actor_Valid);
    }
  } else {
    LOG_ERROR << "hadmap ptr is empty.\n";
  }
}
}  // namespace eval
