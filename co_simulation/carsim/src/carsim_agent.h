// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <stdio.h>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "control.pb.h"
#include "location.pb.h"
#include "vehState.pb.h"

#include "carsim_core.h"
#include "coord_trans.h"
#include "softecu/inc/softecu.h"

#include "glog/logging.h"

#if (defined(_WIN32) || defined(_WIN64))
#  include <windows.h>

#  ifdef __cplusplus
extern "C" {
#  endif  // __cplusplus
#  include "carsim/win/vs_deftypes.h"
#  ifdef __cplusplus
}
#  endif  // __cplusplus

#else
#  include <stdlib.h>

#  ifdef __cplusplus
extern "C" {
#  endif  // __cplusplus
#  include "carsim/linux/vs_deftypes.h"
#  ifdef __cplusplus
}
#  endif  // __cplusplus

#endif

namespace tx_carsim {
// carsim
class CarSimState {
 private:
  tx_carsim::SCarsimState mState;

 public:
  CarSimState();
  void setCarsimState(const tx_carsim::SCarsimState& stateIN);
  const tx_carsim::SCarsimState& getCarsimState();
  bool isCarsimOk();
};

class CarSimAgent {
 public:
  CarSimAgent();
  virtual ~CarSimAgent();

  void initCarSim(const tx_carsim::SCarsimInit& init);
  void updateCarSim(double t_ms);
  void stopCarSim();

  void applyControlCmd(sim_msg::Control& msg);
  void setLocation(sim_msg::Location& msg);
  void setVehicleState(sim_msg::VehicleState& msg);

  void getMessageByTopic(const std::string& topic, std::string& payload);
  void setMessageByTopic(const std::string& topic, const std::string& payload);

  CarSimState getCarSimState() const { return mState; }

 protected:
  void releaseCarsim();

 public:
  // external functions to extend carsim math model
  static void external_calc(vs_real t, vs_ext_loc where);
  static void external_setdef(void);
  static void external_echo(vs_ext_loc where);

  // external functions to extend carsim math model
  static void external_calc_win(vs_real t, vs_ext_loc where);
  static void external_setdef_win(void);

 private:
  static CarSimAgent* sAgentPtr;
  static tx_carsim::SCarsimIn sInput;
  static tx_carsim::SCarsimOut sOutput;
  static tx_carsim::SCarsimInit sParam;
  static sim_msg::Location sStartLoc;

 private:
  std::string mName;
  std::string mPayload;
  std::map<std::string, std::string> mMsgs;

  // carsim state
  CarSimState mState;

  // pb msgs
  sim_msg::Control _ctrlMsg;
  sim_msg::Location _locMsg;
  sim_msg::VehicleState _vehStateMsg;

  // carsim dll path
  char mDLLPath[MAX_FILENAME_LENGTH];

  // softecu
  softecuModelClass mSoftECU;

#if (defined(_WIN32) || defined(_WIN64))
  HMODULE _vsDLL = nullptr;
#else
  void* _vsDLL = nullptr;
#endif
};
}  // namespace tx_carsim
