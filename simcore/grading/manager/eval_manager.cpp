// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "manager/eval_manager.h"

#include "boost/interprocess/managed_shared_memory.hpp"
#include "boost/interprocess/sync/named_mutex.hpp"
#include "boost/interprocess/sync/scoped_lock.hpp"
#include "factory/eval_factory_interface.h"
#include "factory/eval_factory_no_check.h"
#include "utils/eval_lib_loader.h"

namespace eval {
EvalManager::EvalManager() {
  _actor_mgr = CActorManager::GetInstance();
  _map_mgr = CMapManager::GetInstance();
  _msg_mgr = CMsgManager::GetInstance();

  VLOG_0 << "EvalManager constructed.\n";
}

EvalManager::~EvalManager() {
  Release();
  VLOG_0 << "EvalManager released.\n";
}

/*=========== eval algorithms ===========*/
void EvalManager::CreateEvalAlgorithms(const EvalCfgMap &eval_cfgs) {
  // release eval algorithms
  _eval_maps.clear();

  // eval algorithms that need result check
  EvalAlgorithmMap eval_algorithms_check;  // included into result check

  // factory to make eval algorithms that do not need result check
  EvalFactoryNoCheck m_factory_nocheck;

  // eval algorithms that from external customized kpi
  EvalAlgorithmMap eval_algorithms_external;  // included into external algorithm

  // get cGetGlobalFactory() function from external library
  eval::EvalFactoryInterface *(*lib_get_global_factory)() = nullptr;
  try {
    if (_external_lib) {
      void *lib_func = LibraryLoader::GetLibraryFunc("cGetGlobalFactory", _external_lib);
      lib_get_global_factory = reinterpret_cast<eval::EvalFactoryInterface *(*)()>(lib_func);
    }
  } catch (const std::exception &e) {
    VLOG_0 << "error in getting cGetGlobalFactory() function from lib : " << e.what() << '\n';
    lib_get_global_factory = nullptr;
  }

  // do make
  const auto &kpiGroup = m_initHelper.getGradingKpiGroup();
  for (const auto &kpi : kpiGroup.kpi()) {
    const std::string &kpi_name = kpi.name();
    auto eval_algorithm = getGlobalFactory()->Make(kpi_name);
    if (eval_algorithm.get()) {
      eval_algorithms_check[kpi_name] = eval_algorithm;
      continue;
    }

    // get eval algorithm from external library
    if (lib_get_global_factory == nullptr) continue;
    eval_algorithm = lib_get_global_factory()->Make(kpi_name);
    if (eval_algorithm.get()) {
      eval_algorithms_external[kpi_name] = eval_algorithm;
      continue;
    }
  }

  // push into maps
  _eval_maps[C_EvalAlgorithmType::No_Result_Check] = m_factory_nocheck.MakeAll();
  _eval_maps[C_EvalAlgorithmType::Need_Result_Check] = eval_algorithms_check;
  _eval_maps[C_EvalAlgorithmType::External_Algorithm] = eval_algorithms_external;
}

void EvalManager::InitEvalAlgorithms(EvalAlgorithmMap &eval_algorithms, tx_sim::ResetHelper &helper,
                                     bool is_external_kpi) {
  // do init
  for (auto pair = eval_algorithms.begin(); pair != eval_algorithms.end(); ++pair) {
    auto eval_algorithm = pair->second;
    try {
      if (eval_algorithm.get() == nullptr) {
        LOG_WARNING << "no eval algorithm for kpi " << pair->first << "\n";
        continue;
      }

      if (!is_external_kpi) {
        eval_algorithm->Init(m_initHelper);
      } else {
        eval_algorithm->Init(m_initHelper, _actor_mgr.get(), _map_mgr.get(), _msg_mgr.get());
      }
    } catch (const std::exception &e) {
      LOG_ERROR << "fail to run eval algorithm, reason " << e.what() << "\n";
    }
  }
}

void EvalManager::UpdateEvalAlgorithms(tx_sim::StepHelper &helper, bool &should_stop, std::string &reason) {
  // clear detected event flag of grading msg in this frame
  {
    auto &gradingMsg = m_stepHelper.GetGradingMsg();
    gradingMsg.mutable_event_detector()->set_hasedge(sim_msg::Grading_EventDetector_EventState_EventNotDetected);
  }

  // advance
  for (auto eval_map = _eval_maps.begin(); eval_map != _eval_maps.end(); ++eval_map) {
    // one map of eval algorithms
    EvalAlgorithmMap &eval_algorithms = eval_map->second;

    // do update
    for (auto pair = eval_algorithms.begin(); pair != eval_algorithms.end(); ++pair) {
      auto eval_algorithm = pair->second;
      try {
        if (eval_algorithm.get() == nullptr) {
          // LOG_ERROR << "no eval algorithm for kpi " << pair->first << "\n";
          continue;
        }

        // update eval algorithm
        if (eval_map->first != External_Algorithm) {
          eval_algorithm->Step(m_stepHelper);
        } else {
          eval_algorithm->Step(m_stepHelper, _actor_mgr.get(), _map_mgr.get(), _msg_mgr.get());
        }

        // set grading message for simcity and legacy
        eval_algorithm->SetGradingMsg(m_stepHelper.GetGradingMsg());

        // check if should stop simulation
        should_stop = should_stop || eval_algorithm->ShouldStopScenario(reason);
      } catch (const std::exception &e) {
        LOG_ERROR << "fail to run eval algorithm: " << pair->first << " , reason: " << e.what() << "\n";
      }
    }
  }
}

void EvalManager::StopEvalAlgorithms(tx_sim::StopHelper &helper, bool &eval_pass, std::string &reason) {
  if (!IsModuleValid()) {
    LOG_ERROR << "module check error.\n";
    eval_pass = false;
    reason = std::string("eval fault, used map/msg/actor/helper module error.");
    return;
  }

  for (auto eval_map = _eval_maps.begin(); eval_map != _eval_maps.end(); ++eval_map) {
    // one map of eval algorithms
    C_EvalAlgorithmType eval_type = eval_map->first;
    EvalAlgorithmMap &eval_algorithms = eval_map->second;

    // collect algorithms eval result that need check
    for (auto pair = eval_algorithms.begin(); pair != eval_algorithms.end(); ++pair) {
      try {
        auto eval_algorithm = pair->second;

        // continue
        if (eval_algorithm.get() == nullptr) {
          continue;
        }

        // all eval algorithms stop
        if (eval_map->first != External_Algorithm) {
          eval_algorithm->Stop(m_stopHelper);
        } else {
          eval_algorithm->Stop(m_stopHelper, _actor_mgr.get(), _map_mgr.get(), _msg_mgr.get());
        }

        // legacy grading report
        eval_algorithm->SetLegacyReport(m_stopHelper.GetGradingStatistics());

        // continue if no result check required
        if (eval_type == C_EvalAlgorithmType::No_Result_Check) {
          continue;
        }

        // check if pass or not
        auto eval_result = eval_algorithm->IsEvalPass();

        bool is_pass = eval_result._state == sim_msg::TestReport_TestState::TestReport_TestState_PASS;
        if (!is_pass) reason = reason.empty() ? eval_result._reason : reason + ";" + eval_result._reason;
        eval_pass = eval_pass && is_pass;

        // collect test report
        {
          sim_msg::TestReport_Case &case_ = eval_algorithm->GetReportCase();

          // set result of test case
          ReportHelper::SetTestResult(*case_.mutable_info()->mutable_result(), eval_result._state, eval_result._reason,
                                      INT32_MIN, INT32_MIN);

          // calculate score of case by pass condition
          ReportHelper::CalScoreOfCase(case_.info().grading_kpi(), case_, true);

          // add case to report
          ReportHelper::AddCase2EgoCase(*m_stopHelper.GetEgoCase(), case_);
        }

        // collect accompay testcase if there are
        {
          for (auto &testcase : eval_algorithm->GetAccompanyCases()) {
            if (testcase.get() == nullptr) {
              continue;
            }

            // add case to report
            sim_msg::TestReport_Case &case_ = testcase->m_reportCase;
            if (!case_.info().has_result()) {
              ReportHelper::SetTestResult(*case_.mutable_info()->mutable_result(),
                                          sim_msg::TestReport_TestState_UN_DEFINED, "", INT32_MIN, INT32_MIN);
            }
            ReportHelper::AddCase2EgoCase(*m_stopHelper.GetEgoCase(), case_);
          }
        }
      } catch (const std::exception &e) {
        LOG_ERROR << "fail to stop eval algorithm, reason " << e.what() << "\n";
        eval_pass = false;
        reason = std::string("eval fault, ") + e.what();
      }
    }
  }
  // set ego meta and add to report
  {
    m_stopHelper.GetEgoMeta()->set_ego_id(getMyEgoGroupName());
    ReportHelper::AddEgoMeta2Report(*m_stopHelper.GetReport(), *m_stopHelper.GetEgoMeta());
  }

  // set ego cases and add to report
  {
    m_stopHelper.GetEgoCase()->set_ego_id(getMyEgoGroupName());
    ReportHelper::AddEgoCase2Report(*m_stopHelper.GetReport(), *m_stopHelper.GetEgoCase());
  }
}

/*=========== eval ticks ===========*/
bool EvalManager::Init(tx_sim::ResetHelper &helper, const EvalParams &params) {
  // init params
  setEvalParams(params);
  m_params = getEvalParams();

  // init t
  _cur_t_ms = 0.0;
  m_pre_t_ms = 0.0;

  // flag if eval has been executed
  m_executed = false;

  if (!IsModuleValid()) {
    return true;
  }

  // scenario info
  ScenarioInfo scenario_info;
  scenario_info.m_cloud_dir = m_params.m_cloud_dir;
  scenario_info.m_report_dir = m_params.m_log_folder;
  scenario_info.m_start_location = helper.ego_start_location();
  scenario_info.m_map_file = helper.map_file_path();

  // get scenario name
  _scenario_name = CFileUtils::BaseName(CFileUtils::AbsPath(helper.scenario_file_path()));

  // get grading.json file and .sim config
  std::string evalCfgFile = GetEvalCfgFromSim(helper, scenario_info);
  _grading_cfg_path = evalCfgFile;
  scenario_info.m_eval_cfg_file = evalCfgFile;

  // connect hadmap
  if (_map_mgr->Connect(helper.map_file_path(), helper.ego_start_location())) {
    VLOG_0 << "connectted to hadmap " << helper.map_file_path() << "\n";
  } else {
    LOG_ERROR << "fail to connect hadmap " << helper.map_file_path() << "\n";
    return false;
  }

  // waypoints
  {
    std::vector<tx_sim::Vector3d> &&waypoints = helper.ego_path();
    scenario_info.m_waypoints.clear();
    for (auto i = 0; i < waypoints.size() && _map_mgr; ++i) {
      const tx_sim::Vector3d &point = waypoints.at(i);
      scenario_info.m_waypoints.emplace_back(point.x, point.y, point.z, 0.0, 0.0, 0.0, Coord_WGS84);
      _map_mgr->WGS84ToENU(*scenario_info.m_waypoints.at(i).MutablePosition());
      scenario_info.m_waypoints.at(i).DebugShow("waypoint:");
    }
  }

  // collect scenario info
  {
    CPosition end_point_enu(helper.ego_destination().x, helper.ego_destination().y, helper.ego_destination().z,
                            Coord_WGS84);
    _map_mgr->WGS84ToENU(end_point_enu);
    scenario_info.m_end_point_enu = end_point_enu;
    scenario_info.m_ego_speed_limit = helper.ego_speed_limit();
  }

  // reset actor manager
  {
    // actor manager parameters
    ActorManagerParam actorManagerParams;

    std::string ego_geometry_payload;
    std::vector<std::pair<int64_t, std::string>> veh_geometry;
    helper.vehicle_measurements(veh_geometry);
    if (veh_geometry.size() > 0) ego_geometry_payload = veh_geometry.at(0).second;

    actorManagerParams.egoGeometry.ParseFromString(ego_geometry_payload);
    actorManagerParams.sceneDescription.ParseFromString(helper.scene_pb());
    // std::cout << actorManagerParams.sceneDescription.Utf8DebugString() << "\n";

    VLOG_0 << "the number of ego in this scene is " << actorManagerParams.sceneDescription.egos_size() << std::endl;
    setTotalEgoNum(actorManagerParams.sceneDescription.egos_size());

    _actor_mgr->Init(actorManagerParams);
  }

  // clear msg manager
  _msg_mgr->Init();

  // init eval helper
  {
    m_initHelper.Init(scenario_info);
    m_stepHelper.Init();
    m_stopHelper.Init();
  }

  // loading user's module dynamic library.
  _external_lib = nullptr;
  try {
    if (!params.external_lib_file.empty()) {
      VLOG_0 << "loading external lib: " << params.external_lib_file << std::endl;
      _external_lib = LibraryLoader::LoadSharedLibrary(params.external_lib_file);
      if (_external_lib == nullptr) {
        VLOG_0 << "can't load lib: " << params.external_lib_file << std::endl;
        return false;
      }
      // get global function pointers from library and call them
      void *lib_func_1 = LibraryLoader::GetLibraryFunc("setEvalParams", _external_lib);
      void *lib_func_2 = LibraryLoader::GetLibraryFunc("setModuleStepTime", _external_lib);
      void *lib_func_3 = LibraryLoader::GetLibraryFunc("setTotalEgoNum", _external_lib);
      auto lib_set_eval_params = reinterpret_cast<void (*)(const EvalParams &)>(lib_func_1);
      auto lib_set_module_step_time = reinterpret_cast<void (*)(double)>(lib_func_2);
      auto lib_set_total_ego_num = reinterpret_cast<void (*)(size_t)>(lib_func_3);
      lib_set_eval_params(params);
      lib_set_module_step_time(getModuleStepTime());
      lib_set_total_ego_num(getTotalEgoNum());
    }
  } catch (const std::exception &e) {
    VLOG_0 << params.external_lib_file << " loading user library or get global function error: " << e.what()
           << std::endl;
    LibraryLoader::UnLoadLibrary(_external_lib);
    _external_lib = nullptr;
    return false;
  }

  // create eval algorithm according to eval cfg
  CreateEvalAlgorithms(m_initHelper.GetEvalCfgs());

  // init all eval algorithm
  InitEvalAlgorithms(_eval_maps[C_EvalAlgorithmType::No_Result_Check], helper);
  InitEvalAlgorithms(_eval_maps[C_EvalAlgorithmType::Need_Result_Check], helper);
  InitEvalAlgorithms(_eval_maps[C_EvalAlgorithmType::External_Algorithm], helper, true);

  // mark start time of test
  ReportHelper::SetStartSecond(*m_stopHelper.GetReport(), EvalHelper::GetSystemUnixTime());

  // create or open managed_shared_memory, and set grading_start_time.
  if (isReportEnabled()) {
    boost::interprocess::named_mutex report_mtx(boost::interprocess::open_or_create, "grading_named_mutex");
    boost::interprocess::scoped_lock<boost::interprocess::named_mutex> lock(report_mtx);
    boost::interprocess::managed_shared_memory managed_shm(boost::interprocess::open_or_create, "grading_shared_memory",
                                                           1024);
    int64_t *grading_start_time = managed_shm.find_or_construct<int64_t>("grading_start_time")(
        static_cast<int64_t>(EvalHelper::GetSystemUnixTime()));
    _start_system_t = EvalHelper::GetCurrentSystemTime(*grading_start_time);
    VLOG_0 << "The grading start_time: " << _start_system_t << std::endl;
  }

  return true;
}

bool EvalManager::Step(tx_sim::StepHelper &helper) {
  // timestamp and state
  _cur_t_ms = helper.timestamp();
  m_executed = true;

  // set timestamp in grading.proto for simcity
  sim_msg::Grading &grading_msg = m_stepHelper.GetGradingMsg();
  grading_msg.mutable_event_detector()->set_cur_t_second(_cur_t_ms * 0.001);
  grading_msg.mutable_event_detector()->set_dt_second((_cur_t_ms - m_pre_t_ms) * 0.001);
  grading_msg.mutable_timestamp()->set_timestamp(_cur_t_ms);

  m_pre_t_ms = _cur_t_ms;

  // get module config
  std::string module_cfg_str;
  helper.GetSubscribedMessage(topic::MODULE_CONFIG, module_cfg_str);
  m_stepHelper.SetModuleCfg(module_cfg_str);

  // should stop scenario
  bool should_stop_scenario = false;
  std::string reason;

  // do update
  if (!IsModuleValid()) {
    LOG_ERROR << "module check error.\n";
    return true;
  }

  // update message
  for (int i = 0; i < m_params.m_sub_topics.size(); ++i) {
    const std::string &topic = m_params.m_sub_topics.at(i);
    std::string payload;
    helper.GetSubscribedMessage(topic, payload);
    _msg_mgr->Add(_cur_t_ms, topic, payload);
  }

  // update eval helper
  m_stepHelper.Update(_cur_t_ms);

  // update eval algorithm step
  UpdateEvalAlgorithms(helper, should_stop_scenario, reason);

  // if stop scenario
  if (should_stop_scenario) helper.StopScenario(reason);

  return true;
}

bool EvalManager::Stop(tx_sim::StopHelper &helper) {
  // collect eval result
  bool eval_pass = true;
  std::string reason;
  StopEvalAlgorithms(helper, eval_pass, reason);

  // legacy grading report
  helper.set_feedback("Time", std::to_string(_cur_t_ms));
  helper.set_feedback("L3_Result", "");
  helper.set_feedback("is_pass", std::to_string(eval_pass));
  helper.set_feedback("reason", reason);
  helper.set_feedback("SceneName", _scenario_name);

  // feedback
  auto feedback = m_stopHelper.GetFeedback();
  for (auto iter = feedback.begin(); iter != feedback.end(); ++iter) {
    helper.set_feedback(iter->first, iter->second);
  }

  // legacy grading report
  sim_msg::Grading_Statistics &_grading_statistics = m_stopHelper.GetGradingStatistics();
  _grading_statistics.set_scene_name(_scenario_name);
  _grading_statistics.set_report_date(EvalHelper::GetCurrentSystemTime());
  _grading_statistics.set_is_pass(eval_pass);
  _grading_statistics.set_reason(reason);
  _grading_statistics.set_total_time(_cur_t_ms / 1000.0);
  _grading_statistics.set_grading_xml(_grading_cfg_path);

  // post process grading report
  if (isReportEnabled()) {
    // get module status
    std::string module_status = helper.get_module_status();
    m_stepHelper.SetModuleStatus(module_status);

    // report object
    sim_msg::TestReport *report = m_stopHelper.GetReport();
    std::string report_json_file = GenReportJsonFileName(_scenario_name, _start_system_t);
    ReportHelper::SetReportInfo(*report, _scenario_name, m_initHelper.GetScenarioInfo().m_scene_description,
                                m_initHelper.GetScenarioInfo().m_scene_id,
                                m_initHelper.GetScenarioInfo().m_scene_version,
                                m_initHelper.GetScenarioInfo().m_date_version);

    ReportHelper::SetEndSecond(*report, EvalHelper::GetSystemUnixTime());
    ReportHelper::ReportPostProcess(*report, eval_pass, m_stepHelper.GetModuleCfg(),
                                    _actor_mgr->getParameters().sceneDescription, reason, INT32_MIN, INT32_MIN);

    if (m_executed) {
      // do inter proccess communication for ego group reports
      std::unique_ptr<boost::interprocess::named_mutex> report_mtx_ptr(
          new boost::interprocess::named_mutex(boost::interprocess::open_or_create, "grading_named_mutex"));
      report_mtx_ptr->lock();
      // process for ego group reports. merge this report with the previous report
      sim_msg::TestReport previous_report;
      sim_msg::TestReport_TestState previous_result = ReportHelper::GetPreviousReport(
          &previous_report, m_initHelper.GetScenarioInfo().m_report_dir, report_json_file);
      if (previous_result != sim_msg::TestReport_TestState_SKIPPED) {
        ReportHelper::MergeReport(std::move(previous_report), *report);
      }

      ReportHelper::DumpReportJson2File(m_stopHelper.GetReportUniPtr(), m_initHelper.GetScenarioInfo().m_report_dir,
                                        report_json_file, std::move(report_mtx_ptr), m_params.m_sync_save_report,
                                        previous_result);
    } else {
      // manually remove the mutex and shared memory
      boost::interprocess::named_mutex::remove("grading_named_mutex");
      boost::interprocess::shared_memory_object::remove("grading_shared_memory");
      VLOG_0 << "grading_named_mutex and grading_shared_memory are removed for m_executed is false." << std::endl;
    }
  }
  return true;
}

/*=========== eval helper tools ===========*/
sim_msg::Grading EvalManager::GetGradingMsg() { return m_stepHelper.GetGradingMsg(); }

sim_msg::Grading_Statistics EvalManager::GetGradingStatMsg() { return m_stopHelper.GetGradingStatistics(); }

// get grading.json file path and .sim config, active after 20230528
std::string EvalManager::GetEvalCfgFromSim(tx_sim::ResetHelper &helper, ScenarioInfo &scenario_info) {
  std::string evalCfgFile;
  EvalCfgLoader sim_loader;

  if (sim_loader.LoadXMLFile(helper.scenario_file_path())) {
    // get grading.json and sim env
    sim_loader.ParseFromSim(evalCfgFile, scenario_info);
  } else {
    LOG(ERROR) << "fail to open .sim file " << helper.scenario_file_path() << "\n";
  }
  if (CFileUtils::Exist(scenario_info.m_cloud_dir)) {
    // get grading.xml in cloud env
    evalCfgFile = CFileUtils::JoinPath(scenario_info.m_cloud_dir, "grading.json");
    VLOG_0 << "cloud eval config file is " << evalCfgFile << "\n";
  }
  // if previous grading.xml file does not exist, use default one
  if (!CFileUtils::IsFileExist(evalCfgFile)) {
    auto scenario_dir = CFileUtils::ParentDir(helper.scenario_file_path());
    evalCfgFile = CFileUtils::JoinPath(scenario_dir, "../grading.json");
  }
  VLOG_0 << "eval config file is " << evalCfgFile << "\n";

  return evalCfgFile;
}

std::string EvalManager::GenReportJsonFileName(const std::string &scene_name, const std::string &date) {
  return scene_name + "." + date + ".dict";
}

void EvalManager::Release() {
  _eval_maps.clear();

  // unload external library
  if (_external_lib != nullptr) {
    LibraryLoader::UnLoadLibrary(_external_lib);
    _external_lib = nullptr;
    VLOG_0 << "external library is unloaded";
  }
}
}  // namespace eval
