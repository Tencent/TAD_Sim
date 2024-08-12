// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "DriveLineExt.h"
#include "../../inc/car_common.h"
#include "../../inc/car_log.h"
#include "../../inc/proto_helper.h"
#include "car.pb.h"

#include <algorithm>

namespace tx_car {
namespace driveline {
tx_car::DriveLine driveline;
tx_car::Pow_ECU soft_ecu;
tx_car::WheelTireDD wheel_tire;
tx_car::CarType cartype;
tx_car::SprungMassDD sprung;

DriveLineExt::DriveLineExt(/* args */) : TX_DriveLine() {}

DriveLineExt::~DriveLineExt() {}

void DriveLineExt::initialize() {
  initMDL();
  TX_DriveLine::initialize();
}

void DriveLineExt::step() { TX_DriveLine::step(); }

void DriveLineExt::terminate() { TX_DriveLine::terminate(); }

bool DriveLineExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init driveline parameter.\n";

  if (driveline.ByteSizeLong() == 0) {
    LOG_ERROR << "driveline data empty.\n";
    return false;
  }

  // parse cartype
  {
    auto dirveType = cartype.drivetrain_type();
    switch (dirveType) {
      case tx_car::CarType_DriveTrain_Type_FrontWheel_Drive:
        TX_DriveLine_P.drive_type = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_RearWheel_Drive:
        TX_DriveLine_P.drive_type = 2;  // DriveType 1-FD 2-RD 3-4WD
        break;
      case tx_car::CarType_DriveTrain_Type_FourWheel_Drive:
        TX_DriveLine_P.drive_type = 3;  // DriveType 1-FD 2-RD 3-4WD
        break;
      default:
        TX_DriveLine_P.drive_type = 1;  // DriveType 1-FD 2-RD 3-4WD
        break;
    }
    auto proptype = cartype.propulsion_type();
    switch (proptype) {
      case tx_car::CarType_Propulsion_Type_ICE_Engine:
        TX_DriveLine_P.driveline_pow_type = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
      case tx_car::CarType_Propulsion_Type_Electric_Drive:
        TX_DriveLine_P.driveline_pow_type = 2;  // DriveLinePowType 1-ICE 2-MOT
        break;
      default:
        TX_DriveLine_P.driveline_pow_type = 1;  // DriveLinePowType 1-ICE 2-MOT
        break;
    }

    std::cout << "Parsing driveline_pow_type " << static_cast<uint32_T>(TX_DriveLine_P.driveline_pow_type)
              << " driveline type " << TX_DriveLine_P.drive_type << "\n";
    LOG_0 << "Parsing driveline_pow_type " << static_cast<uint32_T>(TX_DriveLine_P.driveline_pow_type)
          << " driveline type " << TX_DriveLine_P.drive_type << "\n";
  }

  // EV car driveline trans ratio warning
  {
    // EV car
    if (TX_DriveLine_P.driveline_pow_type == 2) {
      double frontRatio =
          driveline.front_motor_ratio().front_motor_gear_ratio().val() * driveline.front_fd().ratio_diff_front().val();
      double rearRatio =
          driveline.rear_motor_ratio().rear_motor_gear_ratio().val() * driveline.rear_fd().ratio_diff_rear().val();

      if (TX_DriveLine_P.drive_type == tx_car::CarType_DriveTrain_Type_FrontWheel_Drive) {
        if (std::abs(frontRatio) < 3.0) {
          LOG_WARNING << "EV car front driveline transfer raito too low warning, total " << frontRatio << ".\n";
          // throw(std::runtime_error("EV car front driveline transfer raito too low warning"));
        }
      } else if (TX_DriveLine_P.drive_type == tx_car::CarType_DriveTrain_Type_RearWheel_Drive) {
        if (std::abs(rearRatio) < 3.0) {
          LOG_WARNING << "EV car rear driveline transfer raito too low warning, total " << rearRatio << ".\n";
          // throw(std::runtime_error("EV car rear driveline transfer raito too low warning"));
        }
      }
      if (TX_DriveLine_P.drive_type == tx_car::CarType_DriveTrain_Type_FourWheel_Drive) {
        if (std::abs(frontRatio) < 3.0) {
          LOG_WARNING << "EV car front driveline transfer raito too low warning, total " << frontRatio << ".\n";
          // throw(std::runtime_error("EV car front driveline transfer raito too low warning"));
        }
        if (std::abs(rearRatio) < 3.0) {
          LOG_WARNING << "EV car rear driveline transfer raito too low warning, total " << rearRatio << ".\n";
          // throw(std::runtime_error("EV car rear driveline transfer raito too low warning"));
        }
      }
    }
  }

