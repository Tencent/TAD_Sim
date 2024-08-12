// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_vehicle_behavior_configure_utils.h"
#include <boost/optional.hpp>
#include <fstream>
#include <sstream>
#include "tx_enum_utils.h"
#include "tx_path_utils.h"
TX_NAMESPACE_OPEN(Cfg)
struct vehicle_param {
  /*
  "Friction_Factor" : 0.6,
  "LaneKeep" : 0.5,
  "MAX_SPEED" : 27.8,
  "ChangeTopologyThreshold" : 0.76,
  "NoCheckLCAbortThreshold" : 0.8,
  "EmptyTargetLaneGapThreshold" : 50,
  "SpeedDenominatorOffset" : 1,
  "SafetyGap_Rear" : 3,
  "SafetyGap_Front" : 2,
  "SafetyGap_Side" : 1,
  "ComfortGap" : 40,
  "Max_Ped_Reaction_Distance" : 30,
  "MAX_REACTION_GAP" : 100,
  "LaneChanging_Aggressive_Threshold" : 0.5,
  "Tolerance_Threshold" : 0.8,
  "Max_Deceleration" : -10.0,
  "Signal_Reaction_Gap" : 30.0,
  "Max_Comfort_Deceleration" : -10.0,
  "Yielding_Aggressive_Threshold" : 0.5,
  "g_Driving_Parameters" : [1.50, 1.30, 4.00, -12.00,-0.25,1.35,0.0006, 0.25,2.00,1.50]
  */
  Base::txFloat IDM_Vision_Width = 5.5;
  Base::txFloat IDM_Vision_Length = 50.0;
  Base::txFloat SideGapMinVelocity = 0.2;
  Base::txFloat SideFrontVisionDistance = 2.0 * FLAGS_EGO_Length;
  Base::txFloat SideRearVisionDistance = 0.0;
  Base::txFloat Friction_Factor = 0.6;
  Base::txFloat LaneKeep = 5.0;
  Base::txFloat MAX_SPEED = 27.8;
  Base::txFloat ChangeTopologyThreshold = 0.76;
  Base::txFloat NoCheckLCAbortThreshold = 0.8;
  Base::txFloat EmptyTargetLaneGapThreshold = 50;
  Base::txFloat SpeedDenominatorOffset = 1.0;
  Base::txFloat SafetyGap_Rear = 3;
  Base::txFloat SafetyGap_Front = 3;
  Base::txFloat SafetyGap_Side = 1;
  Base::txFloat ComfortGap = 40.0;
  Base::txFloat Max_Ped_Reaction_Distance = 30.0;
  Base::txFloat MAX_REACTION_GAP = 100.0;
  Base::txFloat LaneChanging_Aggressive_Threshold = 0.5;
  Base::txFloat Tolerance_Threshold = 0.8;
  Base::txFloat Max_Deceleration = -10.0;
  Base::txFloat Signal_Reaction_Gap = 20.0;
  Base::txFloat Max_Comfort_Deceleration = -10.0;
  Base::txFloat Yielding_Aggressive_Threshold = 0.5;
  Base::txFloat L2W_Switch_Time = -1.0;
  Base::txFloat L2W_Switch_EndTime = 15.0;
  Base::txFloat L2W_Speed_Ratio = 0.5;
  Base::txFloat L2W_Speed_Ratio_Upper = 0.5;
  Base::txFloat L2W_Log2worldsim_Timestep = -200.0;
  Base::txFloat L2W_Log2worldsim_Duration = -1800.0;
  Base::txInt SceneMaxVehicleSize = -400;
  Base::txBool L2W_SendTraficTopic = false;
  Base::txFloat LaneKeepA = 1.0;
  Base::txFloat LaneKeepB = 3.0;

  Base::txFloat W74_DEFAULT_ax = 1.16;
  Base::txFloat W74_DEFAULT_bx_add = 1.1;
  Base::txFloat W74_DEFAULT_bx_mult = 2.04;
  Base::txFloat W74_DEFAULT_cx = 40;
  Base::txFloat W74_DEFAULT_ex_add = 1.5;
  Base::txFloat W74_DEFAULT_ex_mult = 0.55;
  Base::txFloat W74_DEFAULT_opdv_mult = 1.5;
  Base::txFloat W74_DEFAULT_bnull_mult = 0.25;
  Base::txBool W74_DEFAULT_v2cav = false;

