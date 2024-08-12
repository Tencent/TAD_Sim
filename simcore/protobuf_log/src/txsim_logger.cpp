// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txsim_logger.h"
#include <stdio.h>
#include <iomanip>

#include "settings.pb.h"
#include "union.pb.h"

#define SPLIT_LINE "======================================="

namespace pblog {
txSimLogger::txSimLogger() {
  _post_script = "";
  _data_logged = false;
  _sync_process = 0;
  LOG(INFO) << "Hi, my name is " << this->name << "." << std::endl;
}

txSimLogger::~txSimLogger() {
  // 如果未记录任何内容，删除空日志
  DeleteEmptyLog(_data_logged);
  LOG(INFO) << this->name << " destroyed." << std::endl;
}

bool txSimLogger::IsInSubTopic(const std::string &topicIn) {
  // 循环遍历 _topic_list 中的所有字符串
  for (auto iterStr = _topic_list.begin(); iterStr != _topic_list.end(); ++iterStr) {
    // 如果 _topic_list 中的当前字符串与 topicIn 相等，返回true，即有匹配的topic
    if (topicIn == *iterStr) {
      return true;
    }
  }

  return false;
}

void txSimLogger::Init(tx_sim::InitHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "initializing protobuf logger module ...\n";

  // default pblog folder
  // 若无"log-folder"输入，则默认设置为：
  {
#ifdef _WIN32
    _log_file_path = std::string(getenv("HOMEDRIVE")) + std::string(getenv("HOMEPATH")) +
                     "\\AppData\\Roaming\\tadsim\\data\\service_data\\sim_data\\pblog";
#else
    _log_file_path = std::string(getenv("HOME")) + "/.config/tadsim/data/service_data/sim_data/pblog";
#endif
  }

  // 输入参数1: "log-folder"，日志存放的文件夹
  auto argIt = helper.GetParameter("log-folder");
  if (!argIt.empty()) {
    _log_file_path = argIt;
  }

  // 输入参数2: "sync"，表示后处理时是否等待同步
  std::string sync_str = helper.GetParameter("sync");
  if (!sync_str.empty()) {
    _sync_process = std::atoi(sync_str.c_str());
  }

  // get all topic key and name, sub topic
  // 定义并初始化为以下的topic列表，若无输入，则以这些为准
  std::string topic_list = "TRAFFIC||TRAJECTORY||CONTROL||CONTROL_V2||LOCATION||VEHICLE_STATE";

  // 输入参数3: "channels"，代表自定义订阅的topic
  argIt = helper.GetParameter("channels");
  if (!argIt.empty()) topic_list = argIt;
  pblog::CPBLogCommon::StringSplit(topic_list, "||", _topic_list);

  std::string channelsLog = "logger | ";
  for (std::string &topic : _topic_list) {
    // 如果在内置的主车topic列表中，即使不带tx_sim::topic::kUnionFlag，也手动添加
    if (kEgoTopicList.find(topic) != kEgoTopicList.end()) {
      topic = tx_sim::topic::kUnionFlag + topic;
    }
    channelsLog = channelsLog + topic + " ";
    helper.Subscribe(topic);
  }
  LOG(INFO) << channelsLog << "\n";

  // pblog file path
  helper.Publish(TOPIC_PBLOG_FILE_PATH);

// get post process python script
// 默认为展示当前目录文件的命令，代表无后处理脚本
#ifdef _WIN32
  _post_script = "dir";
#else
  _post_script = "ls";
#endif

  // 输入参数4: "post-script"，后处理的程序位置
  argIt = helper.GetParameter("post-script");
  if (!argIt.empty()) {
    _post_script = argIt;
  }

  _pbLog_queue_ptr = nullptr;
}

void txSimLogger::Reset(tx_sim::ResetHelper &helper) {
  // here we could get some information(e.g. the map which the scenario runs on,
  // suggested local origin coordinate and the initial state of the ego car) of
  // the current scenario.
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "logger | " << this->name << " reset.\n";

  // 生成log的文件
  pblog::CPBLogCommon::GenPBLogFileName(_log_file_name);
  // 获取场景名字
  std::string scene_name = pblog::CPBLogCommon::GetSceneName(helper.scenario_file_path());

  _log_file_name = scene_name + "." + _log_file_name;

#ifdef _WIN32
  _log_file_name = _log_file_path + "\\" + _log_file_name;
#else
  _log_file_name = _log_file_path + "/" + _log_file_name;
#endif

  LOG(INFO) << "logger | pb log file path is " << _log_file_name << "\n";

  // reset pb log queue object
  ClearPbLogQueuePtr();
  _pbLog_queue_ptr = new CPBLogQueue(1024);
  _pbLog_queue_ptr->Initialize(_log_file_name, PBLogQueueDir::PBLogQueue_Write);

  // reset _data_logged
  _data_logged = false;

  // save scene info
  { ProducePBLogEvent("Scenario_Info", helper.scene_pb(), 0); }

  // save topic list
  {
    std::string topic_list;
    for (const std::string &topic : _topic_list) {
      topic_list += topic + "||";
    }
    ProducePBLogEvent("Topic_List", topic_list, 0);
  }

  // save setting info
  {
    sim_msg::Setting setting_info;
    setting_info.set_scenario_path(helper.scenario_file_path());
    setting_info.set_hadmap_path(helper.map_file_path());
    std::string payload;
    setting_info.SerializeToString(&payload);
    ProducePBLogEvent("Setting_Info", payload, 0);
  }

  // save ego geometry
  {
    std::string ego_geometry_payload;
    std::vector<std::pair<int64_t, std::string>> veh_geometry;
    helper.vehicle_measurements(veh_geometry);
    if (veh_geometry.size() > 0) ego_geometry_payload = veh_geometry.at(0).second;
    ProducePBLogEvent("VEHICLE_GEOMETRY", ego_geometry_payload, 0);
  }
}

void txSimLogger::ClearPbLogQueuePtr() {
  if (_pbLog_queue_ptr != nullptr) {
    _pbLog_queue_ptr->Clear(PBLogQueueDir::PBLogQueue_Write);
    delete _pbLog_queue_ptr;
    _pbLog_queue_ptr = nullptr;
  }
}

void txSimLogger::Stop(tx_sim::StopHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";

  ClearPbLogQueuePtr();

  // remove empty log file
  DeleteEmptyLog(_data_logged);

  // post process
  // 如果已记录数据，则启动一个新线程来执行后期处理函数
  if (_data_logged) {
    // 创建一个新线程，并使用PostProcessFunc函数处理后期处理任务
    std::thread pp = std::thread(&txSimLogger::PostProcessFunc, this, _post_script, _log_file_name);

    // 如果不是同步处理，则等待4秒后detach
    if (_sync_process == 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(4000));
      pp.detach();
      LOG(INFO) << "thread deteched.\n";
    } else {
      // 否则是同步处理，则join
      if (pp.joinable()) pp.join();
      LOG(INFO) << "post-proess thread quit.\n";
    }
  }

