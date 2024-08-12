// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "interpolate_player_module.h"
#include <fstream>
#include <sstream>
#include "tad_sim.h"
#include "tx_catalog_cache.h"
#include "tx_path_utils.h"
#include "tx_string_utils.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"

#if !USE_PB_LOGSIM
using namespace tx_sim::record;
#endif /*USE_PB_LOGSIM*/
#define SIM_INFO LOG(INFO)
#define SIM_WARN LOG(WARNING)
#define InfoLog LOG_IF(INFO, FLAGS_LogLevel_XOSC_Replay)

const double InterpolatePlayerModule::cache_time_ms_ = 500.0;

static constexpr double PI = 3.1415926536;
static constexpr double TWO_PI = PI * 2;
static double interp(double t1, double t2, double v1, double v2, double t) {
  return v1 + (v2 - v1) * (t - t1) / (t2 - t1);
}

static double interp_angle(double t1, double t2, double a1, double a2, double t) {
  if (a2 - a1 > PI) {
    a1 += TWO_PI;
  } else if (a1 - a2 > PI) {
    a2 += TWO_PI;
  }
  double res = interp(t1, t2, a1, a2, t);
  if (res > PI) {
    res -= TWO_PI;
  }
  return res;
}

static sim_msg::Vec3 interp(double t1, double t2, const sim_msg::Vec3& v1, const sim_msg::Vec3& v2, double t) {
  sim_msg::Vec3 res;
  res.set_x(interp(t1, t2, v1.x(), v2.x(), t));
  res.set_y(interp(t1, t2, v1.y(), v2.y(), t));
  res.set_z(interp(t1, t2, v1.z(), v2.z(), t));
  return res;
}

sim_msg::Vec3 interp_wgs84(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Vec3& v1,
                           const sim_msg::Vec3& v2, const Base::txFloat t) TX_NOEXCEPT {
  sim_msg::Vec3 res;
  Coord::txENU car1_enu;
  car1_enu.FromWGS84(v1.x(), v1.y());
  Coord::txENU car2_enu;
  car2_enu.FromWGS84(v2.x(), v2.y());
  Coord::txWGS84 res_wgs84;
  res_wgs84.FromENU(interp(t1, t2, car1_enu.X(), car2_enu.X(), t), interp(t1, t2, car1_enu.Y(), car2_enu.Y(), t));
  res.set_x(res_wgs84.Lon() /*interp(t1, t2, v1.x(), v2.x(), t)*/);
  res.set_y(res_wgs84.Lat() /*interp(t1, t2, v1.y(), v2.y(), t)*/);
  res.set_z(interp(t1, t2, v1.z(), v2.z(), t));
  return std::move(res);
}

static sim_msg::Location interp(double t1, double t2, const sim_msg::Location& loc1, const sim_msg::Location& loc2,
                                double t) {
  sim_msg::Location res;
  res.set_t(Utils::MillisecondToSecond(t));
  *res.mutable_position() = interp(t1, t2, loc1.position(), loc2.position(), t);
  *res.mutable_velocity() = interp(t1, t2, loc1.velocity(), loc2.velocity(), t);
  if (CallSucc(InterpolatePlayerModule::Heading_Interpolation())) {
    *res.mutable_angular() = interp(t1, t2, loc1.angular(), loc2.angular(), t);
  } else {
    *res.mutable_angular() = loc1.angular();
  }
  *res.mutable_rpy() = interp(t1, t2, loc1.rpy(), loc2.rpy(), t);
  *res.mutable_acceleration() = interp(t1, t2, loc1.acceleration(), loc2.acceleration(), t);
  return res;
}

static sim_msg::Car interp(double t1, double t2, const sim_msg::Car& car1, const sim_msg::Car& car2, double t) {
  sim_msg::Car res = car1;
  res.set_t(Utils::MillisecondToSecond(t));

  Coord::txENU car1_enu;
  car1_enu.FromWGS84(car1.x(), car1.y());
  Coord::txENU car2_enu;
  car2_enu.FromWGS84(car2.x(), car2.y());
  Coord::txWGS84 res_wgs84;
  res_wgs84.FromENU(interp(t1, t2, car1_enu.X(), car2_enu.X(), t), interp(t1, t2, car1_enu.Y(), car2_enu.Y(), t));
  res.set_x(res_wgs84.Lon() /*interp(t1, t2, car1.x(), car2.x(), t)*/);
  res.set_y(res_wgs84.Lat() /*interp(t1, t2, car1.y(), car2.y(), t)*/);
  res.set_v(interp(t1, t2, car1.v(), car2.v(), t));
  res.set_vl(interp(t1, t2, car1.vl(), car2.vl(), t));

  if (CallSucc(InterpolatePlayerModule::Heading_Interpolation())) {
    res.set_heading(interp_angle(t1, t2, car1.heading(), car2.heading(), t));
    res.set_theta(interp_angle(t1, t2, car1.theta(), car2.theta(), t));
  } else {
    res.set_heading(car1.heading());
    res.set_theta(car1.theta());
  }

  res.set_length(interp(t1, t2, car1.length(), car2.length(), t));
  res.set_width(interp(t1, t2, car1.width(), car2.width(), t));
  res.set_height(interp(t1, t2, car1.height(), car2.height(), t));

  return std::move(res);
}