  std::vector<Base::txFloat> g_Driving_Parameters = {1.84, 1.45, 4.4, -12.00, -0.25, 1.35, 0.0006, 0.25, 2.00, 1.50};
  friend std::ostream& operator<<(std::ostream& os, const vehicle_param& v) TX_NOEXCEPT {
    os << TX_VARS_NAME(IDM_Vision_Length, v.IDM_Vision_Length) << std::endl
       << TX_VARS_NAME(IDM_Vision_Width, v.IDM_Vision_Width) << std::endl

       << TX_VARS_NAME(SideFrontVisionDistance, v.SideFrontVisionDistance) << std::endl
       << TX_VARS_NAME(SideRearVisionDistance, v.SideRearVisionDistance) << std::endl
       << TX_VARS_NAME(SideGapMinVelocity, v.SideGapMinVelocity) << std::endl

       << TX_VARS_NAME(Friction_Factor, v.Friction_Factor) << std::endl
       << TX_VARS_NAME(LaneKeep, v.LaneKeep) << std::endl
       << TX_VARS_NAME(MAX_SPEED, v.MAX_SPEED) << std::endl
       << TX_VARS_NAME(ChangeTopologyThreshold, v.ChangeTopologyThreshold) << std::endl
       << TX_VARS_NAME(NoCheckLCAbortThreshold, v.NoCheckLCAbortThreshold) << std::endl
       << TX_VARS_NAME(EmptyTargetLaneGapThreshold, v.EmptyTargetLaneGapThreshold) << std::endl
       << TX_VARS_NAME(SpeedDenominatorOffset, v.SpeedDenominatorOffset) << std::endl

       << TX_VARS_NAME(param_0, v.g_Driving_Parameters[0]) << std::endl
       << TX_VARS_NAME(param_1, v.g_Driving_Parameters[1]) << std::endl
       << TX_VARS_NAME(param_2, v.g_Driving_Parameters[2]) << std::endl
       << TX_VARS_NAME(param_3, v.g_Driving_Parameters[3]) << std::endl
       << TX_VARS_NAME(param_4, v.g_Driving_Parameters[4]) << std::endl
       << TX_VARS_NAME(param_5, v.g_Driving_Parameters[5]) << std::endl
       << TX_VARS_NAME(param_6, v.g_Driving_Parameters[6]) << std::endl
       << TX_VARS_NAME(param_7, v.g_Driving_Parameters[7]) << std::endl
       << TX_VARS_NAME(param_8, v.g_Driving_Parameters[8]) << std::endl
       << TX_VARS_NAME(param_9, v.g_Driving_Parameters[9]) << std::endl

       << TX_VARS_NAME(SafetyGap_Rear, v.SafetyGap_Rear) << std::endl
       << TX_VARS_NAME(SafetyGap_Front, v.SafetyGap_Front) << std::endl
       << TX_VARS_NAME(SafetyGap_Side, v.SafetyGap_Side) << std::endl
       << TX_VARS_NAME(ComfortGap, v.ComfortGap) << std::endl
       << TX_VARS_NAME(Max_Ped_Reaction_Distance, v.Max_Ped_Reaction_Distance) << std::endl
       << TX_VARS_NAME(MAX_REACTION_GAP, v.MAX_REACTION_GAP) << std::endl
       << TX_VARS_NAME(LaneChanging_Aggressive_Threshold, v.LaneChanging_Aggressive_Threshold) << std::endl
       << TX_VARS_NAME(Tolerance_Threshold, v.Tolerance_Threshold) << std::endl
       << TX_VARS_NAME(Max_Deceleration, v.Max_Deceleration) << std::endl
       << TX_VARS_NAME(Signal_Reaction_Gap, v.Signal_Reaction_Gap) << std::endl
       << TX_VARS_NAME(Max_Comfort_Deceleration, v.Max_Comfort_Deceleration) << std::endl
       << TX_VARS_NAME(Yielding_Aggressive_Threshold, v.Yielding_Aggressive_Threshold) << std::endl

       << TX_VARS_NAME(L2W_Switch_Time, v.L2W_Switch_Time) << std::endl
       << TX_VARS_NAME(L2W_Speed_Ratio, v.L2W_Speed_Ratio) << std::endl
       << TX_VARS_NAME(L2W_Speed_Ratio_Upper, v.L2W_Speed_Ratio_Upper) << std::endl
       << TX_VARS_NAME(L2W_Log2worldsim_Timestep, v.L2W_Log2worldsim_Timestep) << std::endl
       << TX_VARS_NAME(L2W_Log2worldsim_Duration, v.L2W_Log2worldsim_Duration) << std::endl
       << TX_VARS_NAME(SceneMaxVehicleSize, v.SceneMaxVehicleSize) << std::endl
       << TX_COND_NAME(L2W_SendTraficTopic, v.L2W_SendTraficTopic) << std::endl
       << TX_VARS_NAME(LaneKeepA, v.LaneKeepA) << std::endl
       << TX_VARS_NAME(LaneKeepB, v.LaneKeepB) << std::endl

       << TX_VARS_NAME(W74_DEFAULT_ax, v.W74_DEFAULT_ax) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_bx_add, v.W74_DEFAULT_bx_add) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_bx_mult, v.W74_DEFAULT_bx_mult) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_cx, v.W74_DEFAULT_cx) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_ex_add, v.W74_DEFAULT_ex_add) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_ex_mult, v.W74_DEFAULT_ex_mult) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_opdv_mult, v.W74_DEFAULT_opdv_mult) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_bnull_mult, v.W74_DEFAULT_bnull_mult) << std::endl
       << TX_VARS_NAME(W74_DEFAULT_v2cav, v.W74_DEFAULT_v2cav) << std::endl;
    return os;
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(
    vehicle_param, IDM_Vision_Width, IDM_Vision_Length, SideGapMinVelocity, SideFrontVisionDistance,
    SideRearVisionDistance, Friction_Factor, LaneKeep, MAX_SPEED, ChangeTopologyThreshold, NoCheckLCAbortThreshold,
    EmptyTargetLaneGapThreshold, SpeedDenominatorOffset, SafetyGap_Rear, SafetyGap_Front, SafetyGap_Side, ComfortGap,
    Max_Ped_Reaction_Distance, MAX_REACTION_GAP, LaneChanging_Aggressive_Threshold, Tolerance_Threshold,
    Max_Deceleration, Signal_Reaction_Gap, Max_Comfort_Deceleration, Yielding_Aggressive_Threshold, L2W_Switch_Time,
    L2W_Speed_Ratio, L2W_Speed_Ratio_Upper, L2W_Log2worldsim_Timestep, L2W_Log2worldsim_Duration, SceneMaxVehicleSize,
    L2W_SendTraficTopic, LaneKeepA, LaneKeepB, W74_DEFAULT_ax, W74_DEFAULT_bx_add, W74_DEFAULT_bx_mult, W74_DEFAULT_cx,
    W74_DEFAULT_ex_add, W74_DEFAULT_ex_mult, W74_DEFAULT_opdv_mult, W74_DEFAULT_bnull_mult, W74_DEFAULT_v2cav,
    g_Driving_Parameters);

