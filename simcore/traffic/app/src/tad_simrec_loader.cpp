// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_simrec_loader.h"
#include <fstream>
#include "log2world_traj_manager.h"
#include "traffic.pb.h"
#include "tx_catalog_cache.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_marco.h"
#include "tx_path_utils.h"
#include "tx_profile_utils.h"
#include "tx_sim_point.h"
#include "tx_spatial_query.h"
#include "tx_string_utils.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneReader)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(SceneLoader)

Simrec_SceneLoader::Simrec_SceneLoader() : _class_name(__func__) {}

Base::txBool Simrec_SceneLoader::IsValid() TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 状态为ready，并且source log文件存在
  return (+EStatus::eReady == Status()) && (exists(FilePath(_source)));
}

Simrec_SceneLoader::EgoType Simrec_SceneLoader::GetEgoType() const TX_NOEXCEPT {
  // 若trailer存在，直接返回truck
  if (FLAGS_ForcedTrailer) {
    return EgoType::eTruck;
  }

  // 若trailer不存在，获取ego类型
  const Base::txString strType = GetSimSimulationPlannerEgoType();
  if (Base::txString("truck") == strType) {
    return EgoType::eTruck;
  } else {
    return EgoType::eVehicle;
  }
}

Base::txString Simrec_SceneLoader::GetSimSimulationMapFile() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.mapfile) : (""));
}

Base::txFloat Simrec_SceneLoader::GetSimSimulationMapfileLongitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lon) : (0.0));
}

Base::txFloat Simrec_SceneLoader::GetSimSimulationMapfileLatitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.lat) : (0.0));
}

Base::txFloat Simrec_SceneLoader::GetSimSimulationMapfileAltitude() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_mapfile.alt) : (FLAGS_default_altitude));
}

Base::txString Simrec_SceneLoader::GetSimSimulationTraffic() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->traffic_file_path) : (""));
}

// 获取仿真场景中规划路径的起点
Base::txBool Simrec_SceneLoader::GetSimSimulationPlannerRouteStart(std::tuple<Base::txFloat, Base::txFloat> &res) const
    TX_NOEXCEPT {
  // 获取仿真场景中规划路径的起点字符串
  Base::txString strStart = (m_DataSource_Scene->_planner.route.start);
  std::vector<Base::txString> results;
  // 使用逗号分隔起点字符串，并将结果存储在结果向量中
  boost::algorithm::split(results, strStart, boost::is_any_of(","));
  try {
    // 将结果向量中的两个元素转换为浮点数，并存储在元组中
    res = std::make_tuple<Base::txFloat, Base::txFloat>(std::stod(results[0]), std::stod(results[1]));
  } catch (const std::invalid_argument &) {
    // 如果转换失败，记录警告日志并返回false
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  } catch (const std::out_of_range &) {
    // 如果转换失败，记录警告日志并返回false
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  } catch (...) {
    // 如果转换失败，记录警告日志并返回false
    LOG(WARNING) << "GetSimSimulationPlannerRouteStart "
                 << TX_VARS_NAME(ErrorContent, m_DataSource_Scene->_planner.route.start);
    return false;
  }
  return true;
}

// 获取地图管理器初始化参数
Base::txBool Simrec_SceneLoader::GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t &refParams)
    TX_NOEXCEPT {
  // 设置地图管理器初始化参数的交通文件路径
  refParams.strTrafficFilePath = GetSimSimulationTraffic(); /*simulation.traffic,*/
  // 设置地图管理器初始化参数的高精度地图文件路径
  refParams.strHdMapFilePath = GetSimSimulationMapFile(); /*simData.scene.c_str(),*/

  LogInfo << "Map File From .sim : " << refParams.strHdMapFilePath;
  using namespace Utils;
  using namespace boost::filesystem;
  // 获取当前路径
  FilePath path_CurrentPath = FilePath(_source).parent_path();
  // 将地图文件路径添加到当前路径
  path_CurrentPath.append(refParams.strHdMapFilePath);
  // 更新地图文件路径
  refParams.strHdMapFilePath = path_CurrentPath.string();
  LogInfo << "Map File Modify .sim : " << refParams.strHdMapFilePath;

  // 设置地图管理器初始化参数的场景原点GPS经度，纬度，高度
  __Lon__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLongitude();
  __Lat__(refParams.SceneOriginGPS) = GetSimSimulationMapfileLatitude();
  __Alt__(refParams.SceneOriginGPS) = GetSimSimulationMapfileAltitude();

  return true;
}

Base::txBool Simrec_SceneLoader::Release() TX_NOEXCEPT {
  m_DataSource_Scene = nullptr;
  trafficRecords_.Clear();
  _simrec_ego_start_timestamp_ms = 0.0;
  _simrec_ego_end_timestamp_ms = 0.0;
  mSpaceTimeCarListWithSameId.clear();
  mSpaceTimeEgoPtr = nullptr;
  m_map_rid2routePtr.clear();
  m_route_base = 1;
  return true;
}