static sim_msg::DynamicObstacle interp(double t1, double t2, const sim_msg::DynamicObstacle& ped1,
                                       const sim_msg::DynamicObstacle& ped2, double t) {
  sim_msg::DynamicObstacle res = ped1;
  res.set_t(Utils::MillisecondToSecond(t));
  res.set_x(interp(t1, t2, ped1.x(), ped2.x(), t));
  res.set_y(interp(t1, t2, ped1.y(), ped2.y(), t));
  res.set_z(interp(t1, t2, ped1.z(), ped2.z(), t));

  if (CallSucc(InterpolatePlayerModule::Heading_Interpolation())) {
    res.set_heading(interp_angle(t1, t2, ped1.heading(), ped2.heading(), t));
  } else {
    res.set_heading(ped1.heading());
  }

  res.set_v(interp(t1, t2, ped1.v(), ped2.v(), t));
  res.set_vl(interp(t1, t2, ped1.vl(), ped2.vl(), t));
  res.set_age(interp(t1, t2, ped1.age(), ped2.age(), t));
  res.set_length(interp(t1, t2, ped1.length(), ped2.length(), t));
  res.set_width(interp(t1, t2, ped1.width(), ped2.width(), t));
  res.set_height(interp(t1, t2, ped1.height(), ped2.height(), t));
  return std::move(res);
}

static sim_msg::StaticObstacle interp(double t1, double t2, const sim_msg::StaticObstacle& obs1,
                                      const sim_msg::StaticObstacle& obs2, double t) {
  sim_msg::StaticObstacle res = obs1;
  res.set_t(Utils::MillisecondToSecond(t));
  res.set_x(interp(t1, t2, obs1.x(), obs2.x(), t));
  res.set_y(interp(t1, t2, obs1.y(), obs2.y(), t));
  res.set_z(interp(t1, t2, obs1.z(), obs2.z(), t));
  if (CallSucc(InterpolatePlayerModule::Heading_Interpolation())) {
    res.set_heading(interp_angle(t1, t2, obs1.heading(), obs2.heading(), t));
  } else {
    res.set_heading(obs1.heading());
  }
  res.set_length(interp(t1, t2, obs1.length(), obs2.length(), t));
  res.set_width(interp(t1, t2, obs1.width(), obs2.width(), t));
  res.set_height(interp(t1, t2, obs1.height(), obs2.height(), t));
  res.set_age(interp(t1, t2, obs1.age(), obs2.age(), t));
  return std::move(res);
}

static sim_msg::TrafficLight interp(double t1, double t2, const sim_msg::TrafficLight& light1,
                                    const sim_msg::TrafficLight& light2, double t) {
  sim_msg::TrafficLight res = light1;
  return std::move(res);
}

bool GetSimRecFilePath(const std::string& strSimFilePath, std::string& strSimRecFilePath) {
  using namespace Utils;
  using namespace boost::filesystem;
  if (exists(FilePath(strSimFilePath))) {
    SIM_INFO << "Loading TAD Scene File : " << strSimFilePath;
    SceneLoader::Sim::simulation_ptr m_DataSource_Scene = nullptr;
    try {
      m_DataSource_Scene = SceneLoader::Sim::load_scene_sim(strSimFilePath);
    } catch (...) {
      SIM_WARN << "Loading TAD Scene File : " << strSimFilePath << " Except.";
      return false;
    }

    FilePath path_CurrentPath = FilePath(strSimFilePath).parent_path();
    FilePath sim_file_path = path_CurrentPath;
    sim_file_path.append((m_DataSource_Scene->traffic_file_path));
    strSimRecFilePath = (sim_file_path).string();
    SIM_WARN << "Loading TAD Traffic File :" << strSimRecFilePath;
    SIM_WARN << "Scene File mapfile =" << (m_DataSource_Scene->_mapfile.mapfile);
    return true;
  } else {
    SIM_WARN << "Scene File is not existing. File : " << strSimFilePath;
    return false;
  }
}