TX_NAMESPACE_CLOSE(Cfg)

TX_NAMESPACE_OPEN(Utils)

Cfg::vehicle_param g_vehicle_param;

Base::txBool VehicleBehaviorCfg::Initialize(const Base::txString cfg_value,
                                            ConfigType cfg_type = ConfigType::file_path) TX_NOEXCEPT {
  // 如果配置类型为文件路径
  if (ConfigType::file_path == cfg_type) {
    return Initialize_json_path(cfg_value);
  } else if (ConfigType::json_content == cfg_type) {  // 如果配置类型为JSON内容
    return Initialize_json_content(cfg_value);
  } else {
    LOG(FATAL) << "un support cfg type" << Utils::to_underlying(cfg_type);
    return false;
  }
  return true;
}

Base::txBool VehicleBehaviorCfg::Initialize_json_path(const Base::txString cfg_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 如果配置文件存在
  if (exists(FilePath(cfg_path))) {
    std::ifstream in_json(cfg_path);
    nlohmann::json re_parse_json = nlohmann::json::parse(in_json);
    // 使用解析后的JSON对象初始化车辆行为配置
    return Initialize_json_object(re_parse_json TX_MARK("Cfg::from_json(re_parse_json, g_vehicle_param);"));
  } else {
    LOG(INFO) << "[vehicle_params][not find param file][use default value] " << std::endl << Str();
  }
  return true;
}

