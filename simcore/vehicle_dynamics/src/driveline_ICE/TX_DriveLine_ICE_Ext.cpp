// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "TX_DriveLine_ICE_Ext.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

#include <algorithm>

namespace tx_car {
namespace driveline_ice {
tx_car::DriveLine driveline;
tx_car::Pow_ECU soft_ecu;
tx_car::WheelTireDD wheel_tire;
tx_car::CarType cartype;
tx_car::SprungMassDD sprung;

TX_DriveLine_ICE_Ext::TX_DriveLine_ICE_Ext(/* args */) : TX_DriveLine_ICE() {
}

TX_DriveLine_ICE_Ext::~TX_DriveLine_ICE_Ext() {}

void TX_DriveLine_ICE_Ext::initialize() {
  initMDL();
  TX_DriveLine_ICE::initialize();
}

void TX_DriveLine_ICE_Ext::step() { TX_DriveLine_ICE::step(); }

void TX_DriveLine_ICE_Ext::terminate() { TX_DriveLine_ICE::terminate(); }

bool TX_DriveLine_ICE_Ext::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init driveline parameter.\n";

  if (driveline.ByteSizeLong() == 0) {
    LOG_ERROR << "driveline data empty.\n";
    return false;
  }

  // parse cartype
  {
    auto dirveType = cartype.drivetrain_type();
    auto powType = cartype.propulsion_type();
    switch (dirveType) {
      case tx_car::CarType_DriveTrain_Type_FrontWheel_Drive:
        TX_DriveLine_ICE_P.drive_type = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_RearWheel_Drive:
        TX_DriveLine_ICE_P.drive_type = 2;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_FourWheel_Drive:
        TX_DriveLine_ICE_P.drive_type = 3;  // DriveType 1-FD 2-RD 3-4WD
        break;
      default:
        TX_DriveLine_ICE_P.drive_type = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
    }
    // propulsion_type [2,5] --> hybrid --> engine is front wheel drive
    int powType_i = static_cast<int>(powType);
    if (2 <= powType_i && powType_i <= 5) {
      TX_DriveLine_ICE_P.drive_type = 1;  // DriveType 1-FD 2-RD 3-4WD
    }
    std::cout << " driveline type " << TX_DriveLine_ICE_P.drive_type << "\n";
    LOG_0 << " driveline type " << TX_DriveLine_ICE_P.drive_type << "\n";
  }

  // init sprung mass
  initVehParam(TX_DriveLine_ICE_P.VEH, sprung);

  // init wheel radius
  TX_DriveLine_ICE_P.StatLdWhlR[0] = std::max(wheel_tire.tire().wheel_radius().val(), 0.1);
  TX_DriveLine_ICE_P.StatLdWhlR[1] = TX_DriveLine_ICE_P.StatLdWhlR[0];
  TX_DriveLine_ICE_P.StatLdWhlR[2] = TX_DriveLine_ICE_P.StatLdWhlR[0];
  TX_DriveLine_ICE_P.StatLdWhlR[3] = TX_DriveLine_ICE_P.StatLdWhlR[0];

  // engine idle speed and max torque
  // TX_DriveLine_ICE_P.EngIdleSpd = std::max(soft_ecu.soft_ems().eng_idl_speed().val(), 750.0);
  // TX_DriveLine_ICE_P.pre_shift_wait_time = 0.05;

  // parse EV driveline data
  {
    // front wheel drive
    // TX_DriveLine_ICE_P.EVFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.EVFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0]; rear wheel drive
    // TX_DriveLine_ICE_P.EVRearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.EVRearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0];
  }

  // parse ICE driveline data
  {
    // Final Diff Paramters
    TX_DriveLine_ICE_P.ratio_diff_front =
        driveline.front_fd()
            .ratio_diff_front()
            .val();  // front differential gear ratio  driveshaft ratio (NC/ND), Ndiff []: 2.77
    TX_DriveLine_ICE_P.ratio_diff_rear =
        driveline.rear_fd()
            .ratio_diff_rear()
            .val();  // rear differential gear ratio    driveshaft ratio (NC/ND), Ndiff []: 2.77

    // TX_DriveLine_ICE_P.IdealFixedGearTransmission_omega_o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0] * (TX_DriveLine_ICE_P.ratio_diff_front+ TX_DriveLine_ICE_P.ratio_diff_rear)/2.0;
    // //Initial output velocity, omega_o [rad/s]: front wheel drive
    // TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.ICEFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0]; rear wheel drive
    // TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.ICERearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0]; all wheel drive
    // TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.uWD_ICEFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0];
    // TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.uWD_ICERearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0]; TX_DriveLine_ICE_P.TransferCase_omegaw1o =
    // sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0] * TX_DriveLine_ICE_P.ratio_diff_front;
    // TX_DriveLine_ICE_P.TransferCase_omegaw2o = sprung.initiallongvel().val() / TX_DriveLine_ICE_P.StatLdWhlR[0] *
    // TX_DriveLine_ICE_P.ratio_diff_rear;
  }

  LOG_2 << "parsing driveline parameter successfully!\n";

  return true;
}

bool TX_DriveLine_ICE_Ext::loadParam(const std::string& par_path) {
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
bool TX_DriveLine_ICE_Ext::initParam() {
  char log[1024];
  bool ok = false;
  ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing driveline parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool TX_DriveLine_ICE_Ext::initMDL() {
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
}  // namespace driveline_ice
}  // namespace tx_car
