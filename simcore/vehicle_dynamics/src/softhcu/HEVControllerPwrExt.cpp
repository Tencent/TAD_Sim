// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "HEVControllerPwrExt.h"

#include "car.pb.h"
#include "inc/car_common.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

uint32_t HEVControllerPwr::m_EngOptimal_Pwr_MaxIndex = 10;
uint32_t HEVControllerPwr::m_Inv_EngOptimal_Pwr_MaxIndex = 10;
uint32_t HEVControllerPwr::m_FrontMot_TrqTable_MaxIndex = 12;
uint32_t HEVControllerPwr::m_P4Mot_TrqTable_MaxIndex = 12;

namespace tx_car {
namespace hcu {
tx_car::Pow_ECU ecu;
tx_car::CarType cartype;
tx_car::WheelTireDD wheel_tire;
tx_car::SprungMassDD sprungMass;
tx_car::Propulsion g_propulsion;

HEVControllerPwrExt::HEVControllerPwrExt(/* args */) {}

HEVControllerPwrExt::~HEVControllerPwrExt() {}

tx_car::Real1DMap InvEngineOptimal_Pwr(const tx_car::Real1DMap& engineOptimal_Pwr) {
  tx_car::Real1DMap InvEngineOptimal_Pwr;

  if (!tx_car::map1DFormatChecker(engineOptimal_Pwr)) {
    std::abort();
  }

  // find longest increase sub sequence
  int lngstSize = 0, lngstS = 0, lngstE = 0;
  int curS = 0, curE = 0, curSize = 0;

  // pwr size
  int endIndex = engineOptimal_Pwr.y0_axis().data_size() - 1;
  const auto& yAxis = engineOptimal_Pwr.y0_axis();
  const auto& xAxis = engineOptimal_Pwr.u0_axis();

  for (curS = 0, curE = 1; curE < endIndex; ++curE) {
    if (yAxis.data().at(curE) > yAxis.data().at(curE - 1)) {
      curE++;
      curSize++;
    } else {
      if (curSize > lngstSize) {
        lngstS = curS;
        lngstE = curE - 1;
        lngstSize = curSize;
      }
      curS = curE;
    }
  }

  // copy longest increase sub sequence
  for (auto i = lngstS; i <= lngstE; ++i) {
    InvEngineOptimal_Pwr.mutable_u0_axis()->add_data(yAxis.data().at(i));
    InvEngineOptimal_Pwr.mutable_y0_axis()->add_data(xAxis.data().at(i));
  }

  if (!tx_car::map1DFormatChecker(InvEngineOptimal_Pwr)) {
    LOG_ERROR << "InvEngineOptimal_Pwr format error.\n";
    std::cout << "InvEngineOptimal_Pwr format error.\n";
    LOG_ERROR << InvEngineOptimal_Pwr.DebugString() << "\n";
    std::cout << InvEngineOptimal_Pwr.DebugString() << "\n";
    throw std::runtime_error("InvEngineOptimal_Pwr format error.");
  }

  return InvEngineOptimal_Pwr;
}

void HEVControllerPwrExt::initialize() {
  initMDL();  //
  HEVControllerPwr::initialize();
}

void HEVControllerPwrExt::step() {
  HEVControllerPwr::step();
  // LOG_0<<"ecu out "<<getExternalOutputs().ecu_out.pt_trq_cmd_Nm<<"\n";
}

void HEVControllerPwrExt::terminate() { HEVControllerPwr::terminate(); }

bool HEVControllerPwrExt::parsingParameterFromJson(char* errorLog) {
  LOG_2 << "init HEVControllerPwr parameter.\n";

  // init VEH
  tx_car::initVehParam(HEVControllerPwr_P.VEH, sprungMass);

  // step time
  HEVControllerPwr_P.stepTime = 0.001;

  // auto hold vx speed threshold
  HEVControllerPwr_P.speedOffset = 0.01;

  // brake enable acc threshold
  HEVControllerPwr_P.brakeEnableThresh_Reverse_const = 0.01;
  HEVControllerPwr_P.brakeEnableThresh_const = -0.01;

  // delay 100 steps to calculate ax in low speed
  HEVControllerPwr_P.softecu_ax_delay = 100;

  // offset to check if reset hcu pid
  // HEVControllerPwr_P.Forward_const = HEVControllerPwr_P.speedOffset;
  // HEVControllerPwr_P.Reverse_const = -HEVControllerPwr_P.speedOffset;
  HEVControllerPwr_P.ReverseSpeedLimit_kph_OnVal = -5 * 3.6;
  HEVControllerPwr_P.ReverseSpeedLimit_kph_OffVal = -6 * 3.6;

  if (ecu.ByteSizeLong()) {
    // default switch
    HEVControllerPwr_P.EnableP4 = 0;
    HEVControllerPwr_P.DriveLinePowType = 2;  // used in autohold

    auto proptype = cartype.propulsion_type();

    switch (proptype) {
      case tx_car::CarType_Propulsion_Type_ICE_Engine:
        HEVControllerPwr_P.DriveLinePowType = 1;  // DriveLinePowType 1-ICE 2-MOT
        throw std::runtime_error("propulsion type is Engine, expected Hybrid.\n");
        break;
      case tx_car::CarType_Propulsion_Type_Electric_Drive:
        HEVControllerPwr_P.DriveLinePowType = 2;  // DriveLinePowType 1-ICE 2-MOT
        throw std::runtime_error("propulsion type is EV, expected Hybrid.\n");
        break;
      case tx_car::CarType_Propulsion_Type_Hybrid_Gen_P2:
        HEVControllerPwr_P.EnableP4 = 0;
        LOG_0 << "car propulsion type is P2 with Generator.\n";
        break;
      case tx_car::CarType_Propulsion_Type_Hybrid_Gen_P3:
        HEVControllerPwr_P.EnableP4 = 0;
        LOG_0 << "car propulsion type is P3 with Generator.\n";
        break;
      case tx_car::CarType_Propulsion_Type_Hybrid_Gen_P2P4:
        HEVControllerPwr_P.EnableP4 = 1;
        LOG_0 << "car propulsion type is P2P4 with Generator.\n";
        break;
      case tx_car::CarType_Propulsion_Type_Hybrid_Gen_P3P4:
        HEVControllerPwr_P.EnableP4 = 1;
        LOG_0 << "car propulsion type is P3P4 with Generator.\n";
        break;
      default:
        HEVControllerPwr_P.DriveLinePowType = 2;  // DriveLinePowType 1-ICE 2-MOT
        HEVControllerPwr_P.EnableP4 = 0;
        break;
    }
    LOG_2 << "DriveLinePowType is " << HEVControllerPwr_P.DriveLinePowType << "\n";

    // max acc
    HEVControllerPwr_P.ecu_max_acc = ecu.ecu_max_acc().val();
    HEVControllerPwr_P.ecu_max_dec = ecu.ecu_max_dec().val() >= 0.0 ? -10 : ecu.ecu_max_dec().val();

    // hcu
    const tx_car::Pow_ECU_SoftHCU& hcu = ecu.soft_hcu();
    HEVControllerPwr_P.hcu_acc_P = hcu.hcu_acc_p().val();
    HEVControllerPwr_P.hcu_acc_I = hcu.hcu_acc_i().val();
    HEVControllerPwr_P.hcu_acc_D = hcu.hcu_acc_d().val();

    // abs
    const tx_car::Pow_ECU_SoftABS& abs = ecu.soft_abs();
    HEVControllerPwr_P.enable_abs = abs.abs_enable().val() > 0.5;
    HEVControllerPwr_P.slip_ratio_ref = abs.slip_ratio_ref().val();
    HEVControllerPwr_P.slip_disable_ratio_ref = std::max(abs.slip_ratio_disable_ref().val(), 0.1);
    HEVControllerPwr_P.abs_brake_pressure = abs.abs_brake_pressure().val();

    HEVControllerPwr_P.brake_P = abs.brake_p().val();
    HEVControllerPwr_P.brake_I = abs.brake_i().val();
    HEVControllerPwr_P.brake_D = abs.brake_d().val();

    // engine
    HEVControllerPwr_P.EngIdleSpd = 750;
    HEVControllerPwr_P.EngMaxSpd = 6700;
    tx_car::initMap1D(hcu.optimal_engine_pwr(), HEVControllerPwr_P.HEVController_EngOptimal_Speed_RPM,
                      HEVControllerPwr_P.HEVController_EngOptimal_Pwr_KW, m_EngOptimal_Pwr_MaxIndex);

    auto&& invEngineOptimalPwr = InvEngineOptimal_Pwr(hcu.optimal_engine_pwr());
    // std::cout << invEngineOptimalPwr.DebugString() << "\n";
    LOG_2 << invEngineOptimalPwr.DebugString() << "\n";
    tx_car::initMap1D(invEngineOptimalPwr, HEVControllerPwr_P.Inverse_EngOptimal_Pwr_KW,
                      HEVControllerPwr_P.Inverse_EngOptimal_Speed_RPM, m_Inv_EngOptimal_Pwr_MaxIndex);

    // HEV
    HEVControllerPwr_P.HEV_Mode_Duration = 0.5;
    HEVControllerPwr_P.SoftHCU_AirDensity = 1.204;
    HEVControllerPwr_P.SoftHCU_RollingCoeff = 0.02;
    HEVControllerPwr_P.VXLOW_ReGen_const = 10;  // unit kph, power regen is enabled if vehicle speed is above this value
    HEVControllerPwr_P.SoftHCU_MaxPwrOfSystem = std::max(hcu.pwr_max_system().val(), 22500.0);
    // HEVControllerPwr_P.MaxPwrOfSystem_Value = std::max(HEVControllerPwr_P.SoftHCU_MaxPwrOfSystem, 22500.0);
    HEVControllerPwr_P.HEVController_BrakeEnableThresh = 0.01;
    HEVControllerPwr_P.HEVController_ThrottleDisableThresh = 0.01;
    HEVControllerPwr_P.HEVController_PwrDmnd_EV_Only = std::max(hcu.pwr_dmnd_ev().val(), 3300.0);
    HEVControllerPwr_P.HEVController_PwrDmnd_Hybrid = std::max(hcu.pwr_dmnd_hybrid().val(), 10000.0);
    HEVControllerPwr_P.HEVController_MaxChrgPwr = std::max(hcu.pwr_max_charge().val(), 3300.0);
    HEVControllerPwr_P.HEVController_DefaultChrgPwr = std::max(hcu.pwr_default_charge().val(), 3300.0);
    HEVControllerPwr_P.HEVController_MaxReGenPwr = std::max(hcu.pwr_max_regen().val(), 3300.0);
    HEVControllerPwr_P.HEVController_MinSocDriveVehicle = std::max(hcu.soc_min_required().val(), 10.0);
    HEVControllerPwr_P.HEVController_StartChargingSoc = std::max(hcu.soc_start_charging().val(), 15.0);
    HEVControllerPwr_P.HEVController_TargetSoc = std::max(hcu.soc_end_charging().val(), 70.0);
    HEVControllerPwr_P.HEVController_EngineStepInSpeed_kph = std::max(hcu.engine_stepin_speed_kph().val(), 20.0);

    // front motor
    const auto& frontMotTrq_Map1D = g_propulsion.front_motor_parameter().mot_max_tor_map();
    tx_car::initMap1D(frontMotTrq_Map1D, HEVControllerPwr_P.HEVController_FrontMotor_Speed_rads,
                      HEVControllerPwr_P.HEVController_FrontMotor_MaxTorque, m_FrontMot_TrqTable_MaxIndex);

    // P4 motor
    if (HEVControllerPwr_P.EnableP4 > 0.5) {
      const auto& P4MotTrq_Map1D = g_propulsion.rear_motor_parameter().mot_max_tor_map();
      tx_car::initMap1D(P4MotTrq_Map1D, HEVControllerPwr_P.HEVController_P4_Motor_Speed_rads,
                        HEVControllerPwr_P.HEVController_P4_Motor_MaxTorque, m_P4Mot_TrqTable_MaxIndex);
    }

    HEVControllerPwr_P.speedOffset = 0.2;  // 0.01;
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

bool HEVControllerPwrExt::loadParam(const std::string& par_path) {
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
        sprungMass.CopyFrom(car_param.sprung_mass());
        g_propulsion.CopyFrom(car_param.propulsion());
        return true;
      }
    }
  }
  return false;
}

/* init model by loaded json file */
bool HEVControllerPwrExt::initParam() {
  char log[1024]{'\0'};
  bool ok = parsingParameterFromJson(log);
  if (!ok) {
    LOG_0 << "parsing power ecu parameters failed:\n " << log << "\n";
  }
  return ok;
}

/* load parameter from json file and init model */
bool HEVControllerPwrExt::initMDL() {
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
}  // namespace hcu
}  // namespace tx_car