Base::txBool VehicleBehaviorCfg::Initialize_json_object(const nlohmann::json& jonObj) TX_NOEXCEPT {
  Cfg::from_json(jonObj, g_vehicle_param);
  // 计算并设置L2W_Switch_EndTime
  g_vehicle_param.L2W_Switch_EndTime = g_vehicle_param.L2W_Switch_Time + g_vehicle_param.L2W_Log2worldsim_Duration;
  // 获取并设置LaneKeepA和LaneKeepB标志
  FLAGS_LK_A = LaneKeepA();
  FLAGS_LK_B = LaneKeepB();
  LOG(INFO) << "[vehicle_params][find param configure file] " << std::endl
            << Str() << std::endl
            << TX_VARS(FLAGS_LK_A) << std::endl
            << TX_VARS(FLAGS_LK_B) << std::endl;
  return true;
}

Base::txBool VehicleBehaviorCfg::Initialize_json_content(const Base::txString json_content) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 如果JSON字符串非空
  if (_NonEmpty_(json_content)) {
    nlohmann::json re_parse_json = nlohmann::json::parse(json_content);
    // 使用解析后的JSON对象初始化车辆行为配置
    return Initialize_json_object(re_parse_json TX_MARK("Cfg::from_json(re_parse_json, g_vehicle_param);"));
  } else {
    LOG(INFO) << "[vehicle_params][json_content_null][use default value] " << std::endl << Str();
  }
  return true;
}

Base::txBool VehicleBehaviorCfg::SetVisualizerParams(const Base::txFloat CC0, const Base::txFloat CC1,
                                                     const Base::txFloat CC2) TX_NOEXCEPT {
  // 设置g_Driving_Parameters数组的前三个元素为CC0、CC1和CC2
  g_vehicle_param.g_Driving_Parameters[0] = CC0;
  g_vehicle_param.g_Driving_Parameters[1] = CC1;
  g_vehicle_param.g_Driving_Parameters[2] = CC2;
  LOG(INFO) << "[vehicle_params][SetVisualizerParams] " << TX_VARS(CC0) << TX_VARS(CC1) << TX_VARS(CC2);
  LOG(INFO) << "[vehicle_params][SetVisualizerParams] All Params : " << std::endl << Str();
  return true;
}

