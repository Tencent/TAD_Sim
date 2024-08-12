// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_simulation_loop.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "tx_parallel_def.h"
#include "tx_protobuf_utils.h"
TX_NAMESPACE_OPEN(Base)

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const txString& strPB) TX_NOEXCEPT {
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::Traffic& sendTrafficInfo) TX_NOEXCEPT {
  // 将sendTrafficInfo序列化为字符串
  static txString strPB;
  sendTrafficInfo.SerializeToString(&strPB);
  // 使用helper发布消息
  helper.PublishMessage(strTopics, strPB);

  // 如果启用了日志记录
  if (FLAGS_LogLevel_Traffic_PB_Output) {
    // 创建一个并行向量，用于存储日志字符串
    tbb::concurrent_vector<Base::txString> conVecLogStr;
    // 遍历sendTrafficInfo中的汽车信息，并将其转换为日志字符串
    tbb::parallel_for_each(sendTrafficInfo.cars().begin(), sendTrafficInfo.cars().end(), [&](auto& element) {
      conVecLogStr.emplace_back(Utils::ProtobufDebugLogString(std::ref(element)));
    }); /*lamda function*/
        /* parallel_for */

    // 遍历sendTrafficInfo中的动态障碍物信息，并将其转换为日志字符串
    tbb::parallel_for_each(sendTrafficInfo.dynamicobstacles().begin(), sendTrafficInfo.dynamicobstacles().end(),
                           [&](auto& element) {
                             conVecLogStr.emplace_back(Utils::ProtobufDebugLogString(std::ref(element)));
                           }); /*lamda function*/
                               /* parallel_for */

    // 遍历sendTrafficInfo中的静态障碍物信息，并将其转换为日志字符串
    tbb::parallel_for_each(sendTrafficInfo.staticobstacles().begin(), sendTrafficInfo.staticobstacles().end(),
                           [&](auto& element) {
                             conVecLogStr.emplace_back(Utils::ProtobufDebugLogString(std::ref(element)));
                           }); /*lamda function*/
                               /* parallel_for */

    // 遍历sendTrafficInfo中的交通灯信息，并将其转换为日志字符串
    tbb::parallel_for_each(sendTrafficInfo.trafficlights().begin(), sendTrafficInfo.trafficlights().end(),
                           [&](auto& element) {
                             conVecLogStr.emplace_back(Utils::ProtobufDebugLogString(std::ref(element)));
                           }); /*lamda function*/
                               /* parallel_for */

    for (Base::txInt idx = 0; idx < conVecLogStr.size(); ++idx) {
      LOG(INFO) << "[Traffic_PB_Output] " << TX_VARS(helper.timestamp()) << conVecLogStr[idx];
    }
  }
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::Location& sendLocation) TX_NOEXCEPT {
  static txString strPB;
  sendLocation.SerializeToString(&strPB);
  // 使用helper发布消息
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::Trajectory& sendTrajectory) TX_NOEXCEPT {
  static txString strPB;
  sendTrajectory.SerializeToString(&strPB);
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::VehicleInteraction& sendVehicleInteraction) TX_NOEXCEPT {
  static txString strPB;
  sendVehicleInteraction.SerializeToString(&strPB);
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::Control& sendControl) TX_NOEXCEPT {
  static txString strPB;
  sendControl.SerializeToString(&strPB);
  // 使用helper发布消息
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::PublishMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                          const sim_msg::Control_V2& sendControl_v2) TX_NOEXCEPT {
  static txString strPB;
  sendControl_v2.SerializeToString(&strPB);
  helper.PublishMessage(strTopics, strPB);
}

void txSimulationTemplate::GetSubscribedMessage(tx_sim::StepHelper& helper, const txString& strTopics,
                                                txString& strPB) TX_NOEXCEPT {
  helper.GetSubscribedMessage(strTopics, strPB);
}

TX_NAMESPACE_CLOSE(Base)
