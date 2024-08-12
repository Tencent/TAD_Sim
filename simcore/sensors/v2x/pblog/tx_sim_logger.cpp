/**
 * @file txSimLogger.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "tx_sim_logger.h"
#include <stdio.h>
#include <iomanip>
#include "settings.pb.h"

#define SPLIT_LINE "======================================="

namespace pblog {

/**
 * @brief Construct a new tx Sim Logger::tx Sim Logger object
 *
 */
txSimLogger::txSimLogger() {
  _post_script = "";
  _data_logged = false;
  _sync_process = 0;
  LOG(INFO) << "Hi, my name is " << this->name << "." << std::endl;
}

/**
 * @brief Destroy the tx Sim Logger::tx Sim Logger object
 *
 */
txSimLogger::~txSimLogger() {
  DeleteEmptyLog(_data_logged);
  LOG(INFO) << this->name << " destroyed." << std::endl;
}

/**
 * @brief is in sub topic
 *
 * @param topicIn topic string
 * @return true if in sub topic
 * @return false
 */
bool txSimLogger::IsInSubTopic(const std::string& topicIn) {
  for (auto iterStr = _topic_list.begin(); iterStr != _topic_list.end(); ++iterStr) {
    if (topicIn == *iterStr) {
      return true;
    }
  }
  return false;
}

/**
 * @brief init function
 *
 * @param helper helper of txsim
 */
void txSimLogger::Init(tx_sim::InitHelper& helper) {
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "initializing protobuf logger module ...\n";

  std::string sync_str = helper.GetParameter("sync");
  if (!sync_str.empty()) {
    _sync_process = std::atoi(sync_str.c_str());
  }
  std::string sInputJson = helper.GetParameter("json");
  if (sInputJson == "false") m_bJson = false;
// get post process python script
#ifdef _WIN32
  _post_script = "dir";
#else
  _post_script = "ls";
#endif

  _pbLog_queue_ptr = nullptr;
}

/**
 * @brief Reset function
 *
 * @param helper helper of txsim
 */
void txSimLogger::Reset(tx_sim::ResetHelper& helper) {
  // here we could get some information(e.g. the map which the scenario runs on, suggested local origin coordinate
  // and the initial state of the ego car) of the current scenario.
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "logger | " << this->name << " reset.\n";

  sim_msg::Setting setting;
  setting.ParseFromString(helper.setting_pb());
  _log_file_path = setting.log_dir();
  if (_log_file_path.empty()) return;

  pblog::CPBLogCommon::GenPBLogFileName(_log_file_name);
  std::string scene_name = pblog::CPBLogCommon::GetSceneName(helper.scenario_file_path());

  _log_file_name = scene_name + "." + _log_file_name;
  if (m_bJson) _log_file_name += ".json";
#ifdef _WIN32
  _log_file_name = _log_file_path + "\\" + _log_file_name;
#else
  _log_file_name = _log_file_path + "/" + _log_file_name;
#endif

  LOG(INFO) << "logger | pb log file path is " << _log_file_name << "\n";

  // reset pb log queue object
  ClearPbLogQueuePtr();
  _pbLog_queue_ptr = new CPBLogQueue(1024);
  _pbLog_queue_ptr->Initialize(_log_file_name, PBLogQueueDir::PBLogQueue_Write, m_bJson);

  _data_logged = false;

  // save scenario info
  {
    std::string payload;
    setting.SerializeToString(&payload);
    if (m_bJson) {
      ProducePBLogEvent("Scenario_Info", setting.DebugString(), 0);
    } else {
      ProducePBLogEvent("Scenario_Info", payload, 0);
    }
  }
}

/**
 * @brief clear queue of pb log
 *
 */
void txSimLogger::ClearPbLogQueuePtr() {
  if (_pbLog_queue_ptr != nullptr) {
    _pbLog_queue_ptr->Clear(PBLogQueueDir::PBLogQueue_Write);
    delete _pbLog_queue_ptr;
    _pbLog_queue_ptr = nullptr;
  }
}

