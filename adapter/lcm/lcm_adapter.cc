// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "lcm_adapter.h"

#include "glog/logging.h"

void LCMAdapter::Init(tx_sim::InitHelper& helper) {
  sub_topics_.clear();
  pub_topics_.clear();
  const std::string sub_topic_param = helper.GetParameter("subscription");
  if (sub_topic_param.empty()) {
    LOG(WARNING) << "No subscription specified. No message will publish to LCM system." << std::endl;
  } else {
    ParseTopicParams(sub_topic_param, sub_topics_);
  }
  const std::string pub_topic_param = helper.GetParameter("publication");
  if (pub_topic_param.empty()) {
    LOG(WARNING) << "No publication specified. No message will publish to TADSim system." << std::endl;
  } else {
    ParseTopicParams(pub_topic_param, pub_topics_);
  }

  for (const std::string& sub : sub_topics_) {
    helper.Subscribe(sub);
  }
  for (const std::string& pub : pub_topics_) {
    helper.Publish(pub);
  }

  lcm_url_ = helper.GetParameter("lcm_url");
}

void LCMAdapter::Reset(tx_sim::ResetHelper& helper) {
  if (lcm_) {
    lcm_.reset(nullptr);
  }

  lcm_.reset(new lcm::LCM(lcm_url_));
  if (!lcm_->good()) {
    throw std::runtime_error("lcm initialization with url \"" + lcm_url_ + "\" failed.");
  }

  for (const std::string& pub : pub_topics_) {
    lcm_->subscribe(pub, &LCMAdapter::HandleMessageFromLCM, this);
  }

  destination_ = helper.ego_destination();
  // TODO(nemo): publish the destination info.
}

void LCMAdapter::Step(tx_sim::StepHelper& helper) {
  LOG(INFO) << "=================================" << std::endl;
  for (const auto& sub : sub_topics_) {
    helper.GetSubscribedMessage(sub, payload_);
    lcm_->publish(sub, payload_.c_str(), payload_.size());
  }

  for (size_t count = 0; count < pub_topics_.size(); count++) {
    LOG(INFO) << "lcm handing..." << std::endl;
    if (lcm_->handle() != 0) {
      throw std::runtime_error("lcm handle() error.");
    }
    helper.PublishMessage(received_topic_, payload_);
  }
}

void LCMAdapter::Stop(tx_sim::StopHelper& helper) { lcm_.reset(nullptr); }

void LCMAdapter::HandleMessageFromLCM(const lcm::ReceiveBuffer* rbuf, const std::string& channel) {
  payload_.assign(reinterpret_cast<char*>(rbuf->data), rbuf->data_size);
  received_topic_ = channel;
  LOG(INFO) << "received topic " << channel << " with message of " << rbuf->data_size << " bytes." << std::endl;
}

void LCMAdapter::ParseTopicParams(const std::string& param, std::vector<std::string>& topics) {
  size_t pos = param.find_first_of(';'), start = 0;
  while (pos != std::string::npos) {
    if (pos > start) {
      topics.push_back(param.substr(start, pos - start));
    }
    start = pos + 1;
    pos = param.find_first_of(';', start);
  }
  if (start < param.size()) {
    topics.push_back(param.substr(start));
  }
}

extern "C" {
tx_sim::SimModule* NewSimModule() { return new LCMAdapter(); }
}