bool LoadCatalogData(const std::string& strSimFilePath) {
  using namespace Utils;
  using namespace boost::filesystem;
  if (exists(FilePath(strSimFilePath))) {
    SIM_INFO << "Loading TAD Scene File : " << strSimFilePath;
    FilePath path_CurrentPath = FilePath(strSimFilePath).parent_path();

    try {
      // 获取veh catalog路径
      FilePath veh_cata_path = path_CurrentPath;
      veh_cata_path.append(FLAGS_Catalog_VehicleCatalog);

      // 获取ped catalog路径
      FilePath ped_cata_path = path_CurrentPath;
      ped_cata_path.append(FLAGS_Catalog_PedestrianCatalog);

      FilePath obs_cata_path = path_CurrentPath;
      obs_cata_path.append(FLAGS_Catalog_MiscObjectCatalog);
      LOG(WARNING) << "Loading TAD Catalog File : " << TX_VARS(veh_cata_path.string())
                   << TX_VARS(ped_cata_path.string()) << TX_VARS(ped_cata_path.string());

      Base::CatalogCache::Release();
      if (CallSucc(
              Base::CatalogCache::Initialize(veh_cata_path.string(), ped_cata_path.string(), obs_cata_path.string()))) {
        SIM_INFO << "Base::CatalogCache::Initialize() success.";
        Base::CatalogCache::ShowCacheInfo();
      } else {
        SIM_WARN << "Base::CatalogCache::Initialize() failure.";
      }
    } catch (...) {
      SIM_WARN << "Loading Catalog File : " << strSimFilePath << " Except.";
    }
    return true;
  } else {
    SIM_WARN << "Scene File is not existing. File : " << strSimFilePath;
    return false;
  }
}

bool QueryCatalog(const Base::Enums::VEHICLE_TYPE _vehType, double& length, double& width, double& height) TX_NOEXCEPT {
  // 查询指定类型的车辆信息
  Base::CatalogCache::Catalog_Vehicle veh_catalog;
  if (CallSucc(Base::CatalogCache::Query_Vehicle_Catalog(_vehType, veh_catalog))) {
    const auto& dim = veh_catalog.bbx.dim;
    // 使用底部中心点坐标初始化车辆信息
    length = dim.dbLength;
    width = dim.dbWidth;
    height = dim.dbHeight;
    return true;
  } else {
    return false;
  }
}

bool QueryCatalog(const Base::Enums::PEDESTRIAN_TYPE _pedType, double& length, double& width,
                  double& height) TX_NOEXCEPT {
  // 从目录中查询指定类型的行人信息
  Base::CatalogCache::Catalog_Pedestrian ped_catalog;
  if (CallSucc(Base::CatalogCache::Query_Pedestrian_Catalog(_pedType, ped_catalog))) {
    const auto& dim = ped_catalog.bbx.dim;
    // 使用底部中心点坐标初始化车辆信息
    length = dim.dbLength;
    width = dim.dbWidth;
    height = dim.dbHeight;
    return true;
  } else {
    return false;
  }
}

bool QueryCatalog(const Base::Enums::STATIC_ELEMENT_TYPE _obsType, double& length, double& width,
                  double& height) TX_NOEXCEPT {
  // 根据给定的静态元素类型，从目录中查询对应的静态元素信息
  Base::CatalogCache::Catalog_MiscObject obs_catalog;
  if (CallSucc(Base::CatalogCache::Query_Obstacle_Catalog(_obsType, obs_catalog))) {
    const auto& dim = obs_catalog.bbx.dim;
    // 使用底部中心点坐标初始化车辆信息
    length = dim.dbLength;
    width = dim.dbWidth;
    height = dim.dbHeight;
    return true;
  } else {
    return false;
  }
}

void InterpolatePlayerModule::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  if (FLAGS_LogsimPub_Location) {
    helper.Publish(FLAGS_LOGSIM_RawLocation_TopicName);
    SIM_INFO << "register topic: " << FLAGS_LOGSIM_RawLocation_TopicName;
  }
  if (FLAGS_LogsimPub_Traffic) {
    helper.Publish(FLAGS_LOGSIM_RawTraffic_TopicName);
    SIM_INFO << "register topic: " << FLAGS_LOGSIM_RawTraffic_TopicName;
  }
  if (FLAGS_LogsimPub_Trajectory) {
    helper.Publish(FLAGS_LOGSIM_RawTrajectory_TopicName);
    SIM_INFO << "register topic: " << FLAGS_LOGSIM_RawTrajectory_TopicName;
  }
}

