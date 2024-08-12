// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_sim_ego_template.h"
#include "HdMap/tx_hashed_road.h"
#include "HdMap/tx_hd_map_cache_concurrent.h"
#include "SceneLoader/tx_tadloader.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_application.h"
#include "tx_planning_flags.h"
#include "tx_planning_scene_event_loader.h"
#include "tx_planning_scene_loader.h"
#include "tx_planning_vehicle_element.h"
#include "tx_spatial_query.h"
#include "tx_string_utils.h"
#include "tx_time_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_Ego_Loop)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

void SimEgoTemplate::ParseInitParameter(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  if (Utils::Str2Type(helper.GetParameter("LogLevel_Ego_Loop"), FLAGS_LogLevel_Ego_Loop)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(LogLevel_Ego_Loop);
  }

  if (Utils::Str2Type(helper.GetParameter("LogLevel_Ego_Element"), FLAGS_LogLevel_Ego_Element)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(LogLevel_Ego_Element);
  }

  if (Utils::Str2Type(helper.GetParameter("ego_rnd_seed"), FLAGS_ego_rnd_seed)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(ego_rnd_seed);
  }

  if (Utils::Str2Type(helper.GetParameter("ego_pubilc_location"), FLAGS_ego_pubilc_location)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(ego_pubilc_location);
  }

  if (Utils::Str2Type(helper.GetParameter("ego_pubilc_trajectory"), FLAGS_ego_pubilc_trajectory)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(ego_pubilc_trajectory);
  }

  if (Utils::Str2Type(helper.GetParameter("dummy_ignore_perception"), FLAGS_dummy_ignore_perception)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(dummy_ignore_perception);
  }
  /*ego_location_closed_loop*/
  if (Utils::Str2Type(helper.GetParameter("ego_location_closed_loop"), FLAGS_ego_location_closed_loop)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(ego_location_closed_loop);
  }

  if (Utils::Str2Type(helper.GetParameter("LogLevel_Ego_SceneEvent"), FLAGS_LogLevel_Ego_SceneEvent)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(LogLevel_Ego_SceneEvent);
  }

  if (Utils::Str2Type(helper.GetParameter("route_end_behavior_force_stop"), FLAGS_route_end_behavior_force_stop)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(route_end_behavior_force_stop);
  }

  if (Utils::Str2Type(helper.GetParameter("looking_distance_factor"), FLAGS_looking_distance_factor)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(looking_distance_factor);
  }

  if (Utils::Str2Type(helper.GetParameter("wheel2steer"), FLAGS_wheel2steer)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(wheel2steer);
  }

  if (Utils::Str2Type(helper.GetParameter("LogLevel_Ego_Traj_ClosedLoop"), FLAGS_LogLevel_Ego_Traj_ClosedLoop)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(LogLevel_Ego_Traj_ClosedLoop);
  }

  if (Utils::Str2Type(helper.GetParameter("LogLevel_Ego_Traj"), FLAGS_LogLevel_Ego_Traj)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(LogLevel_Ego_Traj);
  }

  if (Utils::Str2Type(helper.GetParameter("Use_Control_Debug_Object"), FLAGS_Use_Control_Debug_Object)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(Use_Control_Debug_Object);
  }

  if (Utils::Str2Type(helper.GetParameter("use_ego_lanechange"), FLAGS_ego_use_lanechange)) {
    LOG(INFO) << "[Input_Parameter] " << TX_FLAGS(ego_use_lanechange);
  }
}

