// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "carsim_agent.h"
#include "eigen3/Eigen/Dense"

#if (defined(_WIN32) || defined(_WIN64))
#  include <windows.h>

#  ifdef __cplusplus
extern "C" {
#  endif  // __cplusplus
#  include "carsim/win/vs_api.h"
#  ifdef __cplusplus
}
#  endif  // __cplusplus

#else
#  include <stdio.h>
#  include <stdlib.h>

#  ifdef __cplusplus
extern "C" {
#  endif  // __cplusplus
#  include "carsim/linux/vs_api.h"
#  ifdef __cplusplus
}
#  endif  // __cplusplus

#endif

namespace tx_carsim {
// carsim agent pointer
CarSimAgent* CarSimAgent::sAgentPtr = nullptr;

// carsim input/output
tx_carsim::SCarsimIn CarSimAgent::sInput;
tx_carsim::SCarsimInit CarSimAgent::sParam;
tx_carsim::SCarsimOut CarSimAgent::sOutput;

// start location
sim_msg::Location CarSimAgent::sStartLoc;

/// <summary>
/// carsim state
/// </summary>
CarSimState::CarSimState() { mState = tx_carsim::SCarsimState::COK; }
void CarSimState::setCarsimState(const tx_carsim::SCarsimState& stateIN) { mState = stateIN; }
const tx_carsim::SCarsimState& CarSimState::getCarsimState() { return mState; }
bool CarSimState::isCarsimOk() { return mState == tx_carsim::SCarsimState::COK ? true : false; }

/// <summary>
/// carsim agent
/// </summary>
CarSimAgent::CarSimAgent() {
  mName = "TADSim co-simulation with CarSim/TruckSim";
  CarSimAgent::sAgentPtr = this;
  mDLLPath[0] = {'\0'};

  VLOG(1) << "tx_carsim | module name is " << mName << "\n";
}
CarSimAgent::~CarSimAgent() {
  CarSimAgent::sAgentPtr = nullptr;

  releaseCarsim();
  VLOG(1) << "tx_carsim | " << mName << " destroyed.\n";
}

void CarSimAgent::initCarSim(const tx_carsim::SCarsimInit& init) {
  VLOG(1) << SPLIT_LINE;

  // clear message map
  mMsgs.clear();

  // init parameter
  sParam = init;

  memset(&sInput, 0x00, sizeof(SCarsimIn));
  memset(&sOutput, 0x00, sizeof(SCarsimOut));

  // get start location
  sStartLoc.ParseFromString(sParam.startLocationPayload);

  // update map origin
  sParam.mapOrigin.x = sStartLoc.position().x();
  sParam.mapOrigin.y = sStartLoc.position().y();
  sParam.mapOrigin.z = sStartLoc.position().z();

  // print map origin
  VLOG(1) << "tx_carsim | original long: " << sParam.mapOrigin.x << ", lat: " << sParam.mapOrigin.y
          << ", alt: " << sParam.mapOrigin.z << "\n";

  // init time
  sOutput.t = 0.0;
  // set longtitude, latitude, altitude
  sParam.x = sStartLoc.position().x();
  sParam.y = sStartLoc.position().y();
  sParam.z = sStartLoc.position().z();
  VLOG(1) << "tx_carsim | start location long: " << sParam.x << ", lat: " << sParam.y << ", alt: " << sParam.z << "\n";

  // convert coordinate from gps to local
  coord_trans_api::lonlat2enu(sParam.x, sParam.y, sParam.z, sParam.mapOrigin.x, sParam.mapOrigin.y, sParam.mapOrigin.z);
  VLOG(1) << "tx_carsim | start location x: " << sParam.x << ", y: " << sParam.y << ", z: " << sParam.z << "\n";

  // set yaw of vehicle
  sParam.yaw = (sStartLoc.rpy().z());

  // set velocity of vehicle
  auto v = std::hypot(sStartLoc.velocity().x(), sStartLoc.velocity().y());

  sParam.vx = (v);
  sParam.vy = 0.0;
  sParam.vz = 0.0;

  VLOG(1) << "tx_carsim | init velocity is " << sParam.vx << " m/s\n";
  VLOG(1) << "tx_carsim | init yaw is " << sParam.yaw << " rad\n";

  // unload carsim solver
  releaseCarsim();

  // get dll from simfile
  if (vs_get_dll_path(sParam.carsimSimFile.c_str(), mDLLPath) != 0) {
    VLOG(0) << "fail to get carsim dll path with carsim .sim file:" << sParam.carsimSimFile << "\n";
    mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
    return;
  }

  // load vehicle solver dll
#if (defined(_WIN32) || defined(_WIN64))
  LPCSTR strptr = mDLLPath;
  _vsDLL = LoadLibrary(strptr);
#else
  _vsDLL = vs_load_library(mDLLPath);
#endif
  VLOG(1) << "tx_carsim | get dll of carsim from sim file done. " << std::string(mDLLPath) << ", pointer is " << _vsDLL
          << "\n";

  // check if carsim dll loaded
  if (_vsDLL == nullptr) {
    mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
    VLOG(1) << "tx_carsim | fail to load carsim dll " << std::string(mDLLPath) << "\n";
    return;
  }
  VLOG(1) << "tx_carsim | load carsim dll done. " << std::string(mDLLPath) << "\n";

  // check if get api from carsim dll done
  if (vs_get_api(_vsDLL, mDLLPath) != 0) {
    mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
    VLOG(1) << "tx_carsim | fail to get api from carsim dll: " << mDLLPath << "\n";
    return;
  }
  VLOG(1) << "tx_carsim | get api from carsim dll done.\n";

  // set input and output variable
#if (defined(_WIN32) || defined(_WIN64))
  vs_install_setdef_function(CarSimAgent::external_setdef);
  vs_install_calc_function(CarSimAgent::external_calc);
#else
  vs_install_setdef_function(CarSimAgent::external_setdef);
  vs_install_calc_function(CarSimAgent::external_calc);
#endif
  VLOG(1) << "tx_carsim | install external functions done.\n";

  // load data from .sim file
  double t = vs_setdef_and_read(sParam.carsimSimFile.c_str(), NULL, NULL);
  if (vs_error_occurred()) {
    mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
    VLOG(1) << "tx_carsim | fail to load data from sim file " << sParam.carsimSimFile << "\n";
    VLOG(1) << "tx_carsim | carsim error msg is:" << vs_get_error_message() << "\n";
    return;
  }

  VLOG(1) << "tx_carsim | load data from sim file done." << sParam.carsimSimFile << "\n";
#ifdef UNIX
  VLOG(1) << "tx_carsim | carsim database dir is " << vs_get_database_dir() << "\n";
#endif
  // initialize carsim
  vs_initialize(t, NULL, NULL);

  if (vs_error_occurred()) {
    mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
    VLOG(1) << "tx_carsim | fail to initialize carsim.\n";
    VLOG(1) << "tx_carsim | carsim error msg is:" << vs_get_error_message() << "\n";
    return;
  }

  mState.setCarsimState(tx_carsim::SCarsimState::COK);
  VLOG(1) << "tx_carsim | initialize carsim run model done.\n";

  mSoftECU.initialize();
}
void CarSimAgent::updateCarSim(double t_ms) {
  VLOG(1) << SPLIT_LINE;
  /*
  if (vs_stop_run()) {
      VLOG(1) << "tx_carsim | carsim has stopped running. error msg is:" << vs_get_error_message() << "\n";
      mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
      return;
  }
  */
  // 1. get control command
  getMessageByTopic(topic::CONTROL, mPayload);
  if (mPayload.size() > 0) {
    _ctrlMsg.ParseFromString(mPayload);
    applyControlCmd(_ctrlMsg);
  }

  VLOG(1) << "tx_carsim | carsim time is " << sOutput.t << "\n";
  // VLOG(1)<< "tx_carsim | throttle : " << *throttle << ", steer : " << *steer << "\n";

  for (auto i = 0; i < 10; ++i) {
    // check if carsim is ok
    if (!vs_error_occurred()) {
      vs_integrate(&sOutput.t, nullptr);
    } else {
      mState.setCarsimState(tx_carsim::SCarsimState::CERROR);
      VLOG(1) << "tx_carsim | error msg, " << vs_get_error_message() << "\n";
      return;
    }
  }

  // 2. cal location
  _locMsg.set_t(t_ms * 0.001);
  setLocation(_locMsg);
  _locMsg.SerializeToString(&mPayload);
  setMessageByTopic(topic::LOCATION, mPayload);

  // 3. cal vehicle state
  _vehStateMsg.set_time_stamp(t_ms * 0.001);
  setVehicleState(_vehStateMsg);
  _vehStateMsg.SerializeToString(&mPayload);
  setMessageByTopic(topic::VEHICLE_STATE, mPayload);
}
void CarSimAgent::stopCarSim() {
  VLOG(1) << SPLIT_LINE;
  sAgentPtr = nullptr;
  releaseCarsim();
  mSoftECU.terminate();
}

void CarSimAgent::getMessageByTopic(const std::string& topic, std::string& payload) {
  auto iter = mMsgs.find(topic);
  if (iter != mMsgs.end()) {
    payload = iter->second;
    return;
  }
  payload = std::string("");
}
void CarSimAgent::setMessageByTopic(const std::string& topic, const std::string& payload) { mMsgs[topic] = payload; }

// external functions to extend carsim math model
void CarSimAgent::external_calc_win(vs_real t, vs_ext_loc where) {
  switch (where) {
    case VS_EXT_AFTER_READ: {  // after having read the Parsfile inputs
      // define output var
      {
        vs_define_out(To_Char_Ptr("EXP_XCG"), To_Char_Ptr(""), &sOutput.xo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
        vs_define_out(To_Char_Ptr("EXP_YCG"), To_Char_Ptr(""), &sOutput.yo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
        vs_define_out(To_Char_Ptr("EXP_ZCG"), To_Char_Ptr(""), &sOutput.zo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
      }
      // get output of carsim
      {
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_XCG=XCG_TM"), 1);
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_YCG=YCG_TM"), 1);
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_ZCG=ZCG_TM"), 1);
      }
      break;
    }
    case VS_EXT_EQ_IN: {  // calculations at the start of a time step
      break;
    }
    case VS_EXT_EQ_OUT: {  // calculate output variables at the end of a time step
      break;
    }
      /* unused locations for this example:
      VS_EXT_EQ_PRE_INIT, VS_EXT_EQ_INIT, VS_EXT_EQ_INIT2,
      VS_EXT_EQ_SAVE, VS_EXT_EQ_FULL_STEP, VS_EXT_EQ_END */
  }
}
void CarSimAgent::external_setdef_win(void) {
  vs_real zero = 0.0;

  // define units
  {
    vs_define_units(To_Char_Ptr("N-M"), 1.0);
    vs_define_units(To_Char_Ptr("ENUM"), 1.0);
    vs_define_units(To_Char_Ptr("%"), 1.0);
  }
  // define intialize vehicle state
  {
    vs_define_imp(To_Char_Ptr("IMP_XO_INIT"), &sParam.x, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_YO_INIT"), &sParam.y, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_ZO_INIT"), &sParam.z, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_YAW_INIT"), &sParam.yaw, To_Char_Ptr("RAD"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VX_INIT"), &sParam.vx, To_Char_Ptr("KM/H"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VY_INIT"), &sParam.vy, To_Char_Ptr("KM/H"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VZ_INIT"), &sParam.vz, To_Char_Ptr("KM/H"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
  }
}

// initialze carsim
void CarSimAgent::external_setdef(void) {
  vs_real zero = 0.0;

  // define units
  {
    vs_define_units(To_Char_Ptr("N-M"), 1.0);
    vs_define_units(To_Char_Ptr("ENUM"), 1.0);
    vs_define_units(To_Char_Ptr("%"), 1.0);
    vs_define_units(To_Char_Ptr("RATIO"), 1.0);
  }
  // define intialize vehicle state
  {
    vs_define_imp(To_Char_Ptr("IMP_XO_INIT"), &sParam.x, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_YO_INIT"), &sParam.y, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_ZO_INIT"), &sParam.z, To_Char_Ptr("M"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_YAW_INIT"), &sParam.yaw, To_Char_Ptr("RAD"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VX_INIT"), &sParam.vx, To_Char_Ptr("M/S"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VY_INIT"), &sParam.vy, To_Char_Ptr("M/S"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
    vs_define_imp(To_Char_Ptr("IMP_VZ_INIT"), &sParam.vz, To_Char_Ptr("M/S"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
  }
}

void CarSimAgent::external_calc(vs_real t, vs_ext_loc where) {
  switch (where) {
    case VS_EXT_EQ_INIT: {
      break;
    }
    case VS_EXT_AFTER_READ: {  // after having read the Parsfile inputs
      // define input var
      //  active import variables
      {
        // replace carsim internal database with import value
        vs_statement("IMPORT", "IMP_STEER_SW vs_replace", 1);

        // acceleration, unit [0,1]
        vs_define_imp(To_Char_Ptr("IMP_THROTTLE_ENGINE_CONSTANT"), nullptr, To_Char_Ptr("RATIO"), 0, To_Char_Ptr(""),
                      To_Char_Ptr(""));
        vs_statement("IMPORT", "IMP_THROTTLE_ENGINE_CONSTANT vs_replace", 1);

        // brake, unit Pa
        vs_define_imp(To_Char_Ptr("IMP_PBK_CON_CONSTANT"), nullptr, To_Char_Ptr("RATIO"), 1, To_Char_Ptr(""),
                      To_Char_Ptr(""));
        vs_statement("IMPORT", "IMP_PBK_CON_CONSTANT vs_replace", 1);

        vs_define_imp(To_Char_Ptr("IMP_GEAR_OL"), nullptr, To_Char_Ptr("ENUM"), 0, To_Char_Ptr(""), To_Char_Ptr(""));
        vs_statement("IMPORT", "IMP_GEAR_OL vs_replace", 1);
      }
      // define output var
      {
        vs_define_out(To_Char_Ptr("EXP_XCG"), To_Char_Ptr(""), &sOutput.xo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
        vs_define_out(To_Char_Ptr("EXP_YCG"), To_Char_Ptr(""), &sOutput.yo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
        vs_define_out(To_Char_Ptr("EXP_ZCG"), To_Char_Ptr(""), &sOutput.zo, To_Char_Ptr("M"), To_Char_Ptr(""),
                      To_Char_Ptr(""), To_Char_Ptr(""));
        vs_define_out(To_Char_Ptr("EXP_STR"), To_Char_Ptr(""), &sOutput.steerWheelAngle, To_Char_Ptr("RAD"),
                      To_Char_Ptr(""), To_Char_Ptr(""), To_Char_Ptr(""));
      }
      // set input
      {
        // vs_statement(To_Char_Ptr("EQ_IN"), To_Char_Ptr("THROTTLE=IMP_THROTTLE"), 1);
      }
      // get output of carsim
      {
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_XCG=XCG_TM"), 1);
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_YCG=YCG_TM"), 1);
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_ZCG=ZCG_TM"), 1);
        vs_statement(To_Char_Ptr("EQ_OUT"), To_Char_Ptr("EXP_STR=STEER_SW"), 1);
      }
      // set initial state of carim
      {
        vs_statement(To_Char_Ptr("EQ_INIT"), To_Char_Ptr("SV_XO=IMP_XO_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_INIT"), To_Char_Ptr("SV_YO=IMP_YO_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_PRE_INIT"), To_Char_Ptr("SV_ZO=IMP_ZO_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_INIT"), To_Char_Ptr("SV_YAW=IMP_YAW_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_PRE_INIT"), To_Char_Ptr("SV_VXS=IMP_VX_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_PRE_INIT"), To_Char_Ptr("SV_VYS=IMP_VY_INIT"), 1);
        vs_statement(To_Char_Ptr("EQ_PRE_INIT"), To_Char_Ptr("SV_VZS=IMP_VZ_INIT"), 1);
      }
      break;
    }
    case VS_EXT_EQ_IN: {  // calculations at the start of a time step
      // vs_real* gear = vs_get_var_ptr("GEAR_OL");
      // *gear = sInput.driveMode;

      // I do not why, but this is the only way that works
      vs_real* throttle = vs_get_var_ptr("IMP_THROTTLE_ENGINE_CONSTANT");
      *throttle = sInput.throttle;
      throttle = vs_get_var_ptr("THROTTLE_ENGINE_CONSTANT");
      *throttle = sInput.throttle;

      // I do not why, but this is the only way that works
      vs_real* brake = vs_get_var_ptr("IMP_PBK_CON_CONSTANT");
      *brake = sParam.maxCylinderBrakePressure * sInput.brake * 1e6;
      brake = vs_get_var_ptr("PBK_CON_CONSTANT");
      *brake = sParam.maxCylinderBrakePressure * sInput.brake * 1e6;
      // vs_real* pbkGain = vs_get_var_ptr("PBK_CON_GAIN");
      // VLOG(1) << "PBK_CON_GAIN:" << *pbkGain << ", sParam.maxCylinderBrakePressure:" <<
      // sParam.maxCylinderBrakePressure << "\n";

      vs_real* steer = vs_get_var_ptr("IMP_STEER_SW");
      *steer = sInput.steer;

      // VLOG(1) << "throttle:" << sInput.throttle << ", brake:" << *brake << ", sInput.brake:"<< sInput.brake << ",
      // steer:" << *steer << "\n";

      break;
    }
    case VS_EXT_EQ_OUT: {  // calculate output variables at the end of a time step
      break;
    }
      /* unused locations for this example:
      VS_EXT_EQ_PRE_INIT, VS_EXT_EQ_INIT, VS_EXT_EQ_INIT2,
      VS_EXT_EQ_SAVE, VS_EXT_EQ_FULL_STEP, VS_EXT_EQ_END */
  }
}
void CarSimAgent::external_echo(vs_ext_loc where) { return; }
void CarSimAgent::releaseCarsim() {
  if (_vsDLL != nullptr) {
    vs_terminate(sOutput.t, CarSimAgent::external_echo);
#if (defined(_WIN32) || defined(_WIN64))
    FreeLibrary(_vsDLL);
#else
    vs_free_library(_vsDLL);
#endif
    _vsDLL = nullptr;
  }
}

void CarSimAgent::applyControlCmd(sim_msg::Control& msg) {
#ifdef _OPEN_LOOP_
  // set target acc and gear mode
  _s_carsimIn.tarAcc = 0.0;
  // set default gear mode
  _s_carsimIn.driveMode = IMP_DriveMode::R_TORQUE;

  // set steer request
  _s_carsimIn.steer = RAD_2_DEG(0.0);
#else
  if (msg.contrl_type() == sim_msg::Control::CONTROL_TYPE::Control_CONTROL_TYPE_ACC_CONTROL) {
    // use accel to control vehicle. @dhu

    // input to soft acc
    vs_real* steerL1 = vs_get_var_ptr("STEER_L1");
    vs_real* steerR1 = vs_get_var_ptr("STEER_R1");
    mSoftECU.softecu_U.cur_front_wheel_anglerad = (*steerL1 + *steerR1) / 2.0;
    mSoftECU.softecu_U.cur_accms2 = sOutput.ax;
    mSoftECU.softecu_U.vxms = sOutput.vx;

    // set input of simulink model
    mSoftECU.softecu_U.target_accms2 = msg.acc_cmd().acc();
    mSoftECU.softecu_U.gear_lever_from_controller = msg.gear_cmd();
    mSoftECU.softecu_U.target_front_wheel_anglerad = msg.acc_cmd().front_wheel_angle();

    if (mSoftECU.softecu_U.gear_lever_from_controller == sim_msg::Control::GEAR_MODE::Control_GEAR_MODE_REVERSE) {
      mSoftECU.softecu_U.target_accms2 = -mSoftECU.softecu_U.target_accms2;
    }

    mSoftECU.step();

    if (mSoftECU.softecu_U.gear_lever_from_controller == sim_msg::Control::GEAR_MODE::Control_GEAR_MODE_DRIVE &&
        mSoftECU.softecu_U.target_accms2 < -3.0) {
      mSoftECU.softecu_Y.brakepedal = 100.0;
    }

    // override steer
    // mSoftECU.softecu_Y.steerdeg = RAD_2_DEG(msg.acc_cmd().front_wheel_angle()) * sParam.steer_ratio;
    mSoftECU.softecu_Y.steerdeg = RAD_2_DEG(msg.acc_cmd().front_wheel_angle()) * sParam.steer_ratio;

    VLOG(2) << "vd | use acc control.\n";
  } else {
    LOG(ERROR) << "only support acc control now.\n";
  }

  // manual control, brake pedal set to 100%
  if (msg.control_mode() == sim_msg::Control::CONTROL_MODE::Control_CONTROL_MODE_CM_MANUAL) {
    mSoftECU.softecu_Y.accpedal = 0.0;
    if (abs(sOutput.vx) <= 0.5) {
      mSoftECU.softecu_Y.brakepedal = 60.0;
      mSoftECU.softecu_Y.steerdeg = 0.0;
      VLOG(2) << "vd | vehicle is in manual control mode.\n";
    }
  }

  VLOG(2) << "vd | accelPedal: " << mSoftECU.softecu_Y.accpedal << ", brakePedal: " << mSoftECU.softecu_Y.brakepedal
          << ", steeringWheelAngle[deg]: " << mSoftECU.softecu_Y.steerdeg << "\n";
  VLOG(2) << "vd | gear lever: " << mSoftECU.softecu_Y.gear_lever_out1reverse0netural0drive
          << ", gear lever from controller: " << mSoftECU.softecu_U.gear_lever_from_controller << "\n";
  VLOG(2) << "vd | currentAcc: " << mSoftECU.softecu_U.cur_accms2 << ", targetAcc: " << mSoftECU.softecu_U.target_accms2
          << ", curfrontWheelAngle[deg]: " << RAD_2_DEG(mSoftECU.softecu_U.cur_front_wheel_anglerad)
          << ", targetFrontWheelAngle[deg]: " << RAD_2_DEG(msg.acc_cmd().front_wheel_angle()) << "\n";

  // set target acc and gear mode
  sInput.throttle = mSoftECU.softecu_Y.accpedal * 0.01;

  // brake
  sInput.brake = mSoftECU.softecu_Y.brakepedal * 0.01;

  // set default gear mode
  sInput.driveMode = mSoftECU.softecu_U.gear_lever_from_controller;

  // set steer request
  sInput.steer = DEG_2_RAD(mSoftECU.softecu_Y.steerdeg);
#endif
}
void CarSimAgent::setLocation(sim_msg::Location& locMsg) {
  // update position, use marco to detemine if using z info, @dhu
  sim_msg::Vec3* ptrLoc = locMsg.mutable_position();
  double xo = sOutput.xo, yo = sOutput.yo, zo = sOutput.zo;
  VLOG(2) << "tx_carsim | location ENU x: " << xo << ", y: " << yo << ", z: " << zo << "\n";

  // convert coordinate from local to gps
  coord_trans_api::enu2lonlat(xo, yo, zo, sParam.mapOrigin.x, sParam.mapOrigin.y, sParam.mapOrigin.z);
  VLOG(2) << "tx_carsim | location GPS x: " << xo << ", y: " << yo << ", z: " << zo << "\n";

  ptrLoc->set_x(xo);
  ptrLoc->set_y(yo);
  ptrLoc->set_z(zo);

  // update row pitch yaw
  sim_msg::Vec3* ptrRPY = locMsg.mutable_rpy();

  // roll, pitch, yaw
  vs_real* yaw_ptr = vs_get_var_ptr("YAW");
  vs_real* roll_ptr = vs_get_var_ptr("ROLL");
  vs_real* pitch_ptr = vs_get_var_ptr("PITCH");
  ptrRPY->set_x(*roll_ptr);
  ptrRPY->set_y(*pitch_ptr);
  ptrRPY->set_z(*yaw_ptr);

  // update velocity, use marco to detemine if using z info, @dhu
  sim_msg::Vec3* ptrVel = locMsg.mutable_velocity();
  vs_real* vx = vs_get_var_ptr("VX");
  vs_real* vy = vs_get_var_ptr("VY");
  vs_real* vz = vs_get_var_ptr("VZ");
  Eigen::AngleAxisd Rot_Yaw(*yaw_ptr, Eigen::Vector3d::UnitZ());
  Eigen::Vector3d velocityENU = Rot_Yaw.matrix() * Eigen::Vector3d(*vx, *vy, *vz);
  ptrVel->set_x(velocityENU[0]);
  ptrVel->set_y(velocityENU[1]);
  ptrVel->set_z(velocityENU[2]);

  // update angular speed(rad/s)
  sim_msg::Vec3* ptrAngular = locMsg.mutable_angular();
  vs_real* avx_ptr = vs_get_var_ptr("AVX");
  vs_real* avy_ptr = vs_get_var_ptr("AVY");
  vs_real* yaw_rate_ptr = vs_get_var_ptr("AVZ");
  ptrAngular->set_x((*avx_ptr));
  ptrAngular->set_y((*avy_ptr));
  ptrAngular->set_z((*yaw_rate_ptr));

  sim_msg::Vec3* ptrAccel = locMsg.mutable_acceleration();
  vs_real* ax = vs_get_var_ptr("AX");
  vs_real* ay = vs_get_var_ptr("AY");
  vs_real* az = vs_get_var_ptr("AZ");
  ptrAccel->set_x(*ax);
  ptrAccel->set_y(*ay);
  ptrAccel->set_z(*az);

  VLOG(2) << "tx_carsim | location vx is " << locMsg.velocity().x() << " m/s, vy is " << locMsg.velocity().y()
          << " m/s, yaw_rate is " << locMsg.angular().z() << " rad/s\n";
  VLOG(2) << "tx_carsim | location ax is " << locMsg.acceleration().x() << " m/s2, ay is " << locMsg.acceleration().y()
          << " m/s2\n";
  VLOG(2) << "tx_carsim | location roll is " << locMsg.rpy().x() << " rad, pitch is " << locMsg.rpy().y()
          << " rad, yaw is " << locMsg.rpy().z() << " rad.\n";
  VLOG(2) << "tx_carsim | avx is " << locMsg.angular().x() << " rad/s, avy is " << locMsg.angular().y()
          << " rad/s, avz is " << locMsg.angular().z() << " rad/s.\n";
}
void CarSimAgent::setVehicleState(sim_msg::VehicleState& vehState) {
  // set wheel speed
  /*
  auto wheel_speed = vehState.mutable_chassis_state()->mutable_wheel_speed();
  vs_real* omega_L1 = vs_get_var_ptr("AVx_L1");
  vs_real* omega_L2 = vs_get_var_ptr("AVx_L2");
  vs_real* omega_R1 = vs_get_var_ptr("AVx_R1");
  vs_real* omega_R2 = vs_get_var_ptr("AVx_R2");

  wheel_speed->set_frontleft(DEG_2_RAD(*omega_L1) * sParam.wheel_radius);
  wheel_speed->set_frontright(DEG_2_RAD(*omega_L2) * sParam.wheel_radius);
  wheel_speed->set_rearleft(DEG_2_RAD(*omega_R1)* sParam.wheel_radius);
  wheel_speed->set_rearright(DEG_2_RAD(*omega_R2) * sParam.wheel_radius);
  */

  // set current gear
  vehState.mutable_powertrain_state()->set_gear_engaged(1);

  // set engine speed
  vehState.mutable_powertrain_state()->set_engine_speed(0.0);

  // set gear
  vehState.mutable_powertrain_state()->set_gead_mode(sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_NEUTRAL);
  if (mSoftECU.softecu_Y.gear_lever_out1reverse0netural0drive < 0.0) {
    vehState.mutable_powertrain_state()->set_gead_mode(
        sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_REVERSE);
  } else if (mSoftECU.softecu_Y.gear_lever_out1reverse0netural0drive > 0.5) {
    vehState.mutable_powertrain_state()->set_gead_mode(sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_DRIVE);
  }

  // set throttle and brake pedal, @dhu
  vehState.mutable_powertrain_state()->set_accpedal_position(mSoftECU.softecu_Y.accpedal);
  vehState.mutable_chassis_state()->set_brakepedalpos(mSoftECU.softecu_Y.brakepedal);

  // set steer wheel angle, in haval h7, chassis feedback steerwheelangle is front wheel angle. @dhu
  vs_real* steerL1 = vs_get_var_ptr("STEER_L1");
  vs_real* steerR1 = vs_get_var_ptr("STEER_R1");
  vehState.mutable_chassis_state()->set_steeringwheelangle((*steerL1 + *steerR1) / 2.0);
  vehState.mutable_chassis_state()->set_steeringwheelanglesign(RAD_2_DEG(sOutput.steerWheelAngle));

  // set esp master cylinder brake pressure, @dhu
  vehState.mutable_chassis_state()->set_esp_mastercylindbrakepress(mSoftECU.softecu_Y.brakepedal * 0.01 * 10e6);

  // set yaw_rate
  vs_real* yaw_rate_ptr = vs_get_var_ptr("AVZ");
  vehState.mutable_chassis_state()->set_vehdynyawrate(*yaw_rate_ptr);
}
}  // namespace tx_carsim