Base::txBool Simrec_SceneLoader::FrameData2ObjectData(
    const Base::txFloat time_diff_, std::map<Base::txFloat, sim_msg::Location> &locations_,
    std::unordered_map<Base::txSysId, std::map<Base::txFloat, sim_msg::Car>> &cars_) TX_NOEXCEPT {
  cars_.clear();
  // 遍历仿真场景中的交通记录
  for (const auto &traffic : trafficRecords_.traffic_record()) {
    // 遍历交通记录中的车辆信息
    for (int i = 0; i < traffic.cars_size(); ++i) {
      const auto &car = traffic.cars(i);
      LogInfo << "car " << car.id() << " time: " << (car.t() - time_diff_);
      // 将车辆信息添加到车辆信息映射中
      cars_[car.id()][car.t() - time_diff_].CopyFrom(car);
    }
  }
  // 清空位置信息
  locations_.clear();
  // 遍历仿真场景中的位置信息
  for (const auto &location : trafficRecords_.ego_location()) {
    LogInfo << "location time: " << TX_VARS(location.t()) << TX_VARS(location.t() - time_diff_);
    // 将位置信息添加到位置信息映射中
    locations_[location.t() - time_diff_].CopyFrom(location);
  }
  LogInfo << TX_VARS(locations_.size()) << TX_VARS(cars_.size());
  // 如果位置信息和车辆信息非空，则返回true，否则返回false
  return _NonEmpty_(locations_) && _NonEmpty_(cars_);
}