void fake_scene(tx_sim::ResetHelper& helper, sim_msg::Scene& ref_scene) TX_NOEXCEPT {
  ref_scene.mutable_setting()->set_version("1.0.0.0");
  ref_scene.mutable_setting()->set_hadmap_path(helper.map_file_path());
  ref_scene.mutable_setting()->set_scenario_path(helper.scenario_file_path());

  auto ego_ptr = ref_scene.add_egos();
  ego_ptr->set_id(66);
  ego_ptr->set_name("dummy_66");
  // ego_ptr->set_type("TrafficVehicle");

  auto common_ptr = ego_ptr->mutable_physicles()->Add()->mutable_common();
  common_ptr->mutable_bounding_box()->set_length(FLAGS_EGO_Length);
  common_ptr->mutable_bounding_box()->set_width(FLAGS_EGO_Width);
  common_ptr->mutable_bounding_box()->set_higth(FLAGS_EGO_Height);

  common_ptr->mutable_bounding_box()->mutable_center()->set_x(helper.ego_path().front().x);
  common_ptr->mutable_bounding_box()->mutable_center()->set_y(helper.ego_path().front().y);
  common_ptr->mutable_bounding_box()->mutable_center()->set_z(helper.ego_path().front().z);

  common_ptr->set_color(sim_msg::Color::COLOR_WHITE);
  common_ptr->set_mass(1000.0);
  // common_ptr->set_reference_point(sim_msg::ReferencePoint::REFERENCE_POINT_REAR_AXIS);

  auto init_ptr = ego_ptr->mutable_initial();
  auto sim_ptr = SceneLoader::Sim::load_scene_sim(helper.scenario_file_path());
  google::protobuf::DoubleValue double_value;

  init_ptr->mutable_common()->set_speed(sim_ptr->_planner.start_v);
  auto route_waypoints_ptr = init_ptr->mutable_common()->mutable_waypoints();

  auto ego_path = helper.ego_path();
  for (const auto& ref_wp : ego_path) {
    auto cur_wp_ptr = route_waypoints_ptr->Add();
    cur_wp_ptr->mutable_position()->mutable_world()->set_x(ref_wp.x);
    cur_wp_ptr->mutable_position()->mutable_world()->set_y(ref_wp.y);
    cur_wp_ptr->mutable_position()->mutable_world()->set_z(ref_wp.z);
  }

  init_ptr->mutable_activate()->set_automode(sim_msg::Activate::Automode::Activate_Automode_AUTOMODE_AUTOPILOT);
  init_ptr->mutable_assign()->set_controller_name("test_cruise");
  init_ptr->mutable_assign()->mutable_cruise()->set_cancel_sw(sim_msg::OnOff::ONOFF_ON);
  init_ptr->mutable_assign()->mutable_cruise()->set_resume_sw(sim_msg::OnOff::ONOFF_ON);
  init_ptr->mutable_assign()->mutable_cruise()->set_speed_inc_sw(sim_msg::OnOff::ONOFF_ON);
  init_ptr->mutable_assign()->mutable_cruise()->set_speed_dec_sw(sim_msg::OnOff::ONOFF_ON);
  init_ptr->mutable_assign()->mutable_cruise()->set_set_timegap(10.0);
  init_ptr->mutable_assign()->mutable_cruise()->set_set_speed(22.0);

  auto dynamic_ptr = ego_ptr->mutable_dynamic();

  {
    auto event_ptr = dynamic_ptr->add_events();
    event_ptr->set_id(101);
    event_ptr->set_name("time_trigger");
    auto trigger_condition_ptr = event_ptr->mutable_trigger()->mutable_conditions()->Add();
    trigger_condition_ptr->set_edge(sim_msg::ConditionEdge::CONDITION_EDGE_BOTH);
    trigger_condition_ptr->set_delay(3.0);
    trigger_condition_ptr->set_count(1);
    auto time_trigger_ptr = trigger_condition_ptr->mutable_simulation_time();
    time_trigger_ptr->set_value(10.0);
    time_trigger_ptr->set_rule(sim_msg::Rule::RULE_EQUAL_TO);

    {
      auto action_ptr = event_ptr->mutable_actions()->Add();
      action_ptr->mutable_activate()->set_automode(sim_msg::Activate_Automode::Activate_Automode_AUTOMODE_LATERAL);
    }
    {
      auto action_ptr = event_ptr->mutable_actions()->Add();
      auto cruise_ptr = action_ptr->mutable_assign()->mutable_cruise();

      cruise_ptr->set_cancel_sw(sim_msg::OnOff::ONOFF_OFF);
      cruise_ptr->set_resume_sw(sim_msg::OnOff::ONOFF_OFF);
      cruise_ptr->set_speed_inc_sw(sim_msg::OnOff::ONOFF_OFF);
      cruise_ptr->set_speed_dec_sw(sim_msg::OnOff::ONOFF_OFF);
      cruise_ptr->set_set_timegap(15.0);
      cruise_ptr->set_set_speed(17.0);
    }
    {
      auto action_ptr = event_ptr->mutable_actions()->Add();
      auto override_ptr = action_ptr->mutable_override();

      override_ptr->mutable_brake()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_brake()->set_value(1.0);

      override_ptr->mutable_clutch()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_clutch()->set_value(2.0);

      override_ptr->mutable_gear()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_gear()->set_value(sim_msg::GearSts::GEAR_STS_R);

      override_ptr->mutable_parking_brake()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_parking_brake()->set_value(sim_msg::ParkingBrakeSts::PARKING_BRAKE_STS_APPLYING);

      override_ptr->mutable_steering_wheel()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_steering_wheel()->set_value(3.0);

      override_ptr->mutable_throttle()->set_action(sim_msg::OnOff::ONOFF_ON);
      override_ptr->mutable_throttle()->set_value(4.0);
    }

    {
      auto action_ptr = event_ptr->mutable_actions()->Add();
      auto command_ptr = action_ptr->mutable_command();
      command_ptr->set_emergency_stop(sim_msg::OnOff::ONOFF_ON);
    }

    {
      auto action_ptr = event_ptr->mutable_actions()->Add();
      auto status_ptr = action_ptr->mutable_status();
      status_ptr->set_right_turn_light(sim_msg::OnOff::ONOFF_ON);
    }
  }
}

