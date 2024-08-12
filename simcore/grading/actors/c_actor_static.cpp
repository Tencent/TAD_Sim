// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_static.h"
#include "manager/map_manager.h"
#include "traffic.pb.h"

namespace eval {
void CStaticActor::CopyFrom(const CStaticActor &actor_in) {
  _id = actor_in.GetID();
  _uuid = actor_in.GetUUID();
  _shape = actor_in.GetShape();
  _simtime = actor_in.GetSimTime();
  _accel = actor_in.GetAcc();
  _angular_v = actor_in.GetAngularV();
  _speed = actor_in.GetSpeed();
  _loc = actor_in.GetLocation();
  _type = actor_in.GetType();
}

void CStaticActor::SetLaneID(uint64_t road_id, uint64_t section_id, int64_t lane_id, uint64_t lanelink_id) {
  _lane_id.tx_road_id = road_id;
  _lane_id.tx_section_id = section_id;
  _lane_id.tx_lane_id = lane_id;
  _lane_id.tx_lanelink_id = lanelink_id;
}

void CStaticActor::SetLaneID(const LaneID &laneID) { _lane_id = laneID; }

void CStaticActor::SetTypeID(int64_t typeID) { _type_id = typeID; }

EVector3d CStaticActor::TransPoint2BaseCoord(const Eigen::Vector3d &B_pos) {
  Eigen::Matrix3d &&R_GB = _loc.GetEuler().GetRotMatrix();
  Eigen::Vector3d &&G_pos = _loc.GetPosition().GetPoint();
  Eigen::Vector3d GB_pos = G_pos + R_GB * B_pos;
  return EVector3d(GB_pos[0], GB_pos[1], GB_pos[2]);
}

RectCorners CStaticActor::TransCorners2BaseCoord() {
  Eigen::Matrix3d &&R_GB = _loc.GetEuler().GetRotMatrix();
  Eigen::Vector3d &&G_pos = _loc.GetPosition().GetPoint();

  RectCorners corners_enu;
  RectCorners &&corners = _shape.GetRectCorners();

  for (auto i = 0; i < corners.size(); ++i) {
    corners_enu.push_back(G_pos + R_GB * corners.at(i));
  }
  return corners_enu;
}

EVector3d CStaticActor::TransMiddleFront2BaseCoord() { return TransPoint2BaseCoord(_shape.GetMidFront()); }
EVector3d CStaticActor::TransMiddleRear2BaseCoord() { return TransPoint2BaseCoord(_shape.GetMidRear()); }

void CStaticActorBuilder::Build(const EvalMsg &msg, ActorReposity &actor_repo) {
  if (msg.GetTopic() == topic::TRAFFIC) {
    CStaticActorBuilder::BuildFrom(msg, actor_repo);
  }
}

int CStaticActorBuilder::BuildFrom(const EvalMsg &msg, ActorReposity &actor_repo) {
  // get map ptr
  auto map_ptr = CMapManager::GetInstance();

  if (map_ptr && msg.GetPayload().size() > 0) {
    // get sim time
    double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(msg.GetPayload());
    ActorAgentsPtr actors = actor_repo[Actor_Static];

    for (auto i = 0; i < traffic.staticobstacles().size() && i < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CStaticActor *>(actors->at(i)->GetActorPtr());
      auto fellow = traffic.staticobstacles().at(i);

      if (actor_ptr == nullptr) throw "static fellow actor is nullptr.\n";

      // set lane id, default is invalid
      actor_ptr->SetLaneID();

      actor_ptr->SetType(Actor_Static);
      actor_ptr->SetID(fellow.id());
      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);

      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      map_ptr->WGS84ToENU(*pos_ptr);
      actor_ptr->MutableShape()->SetValues(fellow.length(), fellow.width(), fellow.height());

      actors->at(i)->SetState(Actor_Valid);
    }

    return traffic.staticobstacles().size();
  } else {
    LOG_ERROR << "hadmap ptr is empty.\n";
  }

  return 0;
}
}  // namespace eval
