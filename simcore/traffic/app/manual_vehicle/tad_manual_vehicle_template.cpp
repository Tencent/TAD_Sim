// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_manual_vehicle_template.h"
#include "tad_manual_vehicle_system.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"

#define LogInfo LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
#define LogWarn LOG(WARNING)

TX_NAMESPACE_OPEN(TrafficFlow)

void TAD_ManualVehicleSimLoop::Init(tx_sim::InitHelper& helper) TX_NOEXCEPT {
  ParentClass::Init(helper);
  helper.Subscribe(FLAGS_ManualVehicleTopicName);
  LOG(INFO) << " Subscribe Topics : " << FLAGS_ManualVehicleTopicName;
}

void TAD_ManualVehicleSimLoop::Step(tx_sim::StepHelper& helper) TX_NOEXCEPT {
  using TimeParamManager = Base::TimeParamManager;
  strManualVehicle_.clear();
  if (NonNull_Pointer(TrafficSystemPtr()) && CallSucc(TrafficSystemPtr()->IsAlive())) {
    const auto timeStamp = helper.timestamp();
    Base::txFloat const relativeTime = Utils::MillisecondToSecond(timeStamp - m_lastStepTime);
  }

  using TimeParamManager = Base::TimeParamManager;
  strManualVehicle_.clear();
  if (NonNull_Pointer(TrafficSystemPtr()) && CallSucc(TrafficSystemPtr()->IsAlive())) {
    const auto time_stamp = helper.timestamp();

    Base::txFloat const relativeTime = Utils::MillisecondToSecond(time_stamp - m_lastStepTime);
    const Base::TimeParamManager timeMgr(Utils::MillisecondToSecond(time_stamp), relativeTime,
                                         m_passTime + relativeTime, time_stamp);

    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle) << "[ManualVehicle_simulation_time] : " << timeMgr.str();

    GetSubscribedMessage(helper, FLAGS_ManualVehicleTopicName, strManualVehicle_);
    /*helper.GetSubscribedMessage(FLAGS_ManualVehicleTopicName, strManualVehicle_);*/
    /*if (m_passTime < 5.0) {
        helper.GetSubscribedMessage(tx_sim::topic::kLocation, strManualVehicle_);
        sim_msg::Location kl;
        kl.ParseFromString(strManualVehicle_);
        sim_msg::ManualVehicleControlInfoArray tmp;
        ::sim_msg::ManualVehicleControlInfo*  ptr = tmp.add_vehicles_control_info();
        ptr->set_id(1);
        (*(ptr->mutable_veh_control_info())) = kl;
        strManualVehicle_.clear();
        tmp.SerializeToString(&strManualVehicle_);
    } else {
        LOG(WARNING) << " swicth manual to auto.";
    }*/

    LOG_IF(INFO, FLAGS_LogLevel_ManualVehicle)
        << " GetSubscribedMessage, " << TX_VARS_NAME(Topic, FLAGS_ManualVehicleTopicName)
        << TX_VARS(strManualVehicle_.size());

    if (NonNull_Pointer(ManualTrafficSystemPtr())) {
      ManualTrafficSystemPtr()->UpdateManualVehicleData(timeMgr, strManualVehicle_);
    }
  }

  ParentClass::Step(helper);
}

void TAD_ManualVehicleSimLoop::CreateSystem() TX_NOEXCEPT {
  m_tad_manual_vehicle_system_ptr = std::make_shared<TAD_ManualVehicleSystem>();
  m_TrafficElementSystem_Ptr = m_tad_manual_vehicle_system_ptr;
  if (NonNull_Pointer(m_tad_manual_vehicle_system_ptr)) {
    LogInfo << "Create TrafficElementSystem Success.";
  } else {
    LogWarn << "Create TrafficElementSystem Failure.";
  }
}

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef LogInfo
#undef LogWarn
