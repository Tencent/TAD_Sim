// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_template.h"
#include "HdMap/tx_hashed_road.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_application.h"
#include "tx_hadmap_utils.h"
#include "tx_path_utils.h"
#include "tx_spatial_query.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_gflags.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Cloud)
#define SceneLoaderLogInfo LOG_IF(INFO, FLAGS_LogLevel_Cloud)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

Base::txBool ParseMapInitParams(const Scene::InitInfoWrap& inputeParams,
                                HdMap::HadmapCacheConCurrent::InitParams_t& refParams) TX_NOEXCEPT {
  refParams.op_map_range = boost::none;
  Base::map_range_t range;
  const auto regions_range = inputeParams.regions_range();
  if (false && _NonEmpty_(regions_range)) {
#if 1
    Base::txVec3 _gps_max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    Base::txVec3 _gps_min(FLT_MAX, FLT_MAX, FLT_MAX);
#else
    hadmap::txPoint _gps_max = inputeParams.get_map_manager_range_info().map_range.bottom_left;
    Base::txVec3 _enu_max(__Lon__(_gps_max), __Lat__(_gps_max), __Alt__(_gps_max));
    hadmap::txPoint _gps_min = inputeParams.get_map_manager_range_info().map_range.top_right;
    TX_MARK("reverse");
    Base::txVec3 _enu_min(__Lon__(_gps_min), __Lat__(_gps_max), __Alt__(_gps_min));
#endif
    for (const auto& curRange : regions_range) {
      Base::txVec3 _cur_pt_bottom_left(__Lon__(curRange.bottom_left), __Lat__(curRange.bottom_left),
                                       __Alt__(curRange.bottom_left));
      std::tie(_gps_max, _gps_min) = Utils::MaxMin::join(_gps_max, _gps_min, _cur_pt_bottom_left);

      Base::txVec3 _cur_pt_top_right(__Lon__(curRange.top_right), __Lat__(curRange.top_right),
                                     __Alt__(curRange.top_right));
      std::tie(_gps_max, _gps_min) = Utils::MaxMin::join(_gps_max, _gps_min, _cur_pt_top_right);
    }

    const Base::txVec3 centerPt = (_gps_max + _gps_min) / 2.0;
    range.center = hadmap::txPoint(centerPt.x(), centerPt.y(), centerPt.z());

    const Base::txFloat x_step = (_gps_max.x() - _gps_min.x()) / 20.0;
    const Base::txFloat y_step = (_gps_max.y() - _gps_min.y()) / 20.0;
    range.bottom_left = hadmap::txPoint(_gps_min.x() - x_step, _gps_min.y() - y_step, _gps_min.z());
    range.top_right = hadmap::txPoint(_gps_max.x() + x_step, _gps_max.y() + y_step, _gps_max.z());

    refParams.SceneOriginGPS = range.center;

    LOG(INFO) << TX_VARS_NAME(_gps_max, Utils::ToString(_gps_max)) << TX_VARS_NAME(_gps_min, Utils::ToString(_gps_min))
              << TX_VARS_NAME(centerPt, Utils::ToString(centerPt))
              << TX_VARS_NAME(range.bottom_left, Utils::ToString(range.bottom_left))
              << TX_VARS_NAME(range.top_right, Utils::ToString(range.top_right));
  } else {
    range = inputeParams.get_map_manager_range_info().map_range;
    refParams.SceneOriginGPS = inputeParams.get_scene_origin_pgs();
  }
  refParams.op_map_range = range;

  refParams.strTrafficFilePath = inputeParams.get_traffic_xml_path();

  refParams.strHdMapFilePath = inputeParams.get_scene_hdmap_path();
  refParams.strHadmapFilter = inputeParams.get_scene_hdmap_filter_path();
  return true;
}