  // init sprung mass
  initVehParam(TX_DriveLine_P.VEH, sprung);

  // init wheel radius
  TX_DriveLine_P.StatLdWhlR[0] = std::max(wheel_tire.tire().wheel_radius().val(), 0.1);
  TX_DriveLine_P.StatLdWhlR[1] = TX_DriveLine_P.StatLdWhlR[0];
  TX_DriveLine_P.StatLdWhlR[2] = TX_DriveLine_P.StatLdWhlR[0];
  TX_DriveLine_P.StatLdWhlR[3] = TX_DriveLine_P.StatLdWhlR[0];

  // engine idle speed and max torque
  TX_DriveLine_P.EngIdleSpd = std::max(soft_ecu.soft_ems().eng_idl_speed().val(), 750.0);
  TX_DriveLine_P.pre_shift_wait_time = 0.05;

  // parse EV driveline data
  {
    // motor gear num
    TX_DriveLine_P.front_motor_gear_ratio = driveline.front_motor_ratio()
                                                .front_motor_gear_ratio()
                                                .val();  // front gear ratio for motor drive type without fd ratio
    TX_DriveLine_P.rear_motor_gear_ratio = driveline.rear_motor_ratio()
                                               .rear_motor_gear_ratio()
                                               .val();  // rear gear ratio for motor drive type without fd ratio

    // front wheel drive
    // TX_DriveLine_P.EVFrontLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; TX_DriveLine_P.EVFrontLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0]; rear wheel drive
    // TX_DriveLine_P.EVRearLimitedSlipDifferential_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; TX_DriveLine_P.EVRearLimitedSlipDifferential_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0];
  }

