// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txsim_log_player.h"

#include <iomanip>

#include "scene.pb.h"

#define SPLIT_LINE "======================================="

using namespace pblog;

txSimLogplayer::txSimLogplayer() {
  this->name = "protobuf log player";
  LOG(INFO) << "Hi, my name is " << this->name << "." << std::endl;
}

void txSimLogplayer::ClearPbLogQueuePtr() {
  if (_pblog_queue_ptr != nullptr) {
    _pblog_queue_ptr->Clear(PBLogQueueDir::PBLogQueue_Read);
    delete _pblog_queue_ptr;
    _pblog_queue_ptr = nullptr;
  }
}

txSimLogplayer::~txSimLogplayer() { LOG(INFO) << this->name << " destroyed." << std::endl; }

void txSimLogplayer::Init(tx_sim::InitHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "initializing protobuf log replay module ...\n";

  // 输入参数1: "log-folder"，日志存放的文件夹
  auto argIt = helper.GetParameter("log-file");
  if (!argIt.empty()) {
    _log_file_path = argIt;
  }

  // open log file for reading some information
  _pblog_queue_ptr = new CPBLogQueue(256);
  _pblog_queue_ptr->Initialize(_log_file_path, PBLogQueueDir::PBLogQueue_Read);
  // wait for log file to be consumed
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  ReadEgoGroupInfo();
  ReadTopicList();

  // 输入参数2: "channels"，代表发布的topic
  argIt = helper.GetParameter("channels");
  _playback_channel_list.clear();
  if (!argIt.empty()) {
    std::string strChannels = argIt;
    std::vector<std::string> strChannels_vec;
    LOG(INFO) << "player | argIt strChannels: " << strChannels << "\n";
    // split playback channel
    CPBLogCommon::StringSplit(strChannels, "||", strChannels_vec);
    for (const std::string &topic : strChannels_vec) {
      PreProcessTopic(topic, _playback_channel_list);
    }
  } else {
    // playback all channels when argIt is empty, playback channel list set to empty
    for (const auto &topic : _channel_list_in_file) {
      PreProcessTopic(topic, _playback_channel_list);
    }
    LOG(INFO) << "player | playback all channels.\n";
  }

  // topic names that will be replayed
  for (const std::string &topic : _playback_channel_list) {
    helper.Publish(topic);
    LOG(INFO) << "player | playback channels: " << topic << "\n";
  }
}

void txSimLogplayer::ReadEgoGroupInfo() {
  PBLogEvent event = _pblog_queue_ptr->GetOnePBLogEvent();
  _pblog_queue_ptr->ConsumeOnePBLogEvent();
  LOG(INFO) << "player | event.m_channelStringSize: " << event.m_channelStringSize
            << ", event.m_channel: " << event.m_channel << "\n";
  if (event.m_channelStringSize == 0 || event.m_channel != "Scenario_Info") {
    LOG(ERROR) << "log file is not valid. can't find Scenario_Info in first event. log file: " << _log_file_path
               << std::endl;
    throw std::runtime_error("log file is not valid. can't find Scenario_Info in first event.");
  }
  sim_msg::Scene scene;
  scene.ParsePartialFromString(event.m_payload);
  _ego_group_list.clear();
  for (size_t i = 0; i < scene.egos_size(); ++i) {
    _ego_group_list.push_back(scene.egos(i).group());
    LOG(INFO) << "player | ego name[" << i << "]: " << scene.egos(i).group() << "\n";
  }
}

void txSimLogplayer::ReadTopicList() {
  PBLogEvent event = _pblog_queue_ptr->GetOnePBLogEvent();
  _pblog_queue_ptr->ConsumeOnePBLogEvent();
  LOG(INFO) << "player | event.m_channelStringSize: " << event.m_channelStringSize
            << ", event.m_channel: " << event.m_channel << "\n";
  if (event.m_channelStringSize == 0 || event.m_channel != "Topic_List") {
    LOG(ERROR) << "log file is not valid. can't find Topic_List in second event. log file: " << _log_file_path
               << std::endl;
    throw std::runtime_error("log file is not valid. can't find Topic_List in second event.");
  }
  _channel_list_in_file.clear();
  CPBLogCommon::StringSplit(event.m_payload, "||", _channel_list_in_file);
}