void InterpolatePlayerModule::Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  std::string record_files;
  trafficRecords_.Clear();
  m_time_diff_ns = 0.0;

  if (GetSimRecFilePath(helper.scenario_file_path(), record_files) && CallSucc(IsSimModeValid(record_files))) {
    LoadCatalogData(helper.scenario_file_path());
#if USE_PB_LOGSIM
    std::fstream input(record_files, std::ios::in | std::ios::binary);
    if (!trafficRecords_.ParseFromIstream(&input)) {
      std::cerr << "Failed to parse dynamic data." << std::endl;
    } else {
      SIM_INFO << TX_VARS(trafficRecords_.ego_location_size())
               << TX_VARS(trafficRecords_.traffic_record_size()) TX_VARS(trafficRecords_.duration_ms());
      for (auto info : trafficRecords_.disengage_events()) {
        SIM_INFO << TX_VARS(info.timestamp_ms()) << TX_VARS(info.type());
      }
    }

    begin_time = 0.0;
    if (trafficRecords_.ego_location_size() > 0) {
      // !!! xosc2simrec use ms, but vissim sceneexport use s
      begin_time = trafficRecords_.ego_location(0).t();
      if (begin_time < 1E12) {
        begin_time = Utils::SecondToMillisecond(begin_time);
      }
      m_originPoint.x = trafficRecords_.ego_location(0).position().x();
      m_originPoint.y = trafficRecords_.ego_location(0).position().y();
      m_originPoint.z = trafficRecords_.ego_location(0).position().z();
    }

    SIM_INFO << TX_VARS(begin_time) << _StreamPrecision_ << TX_VARS(m_originPoint.x) << TX_VARS(m_originPoint.y)
             << TX_VARS(m_originPoint.z) << TX_VARS(trafficRecords_.traffic_record_size());

    if (trafficRecords_.traffic_record_size() > 0 && trafficRecords_.traffic_record(0).cars_size() > 0) {
      if (begin_time > 0.0) {
        begin_time = std::min(begin_time, trafficRecords_.traffic_record(0).cars(0).t());
        InfoLog << TX_VARS(begin_time);
      } else {
        begin_time = trafficRecords_.traffic_record(0).cars(0).t();
        InfoLog << TX_VARS(begin_time);
      }
    }
#  if _Pedestrian_
    if (trafficRecords_.traffic_record_size() > 0 && trafficRecords_.traffic_record(0).dynamicobstacles_size() > 0) {
      if (begin_time > 0.0) {
        begin_time = std::min(begin_time, trafficRecords_.traffic_record(0).dynamicobstacles(0).t());
      } else {
        begin_time = trafficRecords_.traffic_record(0).dynamicobstacles(0).t();
      }
    }
    if (trafficRecords_.traffic_record_size() > 0 && trafficRecords_.traffic_record(0).staticobstacles_size() > 0) {
      if (begin_time > 0.0) {
        begin_time = std::min(begin_time, trafficRecords_.traffic_record(0).staticobstacles(0).t());
      } else {
        begin_time = trafficRecords_.traffic_record(0).staticobstacles(0).t();
      }
    }
#  endif /*_Pedestrian_*/

    InfoLog << TX_VARS(begin_time);
    double time_stamp = 0.0;
    uint64_t current_time_ns = static_cast<uint64_t>(time_stamp * 1000000);
    uint64_t cache_time_ns = static_cast<uint64_t>(cache_time_ms_ * 1000000);
    m_time_diff_ns = begin_time - current_time_ns;
    cars_.clear();
#  if _Pedestrian_
    pedestrians_.clear();
    obstacles_.clear();
    lights_.clear();
#  endif /*_Pedestrian_*/
    for (const auto& traffic : trafficRecords_.traffic_record()) {
      for (int i = 0; i < traffic.cars_size(); ++i) {
        auto& car = traffic.cars(i);
        // SIM_INFO << "car " << car.id() << " time: " << (car.t() - time_diff_);
        cars_[car.id()][car.t() - m_time_diff_ns] = car;
        InfoLog << _StreamPrecision_ << TX_VARS(car.t()) << TX_VARS(car.t() - m_time_diff_ns) << TX_VARS(m_time_diff_ns)
                << TX_VARS(car.id()) << TX_VARS(car.x()) << TX_VARS(car.y());
      }
#  if _Pedestrian_
      for (int i = 0; i < traffic.dynamicobstacles_size(); ++i) {
        auto& ped = traffic.dynamicobstacles(i);
        pedestrians_[ped.id()][ped.t() - m_time_diff_ns] = ped;
      }
      for (int i = 0; i < traffic.staticobstacles_size(); ++i) {
        auto& obs = traffic.staticobstacles(i);
        obstacles_[obs.id()][obs.t() - m_time_diff_ns] = obs;
      }
      for (int i = 0; i < traffic.trafficlights_size(); ++i) {
        auto& light = traffic.trafficlights(i);
        lights_[light.id()][traffic.time() - m_time_diff_ns] = light;
      }
#  endif /*_Pedestrian_*/
    }

    for (const auto& refCarList : cars_) {
      std::vector<Coord::txWGS84> tmp;
      for (const auto& refCarPair : refCarList.second) {
        const auto& refCar = refCarPair.second;
        Coord::txWGS84 gps;
        gps.FromWGS84(refCar.x(), refCar.y());
        tmp.push_back(gps);
      }
      std::ostringstream oss;
      oss << TX_VARS_NAME(carId, refCarList.first);
      for (size_t i = 1; i < tmp.size(); i++) {
        oss << "{" << TX_VARS(i)
            << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(tmp[i], tmp[i - 1])) << "},";
      }
      InfoLog << oss.str();
    }

    for (const auto& refPedList : pedestrians_) {
      std::vector<Coord::txWGS84> tmp;
      for (const auto& refPedPair : refPedList.second) {
        const auto& refPed = refPedPair.second;
        Coord::txWGS84 gps;
        gps.FromWGS84(refPed.x(), refPed.y());
        tmp.push_back(gps);
      }
      std::ostringstream oss;
      oss << TX_VARS_NAME(pedId, refPedList.first);
      for (size_t i = 1; i < tmp.size(); i++) {
        oss << "{" << TX_VARS(i)
            << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(tmp[i], tmp[i - 1])) << "},";
      }
      InfoLog << oss.str();
    }

    locations_.clear();
    for (const auto& location : trafficRecords_.ego_location()) {
      // !!! xosc2simrec use ms, but vissim sceneexport use s
      Base::txFloat timestamp = location.t();
      if (timestamp < 1E12) {
        timestamp = Utils::SecondToMillisecond(timestamp);
      }
      InfoLog << "location time: " << TX_VARS(timestamp) << TX_VARS(timestamp - m_time_diff_ns);
      locations_[timestamp - m_time_diff_ns] = location;
    }