  // parse ICE driveline data
  {
    // TCU
    const tx_car::DriveLine_TCU& tcu = driveline.tcu();
    // AT
    const tx_car::DriveLine_TM& tm = driveline.tm();

    // gear number
    if (tm.tr_gear_ratio_table().u0_axis().data_size() < 3) {
      LOG_ERROR << "gear number should at least be 3.\n";
      return false;
    }

    m_AT_gear_num = tm.tr_gear_ratio_table().u0_axis().data_size();  // -1 0 1~8
    m_AT_gear_num = std::min(constMaxGearNum, m_AT_gear_num);        // limit gear number
    m_AT_shift_gear_num = m_AT_gear_num - 3;                         // gear number involved in shifting
    // std::cout << "m_AT_gear_num:" << m_AT_gear_num << "\n";
    // std::cout << "m_AT_shift_gear_num:" << m_AT_shift_gear_num << "\n";

    // TX_DriveLine_P.TransmissionControllerPRNDL_LugSpd = 0.0;
    // TX_DriveLine_P.TransmissionControllerPRNDL_MinUpSpd = 0.0;
    // TX_DriveLine_P.TransmissionControllerPRNDL_GearInit = tcu.gear_init_num().val();
    // TX_DriveLine_P.PropShaft_b = 50.0;

    // up shift table
    int upshitTableAccNum = tcu.muc_up_shift_spd_table().u0_axis().data_size();
    int upshitTableGearNum = tcu.muc_up_shift_spd_table().u1_axis().data_size();
    int upshitTableSpdNum = tcu.muc_up_shift_spd_table().y0_axis().data_size();

    if (upshitTableAccNum * upshitTableGearNum != upshitTableSpdNum) {
      LOG_ERROR << "mcu upshit table spd points number is not equal acc load number x gear number !\n";
      return false;
    }

    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Pedal_Positions_UpShift, tcu.muc_up_shift_spd_table().u0_axis());
    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Up_Gears, tcu.muc_up_shift_spd_table().u1_axis(),
                     m_AT_shift_gear_num);
    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Upshift_Speeds, tcu.muc_up_shift_spd_table().y0_axis());
    TX_DriveLine_P.CalculateUpshiftThreshold_maxIndex[0] = upshitTableAccNum - 1;
    TX_DriveLine_P.CalculateUpshiftThreshold_maxIndex[1] = upshitTableGearNum - 1;
    m_upshift_accload_points_num = upshitTableAccNum;

    // std::cout << "upshitTableAccNum:" << upshitTableAccNum << "\n";
    // std::cout << "upshitTableGearNum:" << upshitTableGearNum << "\n";
    // std::cout << "upshitTableSpdNum:" << upshitTableSpdNum << "\n";

    // parsing down shift MCU table map
    int downshitTableAccNum = tcu.muc_down_shift_spd_table().u0_axis().data_size();
    int downshitTableGearNum = tcu.muc_down_shift_spd_table().u1_axis().data_size();
    int downshitTableSpdNum = tcu.muc_down_shift_spd_table().y0_axis().data_size();
    if (downshitTableAccNum * downshitTableGearNum != downshitTableSpdNum) {
      LOG_ERROR << "mcu downshit table spd points number is not equal acc load number x gear number !\n";
      return false;
    }

    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Pedal_Positions_DnShift, tcu.muc_down_shift_spd_table().u0_axis());
    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Dn_Gears, tcu.muc_down_shift_spd_table().u1_axis(),
                     m_AT_shift_gear_num);
    tx_car::initAxis(DriveLineExt::TX_DriveLine_P.Downshift_Speeds, tcu.muc_down_shift_spd_table().y0_axis());
    TX_DriveLine_P.CalculateDownshiftThreshold_maxIndex[0] = downshitTableAccNum - 1;
    TX_DriveLine_P.CalculateDownshiftThreshold_maxIndex[1] = downshitTableGearNum - 1;
    m_downshift_accload_points_num = downshitTableAccNum;

    // std::cout << "downshitTableAccNum:" << downshitTableAccNum << "\n";
    // std::cout << "downshitTableGearNum:" << downshitTableGearNum << "\n";
    // std::cout << "downshitTableSpdNum:" << downshitTableSpdNum << "\n";
    // std::cout << "TX_DriveLine_P.StatLdWhlR[0]:" << TX_DriveLine_P.StatLdWhlR[0] << "\n";

    int GearRatiosTableU0Num =
        std::min(tm.tr_gear_ratio_table().u0_axis().data_size(), constMaxGearNum);  // limit gear number
    int GearRatiosTableY0Num = tm.tr_gear_ratio_table().y0_axis().data_size();

    int EfficienciesTableU0Num = tm.tr_gear_num_efficien_table().u0_axis().data_size();
    int EfficienciesTableY0Num = tm.tr_gear_num_efficien_table().y0_axis().data_size();

    int DampingCoeffsTableU0Num = tm.tr_gear_num_damping_coeffs_table().u0_axis().data_size();
    int DampingCoeffsTableY0Num = tm.tr_gear_num_damping_coeffs_table().y0_axis().data_size();

    int InertiasTableU0Num = tm.tr_gear_num_inertias_table().u0_axis().data_size();
    int InertiasTableY0Num = tm.tr_gear_num_inertias_table().y0_axis().data_size();

    if (!((GearRatiosTableU0Num == EfficienciesTableU0Num) && (GearRatiosTableU0Num == DampingCoeffsTableU0Num) &&
          (GearRatiosTableU0Num == InertiasTableU0Num) && (GearRatiosTableU0Num == GearRatiosTableY0Num) &&
          (GearRatiosTableU0Num == EfficienciesTableY0Num) && (GearRatiosTableU0Num == DampingCoeffsTableY0Num) &&
          (GearRatiosTableU0Num == InertiasTableY0Num) && GearRatiosTableU0Num >= 3)
            size of all ta) {  // ble should be same
      LOG_ERROR << "GearRatios table,  Efficiencies table, DampingCoeffs table or Inertias table does not has the same "
                   "points please check !\n";
      return false;
    }

    // std::cout << "GearRatiosTableU0Num:" << GearRatiosTableU0Num << "\n";

    for (auto i = 0; i < GearRatiosTableU0Num; i++) {
      TX_DriveLine_P.Trans.GearNums[i] = tm.tr_gear_ratio_table().u0_axis().data(
          i);  // Gear number vector, G []:  -1     0     1     2     3     4     5     6     7     8
      TX_DriveLine_P.Trans.GearRatios[i] = tm.tr_gear_ratio_table().y0_axis().data(
          i);  // Gear number vector, G []:
               // -3.8200    4.5600    4.5600    2.9700    2.0800    1.6900    1.2700    1.0000    0.8500    0.6500
      TX_DriveLine_P.Trans.Efficiencies[i] = tm.tr_gear_num_efficien_table().y0_axis().data(
          i);  // Efficiency vector, eta []:   { 0.9, 0.9, 0.9, 0.9, 0.9, 0.95, 0.95, 0.95, 0.95, 0.95 },
      TX_DriveLine_P.Trans.DampingCoeffs[i] = tm.tr_gear_num_damping_coeffs_table().y0_axis().data(
          i);  // Damping vector, bout [N*m*s/rad]: 0.0030    0.0010    0.0030    0.0025    0.0020    0.0010    0.0010
               // 0.0010    0.0010    0.0010
      TX_DriveLine_P.Trans.Inertias[i] = tm.tr_gear_num_inertias_table().y0_axis().data(
          i);  // Inertia vector, Jout [kg*m^2]: 0.1280    0.0100    0.1280    0.1000    0.0620    0.0280    0.0100
               // 0.0100    0.0100    0.0100
    }

    // netural gear ratio set to no-zero
    TX_DriveLine_P.Trans.GearRatios[1] = TX_DriveLine_P.Trans.GearRatios[2];

    TX_DriveLine_P.Trans.ShiftTau = tm.gear_shift_tau().val();  // Shift time constant, tau_s [s]:
    TX_DriveLine_P.Trans.ShiftTau = std::max(TX_DriveLine_P.Trans.ShiftTau, 0.05);

    TX_DriveLine_P.IdealFixedGearTransmission_G_o = tcu.gear_init_num().val();  // init gear

    // TX_DriveLine_P.IdealFixedGearTransmission_omegaN_o = TX_DriveLine_P.EngIdleSpd / 60 * 2 * 3.1415926; //Initial
    // neutral gear input rotational velocity, omegaN_o [rad/s]:

    // Final Diff Paramters
    TX_DriveLine_P.ratio_diff_front =
        driveline.front_fd()
            .ratio_diff_front()
            .val();  // front differential gear ratio  driveshaft ratio (NC/ND), Ndiff []: 2.77
    TX_DriveLine_P.ratio_diff_rear =
        driveline.rear_fd()
            .ratio_diff_rear()
            .val();  // rear differential gear ratio    driveshaft ratio (NC/ND), Ndiff []: 2.77

    // driveline inertia and damping
    TX_DriveLine_P.EVRearLimitedSlipDifferential_Jd = 0.013;
    TX_DriveLine_P.EVRearLimitedSlipDifferential_Jw1 = 9e-3;
    TX_DriveLine_P.EVRearLimitedSlipDifferential_Jw2 = 9e-3;
    TX_DriveLine_P.EVRearLimitedSlipDifferential_bd = 1e-4;
    TX_DriveLine_P.EVRearLimitedSlipDifferential_bw2 = 1e-4;
    TX_DriveLine_P.EVRearLimitedSlipDifferential_bw1 = 1e-4;

    // TX_DriveLine_P.IdealFixedGearTransmission_omega_o = sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0]
    // * (TX_DriveLine_P.ratio_diff_front+ TX_DriveLine_P.ratio_diff_rear)/2.0;  //Initial output velocity, omega_o
    // [rad/s]: front wheel drive TX_DriveLine_P.ICEFrontLimitedSlipDifferential_omegaw1o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0];
    // TX_DriveLine_P.ICEFrontLimitedSlipDifferential_omegaw2o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; rear wheel drive TX_DriveLine_P.ICERearLimitedSlipDifferential_omegaw1o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0];
    // TX_DriveLine_P.ICERearLimitedSlipDifferential_omegaw2o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; all wheel drive TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_omegaw1o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0];
    // TX_DriveLine_P.uWD_ICEFrontLimitedSlipDifferential_omegaw2o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_omegaw1o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0];
    // TX_DriveLine_P.uWD_ICERearLimitedSlipDifferential_omegaw2o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0]; TX_DriveLine_P.TransferCase_omegaw1o = sprung.initiallongvel().val() /
    // TX_DriveLine_P.StatLdWhlR[0] * TX_DriveLine_P.ratio_diff_front; TX_DriveLine_P.TransferCase_omegaw2o =
    // sprung.initiallongvel().val() / TX_DriveLine_P.StatLdWhlR[0] * TX_DriveLine_P.ratio_diff_rear;
  }

  LOG_2 << "parsing driveline parameter successfully!\n";

  return true;
}

bool DriveLineExt::loadParam(const std::string& par_path) {
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
bool DriveLineExt::initParam() {
  char log[1024];
  bool ok = false;
  ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing driveline parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool DriveLineExt::initMDL() {
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
}  // namespace driveline
}  // namespace tx_car
