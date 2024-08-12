// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "SoftECUExt.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

namespace tx_car {
namespace ecu {
tx_car::Pow_ECU ecu;
tx_car::CarType cartype;
tx_car::WheelTireDD wheel_tire;

SoftECUExt::SoftECUExt(/* args */) {}

SoftECUExt::~SoftECUExt() {}

void SoftECUExt::initialize() {
  initMDL();  //
  SoftECU::initialize();
}

void SoftECUExt::step() {
  SoftECU::step();
  // LOG_0<<"ecu out "<<getExternalOutputs().ecu_out.pt_trq_cmd_Nm<<"\n";
}

void SoftECUExt::terminate() { SoftECU::terminate(); }

bool SoftECUExt::parsingParameterFromJson(char *errorLog) {
  LOG_2 << "init softecu parameter.\n";

  // auto hold vx speed threshold
  SoftECU_P.speedOffset = 0.01;

  // brake enable acc threshold
  SoftECU_P.brakeEnableThresh_Reverse_const = 0.1;
  SoftECU_P.brakeEnableThresh_const = -0.1;

  // average motor torque coeff from max motor torque, used to estimate power
  SoftECU_P.averageTorqueCoeff_Value = 0.4;

  if (ecu.ByteSizeLong()) {
    auto proptype = cartype.propulsion_type();
    switch (proptype) {
      case tx_car::CarType_Propulsion_Type_ICE_Engine:
        SoftECU_P.DriveLinePowType = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
      case tx_car::CarType_Propulsion_Type_Electric_Drive:
        SoftECU_P.DriveLinePowType = 2;  // DriveLinePowType 1-ICE 2-MOT
        break;
      default:
        SoftECU_P.DriveLinePowType = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
    }
    LOG_2 << "DriveLinePowType is " << SoftECU_P.DriveLinePowType << "\n";

    // max acc
    SoftECU_P.ecu_max_acc = ecu.ecu_max_acc().val();
    SoftECU_P.ecu_max_dec = ecu.ecu_max_dec().val() >= 0.0 ? -10 : ecu.ecu_max_dec().val();

    // engine
    const tx_car::Pow_ECU_SoftEMS &ems = ecu.soft_ems();
    SoftECU_P.EngMaxTrq = ems.engmaxtrq().val();
    SoftECU_P.eng_acc_P = ems.eng_acc_p().val();
    SoftECU_P.eng_acc_I = ems.eng_acc_i().val();
    SoftECU_P.eng_acc_D = ems.eng_acc_d().val();

    // vcu
    const tx_car::Pow_ECU_SoftVCU &vcu = ecu.soft_vcu();
    SoftECU_P.MotorMaxTrq = vcu.motormaxtrq().val();
    SoftECU_P.vcu_acc_P = vcu.vcu_acc_p().val();
    SoftECU_P.vcu_acc_I = vcu.vcu_acc_i().val();
    SoftECU_P.vcu_acc_D = vcu.vcu_acc_d().val();
    SoftECU_P.MotorPowerRegenerationRatio = vcu.vcu_power_regeneration_ratio().val();
    SoftECU_P.MotorPowerRegenerationRatio = std::min(SoftECU_P.MotorPowerRegenerationRatio, 1.0);
    SoftECU_P.MotorPowerRegenerationRatio = std::max(SoftECU_P.MotorPowerRegenerationRatio, 0.1);
    SoftECU_P.MotorPowerRegenerationRatio = 0.0;

    // abs
    const tx_car::Pow_ECU_SoftABS &abs = ecu.soft_abs();
    SoftECU_P.enable_abs = abs.abs_enable().val() > 0.5;
    SoftECU_P.slip_ratio_ref = abs.slip_ratio_ref().val();
    SoftECU_P.slip_disable_ratio_ref = std::max(abs.slip_ratio_disable_ref().val(), 0.1);
    SoftECU_P.abs_brake_pressure = abs.abs_brake_pressure().val();

    /*
    SoftECU_P.max_main_cylinder_pressure =
        wheel_tire.brake().max_main_cylinder_pressure().val();
    SoftECU_P.rear_brake_ratio =
        wheel_tire.brake().rear_brk_pressure_ratio().val();
    SoftECU_P.brake_const_time_fluid =
        wheel_tire.brake().const_fluid_time_front().val();
    SoftECU_P.brake_const_time_fluid = std::abs(SoftECU_P.brake_const_time_fluid) < 1e-4 ? 0.0 :
    SoftECU_P.brake_const_time_fluid; SoftECU_P.brake_fluid_transport_const_time =
        wheel_tire.brake().const_fluid_transport_time_front().val();
    */
    SoftECU_P.brake_P = abs.brake_p().val();
    SoftECU_P.brake_I = abs.brake_i().val();
    SoftECU_P.brake_D = abs.brake_d().val();

    SoftECU_P.speedOffset = 0.01;  // 0.2
  } else {
    if (errorLog) {
      char failed[] = "parsing json to pb file for driveline parameters failed!\n";
      memcpy(errorLog, failed, strlen(failed));
      errorLog[strlen(failed)] = '\0';
    }
    return false;
  }
  LOG_2 << "parsing ecu parameter from json file sucessfully\n";
  return true;
}

bool SoftECUExt::loadParam(const std::string &par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_0 << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        ecu.CopyFrom(car_param.ecu());
        cartype.CopyFrom(car_param.car_type());
        wheel_tire.CopyFrom(car_param.wheel_tire());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool SoftECUExt::initParam() {
  char log[1024]{'\0'};
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing power ecu parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool SoftECUExt::initMDL() {
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
}  // namespace ecu
}  // namespace tx_car