// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "manager/eval_helper.h"
#include "json/json.h"
#include "report_helper.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace eval {
/*==============================*/
EvalHelper::EvalHelper() {
  _actor_mgr = CActorManager::GetInstance();
  _map_mgr = CMapManager::GetInstance();
  _msg_mgr = CMsgManager::GetInstance();
}

std::string EvalHelper::GetCurrentSystemTime(const int64_t unix_time, const bool detail_time) {
  struct tm *ptm = nullptr;
  if (unix_time >= 0) {
    // transfer given unix time to local time
    std::time_t timestamp = unix_time;
    ptm = std::gmtime(&timestamp);
    // transfer to Beijing time
    ptm->tm_hour += 8;
    std::mktime(ptm);
  } else {
    // get current local time
    auto tt = time(0);
    ptm = std::localtime(&tt);
  }

  std::string date =
      std::to_string(ptm->tm_year + 1900) + "_" + std::to_string(ptm->tm_mon + 1) + "_" + std::to_string(ptm->tm_mday);

  if (detail_time) {
    date = date + "_" + std::to_string(ptm->tm_hour) + "_" + std::to_string(ptm->tm_min) + "_" +
           std::to_string(ptm->tm_sec);
  }

  return date;
}

void EvalHelper::SetModuleCfg(const std::string &module_cfg) {
  // print
  VLOG_2 << "module config json string:" << module_cfg << "\n";

  if (!module_cfg.empty()) {
    // clear module config
    m_module_cfg.Clear();

    // get module config
    try {
      Json::Value json_root;
      JSONCPP_STRING errs;
      Json::CharReaderBuilder readerBuilder;
      std::unique_ptr<Json::CharReader> const json_reader(readerBuilder.newCharReader());
      if (json_reader->parse(module_cfg.c_str(), module_cfg.c_str() + module_cfg.length(), &json_root, &errs)) {
        const Json::Value &json_configs = json_root["configs"];

        // pre-check
        if (json_configs.isNull()) {
          LOG_ERROR << "no configs in module config:" << module_cfg << "\n";
          return;
        }

        for (auto i = 0; i < json_configs.size(); ++i) {
          const Json::Value &config = json_configs[i];
          std::string elf_path((const char *)config["binPath"].asCString());
          if (elf_path.empty()) elf_path = config["soPath"].asCString();
          m_module_cfg.m_module_paths.emplace_back((const char *)elf_path.c_str());
          m_module_cfg.m_module_names.emplace_back((const char *)config["name"].asCString());
          m_module_cfg.m_module_types.push_back(config["category"].asInt());
        }
      } else {
        LOG_ERROR << "fail to parse module config from string:" << module_cfg << "\n";
      }
    } catch (const char *msg) {
      LOG_ERROR << "exception when parse module config, " << msg << "\n";
    } catch (const std::exception &e) {
      LOG_ERROR << "exception when parse module config, " << e.what() << "\n";
    } catch (...) {
      LOG_ERROR << "exception in void EvalHelper::SetModuleCfg(const std::string &module_cfg).\n";
    }
  }
}

void EvalHelper::SetModuleStatus(const std::string &module_status) {
  // print
  VLOG_2 << "module status json string:" << module_status << "\n";

  // pre check
  if (module_status.empty()) {
    return;
  }

  // get module status
  try {
    Json::Value json_root;
    JSONCPP_STRING errs;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const json_reader(readerBuilder.newCharReader());
    if (json_reader->parse(module_status.c_str(), module_status.c_str() + module_status.length(), &json_root, &errs)) {
      // get all json members
      const Json::Value::Members &json_members = json_root.getMemberNames();

      // get infomation module by module
      for (const auto &module_name : m_module_cfg.m_module_names) {
        // find information from json
        for (const auto &member_name : json_members) {
          if (member_name == module_name) {
            const auto &status = json_root[member_name];

            double step_time = status["executePeriod"].asInt() * 0.001;
            double total_time = status["totalStepRealTime"].asDouble() * 0.001;
            int step_count = status["stepCount"].asInt();
            int overrun_count = status["timeOut"].asInt();
            double avg_time = 0.0, duty_cycle = 0.0;
            double avg_expected = -1;
            if (step_count > 0 && step_time > 0) {
              avg_time = total_time / step_count;
              duty_cycle = avg_time / step_time * 100.0;
            }

            auto toStringWithPrecision = [](double in, int precision = 6) -> std::string {
              std::string out;
              std::stringstream ss;
              ss << std::fixed << std::setprecision(precision) << in;
              out = ss.str();
              return out;
            };

            m_module_cfg.m_avg_steptime[member_name] = std::to_string(avg_time);
            m_module_cfg.m_expected_avg_steptime[member_name] = std::to_string(avg_expected);
            m_module_cfg.m_steptime[member_name] = toStringWithPrecision(step_time, 2);
            m_module_cfg.m_dutycycle[member_name] = std::to_string(duty_cycle);
            m_module_cfg.m_overrun_count[member_name] = std::to_string(overrun_count);
          }
        }
      }
    } else {
      LOG_ERROR << "fail to parse module status from string:" << module_status << "\n";
    }
  } catch (const char *msg) {
    LOG_ERROR << "exception when parse module status, " << msg << "\n";
  } catch (const std::exception &e) {
    LOG_ERROR << "exception when parse module config, " << e.what() << "\n";
  } catch (...) {
    LOG_ERROR << "exception in void EvalHelper::SetModuleStatus(const std::string &module_status).\n";
  }
}