Base::txString VehicleBehaviorCfg::Str() TX_NOEXCEPT {
  std::ostringstream oss;
  oss << TX_VARS(IDM_Vision_Length()) << std::endl
      << TX_VARS(IDM_Vision_Width()) << std::endl

      << TX_VARS(SideFrontVisionDistance()) << std::endl
      << TX_VARS(SideRearVisionDistance()) << std::endl
      << TX_VARS(SideGapMinVelocity()) << std::endl

      << TX_VARS(Friction_Factor()) << std::endl
      << TX_VARS(LaneKeep()) << std::endl
      << TX_VARS(MAX_SPEED()) << std::endl
      << TX_VARS(ChangeTopologyThreshold()) << std::endl
      << TX_VARS(NoCheckLCAbortThreshold()) << std::endl
      << TX_VARS(EmptyTargetLaneGapThreshold()) << std::endl
      << TX_VARS(SpeedDenominatorOffset()) << std::endl

      << TX_VARS(g_Driving_Parameters(0)) << std::endl
      << TX_VARS(g_Driving_Parameters(1)) << std::endl
      << TX_VARS(g_Driving_Parameters(2)) << std::endl
      << TX_VARS(g_Driving_Parameters(3)) << std::endl
      << TX_VARS(g_Driving_Parameters(4)) << std::endl
      << TX_VARS(g_Driving_Parameters(5)) << std::endl
      << TX_VARS(g_Driving_Parameters(6)) << std::endl
      << TX_VARS(g_Driving_Parameters(7)) << std::endl
      << TX_VARS(g_Driving_Parameters(8)) << std::endl
      << TX_VARS(g_Driving_Parameters(9)) << std::endl

      << TX_VARS(SafetyGap_Rear()) << std::endl
      << TX_VARS(SafetyGap_Front()) << std::endl
      << TX_VARS(SafetyGap_Side()) << std::endl
      << TX_VARS(ComfortGap()) << std::endl
      << TX_VARS(Max_Ped_Reaction_Distance()) << std::endl
      << TX_VARS(MAX_REACTION_GAP()) << std::endl
      << TX_VARS(LaneChanging_Aggressive_Threshold()) << std::endl
      << TX_VARS(Tolerance_Threshold()) << std::endl
      << TX_VARS(Max_Deceleration()) << std::endl
      << TX_VARS(Signal_Reaction_Gap()) << std::endl
      << TX_VARS(Max_Comfort_Deceleration()) << std::endl
      << TX_VARS(Yielding_Aggressive_Threshold()) << std::endl

      << TX_VARS(L2W_Switch_Time()) << std::endl
      << TX_VARS(L2W_Speed_Ratio()) << std::endl
      << TX_VARS(L2W_Speed_Ratio_Upper()) << std::endl
      << TX_VARS(L2W_Log2worldsim_Timestep()) << std::endl
      << TX_VARS(L2W_Log2worldsim_Duration()) << std::endl
      << TX_VARS(SceneMaxVehicleSize()) << std::endl
      << TX_COND(L2W_SendTraficTopic()) << std::endl
      << TX_VARS(LaneKeepA()) << std::endl
      << TX_VARS(LaneKeepB()) << std::endl

      << TX_VARS(W74_DEFAULT_ax()) << std::endl
      << TX_VARS(W74_DEFAULT_bx_add()) << std::endl
      << TX_VARS(W74_DEFAULT_bx_mult()) << std::endl
      << TX_VARS(W74_DEFAULT_cx()) << std::endl
      << TX_VARS(W74_DEFAULT_ex_add()) << std::endl
      << TX_VARS(W74_DEFAULT_ex_mult()) << std::endl
      << TX_VARS(W74_DEFAULT_opdv_mult()) << std::endl
      << TX_VARS(W74_DEFAULT_bnull_mult()) << std::endl
      << TX_COND(W74_DEFAULT_v2cav()) << std::endl;

  return oss.str();
}

Base::txFloat VehicleBehaviorCfg::IDM_Vision_Length() TX_NOEXCEPT { return g_vehicle_param.IDM_Vision_Length; }

Base::txFloat VehicleBehaviorCfg::IDM_Vision_Width() TX_NOEXCEPT { return g_vehicle_param.IDM_Vision_Width; }

Base::txFloat VehicleBehaviorCfg::SideGapMinVelocity() TX_NOEXCEPT { return g_vehicle_param.SideGapMinVelocity; }

Base::txFloat VehicleBehaviorCfg::SideFrontVisionDistance() TX_NOEXCEPT {
  return g_vehicle_param.SideFrontVisionDistance;
}

Base::txFloat VehicleBehaviorCfg::SideRearVisionDistance() TX_NOEXCEPT {
  return g_vehicle_param.SideRearVisionDistance;
}

Base::txFloat VehicleBehaviorCfg::Friction_Factor() TX_NOEXCEPT { return g_vehicle_param.Friction_Factor; }

Base::txFloat VehicleBehaviorCfg::LaneKeep() TX_NOEXCEPT { return g_vehicle_param.LaneKeep; }

Base::txFloat VehicleBehaviorCfg::MAX_SPEED() TX_NOEXCEPT { return g_vehicle_param.MAX_SPEED; }

Base::txFloat VehicleBehaviorCfg::ChangeTopologyThreshold() TX_NOEXCEPT {
  return g_vehicle_param.ChangeTopologyThreshold;
}

Base::txFloat VehicleBehaviorCfg::NoCheckLCAbortThreshold() TX_NOEXCEPT {
  return g_vehicle_param.NoCheckLCAbortThreshold;
}

