// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "DynamicSteerExt.h"
#include "car.pb.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
namespace steer {
tx_car::SteerMapped g_steer_mapped;
tx_car::DynamicSteer g_dynamic_steer;

DynamicSteerExt::DynamicSteerExt() { g_steer_mapped.Clear(); }

DynamicSteerExt::~DynamicSteerExt() {}

/* Initial conditions function */
void DynamicSteerExt::initialize() {
  initMDL();
  DynamicSteer::initialize();
}

/* model step function */
void DynamicSteerExt::step() { DynamicSteer::step(); }

/* model terminate function */
void DynamicSteerExt::terminate() { DynamicSteer::terminate(); }

bool DynamicSteerExt::loadParam(const std::string& par_path) {
  if (tx_car::isFileExist(par_path)) {
    std::string car_json;
    if (tx_car::car_config::loadFromFile(car_json, par_path)) {
      tx_car::car car_param;
      // LOG_DEBUG << "json string:\n" << car_json << "\n";
      //  parse from json content
      if (tx_car::jsonToProto(car_json, car_param)) {
        g_steer_mapped.CopyFrom(car_param.steer_system().steermapped());
        g_dynamic_steer.CopyFrom(car_param.steer_system().dynamicsteer());
        return true;
      }
    }
  }

  return false;
}

/* init model by loaded json file */
bool DynamicSteerExt::initParam() {
  // mapped steer
  initMappedSteer();

  // rack and pinion dynamic steer
  initDynamicSteer();

  return true;
}

/* set values of mapped steer */
void DynamicSteerExt::initMappedSteer() {
  LOG_2 << "init mapped steer parameter.\n";

  // steer second order dynamic
  double steerNaturalFrequency =
      g_steer_mapped.steernaturalfreuency().val() <= 0.0 ? 16.1097 : g_steer_mapped.steernaturalfreuency().val();
  double steerDampingRatio =
      g_steer_mapped.steerdampingratio().val() <= 0.0 ? 0.4705 : g_steer_mapped.steerdampingratio().val();
  steerDampingRatio = std::min(steerDampingRatio, 1.0);
  DynamicSteerExt::DynamicSteer_P.TransferFcn_A[0] = -2 * steerDampingRatio * steerNaturalFrequency;
  DynamicSteerExt::DynamicSteer_P.TransferFcn_A[1] = -1 * steerNaturalFrequency * steerNaturalFrequency;
  DynamicSteerExt::DynamicSteer_P.TransferFcn_C[1] = steerNaturalFrequency * steerNaturalFrequency;

  // rack vs steer
  tx_car::initMap1D(g_steer_mapped.rackdisp_vs_steerang(), DynamicSteerExt::DynamicSteer_P.MappedSteer_SteerAngBps,
                    DynamicSteerExt::DynamicSteer_P.MappedSteer_RackDispBpts, m_steer_bps_maxIndex);
  /*
  // left wheel angle vs rack
  tx_car::initMap1D(g_steer_mapped.leftwhl_vs_rackdisp(),
      DynamicSteerExt::DynamicSteer_P.MappedSteer_RackDispBpts,
      DynamicSteerExt::DynamicSteer_P.MappedSteer_WhlLftTbl, m_steer_bps_maxIndex);

  // right wheel angle vs rack
  tx_car::initMap1D(g_steer_mapped.rightwhl_vs_rackdisp(),
      DynamicSteerExt::DynamicSteer_P.MappedSteer_RackDispBpts,
      DynamicSteerExt::DynamicSteer_P.MappedSteer_WhlRghtTbl, m_steer_bps_maxIndex);
  */
  auto averageAxis = [](tx_car::RealAxis& yAxis_1, tx_car::RealAxis& yAxis_2) {
    const int l_flip = -1;

    // flip axis data
    for (auto i = 0ul; i < yAxis_1.data_size() && i < yAxis_2.data_size(); ++i) {
      double avg = (yAxis_1.data(i) + yAxis_2.data(i)) / 2.0;
      yAxis_1.set_data(i, avg);
      yAxis_2.set_data(i, avg);
    }
  };
  auto& axis_1 = *g_steer_mapped.mutable_leftwhl_vs_rackdisp()->mutable_y0_axis();
  auto& axis_2 = *g_steer_mapped.mutable_rightwhl_vs_rackdisp()->mutable_y0_axis();
  flipAndExchangeAxisData(axis_1);
  flipAndExchangeAxisData(axis_2);
  // averageAxis(axis_1, axis_2);

  // left wheel angle vs rack
  tx_car::initMap1D(g_steer_mapped.leftwhl_vs_rackdisp(), DynamicSteerExt::DynamicSteer_P.MappedSteer_RackDispBpts,
                    DynamicSteerExt::DynamicSteer_P.MappedSteer_WhlLftTbl, m_steer_bps_maxIndex);

  // right wheel angle vs rack
  tx_car::initMap1D(g_steer_mapped.rightwhl_vs_rackdisp(), DynamicSteerExt::DynamicSteer_P.MappedSteer_RackDispBpts,
                    DynamicSteerExt::DynamicSteer_P.MappedSteer_WhlRghtTbl, m_steer_bps_maxIndex);

  auto steerTableSize = g_steer_mapped.rackdisp_vs_steerang().u0_axis().data_size();
  auto leftTableSize = g_steer_mapped.leftwhl_vs_rackdisp().u0_axis().data_size();
  auto rightTableSize = g_steer_mapped.rightwhl_vs_rackdisp().u0_axis().data_size();

  if (steerTableSize != leftTableSize || leftTableSize != rightTableSize) {
    LOG_ERROR << "map data size of in steer mapped should be exactly same. steerTableSize:" << steerTableSize
              << ", leftTableSize:" << leftTableSize << ", rightTableSize:" << rightTableSize << "\n";
    throw(std::runtime_error("map data size of in steer mapped should be exactly same."));
  }

  // speed coeff
  tx_car::initMap1D(g_steer_mapped.vehiclespeedcoeff(), DynamicSteerExt::DynamicSteer_P.MappedSteer_VehSpdBpts,
                    DynamicSteerExt::DynamicSteer_P.MappedSteer_SpdFctTbl, m_speed_coef_maxIndex);

  LOG_2 << "end of init mapped steer parameter.\n";
}

/* set values of dynamic steer */
void DynamicSteerExt::initDynamicSteer() {
  LOG_2 << "init dynamic steer parameter.\n";

  // geometry
  {
    // track width
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_TrckWdth = g_dynamic_steer.steergeometry().steertrackwidth().val();

    // steer angle range
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_StrgRng =
        g_dynamic_steer.steergeometry().steeringwheelanglerange().val();

    // pinion radius table
    tx_car::initMap1D(g_dynamic_steer.steergeometry().pinradiustable(),
                      DynamicSteerExt::DynamicSteer_P.DynamicSteering_StrgAngBpts,
                      DynamicSteerExt::DynamicSteer_P.DynamicSteering_PnnRadiusTbl, m_pinion_radius_maxIndex);

    // rack casing length
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_RckCsLngth =
        g_dynamic_steer.steergeometry().rackcasinglength().val();

    // tie rod length
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_TieRodLngth = g_dynamic_steer.steergeometry().tierodlength().val();

    // arm length
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_StrgArmLngth =
        g_dynamic_steer.steergeometry().steerarmlength().val();

    // arm length
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_D =
        g_dynamic_steer.steergeometry().dist_rackandfrontaxle_longitudinal().val();
  }

  // dynamic
  {
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_J1 = g_dynamic_steer.steerdynamics().steercolumninertia().val();
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_b2 = g_dynamic_steer.steerdynamics().steerwheeldamping().val();

    DynamicSteerExt::DynamicSteer_P.DynamicSteering_J2 = g_dynamic_steer.steerdynamics().steercolumninertia().val();
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_b3 = g_dynamic_steer.steerdynamics().steercolumndamping().val();

    DynamicSteerExt::DynamicSteer_P.DynamicSteering_k1 = g_dynamic_steer.steerdynamics().hysteresisstiffness().val();
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_b1 = g_dynamic_steer.steerdynamics().hysteresisdamping().val();

    DynamicSteerExt::DynamicSteer_P.DynamicSteering_FricTrq = g_dynamic_steer.steerdynamics().steerfrictorque().val();
  }

  // power assist
  {
    tx_car::initAxis(DynamicSteerExt::DynamicSteer_P.DynamicSteering_TrqBpts,
                     g_dynamic_steer.powerassist().powerassisttable().u0_axis());
    tx_car::initAxis(DynamicSteerExt::DynamicSteer_P.DynamicSteering_VehSpdBpts,
                     g_dynamic_steer.powerassist().powerassisttable().u1_axis());
    tx_car::initAxis(DynamicSteerExt::DynamicSteer_P.DynamicSteering_TrqTbl,
                     g_dynamic_steer.powerassist().powerassisttable().y0_axis());

    m_power_assist_trqIn_maxIndex = g_dynamic_steer.powerassist().powerassisttable().u0_axis().data_size() - 1;
    m_power_assist_speed_maxIndex = g_dynamic_steer.powerassist().powerassisttable().u1_axis().data_size() - 1;
    m_power_assist_assisTrq_maxIndex = g_dynamic_steer.powerassist().powerassisttable().y0_axis().data_size() - 1;

    DynamicSteerExt::DynamicSteer_P.TrqAssistTbl_maxIndex[0] = m_power_assist_trqIn_maxIndex;
    DynamicSteerExt::DynamicSteer_P.TrqAssistTbl_maxIndex[1] = m_power_assist_speed_maxIndex;

    DynamicSteerExt::DynamicSteer_P.DynamicSteering_TrqLmt = g_dynamic_steer.powerassist().maxassisttorque().val();
    DynamicSteerExt::DynamicSteer_P.DynamicSteering_PwrLmt = g_dynamic_steer.powerassist().maxassistpower().val();

    if (!tx_car::map2DFormatChecker(g_dynamic_steer.powerassist().powerassisttable())) {
      LOG(ERROR) << "map 2d error. " << g_dynamic_steer.powerassist().powerassisttable().disp_name() << "\n";
      std::abort();
    }
  }

  LOG_2 << "end of init dynamic steer parameter.\n";
}

/* load parameter from json file and init model */
bool DynamicSteerExt::initMDL() {
  m_err_info = {tx_car::TxCarStateCode::no_error, "no error"};

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
}  // namespace steer
}  // namespace tx_car
