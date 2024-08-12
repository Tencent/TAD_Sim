// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "carsim_interface.h"

namespace tx_carsim {
CarSimInterface::CarSimInterface() {
  mName = "carsim/trucksim interface";

  VLOG(1) << "tx_carsim | module name is " << mName << "\n";
}

CarSimInterface::~CarSimInterface() { VLOG(1) << "tx_carsim | " << mName << " destroyed.\n"; }
void CarSimInterface::Init(tx_sim::InitHelper& helper) {
  VLOG(1) << SPLIT_LINE;

  // pub/sub our interested topics
  helper.Subscribe(tx_sim::topic::kControl);
  helper.Publish(tx_sim::topic::kLocation);
  helper.Publish(tx_sim::topic::kVehicleState);

  // get init velocity
  std::string simFile = helper.GetParameter("sim-file");
  if (!simFile.empty()) {
    mParam.carsimSimFile = simFile;
    VLOG(1) << "tx_sim | carsim sim file is " << simFile << "\n";
  }

  // set log level
  FLAGS_v = 2;
  std::string logLevel = helper.GetParameter("_log_level");
  if (logLevel.size() > 0) {
    FLAGS_v = std::atoi(logLevel.c_str());
    VLOG(0) << "log level is " << FLAGS_v << "\n";
  }
}

void CarSimInterface::Reset(tx_sim::ResetHelper& helper) {
  VLOG(1) << SPLIT_LINE;

  // get start location
  mParam.startLocationPayload = helper.ego_start_location();

  // make carsim agent
  mAgent = std::make_shared<CarSimAgent>();

  if (mAgent.get() == nullptr) {
    LOG(ERROR) << "fail to make carsim agent.\n";
    return;
  }

  // init carsim
  mAgent->initCarSim(mParam);

  if (!mAgent->getCarSimState().isCarsimOk()) {
    VLOG(0) << "carsim is not working properly.\n";
  }
}

void CarSimInterface::Step(tx_sim::StepHelper& helper) {
  VLOG(1) << SPLIT_LINE;

  if (mAgent.get() == nullptr) {
    LOG(ERROR) << "carsim agent is nullptr.\n";
    return;
  }

  if (!mAgent->getCarSimState().isCarsimOk()) {
    LOG(ERROR) << "carsim agent reports carsim error.\n";
  }

  // 1. get control command
  helper.GetSubscribedMessage(tx_sim::topic::kControl, mPayload);
  mAgent->setMessageByTopic(tx_sim::topic::kControl, mPayload);

  // 2. update carsim
  mAgent->updateCarSim(helper.timestamp());

  // 3. publish location
  mAgent->getMessageByTopic(tx_sim::topic::kLocation, mPayload);
  helper.PublishMessage(tx_sim::topic::kLocation, mPayload);

  // 4. publish vehicle state
  mAgent->getMessageByTopic(tx_sim::topic::kVehicleState, mPayload);
  helper.PublishMessage(tx_sim::topic::kVehicleState, mPayload);
}

void CarSimInterface::Stop(tx_sim::StopHelper& helper) {
  VLOG(1) << SPLIT_LINE;
  mAgent.reset();
}
}  // namespace tx_carsim

TXSIM_MODULE(tx_carsim::CarSimInterface)