void txSimLogplayer::Reset(tx_sim::ResetHelper &helper) {
  // here we could get some information(e.g. the map which the scenario runs on,
  // suggested local origin coordinate and the initial state of the ego car) of
  // the current scenario.
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "player | " << this->name << " reset.\n";

  // reset pb log queue
  ClearPbLogQueuePtr();
  _pblog_queue_ptr = new CPBLogQueue(256);
  _pblog_queue_ptr->Initialize(_log_file_path, PBLogQueueDir::PBLogQueue_Read);

  _first_frame = true;
  _first_sim_t = 0;
  _first_log_t = 0;
}

void txSimLogplayer::Stop(tx_sim::StopHelper &helper) {
  LOG(INFO) << SPLIT_LINE << "\n";

  _pblog_queue_ptr->Clear(PBLogQueueDir::PBLogQueue_Read);

  LOG(INFO) << this->name << " module stop" << "\n";
}

bool txSimLogplayer::IsPlaybackChannel(const std::string &topic) {
  // when no specific topics in list
  // playback all channels
  if (_playback_channel_list.size() == 0) {
    return true;
  }

  // playback specified channel
  for (const auto &topic_ : _playback_channel_list) {
    if (topic_ == topic) return true;
  }
  return false;
}

void txSimLogplayer::Step(tx_sim::StepHelper &helper) {
  // 1. get current simulation timestamp.
  int64_t simTimeStamp = CPBLogCommon::millisecondToMicrosecond(helper.timestamp());
  LOG(INFO) << SPLIT_LINE << "\n";
  LOG(INFO) << "player | simulation time stamp: " << helper.timestamp() << "\n";

  int32_t _queueEmptyCounter = 0;
  bool quitThisStep = false;

  while (!quitThisStep) {
    // read one event from queue
    PBLogEvent event = _pblog_queue_ptr->GetOnePBLogEvent();

    if (event.m_channelStringSize == 0) {
      // situation when no PBLogEvent read from file
      _queueEmptyCounter++;
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      quitThisStep = _queueEmptyCounter >= 4 ? true : false;

      // situation when to quit playback
      if (quitThisStep) {
        std::string reason = "wait log player queue to be filled, log player may reach the end.";
        LOG(INFO) << "player | " << reason << "\n";
        helper.StopScenario(reason);
        return;
      }
    } else {
      // situation when PBLogEvent read from file
      if (_first_frame) {
        _first_log_t = event.m_timestamp;
        _first_sim_t = simTimeStamp;
        _first_frame = false;
      }

      int64_t logRelativeTimestamp = event.m_timestamp - _first_log_t;
      int64_t simRelativeTimestamp = simTimeStamp - _first_sim_t;

      if (logRelativeTimestamp <= simRelativeTimestamp) {
        // consume on pb log event
        _pblog_queue_ptr->ConsumeOnePBLogEvent();
        // publish protobuf msg
        if (IsPlaybackChannel(event.m_channel)) {
          helper.PublishMessage(event.m_channel, event.m_payload);
          LOG(INFO) << "player | " << event.m_channel << " in protobuf log relative timestamp is "
                    << CPBLogCommon::microsecondToMillisecond(logRelativeTimestamp) << ", msg published on channel "
                    << event.m_channel << " at " << CPBLogCommon::microsecondToMillisecond(simTimeStamp)
                    << ", payload size:" << event.m_payload.size() << ".\n";
        }
      } else {
        // terminiate current step
        quitThisStep = true;
      }
    }
  }
}

void txSimLogplayer::PreProcessTopic(const std::string &original_topic, std::vector<std::string> &all_topics) {
  std::string topic_name = original_topic;
  // 如果在内置的主车topic列表中，即使不带tx_sim::topic::kUnionFlag，也手动添加
  if (kEgoTopicList.find(original_topic) != kEgoTopicList.end()) {
    topic_name = tx_sim::topic::kUnionFlag + original_topic;
  }

  // 前缀不是tx_sim::topic::kUnionFlag，加入后直接返回
  if (topic_name.compare(0, tx_sim::topic::kUnionFlag.size(), tx_sim::topic::kUnionFlag) != 0) {
    all_topics.push_back(topic_name);
    return;
  }
  // 前缀是tx_sim::topic::kUnionFlag，需要解析
  // 再加入解析后的topic
  std::string topic_name_suffix = topic_name.substr(tx_sim::topic::kUnionFlag.size());
  for (const std::string &ego_group : _ego_group_list) {
    all_topics.push_back(ego_group + "/" + topic_name_suffix);
  }
  return;
}

// must including this macro definition to access to simulation system
TXSIM_MODULE(txSimLogplayer)