double EvalHelper::GetSystemUnixTime() {
  time_t t_now = time(0);
  return static_cast<double>(t_now);
}

/*==============================*/
EvalCfg EvalInit::GetEvalCfg(const std::string &kpi_name) const {
  if (_eval_cfgs.find(kpi_name) != _eval_cfgs.end()) {
    return _eval_cfgs.find(kpi_name)->second;
  }
  return EvalCfg();
}

/**
 * @brief get grading kpi group
 * @return
 */
const sim_msg::GradingKpiGroup &EvalInit::getGradingKpiGroup() { return _eval_cfg_loader.getGradingKpiGroup(); }

/**
 * @brief get grading kpi from kpi group by kpi name
 * @param kpiName
 * @return true if bingo, otherwise false
 */
bool EvalInit::getGradingKpiByName(const std::string &kpiName, sim_msg::GradingKpiGroup_GradingKpi &kpiOut) {
  return _eval_cfg_loader.getGradingKpiByName(kpiName, kpiOut);
}

void EvalInit::CalReportDir() {
  if (m_scenario_info.m_report_dir.empty()) {
    // default pblog folder
#ifdef _WIN32
    m_scenario_info.m_report_dir = std::string(getenv("HOMEDRIVE")) + std::string(getenv("HOMEPATH")) +
                                   "\\AppData\\Roaming\\tadsim\\data\\service_data\\sim_data\\pblog";
#else
    m_scenario_info.m_report_dir = std::string(getenv("HOME")) + "/.config/tadsim/data/service_data/sim_data/pblog";
#endif
  }

  VLOG_0 << "report dir is " << m_scenario_info.m_report_dir << "\n";
}

