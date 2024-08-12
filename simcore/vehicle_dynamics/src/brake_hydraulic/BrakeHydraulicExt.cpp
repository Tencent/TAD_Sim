// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "BrakeHydraulicExt.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

namespace tx_car {
namespace brake_hydraulic {
tx_car::WheelTireDD wheel_tire;

BrakeHydraulicExt::BrakeHydraulicExt(/* args */) {}

BrakeHydraulicExt::~BrakeHydraulicExt() {}

void BrakeHydraulicExt::initialize() {
  initMDL();  //
  BrakeHydraulic::initialize();
}

void BrakeHydraulicExt::step() {
  BrakeHydraulic::step();
  // LOG_0<<"ecu out "<<getExternalOutputs().ecu_out.pt_trq_cmd_Nm<<"\n";
}

void BrakeHydraulicExt::terminate() { BrakeHydraulic::terminate(); }

bool BrakeHydraulicExt::parsingParameterFromJson(char *errorLog) {
  LOG_2 << "init softecu parameter.\n";

  if (wheel_tire.ByteSizeLong() > 0) {
    BrakeHydraulic_P.max_main_cylinder_pressure = wheel_tire.brake().max_main_cylinder_pressure().val();
    BrakeHydraulic_P.rear_brake_ratio = wheel_tire.brake().rear_brk_pressure_ratio().val();
    BrakeHydraulic_P.brake_const_time_fluid = wheel_tire.brake().const_fluid_time_front().val();
    BrakeHydraulic_P.brake_const_time_fluid =
        std::abs(BrakeHydraulic_P.brake_const_time_fluid) < 1e-4 ? 0.0 : BrakeHydraulic_P.brake_const_time_fluid;
    BrakeHydraulic_P.brake_fluid_transport_const_time = wheel_tire.brake().const_fluid_transport_time_front().val();
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

bool BrakeHydraulicExt::loadParam(const std::string &par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        wheel_tire.CopyFrom(car_param.wheel_tire());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool BrakeHydraulicExt::initParam() {
  char log[1024]{'\0'};
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing brake hydraulic parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool BrakeHydraulicExt::initMDL() {
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
}  // namespace brake_hydraulic
}  // namespace tx_car