Base::txFloat VehicleBehaviorCfg::EmptyTargetLaneGapThreshold() TX_NOEXCEPT {
  return g_vehicle_param.EmptyTargetLaneGapThreshold;
}

Base::txFloat VehicleBehaviorCfg::SpeedDenominatorOffset() TX_NOEXCEPT {
  return g_vehicle_param.SpeedDenominatorOffset;
}

Base::txFloat VehicleBehaviorCfg::g_Driving_Parameters(const Base::txInt idx) TX_NOEXCEPT {
  return g_vehicle_param.g_Driving_Parameters[idx];
}

Base::txFloat VehicleBehaviorCfg::SafetyGap_Rear() TX_NOEXCEPT { return g_vehicle_param.SafetyGap_Rear; }

Base::txFloat VehicleBehaviorCfg::SafetyGap_Front() TX_NOEXCEPT { return g_vehicle_param.SafetyGap_Front; }

Base::txFloat VehicleBehaviorCfg::SafetyGap_Side() TX_NOEXCEPT { return g_vehicle_param.SafetyGap_Side; }

Base::txFloat VehicleBehaviorCfg::ComfortGap() TX_NOEXCEPT { return g_vehicle_param.ComfortGap; }

Base::txFloat VehicleBehaviorCfg::Max_Ped_Reaction_Distance() TX_NOEXCEPT {
  return g_vehicle_param.Max_Ped_Reaction_Distance;
}

Base::txFloat VehicleBehaviorCfg::MAX_REACTION_GAP() TX_NOEXCEPT { return g_vehicle_param.MAX_REACTION_GAP; }

Base::txFloat VehicleBehaviorCfg::LaneChanging_Aggressive_Threshold() TX_NOEXCEPT {
  return g_vehicle_param.LaneChanging_Aggressive_Threshold;
}

Base::txFloat VehicleBehaviorCfg::Tolerance_Threshold() TX_NOEXCEPT { return g_vehicle_param.Tolerance_Threshold; }

Base::txFloat VehicleBehaviorCfg::Max_Deceleration() TX_NOEXCEPT { return g_vehicle_param.Max_Deceleration; }

Base::txFloat VehicleBehaviorCfg::Signal_Reaction_Gap() TX_NOEXCEPT { return g_vehicle_param.Signal_Reaction_Gap; }

Base::txFloat VehicleBehaviorCfg::Max_Comfort_Deceleration() TX_NOEXCEPT {
  return g_vehicle_param.Max_Comfort_Deceleration;
}

Base::txFloat VehicleBehaviorCfg::Yielding_Aggressive_Threshold() TX_NOEXCEPT {
  return g_vehicle_param.Yielding_Aggressive_Threshold;
}

void VehicleBehaviorCfg::Set_L2W_Switch_Time(Base::txFloat _time) TX_NOEXCEPT {
  g_vehicle_param.L2W_Switch_Time = _time;
}

Base::txFloat VehicleBehaviorCfg::L2W_Switch_Time() TX_NOEXCEPT { return g_vehicle_param.L2W_Switch_Time; }

Base::txFloat VehicleBehaviorCfg::L2W_Switch_EndTime() TX_NOEXCEPT { return g_vehicle_param.L2W_Switch_EndTime; }

void VehicleBehaviorCfg::Set_L2W_Switch_EndTime(Base::txFloat _time) TX_NOEXCEPT {
  g_vehicle_param.L2W_Switch_EndTime = _time;
}

Base::txFloat VehicleBehaviorCfg::L2W_Speed_Ratio() TX_NOEXCEPT { return g_vehicle_param.L2W_Speed_Ratio; }

Base::txFloat VehicleBehaviorCfg::L2W_Speed_Ratio_Upper() TX_NOEXCEPT { return g_vehicle_param.L2W_Speed_Ratio_Upper; }

Base::txFloat VehicleBehaviorCfg::L2W_Log2worldsim_Timestep() TX_NOEXCEPT {
  return g_vehicle_param.L2W_Log2worldsim_Timestep;
}

Base::txFloat VehicleBehaviorCfg::L2W_Log2worldsim_Duration() TX_NOEXCEPT {
  return g_vehicle_param.L2W_Log2worldsim_Duration;
}

