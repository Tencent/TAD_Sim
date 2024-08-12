// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "SoftABSExt.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

namespace tx_car {
namespace abs {
tx_car::Pow_ECU ecu;
tx_car::CarType cartype;
tx_car::WheelTireDD wheel_tire;

SoftABSExt::SoftABSExt(/* args */) {}

SoftABSExt::~SoftABSExt() {}

void SoftABSExt::initialize() {
  initMDL();  //
  SoftABS::initialize();
}

void SoftABSExt::step() {
  SoftABS::step();
  // LOG_0<<"ecu out "<<getExternalOutputs().ecu_out.pt_trq_cmd_Nm<<"\n";
}

void SoftABSExt::terminate() { SoftABS::terminate(); }

bool SoftABSExt::parsingParameterFromJson(char *errorLog) {
  LOG_2 << "init SoftABS parameter.\n";

  // auto hold vx speed threshold
  SoftABS_P.speedOffset = 0.01;

  // brake enable acc threshold
  SoftABS_P.brakeEnableThresh_Reverse_const = 0.15;
  SoftABS_P.brakeEnableThresh_const = -0.15;

  if (ecu.ByteSizeLong()) {
    auto proptype = cartype.propulsion_type();
    switch (proptype) {
      case tx_car::CarType_Propulsion_Type_ICE_Engine:
        SoftABS_P.DriveLinePowType = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
      case tx_car::CarType_Propulsion_Type_Electric_Drive:
        SoftABS_P.DriveLinePowType = 2;  // DriveLinePowType 1-ICE 2-MOT
        break;
      default:
        SoftABS_P.DriveLinePowType = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
    }
    LOG_2 << "DriveLinePowType is " << SoftABS_P.DriveLinePowType << "\n";

    // max acc
    SoftABS_P.ecu_max_dec = ecu.ecu_max_dec().val() >= 0.0 ? -10 : ecu.ecu_max_dec().val();

    // abs
    const tx_car::Pow_ECU_SoftABS &abs = ecu.soft_abs();
    SoftABS_P.enable_abs = abs.abs_enable().val() > 0.5;
    SoftABS_P.slip_ratio_ref = abs.slip_ratio_ref().val();
    SoftABS_P.slip_disable_ratio_ref = std::max(abs.slip_ratio_disable_ref().val(), 0.1);
    SoftABS_P.abs_brake_pressure = abs.abs_brake_pressure().val();

    SoftABS_P.brake_P = abs.brake_p().val();
    SoftABS_P.brake_I = abs.brake_i().val();
    SoftABS_P.brake_D = abs.brake_d().val();

    SoftABS_P.speedOffset = 0.01;  // 0.2
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

bool SoftABSExt::loadParam(const std::string &par_path) {
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
bool SoftABSExt::initParam() {
  char log[1024]{'\0'};
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing power ecu parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool SoftABSExt::initMDL() {
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
}  // namespace abs
}  // namespace tx_car