bool EvalInit::Init(const ScenarioInfo &scenario_info) {
  // clear
  m_scenario_info = ScenarioInfo();
  _eval_cfgs.clear();

  // module valid
  if (IsModuleValid()) {
    // set scenario info
    m_scenario_info = scenario_info;

    // calculate report dir
    CalReportDir();

    // build start location
    {
      _msg_mgr->Add(0.0, topic::LOCATION, m_scenario_info.m_start_location);
      _actor_mgr->BuildActors(0.0, _msg_mgr->Get(topic::LOCATION));
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    // load grading config
    _eval_cfg_loader.loadGradingKpiGroup(scenario_info.m_eval_cfg_file);
  }

  return false;
}

/*==============================*/
EvalStep::~EvalStep() { m_veh_flow.Stop(); }

bool EvalStep::Init() {
  _grading.Clear();
  m_veh_flow.Init();
  return true;
}

bool EvalStep::Update(double t_ms) {
  VLOG_1 << "sim time is " << t_ms << " ms.\n";

  // set sim time, uint s
  SetSimTime(t_ms * 0.001);
  VLOG_2 << "[Update] start\n";
  if (IsModuleValid()) {
    // build actors
    _actor_mgr->ResetActorState();
    _actor_mgr->BuildActors(t_ms, _msg_mgr->Get(topic::ENVIRONMENTAL));
    _actor_mgr->BuildActors(t_ms, _msg_mgr->Get(topic::LOCATION));
    _actor_mgr->BuildActors(t_ms, _msg_mgr->Get(topic::LOCATION_TRAILER));
    _actor_mgr->BuildActors(t_ms, _msg_mgr->Get(topic::LOCATION_UNION));
    _actor_mgr->BuildActors(t_ms, _msg_mgr->Get(topic::TRAFFIC));

    // state-flow update
    UpdateEgoStateFlow();
    VLOG_1 << "ego behavior is " << m_veh_flow.GetOutput().m_veh_behav << "\n";

    // calculate leading vehicle
    CalLeadingActor(_actor_mgr);

    // for test only
    /*
    {
      // find vehicle in front of ego
      auto vehicleActors = FindFrontAndAdjacentVehicleActor(_actor_mgr->GetEgoFrontActorPtr());
      for (auto &vehicle : vehicleActors) {
        VLOG_0 << "vehicle ID:" << vehicle->GetID() << ", position:" << vehicle->GetLocation().GetPosition().GetX()
               << ", " << vehicle->GetLocation().GetPosition().GetY() << "\n";
      }

      // find static actor in front of ego
      auto staticActors = FindFrontStaticActor(_actor_mgr->GetEgoFrontActorPtr());
      for (auto &static_ : staticActors) {
        VLOG_0 << "static_ ID:" << static_->GetID() << ", position:" << static_->GetLocation().GetPosition().GetX()
               << ", " << static_->GetLocation().GetPosition().GetY() << "\n";
      }
    }
    */

    // FindFrontDynamicActor(_actor_mgr->GetEgoFrontActorPtr(), 0);
  }
  VLOG_2 << "[Update] end\n";
  return true;
}

// get vehicle actors which are in front of ego and adjacent to lane of ego
ActorList<CVehicleActorPtr> EvalStep::FindFrontAndAdjacentVehicleActor(CEgoActorPtr ego, double range) {
  ActorList<CVehicleActorPtr> actors;
  actors.reserve(const_max_actor_size);

  if (ego == nullptr) {
    VLOG_1 << "ego is nullptr.\n";
    return actors;
  }

  if (range < 1e-9) {
    VLOG_1 << "range is too small " << range << "\n";
    return actors;
  }

  // get ego position and orentation
  const auto &egoLoc = ego->GetLocation();
  const auto &egoPos = egoLoc.GetPosition();
  const auto &egoEuler = egoLoc.GetEuler();
  const auto &egoLaneID = ego->GetLaneID();

  if (!egoLaneID.IsLaneValid()) {
    VLOG_1 << "ego lane id is not valid." << "\n";
    egoLaneID.DebugShow("ego lane id --> ");
    return actors;
  }

  // get all vehicle actors
  auto vehicleActors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(ActorType::Actor_Vehicle);

  for (auto &actor : vehicleActors) {
    const auto &actorLaneID = actor->GetLaneID();

    if (!actorLaneID.IsLaneValid()) {
      VLOG_2 << "vehicle actor lane id not valid, whose ID is " << actor->GetID() << "\n";
      continue;
    }

    const auto &actorLoc = actor->GetLocation();
    const auto &actorPos = actorLoc.GetPosition();

    // check distance
    double dist = CEvalMath::AbsoluteDistance2D(actorPos, egoPos);
    if (dist > range) {
      continue;
    }

    // from ego to actor
    const auto &posVec = CEvalMath::Sub(actorPos, egoPos);
    double yaw = std::atan2(posVec.GetY(), posVec.GetX());
    CEuler euler{0, 0, yaw};

    double yawDiff = CEvalMath::YawDiff(egoEuler, euler);

    // actor is in front of ego
    if (yawDiff > -M_PI / 2 && yawDiff < M_PI / 2) {
      // same road
      if (egoLaneID.tx_road_id == actorLaneID.tx_road_id) {
        // adjacent lanes
        auto laneDiff = std::abs(egoLaneID.tx_lane_id - actorLaneID.tx_lane_id);

        if (laneDiff == 1) {
          actors.push_back(actor);
        }
      }
    }
  }

  return actors;
}

// get vehicle actors which are surrounding of ego and have same road
ActorList<CVehicleActorPtr> EvalStep::FindSurroundingVehicleActor(CEgoActorPtr ego, double range) {
  ActorList<CVehicleActorPtr> actors;
  actors.reserve(const_max_actor_size);

  if (ego == nullptr) {
    VLOG_1 << "ego is nullptr.\n";
    return actors;
  }

  if (range < 1e-9) {
    VLOG_1 << "range is too small " << range << "\n";
    return actors;
  }

  // get ego position and orentation
  const auto &egoLoc = ego->GetLocation();
  const auto &egoPos = egoLoc.GetPosition();
  const auto &egoEuler = egoLoc.GetEuler();
  const auto &egoLaneID = ego->GetLaneID();

  if (!egoLaneID.IsLaneValid()) {
    VLOG_1 << "ego lane id is not valid." << "\n";
    egoLaneID.DebugShow("ego lane id --> ");
    return actors;
  }

  // get all vehicle actors
  auto vehicleActors = _actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(ActorType::Actor_Vehicle);

  for (auto &actor : vehicleActors) {
    const auto &actorLaneID = actor->GetLaneID();

    if (!actorLaneID.IsLaneValid()) {
      VLOG_2 << "vehicle actor lane id not valid, whose ID is " << actor->GetID() << "\n";
      continue;
    }

    const auto &actorLoc = actor->GetLocation();
    const auto &actorPos = actorLoc.GetPosition();

    // check distance
    double dist = CEvalMath::AbsoluteDistance2D(actorPos, egoPos);
    if (dist > range) {
      continue;
    }

    // same road
    if (egoLaneID.tx_road_id == actorLaneID.tx_road_id) {
      actors.push_back(actor);
    }
  }

  return actors;
}

// get static actors which are in front of ego
ActorList<CStaticActorPtr> EvalStep::FindFrontStaticActor(CEgoActorPtr ego, double range) {
  ActorList<CStaticActorPtr> actors;
  actors.reserve(const_max_actor_size);

  if (ego == nullptr) {
    return actors;
  }

  if (range < 1e-9) {
    VLOG_1 << "range is too small " << range << "\n";
    return actors;
  }

  // get ego position and orentation
  const auto &egoLoc = ego->GetLocation();
  const auto &egoPos = egoLoc.GetPosition();
  const auto &egoEuler = egoLoc.GetEuler();

  // get all vehicle actors
  auto staticActors = _actor_mgr->GetFellowActorsByType<CStaticActorPtr>(ActorType::Actor_Static);

  for (auto &actor : staticActors) {
    const auto &actorLoc = actor->GetLocation();
    const auto &actorPos = actorLoc.GetPosition();

    // check distance
    double dist = CEvalMath::Distance2D(actorPos, egoPos);
    if (dist > range) {
      continue;
    }

    if (ego->CheckFrontOrRear(actor->GetLocation()) == 1) {
      VLOG_2 << "[FindFrontStaticActor] find dynamic actor [" << actor->GetTypeID() << "]\n";
      actors.push_back(actor);
    }
  }

  return actors;
}

static const std::vector<std::unordered_set<uint64_t>> g_dynamic_actor_set = {
    std::unordered_set<uint64_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18},  // Pedestrian
    std::unordered_set<uint64_t>{100, 101},                                                          // animal
    std::unordered_set<uint64_t>{201, 202, 203, 204, 205, 206, 308, 309, 310, 311},                  // bike
    std::unordered_set<uint64_t>{301, 302, 303, 304, 305, 306, 307},                                 // motor
    std::unordered_set<uint64_t>{401, 601, 602, 603, 604, 605, 606, 607, 608, 609}                   // PortCrane
};