#endif

#if __TX_Mark__("frame-base")
    m_vec_Traffic.clear();
    m_vec_Location.clear();
    for (const auto& traffic : trafficRecords_.traffic_record()) {
      const double cur_frame_time = ComputeFrameTime(traffic);
      if (cur_frame_time < -100.0) {
        m_vec_Traffic.emplace_back(std::make_tuple(cur_frame_time - m_time_diff_ns, traffic));
      }
    }

    for (const auto& location : trafficRecords_.ego_location()) {
      m_vec_Location.emplace_back(std::make_tuple(location.t() - m_time_diff_ns, location));
    }
    SIM_INFO << TX_VARS(m_vec_Location.size()) << TX_VARS(m_vec_Traffic.size());
#endif /*__TX_Mark__("frame-base")*/

    Coord::txReferencePoint::SetReferencePoint(m_originPoint);
  } else {
    SIM_WARN << "open sim file failure. " << helper.scenario_file_path();
    exit(201);
  }
}

double InterpolatePlayerModule::ComputeFrameTime(const sim_msg::Traffic& ref_traffic) {
  if (ref_traffic.cars_size() > 0) {
    return ref_traffic.cars(0).t();
  } else if (ref_traffic.dynamicobstacles_size() > 0) {
    return ref_traffic.dynamicobstacles(0).t();
  } else if (ref_traffic.staticobstacles_size() > 0) {
    return ref_traffic.staticobstacles(0).t();
  } else {
    return -999.0;
  }
}

#if _Pedestrian_
void InterpolatePlayerModule::InterpPedestrians(sim_msg::Traffic& traffic, double time_ms) {
  auto ped_it = pedestrians_.begin();
  while (ped_it != pedestrians_.end()) {
    std::map<double, sim_msg::DynamicObstacle>& refPedMap = ped_it->second;
    while (!refPedMap.empty() && time_ms - refPedMap.begin()->first > cache_time_ms_) {
      refPedMap.erase(refPedMap.begin());
    }
    if (refPedMap.empty()) {
      ped_it = pedestrians_.erase(ped_it);
    } else {
      if (refPedMap.begin()->first <= time_ms) {
        auto it2 = ++(refPedMap.begin());
        while (it2 != refPedMap.end()) {
          if (std::prev(it2)->first <= time_ms && it2->first >= time_ms) {
            *traffic.add_dynamicobstacles() =
                interp(std::prev(it2)->first, it2->first, std::prev(it2)->second, it2->second, time_ms);
            refPedMap.erase(refPedMap.begin(), std::prev(it2));
            break;
          }
          ++it2;
        }
      }
      ++ped_it;
    }
  }
}

void InterpolatePlayerModule::InterpObstacles(sim_msg::Traffic& traffic, double time_ms) {
  auto obs_it = obstacles_.begin();
  while (obs_it != obstacles_.end()) {
    std::map<double, sim_msg::StaticObstacle>& refObsMap = obs_it->second;
    while (!refObsMap.empty() && time_ms - refObsMap.begin()->first > cache_time_ms_) {
      refObsMap.erase(refObsMap.begin());
    }
    if (refObsMap.empty()) {
      obs_it = obstacles_.erase(obs_it);
    } else {
      if (refObsMap.begin()->first <= time_ms) {
        auto it2 = ++(refObsMap.begin());
        while (it2 != refObsMap.end()) {
          if (std::prev(it2)->first <= time_ms && it2->first >= time_ms) {
            *traffic.add_staticobstacles() =
                interp(std::prev(it2)->first, it2->first, std::prev(it2)->second, it2->second, time_ms);
            refObsMap.erase(refObsMap.begin(), std::prev(it2));
            break;
          }
          ++it2;
        }
      }
      ++obs_it;
    }
  }
}

