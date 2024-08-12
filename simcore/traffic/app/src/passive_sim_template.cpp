// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "passive_sim_template.h"
#include <chrono>
#include <thread>
#include "HdMap/tx_hashed_lane_info.h"
#include "HdMap/tx_hashed_road.h"
#include "SceneLoader/tx_tadloader.h"
#include "tad_event_dispatcher_1_0_0_0.h"
#include "tad_stand_alone_traffic_element_system.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_application.h"
#include "tx_class_counter.h"
#include "tx_event_dispatcher.h"
#include "tx_scene_loader_factory.h"
#include "tx_spatial_query.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_TADSim_Loop)
#define SceneLoaderLogInfo LOG_IF(INFO, FLAGS_LogLevel_SceneLoader)
#define LogWarn LOG(WARNING)
TX_NAMESPACE_OPEN(TrafficFlow)

void txPassiveSimTemplate::ResetVarsPassive(Base::TrafficElementSystemPtr _sys_ptr) TX_NOEXCEPT {
  // 初始化变量
  m_step_count_ = 0;
  m_max_step_count_ = 100;
  m_payload_.clear();
  m_lastStepTime = 0.0;
  Base::ITrafficElement::ResetSysId();
  m_passTime = 0.0;
  // 初始化事件分发器
  TrafficFlow::InitializeEventDispatcher();

  // 释放HdMap相关资源
  HdMap::HadmapCacheConCurrent::Release();
  HdMap::txRoadNetwork::Release();
#if USE_RTree
  Geometry::SpatialQuery::ClearSpatialQueryInfo();
#endif
#if USE_HashedRoadNetwork
  HdMap::HashedRoadCacheConCurrent::Release();
#endif /*USE_HashedRoadNetwork*/

  // 初始化角度表
  Unit::IAngle::GenerateSinCosLookupTable();

  // 初始化行为配置数据库
  if (_NonEmpty_(m_inject_vehicle_behavior_db)) {
    const std::string str = RawVehicleBehaviorJson();
    LOG(INFO) << "TAD_Vehicle_Behavior content : " << str;
    Utils::BehaviorCfgDB::Initialize(str, Utils::ConfigType::json_content);
    Utils::BehaviorCfgDB::ResetDefaultCfg();
  } else {
    const auto cfg_path = txApplication::AppAbstractPath() + Base::txString("/") + FLAGS_behavior_cfg_name;
    LOG(INFO) << "TAD_Vehicle_Behavior config : " << cfg_path;
    Utils::BehaviorCfgDB::Initialize(cfg_path, Utils::ConfigType::file_path);
    Utils::BehaviorCfgDB::ResetDefaultCfg();
  }
  LOG(WARNING) << "#############################################" << std::endl
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfo>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Base::ITrafficElement>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Geometry::SpatialQuery::HashedLaneInfoOrthogonalList>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<HdMap::txLaneInfoInterface>::howMany()) << std::endl
               << TX_VARS(Utils::Counter<Base::JointPointTreeNode>::howMany()) << std::endl
               << "###########################################################";
}

Base::txString txPassiveSimTemplate::RawVehicleBehaviorJson() const TX_NOEXCEPT {
  std::string str = m_inject_vehicle_behavior_db;
  /*std::regex pattern_1(R"(\\n)");
  str = std::regex_replace(str, pattern_1, Base::txString(""));
  std::regex pattern_2(R"(\\)");
  str = std::regex_replace(str, pattern_2, Base::txString(""));*/
  Utils::StringReplaceRegex(str, R"(\\n)", "");
  Utils::StringReplaceRegex(str, R"(\\\")", "\"");
  Utils::trim(str);
  if (_NonEmpty_(str) && ('\"') == (str.front())) {
    str.erase(str.begin());
  }
  if (_NonEmpty_(str) && ('\"') == (str.back())) {
    str.pop_back();
  }
  return str;
}

void txPassiveSimTemplate::CreateSystemPassive(const Base::txString sim_path, const Base::txString scene_path,
                                               const Base::txString map_path) TX_NOEXCEPT {
  // 创建交通元素系统实例。
  m_TrafficElementSystem_Ptr = std::make_shared<TAD_StandAlone_TrafficElementSystem>();

  // 检查交通元素系统实例是否为非空指针，记录日志。
  if (NonNull_Pointer(m_TrafficElementSystem_Ptr)) {
    LogInfo << "Create TrafficElementSystem Success.";
  } else {
    LogWarn << "Create TrafficElementSystem Failure.";
  }
}