DynamicActorType EvalStep::GetDynamicActorType(CDynamicActorPtr actor) {
  if (actor) {
    auto actor_type_id = actor->GetTypeID();
    for (int i = 0; i < g_dynamic_actor_set.size(); i++) {
      const auto &type_set = g_dynamic_actor_set[i];
      if (type_set.find(actor_type_id) != type_set.end()) {
        return DynamicActorType(i);
      }
    }
    return DynamicErrortype;
  }
  VLOG_2 << "[GetDynamicActorType] the actor is nullptr\n";
  return DynamicErrortype;
}

std::vector<ActorList<CDynamicActorPtr>> EvalStep::FindFrontDynamicActor(CEgoActorPtr ego,
                                                                         std::vector<eval::DynamicActorType> types,
                                                                         double range) {
  std::vector<ActorList<CDynamicActorPtr>> actors;

  if (ego == nullptr) {
    VLOG_2 << "ego is nullptr\n";
    return actors;
  }
  actors.reserve(types.size());

  if (range < 1e-9) {
    VLOG_2 << "range is too small " << range << "\n";
    return actors;
  }

  // get ego position and orentation
  const auto &egoLoc = ego->GetLocation();
  const auto &egoPos = egoLoc.GetPosition();
  const auto &egoEuler = egoLoc.GetEuler();

  // get all vehicle actors
  auto dynamicActors = _actor_mgr->GetFellowActorsByType<CDynamicActorPtr>(ActorType::Actor_Dynamic);

  for (auto type : types) {
    ActorList<CDynamicActorPtr> tmp;
    tmp.reserve(const_max_actor_size);
    for (auto &actor : dynamicActors) {
      // check type id
      VLOG_2 << "the actor id is " << actor->GetTypeID() << "\n";
      if (g_dynamic_actor_set[type].find(actor->GetTypeID()) == g_dynamic_actor_set[type].end()) continue;

      const auto &actorLoc = actor->GetLocation();
      const auto &actorPos = actorLoc.GetPosition();

      double dist = CEvalMath::Distance2D(actorPos, egoPos);
      if (dist > range) {
        continue;
      }

      if (ego->CheckFrontOrRear(actor->GetLocation()) == 1) {
        VLOG_2 << "[FindFrontDynamicActor] find dynamic actor [" << actor->GetTypeID() << "]\n";
        tmp.push_back(actor);
      }
    }
    actors.push_back(std::move(tmp));
  }

  return actors;
}