void InterpolatePlayerModule::InterpLights(sim_msg::Traffic& traffic, double time_ms) {
  auto light_it = lights_.begin();
  while (light_it != lights_.end()) {
    std::map<double, sim_msg::TrafficLight>& refLightMap = light_it->second;
    while (!refLightMap.empty() && time_ms - refLightMap.begin()->first > cache_time_ms_) {
      refLightMap.erase(refLightMap.begin());
    }
    if (refLightMap.empty()) {
      light_it = lights_.erase(light_it);
    } else {
      if (refLightMap.begin()->first <= time_ms) {
        auto it2 = ++(refLightMap.begin());
        while (it2 != refLightMap.end()) {
          if (std::prev(it2)->first <= time_ms && it2->first >= time_ms) {
            *traffic.add_trafficlights() =
                interp(std::prev(it2)->first, it2->first, std::prev(it2)->second, it2->second, time_ms);
            refLightMap.erase(refLightMap.begin(), std::prev(it2));
            break;
          }
          ++it2;
        }
      }
      ++light_it;
    }
  }
}
#endif /*_Pedestrian_*/

void InterpolatePlayerModule::Stop(tx_sim::StopHelper& helper) TX_NOEXCEPT {
#if USE_PB_LOGSIM
  trafficRecords_.Clear();
#endif
}

sim_msg::Traffic InterpolatePlayerModule::SearchTraffic(double time_ms) {
#define _time_idx_ (0)
#define _data_idx_ (1)
  while (!m_vec_Traffic.empty() && (time_ms - std::get<_time_idx_>(*m_vec_Traffic.begin())) > cache_time_ms_) {
    m_vec_Traffic.erase(m_vec_Traffic.begin());
  }
  sim_msg::Traffic ret;
  if (_NonEmpty_(m_vec_Traffic)) {
    if (std::get<_time_idx_>(*m_vec_Traffic.begin()) <= time_ms) {
      auto it2 = ++(m_vec_Traffic.begin());
      while (it2 != m_vec_Traffic.end()) {
        const double prev_time = std::get<_time_idx_>(*std::prev(it2));
        const double curr_time = std::get<_time_idx_>(*it2);
        InfoLog << TX_VARS(prev_time) << TX_VARS(time_ms) << TX_VARS(curr_time);
        if (prev_time <= time_ms && time_ms < curr_time) {
          ret.CopyFrom(std::get<_data_idx_>(*std::prev(it2)));
          m_vec_Traffic.erase(std::prev(it2));
          break;
        }
        ++it2;
      }
    }
  }
  return std::move(ret);

#undef _time_idx_
#undef _data_idx_
}

sim_msg::Location InterpolatePlayerModule::SearchLocation(double time_ms) {
#define _time_idx_ (0)
#define _data_idx_ (1)

  SIM_INFO << "interp location for " << time_ms;
  while (m_vec_Location.size() >= 2 && (time_ms - std::get<_time_idx_>(*m_vec_Location.begin())) > cache_time_ms_) {
    m_vec_Location.erase(m_vec_Location.begin());
  }

  sim_msg::Location ret;
  if (_NonEmpty_(m_vec_Location)) {
    auto it1 = m_vec_Location.begin();
    auto it2 = ++m_vec_Location.begin();

    if (std::get<_time_idx_>(*it1) > time_ms) {
      ret.CopyFrom(std::get<_data_idx_>(*it1));
      return std::move(ret);
    }
    while (it2 != m_vec_Location.end()) {
      const double left_time = std::get<_time_idx_>(*it1);
      const double right_time = std::get<_time_idx_>(*it2);
      if (left_time <= time_ms && right_time >= time_ms) {
        ret.CopyFrom(std::get<_data_idx_>(*it1));
        return std::move(ret);
      }
      ++it1;
      ++it2;
    }
  }
  return std::move(ret);

#undef _time_idx_
#undef _data_idx_
}

sim_msg::Trajectory InterpolatePlayerModule::SearchTrajectory(double time_ms) { return InterpTrajectory(time_ms); }

