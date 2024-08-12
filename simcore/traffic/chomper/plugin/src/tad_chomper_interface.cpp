// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_chomper_interface.h"
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_parallel_def.h"
#include "version.h"
#if USE_TBB
#  if TBB_NEW_API
#    define TBB_PREVIEW_GLOBAL_CONTROL 1
#    include <tbb/global_control.h>
#  else
#    include "tbb/task_scheduler_init.h"
#  endif /*TBB_NEW_API*/
#endif   /*USE_TBB*/
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/optional.hpp>
#include "SceneLoader/tx_tadloader.h"
#include "tad_vehicle_behavior_configure_utils.h"
#include "worldsim_sim_template.h"
#include "boost/dll/alias.hpp"
#include "boost/make_shared.hpp"
#include "boost/shared_ptr.hpp"
#include "tx_path_utils.h"
#include "tx_string_utils.h"
#include "tx_tadsim_flags.h"

class ChomperGAImpl : public ChomperGAInterface {
 protected:
  virtual std::string app_name() const TX_NOEXCEPT { return "Chomper_ga"; }
  boost::optional<sim_msg::Visualizer2TrafficCommand> m_input_cmd;
  TrafficFlow::txPassiveSimTemplatePtr m_SimPtr = nullptr;
  Base::txInt step_no = 0;
  Base::txFloat history_displacement = 0.0;
  Base::txFloat m_cur_frame_maxdistance = 0.0;
  std::list<Base::txFloat> m_list_frame_ave_velocity;
  std::list<Base::txFloat> m_vec_V_Output;

 public:
  static boost::shared_ptr<ChomperGAInterface> create(void) { return boost::make_shared<ChomperGAImpl>(); }

 public:
  ChomperGAImpl() {
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
#  endif /*TBB_NEW_API*/
#endif   /*USE_TBB*/

#if 1
    FLAGS_log_dir = ".";

#endif
    LOG(WARNING) << TX_VARS_NAME(Load_Module_Path, boost::dll::program_location())
                 << TX_VARS_NAME(module_name, module_name()) << TX_VARS_NAME(module_version, module_version());
  }
  virtual ~ChomperGAImpl() { std::cout << "ChomperGAImpl Release." << std::endl; }

  const char *module_name(void) const noexcept override { return FILE_VERSION_STR(STR_GIT_BRANCH, STR_COMMIT_ID); }

  const char *module_version(void) const noexcept override { return _ST(PRO_PATH); }

  char *convert(const std::string &s) TX_NOEXCEPT {
    char *pc = new char[s.size() + 1];
    std::strcpy(pc, s.c_str());
    return pc;
  }

  void fresh_log() noexcept {
    FLAGS_logtostderr = 0;

    FLAGS_logbufsecs = 0;

    FLAGS_max_log_size = 1000;

    FLAGS_stop_logging_if_full_disk = true;

    // google::SetLogFilenameExtension("stlog");

    /*FLAGS_logtostderr = true;*/
    FLAGS_alsologtostderr = true;

    FLAGS_colorlogtostderr = false;

    google::InitGoogleLogging(app_name().c_str());

    LOG(INFO) << "Log init success." << TX_VARS(FLAGS_log_dir);
  }

  void fresh_log_level(std::string flags_params) TX_NOEXCEPT {
    auto paramsVec = Utils::SpliteStringVector(flags_params, Base::txString(" "));
    if (_NonEmpty_(paramsVec)) {
      std::string strAppInfo;
      Utils::GetAppPath(app_name(), strAppInfo);
      paramsVec.insert(paramsVec.begin(), strAppInfo);
      int argc = paramsVec.size();
      char **argv = new char *[argc + 1];
      for (int idx = 0; idx < argc; ++idx) {
        Utils::trim(paramsVec[idx]);
        argv[idx] = convert(paramsVec[idx]);
      }
      google::AllowCommandLineReparsing();
      google::ParseCommandLineFlags(&argc, &argv, true TX_MARK("erase gflags item."));
      fresh_log();
      PrintFlagsValues_TADSim();
    }
    return;
  }