// update ego state flow
void EvalStep::UpdateEgoStateFlow() {
  VehInput input;
  m_ego_assemble.SetFront(_actor_mgr->GetEgoFrontActorPtr());
  m_ego_assemble.AddTrailer(_actor_mgr->GetEgoTrailerActorPtr());
  input.m_t_s = GetSimTime();
  if (m_ego_assemble.GetFront()) {
    input.m_lane_id = m_ego_assemble.GetFront()->GetLaneID();
    input.m_map_info = m_ego_assemble.GetFront()->GetMapInfo();
  }
  input.m_ego = &m_ego_assemble;

  // update state-flow
  m_veh_flow.Update(input);
}

void EvalStep::CalLeadingActor(const ActorManagerPtr &actor_mgr) {
  _leading_vehicle = nullptr;

  auto vehicle_actor_ptrs = actor_mgr->GetFellowActorsByType<CVehicleActorPtr>(Actor_Vehicle);
  auto ego_ptr = actor_mgr->GetEgoFrontActorPtr();

  if (ego_ptr) {
    // ego middle-front enu
    CPosition &&ego_front_enu = ego_ptr->TransMiddleFront2BaseCoord();
    double ego_width = ego_ptr->GetShape().GetWidth();
    double x_min = const_max_dist;

    for (auto fellow : vehicle_actor_ptrs) {
      if (fellow) {
        // fellow middle-rear enu
        CPosition &&fellow_tail_enu = fellow->TransMiddleRear2BaseCoord();
        double fellow_width = fellow->GetShape().GetWidth();

        // vector from ego to fellow
        EVector3d ego_to_fellow = CEvalMath::Sub(fellow_tail_enu, ego_front_enu);
        Eigen::Vector3d &&G_Vec = ego_to_fellow.GetPoint();

        const Eigen::Matrix3d &R_GB = ego_ptr->GetLocation().GetRotMatrix();
        const Eigen::Matrix3d &R_BG = R_GB.inverse();
        Eigen::Vector3d &&B_vec = R_BG * G_Vec;

        // find leading vehicle
        double y_max_dist = (ego_width + fellow_width) / 2.0;

        VLOG_2 << "fellow id:" << fellow->GetID() << ", y_max_dist:" << y_max_dist << ", x dist:" << B_vec[0]
               << ", y dist:" << B_vec[1] << "\n";

        if (std::abs(B_vec[1]) <= y_max_dist) {
          if (0.0 < B_vec[0] && B_vec[0] < x_min) {
            x_min = B_vec[0];
            _leading_vehicle = fellow;
          }
        }
      }
    }
  }

  if (_leading_vehicle) {
    VLOG_1 << "leading actor id is " << _leading_vehicle->GetID() << "\n";
  } else {
    VLOG_1 << "leading vehicle missing.\n";
  }
}

// init function
bool EvalStop::Init() {
  _grading_stat.Clear();

  _report.release();
  _ego_meta.release();
  _ego_cases.release();

  m_feedback.clear();

  _report = std::move(std::unique_ptr<sim_msg::TestReport>(new sim_msg::TestReport()));
  _ego_meta = std::move(std::unique_ptr<sim_msg::TestReport_EgoMeta>(new sim_msg::TestReport_EgoMeta()));
  _ego_cases = std::move(std::unique_ptr<sim_msg::TestReport_EgoCase>(new sim_msg::TestReport_EgoCase()));

  return true;
}
}  // namespace eval