void InterpolatePlayerModule::Step(tx_sim::StepHelper& helper) TX_NOEXCEPT {
#if USE_PB_LOGSIM
  const double time_stamp = helper.timestamp();

  if (!locations_.empty() && FLAGS_LogsimPub_Location) {
    sim_msg::Location location;
    if (Frame_Interpolation()) {
      location = InterpLocation(time_stamp);
    } else {
      location = SearchLocation(time_stamp);
    }

    InfoLog << time_stamp << " location = " << location.DebugString();
    helper.PublishMessage(FLAGS_LOGSIM_RawLocation_TopicName, location.SerializeAsString());
  }

  if (FLAGS_LogsimPub_Traffic) {
    sim_msg::Traffic traffic;
    if (Frame_Interpolation()) {
      traffic = InterpTraffic(time_stamp);
    } else {
      traffic = SearchTraffic(time_stamp);
    }

    if (!locations_.empty() && FLAGS_LogsimPub_Location && (traffic.egos_size() == 0)) {
      // add egoInfo
      auto ego_info = traffic.add_egos();
    }

    // Set the size and display attributes of traffic participants
    for (int i = 0; i < traffic.cars_size(); ++i) {
      double length = 0.0, width = 0.0, height = 0.0;
      QueryCatalog(__int2enum__(VEHICLE_TYPE, traffic.cars().at(i).type()), length, width, height);

      InfoLog << TX_VARS_NAME(vehicleType, __int2enum__(VEHICLE_TYPE, traffic.cars().at(i).type()))
              << TX_VARS_NAME(vehicleTypeStr,
                              __enum2lpsz__(VEHICLE_TYPE, __int2enum__(VEHICLE_TYPE, traffic.cars().at(i).type())))
              << TX_VARS(length) << TX_VARS(width) << TX_VARS(height);
      traffic.mutable_cars(i)->set_length(length);
      traffic.mutable_cars(i)->set_width(width);
      traffic.mutable_cars(i)->set_height(height);
      // traffic.mutable_cars(i)->set_show_abs_velocity(traffic.cars().at(i).v());
      // traffic.mutable_cars(i)->set_show_abs_acc(traffic.cars().at(i).acc());
    }

    for (int i = 0; i < traffic.dynamicobstacles_size(); ++i) {
      double length = 0.0, width = 0.0, height = 0.0;
      QueryCatalog(__int2enum__(PEDESTRIAN_TYPE, traffic.dynamicobstacles().at(i).type()), length, width, height);

      traffic.mutable_dynamicobstacles(i)->set_length(length);
      traffic.mutable_dynamicobstacles(i)->set_width(width);
      traffic.mutable_dynamicobstacles(i)->set_height(height);
      // traffic.mutable_dynamicobstacles(i)->set_show_abs_velocity(traffic.dynamicobstacles().at(i).v());
      // traffic.mutable_dynamicobstacles(i)->set_show_abs_acc(traffic.dynamicobstacles().at(i).acc());
    }

    for (int i = 0; i < traffic.staticobstacles_size(); ++i) {
      double length = 0.0, width = 0.0, height = 0.0;
      QueryCatalog(__int2enum__(STATIC_ELEMENT_TYPE, traffic.staticobstacles().at(i).type()), length, width, height);

      traffic.mutable_staticobstacles(i)->set_length(length);
      traffic.mutable_staticobstacles(i)->set_width(width);
      traffic.mutable_staticobstacles(i)->set_height(height);
    }

    InfoLog << time_stamp << " num cars = " << traffic.cars_size();
    helper.PublishMessage(FLAGS_LOGSIM_RawTraffic_TopicName, traffic.SerializeAsString());
  }

  if (FLAGS_LogsimPub_Trajectory) {
    sim_msg::Trajectory traj;
    if (Frame_Interpolation()) {
      traj = InterpTrajectory(time_stamp);
    } else {
      traj = SearchTrajectory(time_stamp);
    }

    InfoLog << time_stamp << " traj = " << traj.point_size();
    helper.PublishMessage(FLAGS_LOGSIM_RawTrajectory_TopicName, traj.SerializeAsString());
  }

  if (Finished(time_stamp)) {
    helper.StopScenario("Play finished");
  }
#endif
}

sim_msg::Traffic InterpolatePlayerModule::InterpTraffic(double time_ms) {
  InfoLog << "interp traffic for " << time_ms;
  sim_msg::Traffic traffic;
  {
    auto car_it = cars_.begin();
    while (car_it != cars_.end()) {
      std::map<double, sim_msg::Car>& refCarMap = car_it->second;
      while (!refCarMap.empty() && time_ms - refCarMap.begin()->first > cache_time_ms_) {
        refCarMap.erase(refCarMap.begin());
      }
      if (refCarMap.empty()) {
        car_it = cars_.erase(car_it);
      } else {
        InfoLog << TX_VARS(refCarMap.begin()->first) << TX_VARS(time_ms);
        if (refCarMap.begin()->first <= time_ms) {
          auto it2 = ++(refCarMap.begin());
          while (it2 != refCarMap.end()) {
            InfoLog << TX_VARS(std::prev(it2)->first) << TX_VARS(time_ms) << TX_VARS(it2->first) << TX_VARS(time_ms);
            if (std::prev(it2)->first <= time_ms && it2->first >= time_ms) {
              *traffic.add_cars() =
                  interp(std::prev(it2)->first, it2->first, std::prev(it2)->second, it2->second, time_ms);
              refCarMap.erase(refCarMap.begin(), std::prev(it2));
              break;
            }
            ++it2;
          }
        }
        ++car_it;
      }
    }
  }
  /*traffic.clear_cars();*/

#if _Pedestrian_
  InterpPedestrians(traffic, time_ms);
  InterpObstacles(traffic, time_ms);
  InterpLights(traffic, time_ms);
#endif /*_Pedestrian_*/
  if (FLAGS_LogLevel_XOSC_Replay) {
    InfoLog << "#############################";
    std::vector<Coord::txWGS84> tmp;
    for (const auto& refCar : traffic.cars()) {
      InfoLog << _StreamPrecision_ << TX_VARS(refCar.t()) << TX_VARS(refCar.id()) << TX_VARS(refCar.x())
              << TX_VARS(refCar.y());
      Coord::txWGS84 gps;
      gps.FromWGS84(refCar.x(), refCar.y());
      tmp.push_back(gps);
    }
    for (size_t i = 1; i < tmp.size(); i++) {
      InfoLog << TX_VARS(i)
              << TX_VARS_NAME(dist, Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(tmp[i], tmp[i - 1]));
    }
    InfoLog << "#############################";
  }
  return std::move(traffic);
}

