// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "actors/c_actor_dynamic_flu.h"
#include "actors/c_actor_ego.h"
#include "manager/actor_manager.h"
#include "traffic.pb.h"
#include "union.pb.h"
#include "vehicle_geometry.pb.h"

namespace eval {
void CDynamicActorFLUBuilder::Build(const EvalMsg &msg, ActorReposity &actor_repo) {
  if (msg.GetTopic() == topic::TRAFFIC) {
    CDynamicActorFLUBuilder::BuildFromTraffic(msg, actor_repo);
  }
  if (msg.GetTopic() == topic::LOCATION_UNION) {
    CDynamicActorFLUBuilder::BuildFromLocationUnion(msg, actor_repo);
  }
}

void CDynamicActorFLUBuilder::BuildFromTraffic(const EvalMsg &msg, ActorReposity &actor_repo) {
  // get map ptr
  auto map_ptr = CMapManager::GetInstance();

  // get ego front
  ActorAgentPtr ego_agent = actor_repo[Actor_Ego_Front]->at(0);
  CEgoActorPtr ego_ptr = nullptr;
  if (ego_agent->GetActorState() == Actor_Valid) {
    ego_ptr = dynamic_cast<CEgoActorPtr>(actor_repo[Actor_Ego_Front]->at(0)->GetActorPtr());
  }
  if (ego_ptr == nullptr) {
    VLOG_1 << "ego ptr is nullptr, quit build CActorDynamicFLU.\n";
    return;
  }

  if (map_ptr && msg.GetPayload().size() > 0) {
    // get sim time
    double sim_t_ms = msg.GetSimTime().GetMilliseond();

    // build traffic
    sim_msg::Traffic traffic;
    traffic.ParseFromString(msg.GetPayload());

    ActorAgentsPtr actors = actor_repo[Actor_Dynamic_FLU];

    auto actorMgrPtr = CActorManager::GetInstance();
    const auto &actorMgrParams = actorMgrPtr->getParameters();
    const auto &actorVehiclesTypes = actorMgrParams.sceneDescription.vehicles();

    // dynamicobstacles and traffic cars are between [egos_size, actors->size()) in actors
    size_t actor_index = actorMgrParams.sceneDescription.egos_size();
    for (auto i = 0ul; i < traffic.dynamicobstacles().size() && actor_index < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CDynamicActorFLUPtr>(actors->at(actor_index)->GetActorPtr());
      const sim_msg::DynamicObstacle &fellow = traffic.dynamicobstacles().at(i);

      if (actor_ptr == nullptr) throw "dynamic fellow actor is nullptr.\n";

      // set lane id, default is invalid
      actor_ptr->SetLaneID();

      if (601 <= fellow.type() && fellow.type() <= 650) {
        continue;
      }

      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
      actor_ptr->SetType(Actor_Dynamic_FLU);
      actor_ptr->SetID(fellow.id());
      actor_ptr->SetTypeID(fellow.type());
      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      map_ptr->WGS84ToENU(*pos_ptr);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      actor_ptr->MutableShape()->SetValues(fellow.length(), fellow.width(), fellow.height());

      auto vx = fellow.v() * std::cos(fellow.heading());
      auto vy = fellow.v() * std::sin(fellow.heading());
      actor_ptr->MutableSpeed()->SetValues(vx, vy, 0.0);

      actors->at(actor_index)->SetState(Actor_Valid);

      CActorBase &&flu_actor = CActorBase::CalXB(*ego_ptr, *actor_ptr);
      *(actor_ptr->MutableLocation()) = flu_actor.GetLocation();
      *(actor_ptr->MutableSpeed()) = flu_actor.GetSpeed();

      actor_index++;
    }

    // dynamicobstacles and traffic cars are between [egos_size, actors->size()) in actors
    for (auto i = 0; i < traffic.cars().size() && actor_index < actors->size(); ++i) {
      auto actor_ptr = dynamic_cast<CDynamicActorFLUPtr>(actors->at(actor_index)->GetActorPtr());
      const sim_msg::Car &fellow = traffic.cars().at(i);

      if (actor_ptr == nullptr) throw "dynamic fellow actor is nullptr.\n";

      // set lane id, default is invalid
      actor_ptr->SetLaneID(fellow.tx_road_id(), fellow.tx_section_id(), fellow.tx_lane_id(), fellow.tx_lanelink_id());

      auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
      auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();

      actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
      actor_ptr->SetType(Actor_Vehicle_FLU);
      actor_ptr->SetID(fellow.id());
      actor_ptr->SetTypeID(fellow.type());
      pos_ptr->SetValues(fellow.x(), fellow.y(), fellow.z(), CoordType::Coord_WGS84);
      map_ptr->WGS84ToENU(*pos_ptr);
      rpy_ptr->SetValues(0.0, 0.0, fellow.heading());
      actor_ptr->MutableShape()->SetValues(fellow.length(), fellow.width(), fellow.height());

      auto vx = fellow.v() * std::cos(fellow.heading());
      auto vy = fellow.v() * std::sin(fellow.heading());
      actor_ptr->MutableSpeed()->SetValues(vx, vy, 0.0);

      actors->at(actor_index)->SetState(Actor_Valid);

      // trans vehicle coordinate origin to bounding box center
      auto TransToBBX = [](CLocation *loc, const sim_msg::VehicleGeometory &veh_geometry) {
        const sim_msg::Vec3 &bbx = veh_geometry.vehicle_coord().bounding_box_center();
        Eigen::Vector3d bbx_offset = {bbx.x(), bbx.y(), bbx.z()};
        Eigen::Vector3d &&bbx_trans = loc->GetRotMatrix() * bbx_offset;
        CPosition ori = loc->GetPosition();
        loc->MutablePosition()->SetValues(ori.GetX() + bbx_trans[0], ori.GetY() + bbx_trans[1],
                                          ori.GetZ() + bbx_trans[2]);
      };

      // adapted for traffic vehicle geometry
      auto typeID = fellow.type();
      bool fellowVehicleTypeMatched = false;
      for (const auto &typeOfVehicle : actorVehiclesTypes) {
        if (typeID == typeOfVehicle.physicle().common().model_id()) {
          const auto &geometry = typeOfVehicle.physicle().geometory();
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

      // transform to ego coordinate
      CActorBase &&flu_actor = CActorBase::CalXB(*ego_ptr, *actor_ptr);
      *(actor_ptr->MutableLocation()) = flu_actor.GetLocation();
      *(actor_ptr->MutableSpeed()) = flu_actor.GetSpeed();
      actor_index++;
    }
  } else {
    LOG_ERROR << "hadmap ptr is empty.\n";
  }
}

void CDynamicActorFLUBuilder::BuildFromLocationUnion(const EvalMsg &msg, ActorReposity &actor_repo) {
  // WARNING: this function has some todo below. Otherwise, some indicators will be unavailable
  // get map ptr, actor manager ptr
  auto map_ptr = CMapManager::GetInstance();
  auto actorMgrPtr = CActorManager::GetInstance();
  // get ego front
  ActorAgentPtr ego_agent = actor_repo[Actor_Ego_Front]->at(0);
  CEgoActorPtr ego_ptr = nullptr;
  if (ego_agent->GetActorState() == Actor_Valid) {
    ego_ptr = dynamic_cast<CEgoActorPtr>(actor_repo[Actor_Ego_Front]->at(0)->GetActorPtr());
  }

  if (nullptr == ego_ptr) {
    LOG_ERROR << "ego ptr is nullptr, quit build CActorDynamicFLU.\n";
    return;
  }
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

  const auto &actorMgrParams = actorMgrPtr->getParameters();
  const auto &actorVehiclesTypes = actorMgrParams.sceneDescription.vehicles();
  const auto &actorSceneEgos = actorMgrParams.sceneDescription.egos();

  // get sim time
  double sim_t_ms = msg.GetSimTime().GetMilliseond();

  // build traffic
  sim_msg::Union location_union;
  location_union.ParsePartialFromString(msg.GetPayload());
  ActorAgentsPtr actors = actor_repo[Actor_Dynamic_FLU];

  // determine own ego group name with fallback for single-ego case
  std::string my_ego_group_name = getMyEgoGroupName();
  if (my_ego_group_name.empty() && actorSceneEgos.size() == 1) {
    my_ego_group_name = actorSceneEgos[0].group();
  }

  // egos is between [0,egos_size) in actors
  size_t actor_index = 0;
  for (size_t i = 0; i < location_union.messages_size() && actor_index < actorSceneEgos.size(); ++i) {
    // ignore own location
    if (my_ego_group_name == location_union.messages().at(i).groupname()) continue;
    // parse and check if is valid location
    sim_msg::Location fellow;
    const std::string &loc_content = location_union.messages().at(i).content();
    bool is_parse_ok = fellow.ParseFromString(loc_content);
    if (!is_parse_ok) {
      VLOG_0 << "Failed to parse location_union[" << i << "] message.\n";
      continue;
    }

    auto actor_ptr = dynamic_cast<CDynamicActorFLUPtr>(actors->at(actor_index)->GetActorPtr());
    // todo: because lane id is deprecated and invalid
    actor_ptr->SetLaneID(fellow.ego_lane().roadpkid(), fellow.ego_lane().sectionpkid(), fellow.ego_lane().lanepkid());

    actor_ptr->MutableSimTime()->FromMilliseond(sim_t_ms);
    actor_ptr->SetType(Actor_Dynamic_FLU);
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
    } catch (const std::exception &e) {
      std::cerr << e.what() << '\n';
      actor_ptr->SetID(-6);
    }

    auto pos_ptr = actor_ptr->MutableLocation()->MutablePosition();
    auto rpy_ptr = actor_ptr->MutableLocation()->MutableEuler();
    pos_ptr->SetValues(fellow.position().x(), fellow.position().y(), fellow.position().z(), CoordType::Coord_WGS84);
    rpy_ptr->SetValues(fellow.rpy().x(), fellow.rpy().y(), fellow.rpy().z());
    map_ptr->WGS84ToENU(*pos_ptr);
    actor_ptr->MutableSpeed()->SetValues(fellow.velocity().x(), fellow.velocity().y(), fellow.velocity().z());
    actor_ptr->MutableAcc()->SetValues(fellow.acceleration().x(), fellow.acceleration().y(), fellow.acceleration().z());

    // trans ego coordinate origin to bounding box center
    auto TransToBBX = [](CLocation *loc, const sim_msg::VehicleGeometory &veh_geometry) {
      const sim_msg::Vec3 &bbx = veh_geometry.vehicle_coord().bounding_box_center();
      Eigen::Vector3d bbx_offset = {bbx.x(), bbx.y(), bbx.z()};
      Eigen::Vector3d &&bbx_trans = loc->GetRotMatrix() * bbx_offset;
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
      const sim_msg::VehicleGeometory &ego_geometry = actorSceneEgos[ego_idx].physicles(0).geometory();
      TransToBBX(actor_ptr->MutableLocation(), ego_geometry);
      actor_ptr->MutableShape()->SetValues(ego_geometry.vehicle_geometory().length(),
                                           ego_geometry.vehicle_geometory().width(),
                                           ego_geometry.vehicle_geometory().height());
    } else {
      VLOG_0 << "Failed to get the " << i << " ego info in scene message. use own ego shape, not theirs\n";
      actor_ptr->MutableShape()->SetValues(ego_size::ego.length, ego_size::ego.width, ego_size::ego.height);
      const sim_msg::VehicleGeometoryList &ego_geometry = actorMgrParams.egoGeometry;
      if (ego_geometry.has_front()) {
        const sim_msg::VehicleGeometory &front_geometry = ego_geometry.front();
        TransToBBX(actor_ptr->MutableLocation(), front_geometry);
      }
    }

    // transform to ego coordinate
    CActorBase &&flu_actor = CActorBase::CalXB(*ego_ptr, *actor_ptr);
    *(actor_ptr->MutableLocation()) = flu_actor.GetLocation();
    *(actor_ptr->MutableSpeed()) = flu_actor.GetSpeed();

    actors->at(actor_index)->SetState(Actor_Valid);
    ++actor_index;
  }
}
}  // namespace eval