Base::txString Traffic2EventJson(const Base::txString& traffic_xml_path) TX_NOEXCEPT {
  using namespace boost::filesystem;
  Utils::FilePath xml_abs_path = Utils::FilePath(traffic_xml_path);
  Utils::FilePath path_CurrentPath = xml_abs_path.parent_path();
  const auto xml_without_ext = xml_abs_path.filename().stem().string();
  path_CurrentPath.append(xml_without_ext + ".json");
  LogWarn << TX_VARS(traffic_xml_path) << TX_VARS(path_CurrentPath);
  return path_CurrentPath.string();
}

Base::TimeParamManager PluginTemplate::MakeTimeMgrAbsTimeInMillisecondSecond(const uint64_t abs_time_ms) TX_NOEXCEPT {
  /*[simulation_time] :  abs = 0.02, relative = 0.02, pass = 0.02, time_stamp = 20 time_stamp = 20,*/
  const Base::txFloat relativeTimeInSec = inputeParams.get_time_step_in_s();
  m_passTime += relativeTimeInSec;
  const Base::txFloat abs_time_s = Utils::MillisecondToSecond(abs_time_ms);
  const Base::txFloat time_stamp = abs_time_ms;
  m_cloud_time_mgr =
      Base::TimeParamManager(abs_time_s, inputeParams.get_time_step_in_s() /*0.1*/, m_passTime, abs_time_ms);
  cur_AbsTime = m_cloud_time_mgr.AbsTime();
  return GetTimeMgr();
}

Base::TimeParamManager PluginTemplate::MakeTimeMgrRelativeTimeInSecond(const Base::txFloat relativeTimeInSec)
    TX_NOEXCEPT {
  const Base::txFloat time_stamp = Utils::SecondToMillisecond(relativeTimeInSec) + m_lastStepTime;
  m_passTime += relativeTimeInSec;
  m_cloud_time_mgr =
      Base::TimeParamManager(Utils::MillisecondToSecond(time_stamp), relativeTimeInSec, m_passTime, time_stamp);
  // LOG(WARNING) << "[simulation_time] : " << timeMgr.str() << TX_VARS(time_stamp);
  cur_AbsTime = m_cloud_time_mgr.AbsTime();
  return GetTimeMgr();
}

void PluginTemplate::CreateEnv(const Base::txBool res_ego, sim_msg::Location& loc) TX_NOEXCEPT {
  LogInfo << "call " << __func__ << TX_COND(res_ego);
  ResetVarsPlugin();

  const Base::txString traffic_xml_path = inputeParams.get_traffic_xml_path();
  Base::ISceneLoaderPtr loader = CreateSceneLoader();
  LogInfo << loader->ClassName();

  const auto scene_hdmap_path = inputeParams.get_scene_hdmap_path();

  if (loader->Load(traffic_xml_path)) {
    if (NonNull_Pointer(CloudTrafficSystemPtr())) {
      if (CallSucc(res_ego)) {
        loader->SetEgoData(loc);
        loader->SetVehicleGeometory(0, vehGeomList);
        CloudTrafficSystemPtr()->RegEgo() = true;
      } else {
        CloudTrafficSystemPtr()->RegEgo() = false;
      }
      HdMap::HadmapCacheConCurrent::InitParams_t initParams;
      ParseMapInitParams(inputeParams, initParams);
      if (CallFail(IsResetHdMapCache()) || HdMap::HadmapCacheConCurrent::Initialize(initParams)) {
        const auto& map_range = inputeParams.get_map_manager_range_info();
        if (CloudTrafficSystemPtr()->Initialize(loader, inputeParams.get_map_manager_range_info().map_range)) {
          if (isVirtualCity()) {
            const Base::txString event_json_abs_path = Traffic2EventJson(traffic_xml_path);
            if (exists(boost::filesystem::path(event_json_abs_path))) {
              std::ifstream in_json(event_json_abs_path);
              std::stringstream ss;
              ss << in_json.rdbuf();
              CloudTrafficSystemPtr()->ResetForecastSimScene(its::txStatusRecord(), sim_msg::TrafficRecords(),
                                                             ss.str());
            } else {
              LOG(INFO) << "[event_json][not find event file]" << event_json_abs_path;
            }
          } else if (isParallelSimulation()) {
#if __pingsn__
            its::txSimStatus simStat;
            std::string init_state_string = inputeParams.get_ps_init_state_string();
            if (CallSucc(simStat.ParseFromString(init_state_string))) {
              LOG(INFO) << "parallel simulation simStatus parse success.";
            } else {
              LOG(WARNING) << "parallel simulation simStatus parse failure."
                           << TX_VARS(inputeParams.get_ps_init_state_string());
            }
            simStat.set_time(0);
            loadVehicles(simStat);
            ParallelSimulationSystemPtr()->CreateTrafficLightElement(inputeParams.get_ps_traffic_light_string());
#endif /*__pingsn__*/
          }
          LOG(INFO) << " Traffic Flow Simulation System Init Success.";
        } else {
          LOG(WARNING) << " TrafficElementSystem Init Failure.";
        }
      } else {
        LogWarn << "MapManager Init Failure.";
      }
    } else {
      LogWarn << "TrafficElementSystem is Null.";
    }
  } else {
    LOG(WARNING) << "loader->Load(params.scenariopath()) return false.";
  }
}