sim_msg::Location InterpolatePlayerModule::InterpLocation(double time_ms) {
  SIM_INFO << "interp location for " << time_ms;
  while (locations_.size() >= 2 && time_ms - locations_.begin()->first > cache_time_ms_) {
    locations_.erase(locations_.begin());
  }

  if (locations_.empty()) {
    throw std::runtime_error("no location cached yet");
  }

  auto it1 = locations_.begin();
  auto it2 = ++locations_.begin();

  if (it1->first > time_ms) {
    return it1->second;
  }

  while (it2 != locations_.end()) {
    if (it1->first <= time_ms && it2->first >= time_ms) {
      return interp(it1->first, it2->first, it1->second, it2->second, time_ms);
    }
    ++it1;
    ++it2;
  }

  return it1->second;
}

sim_msg::Trajectory InterpolatePlayerModule::InterpTrajectory(double time_ms) {
  const double time_stamp_s = Utils::MillisecondToSecond(time_ms) + m_time_diff_ns / 1000.0;
  sim_msg::Trajectory ret_traj;
  const Base::txInt ego_trajectory_size = trafficRecords_.ego_trajectory_size();
  for (Base::txInt idx = 0; idx < ego_trajectory_size - 1; ++idx) {
    const Base::txFloat left_time_stamp_s = trafficRecords_.ego_trajectory(idx).time_stamp();
    const Base::txFloat right_time_stamp_s = trafficRecords_.ego_trajectory(idx + 1).time_stamp();

    if (left_time_stamp_s < right_time_stamp_s && left_time_stamp_s <= time_stamp_s &&
        time_stamp_s < right_time_stamp_s) {
      if ((time_stamp_s - left_time_stamp_s) < (right_time_stamp_s - time_stamp_s)) {
        ret_traj.CopyFrom(trafficRecords_.ego_trajectory(idx).trajectory_line());
        InfoLog << _StreamPrecision_ << TX_VARS(left_time_stamp_s) << TX_VARS(time_stamp_s)
                << TX_VARS(right_time_stamp_s) << TX_VARS(idx);
      } else {
        ret_traj.CopyFrom(trafficRecords_.ego_trajectory(idx + 1).trajectory_line());
        InfoLog << _StreamPrecision_ << TX_VARS(left_time_stamp_s) << TX_VARS(time_stamp_s)
                << TX_VARS(right_time_stamp_s) << TX_VARS(idx + 1);
      }

      break;
    }
  }
  return std::move(ret_traj);
}

bool InterpolatePlayerModule::Finished(const double time_stamp) const noexcept {
  LOG(WARNING) << TX_VARS(locations_.size());
  if (locations_.size() > 0) {
    for (const auto& ref : locations_) {
      LOG(WARNING) << TX_VARS(ref.first) << TX_VARS(time_stamp);
      if (ref.first >= time_stamp) {
        return false;
      }
    }
    return true;
  } else if (_NonEmpty_(cars_) || _NonEmpty_(pedestrians_) || _NonEmpty_(obstacles_)) {
    for (const auto& ref : cars_) {
      LOG(WARNING) << TX_VARS(ref.first) << TX_VARS(time_stamp);
      if (ref.first >= time_stamp) {
        return false;
      }
    }
    for (const auto& ref : pedestrians_) {
      if (ref.first >= time_stamp) {
        return false;
      }
    }
    for (const auto& ref : obstacles_) {
      if (ref.first >= time_stamp) {
        return false;
      }
    }
    return true;
  }
  { return true; }
}

Base::txBool InterpolatePlayerModule::IsSimModeValid(const Base::txString strScenePath) const TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  std::string strExt = FilePath(strScenePath).extension().string();
  if (boost::iequals(strExt, FLAGS_Logsim_Extension)) {
    return true;
  } else {
    LOG(WARNING) << "[Error] SimMode is logsim, " << TX_VARS(strExt) << TX_VARS(FLAGS_Logsim_Extension);
    return false;
  }
}

#undef InfoLog
#undef SIM_INFO
#undef SIM_WARN