  ErrCode init(std::string input_params) noexcept override {
    LOG(INFO) << TX_VARS(input_params);
    // fresh_log_level(input_params);
    return ChomperGAInterface::gaSucc;
  }

  ErrCode bool2ErrCode(const Base::txBool in) const TX_NOEXCEPT {
    return ((in) ? (ErrCode::gaSucc) : (ErrCode::gaErr));
  }

  void reset_behavior_params(const Base::txFloat cc0, const Base::txFloat cc1, const Base::txFloat cc2) noexcept {
    const auto cfg_path =
        boost::dll::program_location().parent_path().string() + Base::txString("/") + FLAGS_behavior_cfg_name;
    LOG(INFO) << "Chmoper config : " << cfg_path;
    Utils::BehaviorCfgDB::Initialize(cfg_path, Utils::ConfigType::file_path, "normal", cc0, cc1, cc2);
    Utils::BehaviorCfgDB::ResetDefaultCfg();
  }

  void clear_status_vars() noexcept {
    step_no = 0;
    m_input_cmd = boost::none;
    m_SimPtr = nullptr;
    history_displacement = 0.0;
    m_cur_frame_maxdistance = 0.0;
    m_list_frame_ave_velocity.clear();
    m_vec_V_Output.clear();
  }

  ErrCode connect(const sim_msg::Visualizer2TrafficCommand &v2tcmd) noexcept override {
    LOG(INFO) << TX_VARS_NAME(v2tcmd, v2tcmd.DebugString());
    clear_status_vars();
    m_input_cmd = v2tcmd;
    m_SimPtr = std::make_shared<TrafficFlow::ChomperGATemplate>();
    auto loader = std::make_shared<SceneLoader::TAD_SceneLoader>();
    Base::txString strSceneDataPath, strHadmapPath;
    const Base::txBool res_Sim2SceneFile =
        SceneLoader::TAD_SceneLoader::Sim2SceneFile(scenario_file_path(), strSceneDataPath, strHadmapPath);
    if (NonNull_Pointer(m_SimPtr) && NonNull_Pointer(loader) && CallSucc(res_Sim2SceneFile)) {
      m_SimPtr->CreateSystemPassive(scenario_file_path(), strSceneDataPath, strHadmapPath);
      m_SimPtr->ResetVarsPassive(m_SimPtr->TrafficSystemPtrPassive());
      reset_behavior_params(cc0(), cc1(), cc2());
      Base::txBool res = (m_SimPtr->CreateEnvPassive(loader, scenario_file_path(), strSceneDataPath, strHadmapPath));
      return bool2ErrCode(res);
    } else {
      LOG(WARNING) << "create loop failure.";
      return ChomperGAInterface::gaErr;
    }
  }

  Base::txBool check_finish() noexcept {
    history_displacement += m_cur_frame_maxdistance;
    if (history_displacement >= max_displacement()) {
      std::ostringstream oss;
      std::copy(std::begin(m_list_frame_ave_velocity), std::end(m_list_frame_ave_velocity),
                std::ostream_iterator<float>(oss, " "));
      Base::txFloat cur_sample_interval_ave_velocity =
          std::accumulate(std::begin(m_list_frame_ave_velocity), std::end(m_list_frame_ave_velocity), 0.0) /
          (m_list_frame_ave_velocity.size());
      m_vec_V_Output.push_back(cur_sample_interval_ave_velocity);
      m_list_frame_ave_velocity.clear();
      LOG_IF(WARNING, FLAGS_LogLevel_Visualizer)
          << TX_VARS(history_displacement) << TX_VARS(cur_sample_interval_ave_velocity) << TX_VARS(oss.str());
      history_displacement = 0.0;
      if (m_vec_V_Output.size() >= TT_ObsCount()) {
        return true;
      }
    }
    return false;
  }