Base::txBool Simrec_SceneLoader::Load(const txString &strFilePath, const txString &strSimRecFilePath,
                                      const txString &) TX_NOEXCEPT {
  // Base::txBool Simrec_SceneLoader::Load(const txString &strFilePath) TX_NOEXCEPT
  using namespace Utils;
  using namespace boost::filesystem;
  _source = strFilePath;
  // 首先释放资源
  Release();
  // 文件存在
  if (exists(FilePath(_source))) {
    _status = EStatus::eLoading;

    try {
      using namespace boost::filesystem;
      // 获取文件上一级目录
      FilePath path_CurrentPath = FilePath(_source).parent_path();

      // 拼接catalog_vehicle路径
      FilePath veh_cata_path = path_CurrentPath;
      veh_cata_path.append(FLAGS_Catalog_VehicleCatalog);

      // 拼接catalog_pedestrian路径
      FilePath ped_cata_path = path_CurrentPath;
      ped_cata_path.append(FLAGS_Catalog_PedestrianCatalog);

      // 拼接catalog_miscobject路径
      FilePath obs_cata_path = path_CurrentPath;
      obs_cata_path.append(FLAGS_Catalog_MiscObjectCatalog);

      // 初始化catalog缓存
#if USE_CustomModelImport
      FilePath veh_sys_cata_path = path_CurrentPath;
      veh_sys_cata_path.append(FLAGS_Catalog_Sys_VehicleCatalog);

      FilePath ped_sys_cata_path = path_CurrentPath;
      ped_sys_cata_path.append(FLAGS_Catalog_Sys_PedestrianCatalog);

      FilePath obs_sys_cata_path = path_CurrentPath;
      obs_sys_cata_path.append(FLAGS_Catalog_Sys_MiscObjectCatalog);
      if (CallSucc(Base::CatalogCache::Initialize(veh_cata_path.string(), ped_cata_path.string(),
                                                  obs_cata_path.string(), veh_sys_cata_path.string(),
                                                  ped_sys_cata_path.string(), obs_sys_cata_path.string()))) {
#else  /*USE_CustomModelImport*/
      if (CallSucc(
              Base::CatalogCache::Initialize(veh_cata_path.string(), ped_cata_path.string(), obs_cata_path.string()))) {
#endif /*USE_CustomModelImport*/
        LOG(INFO) << "Base::CatalogCache::Initialize() success.";
        Base::CatalogCache::ShowCacheInfo();
      } else {
        LOG(WARNING) << "Base::CatalogCache::Initialize() failure.";
      }
    } catch (...) {
      LogWarn << "Loading Catalog File : " << _source << " Except.";
    }

    try {
      // 加载sim log场景
      m_DataSource_Scene = SceneLoader::Sim::load_scene_sim(_source);
      LogInfo << "Loading TAD Scene File : " << _source;
      LogInfo << (*m_DataSource_Scene);
    } catch (...) {
      LogWarn << "Loading TAD Scene File : " << _source << " Except.";
      _status = EStatus::eError;
      return false;
    }

    using namespace boost::filesystem;
    /*FilePath path_CurrentPath = FilePath(_source).parent_path();
    path_CurrentPath.append((m_DataSource_Scene->traffic_file_path));
    std::string strSimRecFilePath = (path_CurrentPath).string();*/
    LogWarn << "Loading TAD Traffic File :" << strSimRecFilePath;
    LogWarn << "Scene File mapfile =" << (m_DataSource_Scene->_mapfile.mapfile);

    // pb文件加载
    std::fstream input(strSimRecFilePath, std::ios::in | std::ios::binary);
    if (!trafficRecords_.ParseFromIstream(&input)) {
      LOG(FATAL) << "Failed to parse dynamic data." << std::endl;
    } else {
      LogInfo << TX_VARS(trafficRecords_.ego_location_size()) << TX_VARS(trafficRecords_.traffic_record_size())
              << TX_VARS(trafficRecords_.ego_trajectory_size()) << TX_VARS(trafficRecords_.duration_ms());
      for (auto info : trafficRecords_.disengage_events()) {
        LogInfo << TX_VARS(info.timestamp_ms()) << TX_VARS(info.type());
      }
    }

    // 开始时间戳->第一个loc的时间戳
    if (trafficRecords_.ego_location_size() > 0) {
      _simrec_ego_start_timestamp_ms = trafficRecords_.ego_location(0).t();
    }

    // 结束时间戳->最后一个loc的时间戳
    if (trafficRecords_.ego_location_size() > 1) {
      _simrec_ego_end_timestamp_ms = trafficRecords_.ego_location(trafficRecords_.ego_location_size() - 1).t();
    } else {
      // 如果没有loc，则结束时间戳等于开始时间戳
      _simrec_ego_end_timestamp_ms = _simrec_ego_start_timestamp_ms;
    }

    // 遍历交通车辆
    if (trafficRecords_.traffic_record_size() > 0) {
      for (auto rIdx = trafficRecords_.traffic_record_size() - 1; rIdx >= 0; --rIdx) {
        const auto &refTraffic = trafficRecords_.traffic_record(rIdx);
        if (refTraffic.cars_size() > 0) {
          // 当前traffic的结束时间戳->第一个car的时间戳
          _simrec_traffic_end_timestamp_ms = refTraffic.cars(0).t();
          break;
        }
      }
    }

    // ego的轨迹数量判断
    if (trafficRecords_.ego_trajectory_size() > 0 && FLAGS_LogLevel_SceneLoader) {
      for (const auto &refTraj : trafficRecords_.ego_trajectory()) {
        // 打印日志
        std::ostringstream oss;
        oss << _StreamPrecision_ << TX_VARS_NAME(traj_time_stamp, refTraj.time_stamp());
        for (const auto &refTrajPt : refTraj.trajectory_line().point()) {
          oss << _StreamPrecision_ << "{" << TX_VARS_NAME(time_stamp, refTrajPt.t()) << TX_VARS_NAME(lon, refTrajPt.x())
              << TX_VARS_NAME(lat, refTrajPt.y())
              /*<< TX_VARS_NAME(heading, refTrajPt.theta()) << TX_VARS_NAME(speed, refTrajPt.v()) */
              << "},";
        }
        LogWarn << oss.str();
      }
    }
    LogWarn << _StreamPrecision_ << TX_VARS(SimrecStartTime()) << TX_VARS(SimrecEndTime()) << TX_VARS(SimrecDuration())
            << TX_VARS(SimrecEgoEndTime()) << TX_VARS(SimrecTrafficEndTime());

    // 置换状态
    _status = +EStatus::eReady;
    LogInfo << "Scene File lon =" << (m_DataSource_Scene->_mapfile.lon);
    LogInfo << "Scene File lat =" << (m_DataSource_Scene->_mapfile.lat);
    LogInfo << "Scene File alt =" << (m_DataSource_Scene->_mapfile.alt);
    LOG(INFO) << "Scene File mapfile =" << (m_DataSource_Scene->_mapfile.mapfile);
    return true;
  } else {
    LOG(FATAL) << "Scene File is not existing. File : " << _source;
    _status = EStatus::eError;
    return false;
  }
}

Base::txBool Simrec_SceneLoader::LoadObjects() TX_NOEXCEPT {
  // 仿真时间开始
  if (SimrecDuration() > 0.1) {
    std::unordered_map<Base::txSysId, std::map<Base::txFloat, sim_msg::Car>> cars_;
    std::map<Base::txFloat, sim_msg::Location> locations_;
    if (FrameData2ObjectData(SimrecStartTime(), locations_, cars_)) {
      // 生成位置信息和车辆信息
      if (GenerateTrajMgr(locations_, cars_)) {
        LogInfo << "GenerateTrajMgr success.";
        _status = +EStatus::eReady;
        return true;
      } else {
        LogWarn << "GenerateTrajMgr failure.";
        _status = EStatus::eError;
        return false;
      }
    } else {
      LogWarn << "FrameData2ObjectData failure.";
      _status = EStatus::eError;
      return false;
    }
  } else {
    LogWarn << "Scene duration failure. " << TX_VARS(SimrecStartTime()) << TX_VARS(SimrecEndTime())
            << TX_VARS(SimrecDuration());
    _status = EStatus::eError;
    return false;
  }
}

void Simrec_SceneLoader::SpliteCarTrajByLossFrame(
    const std::map<Base::txFloat, sim_msg::Car> &in_car_list,
    std::vector<std::map<Base::txFloat, sim_msg::Car>> &out_car_list) const TX_NOEXCEPT {
  // 清空输出车辆轨迹列表
  out_car_list.clear();
  // 设置丢失帧阈值
  const Base::txFloat loss_threshold = LoseFrameThreshold();
  // 如果输入车辆轨迹列表的大小大于1，则进行分割
  if (in_car_list.size() > 1) {
    // 初始化上一个子序列的起始迭代器
    auto last_start_itr = in_car_list.begin();
    // 初始化当前迭代器
    auto itr = std::next(in_car_list.begin());
    // 遍历输入车辆轨迹列表
    for (; itr != in_car_list.end(); ++itr) {
      // 获取前一个车辆轨迹的时间
      const Base::txFloat prev_time = (std::prev(itr)->first);
      // 获取当前车辆轨迹的时间
      const Base::txFloat curr_time = itr->first;
      // 如果当前车辆轨迹的时间与前一个车辆轨迹的时间之差大于丢失帧阈值，则进行分割
      if ((curr_time - prev_time) > loss_threshold) {
        LogWarn << "[split vehicle] " << TX_VARS_NAME(car_name, (*itr).second.id())
                << TX_VARS_NAME(time_gap, (curr_time - prev_time)) << TX_VARS(loss_threshold);
        /*splite seq*/
        std::map<Base::txFloat, sim_msg::Car> cur_sub_seq;
        cur_sub_seq.insert(last_start_itr, itr);
        out_car_list.emplace_back(std::move(cur_sub_seq));
        last_start_itr = itr;
      }
    }
    // 将最后一个子序列添加到输出车辆轨迹列表中
    std::map<Base::txFloat, sim_msg::Car> cur_sub_seq;
    cur_sub_seq.insert(last_start_itr, in_car_list.end());
    out_car_list.emplace_back(std::move(cur_sub_seq));

    // 检查输入车辆轨迹列表的大小是否等于输出车辆轨迹列表的大小
    const Base::txSize in_size = in_car_list.size();
    Base::txSize out_size = 0;
    for (const auto &refSeq : out_car_list) {
      out_size += refSeq.size();
    }
    LOG_IF(FATAL, in_size != out_size) << "loss input data. " << TX_VARS(in_size) << TX_VARS(out_size);
  } else {
    out_car_list.emplace_back(in_car_list);
  }
}

Base::txVec3 Simrec_SceneLoader::ComputeVehicle_Geometry(const txSpaceTimeCarList &vehicle_seq_list) const TX_NOEXCEPT {
  // 如果车辆序列列表非空，则返回车辆的几何信息
  if (_NonEmpty_(vehicle_seq_list)) {
    // 获取车辆序列列表中的第一个车辆的几何信息
    return Base::txVec3(vehicle_seq_list.front().raw_car_pb.length(), vehicle_seq_list.front().raw_car_pb.width(),
                        vehicle_seq_list.front().raw_car_pb.height());
  } else {
    return Base::txVec3(0, 0, 0);
  }
}

// 计算车辆路径上汽车之间的平均速度，单位为米/秒
// 参数: 汽车列表, 假设列表中的每个元素都代表一个车辆
// 返回: 该路径上所有车辆之间的平均速度
// 注意: 需要确保路径中的所有汽车在同一时刻沿着相同的方向行驶
Base::txFloat Simrec_SceneLoader::ComputeVehicleStartVelocity(const txSpaceTimeCarList &vehicle_seq_list) const
    TX_NOEXCEPT {
  // 检查列表是否为空
  if (_NonEmpty_(vehicle_seq_list)) {
    // 初始化总距离和总持续时间
    Base::txFloat sum_dist = 0.0;
    Base::txFloat sum_duration_ms = 0.0;

    // 遍历汽车列表
    for (Base::txInt idx = 1; idx < vehicle_seq_list.size(); ++idx) {
      // 获取当前汽车和下一辆汽车的坐标信息
      const auto &left_node = vehicle_seq_list[idx - 1];
      const auto &right_node = vehicle_seq_list[idx];

      // 将坐标信息转换为 WGS84 坐标系
      const hadmap::txPoint left_wgs84(left_node.raw_car_pb.x(), left_node.raw_car_pb.x(), 0.0);
      const hadmap::txPoint right_wgs84(right_node.raw_car_pb.x(), right_node.raw_car_pb.x(), 0.0);

      // 计算当前汽车与下一辆汽车之间的距离
      const Base::txFloat dist = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(left_wgs84, right_wgs84);

      // 对距离和持续时间累加
      sum_dist += dist;
      sum_duration_ms += (right_node.rsv_abs_time - left_node.rsv_abs_time);

      // 检查是否超过指定距离
      if (sum_dist > L2W_Motion_Dist()) {
        break;
      }
    }

    // 根据总持续时间计算平均速度
    if (sum_duration_ms > 0.0) {
      return (sum_dist / Utils::MillisecondToSecond(sum_duration_ms));
    } else {
      return 0.0;
    }
  } else {
    return 0.0;
  }
}

Base::Enums::VEHICLE_TYPE Simrec_SceneLoader::ComputeVehicle_Type(const txSpaceTimeCarList &vehicle_seq_list) const
    TX_NOEXCEPT {
  return Base::Enums::VEHICLE_TYPE::_from_integral(vehicle_seq_list.front().raw_car_pb.type());
}

Base::txFloat Simrec_SceneLoader::ComputeVehilce_MaxV(const txSpaceTimeCarList &vehicle_seq_list) const TX_NOEXCEPT {
  // 初始化速度之和
  Base::txFloat sumV = 0.0;
  // 遍历车辆序列列表
  for (const auto &item : vehicle_seq_list) {
    // 累加车辆的速度
    sumV += item.raw_car_pb.v();
  }
  // 计算平均速度并返回
  return sumV / vehicle_seq_list.size();
}

// 计算车辆的初始位置信息
void Simrec_SceneLoader::ComputeVehilce_InitLocationInfo(const txSpaceTimeCarList &vehicle_seq_list,
                                                         Base::txInt &refLaneID, Base::txFloat &refStartS,
                                                         Base::txFloat &refLOffset) const TX_NOEXCEPT {
  // 获取车辆序列列表中的第一个车辆的位置信息
  const auto frontItem = vehicle_seq_list.front();
  // 获取车辆的经度纬度
  const Base::txFloat start_lon = frontItem.raw_car_pb.x();
  const Base::txFloat start_lat = frontItem.raw_car_pb.y();

  // 将经纬度转换为WGS84坐标
  Coord::txWGS84 wgs84;
  wgs84.FromWGS84(start_lon, start_lat);
  // 初始化车道位置信息
  Base::Info_Lane_t laneLocInfo;
  // 获取车辆的车道位置信息
  Geometry::SpatialQuery::RTree2D_NearestReferenceLine::getInstance().Get_ST_Coord_By_Enu_Pt(wgs84, laneLocInfo,
                                                                                             refStartS, refLOffset);
  // 如果车辆在车道上，则设置车道ID
  if (laneLocInfo.IsOnLane()) {
    refLaneID = laneLocInfo.onLaneUid.laneId;
  } else {
    refLaneID = /*-1 * */ laneLocInfo.onLinkId_without_equal;
  }
}

// 获取路径数据
Simrec_SceneLoader::IRouteViewerPtr Simrec_SceneLoader::GetRouteData(Base::txSysId const id) TX_NOEXCEPT {
  // 如果路径ID在路径映射中，则返回对应的路径数据
  if (_Contain_(m_map_rid2routePtr, id)) {
    return m_map_rid2routePtr.at(id);
  } else {  // 如果路径ID不在路径映射中，则记录警告日志并返回空指针
    LogWarn << "can not find route " << id;
    return nullptr;
  }
}

Simrec_SceneLoader::IRouteViewerPtr Simrec_SceneLoader::GetEgoRouteData() TX_NOEXCEPT {
  // 如果 ego 位置列表中有数据
  if (trafficRecords_.ego_location_size() > 0) {
    // 初始化路线节点向量
    L2W_RouteViewer::control_path_node_vec cpv;

    // 遍历每个 ego 位置，并将其转换为路线节点
    for (const auto item : trafficRecords_.ego_location()) {
      // 初始化路线节点
      IRouteViewer::control_path_node cp;
      __Lon__(cp.waypoint) = item.position().x();
      __Lat__(cp.waypoint) = item.position().y();
      __Alt__(cp.waypoint) = item.position().z();
      /*cp.gear = +ControlPathGear::drive;*/
      cpv.emplace_back(std::move(cp));
    }

    // 创建新的路线视图
    L2W_RouteViewerPtr newRoutePtr = std::make_shared<L2W_RouteViewer>();
    newRoutePtr->Init(0, cpv);
    // 返回新的路线视图
    return newRoutePtr;
  } else {
    LogWarn << "ego location is empty. create ego route error.";
    return nullptr;
  }
}

SceneLoader::Traffic::Vehicle Simrec_SceneLoader::ComputeVehicle_Route(const txSpaceTimeCarList &vehicle_seq_list)
    TX_NOEXCEPT {
  SceneLoader::Traffic::Vehicle retVehicle;

  /*1. create route info*/
  const Base::txSysId curRouteId = m_route_base++;
  // 初始化路线节点向量
  L2W_RouteViewer::control_path_node_vec cpv;
  // 遍历车辆序列列表
  for (const auto item : vehicle_seq_list) {
    // 初始化路线节点
    IRouteViewer::control_path_node cp;
    // 设置路线经纬度
    __Lon__(cp.waypoint) = item.raw_car_pb.x();
    __Lat__(cp.waypoint) = item.raw_car_pb.y();
    __Alt__(cp.waypoint) = item.raw_car_pb.z();
    /*cp.gear = +ControlPathGear::drive;*/
    // 将路线节点添加到路线节点向量中
    cpv.emplace_back(std::move(cp));
  }

  // 初始化路径长度
  Base::txFloat route_len = 0.0;
  // 遍历路线节点向量
  for (Base::txSize idx = 1; idx < cpv.size(); ++idx) {
    // 计算当前路线节点与前一条路线节点之间的距离
    const Base::txFloat seg_len =
        Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(cpv[idx - 1].waypoint, cpv[idx].waypoint);
    route_len += seg_len;
  }
  // 如果路径长度大于阈值且路线节点数量大于1
  if (route_len > FLAGS_L2W_StopRouteLen && cpv.size() > 1) {
    // 计算起点和终点之间的距离
    const Base::txFloat start_end_len =
        Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(cpv.front().waypoint, cpv.back().waypoint);
    route_len = (route_len > start_end_len) ? (start_end_len) : (route_len);
  }

  // 创建新的路线视图
  L2W_RouteViewerPtr newRoutePtr = std::make_shared<L2W_RouteViewer>();
  // 初始化路线视图
  newRoutePtr->Init(curRouteId, cpv);
  // 将新的路线视图添加到路线映射中
  m_map_rid2routePtr[curRouteId] = newRoutePtr;

  /*2. create vehicle*/
  retVehicle.routeID = curRouteId;

  // 设置车辆的长宽高
  const Base::txVec3 vehicle_geometry = ComputeVehicle_Geometry(vehicle_seq_list);
  retVehicle.length = vehicle_geometry.x();
  retVehicle.width = vehicle_geometry.y();
  retVehicle.height = vehicle_geometry.z();

  // 计算车辆的开始时间 开始速度 激进程度 车辆类型等信息
  retVehicle.start_t = Utils::MillisecondToSecond(vehicle_seq_list.front().rsv_abs_time);
  retVehicle.start_v = ComputeVehicleStartVelocity(vehicle_seq_list); /*vehicle_seq_list.front().raw_car_pb.v();*/
  retVehicle.aggress = 0.5;
  retVehicle.vehicleType = ComputeVehicle_Type(vehicle_seq_list)._to_string();
  retVehicle.max_v = ComputeVehilce_MaxV(vehicle_seq_list) * 1.2;
  if (route_len > FLAGS_L2W_StopRouteLen) {
    // 设置车辆的最大速度
    retVehicle.max_v = FLAGS_vehicle_max_speed;
  }

  // 如果车辆的最大速度小于等于0
  if (retVehicle.max_v <= 0.0) {
    // 设置车辆的起始速度为0
    retVehicle.start_v = 0.0;
  }
  // 计算车辆的初始位置信息
  ComputeVehilce_InitLocationInfo(vehicle_seq_list, retVehicle.laneID, retVehicle.start_s, retVehicle.l_offset);
  // 设置车辆的开始距离 横向偏移 ID accid等
  retVehicle.start_s = 0.0;
  retVehicle.l_offset = 0.0;
  retVehicle.id = vehicle_seq_list.front().raw_car_pb.id();
  retVehicle.accID = 0;
  retVehicle.mergeID = 0;
  // retVehicle.vehicleType = (+Base::Enums::VEHICLE_TYPE::Sedan)._to_string();
  retVehicle.behavior = (+Base::Enums::VEHICLE_BEHAVIOR::eTadAI)._to_string();
  retVehicle.mergeTime = 0.0;
  retVehicle.offsetTime = 0.0;
  LOG(WARNING) << TX_VARS(retVehicle.id) << TX_VARS(retVehicle.max_v) << TX_VARS(route_len)
               << TX_VARS(FLAGS_L2W_StopRouteLen);
  return std::move(retVehicle);
}

Base::txBool Simrec_SceneLoader::GenerateTrajMgr(
    const std::map<Base::txFloat, sim_msg::Location> &locations_,
    const std::unordered_map<Base::txSysId, std::map<Base::txFloat, sim_msg::Car>> &cars_) TX_NOEXCEPT {
#if __TX_Mark__("processing location data.")
  using txSpaceTimeLocation = TrafficFlow::txTrajManagerBase::txSpaceTimeLocation;
  // 初始化txSpaceTimeLocationList类型的变量location_space_time_list
  TrafficFlow::txTrajManagerBase::txSpaceTimeLocationList location_space_time_list;
  // 遍历locations_
  for (const auto item : locations_) {
    // 将locations_中的数据添加到location_space_time_list中
    location_space_time_list.emplace_back(txSpaceTimeLocation(item.first, item.second));
  }
  // 如果启用位置插值
  if (LocationInterpolation()) {
    TX_MARK("smooth");
    // 创建一个新的TrafficFlow::txL2W_Location_SmoothAware_TrajMgr对象并将其赋值给mSpaceTimeEgoPtr
    mSpaceTimeEgoPtr = std::make_shared<TrafficFlow::txL2W_Location_SmoothAware_TrajMgr>();
  } else {
    TX_MARK("raw data replay");
    // 创建一个新的TrafficFlow::txL2W_Location_FrameAware_TrajMgr对象并将其赋值给mSpaceTimeEgoPtr
    mSpaceTimeEgoPtr = std::make_shared<TrafficFlow::txL2W_Location_FrameAware_TrajMgr>();
  }
  // 将location_space_time_list中的数据添加到mSpaceTimeEgoPtr中
  mSpaceTimeEgoPtr->add_new_pos(location_space_time_list);
#endif /*__TX_Mark__("processing location data.")*/

#if __TX_Mark__("processing traffic data.")
  mSpaceTimeCarListWithSameId.clear();

  using txSpaceTimeCar = TrafficFlow::txTrajManagerBase::txSpaceTimeCar;
  // 初始化txSpaceTimeCarList类型的变量car_space_time_list
  TrafficFlow::txTrajManagerBase::txSpaceTimeCarList car_space_time_list;
  // 遍历分割后的子列表
  for (const auto &cur_car_list : cars_) {
    std::vector<std::map<Base::txFloat, sim_msg::Car>> split_car_list_without_loss_frame;
    SpliteCarTrajByLossFrame(cur_car_list.second, split_car_list_without_loss_frame);
    for (const auto &ref_sub_seq : split_car_list_without_loss_frame) {
      /*LogWarn << TX_VARS_NAME(car_id, cur_car_list.first) << TX_VARS_NAME(all_size, cur_car_list.second.size())
          << TX_VARS_NAME(sub_list_size, ref_sub_seq.size());*/
      // 如果子列表非空
      if (_NonEmpty_(ref_sub_seq)) {
        TrafficFlow::txTrajManagerBase::txSpaceTimeCarList car_space_time_list;
        // 将子列表中的数据添加到car_space_time_list中
        for (const auto &item : ref_sub_seq) {
          car_space_time_list.emplace_back(txSpaceTimeCar(item.first, item.second));
        }
        TrafficFlow::txTrajManagerBasePtr cur_vehicle_traj_mgr_ptr = nullptr;
        // 如果启用车辆插值
        if (VehicleInterpolation()) {
          cur_vehicle_traj_mgr_ptr = std::make_shared<TrafficFlow::txL2W_Car_SmoothAware_TrajMgr>();
        } else {
          cur_vehicle_traj_mgr_ptr = std::make_shared<TrafficFlow::txL2W_Car_FrameAware_TrajMgr>();
        }
        // 将car_space_time_list中的数据添加到cur_vehicle_traj_mgr_ptr中
        cur_vehicle_traj_mgr_ptr->add_new_pos(car_space_time_list);
        SceneLoader::Traffic::Vehicle vehicleInfo = ComputeVehicle_Route(car_space_time_list);
        LogInfo << vehicleInfo;
        // 将车辆信息和cur_vehicle_traj_mgr_ptr添加到mSpaceTimeCarListWithSameId中
        mSpaceTimeCarListWithSameId.emplace_back(std::make_tuple(vehicleInfo, cur_vehicle_traj_mgr_ptr));
      }
    }
  }
#endif /*__TX_Mark__("processing traffic data.")*/
  return true;
}

#if __TX_Mark__("Simrec_SceneLoader::L2W_EgoVehiclesViewer")
void Simrec_SceneLoader::L2W_EgoVehiclesViewer::Init(sim_msg::Location const &srcEgo) TX_NOEXCEPT {
  m_ego = srcEgo;
  m_inited = true;
}

Base::txSysId Simrec_SceneLoader::L2W_EgoVehiclesViewer::id() const TX_NOEXCEPT { return FLAGS_Default_EgoVehicle_Id; }
Base::txBool Simrec_SceneLoader::L2W_EgoVehiclesViewer::IsIdValid() const TX_NOEXCEPT { return true; }
Base::txSysId Simrec_SceneLoader::L2W_EgoVehiclesViewer::routeID() const TX_NOEXCEPT { return -1; }
Base::txBool Simrec_SceneLoader::L2W_EgoVehiclesViewer::IsRouteIdValid() const TX_NOEXCEPT { return false; }
Base::txLaneID Simrec_SceneLoader::L2W_EgoVehiclesViewer::laneID() const TX_NOEXCEPT { return 0; }
Base::txBool Simrec_SceneLoader::L2W_EgoVehiclesViewer::IsLaneIdValid() const TX_NOEXCEPT { return true; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::start_s() const TX_NOEXCEPT { return 0.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::start_t() const TX_NOEXCEPT { return 0.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::start_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::max_v() const TX_NOEXCEPT { return 16.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::l_offset() const TX_NOEXCEPT { return 0.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::lon() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().x()) : (0.0));
}
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::lat() const TX_NOEXCEPT {
  return ((m_inited) ? (m_ego.position().y()) : (0.0));
}
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::alt() const TX_NOEXCEPT { return 0.0; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_EGO_Length; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_EGO_Width; }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_EGO_Height; }
Base::txSysId Simrec_SceneLoader::L2W_EgoVehiclesViewer::accID() const TX_NOEXCEPT { return -1; }
Base::txBool Simrec_SceneLoader::L2W_EgoVehiclesViewer::IsAccIdValid() const TX_NOEXCEPT { return false; }
Base::txSysId Simrec_SceneLoader::L2W_EgoVehiclesViewer::mergeID() const TX_NOEXCEPT { return -1; }
Base::txBool Simrec_SceneLoader::L2W_EgoVehiclesViewer::IsMergeIdValid() const TX_NOEXCEPT { return false; }
Base::txString Simrec_SceneLoader::L2W_EgoVehiclesViewer::behavior() const TX_NOEXCEPT { return Base::txString("Ego"); }
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::aggress() const TX_NOEXCEPT { return 0.5; }
Base::txString Simrec_SceneLoader::L2W_EgoVehiclesViewer::vehicleType() const TX_NOEXCEPT {
  return Base::txString("SUV");
}
Base::txSysId Simrec_SceneLoader::L2W_EgoVehiclesViewer::follow() const TX_NOEXCEPT {
  return FLAGS_Topological_Root_Id;
}
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::mergeTime() const TX_NOEXCEPT {
  return FLAGS_LaneChangeDuration;
}
Base::txFloat Simrec_SceneLoader::L2W_EgoVehiclesViewer::offsetTime() const TX_NOEXCEPT {
  return FLAGS_OffsetChangeDuration;
}
#endif  // __TX_Mark__("TAD_SceneLoader::TAD_EgoVehiclesViewer")

#if __TX_Mark__("TAD_TrailerVehiclesViewer")
Base::txSysId Simrec_SceneLoader::L2W_TrailerVehiclesViewer::id() const TX_NOEXCEPT {
  return FLAGS_Default_TrailerVehicle_Id;
}
Base::txFloat Simrec_SceneLoader::L2W_TrailerVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Trailer_Length; }
Base::txFloat Simrec_SceneLoader::L2W_TrailerVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Trailer_Width; }
Base::txFloat Simrec_SceneLoader::L2W_TrailerVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Trailer_Height; }
#endif /*__TX_Mark__("TAD_TrailerVehiclesViewer")*/