void PluginTemplate::Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
    SimulationTraffic(timeMgr);
  } else {
    LOG(WARNING) << "m_TrafficElementSystemPtr is not initialized, Send 0 elements.";
  }
}

void PluginTemplate::SimulationTraffic(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  TX_MARK("move in FillingSpatialQuery"); /*HdMap::HashedRoadCacheConCurrent::UngisterAllElements();*/
  m_TrafficElementSystemPtr->FillingSpatialQuery(/*ClearSpatialQueryElementInfo + UngisterAllElements*/);

  const Base::txBool res = CloudTrafficSystemPtr()->Update(timeMgr);
  m_lastStepTime = timeMgr.TimeStamp();
}

Base::txBool PluginTemplate::initialize_l2w_sim(const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                                const its::txStatusRecord& status_pb_record /* size = 1 */,
                                                const std::string _event_json) TX_NOEXCEPT {
  m_lastStepTime = 0.0;
  Base::ITrafficElement::ResetSysId();
  m_passTime = 0.0;
#if USE_RTree
  Geometry::SpatialQuery::ClearSpatialQueryElementInfo();
  Geometry::SpatialQuery::ClearSpatialQuerySignalInfo();
#endif
  HdMap::HadmapCacheConCurrent::ClearAddDstRefuseLaneChange();
  m_TrafficElementSystemPtr->ResetForecastSimScene(status_pb_record, traffic_pb_record, _event_json);
  return true;
}

Base::txBool PluginTemplate::inject_traffic_event(const std::string _event_json) TX_NOEXCEPT {
  if (NonNull_Pointer(CloudTrafficSystemPtr())) {
    return CloudTrafficSystemPtr()->InjectTrafficEvent(_event_json);
  } else {
    return false;
  }
}

Base::ISceneLoaderPtr PluginTemplate::CreateSceneLoader() const TX_NOEXCEPT {
  return std::make_shared<SceneLoader::TAD_Cloud_SceneLoader>();
}

#if __pingsn__
Base::txBool PluginTemplate::loadVehicles(const its::txSimStatus& startStatus) TX_NOEXCEPT {
  for (auto& vehicle : startStatus.vehicle()) {
    LOG(WARNING) << " loadVehicle:" << vehicle.DebugString();
    if (vehicle.route().road_id_size() == 0) continue;
    trafficRecord_.RecordInputVehicle(vehicle);

    double vehicle_z = 1.5;

    addVehicleToElementManager(
        vehicle.vehicle_id(), vehicle.vehicle_type(), vehicle.lane_id(),
        {vehicle.route().road_id().begin() + vehicle.road_idx(), vehicle.route().road_id().end()}, vehicle.speed(),
        vehicle.lng(), vehicle.lat(), vehicle_z);
  }
  return true;
}

