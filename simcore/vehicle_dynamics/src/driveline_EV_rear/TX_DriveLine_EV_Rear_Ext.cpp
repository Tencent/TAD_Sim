// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "TX_DriveLine_EV_Rear_Ext.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

#include <algorithm>

namespace tx_car {
namespace driveline_ev_rear {
tx_car::DriveLine driveline;
tx_car::Pow_ECU soft_ecu;
tx_car::WheelTireDD wheel_tire;
tx_car::CarType cartype;
tx_car::SprungMassDD sprung;

TX_DriveLine_EV_Rear_Ext::TX_DriveLine_EV_Rear_Ext(/* args */) : TX_DriveLine_EV_Rear() {}

TX_DriveLine_EV_Rear_Ext::~TX_DriveLine_EV_Rear_Ext() {}

void TX_DriveLine_EV_Rear_Ext::initialize() {
  initMDL();
  TX_DriveLine_EV_Rear::initialize();
}

void TX_DriveLine_EV_Rear_Ext::step() { TX_DriveLine_EV_Rear::step(); }

void TX_DriveLine_EV_Rear_Ext::terminate() { TX_DriveLine_EV_Rear::terminate(); }

bool TX_DriveLine_EV_Rear_Ext::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init driveline parameter.\n";

  if (driveline.ByteSizeLong() == 0) {
    LOG_ERROR << "driveline data empty.\n";
    return false;
  }

  // EV car driveline trans ratio warning
  {
    // EV car
    double rearRatio =
        driveline.rear_motor_ratio().rear_motor_gear_ratio().val() * driveline.rear_fd().ratio_diff_rear().val();

    if (std::abs(rearRatio) < 3.0) {
      LOG_WARNING << "EV car rear driveline transfer raito too low warning, total " << rearRatio << ".\n";
      // throw(std::runtime_error("EV car rear driveline transfer raito too low warning"));
    }
  }

  // init sprung mass
  initVehParam(TX_DriveLine_EV_Rear_P.VEH, sprung);

  // init wheel radius
  TX_DriveLine_EV_Rear_P.StatLdWhlR[0] = std::max(wheel_tire.tire().wheel_radius().val(), 0.1);
  TX_DriveLine_EV_Rear_P.StatLdWhlR[1] = TX_DriveLine_EV_Rear_P.StatLdWhlR[0];
  TX_DriveLine_EV_Rear_P.StatLdWhlR[2] = TX_DriveLine_EV_Rear_P.StatLdWhlR[0];
  TX_DriveLine_EV_Rear_P.StatLdWhlR[3] = TX_DriveLine_EV_Rear_P.StatLdWhlR[0];

  // parse EV driveline data
  {
    // motor gear num
    // TX_DriveLine_EV_Rear_P.front_motor_gear_ratio = driveline.front_motor_ratio().front_motor_gear_ratio().val();
    // //front gear ratio for motor drive type without fd ratio
    TX_DriveLine_EV_Rear_P.rear_motor_gear_ratio = driveline.rear_motor_ratio()
                                                       .rear_motor_gear_ratio()
                                                       .val();  // rear gear ratio for motor drive type without fd ratio
    TX_DriveLine_EV_Rear_P.ratio_diff_rear =
        driveline.rear_fd()
            .ratio_diff_rear()
            .val();  // rear differential gear ratio    driveshaft ratio (NC/ND), Ndiff []: 2.77
    // front wheel drive
    // TX_DriveLine_EV_Rear_P.EVFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.EVFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; rear wheel drive
    // TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0];

    // driveline inertia and damping
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_Jd = 0.013;
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_Jw1 = 9e-3;
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_Jw2 = 9e-3;
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_bd = 1e-4;
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_bw2 = 1e-4;
    TX_DriveLine_EV_Rear_P.EVRearLimitedSlipDifferential_bw1 = 1e-4;

    // TX_DriveLine_EV_Rear_P.IdealFixedGearTransmission_omega_o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0] * (TX_DriveLine_EV_Rear_P.ratio_diff_front+
    // TX_DriveLine_EV_Rear_P.ratio_diff_rear)/2.0;  //Initial output velocity, omega_o [rad/s]: front wheel drive
    // TX_DriveLine_EV_Rear_P.ICEFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.ICEFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; rear wheel drive
    // TX_DriveLine_EV_Rear_P.ICERearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.ICERearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; all wheel drive
    // TX_DriveLine_EV_Rear_P.uWD_ICEFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.uWD_ICEFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0];
    // TX_DriveLine_EV_Rear_P.uWD_ICERearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0]; TX_DriveLine_EV_Rear_P.uWD_ICERearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_EV_Rear_P.StatLdWhlR[0];
    // TX_DriveLine_EV_Rear_P.TransferCase_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0] * TX_DriveLine_EV_Rear_P.ratio_diff_front;
    // TX_DriveLine_EV_Rear_P.TransferCase_omegaw2o = sprung.initiallongvel().val() /
    // TX_DriveLine_EV_Rear_P.StatLdWhlR[0] * TX_DriveLine_EV_Rear_P.ratio_diff_rear;
  }

  LOG_2 << "parsing driveline parameter successfully!\n";

  return true;
}

bool TX_DriveLine_EV_Rear_Ext::loadParam(const std::string& par_path) {
  // LOG_0<<"load json "<<par_path<<"\n";
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_0 << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        driveline.CopyFrom(car_param.driveline());
        cartype.CopyFrom(car_param.car_type());
        soft_ecu.CopyFrom(car_param.ecu());
        wheel_tire.CopyFrom(car_param.wheel_tire());
        sprung.CopyFrom(car_param.sprung_mass());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool TX_DriveLine_EV_Rear_Ext::initParam() {
  char log[1024];
  bool ok = false;
  ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing driveline parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool TX_DriveLine_EV_Rear_Ext::initMDL() {
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
}  // namespace driveline_ev_rear
}  // namespace tx_car