void SimEgoTemplate::ParseResetParameter(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  m_groupName = helper.group_name();
  FLAGS_ego_map_file_path = helper.map_file_path();
  FLAGS_ego_scenario_file_path = helper.scenario_file_path();
  const auto originGPS = helper.map_local_origin();
  FLAGS_ego_map_local_origin_lon = originGPS.x;
  FLAGS_ego_map_local_origin_lat = originGPS.y;
  FLAGS_ego_map_local_origin_alt = originGPS.z;
  m_ego_path = helper.ego_path();
  m_ego_start_location.ParseFromString(helper.ego_start_location());
  FLAGS_ego_max_v = helper.ego_speed_limit();

  LOG(INFO) << "call helper.scene_pb() start.";
  const std::string input_scene_pb_str = helper.scene_pb();
  LOG(INFO) << "call helper.scene_pb() finish." << TX_VARS(input_scene_pb_str.size())
            << " scene deserialization start.";
  m_input_dummy_driver_scene.ParseFromString(input_scene_pb_str);
  LOG(INFO) << "scene deserialization finish.";
  // fake_scene(helper, m_input_dummy_driver_scene);
}

void SimEgoTemplate::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
  ParseInitParameter(helper);
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
  if (FLAGS_ego_pubilc_location) {
    LOG(INFO) << "public location";
    helper.Publish(tx_sim::topic::kLocation);
  }
  if (FLAGS_ego_pubilc_trajectory) {
    LOG(INFO) << "public trajectory";
    helper.Publish(tx_sim::topic::kTrajectory);
  }
  helper.Publish(FLAGS_vehicle_interaction_topic);
#if __Control_V2__
  helper.Publish(FLAGS_control_topic);
  if (FLAGS_Use_Control_Debug_Object) {
    helper.Publish(tx_sim::topic::kTraffic);
  }

#endif /*__Control_V2__*/
}

void SimEgoTemplate::Reset(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  LogInfo << "call " << __func__;
  ParseResetParameter(helper);
  PrintFlagsValues_Ego();
  CreateEnv(helper);
}

void SimEgoTemplate::Step(tx_sim::StepHelper& helper) TX_NOEXCEPT {
  const Base::TimeParamManager timeMgr = MakeTimeMgr(helper.timestamp());
  timeMgr.str();
  Simulation(helper, timeMgr);
}