  LOG(INFO) << "logger | " << this->name << " module stop" << "\n";
}

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

bool txSimLogger::ProducePBLogEvent(const std::string &topic, const std::string &payload_in, const int &t_ms) {
  // if(topic.size()>0 && payload_in.size() > 0){
  if (topic.size() > 0) {
    pblog::PBLogEvent event;

    // 将传入的topic、payload和时间戳打包成event
    pblog::CPBLogCommon::PB2Event(event, payload_in, topic, CPBLogCommon::millisecondToMicrosecond(t_ms));
    // 将生成的事件添加到队列自动处理
    _pbLog_queue_ptr->ProduceLogEvent(event);

    return true;
  }

  return false;
}

void txSimLogger::Step(tx_sim::StepHelper &helper) {
  // 1. get current simulation timestamp.
  int64_t simTimeStamp = CPBLogCommon::millisecondToMicrosecond(helper.timestamp());
  LOG_IF(INFO, Log_Switch) << SPLIT_LINE << "\n";
  LOG_IF(INFO, Log_Switch) << "logger | time stamp: " << helper.timestamp() << "\n";

  // publish pblog file path
  helper.PublishMessage(TOPIC_PBLOG_FILE_PATH, _log_file_name);

  // save all topic into protobuf log
  for (const auto &topic : _topic_list) {
    if (this->IsInSubTopic(topic)) {
      this->payload = "";

      helper.GetSubscribedMessage(topic, this->payload);
      std::vector<std::pair<std::string, std::string>> topic_payload_out;
      PreProcessTopicData(topic, this->payload, topic_payload_out);
      for (const auto &payload_pair : topic_payload_out) {
        if (this->ProducePBLogEvent(payload_pair.first, payload_pair.second, helper.timestamp())) {
          // 有数据了，将_data_logged置位
          _data_logged = true;
          LOG_IF(INFO, Log_Switch) << "logger | msg subsribed on channel " << payload_pair.first << " at "
                                   << CPBLogCommon::microsecondToMillisecond(simTimeStamp)
                                   << " ms. topic->payload.size(): " << payload_pair.second.size() << "\n";
        } else {
          LOG_EVERY_N(ERROR, 100) << "logger | no msg on : " << topic << "\n";
        }
      }
    }
  }
}

void txSimLogger::PreProcessTopicData(const std::string &topic_in, const std::string &payload_in,
                                      std::vector<std::pair<std::string, std::string>> &topic_payload_out) {
  // 前缀不是tx_sim::topic::kUnionFlag，直接加入并返回
  if (topic_in.compare(0, tx_sim::topic::kUnionFlag.size(), tx_sim::topic::kUnionFlag) != 0) {
    topic_payload_out.push_back(std::make_pair(topic_in, payload_in));
    return;
  }

  // 前缀是tx_sim::topic::kUnionFlag，需要解析
  // 先加入Union topic，且payload设为空，仅用于记录Union的topic
  topic_payload_out.push_back(std::make_pair(topic_in, ""));
  // 再加入解析后的topic
  std::string topic_name_suffix = topic_in.substr(tx_sim::topic::kUnionFlag.size());
  sim_msg::Union msg_union;
  msg_union.ParsePartialFromString(payload_in);
  for (size_t i = 0; i < msg_union.messages_size(); ++i) {
    std::string group_name = msg_union.messages(i).groupname();
    std::string msg_content = msg_union.messages(i).content();
    topic_payload_out.push_back(std::make_pair(group_name + "/" + topic_name_suffix, msg_content));
  }
  return;
}
}  // namespace pblog
