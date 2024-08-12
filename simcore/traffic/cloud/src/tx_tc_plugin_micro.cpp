// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_tc_plugin_micro.h"
#include <boost/dll/runtime_symbol_info.hpp>
#include <memory>
#include <sstream>
#include "boost/dll/alias.hpp"
#include "boost/make_shared.hpp"
#include "boost/shared_ptr.hpp"
#include "traffic.pb.h"
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_path_utils.h"
#include "tx_protobuf_utils.h"
#include "tx_scene_loader_factory.h"
#include "tx_sim_point.h"
#include "tx_tc_marco.h"
#include "tx_time_utils.h"
#include "tx_timer_on_cpu.h"
#include "tx_traffic_element_base.h"
#include "tx_traffic_element_system.h"

#if USE_TBB
#  include "tbb/blocked_range.h"
#  include "tbb/concurrent_hash_map.h"
#  include "tbb/concurrent_unordered_set.h"
#  include "tbb/parallel_for.h"
#  if TBB_NEW_API
#    define TBB_PREVIEW_GLOBAL_CONTROL 1
#    include <tbb/global_control.h>
#  else
#    include "tbb/task_scheduler_init.h"
#  endif
#  include "tx_hash_utils.h"
#endif

#if USE_RTree
#  include "tx_spatial_query.h"
#endif

#include <mutex>
#include "tad_ego_vehicle_element.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_gflags.h"
#include "tx_tc_init_params.h"
#include "tx_tc_map_range_utils.h"
#include "tx_tc_traffic_system.h"

#include "tx_simulation_loop.h"
#include "tx_tc_template.h"

#if __pingsn__
#  include "parallel_simulation/tx_tc_ps_od_info.h"
#  include "parallel_simulation/tx_tc_ps_traffic_light_element.h"
#  include "parallel_simulation/tx_tc_ps_traffic_record.h"
#  include "parallel_simulation/tx_tc_ps_traffic_system.h"
#  include "parallel_simulation/tx_tc_ps_vehicle_element.h"
#endif /*__pingsn__*/
#include "tad_vehicle_behavior_configure_utils.h"
#include "version.h"

#define CloudDitwInfo LOG_IF(INFO, FLAGS_LogLevel_CloudDitw) << "[cloud_ditw_debug]"
#define MultiRegionInfo LOG_IF(INFO, FLAGS_LogLevel_MultiRegion)
TX_NAMESPACE_OPEN(Cloud)
class tx_tc_traffic_impl : public TrafficCloud::TrafficManager {
  using TrafficManager = TrafficCloud::TrafficManager;

 public:
  static boost::shared_ptr<TrafficManager> create(void) { return boost::make_shared<tx_tc_traffic_impl>(); }

 public:
  using parent = TrafficManager;
  using InitInfo = parent::InitInfo;
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;
  using SimulationMode = Base::txSimulationLoop::SimulationMode;

  tx_tc_traffic_impl(void) : TrafficManager() {
#if USE_TBB
#  if TBB_NEW_API
    if (0 >= FLAGS_MAX_THREAD_NUM) {
      LOG(INFO) << "[MultiThread] thread_num =  auto.";
    } else if (FLAGS_MAX_THREAD_NUM > 0) {
      tbb::global_control control(tbb::global_control::max_allowed_parallelism, FLAGS_MAX_THREAD_NUM);
      LOG(INFO) << "[MultiThread] thread_num = " << FLAGS_MAX_THREAD_NUM;
    }
#  else
    if (0 >= FLAGS_MAX_THREAD_NUM) {
      tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);
      LOG(INFO) << "[MultiThread] thread_num =  auto.";
    } else if (FLAGS_MAX_THREAD_NUM > 0) {
      tbb::task_scheduler_init init(FLAGS_MAX_THREAD_NUM);
      LOG(INFO) << "[MultiThread] thread_num = " << FLAGS_MAX_THREAD_NUM;
    }
#  endif
#endif
#if 0
        FLAGS_log_dir = "d:/log";
        /*2. set log file name with app name*/
        /*std::string strLogName = std::string("d:/log") + "/" + std::string("cloud") + ".";
        google::SetLogDestination(google::GLOG_INFO, strLogName.c_str());
        google::SetLogDestination(google::GLOG_WARNING, strLogName.c_str());
        google::SetLogDestination(google::GLOG_ERROR, strLogName.c_str());
        google::SetLogDestination(google::GLOG_FATAL, strLogName.c_str());*/

        FLAGS_logtostderr = 0;

        FLAGS_logbufsecs = 0;

        FLAGS_max_log_size = 1000;

        FLAGS_stop_logging_if_full_disk = true;

        // google::SetLogFilenameExtension("stlog");

        /*FLAGS_logtostderr = true;*/
        FLAGS_alsologtostderr = false;

        FLAGS_colorlogtostderr = false;

        google::InitGoogleLogging("micro_traffic_cloud");