// 加载场景文件、地图文件并进行解析处理。
Base::txBool txPassiveSimTemplate::CreateEnvPassive(Base::ISceneLoaderPtr loader, const Base::txString sim_path,
                                                    const Base::txString scene_path,
                                                    const Base::txString map_path) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::filesystem;
  // 保证涉及的文件存在，并记录日志
  if (NonNull_Pointer(loader) && CallSucc(exists(FilePath(sim_path))) && CallSucc(exists(FilePath(scene_path))) &&
      CallSucc(exists(FilePath(map_path)))) {
    if (NonNull_Pointer(loader) && CallSucc(loader->Load(sim_path, scene_path, map_path))) {
      if (NonNull_Pointer(TrafficSystemPtrPassive())) {
        HdMap::HadmapCacheConCurrent::InitParams_t initParams;
        if (CallSucc(loader->GetMapManagerInitParams(initParams))) {
          initParams.strHdMapFilePath = map_path;
#if ON_LOCAL
          if (FLAGS_LogLevel_MapFile) {
            HdMap::HadmapCacheConCurrent::PrintMapFile(initParams.strHdMapFilePath);
          }
#endif /*ON_LOCAL*/
          // 如果 HadmapCacheConCurrent 初始化成功
          if (HdMap::HadmapCacheConCurrent::Initialize(initParams)) {
            // 如果 TrafficSystemPtrPassive 初始化成功
            if (TrafficSystemPtrPassive()->Initialize(loader)) {
              // 输出日志，表示交通流量模拟系统初始化成功
              LogInfo << "Traffic Flow Simulation System Init Success.";
              return true;
            } else {
              // 输出日志，表示 TrafficElementSystem 初始化失败
              LogWarn << "TrafficElementSystem Init Failure.";
              return false;
            }
          } else {
            LogWarn << "MapManager Init Failure.";
            return false;
          }
        } else {
          LogWarn << "GetMapManagerInitParams Failure.";
          return false;
        }
      } else {
        LogWarn << "TrafficElementSystem is Null.";
        return false;
      }
    } else {
      LOG(WARNING) << "loader->Load(params.scenariopath()) return false." << TX_VARS(sim_path) << TX_VARS(scene_path)
                   << TX_VARS(map_path);
      return false;
    }
  } else {
    LogWarn << "CreateEnvPassive failure. " << TX_COND(NonNull_Pointer(loader)) << TX_COND(exists(FilePath(sim_path)))
            << TX_COND(exists(FilePath(scene_path))) << TX_COND(exists(FilePath(map_path)));
    return false;
  }
}

Base::TimeParamManager txPassiveSimTemplate::MakeTimeMgrPassive(const Base::txFloat timeStamp_ms) TX_NOEXCEPT {
  // 获取相对时间
  const Base::txFloat relativeTime = Utils::MillisecondToSecond(timeStamp_ms - m_lastStepTime);

  // 更新经过的时间
  m_passTime += relativeTime;

  // 创建并返回时间管理器对象
  Base::TimeParamManager timeMgr(Utils::MillisecondToSecond(timeStamp_ms), relativeTime, m_passTime, timeStamp_ms);
  // LOG(WARNING) << "[simulation_time] : " << timeMgr.str() << TX_VARS(time_stamp);
  return timeMgr;
}

Base::txBool txPassiveSimTemplate::SimulationPassive(const Base::txFloat timeStamp_ms, sim_msg::Location* loc_pb_ptr,
                                                     sim_msg::Traffic* traffic_pb_ptr,
                                                     sim_msg::Trajectory* traj_pb_ptr) TX_NOEXCEPT {
  // 判断交通系统是否为非空指针且调用成功，如果满足条件则进行以下操作
  if (NonNull_Pointer(TrafficSystemPtrPassive()) && CallSucc(TrafficSystemPtrPassive()->IsAlive())) {
    // 根据传入的时间戳创建一个时间管理器
    const Base::TimeParamManager timeMgr = MakeTimeMgrPassive(timeStamp_ms);
    timeMgr.str();
    // 调用被动模拟的交通系统，传入时间管理器和交通数据指针
    SimulationTrafficPassive(timeMgr, traffic_pb_ptr);
    return true;
  } else {
    LOG(WARNING) << "m_TrafficElementSystemPtr is not initialized, Send 0 elements.";
    return false;
  }
}

void txPassiveSimTemplate::SimulationTrafficPassive(const Base::TimeParamManager& timeMgr,
                                                    sim_msg::Traffic* traffic_pb_ptr) TX_NOEXCEPT {
  // 调用被动模拟下的交通系统更新函数，更新交通系统状态
  const Base::txBool res = TrafficSystemPtrPassive()->Update(timeMgr);

  // 将更新后的交通数据填充到传入的交通数据指针
  TrafficSystemPtrPassive()->FillingTrafficData(timeMgr, *traffic_pb_ptr);

  // 更新空间查询
  TrafficSystemPtrPassive()->FillingSpatialQuery();

  // 如果启用了概率查询功能，则更新概率查询
  if (FLAGS_Enable_Sketch) {
    const auto res_size = TrafficSystemPtrPassive()->UpdateSketch(timeMgr, *traffic_pb_ptr);
    LOG_IF(INFO, FLAGS_LogLevel_Sketch) << "[Sketch] UpdateSketch return " << TX_VARS(res_size);
  }
  // 记录最后一步的时间戳
  m_lastStepTime = timeMgr.TimeStamp();
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef SceneLoaderLogInfo
#undef LogWarn
