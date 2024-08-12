// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "RearEMotorExt.h"
#include "../../inc/car_log.h"
#include "car.pb.h"
#include "inc/proto_helper.h"

namespace tx_car {
namespace power {
tx_car::Motor rear_motor;

RearEMotorExt::RearEMotorExt(/* args */) {}

RearEMotorExt::~RearEMotorExt() {}

void RearEMotorExt::initialize() {
  initMDL();
  EMotor::initialize();
}

/* model step function */

void RearEMotorExt::step() { EMotor::step(); }

/* model terminate function */
void RearEMotorExt::terminate() { EMotor::terminate(); }

bool RearEMotorExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init front motor parameter.\n";
  if (rear_motor.ByteSizeLong()) {
    EMotor_P.mot_tc_s = rear_motor.mot_tc_s().val();  // 电机时间常数
    int maxTrqSpdNum = rear_motor.mot_max_tor_map().u0_axis().data_size();
    int maxTrqNum = rear_motor.mot_max_tor_map().y0_axis().data_size();
    // LOG_0<<"maxTrqSpdNum: "<<maxTrqSpdNum<<"maxTrqNum: "<<maxTrqNum<<"\n";
    if (maxTrqNum != maxTrqSpdNum || maxTrqNum == 0 || maxTrqSpdNum == 0) {
      if (errorLog) {
        char failed[] = "motor max trq num is 0; or motor max trq num is not equal spd num!";
        memcpy(errorLog, failed, strlen(failed));
        errorLog[strlen(failed)] = '\0';
      }
      return false;
    }
    setMotTrqMapNum(maxTrqSpdNum);
    for (size_t i = 0; i < maxTrqSpdNum; i++) {
      EMotor_P.MaxTorqueLimit_bp01Data[i] = rear_motor.mot_max_tor_map().u0_axis().data(i);
      EMotor_P.MaxTorqueLimit_tableData[i] = rear_motor.mot_max_tor_map().y0_axis().data(i);
    }

    int lossTableTrqNum = rear_motor.mot_loss_map_spd_tor().u1_axis().data_size();
    int lossTableSpdNum = rear_motor.mot_loss_map_spd_tor().u0_axis().data_size();
    int lossTablePowNum = rear_motor.mot_loss_map_spd_tor().y0_axis().data_size();

    if (lossTablePowNum != lossTableTrqNum * lossTableSpdNum) {
      if (errorLog) {
        char failed[] = "motor loss table num is 0; or motor eff num is not equal motor trq * motor spd!";
        memcpy(errorLog, failed, strlen(failed));
        errorLog[strlen(failed)] = '\0';
      }
      return false;
    }

    EMotor_P.uDLookupTable_maxIndex[0] = lossTableSpdNum - 1;
    EMotor_P.uDLookupTable_maxIndex[1] = lossTableTrqNum - 1;
    setMotLossMapSpdNum(lossTableSpdNum);

    for (size_t i = 0; i < lossTableSpdNum; i++) {
      EMotor_P.uDLookupTable_bp01Data[i] = rear_motor.mot_loss_map_spd_tor().u0_axis().data(i);
    }

    for (size_t i = 0; i < lossTableTrqNum; i++) {
      EMotor_P.uDLookupTable_bp02Data[i] = rear_motor.mot_loss_map_spd_tor().u1_axis().data(i);
    }

    for (size_t i = 0; i < lossTablePowNum; i++) {
      EMotor_P.uDLookupTable_tableData[i] = rear_motor.mot_loss_map_spd_tor().y0_axis().data(i);
    }
  } else {
    if (errorLog) {
      char failed[] = "parsing json to pb file failed!\n";
      memcpy(errorLog, failed, strlen(failed));
      errorLog[strlen(failed)] = '\0';
    }
    return false;
  }
  LOG_2 << "end of init front motor parameter.\n";
  return true;
}

bool RearEMotorExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        rear_motor.CopyFrom(car_param.propulsion().rear_motor_parameter());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool RearEMotorExt::initParam() {
  char log[1024];
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing motor parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool RearEMotorExt::initMDL() {
#ifdef LOCAL_TEST_MODE
  const std::string par_path = "E:\\work\\VehicleDynamics\\param\\car_chassis.json";
#else
  const std::string par_path = tx_car::car_config::getParPath();
#endif  // LOCAL_TEST_MODE

  if (loadParam(par_path)) {
    return initParam();
  }
  return false;
}

void RearEMotorExt::setMotTrqMapNum(int num) { m_mot_trq_map_num = num; }

void RearEMotorExt::setMotLossMapSpdNum(int num) { m_mot_loss_map_spd_num = num; }
}  // namespace power
}  // namespace tx_car