        LOG(INFO) << "Log init success.";
#endif
    LOG(WARNING) << TX_VARS_NAME(Load_Module_Path, boost::dll::program_location())
                 << TX_VARS_NAME(module_name, module_name()) << TX_VARS_NAME(module_version, module_version());
  }

  virtual ~tx_tc_traffic_impl() { LOG(WARNING) << TX_VARS_NAME(Module, FLAGS_app_name) << " Release."; }

  virtual const char* module_name(void) const noexcept override {
    return FILE_VERSION_STR(STR_GIT_BRANCH, STR_COMMIT_ID);
  }

  virtual const char* module_version(void) const noexcept override { return _ST(PRO_PATH); }

 protected:
  SimulationMode m_SimMode = SimulationMode::eNone;
  TrafficFlow::PluginTemplatePtr m_SimPtr = nullptr;
  std::mutex mInterfaceMtx;
  Scene::InitInfoWrap inputeParams;

 protected:
  Base::txBool isParallelSimulation() const TX_NOEXCEPT { return inputeParams.isParallelSimulation(); }
  Base::txBool isVirtualCity() const TX_NOEXCEPT { return inputeParams.isVirtualCity(); }
  TrafficFlow::CloudTrafficElementSystemPtr CloudTrafficSystemPtr() TX_NOEXCEPT {
    if (NonNull_Pointer(m_SimPtr) && CallSucc(isVirtualCity()) && NonNull_Pointer(m_SimPtr->CloudTrafficSystemPtr())) {
      return (m_SimPtr->CloudTrafficSystemPtr());
    } else {
      return nullptr;
    }
  }
  TrafficFlow::CloudTrafficElementSystemPtr CloudTrafficSystem_RawPtr() TX_NOEXCEPT {
    return (m_SimPtr->CloudTrafficSystemPtr());
  }
  TrafficFlow::ParallelSimulation_TrafficElementSystemPtr ParallelSimulationSystemPtr() TX_NOEXCEPT {
    if (NonNull_Pointer(m_SimPtr) && CallSucc(isParallelSimulation()) &&
        NonNull_Pointer(m_SimPtr->ParallelSimulationSystemPtr())) {
      return (m_SimPtr->ParallelSimulationSystemPtr());
    } else {
      return nullptr;
    }
  }
  TrafficFlow::ParallelSimulation_TrafficElementSystemPtr ParallelSimulationSystem_RawPtr() TX_NOEXCEPT {
    return m_SimPtr->ParallelSimulationSystemPtr();
  }

  Base::TimeParamManager CurrentStepTimeMgr() const TX_NOEXCEPT { return m_SimPtr->GetTimeMgr(); }

 public:
  virtual ErrCode init(const InitInfo& v) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    LOG(INFO) << "call init";
    inputeParams.Reset(v);
    LOG(INFO) << "Scene Origin GPS =" << Utils::ToString(inputeParams.get_scene_origin_pgs());
    LOG(INFO) << "Scene File mapfile =" << (inputeParams.get_scene_hdmap_path());

    m_SimPtr = std::make_shared<TrafficFlow::PluginTemplate>();
    if (NonNull_Pointer(m_SimPtr)) {
      m_SimPtr->InputeParams() = (inputeParams);
      m_SimPtr->ResetHdMapCache(true);
      sim_msg::Location unuse;
      m_SimPtr->CreateEnv(false, unuse);
    } else {
      LOG(ERROR) << "create system template failure.";
      return TrafficManager::trafficErr;
    }
    return TrafficManager::trafficSucc;
  }

  virtual ErrCode release() noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    LOG(INFO) << "call release";
    m_SimPtr->ResetHdMapCache(true);
    m_SimPtr->ResetVarsPlugin();
    LOG(INFO) << "return release";
    return TrafficManager::trafficSucc;
  }

  virtual ErrCode addEgo(const int ego_id, const sim_msg::Location& info) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    if (NonNull_Pointer(CloudTrafficSystemPtr())) {
      if (CallSucc(CloudTrafficSystemPtr()->RegisterEgoOnCloud(ego_id, info, false))) {
        LOG(INFO) << "addEgo " << TX_VARS(ego_id) << " success.";
        return TrafficManager::trafficSucc;
      } else {
        LOG(WARNING) << "addEgo " << TX_VARS(ego_id) << " failure.";
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(WARNING) << "addEgo " << TX_VARS(ego_id) << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

  virtual ErrCode addEgoForce(const int ego_id, const sim_msg::Location& info) noexcept {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    if (NonNull_Pointer(CloudTrafficSystemPtr())) {
      if (CallSucc(CloudTrafficSystemPtr()->RegisterEgoOnCloud(ego_id, info, true))) {
        LOG(INFO) << "addEgo " << TX_VARS(ego_id) << " success.";
        return TrafficManager::trafficSucc;
      } else {
        LOG(WARNING) << "addEgo " << TX_VARS(ego_id) << " failure.";
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(WARNING) << "addEgo " << TX_VARS(ego_id) << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

  virtual ErrCode deleteEgo(const int ego_id, const int64_t frame_id) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
      if (CallSucc(CloudTrafficSystemPtr()->EraseEgoById(ego_id))) {
        LOG(INFO) << "deleteEgo " << TX_VARS(ego_id) << " success." << TX_VARS(frame_id);
        return TrafficManager::trafficSucc;
      } else {
        LOG(INFO) << "deleteEgo " << TX_VARS(ego_id) << " failure." << TX_VARS(frame_id);
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(INFO) << "deleteEgo " << TX_VARS(ego_id) << " failure. System Not Alive." << TX_VARS(frame_id);
      return TrafficManager::trafficErr;
    }
  }

  virtual ErrCode getTrafficInfoBySurroundingMapPosition(
      sim_msg::MapPosition pos, sim_msg::MergeTrafficInfo& merge_traffic_info) noexcept override {
    return TrafficManager::trafficErr;
  }

  virtual ErrCode getTrafficInfoByEgo(const int ego_id, const double radius,
                                      sim_msg::Traffic& traffic) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    static Base::txInt sFrameCnt = 0;
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
      Base::TimeParamManager CurrentStepTimeMgr = m_SimPtr->GetTimeMgr();
      if (FLAGS_IgnoreEgoRadius) {
        CloudTrafficSystemPtr()->FillingTrafficData(CurrentStepTimeMgr, traffic);
        LOG_IF(INFO, (((sFrameCnt++) > FLAGS_tc_show_statictis_info_interval) ? (sFrameCnt = 0, true) : (false)))
            << "getTrafficInfoByEgo " << TX_VARS(traffic.cars_size()) << TX_VARS(traffic.trafficlights_size());
        return TrafficManager::trafficSucc;
      } else {
        std::vector<Base::ITrafficElementPtr> queryResultElementVec;
        if (CallSucc(CloudTrafficSystemPtr()->QueryTrafficDataByCircle(ego_id, radius, queryResultElementVec))) {
          CloudTrafficSystemPtr()->SendSpecialTrafficData(CurrentStepTimeMgr, queryResultElementVec, traffic);
          std::stringstream ss;
          int otherEgoCnt = 0;
          for (const auto& elemPtr : queryResultElementVec) {
            if (_plus_(Base::ITrafficElement::ElementType::TAD_Ego) == elemPtr->Type()) {
              ss << TX_VARS_NAME(otherEgoId, elemPtr->Id());
              otherEgoCnt++;
            }
          }
          LOG_IF(INFO, FLAGS_LogLevel_Cloud)
              << "getTrafficInfoByEgo " << TX_VARS(ego_id) << TX_VARS(radius) << " success."
              << TX_VARS_NAME(otherEgoIdList, ss.str()) << TX_VARS(otherEgoCnt)
              << TX_VARS_NAME(queryCnt, queryResultElementVec.size());
          return TrafficManager::trafficSucc;
        } else {
          LOG_IF(INFO, FLAGS_LogLevel_Cloud)
              << "getTrafficInfoByEgo " << TX_VARS(ego_id) << TX_VARS(radius) << " failure.";
          return TrafficManager::trafficErr;
        }
      }
    } else {
      LOG(WARNING) << "getTrafficInfoByEgo " << TX_VARS(ego_id) << TX_VARS(radius) << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

  virtual ErrCode mergeSurroundingMapInfo(const sim_msg::MergeTrafficInfo& merge_traffic_info) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);

    return TrafficManager::trafficSucc;
  }

  virtual ErrCode step(const int64_t frame_id, const int log_level, const int64_t cur_sim_begin_time,
                       const int cur_sim_step_count) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    static Base::TimingCPU s_timer;
    static Base::txInt sFrameCnt = 0;

    /*LOG_IF(INFO, true || FLAGS_LogLevel_Statistics_MultiThread || FLAGS_LogLevel_Statistics) << "TrafficLoop::Step
       start "
        << TX_VARS_NAME(timer, s_timer.StartCounter());*/
    s_timer.StartCounter();
    using TimeParamManager = Base::TimeParamManager;

    TrafficManager::ErrCode retV = TrafficManager::trafficSucc;
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
      const Base::txFloat relativeTimeInSec = inputeParams.get_time_step_in_s();
      const auto CurrentStepTimeMgr = m_SimPtr->MakeTimeMgrRelativeTimeInSecond(relativeTimeInSec);

      LOG_IF(INFO, FLAGS_LogLevel_SendMsg) << "[simulation_time] : " << CurrentStepTimeMgr.str();

      Simulation_traffic_ego(CurrentStepTimeMgr);
      retV = TrafficManager::trafficSucc;
    } else {
      LOG(WARNING) << " m_TrafficElementSystemPtr is not initialized, Send 0 elements.";
      retV = TrafficManager::trafficErr;
    }
    LOG_IF(INFO, (((sFrameCnt++) > FLAGS_tc_show_statictis_info_interval) ? (sFrameCnt = 0, true) : (false)))
        << "TrafficLoop::Step end. " << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds() / 1000.0);
    return retV;
  }

  virtual ErrCode updateEgoInfo(const int ego_id, const sim_msg::Location& info) noexcept override {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
      if (CallSucc(CloudTrafficSystemPtr()->UpdateEgoOnCloud(ego_id, info))) {
        LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "[20201209_debug] updateEgoInfo " << TX_VARS(ego_id) << " success.";
        return TrafficManager::trafficSucc;
      } else {
        LOG(WARNING) << "[20201209_debug] updateEgoInfo " << TX_VARS(ego_id) << " failure.";
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(WARNING) << "[20201209_debug] updateEgoInfo " << TX_VARS(ego_id) << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

 protected:
  void Simulation_traffic_ego(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
    m_SimPtr->SimulationTraffic(timeMgr);
    /*
    CloudTrafficSystemPtr()->FillingSpatialQuery();
    const Base::txBool res = m_TrafficElementSystemPtr->Update(timeMgr);
    m_lastStepTime = timeMgr.AbsTime();*/
    if (isCloudDitw()) {
      CloudTrafficSystemPtr()->PostOpDITWData(timeMgr);
      CloudTrafficSystemPtr()->CheckL2W(timeMgr);
    }
  }

#if __TX_Mark__("__pingsn__")
  virtual ErrCode updateVehicles(const int64_t frame_id, its::txVehicles& info) noexcept TX_OVERRIDE {
    if (NonNull_Pointer(ParallelSimulationSystemPtr())) {
      m_SimPtr->TrafficRecord_RecordInputVehicles(frame_id, info);
      ParallelSimulationSystemPtr()->DynamicGenerateScene(info, ParallelSimulationSystemPtr()->SceneLoader());
      return TrafficManager::trafficSucc;
    } else {
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }

  Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE ColorTypeConver(its::txLightColor _psColor) TX_NOEXCEPT {
    switch (_psColor) {
      case its::txLightColor::Light_Green:
        return _plus_(Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE::eGreen);
      case its::txLightColor::Light_Yellow:
        return _plus_(Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE::eYellow);
      case its::txLightColor::Light_Red:
        return _plus_(Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE::eRed);
      default:
        return _plus_(Base::ISignalPhasePeriod::SIGN_LIGHT_COLOR_TYPE::eGreen);
    }
  }

  virtual ErrCode updateTrafficLight(const int64_t frame_id,
                                     const its::txControllers& lights_info) noexcept TX_OVERRIDE {
    LOG(INFO) << "call updateTrafficLight " << TX_VARS(frame_id);
    std::vector<Base::ITrafficElementPtr>& refTrafficLightVec =
        ParallelSimulationSystemPtr()->ElemMgr()->GetAllSignal();
    for (const auto& reftxController : lights_info.light_controller()) {
      // LOG(INFO) << TXST_TRACE_VARIABLES_NAME(controller_id, reftxController.controller_id());
      for (const auto& reftxLightGroup : reftxController.light_group()) {
        // LOG(INFO) << TXST_TRACE_VARIABLES_NAME(group_id, reftxLightGroup.group_id())
        //     << TXST_TRACE_VARIABLES_NAME(road_id, reftxLightGroup.road_id());
        for (const auto& ref_light : reftxLightGroup.light()) {
          // LOG(INFO) << TXST_TRACE_VARIABLES_NAME(light_id, ref_light.light_id())
          //     << TXST_TRACE_VARIABLES_NAME(light_color, ref_light.color());
          for (const auto& ref_light_element_ptr : refTrafficLightVec) {
            if (NonNull_Pointer(ref_light_element_ptr)) {
              TrafficFlow::PS_SignalLightElementPtr ps_light_element_ptr =
                  std::dynamic_pointer_cast<TrafficFlow::PS_SignalLightElement>(ref_light_element_ptr);
              if (NonNull_Pointer(ps_light_element_ptr) && ref_light.light_id() == (ps_light_element_ptr->Id())) {
                ps_light_element_ptr->Update_ParallelSimulation(ColorTypeConver(ref_light.color()));
              }
            }
          }
        }
      }
    }
    return TrafficManager::trafficSucc;
  }

  virtual ErrCode snapshot(const int64_t frame_id, its::txSimStatus& sceneSnapshot) const TX_NOEXCEPT TX_OVERRIDE {
    if (NonNull_Pointer(m_SimPtr)) {
      m_SimPtr->TrafficRecord_GetCurTrafficState(frame_id, sceneSnapshot);
      return TrafficManager::trafficSucc;
    } else {
      LOG(WARNING) << "sim system is nullptr.";
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }

  virtual ErrCode getMapTrafficInfo(const int64_t frame_id, sim_msg::Traffic& traffic) TX_NOEXCEPT TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);

    if (NonNull_Pointer(m_SimPtr) && CallSucc(m_SimPtr->GetMapTrafficInfo(frame_id, traffic))) {
      return TrafficCloud::TrafficManager::trafficSucc;
    } else {
      LOG(WARNING) << "getMapTrafficInfo "
                   << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

  Base::txString ProtobufDebugJson(const its::txEvents& _tx_event) TX_NOEXCEPT {
    Base::txString retStr;
    if (google::protobuf::util::MessageToJsonString(_tx_event, &retStr).ok()) {
      return retStr;
    } else {
      return (_tx_event.ShortDebugString());
    }
  }

  virtual ErrCode setTrafficEvent(const int64_t frame_id, const its::txEvents& evts) TX_NOEXCEPT TX_OVERRIDE {
    its::txEvents events_hadmap_id = evts;
    // transLaneIDToHadmapID(events_hadmap_id);
    // LOG(INFO) << events_hadmap_id.DebugString();

    if (CallSucc(isParallelSimulation())) {
      LOG_IF(INFO, FLAGS_LogLevel_Cloud) << TX_VARS(frame_id)
                                         << TX_VARS_NAME(txEvent, ProtobufDebugJson(events_hadmap_id))
                                         << TX_VARS(events_hadmap_id.event_size());
      std::lock_guard<std::mutex> lck(mInterfaceMtx);
      if (NonNull_Pointer(ParallelSimulationSystemPtr()) && CallSucc(ParallelSimulationSystemPtr()->IsAlive())) {
        ParallelSimulationSystemPtr()->RegisterSimulationEvent(events_hadmap_id);
        return TrafficManager::trafficSucc;
      } else {
        LOG(WARNING) << "setTrafficEvent failure. System Not Alive.";
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(WARNING) << "simulation type is not parallel simulation.";
      return TrafficManager::trafficErr;
    }
  }

  virtual ErrCode getOdInfo(const std::string& input, std::string& output,
                            const double dbFreeflow = 80.0) TX_NOEXCEPT TX_OVERRIDE {
    if (CallSucc(isParallelSimulation())) {
      std::lock_guard<std::mutex> lck(mInterfaceMtx);
      if (NonNull_Pointer(ParallelSimulationSystemPtr()) && CallSucc(ParallelSimulationSystemPtr()->IsAlive())) {
        LOG_IF(INFO, FLAGS_LogLevel_Cloud) << TX_VARS_NAME(getOdInfo_input_param, input);
        TrafficFlow::od_info odInfo;
        if (CallSucc(odInfo.Import(input))) {
          std::map<Base::txRoadID, std::map<Base::txInt /*veh id*/, Base::txFloat /*velocity*/> >
              road2VehicleVelocityMap;
          ParallelSimulationSystemPtr()->ComputeRoadVehicleInfo(road2VehicleVelocityMap);
          const std::map<Base::txRoadID, std::map<Base::txInt /*veh id*/, Base::txFloat /*velocity*/> >&
              constRoad2VehicleVelocityMap = road2VehicleVelocityMap;
          TrafficFlow::od_route_vector_out& refOutOdRouteVec = odInfo.GetOdRouteVector();
          for (auto& refRouteList : refOutOdRouteVec.route) {
            refRouteList.nVehicleCountFromMicroSim = 0;
            refRouteList.dbAverageVelocityFromMicroSim = 0.0;
            refRouteList.dbCongestionIndexFromMicroSim = 0.0;
            for (const auto strRoadId : refRouteList.road_id) {
              Base::txRoadID nRoadId = boost::lexical_cast<Base::txRoadID>(strRoadId);
              if (_Contain_(constRoad2VehicleVelocityMap, nRoadId)) {
                const auto& refMapId2Velocity = constRoad2VehicleVelocityMap.at(nRoadId);
                for (const auto pareId2Velocity : refMapId2Velocity) {
                  refRouteList.dbAverageVelocityFromMicroSim += pareId2Velocity.second;
                  refRouteList.nVehicleCountFromMicroSim += 1;
                  LOG_IF(INFO, FLAGS_LogLevel_OdInfo) << "Compute vehicle " << pareId2Velocity.first;
                }
              } else {
                LOG_IF(INFO, FLAGS_LogLevel_OdInfo) << "do not contain roadId " << nRoadId;
              }
            }

            if (refRouteList.nVehicleCountFromMicroSim > 0) {
              refRouteList.dbAverageVelocityFromMicroSim =
                  refRouteList.dbAverageVelocityFromMicroSim / refRouteList.nVehicleCountFromMicroSim;
              refRouteList.dbCongestionIndexFromMicroSim = refRouteList.dbAverageVelocityFromMicroSim / dbFreeflow;
            }
          }
          odInfo.Export(output);
          return TrafficManager::trafficSucc;
        } else {
          output.clear();
          return TrafficManager::trafficErr;
        }
      } else {
        LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "getOdInfo failure. System Not Alive.";
        return TrafficManager::trafficErr;
      }
    } else {
      LOG(WARNING) << "simulation type is not parallel simulation.";
      return TrafficManager::trafficErr;
    }
  }

  virtual void reset() TX_NOEXCEPT TX_OVERRIDE {
    if (NonNull_Pointer(m_SimPtr)) {
      m_SimPtr->ResetHdMapCache(false);
      sim_msg::Location unuse;
      m_SimPtr->CreateEnv(false, unuse);
      m_SimPtr->TrafficRecord_PartialClear();
    } else {
      LOG(WARNING) << "system is nullptr.";
    }
  }

  sim_msg::cloud::MapRange MapRangeConvert(const Base::map_range_t& range) TX_NOEXCEPT {
    sim_msg::cloud::MapRange resRange;
    resRange.mutable_bottom_left()->set_x(range.bottom_left.x);
    resRange.mutable_bottom_left()->set_y(range.bottom_left.y);
    resRange.mutable_top_right()->set_x(range.top_right.x);
    resRange.mutable_top_right()->set_y(range.top_right.y);
    return std::move(resRange);
  }

  // egine micro sim entrance
  virtual bool init(const std::string& scenario_file_path, const std::string& hadmap_path,
                    const its::txControllers& traffic_light_controllers,
                    const its::txSimStatus& simStatus) TX_NOEXCEPT TX_OVERRIDE {
    Scene::InitConfigure_t initConf;
    if (CallSucc(exists(Utils::FilePath(scenario_file_path)))) {
      std::ifstream in_json(scenario_file_path);
      nlohmann::json re_parse_json = nlohmann::json::parse(in_json);
      initConf = re_parse_json;
      LOG(INFO) << TX_VARS_NAME(InParams, initConf);
    } else {
      LOG(WARNING) << " Init Conf File do not exist. " << TX_VARS(scenario_file_path);
    }

    initConf.map_data.parallel_simulation_info.map_file = hadmap_path;
    initConf.map_data.parallel_simulation_info.config_file = initConf.map_data.config_path;
    initConf.map_data.parallel_simulation_info.traffic_light_string = traffic_light_controllers.SerializeAsString();
    initConf.map_data.parallel_simulation_info.init_state_string = simStatus.SerializeAsString();

    inputeParams.set_InitConfigure(initConf);

    m_SimPtr = std::make_shared<TrafficFlow::PluginTemplate>();
    if (NonNull_Pointer(m_SimPtr)) {
      m_SimPtr->InputeParams() = (inputeParams);
      m_SimPtr->ResetHdMapCache(true);
      sim_msg::Location unuse;
      m_SimPtr->CreateEnv(false, unuse);

      std::vector<sim_msg::cloud::MapRange> multi_regions;
      for (const auto& refRegion : m_SimPtr->InputeParams().regions_range()) {
        multi_regions.emplace_back(MapRangeConvert(refRegion));
      }
      reset_simulation_range(multi_regions);
    } else {
      LOG(ERROR) << "create system template failure.";
      return false;
    }
    return true;
  }

  virtual bool addVehicle(const its::txVehicle& vehicleMsg) TX_NOEXCEPT TX_OVERRIDE {
    // LOG(INFO) << " addVehicle:" << vehicleMsg.DebugString();

    if (vehicleMsg.route().road_id_size() == 0) {
      LOG(WARNING) << "route is empty";
      return false;
    }

    hadmap::txRoadPtr curRoadPtr =
        HdMap::HadmapCacheConCurrent::GetTxRoadPtr(vehicleMsg.route().road_id(vehicleMsg.road_idx()));

#  if 0
    Base::txLaneID vehicleStartLaneId = -1;
    // double offset_to_start = std::min(3.0, curRoadPtr->getLength());
    // hadmap::txPoint roadStartPoint = curRoadPtr->getSections()[0]
    // // ->get(vehicleStartLaneId)->getGeometry()->getPoint(offset_to_start);
    hadmap::txPoint roadStartPoint = curRoadPtr->getGeometry()->getPoint(vehicleMsg.road_pos());
    Base::txFloat startV = 12;
    m_SimPtr->TrafficRecord_RecordInputVehicle(vehicleMsg);

    return m_SimPtr->addVehicleToElementManager(
        vehicleMsg.vehicle_id(), vehicleMsg.vehicle_type(), vehicleStartLaneId,
        {vehicleMsg.route().road_id().begin(), vehicleMsg.route().road_id().end()}, startV, roadStartPoint.x,
        roadStartPoint.y, roadStartPoint.z);

#  else  // new od assign veh init lane
    if (Null_Pointer(curRoadPtr)) {
      LOG(WARNING) << "curRoadPtr is nullptr";
      return false;
    }

    hadmap::txSectionPtr curSectionPtr = curRoadPtr->getSections()[0];
    if (Null_Pointer(curSectionPtr)) {
      LOG(WARNING) << "curSectionPtr is nullptr";
      return false;
    }

    hadmap::txLanePtr curLanePtr = nullptr;
    hadmap::txLanes resLanes;

    for (auto lanePtr : curSectionPtr->getLanes()) {
      if (Utils::IsDrivingLane(lanePtr->getLaneType())) {
        resLanes.push_back(lanePtr);
      }
    }
    const Base::txInt nLaneSize = resLanes.size();
    if (0 == nLaneSize) {
      curLanePtr = curSectionPtr->get(-1);
    } else {
      int random_index = std::rand() % nLaneSize;
      curLanePtr = resLanes[random_index];
    }

    if (NonNull_Pointer(curLanePtr)) {
      // hadmap::txPoint roadStartPoint = curLanePtr->getGeometry()->getStart();
      hadmap::txPoint roadStartPoint = curRoadPtr->getGeometry()->getPoint(vehicleMsg.road_pos());
      Base::txLaneID vehicleStartLaneId = curLanePtr->getId();

      Base::txFloat startV = 12;
      m_SimPtr->TrafficRecord_RecordInputVehicle(vehicleMsg);

      return m_SimPtr->addVehicleToElementManager(
          vehicleMsg.vehicle_id(), vehicleMsg.vehicle_type(), vehicleStartLaneId,
          {vehicleMsg.route().road_id().begin(), vehicleMsg.route().road_id().end()}, startV, roadStartPoint.x,
          roadStartPoint.y, roadStartPoint.z);
    } else {
      LOG(WARNING) << "veh init lane is null.";
      return false;
    }
#  endif
  }

  virtual bool removeVehicle(const its::txVehicle& vehicleMsg) TX_NOEXCEPT TX_OVERRIDE {
    LOG(INFO) << " removeVehicle:" << vehicleMsg.DebugString();

    return ParallelSimulationSystemPtr()->ElemMgr()->EraseVehicleById(vehicleMsg.vehicle_id());
  }

  virtual ErrCode step(const uint64_t absTime) TX_NOEXCEPT TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    static Base::TimingCPU s_timer;
    static Base::txInt sFrameCnt = 0;

    /*LOG_IF(INFO, true || FLAGS_LogLevel_Statistics_MultiThread || FLAGS_LogLevel_Statistics) << "TrafficLoop::Step
       start "
        << TXST_TRACE_VARIABLES_NAME(timer, s_timer.StartCounter());*/
    s_timer.StartCounter();
    using TimeParamManager = Base::TimeParamManager;

    TrafficManager::ErrCode retV = TrafficManager::trafficSucc;
    if (NonNull_Pointer(CloudTrafficSystem_RawPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      const TimeParamManager timeMgr = m_SimPtr->MakeTimeMgrAbsTimeInMillisecondSecond(absTime);
      LOG_IF(INFO, FLAGS_LogLevel_SendMsg) << "[simulation_time] : " << timeMgr.str();
      Simulation_traffic_ego(timeMgr);
      retV = TrafficManager::trafficSucc;
    } else {
      LOG(WARNING) << ", m_TrafficElementSystemPtr is not initialized, Send 0 elements.";
      retV = TrafficManager::trafficErr;
    }
    LOG_IF(INFO, (((sFrameCnt++) > FLAGS_tc_show_statictis_info_interval) ? (sFrameCnt = 0, true) : (false)))
        << "TrafficLoop::Step end. " << TX_VARS_NAME(timer, s_timer.GetElapsedMicroseconds() / 1000.0);
    return retV;
  }

  virtual its::txSimStat getSimStatMsg() const TX_NOEXCEPT TX_OVERRIDE { return m_SimPtr->GetSimStatMsg(); }
#endif /*__TX_Mark__("__pingsn__")*/

#if __TX_Mark__("virtual city expand capacity")

 public:
  Base::txBool InRange(const hadmap::txPoint gps) const TX_NOEXCEPT {
    return TrafficFlow::TAD_Cloud_AI_VehicleElement::InRanges(__Lon__(gps), __Lat__(gps));
  }

  hadmap::txPoint pb2gps(const its::txVehicle& veh) const TX_NOEXCEPT {
    return std::move(hadmap::txPoint(veh.lng(), veh.lat(), 0.0));
  }

  hadmap::txPoint pb2gps(const sim_msg::Car& car) const TX_NOEXCEPT {
    return std::move(hadmap::txPoint(car.x(), car.y(), car.z()));
  }

  virtual bool reset_simulation_range(const std::vector<sim_msg::cloud::MapRange>& vec_valid_sim_range) noexcept
      TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    LOG(INFO) << "call reset_simulation_range " << TX_VARS(vec_valid_sim_range.size());
    std::vector<Base::map_range_t> vec_map_range;
    for (const auto& refRange : vec_valid_sim_range) {
      LOG(INFO) << "range : " << TX_VARS_NAME(bottom_left, refRange.bottom_left().DebugString())
                << TX_VARS_NAME(top_right, refRange.top_right().DebugString());
      Base::map_range_t newRange;
      newRange.bottom_left = hadmap::txPoint(refRange.bottom_left().x(), refRange.bottom_left().y(), 0.0);
      newRange.top_right = hadmap::txPoint(refRange.top_right().x(), refRange.top_right().y(), 0.0);
      vec_map_range.emplace_back(newRange);
    }

    TrafficFlow::TAD_Cloud_AI_VehicleElement::SetMapRanges(vec_map_range);

    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystemPtr()->IsAlive())) {
      return CloudTrafficSystemPtr()->ReSetInputAgent(vec_map_range);
    } else {
      LOG(WARNING) << "m_TrafficElementSystemPtr is nullptr.";
    }
    return false;
  }

  virtual int32_t reset_ego_info(const uint64_t abs_time_ms,
                                 const std::map<int64_t, sim_msg::Location>& _ego_info_map) noexcept TX_OVERRIDE {
    /*std::lock_guard<std::mutex> lck(mInterfaceMtx);*/
    MultiRegionInfo << "call reset_ego_info " << TX_VARS(abs_time_ms) << TX_VARS(_ego_info_map.size());
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      /*updateEgoInfo*/
      /*addEgo*/
      /*deleteEgo*/
      std::map<int64_t, sim_msg::Location> cur_ego_info_map = _ego_info_map;
      std::set<int64_t> set_need_delete_ego_id;
      auto refEgoArray = CloudTrafficSystem_RawPtr()->ElemMgr()->GetEgoArray();
      for (auto refEgoPtr : refEgoArray) {
        if (NonNull_Pointer(refEgoPtr)) {
          const Base::txSysId curEgoId = refEgoPtr->Id();
          if (_Contain_(cur_ego_info_map, curEgoId)) {
            updateEgoInfo(curEgoId, cur_ego_info_map.at(curEgoId));
            cur_ego_info_map.erase(curEgoId);
          } else {
            set_need_delete_ego_id.insert(curEgoId);
          }
        }
      }
      for (const auto curEgoId : set_need_delete_ego_id) {
        deleteEgo(curEgoId, 0);
      }

      for (const auto& refNewEgo : cur_ego_info_map) {
        addEgoForce(refNewEgo.first, refNewEgo.second);
      }
      return (CloudTrafficSystemPtr()->ElemMgr()->GetEgoCount());
    } else {
      LOG(WARNING) << "m_TrafficElementSystemPtr is nullptr or un-alive.";
      return 0;
    }
  }

  Base::txBool hasSerialArchive(const its::txVehicle& refVehicle) const TX_NOEXCEPT {
    return refVehicle.serialization_archive().size() > 0;
  }
  Base::txBool hasSerialArchive(const sim_msg::Car& refCar) const TX_NOEXCEPT {
    return refCar.serialization_archive().size() > 0;
  }

  void KillOutofRange() TX_NOEXCEPT {
    auto refVehicleArray_kernel = CloudTrafficSystem_RawPtr()->CloudElemMgr()->GetAllVehiclePtr();
    tbb::parallel_for(static_cast<std::size_t>(0), refVehicleArray_kernel.size(), [&](const std::size_t idx) {
      auto& elemPtr = refVehicleArray_kernel[idx];
      TrafficFlow::TAD_Cloud_AI_VehicleElementPtr cloudElemPtr =
          std::dynamic_pointer_cast<TrafficFlow::TAD_Cloud_AI_VehicleElement>(elemPtr);
      if (NonNull_Pointer(elemPtr) && CallSucc(elemPtr->IsAlive()) && NonNull_Pointer(cloudElemPtr)) {
        const Base::txSysId vehId = elemPtr->Id();
        if (CallFail(cloudElemPtr->InRangeAfterSimulation())) {
          cloudElemPtr->Kill();
          MultiRegionInfo << "[cloud_debug][1a][kernel2edge]" << TX_VARS(elemPtr->Id())
                          << TX_VARS(cloudElemPtr->GetLocation().StrWGS84());
        }
      }
    }); /*lamda function*/
        /* parallel_for */
  }

  template <class T>
  int32_t reset_traffic_info_func(const uint64_t abs_time_ms,
                                  tbb::concurrent_hash_map<Base::txSysId, T, Utils::txSysIdHashCompare>& id2pb,
                                  const double maxErrDist) TX_NOEXCEPT {
    using type_id2vehiclePb = tbb::concurrent_hash_map<Base::txSysId, T, Utils::txSysIdHashCompare>;
    const Base::TimeParamManager cur_time_mgr = m_SimPtr->MakeTimeMgrAbsTimeInMillisecondSecond(abs_time_ms);

    CloudTrafficSystem_RawPtr()->CloudElemMgr()->RemoveAllEdgeVehicles();

    KillOutofRange();

    tbb::parallel_for(id2pb.range(), [&](const typename type_id2vehiclePb::range_type& r) {
      for (typename type_id2vehiclePb::iterator itr = r.begin(); itr != r.end(); itr++) {
        const Base::txSysId injectVehId = (*itr).first;
        const T& injectVehOrCar = (*itr).second;
        if (CallSucc(InRange(pb2gps(injectVehOrCar)))) {
          /*in map range.*/
          if (CallSucc(hasSerialArchive(injectVehOrCar))) {
            /*has  serialize archive*/
            TrafficFlow::TAD_Cloud_AI_VehicleElementPtr vehicle =
                std::make_shared<TrafficFlow::TAD_Cloud_AI_VehicleElement>();
            if (NonNull_Pointer(vehicle)) {
              CloudTrafficSystem_RawPtr()->CloudElemMgr()->AddVehiclePtr(vehicle);
              vehicle->UpdateByTxVehicle(injectVehOrCar);
              vehicle->OnStart(cur_time_mgr);
              MultiRegionInfo << "[cloud_debug][2b][edge2kernel] Create Vehicle Element Success."
                              << TX_VARS_NAME(vehicle_id_cloud, (*itr).first);
            } else {
              LOG(WARNING) << "[cloud_debug][2b][edge2kernel] Create Vehicle Element Failure."
                           << TX_VARS_NAME(vehicle_id_cloud, (*itr).first);
            }
          } else {
            /*has simple archive*/
            MultiRegionInfo << "[cloud_debug][2a][kernel2kernel] " << TX_VARS_NAME(vehicle_id_cloud, (*itr).first);
          }
        } else {
          /*out of map range.*/
          TrafficFlow::TAD_Cloud_AI_VehicleElementPtr vehicle =
              std::make_shared<TrafficFlow::TAD_Cloud_AI_VehicleElement>();
          if (NonNull_Pointer(vehicle) &&
              vehicle->Initialize_Cloud_MultiRegion(injectVehOrCar, CloudTrafficSystem_RawPtr()->SceneLoader())) {
            vehicle->CheckStart(cur_time_mgr);
            TX_MARK("register,but will be clear by Simulation_traffic_ego.UngisterAllElements");
            // vehicle->OnStart(cur_time_mgr);
            MultiRegionInfo << "[cloud_debug][2c][edge2dege_or_kernel2edge] create vehicle element success. "
                            << TX_VARS(vehicle->Id()) << TX_VARS(vehicle->GetLocation().StrWGS84())
                            << TX_COND(vehicle->IsAlive());
            CloudTrafficSystem_RawPtr()->CloudElemMgr()->AddEdgeVehiclePtr(vehicle);
          } else {
            LOG(WARNING) << "[cloud_debug][2c][edge2edge_or_kernel2edge] Create Vehicle Element Failure."
                         << TX_VARS_NAME(vehicle_id_cloud, (*itr).first);
          }
        }
      }
    });

    CloudTrafficSystem_RawPtr()->ElemMgr()->ResortKillElement();
    CloudTrafficSystem_RawPtr()->ElemMgr()->GenerateAllTrafficElement();
    return CloudTrafficSystem_RawPtr()->ElemMgr()->GetVehicleCount();
  }

  virtual int32_t reset_traffic_info(const uint64_t abs_time_ms, const its::txSimStatus& cur_sim_status,
                                     const double maxErrDist = 2.0) noexcept TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    MultiRegionInfo << "call reset_traffic_info " << TX_VARS(abs_time_ms) << TX_VARS(cur_sim_status.vehicle_size());
    if (NonNull_Pointer(CloudTrafficSystemPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      /*its::txVehicles cur_traffic_info; cur_traffic_info.CopyFrom(cur_traffic_info_vec);*/
      using type_id2vehiclePb = tbb::concurrent_hash_map<Base::txSysId, its::txVehicle, Utils::txSysIdHashCompare>;

      type_id2vehiclePb con_hash_id2vehiclePB;

      std::ostringstream oss_archive, oss_simple;
      oss_archive << std::setprecision(FloatPrecision) << std::fixed << abs_time_ms << ": ";
      oss_simple << std::setprecision(FloatPrecision) << std::fixed << abs_time_ms << ": ";
      for (const its::txVehicle& refVehiclePb : cur_sim_status.vehicle()) {
        type_id2vehiclePb::accessor a;
        con_hash_id2vehiclePB.insert(a, refVehiclePb.vehicle_id());
        (a->second).CopyFrom(refVehiclePb);
        a.release();
        if (CallSucc(hasSerialArchive(refVehiclePb))) {
          oss_archive << TX_VARS(refVehiclePb.vehicle_id()) << ",";
        } else {
          oss_simple << TX_VARS(refVehiclePb.vehicle_id()) << ",";
        }
      }
      MultiRegionInfo << "[cloud_debug][has_archive] " << oss_archive.str();
      MultiRegionInfo << "[cloud_debug][has_simple] " << oss_simple.str();

      return reset_traffic_info_func(abs_time_ms, con_hash_id2vehiclePB, maxErrDist);
    } else {
      LOG(WARNING) << "m_TrafficElementSystemPtr is nullptr or un-alive.";
      return 0;
    }
  }

  virtual ErrCode snapshot_micro(const int64_t frame_id, its::txSimStatus& microSceneSnapshot) noexcept TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    static Base::txInt sFrameCnt = 0;
    if (NonNull_Pointer(CloudTrafficSystem_RawPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      CloudTrafficSystem_RawPtr()->FillingSimStatus(CurrentStepTimeMgr(), microSceneSnapshot);
      return TrafficManager::trafficSucc;
    } else {
      LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "getMapTrafficInfo "
                                         << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

  virtual int32_t reset_traffic_info(const uint64_t abs_time_ms, const sim_msg::Traffic& cur_traffic,
                                     const double maxErrDist = 2.0) noexcept TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    MultiRegionInfo << "call reset_traffic_info " << TX_VARS(abs_time_ms) << TX_VARS(cur_traffic.cars_size());
    if (NonNull_Pointer(CloudTrafficSystem_RawPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      using type_id2carPb = tbb::concurrent_hash_map<Base::txSysId, sim_msg::Car, Utils::txSysIdHashCompare>;
      type_id2carPb con_hash_id2carPB;
      std::ostringstream oss_archive, oss_simple;
      oss_archive << std::setprecision(FloatPrecision) << std::fixed << abs_time_ms << ": ";
      oss_simple << std::setprecision(FloatPrecision) << std::fixed << abs_time_ms << ": ";
      for (const sim_msg::Car& refCarPb : cur_traffic.cars()) {
        type_id2carPb::accessor a;
        con_hash_id2carPB.insert(a, refCarPb.id());
        (a->second).CopyFrom(refCarPb);
        a.release();
        if (CallSucc(hasSerialArchive(refCarPb))) {
          oss_archive << TX_VARS(refCarPb.id()) << ",";
        } else {
          oss_simple << TX_VARS(refCarPb.id()) << ",";
        }
      }
      MultiRegionInfo << "[cloud_debug][has_archive] " << oss_archive.str();
      MultiRegionInfo << "[cloud_debug][has_simple] " << oss_simple.str();

      return reset_traffic_info_func(abs_time_ms, con_hash_id2carPB, maxErrDist);
    } else {
      LOG(WARNING) << "m_TrafficElementSystemPtr is nullptr or un-alive.";
      return 0;
    }
  }

  virtual ErrCode snapshot_micro(const int64_t frame_id, sim_msg::Traffic& microSceneSnapshot) noexcept TX_OVERRIDE {
    std::lock_guard<std::mutex> lck(mInterfaceMtx);
    static Base::txInt sFrameCnt = 0;
    if (NonNull_Pointer(CloudTrafficSystem_RawPtr()) && CallSucc(CloudTrafficSystem_RawPtr()->IsAlive())) {
      CloudTrafficSystem_RawPtr()->FillingTrafficData(CurrentStepTimeMgr(), microSceneSnapshot);
      return TrafficManager::trafficSucc;
    } else {
      LOG_IF(INFO, FLAGS_LogLevel_Cloud) << "getMapTrafficInfo "
                                         << " failure. System Not Alive.";
      return TrafficManager::trafficErr;
    }
  }

#  if __cloud_ditw__
  virtual ErrCode inject_perception_raw_data(const PerceptionMessageData& _input_frame) noexcept TX_OVERRIDE {
    if (CloudDitwSimType::eForecastSim != m_cur_cloud_ditw_type) {
      m_cur_cloud_ditw_type = CloudDitwSimType::eDitw;
      CloudDitwInfo << TX_VARS(_input_frame.datalist_size()) << TX_VARS(_input_frame.heads_size());
      CloudTrafficSystem_RawPtr()->UpdateDITWData(CurrentStepTimeMgr(), _input_frame);
      return TrafficManager::trafficSucc;
    } else {
      LOG(WARNING) << "[ERROR] Sim Type Mismatch, call " << __func__ << " failure.";
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }

  virtual ErrCode initialize_l2w_sim(const its::txStatusRecord& status_pb_record /* size = 1 */,
                                     const std::string _event_json) noexcept TX_OVERRIDE {
    m_cur_cloud_ditw_type = CloudDitwSimType::eForecastSim;
    CloudDitwInfo << TX_VARS(status_pb_record.start_timestamp()) << TX_VARS(status_pb_record.end_timestamp())
                  << TX_VARS(status_pb_record.status_size());

    if (CallSucc(m_SimPtr->initialize_l2w_sim(sim_msg::TrafficRecords(), status_pb_record, _event_json))) {
      return TrafficCloud::TrafficManager::trafficSucc;
    } else {
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }

  virtual ErrCode initialize_l2w_sim(const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                     const std::string _event_json) noexcept TX_OVERRIDE {
    m_cur_cloud_ditw_type = CloudDitwSimType::eForecastSim;
    CloudDitwInfo << TX_VARS(traffic_pb_record.traffic_record_size());
    if (CallSucc(m_SimPtr->initialize_l2w_sim(traffic_pb_record, its::txStatusRecord(), _event_json))) {
      return TrafficCloud::TrafficManager::trafficSucc;
    } else {
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }

  virtual ErrCode set_vehicle_cfg(const std::string _cfg_json_content) noexcept TX_OVERRIDE {
    LOG(INFO) << "call set_vehicle_cfg :" << _cfg_json_content;
    Utils::BehaviorCfgDB::Initialize(_cfg_json_content, Utils::ConfigType::json_content);
    Utils::BehaviorCfgDB::ResetDefaultCfg();
    return TrafficManager::trafficSucc;
  }

#  endif /*__cloud_ditw__*/

  virtual ErrCode inject_traffic_event(const std::string _event_json) noexcept TX_OVERRIDE {
    if (NonNull_Pointer(m_SimPtr) && CallSucc(m_SimPtr->inject_traffic_event(_event_json))) {
      return TrafficCloud::TrafficManager::trafficSucc;
    } else {
      return TrafficCloud::TrafficManager::trafficErr;
    }
  }
#endif /*__TX_Mark__("virtual city expand capacity")*/

 protected:
#if __pingsn__

#endif /*__pingsn__*/

#if __cloud_ditw__

 public:
  enum class CloudDitwSimType : txInt { eUnKnow = 0, eDitw = 1, eForecastSim = 2 };
  CloudDitwSimType m_cur_cloud_ditw_type = CloudDitwSimType::eUnKnow;
  Base::txBool isCloudDitw() const TX_NOEXCEPT { return CloudDitwSimType::eDitw == m_cur_cloud_ditw_type; }
  Base::txBool isForecastSim() const TX_NOEXCEPT { return CloudDitwSimType::eForecastSim == m_cur_cloud_ditw_type; }

#endif /*__cloud_ditw__*/
};

BOOST_DLL_ALIAS(tx_tc_traffic_impl::create, create_plugin)

TX_NAMESPACE_CLOSE(Cloud)

std::shared_ptr<TrafficCloud::TrafficManager> createTrafficManagerPtr(void) {
  return std::make_shared<Cloud::tx_tc_traffic_impl>();
}
