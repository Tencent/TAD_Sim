// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_vehicle.h"
#include "manager/actor_manager.h"
#include "traffic.pb.h"
#include "union.pb.h"

namespace eval {
CVehicleActor::CVehicleActor() { _type = Actor_Vehicle; }

void CVehicleActorBuilder::Build(const EvalMsg& msg, ActorReposity& actor_repo) {
  if (msg.GetTopic() == topic::TRAFFIC) {
    CVehicleActorBuilder::BuildFromTraffic(msg, actor_repo);
  }
  if (msg.GetTopic() == topic::LOCATION_UNION) {
    CVehicleActorBuilder::BuildFromLocationUnion(msg, actor_repo);
  }
}

void CVehicleActorBuilder::BuildFromTraffic(const EvalMsg& msg, ActorReposity& actor_repo) {
  // get map ptr
  auto map_ptr = CMapManager::GetInstance();
  auto actorMgrPtr = CActorManager::GetInstance();
  const auto& actorMgrParams = actorMgrPtr->getParameters();
  const auto& actorVehiclesTypes = actorMgrParams.sceneDescription.vehicles();

  if (map_ptr && actorMgrPtr && msg.GetPayload().size() > 0) {
    // get sim time
    double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(msg.GetPayload());
    ActorAgentsPtr actors = actor_repo[Actor_Vehicle];

    // traffic cars are between [egos_size, actors->size()) in actors
    size_t actor_index = actorMgrParams.sceneDescription.egos_size();
    for (auto i = 0; i < traffic.cars().size() && actor_index < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CVehicleActor*>(actors->at(actor_index)->GetActorPtr());
      auto fellow = traffic.cars().at(i);

      if (actor_ptr == nullptr) throw "vehicle fellow actor is nullptr.\n";

      // set lane id, default is invalid
      actor_ptr->SetLaneID(fellow.tx_road_id(), fellow.tx_section_id(), fellow.tx_lane_id(), fellow.tx_lanelink_id());

      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto raw_pos_ptr = actor_ptr->MutableRawLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
      actor_ptr->SetType(Actor_Vehicle);
      actor_ptr->SetID(fellow.id());
      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      raw_pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      map_ptr->WGS84ToENU(*pos_ptr);
      actor_ptr->MutableShape()->SetValues(fellow.length(), fellow.width(), fellow.height());

      auto vx = fellow.v() * std::cos(fellow.heading());
      auto vy = fellow.v() * std::sin(fellow.heading());
      actor_ptr->MutableSpeed()->SetValues(vx, vy, 0.0);
      actor_ptr->MutableAcc()->SetValues(fellow.acc(), 0.0, 0.0);

      // update lane id
      actor_ptr->SetLaneID({fellow.tx_road_id(), fellow.tx_section_id(), fellow.tx_lane_id(), fellow.tx_lanelink_id()});

      actors->at(actor_index)->SetState(Actor_Valid);

      // trans vehicle coordinate origin to bounding box center
      auto TransToBBX = [](CLocation* loc, const sim_msg::VehicleGeometory& veh_geometry) {
        const sim_msg::Vec3& bbx = veh_geometry.vehicle_coord().bounding_box_center();
        Eigen::Vector3d bbx_offset = {bbx.x(), bbx.y(), bbx.z()};
        Eigen::Vector3d&& bbx_trans = loc->GetRotMatrix() * bbx_offset;
        CPosition ori = loc->GetPosition();
        loc->MutablePosition()->SetValues(ori.GetX() + bbx_trans[0], ori.GetY() + bbx_trans[1],
                                          ori.GetZ() + bbx_trans[2]);
      };

      auto typeID = fellow.type();
      bool fellowVehicleTypeMatched = false;
      for (const auto& typeOfVehicle : actorVehiclesTypes) {
        if (typeID == typeOfVehicle.physicle().common().model_id()) {
          const auto& geometry = typeOfVehicle.physicle().geometory();
          if (geometry.ByteSizeLong() > 0) {
            TransToBBX(actor_ptr->MutableLocation(), geometry);
            fellowVehicleTypeMatched = true;
          } else {
            LOG_ERROR << "geometry of type " << fellow.type() << " is empty.\n";
          }
          break;
        }
      }

      if (!fellowVehicleTypeMatched) {
        LOG_ERROR << "unable to find vehicle geometry from scene.proto" << ", fellow id:" << fellow.id()
                  << ", type id:" << fellow.type() << ".\n";
      }

      ++actor_index;
    }
  } else {
    LOG_ERROR << "hadmap ptr is empty.\n";
  }
}

void CVehicleActorBuilder::BuildFromLocationUnion(const EvalMsg& msg, ActorReposity& actor_repo) {
  // WARNING: this function has some todo below. Otherwise, some indicators will be unavailable
  // get map ptr, actor manager ptr
  auto map_ptr = CMapManager::GetInstance();
  auto actorMgrPtr = CActorManager::GetInstance();
  if (nullptr == map_ptr) {
    LOG_ERROR << "hadmap ptr is empty.\n";
    return;
  }
  if (nullptr == actorMgrPtr) {
    LOG_ERROR << "actor manager ptr is empty.\n";
    return;
  }
  if (0 == msg.GetPayload().size()) {
    LOG_ERROR << "payload is empty.\n";
    return;
  }

  const auto& actorMgrParams = actorMgrPtr->getParameters();
  const auto& actorVehiclesTypes = actorMgrParams.sceneDescription.vehicles();
  const auto& actorSceneEgos = actorMgrParams.sceneDescription.egos();

  // get sim time
  double sim_t_ms = msg.GetSimTime().GetMilliseond();

  // build traffic
  sim_msg::Union location_union;
  location_union.ParsePartialFromString(msg.GetPayload());
  ActorAgentsPtr actors = actor_repo[Actor_Vehicle];

  // egos is between [0,egos_size) in actors
  size_t actor_index = 0;
  for (size_t i = 0; i < location_union.messages_size() && actor_index < actorSceneEgos.size(); ++i) {
    // ignore own location
    if (getMyEgoGroupName() == location_union.messages().at(i).groupname()) continue;
    // parse and check if is valid location
    sim_msg::Location fellow;
    const std::string& loc_content = location_union.messages().at(i).content();
    bool is_parse_ok = fellow.ParseFromString(loc_content);
    if (!is_parse_ok) {
      VLOG_0 << "Failed to parse location_union[" << i << "] message.\n";
      continue;
    }

    auto actor_ptr = dynamic_cast<CVehicleActor*>(actors->at(actor_index)->GetActorPtr());
    // todo: because lane id is deprecated and invalid
    actor_ptr->SetLaneID(fellow.ego_lane().roadpkid(), fellow.ego_lane().sectionpkid(), fellow.ego_lane().lanepkid());

    actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
    actor_ptr->SetType(Actor_Vehicle);
    // if groupname is not like "Ego_001", use -6 as default.
    actor_ptr->SetID(-6);
    try {
      std::string its_groupname = location_union.messages().at(i).groupname();
      if (its_groupname.size() < 3) {
        VLOG_0 << "Group name of ego[" << i << "] is too short, use -6 as default.\n";
        actor_ptr->SetID(-6);
      } else {
        actor_ptr->SetID(std::stoll(its_groupname.substr(its_groupname.size() - 3)));
      }
    } catch (const std::exception& e) {
      std::cerr << e.what() << '\n';
      actor_ptr->SetID(-6);
    }

    auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
    auto raw_pos_ptr = actor_ptr->MutableRawLocation()->MutablePosition();
    auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();
    pos_ptr->SetValues(fellow.position().x(), fellow.position().y(), fellow.position().z(), CoordType::Coord_WGS84);
    raw_pos_ptr->SetValues(fellow.position().x(), fellow.position().y(), fellow.position().z(), CoordType::Coord_WGS84);
    rpy_ptr->SetValues(fellow.rpy().x(), fellow.rpy().y(), fellow.rpy().z());
    map_ptr->WGS84ToENU(*pos_ptr);
    actor_ptr->MutableSpeed()->SetValues(fellow.velocity().x(), fellow.velocity().y(), fellow.velocity().z());
    actor_ptr->MutableAcc()->SetValues(fellow.acceleration().x(), fellow.acceleration().y(), fellow.acceleration().z());

    // trans ego coordinate origin to bounding box center
    auto TransToBBX = [](CLocation* loc, const sim_msg::VehicleGeometory& veh_geometry) {
      const sim_msg::Vec3& bbx = veh_geometry.vehicle_coord().bounding_box_center();
      Eigen::Vector3d bbx_offset = {bbx.x(), bbx.y(), bbx.z()};
      Eigen::Vector3d&& bbx_trans = loc->GetRotMatrix() * bbx_offset;
      CPosition ori = loc->GetPosition();
      loc->MutablePosition()->SetValues(ori.GetX() + bbx_trans[0], ori.GetY() + bbx_trans[1],
                                        ori.GetZ() + bbx_trans[2]);
    };

    size_t ego_idx = -1;
    for (size_t j = 0; j < actorSceneEgos.size(); ++j) {
      if (actorSceneEgos[j].group() == location_union.messages().at(i).groupname()) {
        ego_idx = j;
        break;
      }
    }
    if (ego_idx >= 0 && actorSceneEgos[ego_idx].physicles_size() > 0) {
      const sim_msg::VehicleGeometory& ego_geometry = actorSceneEgos[ego_idx].physicles(0).geometory();
      TransToBBX(actor_ptr->MutableLocation(), ego_geometry);
      actor_ptr->MutableShape()->SetValues(ego_geometry.vehicle_geometory().length(),
                                           ego_geometry.vehicle_geometory().width(),
                                           ego_geometry.vehicle_geometory().height());
    } else {
      VLOG_0 << "Failed to get the " << i << " ego info in scene message. use own ego shape, not theirs\n";
      actor_ptr->MutableShape()->SetValues(ego_size::ego.length, ego_size::ego.width, ego_size::ego.height);
      const sim_msg::VehicleGeometoryList& ego_geometry = actorMgrParams.egoGeometry;
      if (ego_geometry.has_front()) {
        const sim_msg::VehicleGeometory& front_geometry = ego_geometry.front();
        TransToBBX(actor_ptr->MutableLocation(), front_geometry);
      }
    }

    actors->at(actor_index)->SetState(Actor_Valid);
    ++actor_index;
  }
}
}  // namespace eval