#if __TX_Mark__("TAD_TruckVehiclesViewer")
Base::txSysId Simrec_SceneLoader::L2W_TruckVehiclesViewer::id() const TX_NOEXCEPT {
  return FLAGS_Default_EgoVehicle_Id;
}
Base::txFloat Simrec_SceneLoader::L2W_TruckVehiclesViewer::length() const TX_NOEXCEPT { return FLAGS_Truck_Length; }
Base::txFloat Simrec_SceneLoader::L2W_TruckVehiclesViewer::width() const TX_NOEXCEPT { return FLAGS_Truck_Width; }
Base::txFloat Simrec_SceneLoader::L2W_TruckVehiclesViewer::height() const TX_NOEXCEPT { return FLAGS_Truck_Height; }
#endif /*__TX_Mark__("TAD_TruckVehiclesViewer")*/

#if __TX_Mark__("L2W_RouteViewer")

void Simrec_SceneLoader::L2W_RouteViewer::Init(Base::txSysId _id, const control_path_node_vec &cp) TX_NOEXCEPT {
  // 设置路线ID
  m_route_id = _id;
  // 如果控制路径非空
  if (_NonEmpty_(cp)) {
    m_control_path = cp;
    m_inited = true;
  } else {
    m_inited = false;
  }
}

#endif /*__TX_Mark__("L2W_RouteViewer")*/
Base::ISceneLoader::IVehiclesViewerPtr Simrec_SceneLoader::GetEgoData() TX_NOEXCEPT {
#if Use_TruckEgo
  // 如果trafficRecords_中的ego_location数量大于0
  if (trafficRecords_.ego_location_size() > 0) {
    // 设置m_EgoData为trafficRecords_中的第一个ego_location
    m_EgoData = trafficRecords_.ego_location(0);
    // 如果Ego类型为车辆
    if (+EgoType::eVehicle == GetEgoType()) {
      // 创建一个新的L2W_EgoVehiclesViewer对象并将其赋值给retPtr
      L2W_EgoVehiclesViewerPtr retPtr = std::make_shared<L2W_EgoVehiclesViewer>();
      retPtr->Init(m_EgoData);
      return retPtr;
    } else {
      // 创建一个新的L2W_EgoVehiclesViewer对象并将其赋值给retPtr
      L2W_TruckVehiclesViewerPtr retPtr = std::make_shared<L2W_TruckVehiclesViewer>();
      retPtr->Init(m_EgoData);
      return retPtr;
    }
  } else {
    return nullptr;
  }
#endif
}