Base::txBool PluginTemplate::addVehicleToElementManager(Base::txSysId vehId, Base::txInt vehType,
                                                        Base::txLaneID vehicleStartLaneId,
                                                        const std::vector<int64_t>& roadIds, Base::txFloat startV,
                                                        double roadStartPointX, double roadStartPointY,
                                                        double roadStartPointZ) TX_NOEXCEPT {
  // LOG(WARNING) << _FootPrint_ << " addVehicleToElementManager:" << vehId;
  hadmap::txPoint roadStartPoint(roadStartPointX, roadStartPointY, roadStartPointZ);
  Scene::TAD_Cloud_ElementGenerator::LocationAgent::LocationAgentPtr locAgentPtr =
      std::make_shared<Scene::TAD_Cloud_ElementGenerator::LocationAgent>();
  locAgentPtr->Init(roadStartPoint);
  Scene::TAD_Cloud_ElementGenerator::VehicleInputeAgent::VehicleInitParam_t vehicleInitParam;
  vehicleInitParam.mLocationPtr = locAgentPtr;

  Base::ISceneLoader::ITrafficFlowViewer::VehTypePtr vehTypePtr =
      m_TrafficElementSystemPtr->SceneLoader()->GetTrafficFlow()->GetVehType(vehType);
  vehicleInitParam.mVehTypePtr = vehTypePtr;
  vehicleInitParam.mBehPtr = m_TrafficElementSystemPtr->SceneLoader()->GetTrafficFlow()->GetBeh(1);
  vehicleInitParam.SetStartV_Cloud(startV);
  vehicleInitParam.SetMaxV_Cloud(15);

  Base::IElementManagerPtr elemMgr = m_TrafficElementSystemPtr->ElemMgr();
  TrafficFlow::PS_VehicleElementPtr vehicle = std::make_shared<TrafficFlow::PS_VehicleElement>();
  if (NonNull_Pointer(vehicle) &&
      CallSucc(vehicle->Initialize(vehId, vehicleInitParam, vehicleStartLaneId, elemMgr->GetDataSource()))) {
    vehicle->AddVehiclePath(vehicleInitParam.mLocationPtr->GPS(), {roadIds.begin(), roadIds.end()});
    elemMgr->AddVehiclePtr(vehicle);
  } else {
    LOG(WARNING) << ", Create Vehicle Element Failure.";
    return false;
  }
  return true;
}

void PluginTemplate::TrafficRecord_GetCurTrafficState(const int64_t frame_id,
                                                      its::txSimStatus& sceneSnapshot) TX_NOEXCEPT {
  static its::txSimStatus snapshotCache_;
  if (snapshotCache_.time() != trafficMsg_.time()) {
    // auto meso_traffic_msg = trafficRecord_.RecordMesoInfo(trafficMsg_);
    snapshotCache_ = trafficRecord_.getCurrentTrafficState(trafficMsg_);
  }
  sceneSnapshot.CopyFrom(snapshotCache_);
}

Base::txBool PluginTemplate::GetMapTrafficInfo(const int64_t frame_id, sim_msg::Traffic& traffic) TX_NOEXCEPT {
  static Base::txInt sFrameCnt = 0;
  if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
    m_TrafficElementSystemPtr->FillingTrafficData(GetTimeMgr(), traffic);
#  if __pingsn__
    traffic.set_time(cur_AbsTime);
    trafficMsg_ = traffic;
    trafficRecord_.RecordTravelTime(traffic, cur_AbsTime);

    LOG_IF(INFO, (((sFrameCnt++) > FLAGS_tc_show_statictis_info_interval) ? (sFrameCnt = 0, true) : (false)))
        << "getTrafficInfoByEgo " << TX_VARS(traffic.cars_size()) << TX_VARS(traffic.trafficlights_size());
    if (traffic.cars_size() > 0) {
      Base::txFloat sum_speed = 0.0;
      for (const auto& car : traffic.cars()) {
        sum_speed += car.v();
      }
      const Base::txSize nCarSize = traffic.cars_size();
      const Base::txFloat averSpeed = sum_speed / ((Base::txFloat)nCarSize);
      traffic.set_road_network_aver_velocity(averSpeed);
      traffic.set_road_network_total_vehicle_count(nCarSize);
      LOG(INFO) << TX_VARS_NAME(set_road_network_aver_velocity, averSpeed)
                << TX_VARS_NAME(set_road_network_total_vehicle_count, nCarSize);
    } else {
      traffic.set_road_network_aver_velocity(0.0);
      traffic.set_road_network_total_vehicle_count(0);
      LOG(INFO) << TX_VARS_NAME(set_road_network_aver_velocity, 0.0)
                << TX_VARS_NAME(set_road_network_total_vehicle_count, 0);
    }
#  endif /*__pingsn__*/
    return true;
  } else {
    return false;
  }
}