Base::txInt VehicleBehaviorCfg::SceneMaxVehicleSize() TX_NOEXCEPT { return g_vehicle_param.SceneMaxVehicleSize; }

Base::txBool VehicleBehaviorCfg::L2W_SendTraficTopic() TX_NOEXCEPT { return g_vehicle_param.L2W_SendTraficTopic; }

Base::txFloat VehicleBehaviorCfg::LaneKeepA() TX_NOEXCEPT { return g_vehicle_param.LaneKeepA; }

Base::txFloat VehicleBehaviorCfg::LaneKeepB() TX_NOEXCEPT { return g_vehicle_param.LaneKeepB; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_ax() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_ax; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_bx_add() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_bx_add; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_bx_mult() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_bx_mult; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_cx() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_cx; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_ex_add() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_ex_add; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_ex_mult() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_ex_mult; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_opdv_mult() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_opdv_mult; }

Base::txFloat VehicleBehaviorCfg::W74_DEFAULT_bnull_mult() TX_NOEXCEPT {
  return g_vehicle_param.W74_DEFAULT_bnull_mult;
}

Base::txBool VehicleBehaviorCfg::W74_DEFAULT_v2cav() TX_NOEXCEPT { return g_vehicle_param.W74_DEFAULT_v2cav; }

TX_NAMESPACE_CLOSE(Utils)

#if __TX_Mark__("BehaviorCfgDB")

TX_NAMESPACE_OPEN(Cfg)

