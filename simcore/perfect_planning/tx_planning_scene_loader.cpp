// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_planning_scene_loader.h"

TX_NAMESPACE_OPEN(SceneLoader)

Planning_SceneLoader::Planning_SceneLoader() : _class_name(__func__) {}

void Planning_SceneLoader::EgoRouteViewer::Init(const std::vector<hadmap::txPoint>& vec_ego_path) TX_NOEXCEPT {
  m_vec_ego_path = vec_ego_path;
  m_inited = _NonEmpty_(m_vec_ego_path);
}

Base::txFloat Planning_SceneLoader::EgoRouteViewer::startLon() const TX_NOEXCEPT {
  if (_NonEmpty_(m_vec_ego_path)) {
    return __Lon__(m_vec_ego_path.front());
  } else {
    return 0.0;
  }
}

Base::txFloat Planning_SceneLoader::EgoRouteViewer::startLat() const TX_NOEXCEPT {
  if (_NonEmpty_(m_vec_ego_path)) {
    return __Lat__(m_vec_ego_path.front());
  } else {
    return 0.0;
  }
}

Base::txFloat Planning_SceneLoader::EgoRouteViewer::endLon() const TX_NOEXCEPT {
  if (m_vec_ego_path.size() > 1) {
    return __Lon__(m_vec_ego_path.back());
  } else {
    return 0.0;
  }
}

Base::txFloat Planning_SceneLoader::EgoRouteViewer::endLat() const TX_NOEXCEPT {
  if (m_vec_ego_path.size() > 1) {
    return __Lat__(m_vec_ego_path.back());
  } else {
    return 0.0;
  }
}

std::vector<std::pair<Base::txFloat, Base::txFloat> > Planning_SceneLoader::EgoRouteViewer::midPoints() const
    TX_NOEXCEPT {
  const auto res_vec_gps = midGPS();
  std::vector<std::pair<Base::txFloat, Base::txFloat> > ret_vec;
  for (const auto& ref_gps : res_vec_gps) {
    ret_vec.emplace_back(std::make_pair(__Lon__(ref_gps), __Lat__(ref_gps)));
  }
  return ret_vec;
}

hadmap::txPoint Planning_SceneLoader::EgoRouteViewer::startGPS() const TX_NOEXCEPT {
  if (_NonEmpty_(m_vec_ego_path)) {
    return (m_vec_ego_path.front());
  } else {
    return hadmap::txPoint();
  }
}

hadmap::txPoint Planning_SceneLoader::EgoRouteViewer::endGPS() const TX_NOEXCEPT {
  if (m_vec_ego_path.size() > 1) {
    return (m_vec_ego_path.back());
  } else {
    return hadmap::txPoint();
  }
}

std::vector<hadmap::txPoint> Planning_SceneLoader::EgoRouteViewer::midGPS() const TX_NOEXCEPT {
  if (m_vec_ego_path.size() > 1) {
    std::vector<hadmap::txPoint> ret_vec = m_vec_ego_path;
    ret_vec.erase(ret_vec.begin());
    ret_vec.erase(ret_vec.end() - 1);
    return ret_vec;
  } else {
    return std::vector<hadmap::txPoint>();
  }
}

Base::ISceneLoader::IRouteViewerPtr Planning_SceneLoader::GetEgoRouteData() TX_NOEXCEPT {
  EgoRouteViewerPtr retPtr = std::make_shared<EgoRouteViewer>();
  if (NonNull_Pointer(retPtr)) {
    retPtr->Init(m_vec_ego_path);
  }
  return retPtr;
}

Base::ISceneLoader::IViewerPtr Planning_SceneLoader::GetEgoStartData() TX_NOEXCEPT {
  EgoStartViewerPtr retPtr = std::make_shared<EgoStartViewer>();
  if (NonNull_Pointer(retPtr)) {
    retPtr->Init(m_ego_start_location);
  }
  return retPtr;
}

Base::txBool Planning_SceneLoader::Init(const std::vector<hadmap::txPoint>& vec_ego_path,
                                        const sim_msg::Location ego_start_loction,
                                        const sim_msg::Scene& sim_scene_pb) TX_NOEXCEPT {
  m_vec_ego_path = vec_ego_path;
  m_ego_start_location.CopyFrom(ego_start_loction);
  m_scene_pb.CopyFrom(sim_scene_pb);
  return true;
}

Base::txFloat Planning_SceneLoader::PlannerStartV() const TX_NOEXCEPT {
  if (m_scene_pb.egos_size() > 0) {
    return GetEgoFromGroupName().initial().common().speed();
  } else {
    return ParentClass::PlannerStartV();
  }
}

Base::txFloat Planning_SceneLoader::PlannerTheta() const TX_NOEXCEPT { return m_ego_start_location.rpy().z(); }

Base::txFloat Planning_SceneLoader::PlannerVelocityMax() const TX_NOEXCEPT {
  if (m_scene_pb.egos_size() > 0 && GetEgoFromGroupName().physicles_size() > 0) {
    return GetEgoFromGroupName().physicles(0).performance().max_speed();
  } else {
    return ParentClass::PlannerVelocityMax();
  }
}

Base::txFloat Planning_SceneLoader::PlannerAccMax() const TX_NOEXCEPT {
  if (m_scene_pb.egos_size() > 0 && GetEgoFromGroupName().physicles_size() > 0) {
    return GetEgoFromGroupName().physicles(0).performance().max_accel();
  } else {
    return ParentClass::PlannerAccMax();
  }
}

Base::txFloat Planning_SceneLoader::PlannerDeceMax() const TX_NOEXCEPT {
  if (m_scene_pb.egos_size() > 0 && GetEgoFromGroupName().physicles_size() > 0) {
    return GetEgoFromGroupName().physicles(0).performance().max_decel();
  } else {
    return ParentClass::PlannerDeceMax();
  }
}

std::vector<sim_msg::Waypoint> Planning_SceneLoader::ControlPathPoints() const TX_NOEXCEPT {
  std::vector<sim_msg::Waypoint> ret_waypoints;
  if (m_scene_pb.egos_size() > 0) {
    const auto ref_way_points = GetEgoFromGroupName().initial().common().waypoints();
    LOG(INFO) << TX_VARS_NAME(GetEgoFromGroupName().initial().common().waypoints(), ref_way_points.size());
    ret_waypoints.resize(ref_way_points.size());
    TX_MARK("ID110365895");
    std::copy(std::begin(ref_way_points), std::end(ref_way_points), std::begin(ret_waypoints));
  } else {
    LOG(WARNING) << TX_VARS(m_scene_pb.egos_size()) << ", Control path empty.";
  }
  return ret_waypoints;
}

const sim_msg::Ego Planning_SceneLoader::GetEgoFromGroupName() const TX_NOEXCEPT {
  for (size_t i = 0; i < m_scene_pb.egos_size(); i++) {
    auto it_ego = m_scene_pb.egos(i);
    if (Base::txString(it_ego.group()) == m_groupname) {
      return it_ego;
    }
  }
  return m_scene_pb.egos(0);
}
TX_NAMESPACE_CLOSE(SceneLoader)