  std::list<Base::txFloat> compute_displacement_field(Base::IElementManagerPtr elemMgr) const noexcept {
    std::list<Base::txFloat> retLinearField;
    if (NonNull_Pointer(elemMgr)) {
      const auto &refAllVehicles = elemMgr->GetAllVehiclePtr();
      for (const auto &veh_ptr : refAllVehicles) {
        if (NonNull_Pointer(veh_ptr) && veh_ptr->IsAlive()) {
          retLinearField.emplace_back(veh_ptr->GetDisplacement());
        }
      }
    }
    LOG_IF(WARNING, FLAGS_LogLevel_Visualizer) << TX_VARS_NAME(DisplacementFieldSize, retLinearField.size());
    return retLinearField;
  }

  std::list<Base::txFloat> compute_velocity_field(Base::IElementManagerPtr elemMgr) const noexcept {
    std::list<Base::txFloat> retLinearField;
    if (NonNull_Pointer(elemMgr)) {
      const auto &refAllVehicles = elemMgr->GetAllVehiclePtr();
      for (const auto &veh_ptr : refAllVehicles) {
        if (NonNull_Pointer(veh_ptr) && veh_ptr->IsAlive()) {
          retLinearField.emplace_back(veh_ptr->GetVelocity());
          LOG_IF(WARNING, FLAGS_LogLevel_Visualizer)
              << TX_VARS_NAME(v_id, veh_ptr->Id()) << TX_VARS_NAME(velocity, veh_ptr->GetVelocity());
        }
      }
    }
    LOG_IF(WARNING, FLAGS_LogLevel_Visualizer) << TX_VARS_NAME(VelocityFieldSize, retLinearField.size());
    return retLinearField;
  }

  Base::txBool post_data_statistics() noexcept {
    if (NonNull_Pointer(m_SimPtr) && NonNull_Pointer(m_SimPtr->TrafficSystemPtrPassive()) &&
        NonNull_Pointer(m_SimPtr->TrafficSystemPtrPassive()->ElemMgr())) {
      auto elemMgr = m_SimPtr->TrafficSystemPtrPassive()->ElemMgr();
      m_cur_frame_maxdistance = 0.0;
      Base::txFloat cur_frame_ave_velocity = 0.0;
      const auto displacementField = compute_displacement_field(elemMgr);
      if (CallFail(displacementField.empty())) {
        m_cur_frame_maxdistance = *(std::max_element(std::begin(displacementField), std::end(displacementField)));
      }

      const auto velocityField = compute_velocity_field(elemMgr);
      if (CallFail(velocityField.empty())) {
        cur_frame_ave_velocity =
            std::accumulate(std::begin(velocityField), std::end(velocityField), 0.0) / (velocityField.size());
      }
      m_list_frame_ave_velocity.push_back(cur_frame_ave_velocity);
      LOG_IF(INFO, FLAGS_LogLevel_Visualizer) << TX_VARS_NAME(MaxMoveDistance, m_cur_frame_maxdistance)
                                              << TX_VARS_NAME(cur_frame_ave_velocity, cur_frame_ave_velocity);
    } else {
      return false;
    }
  }