struct vehicle_behavior_cfg {
  Base::txString eventId;
  Cfg::vehicle_param cfg;
  friend std::ostream& operator<<(std::ostream& os, const vehicle_behavior_cfg& v) TX_NOEXCEPT {
    os << TX_VARS_NAME(eventId, v.eventId) << std::endl << TX_VARS_NAME(cfg, v.cfg);
    return os;
  }
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(vehicle_behavior_cfg, eventId, cfg);

struct vehicle_behavior_db {
  Base::txString cfg_version = Base::txString("20220718");
  Base::txString pulic_data;
  std::vector<vehicle_behavior_cfg> content;
  std::map<Base::txString, vehicle_behavior_cfg> map_eventId2cfg;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(vehicle_behavior_db, cfg_version, pulic_data, content);

TX_NAMESPACE_CLOSE(Cfg)

TX_NAMESPACE_OPEN(Utils)

Cfg::vehicle_behavior_db g_vehicle_behavior_db;

Base::txBool BehaviorCfgDB::Initialize(const Base::txString cfg_value, ConfigType cfg_type) TX_NOEXCEPT {
  // 如果配置类型为file_path，表示配置值为配置文件路径
  if (ConfigType::file_path == cfg_type) {
    return Initialize_json_path(cfg_value);
  } else if (ConfigType::json_content == cfg_type) {  // 如果配置类型为json_content，表示配置值为JSON字符串
    return Initialize_json_content(cfg_value);
  } else {
    LOG(FATAL) << "un support cfg type" << Utils::to_underlying(cfg_type);
    return false;
  }
  return true;
}

// chomper
Base::txBool BehaviorCfgDB::Initialize(const Base::txString cfg_value, ConfigType cfg_type,
                                       const Base::txString strCfgId, const Base::txFloat cc0, const Base::txFloat cc1,
                                       const Base::txFloat cc2) TX_NOEXCEPT {
  // 使用给定的配置类型和配置值初始化车辆行为配置
  if (CallSucc(Initialize(cfg_value, cfg_type))) {
    // 如果配置ID存在于g_vehicle_behavior_db.map_eventId2cfg中
    if (_Contain_(g_vehicle_behavior_db.map_eventId2cfg, strCfgId)) {
      auto& refCfgParams = g_vehicle_behavior_db.map_eventId2cfg[strCfgId];
      // 设置可视化参数
      refCfgParams.cfg.g_Driving_Parameters[0] = cc0;
      refCfgParams.cfg.g_Driving_Parameters[1] = cc1;
      refCfgParams.cfg.g_Driving_Parameters[2] = cc2;
      return true;
    } else {
      LOG(WARNING) << "can not find cfg name: " << TX_VARS(strCfgId);
      return false;
    }
  } else {
    LOG(WARNING) << "Parse cfg failure. " << TX_VARS(cfg_value);
    return false;
  }
}

Base::txBool BehaviorCfgDB::Initialize_json_path(const Base::txString cfg_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 如果给定的JSON文件路径存在
  if (exists(FilePath(cfg_path))) {
    std::ifstream in_json(cfg_path);
    nlohmann::json re_parse_json = nlohmann::json::parse(in_json);
    // 使用解析后的JSON对象初始化车辆行为配置
    return Initialize_json_object(re_parse_json TX_MARK("Cfg::from_json(re_parse_json, g_vehicle_param);"));
  } else {
    LOG(WARNING) << "[vehicle_params][not find param file][use default value] " << std::endl << TX_VARS(cfg_path);
    // << Str();
  }
  return true;
}

Base::txBool BehaviorCfgDB::Initialize_json_content(const Base::txString cfg_content) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 如果给定的JSON字符串非空
  if (_NonEmpty_(cfg_content)) {
    nlohmann::json re_parse_json = nlohmann::json::parse(cfg_content);
    // 使用解析后的JSON对象初始化车辆行为配置
    return Initialize_json_object(re_parse_json TX_MARK("Cfg::from_json(re_parse_json, g_vehicle_param);"));
  } else {
    LOG(WARNING) << "[vehicle_params][json_content_null][use default value] " << std::endl;  // << Str();
  }
  return true;
}

Base::txBool BehaviorCfgDB::Initialize_json_object(const nlohmann::json& jonObj) TX_NOEXCEPT {
  Cfg::from_json(jonObj, g_vehicle_behavior_db);
  g_vehicle_behavior_db.map_eventId2cfg.clear();
  // 遍历g_vehicle_behavior_db.content中的每个配置
  for (const auto& refCfg : g_vehicle_behavior_db.content) {
    // 将配置添加到g_vehicle_behavior_db.map_eventId2cfg中，以eventId为键
    g_vehicle_behavior_db.map_eventId2cfg[refCfg.eventId] = refCfg;
  }
  LOG(INFO) << "[vehicle_params][find param configure file] " << std::endl;  // << Str() << std::endl;
  return true;
}

Base::txString BehaviorCfgDB::Str() TX_NOEXCEPT {
  std::ostringstream oss;
  oss << "[BehaviorCfgDB] " << std::endl
      << TX_VARS_NAME(cfg_version, cfg_version()) << TX_VARS_NAME(pulic_data, pulic_data()) << std::endl;
  for (const auto& refCfg : g_vehicle_behavior_db.content) {
    oss << refCfg << std::endl;
  }
  return oss.str();
}

Base::txString BehaviorCfgDB::cfg_version() TX_NOEXCEPT { return g_vehicle_behavior_db.cfg_version; }

Base::txString BehaviorCfgDB::pulic_data() TX_NOEXCEPT { return g_vehicle_behavior_db.pulic_data; }

Base::txBool BehaviorCfgDB::HasVehicleCfg(const Base::txString& _strEventId) TX_NOEXCEPT {
  if (_Contain_(g_vehicle_behavior_db.map_eventId2cfg, _strEventId)) {
    return true;
  } else {
    LOG(WARNING) << "do not find vehicle cfg " << TX_VARS(_strEventId);
    return false;
  }
}

Base::txBool BehaviorCfgDB::SetVehicleCfg(const Base::txString& _strEventId) TX_NOEXCEPT {
  if (_Contain_(g_vehicle_behavior_db.map_eventId2cfg, _strEventId)) {
    g_vehicle_param = g_vehicle_behavior_db.map_eventId2cfg.at(_strEventId).cfg;
    LOG(INFO) << "[SetVehicleBehaviorCfg] " << _strEventId << std::endl;  // << g_vehicle_param;
    return true;
  } else {
    LOG(WARNING) << "do not find vehicle cfg " << TX_VARS(_strEventId);
    return false;
  }
}

Base::txBool BehaviorCfgDB::ResetDefaultCfg() TX_NOEXCEPT { return SetVehicleCfg("normal"); }

#endif /*__TX_Mark__("BehaviorCfgDB")*/
TX_NAMESPACE_CLOSE(Utils)