Base::txBool Simrec_SceneLoader::GetRoutingInfo(sim_msg::Location &refEgoData) TX_NOEXCEPT {
  // 如果trafficRecords_中的ego_location数量大于0
  if (trafficRecords_.ego_location_size() > 0) {
    // 将trafficRecords_中的第一个ego_location复制到refEgoData
    refEgoData.CopyFrom(trafficRecords_.ego_location(0));
    return true;
  } else {
    return false;
  }
}

Base::ISceneLoader::IVehiclesViewerPtr Simrec_SceneLoader::GetEgoTrailerData() TX_NOEXCEPT {
  // 如果Ego类型为车辆
  if (+EgoType::eVehicle == GetEgoType()) {
    return nullptr;
  } else {
    // 创建一个新的L2W_TrailerVehiclesViewer对象并将其赋值给retPtr
    L2W_TrailerVehiclesViewerPtr retPtr = std::make_shared<L2W_TrailerVehiclesViewer>();
    retPtr->Init(m_EgoData);
    return retPtr;
  }
}

Base::txBool Simrec_SceneLoader::InterpEgoLocation(Base::TimeParamManager const &timeMgr,
                                                   sim_msg::Location &refEgoLocation) TX_NOEXCEPT {
  // 如果mSpaceTimeEgoPtr非空
  if (NonNull_Pointer(mSpaceTimeEgoPtr)) {
    // 计算Ego位置并将结果存储在refEgoLocation中
    return mSpaceTimeEgoPtr->compute_pos(timeMgr, refEgoLocation);
  } else {
    LogWarn << "ego trajectory is null.";
    return false;
  }
  return false;
}