void SimEgoTemplate::ReceiveEgoInfo(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  LOG(WARNING) << "ReceiveEgoInfo";
  const auto sEgoType = TrafficSystemPtr()->GetEgoType();
  {
    m_traffic_payload.clear();
    GetSubscribedMessage(helper, tx_sim::topic::kLocation, m_traffic_payload);
    PlanningTrafficSystemPtr()->UpdatePlanningCarData(timeMgr, Base::Enums::EgoSubType::eLeader, m_traffic_payload);
  }
  {
    m_traffic_payload.clear();
    GetSubscribedMessage(helper, tx_sim::topic::kTraffic, m_traffic_payload);
    PlanningTrafficSystemPtr()->UpdateEnvData(timeMgr, m_traffic_payload);
  }
  {
    m_traffic_payload.clear();
    GetSubscribedMessage(helper, tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation, m_traffic_payload);
    PlanningTrafficSystemPtr()->UpdateGroupPlanningCarData(timeMgr, TrafficSystemPtr()->GetGroupName(),
                                                           m_traffic_payload);
  }
  m_traffic_payload.clear();
}

void SimEgoTemplate::Simulation(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  LogInfo << __func__;

  using TimeParamManager = Base::TimeParamManager;
  if (NonNull_Pointer(TrafficSystemPtr()) && CallSucc(TrafficSystemPtr()->IsAlive())) {
    // LOG(INFO) << timeMgr.str();
    PreSimulation(helper, timeMgr);

    ReceiveEgoInfo(helper, timeMgr);

    SimulationTraffic(helper, timeMgr);

    PostSimulation(helper, timeMgr);
  } else {
    LOG(WARNING) << "m_TrafficElementSystemPtr is not alive / initialized, Send 0 elements.";
  }
  LogInfo << "Step end. ";
}