void PluginTemplate::TrafficRecord_RecordInputVehicle(const its::txVehicle& vehicleMsg) TX_NOEXCEPT {
  trafficRecord_.RecordInputVehicle(vehicleMsg);
}

its::txSimStat PluginTemplate::GetSimStatMsg() const TX_NOEXCEPT {
  return trafficRecord_.getSimStatMsg(trafficMsg_, cur_AbsTime);
}
#endif /*__pingsn__*/

void PluginTemplate::ResetVarsPlugin() TX_NOEXCEPT {
  m_step_count_ = 0;
  m_max_step_count_ = 99999999;
  m_payload_.clear();
  m_lastStepTime = 0.0;
  Base::ITrafficElement::ResetSysId();
  m_passTime = 0.0;
  CreateSystemPlugin();
  if (IsResetHdMapCache()) {
    HdMap::HadmapCacheConCurrent::Release();
    Geometry::SpatialQuery::ClearSpatialQueryReferenceLineInfo();
    HdMap::txRoadNetwork::Release();
#if USE_HashedRoadNetwork
    HdMap::HashedRoadCacheConCurrent::Release();
#endif /*USE_HashedRoadNetwork*/
  } else {
    HdMap::HadmapCacheConCurrent::ShowCacheInfo();
  }
#if USE_RTree
  Geometry::SpatialQuery::ClearSpatialQueryElementInfo();
  Geometry::SpatialQuery::ClearSpatialQuerySignalInfo();
#endif
  Unit::IAngle::GenerateSinCosLookupTable();

  const auto cfg_path = txApplication::AppAbstractPath() + Base::txString("/") + FLAGS_behavior_cfg_name;
  LOG(INFO) << "TAD_Vehicle_Behavior config : " << cfg_path;

  Utils::BehaviorCfgDB::Initialize(cfg_path, Utils::ConfigType::file_path);
  Utils::BehaviorCfgDB::ResetDefaultCfg();

  LOG(WARNING) << "#############################################" << std::endl
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfo>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Base::ITrafficElement>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfoOrthogonalList>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<HdMap::txLaneInfoInterface>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Base::JointPointTreeNode>::howMany()) << std::endl
               << "###########################################################";
  FLAGS_Kill_After_Stop = true;

  cur_AbsTime = 0;
  trafficMsg_.Clear();
}

void PluginTemplate::CreateSystemPlugin() TX_NOEXCEPT {
  m_ps_TrafficElementSystemPtr = nullptr;
  m_TrafficElementSystemPtr = nullptr;
  if (isVirtualCity()) {
    m_TrafficElementSystemPtr = std::make_shared<TrafficFlow::CloudTrafficElementSystem>();
  } else if (isParallelSimulation()) {
#if __pingsn__
    m_ps_TrafficElementSystemPtr = std::make_shared<TrafficFlow::ParallelSimulation_TrafficElementSystem>();
    m_TrafficElementSystemPtr = m_ps_TrafficElementSystemPtr;
#endif /*__pingsn__*/
  }

  if (NonNull_Pointer(m_TrafficElementSystemPtr)) {
    LogInfo << "Create TrafficElementSystem Success." << TX_COND(isVirtualCity()) << TX_COND(isParallelSimulation());
  } else {
    LogWarn << "Create TrafficElementSystem Failure.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef SceneLoaderLogInfo
#undef LogWarn