/**
 * @brief stop
 *
 * @param helper helper of txsim
 */
void txSimLogger::Stop(tx_sim::StopHelper& helper) {
  LOG(INFO) << SPLIT_LINE << "\n";

  ClearPbLogQueuePtr();

  // remove empty log file
  // DeleteEmptyLog(_data_logged);

  // post process
  if (_data_logged) {
    std::thread pp = std::thread(&txSimLogger::PostProcessFunc, this, _post_script, _log_file_name);
    if (_sync_process == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(4000));
      pp.detach();
      LOG(INFO) << "thread deteched.\n";
    } else {
      if (pp.joinable()) pp.join();
      LOG(INFO) << "post-proess thread quit.\n";
    }
  }

  LOG(INFO) << "logger | " << this->name << " module stop"
            << "\n";
}

/**
 * @brief delete empty log
 *
 * @param data_logged
 */
void txSimLogger::DeleteEmptyLog(const bool data_logged) {
  if (!data_logged) {
    std::string shell_cmd = "";
    int exit_state = 0;

#ifdef _WIN32
    shell_cmd = "del " + _log_file_name;
#else
    shell_cmd = "rm " + _log_file_name;
#endif

    pblog::CPBLogCommon::ExecCommand(shell_cmd, exit_state);
  }
}

/**
 * @brief handle function after process
 *
 * @param script cmd
 * @param logfile log file
 */
void txSimLogger::PostProcessFunc(const std::string script, const std::string logfile) {
#ifdef _WIN32
  std::string log_file_wrapper = "\"" + logfile + "\"";
#else
  std::string log_file_wrapper = "\'" + logfile + "\'";
#endif
  LOG(INFO) << "logger | processing " << log_file_wrapper << " with " << script << ".\n";

  // post process
  int exit_code = 0;
  std::string shell_cmd = script + " -f " + log_file_wrapper;

  std::string stdout_shell = pblog::CPBLogCommon::ExecCommand(shell_cmd, exit_code);
}

/**
 * @brief produce event to pb queue
 *
 * @param topic
 * @param payload_in string to be save
 * @param t_ms timestamp
 * @return true on success
 * @return false
 */
bool txSimLogger::ProducePBLogEvent(const std::string& topic, const std::string& payload_in, const int& t_ms) {
  // if(topic.size()>0 && payload_in.size() > 0){
  if (topic.size() > 0 && _pbLog_queue_ptr) {
    pblog::PBLogEvent event;

    pblog::CPBLogCommon::PB2Event(event, payload_in, topic, CPBLogCommon::milli_2_micro(t_ms));
    _pbLog_queue_ptr->ProduceLogEvent(event);

    return true;
  }

  return false;
}

/**
 * @brief step
 *
 * @param helper helper of txsim
 */
void txSimLogger::Step(tx_sim::StepHelper& helper) {
  // 1. get current simulation timestamp.
  int64_t simTimeStamp = CPBLogCommon::milli_2_micro(helper.timestamp());
  LOG_IF(INFO, Log_Switch) << SPLIT_LINE << "\n";
  LOG_IF(INFO, Log_Switch) << "logger | time stamp: " << helper.timestamp() << "\n";

  // save all topic into protobuf log
  for (auto topicStr = _topic_list.begin(); topicStr != _topic_list.end(); ++topicStr) {
    if (this->IsInSubTopic(*topicStr)) {
      this->payload = "";

      helper.GetSubscribedMessage(*topicStr, this->payload);

      if (this->ProducePBLogEvent(*topicStr, this->payload, helper.timestamp())) {
        _data_logged = true;
        LOG_IF(INFO, Log_Switch) << "logger | msg subsribed on channel " << *topicStr << " at " << simTimeStamp / 1000
                                 << "\n";
      } else {
        LOG_EVERY_N(ERROR, 100) << "logger | no msg on : " << *topicStr << "\n";
      }
    }
  }
}
}  // namespace pblog