// 获取所有车辆数据
std::unordered_map<Base::txSysId, Simrec_SceneLoader::IVehiclesViewerPtr> Simrec_SceneLoader::GetAllVehicleData()
    TX_NOEXCEPT {
  // 创建一个用于存储车辆数据的unordered_map对象
  std::unordered_map<Base::txSysId, Simrec_SceneLoader::IVehiclesViewerPtr> retMap;
  // 遍历mSpaceTimeCarListWithSameId
  for (auto &refTuple : mSpaceTimeCarListWithSameId) {
    // 创建一个新的L2W_VehiclesViewer对象并将其赋值给vehPtr
    L2W_VehiclesViewerPtr vehPtr = std::make_shared<L2W_VehiclesViewer>();
    if (NonNull_Pointer(vehPtr)) {
      // 初始化vehPtr
      vehPtr->Init(std::get<0>(refTuple), std::get<1>(refTuple));
      const Base::txSize cnt = retMap.size();
      retMap[cnt] = vehPtr;
    }
  }
  return retMap;
}

#if __TX_Mark__("L2W_VehiclesViewer")

Base::txSysId Simrec_SceneLoader::L2W_VehiclesViewer::routeID() const TX_NOEXCEPT { return m_vehicle.routeID; }
Base::txLaneID Simrec_SceneLoader::L2W_VehiclesViewer::laneID() const TX_NOEXCEPT { return m_vehicle.laneID; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::start_s() const TX_NOEXCEPT { return m_vehicle.start_s; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::start_t() const TX_NOEXCEPT { return m_vehicle.start_t; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::start_v() const TX_NOEXCEPT { return m_vehicle.start_v; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::max_v() const TX_NOEXCEPT { return m_vehicle.max_v; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::l_offset() const TX_NOEXCEPT { return m_vehicle.l_offset; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::length() const TX_NOEXCEPT { return m_vehicle.length; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::width() const TX_NOEXCEPT { return m_vehicle.width; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::height() const TX_NOEXCEPT { return m_vehicle.height; }
Base::txFloat Simrec_SceneLoader::L2W_VehiclesViewer::aggress() const TX_NOEXCEPT { return m_vehicle.aggress; }
Base::txString Simrec_SceneLoader::L2W_VehiclesViewer::vehicleType() const TX_NOEXCEPT { return m_vehicle.vehicleType; }

#endif /*__TX_Mark__("L2W_VehiclesViewer")*/

Base::txString Simrec_SceneLoader::GetSimSimulationPlannerEgoType() const TX_NOEXCEPT {
  return ((NonNull_Pointer(m_DataSource_Scene)) ? (m_DataSource_Scene->_planner.ego.type) : (""));
}

// 获取指定时间戳的轨迹
sim_msg::Trajectory Simrec_SceneLoader::GetTrajectory(const txFloat time_stamp_s) const TX_NOEXCEPT {
  // 创建一个空的Trajectory对象
  sim_msg::Trajectory ret_traj;
  const txInt ego_trajectory_size = trafficRecords_.ego_trajectory_size();
  for (txInt idx = 1; idx < ego_trajectory_size; ++idx) {
    // 获取当前轨迹的时间戳并将其赋值给left_time_stamp_s
    const txFloat left_time_stamp_s = trafficRecords_.ego_trajectory(idx - 1).time_stamp();
    // 获取下一个轨迹的时间戳并将其赋值给right_time_stamp_s
    const txFloat right_time_stamp_s = trafficRecords_.ego_trajectory(idx).time_stamp();
    // 如果left_time_stamp_s小于right_time_stamp_s且time_stamp_s在两者之间
    if (left_time_stamp_s < right_time_stamp_s && left_time_stamp_s <= time_stamp_s &&
        time_stamp_s < right_time_stamp_s) {
      LogInfo << TX_VARS(left_time_stamp_s) << TX_VARS(time_stamp_s) << TX_VARS(right_time_stamp_s);
      // 将当前轨迹复制到ret_traj
      ret_traj.CopyFrom(trafficRecords_.ego_trajectory(idx - 1).trajectory_line());
      break;
    }
  }
  return std::move(ret_traj);
}

TX_NAMESPACE_CLOSE(SceneLoader)