void SimEgoTemplate::SimulationTraffic(tx_sim::StepHelper& helper, const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {
  const Base::txBool res = TrafficSystemPtr()->Update(timeMgr);
  m_output_ego_location.Clear();
  if (FLAGS_ego_pubilc_location) {
    PlanningTrafficSystemPtr()->FillingLocationData(timeMgr, m_output_ego_location);
    PublishMessage(helper, tx_sim::topic::kLocation, m_output_ego_location);
  }
  if (FLAGS_ego_pubilc_trajectory) {
    PlanningTrafficSystemPtr()->FillingTrajectoryData(timeMgr, m_output_ego_trajectory);
    PublishMessage(helper, tx_sim::topic::kTrajectory, m_output_ego_trajectory);
  }
  if (FLAGS_ego_pubilc_veh_interaction) {
    if (NonNull_Pointer(PlanningTrafficSystemPtr())) {
      TrafficFlow::SimPlanningVehicleElementPtr dummy_driver_ptr =
          std::dynamic_pointer_cast<TrafficFlow::SimPlanningVehicleElement>(
              PlanningTrafficSystemPtr()->GetEgoVehicleElementPtr());
      if (NonNull_Pointer(dummy_driver_ptr)) {
        const sim_msg::VehicleInteraction& send_vehicle_interaction_pb =
            dummy_driver_ptr->vehicle_interaction_pb(timeMgr);
        PublishMessage(helper, FLAGS_vehicle_interaction_topic /*vehicle_interaction*/,
                       dummy_driver_ptr->vehicle_interaction_pb(timeMgr));
        LOG(INFO) << "[vehicle_interaction_pb_test]" << TX_VARS_NAME(time, timeMgr.PassTime())
                  << TX_VARS_NAME(pb, send_vehicle_interaction_pb.DebugString());

        if (m_output_ego_trajectory.point_size() > 0) {
#if __Control_V2__
          dummy_driver_ptr->UpdateControlPb(timeMgr, m_output_ego_trajectory.a(),
                                            dummy_driver_ptr->GetAngle().GetRadian());
          PublishMessage(helper, FLAGS_control_topic, dummy_driver_ptr->control_v2_pb(timeMgr));
          if (FLAGS_Use_Control_Debug_Object) {
            PublishMessage(helper, tx_sim::topic::kTraffic, dummy_driver_ptr->DebugTraffic());
          }
#else  /*__Control_V2__*/
          dummy_driver_ptr->UpdateControlPb(timeMgr, m_output_ego_trajectory.a(),
                                            dummy_driver_ptr->GetAngle().GetRadian());
          PublishMessage(helper, FLAGS_control_topic, dummy_driver_ptr->control_pb(timeMgr));
#endif /*__Control_V2__*/
        } else {
          LogWarn << TX_VARS(m_output_ego_trajectory.point_size());
        }
      } else {
        LogWarn << "dummy driver is null.";
      }
    } else {
      LogWarn << TX_COND(NonNull_Pointer(PlanningTrafficSystemPtr()));
    }
  }
  TrafficSystemPtr()->FillingSpatialQuery();

  m_lastStepTime = timeMgr.TimeStamp();

  if (CallFail(res) && NonNull_Pointer(PlanningTrafficSystemPtr()) &&
      CallSucc(PlanningTrafficSystemPtr()->isPlanningMode())) {
    helper.StopScenario(txString("perfect planning is stop."));
  }
}

Base::TimeParamManager SimEgoTemplate::MakeTimeMgr(const Base::txFloat time_stamp) TX_NOEXCEPT {
  const Base::txFloat relativeTime = Utils::MillisecondToSecond(time_stamp - m_lastStepTime);
  m_passTime += relativeTime;
  Base::TimeParamManager timeMgr(Utils::MillisecondToSecond(time_stamp), relativeTime, m_passTime, time_stamp);
  LogInfo << "[ego_time] : " << timeMgr.str() << TX_VARS(time_stamp);
  return timeMgr;
}

void SimEgoTemplate::ResetVars() TX_NOEXCEPT {
  m_lastStepTime = 0.0;
  m_passTime = 0.0;
  TX_MARK("m_ego_path.clear()");
  TX_MARK("m_ego_start_location.Clear()");
  CreateSystem();
  HdMap::HadmapCacheConCurrent::Release();
#if USE_RTree
  Geometry::SpatialQuery::ClearSpatialQueryInfo();
#endif
#if USE_HashedRoadNetwork
  HdMap::HashedRoadCacheConCurrent::Release();
#endif /*USE_HashedRoadNetwork*/
  Unit::IAngle::GenerateSinCosLookupTable();
  const auto cfg_path = txApplication::AppAbstractPath() + Base::txString("/") + FLAGS_behavior_cfg_name;
  LOG(INFO) << "TAD_Vehicle_Behavior config : " << cfg_path;
  Utils::BehaviorCfgDB::Initialize(cfg_path, Utils::ConfigType::file_path);
  Utils::BehaviorCfgDB::ResetDefaultCfg();
}

void SimEgoTemplate::CreateEnv(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  std::vector<std::pair<int64_t, std::string>> measurements;
  sim_msg::VehicleGeometoryList vehGeomList;
  helper.vehicle_measurements(measurements);
  if (1 != measurements.size()) {
    LogWarn << "helper.vehicle_measurements error. " << TX_VARS(measurements.size());
    return;
  } else {
    vehGeomList.ParseFromString(measurements.front().second);
    LOG(INFO) << vehGeomList.ShortDebugString();
  }

  VerifyRouteEndBehavior(helper);

  ResetVars();
  LogInfo << "call " << __func__;

  SceneLoader::PlanningSceneEvent_SceneLoaderPtr loader =
      std::make_shared<SceneLoader::PlanningSceneEvent_SceneLoader>();
  std::vector<hadmap::txPoint> vec_ego_path;
  loader->SetGroupName(m_groupName);
  for (const auto v3 : m_ego_path) {
    vec_ego_path.emplace_back(hadmap::txPoint(v3.x, v3.y, v3.z));
  }

  if (NonNull_Pointer(loader)) {
    loader->Init(vec_ego_path, m_ego_start_location, m_input_dummy_driver_scene);
    loader->LoadSceneEvent(m_input_dummy_driver_scene);
  }

  loader->SetVehicleGeometory(0, vehGeomList);
  if (NonNull_Pointer(TrafficSystemPtr())) {
    HdMap::HadmapCacheConCurrent::InitParams_t initParams;
    GetMapManagerInitParams(initParams);
    if (HdMap::HadmapCacheConCurrent::Initialize(initParams)) {
      if (TrafficSystemPtr()->Initialize(loader)) {
        TrafficSystemPtr()->SetGroupName(m_groupName);
        LogInfo << "Ego System Init Success.";
      } else {
        LogWarn << "Ego System Init Failure.";
      }
    } else {
      LogWarn << "MapManager Init Failure.";
    }
  } else {
    LogWarn << "TrafficElementSystem is Null.";
  }
}

Base::txBool SimEgoTemplate::GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
    TX_NOEXCEPT {
  refParams.strTrafficFilePath = FLAGS_ego_scenario_file_path; /*simulation.traffic,*/
  refParams.strHdMapFilePath = FLAGS_ego_map_file_path;        /*simData.scene.c_str(),*/

  __Lon__(refParams.SceneOriginGPS) = FLAGS_ego_map_local_origin_lon;
  __Lat__(refParams.SceneOriginGPS) = FLAGS_ego_map_local_origin_lat;
  __Alt__(refParams.SceneOriginGPS) = FLAGS_ego_map_local_origin_alt;

  LogInfo << "Map File From helper : " << refParams.strHdMapFilePath
          << TX_VARS_NAME(origin, Utils::ToString(refParams.SceneOriginGPS));
  return true;
}

void SimEgoTemplate::CreateSystem() TX_NOEXCEPT {
  m_PlanningTrafficElementSystemPtr = std::make_shared<PlanningTrafficElementSystem>();
  m_TrafficElementSystem_Ptr = m_PlanningTrafficElementSystemPtr;
  if (NonNull_Pointer(m_TrafficElementSystem_Ptr)) {
    LogInfo << "Create TrafficElementSystem Success.";
  } else {
    LogWarn << "Create TrafficElementSystem Failure.";
  }
}

void SimEgoTemplate::VerifyRouteEndBehavior(tx_sim::ResetHelper& helper) TX_NOEXCEPT {
  LogWarn << "[Input]" << TX_VARS(helper.scenario_file_path()) << TX_VARS(FLAGS_route_end_behavior_force_stop)
          << TX_VARS(FLAGS_route_end_behavior_desired_stop);

  Base::txString strSceneDataPath, strHadmapPath;
  const Base::txBool res_Sim2SceneFile =
      SceneLoader::TAD_SceneLoader::Sim2SceneFile(helper.scenario_file_path(), strSceneDataPath, strHadmapPath);
  if (CallSucc(res_Sim2SceneFile)) {
    using namespace Utils;
    using namespace boost::filesystem;
    std::string strExt = FilePath(strSceneDataPath).extension().string();
    if (boost::iequals(strExt, FLAGS_Worldsim_Extension)) {
      FLAGS_route_end_behavior_desired_stop = true;
      LogWarn << "[Result] worldsim scene " << TX_VARS(strSceneDataPath) << TX_VARS(FLAGS_route_end_behavior_force_stop)
              << TX_VARS(FLAGS_route_end_behavior_desired_stop);
    } else if (boost::iequals(strExt, FLAGS_Logsim_Extension) || boost::iequals(strExt, FLAGS_L2W_Extension)) {
      FLAGS_route_end_behavior_desired_stop = false;
      LogWarn << "[Result] logsim/l2w scene " << TX_VARS(strSceneDataPath)
              << TX_VARS(FLAGS_route_end_behavior_force_stop) << TX_VARS(FLAGS_route_end_behavior_desired_stop);
    } else {
      LogWarn << "[Result] unknow scene type. using default behaivor " << TX_VARS(strSceneDataPath)
              << TX_VARS(FLAGS_route_end_behavior_force_stop) << TX_VARS(FLAGS_route_end_behavior_desired_stop);
    }
  } else {
    LogWarn << "[Result] Parse scenario_file_path failure. using default behaivor "
            << TX_VARS(helper.scenario_file_path()) << TX_VARS(FLAGS_route_end_behavior_force_stop)
            << TX_VARS(FLAGS_route_end_behavior_desired_stop);
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)