  ErrCode step(sim_msg::Traffic2VisualizerCommand **result, bool &isFinish) noexcept override {
    *result = new sim_msg::Traffic2VisualizerCommand();
    auto &resultInfo = **result;
    isFinish = false;
    if (NonNull_Pointer(m_SimPtr)) {
      step_no++;
      const Base::txFloat cur_time_stamp_ms = step_no * timestep_ms();
      if (step_no <= max_step()) {
        const Base::txBool res =
            m_SimPtr->SimulationPassive(cur_time_stamp_ms, nullptr, resultInfo.mutable_trafficinfo(), nullptr);
        if (CallSucc(res) && CallSucc(post_data_statistics())) {
          resultInfo.set_cmdno(1);
          resultInfo.set_taskid(taskId());
          resultInfo.set_scene_timestamp(cur_time_stamp_ms);
          if (CallSucc(check_finish())) {
            isFinish = true;
            resultInfo.set_cmdno(2);
            resultInfo.set_taskid(taskId());
            resultInfo.set_scene_timestamp(cur_time_stamp_ms);
            resultInfo.set_param0(cc0());
            resultInfo.set_param1(cc1());
            resultInfo.set_param2(cc2());
            resultInfo.set_tt_output(cur_time_stamp_ms);
            resultInfo.clear_v_output();
            for (const auto averVelocity : m_vec_V_Output) {
              resultInfo.add_v_output(averVelocity);
            }
            m_vec_V_Output.clear();
          }
          // LOG(WARNING) << "simulation is runing " << step_no << " step." << std::endl;
          return ChomperGAInterface::gaSucc;
        } else {
          LOG(WARNING) << "simulation is failure. " << TX_VARS(cur_time_stamp_ms);
          return ChomperGAInterface::gaErr;
        }
      } else {
        const Base::txFloat past_cur_time_stamp_ms = (cur_time_stamp_ms - timestep_ms());
        isFinish = true;
        resultInfo.set_cmdno(2);
        resultInfo.set_taskid(taskId());
        resultInfo.set_scene_timestamp(past_cur_time_stamp_ms);
        resultInfo.set_param0(cc0());
        resultInfo.set_param1(cc1());
        resultInfo.set_param2(cc2());
        resultInfo.set_tt_output(past_cur_time_stamp_ms);
        resultInfo.clear_v_output();
        for (const auto averVelocity : m_vec_V_Output) {
          resultInfo.add_v_output(averVelocity);
        }
        m_vec_V_Output.clear();
        LOG(WARNING) << "simulation is finish. " << TX_COND(isFinish) << TX_VARS(max_step());
        return ChomperGAInterface::gaErr;
      }
    } else {
      LOG(WARNING) << "simulation is in valid. m_SimPtr is nullptr.";
      return ChomperGAInterface::gaErr;
    }
  }

  ErrCode release() noexcept override {
    LOG(INFO) << "release..";
    clear_status_vars();
    return ChomperGAInterface::gaSucc;
  }

  Base::txBool isValid() const TX_NOEXCEPT { return m_input_cmd.has_value(); }

#define _GET_VAR_(_var_name_, _default_val_) ((isValid()) ? ((*m_input_cmd)._var_name_()) : (_default_val_));
  Base::txInt taskId() const TX_NOEXCEPT { return _GET_VAR_(taskid, 0); }
  Base::txInt cmdNo() const TX_NOEXCEPT { return _GET_VAR_(cmdno, 0); }
  Base::txFloat timestep_ms() const TX_NOEXCEPT { return _GET_VAR_(timestep, 20); }
  Base::txFloat TT_V() const TX_NOEXCEPT { return _GET_VAR_(tt_v, 0.0); }
  Base::txFloat max_displacement() const TX_NOEXCEPT { return _GET_VAR_(max_displacement, 0.0); }
  Base::txInt max_step() const TX_NOEXCEPT { return _GET_VAR_(max_step, 0); }

  Base::txFloat cc0() const TX_NOEXCEPT { return _GET_VAR_(driving_param_0, 0.0); }
  Base::txFloat cc1() const TX_NOEXCEPT { return _GET_VAR_(driving_param_1, 0.0); }
  Base::txFloat cc2() const TX_NOEXCEPT { return _GET_VAR_(driving_param_2, 0.0); }

  Base::txInt TT_ObsCount() const TX_NOEXCEPT { return _GET_VAR_(tt_obscount, 0); }
  Base::txString scenario_file_path() const TX_NOEXCEPT { return _GET_VAR_(scenario_file_path, ""); }
  Base::txFloat front_distance() const TX_NOEXCEPT { return _GET_VAR_(front_distance, 0.0); }
#undef _GET_VAR_
};

BOOST_DLL_ALIAS(ChomperGAImpl::create, create_plugin)

std::shared_ptr<ChomperGAInterface> createGATrainer(void) { return std::make_shared<ChomperGAImpl>(); }
