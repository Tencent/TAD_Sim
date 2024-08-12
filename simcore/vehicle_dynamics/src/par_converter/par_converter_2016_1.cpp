// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "par_converter_2016_1.h"
#include "inc/car_common.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
/***************************************/
ParConverter_2016_1::ParConverter_2016_1() { LOG_0 << "ParConverter_2016_1 constructed.\n"; }

bool ParConverter_2016_1::convertCarType(const tx_car::CarSimParData& parDataSet, tx_car::CarType& carType,
                                         tx_car::Pow_ECU& ecu) {
  // par data
  tx_car::CarSimParData_ParData parData;

  if (findParDataByKeyword(parDataSet, "OPT_PT", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    int optPT = std::atoi(parData.kvpair().value().c_str());
    if (optPT == 3) {
      carType.set_drivetrain_type(tx_car::CarType_DriveTrain_Type_FourWheel_Drive);
    } else if (optPT == 2) {
      carType.set_drivetrain_type(tx_car::CarType_DriveTrain_Type_RearWheel_Drive);
      auto p = ecu.soft_vcu().vcu_acc_p().val();
      auto i = ecu.soft_vcu().vcu_acc_i().val();
      auto maxTorque = ecu.soft_vcu().motormaxtrq().val();
      ecu.mutable_soft_vcu()->mutable_motormaxtrq()->set_val(maxTorque / 2.0);
      ecu.mutable_soft_vcu()->mutable_vcu_acc_p()->set_val(p / 2);
      ecu.mutable_soft_vcu()->mutable_vcu_acc_i()->set_val(i / 2);
    } else if (optPT == 1) {
      carType.set_drivetrain_type(tx_car::CarType_DriveTrain_Type_FrontWheel_Drive);
      auto p = ecu.soft_vcu().vcu_acc_p().val();
      auto i = ecu.soft_vcu().vcu_acc_i().val();
      auto maxTorque = ecu.soft_vcu().motormaxtrq().val();
      ecu.mutable_soft_vcu()->mutable_motormaxtrq()->set_val(maxTorque / 2.0);
      ecu.mutable_soft_vcu()->mutable_vcu_acc_p()->set_val(p / 2);
      ecu.mutable_soft_vcu()->mutable_vcu_acc_i()->set_val(i / 2);
    } else {
      CONVERSION_LOG_FAIL("unkown OPT_PT type of " << optPT);
      return false;
    }
  }
  return true;
}

bool ParConverter_2016_1::convertSprungMass(const tx_car::CarSimParData& parDataSet, tx_car::SprungMassDD& sprungMass) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // wheel base
  if (findParDataByKeyword(parDataSet, "LX_CG_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_frontaxlepositionfromcg()->set_val(std::atof(parData.kvpair().value().c_str()) * MM_2_M);
    sprungMass.mutable_frontaxlepositionfromcg()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  if (findParDataByKeyword(parDataSet, "LX_AXLE", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_wheelbase()->set_val(std::atof(parData.kvpair().value().c_str()) * MM_2_M);
    sprungMass.mutable_wheelbase()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  if (findParDataByKeyword(parDataSet, "H_CG_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_heightcg()->set_val(std::atof(parData.kvpair().value().c_str()) * MM_2_M -
                                           mCar.wheel_tire().tire().wheel_radius().val());
    sprungMass.mutable_heightcg()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  {
    sprungMass.mutable_rearaxlepositionfromcg()->set_val(sprungMass.wheelbase().val() -
                                                         sprungMass.frontaxlepositionfromcg().val());
    sprungMass.mutable_rearaxlepositionfromcg()->set_data_source(tx_car::CarSim_PAR);
  }

  if (findParDataByKeyword(parDataSet, "L_TRACK", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    sprungMass.mutable_trackwidthfront()->set_val(std::atof(parData.kvpair().value().c_str()) * MM_2_M);
    sprungMass.mutable_trackwidthfront()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  if (findParDataByKeyword(parDataSet, "L_TRACK", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    sprungMass.mutable_trackwidthrear()->set_val(std::atof(parData.kvpair().value().c_str()) * MM_2_M);
    sprungMass.mutable_trackwidthrear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // aerodynamics
  if (findParDataByKeyword(parDataSet, "AREA_AERO", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_frontalarea()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_frontalarea()->set_data_source(tx_car::CarSim_PAR);
  }
  if (findParDataByKeyword(parDataSet, "FX_AERO_SHAPING_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData)) {
    auto index = parData.map1d().y0_axis().data_size() / 2;
    sprungMass.mutable_dragcoefficient()->set_val(parData.map1d().y0_axis().data().at(index));
    sprungMass.mutable_dragcoefficient()->set_data_source(tx_car::CarSim_PAR);
  }

  // inertia
  if (findParDataByKeyword(parDataSet, "IYY_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_pitchmomentinertia()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_pitchmomentinertia()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "IXX_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_rollmomentinertia()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_rollmomentinertia()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "IZZ_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_yawmomentinertia()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_yawmomentinertia()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // mass
  if (findParDataByKeyword(parDataSet, "M_SU", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    sprungMass.mutable_sprungmass()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_sprungmass()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "M_US", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    sprungMass.mutable_unsprungmassfrontaxle()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_unsprungmassfrontaxle()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "M_US", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    sprungMass.mutable_unsprungmassrearaxle()->set_val(std::atof(parData.kvpair().value().c_str()));
    sprungMass.mutable_unsprungmassrearaxle()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  {
    sprungMass.mutable_mass()->set_val(sprungMass.unsprungmassfrontaxle().val() +
                                       sprungMass.unsprungmassrearaxle().val() + sprungMass.sprungmass().val());
    sprungMass.mutable_mass()->set_data_source(tx_car::CarSim_PAR);
  }

  LOG_2 << "sprung mass conversion done.\n";

  return true;
}

bool ParConverter_2016_1::convertBody(const tx_car::CarSimParData& parDataSet, tx_car::VehBodyDD& body) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // aerodynamics
  body.mutable_aerodynamics()->mutable_vehiclebody6dof_cd()->set_val(mCar.sprung_mass().dragcoefficient().val());
  body.mutable_aerodynamics()->mutable_vehiclebody6dof_cd()->set_data_source(tx_car::CarSim_PAR);

  LOG_2 << "body vehicle dynamics conversion done.\n";
  return true;
}

bool ParConverter_2016_1::convertEngine(const tx_car::CarSimParData& parDataSet, tx_car::Engine& engine) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // engine convert, from pedal control to request torque
  if (findParDataByKeyword(parDataSet, "MENGINE_CARPET", tx_car::CarSimParData_DataType_MAP_2D, parData)) {
    // map2d from carsim
    auto map2dCarsim = transposeMap2d(parData.map2d());
    auto& throttle_01 = *map2dCarsim.mutable_u0_axis();
    auto& rpm = *map2dCarsim.mutable_u1_axis();
    auto& torque = *map2dCarsim.mutable_y0_axis();

    // find max torque from y0 of map2d
    double maxTorque = 0.0;
    for (auto i = 0; i < torque.data_size(); ++i) {
      maxTorque = torque.data().at(i) > maxTorque ? torque.data().at(i) : maxTorque;
    }

    // map2d in txcar.json, engine output torque
    copyMap2DData(*engine.mutable_engine_outtrq_map(), map2dCarsim, tx_car::DataSource::CarSim_PAR);
    engine.mutable_engine_outtrq_map()->set_data_source(tx_car::CarSim_PAR);
    engine.mutable_engine_outtrq_map()->mutable_u0_axis()->set_data_source(tx_car::CarSim_PAR);
    engine.mutable_engine_outtrq_map()->mutable_u1_axis()->set_data_source(tx_car::CarSim_PAR);
    engine.mutable_engine_outtrq_map()->mutable_y0_axis()->set_data_source(tx_car::CarSim_PAR);

    printMap2d(engine.engine_outtrq_map());

    if (!tx_car::map2DFormatChecker(engine.engine_outtrq_map())) {
      throw(std::runtime_error("MENGINE_CARPET table format error."));
      return false;
    }

    // map2d in txcar.json, engine fuel rate
    copyMap2DData(*engine.mutable_engine_fuelrate_map(), map2dCarsim, tx_car::DataSource::CarSim_PAR);

    // calculate request torque
    auto torqueAxis = engine.mutable_engine_fuelrate_map()->mutable_u0_axis();
    for (auto i = 0; i < torqueAxis->data_size(); ++i) {
      torqueAxis->mutable_data()->Set(i, torqueAxis->data().at(i) * maxTorque);
    }

    const double fuelRateRatio = 8e-9;

    for (auto c = 0; c < engine.engine_fuelrate_map().u1_axis().data_size(); ++c) {
      double rpm = engine.engine_fuelrate_map().u1_axis().data(c);
      for (auto r = 0; r < engine.engine_fuelrate_map().u0_axis().data_size(); ++r) {
        double val = getValueFromMap2d(engine.engine_fuelrate_map(), r, c);
        double fuelRate = rpm * val * fuelRateRatio;
        fuelRate = fuelRate > 0.0 ? fuelRate : 0.0;
        setValueOfMap2d(*engine.mutable_engine_fuelrate_map(), r, c, fuelRate);
      }
    }
    engine.mutable_engine_outtrq_map()->set_data_source(tx_car::CarSim_PAR);

    // calculate max engine speed
    double maxRpm = tx_car::getMaxValueOf(rpm);
    engine.mutable_maxspd()->set_val(maxRpm);
    engine.mutable_maxspd()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // engine time const
  if (findParDataByKeyword(parDataSet, "TC_THR_APP", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    engine.mutable_tmcst()->set_val(std::atof(parData.kvpair().value().c_str()));
    engine.mutable_tmcst()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  LOG_2 << "engine conversion done.\n";

  return true;
}

bool ParConverter_2016_1::convertDriveLine(const tx_car::CarSimParData& parDataSet, tx_car::DriveLine& driveline) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // max forward gear
  const uint32_t maxFwdGearNum = 8;

  // final drive
  double fd = 2.65, rd = 2.65, finalDrive = rd,
         finalDriveRatio = 1.0;  // 3.2 / 2.65;  // @dhu, special final drive ratio
  bool fd_valid = false, rd_valid = false;
  if (findParDataByKeyword(parDataSet, "R_GEAR_FD", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    fd = std::atof(parData.kvpair().value().c_str()) * finalDriveRatio;
    fd_valid = true;
    driveline.mutable_front_fd()->mutable_ratio_diff_front()->set_val(fd);
    driveline.mutable_front_fd()->mutable_ratio_diff_front()->set_data_source(tx_car::CarSim_PAR);
  }
  if (findParDataByKeyword(parDataSet, "R_GEAR_RD", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    rd = std::atof(parData.kvpair().value().c_str()) * finalDriveRatio;
    rd_valid = true;
    driveline.mutable_rear_fd()->mutable_ratio_diff_rear()->set_val(rd);
    driveline.mutable_rear_fd()->mutable_ratio_diff_rear()->set_data_source(tx_car::CarSim_PAR);
  }

  if (fd_valid && rd_valid) {
    finalDrive = (fd + rd) / 2.0;
  } else if (fd_valid) {
    finalDrive = fd;
  } else if (rd_valid) {
    finalDrive = rd;
  } else {
    LOG_ERROR << "fail to get final drive from carsim .par.\n";
    return false;
  }

  // get gears number
  int nGearsForward = 7;
  if (findParDataByKeyword(parDataSet, "NGEARS", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    nGearsForward = std::atoi(parData.kvpair().value().c_str());
    if (nGearsForward < 1) {
      LOG_ERROR << "forward gear number from carsim is " << nGearsForward << "\n";
      return false;
    }

    nGearsForward = nGearsForward > maxFwdGearNum ? maxFwdGearNum : nGearsForward;
  } else {
    return false;
  }

  // get shift time const
  if (findParDataByKeyword(parDataSet, "T_SHIFT", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    double shiftTau = std::atof(parData.kvpair().value().c_str());
    shiftTau = shiftTau < 1e-4 ? shiftTau : 0.05;
    driveline.mutable_tm()->mutable_gear_shift_tau()->set_val(shiftTau);
    driveline.mutable_tm()->mutable_gear_shift_tau()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // get reverse gear ratio etc
  double reverseGearRatio = -3.42, reverseInertia = 0.034, reverseEffeciency = 0.9;
  if (findParDataByKeyword(parDataSet, "R_GEAR_TR_REVERSE", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    reverseGearRatio = std::atof(parData.kvpair().value().c_str());
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "ITR_REVERSE", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    reverseInertia = std::atof(parData.kvpair().value().c_str());
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "R_EFF_TR_F_REVERSE", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    reverseEffeciency = std::atof(parData.kvpair().value().c_str());
  } else {
    return false;
  }

  // inertia neutral
  double neutralInetia = 0.034;
  if (findParDataByKeyword(parDataSet, "ITR_NEUTRAL", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    neutralInetia = std::atof(parData.kvpair().value().c_str());
  } else {
    return false;
  }

  // get forward gear ratio etc.
  std::string frowardGearKeyPrefix{"R_GEAR_TR"}, frowardInertiaKeyPrefix{"ITR"}, frowardEffKeyPrefix{"R_EFF_TR_F"};
  std::vector<double> forwardGearRatios, forwardGearIntertia, forwardEffeciency;

  forwardGearRatios.reserve(maxFwdGearNum);
  forwardGearIntertia.reserve(maxFwdGearNum);
  forwardEffeciency.reserve(maxFwdGearNum);

  for (auto i = 1; i <= nGearsForward; ++i) {
    if (findParDataByKeyword(parDataSet, frowardGearKeyPrefix + "(" + std::to_string(i) + ")",
                             tx_car::CarSimParData_DataType_KV_Type, parData)) {
      forwardGearRatios.push_back(std::atof(parData.kvpair().value().c_str()));
    } else {
      return false;
    }
    if (findParDataByKeyword(parDataSet, frowardInertiaKeyPrefix + "(" + std::to_string(i) + ")",
                             tx_car::CarSimParData_DataType_KV_Type, parData)) {
      forwardGearIntertia.push_back(std::atof(parData.kvpair().value().c_str()));
    } else {
      return false;
    }
    if (findParDataByKeyword(parDataSet, frowardEffKeyPrefix + "(" + std::to_string(i) + ")",
                             tx_car::CarSimParData_DataType_KV_Type, parData)) {
      forwardEffeciency.push_back(std::atof(parData.kvpair().value().c_str()));
    } else {
      return false;
    }
  }

  // gear ratio table
  {
    auto gearRatioTable = driveline.mutable_tm()->mutable_tr_gear_ratio_table();
    auto u0 = gearRatioTable->mutable_u0_axis();
    auto y0 = gearRatioTable->mutable_y0_axis();

    u0->clear_data();
    y0->clear_data();

    for (int i = -1; i <= nGearsForward; ++i) {
      u0->add_data(i);
    }

    y0->add_data(reverseGearRatio);                // reverse
    y0->add_data(forwardGearRatios.front());       // neutral
    for (const auto& ratio : forwardGearRatios) {  // forward
      y0->add_data(ratio);
    }

    gearRatioTable->set_data_source(tx_car::CarSim_PAR);
    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
  }

  // gear inertia table
  {
    auto gearInertiaTable = driveline.mutable_tm()->mutable_tr_gear_num_inertias_table();
    auto u0 = gearInertiaTable->mutable_u0_axis();
    auto y0 = gearInertiaTable->mutable_y0_axis();

    u0->clear_data();
    y0->clear_data();

    for (int i = -1; i <= nGearsForward; ++i) {
      u0->add_data(i);
    }

    y0->add_data(reverseInertia);                      // reverse
    y0->add_data(neutralInetia);                       // neutral
    for (const auto& inertia : forwardGearIntertia) {  // forward
      y0->add_data(inertia);
    }

    gearInertiaTable->set_data_source(tx_car::CarSim_PAR);
    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
  }

  // gear efficiency table
  {
    auto gearEfficiencyTable = driveline.mutable_tm()->mutable_tr_gear_num_efficien_table();
    auto u0 = gearEfficiencyTable->mutable_u0_axis();
    auto y0 = gearEfficiencyTable->mutable_y0_axis();

    u0->clear_data();
    y0->clear_data();

    for (int i = -1; i <= nGearsForward; ++i) {
      u0->add_data(i);
    }

    y0->add_data(reverseEffeciency);             // reverse
    y0->add_data(forwardEffeciency.front());     // neutral
    for (const auto& eff : forwardEffeciency) {  // forward
      y0->add_data(eff);
    }

    gearEfficiencyTable->set_data_source(tx_car::CarSim_PAR);
    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
  }

  // gear damping table
  {
    auto gearDampingTable = driveline.mutable_tm()->mutable_tr_gear_num_damping_coeffs_table();
    auto u0 = gearDampingTable->mutable_u0_axis();
    auto y0 = gearDampingTable->mutable_y0_axis();
    auto y0Copy = gearDampingTable->y0_axis();

    u0->clear_data();
    y0->clear_data();

    for (int i = -1; i <= nGearsForward; ++i) {
      u0->add_data(i);
      y0->add_data(y0Copy.data().at(i + 1));
    }

    gearDampingTable->set_data_source(tx_car::CarSim_PAR);
    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
  }

  // tcu upshift table
  std::vector<tx_car::CarSimParData_ParData> upshift_s;
  if (!findAllParDataByKeyword(parDataSet, "UPSHIFT_TRANS_TABLE", tx_car::CarSimParData_DataType_MAP_1D, upshift_s)) {
    LOG_ERROR << "no DOWNSHIFT_TRANS_TABLE in carsim .par.\n";
    return false;
  }

  if (upshift_s.size() >= 1) {
    const auto& firstUpshift = upshift_s.front();

    auto upshiftTable = driveline.mutable_tcu()->mutable_muc_up_shift_spd_table();
    auto upshift_u0 = upshiftTable->mutable_u0_axis();
    auto upshift_u1 = upshiftTable->mutable_u1_axis();
    auto upshift_y0 = upshiftTable->mutable_y0_axis();

    upshift_u0->clear_data();
    upshift_u1->clear_data();
    upshift_y0->clear_data();

    // copy throttle
    upshift_u0->mutable_data()->CopyFrom(firstUpshift.map1d().u0_axis().data());

    // copy gear
    for (auto i = 1; i < nGearsForward; ++i) {
      upshift_u1->add_data(i);
    }

    // copy speed
    const auto& axle1Size = upshift_u0->data_size();  // throttle
    double wheelRadius = mCar.wheel_tire().tire().wheel_radius().val();

    for (auto i = 0; i < upshift_s.size(); ++i) {
      const auto& upshift = upshift_s.at(i);
      const auto& gearRatio = forwardGearRatios.at(i);
      for (const auto& rpm : upshift.map1d().y0_axis().data()) {
        upshift_y0->add_data(rpm * RPM_2_RADS / gearRatio / finalDrive * wheelRadius * MS_2_KPH);
      }
    }

    upshiftTable->set_data_source(tx_car::CarSim_PAR);
    upshift_u0->set_data_source(tx_car::CarSim_PAR);
    upshift_u1->set_data_source(tx_car::CarSim_PAR);
    upshift_y0->set_data_source(tx_car::CarSim_PAR);
  }

  // tcu downshift table
  std::vector<tx_car::CarSimParData_ParData> downshift_s;
  if (!findAllParDataByKeyword(parDataSet, "DOWNSHIFT_TRANS_TABLE", tx_car::CarSimParData_DataType_MAP_1D,
                               downshift_s)) {
    LOG_ERROR << "no DOWNSHIFT_TRANS_TABLE in carsim .par.\n";
    return false;
  }

  if (downshift_s.size() >= 1) {
    const auto& firstDownshift = downshift_s.front();

    auto downshiftTable = driveline.mutable_tcu()->mutable_muc_down_shift_spd_table();
    auto downshift_u0 = downshiftTable->mutable_u0_axis();
    auto downshift_u1 = downshiftTable->mutable_u1_axis();
    auto downshift_y0 = downshiftTable->mutable_y0_axis();

    downshift_u0->clear_data();
    downshift_u1->clear_data();
    downshift_y0->clear_data();

    // copy throttle
    downshift_u0->mutable_data()->CopyFrom(firstDownshift.map1d().u0_axis().data());

    // copy gear
    for (auto i = 2; i <= nGearsForward; ++i) {
      downshift_u1->add_data(i);
    }

    // copy speed
    const auto& axle1Size = downshift_u0->data_size();  // throttle
    double wheelRadius = mCar.wheel_tire().tire().wheel_radius().val();

    for (auto i = 0; i < downshift_s.size(); ++i) {
      const auto& downshift = downshift_s.at(i);
      const auto& gearRatio = forwardGearRatios.at(i + 1);
      for (const auto& rpm : downshift.map1d().y0_axis().data()) {
        downshift_y0->add_data(rpm * RPM_2_RADS / gearRatio / finalDrive * wheelRadius * MS_2_KPH);
      }
    }

    downshiftTable->set_data_source(tx_car::CarSim_PAR);
    downshift_u0->set_data_source(tx_car::CarSim_PAR);
    downshift_u1->set_data_source(tx_car::CarSim_PAR);
    downshift_y0->set_data_source(tx_car::CarSim_PAR);
  }

  return true;
}

bool ParConverter_2016_1::convertDynamicSteer(const tx_car::CarSimParData& carsimParData,
                                              tx_car::DynamicSteer& dynamicSteer) {
  return true;
}

bool ParConverter_2016_1::convertSteerMapped(const tx_car::CarSimParData& parDataSet,
                                             tx_car::SteerMapped& steerMapped) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // steering wheel angle
  if (findParDataByKeyword(parDataSet, "RACK_TRAVEL_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData)) {
    /*
    auto u0 = steerMapped.mutable_rackdisp_vs_steerang()->mutable_u0_axis();
    auto y0 = steerMapped.mutable_rackdisp_vs_steerang()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * DEG_2_RAD);
    }
    */
    copyMap1DData(*steerMapped.mutable_rackdisp_vs_steerang(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  DEG_2_RAD);
    steerMapped.mutable_rackdisp_vs_steerang()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(steerMapped.rackdisp_vs_steerang())) {
      throw(std::runtime_error("RACK_TRAVEL_TABLE table format error."));
      return false;
    }
  } else {
    return false;
  }

  // left and right wheel angle
  if (findParDataByKeyword(parDataSet, "RACK_KIN_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_No_Axle, tx_car::CarSimParData_Side_SIDE_1)) {
    /*
    auto u0 = steerMapped.mutable_leftwhl_vs_rackdisp()->mutable_u0_axis();
    auto y0 = steerMapped.mutable_leftwhl_vs_rackdisp()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < y0->data_size(); ++i) {
            y0->set_data(i, y0->data().at(i) * DEG_2_RAD);
    }
    */
    copyMap1DData(*steerMapped.mutable_leftwhl_vs_rackdisp(), parData.map1d(), tx_car::DataSource::CarSim_PAR, 1.0,
                  DEG_2_RAD);
    steerMapped.mutable_leftwhl_vs_rackdisp()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(steerMapped.leftwhl_vs_rackdisp())) {
      throw(std::runtime_error("RACK_KIN_TABLE side 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "RACK_KIN_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_No_Axle, tx_car::CarSimParData_Side_SIDE_2)) {
    /*
    auto u0 = steerMapped.mutable_rightwhl_vs_rackdisp()->mutable_u0_axis();
    auto y0 = steerMapped.mutable_rightwhl_vs_rackdisp()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < y0->data_size(); ++i) {
            y0->set_data(i, y0->data().at(i) * DEG_2_RAD);
    }
    */
    copyMap1DData(*steerMapped.mutable_rightwhl_vs_rackdisp(), parData.map1d(), tx_car::DataSource::CarSim_PAR, 1.0,
                  DEG_2_RAD);
    steerMapped.mutable_rightwhl_vs_rackdisp()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(steerMapped.rightwhl_vs_rackdisp())) {
      throw(std::runtime_error("RACK_KIN_TABLE side 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  LOG_2 << "steer mapped conversion done.\n";

  return true;
}

bool ParConverter_2016_1::convertSuspension(const tx_car::CarSimParData& parDataSet, tx_car::SuspensionDD& susp) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // dive
  if (findParDataByKeyword(parDataSet, "SUSP_DIVE_AXLE_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_bumpcaster()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_bumpcaster()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * MM_2_M);
    }
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_bumpcaster(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  MM_2_M);
    susp.mutable_bounce()->mutable_bumpcaster()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().bumpcaster())) {
      throw(std::runtime_error("SUSP_DIVE_AXLE_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // x vs bounce
  if (findParDataByKeyword(parDataSet, "SUSP_X_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_lngwhlctrdisp_front()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_lngwhlctrdisp_front()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_lngwhlctrdisp_front(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_bounce()->mutable_lngwhlctrdisp_front()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().lngwhlctrdisp_front())) {
      throw(std::runtime_error("SUSP_X_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "SUSP_X_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_lngwhlctrdisp_rear()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_lngwhlctrdisp_rear()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_lngwhlctrdisp_rear(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_bounce()->mutable_lngwhlctrdisp_rear()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().lngwhlctrdisp_rear())) {
      throw(std::runtime_error("SUSP_X_TABLE axle 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // camber vs bounce
  if (findParDataByKeyword(parDataSet, "CAMBER_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_bumpcamber_front()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_bumpcamber_front()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * MM_2_M);
    }
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_bumpcamber_front(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  MM_2_M);
    susp.mutable_bounce()->mutable_bumpcamber_front()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().bumpcamber_front())) {
      throw(std::runtime_error("CAMBER_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CAMBER_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_bumpcamber_rear()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_bumpcamber_rear()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * MM_2_M);
    }
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_bumpcamber_rear(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  MM_2_M);
    susp.mutable_bounce()->mutable_bumpcamber_rear()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().bumpcamber_rear())) {
      throw(std::runtime_error("CAMBER_TABLE axle 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // y vs bounce
  if (findParDataByKeyword(parDataSet, "SUSP_LAT_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_latwhlctrdisplacement_front()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_latwhlctrdisplacement_front()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_latwhlctrdisplacement_front(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_bounce()->mutable_latwhlctrdisplacement_front()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().latwhlctrdisplacement_front())) {
      throw(std::runtime_error("SUSP_LAT_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "SUSP_LAT_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_latwhlctrdisplacement_rear()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_latwhlctrdisplacement_rear()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_latwhlctrdisplacement_rear(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_bounce()->mutable_latwhlctrdisplacement_rear()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().latwhlctrdisplacement_rear())) {
      throw(std::runtime_error("SUSP_LAT_TABLE axle 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // toe vs bounce
  if (findParDataByKeyword(parDataSet, "TOE_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_bumptoe_front()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_bumptoe_front()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * MM_2_M);
    }
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_bumptoe_front(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  MM_2_M);
    susp.mutable_bounce()->mutable_bumptoe_front()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().bumptoe_front())) {
      throw(std::runtime_error("TOE_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "TOE_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    /*
    auto u0 = susp.mutable_bounce()->mutable_bumptoe_rear()->mutable_u0_axis();
    auto y0 = susp.mutable_bounce()->mutable_bumptoe_rear()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);

    // unit conversion
    for (auto i = 0; i < u0->data_size(); ++i) {
            u0->set_data(i, u0->data().at(i) * MM_2_M);
    }
    */
    copyMap1DData(*susp.mutable_bounce()->mutable_bumptoe_rear(), parData.map1d(), tx_car::DataSource::CarSim_PAR,
                  MM_2_M);
    susp.mutable_bounce()->mutable_bumptoe_rear()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.bounce().bumptoe_rear())) {
      throw(std::runtime_error("TOE_TABLE axle 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // suspension rates
  if (findParDataByKeyword(parDataSet, "FS_COMP_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_bounce()->mutable_nrmlwhlrates_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_bounce()->mutable_nrmlwhlrates_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "FS_COMP_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_bounce()->mutable_nrmlwhlrates_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_bounce()->mutable_nrmlwhlrates_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // static toe
  if (findParDataByKeyword(parDataSet, "A_TOE", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_static_()->mutable_statictoe_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_static_()->mutable_statictoe_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "A_TOE", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_static_()->mutable_statictoe_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_static_()->mutable_statictoe_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // static camber
  if (findParDataByKeyword(parDataSet, "A_CAMBER", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_static_()->mutable_staticcamber_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_static_()->mutable_staticcamber_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "A_CAMBER", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_static_()->mutable_staticcamber_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_static_()->mutable_staticcamber_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // shock
  if (findParDataByKeyword(parDataSet, "FD_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    /*
    auto u0 = susp.mutable_shock()->mutable_shockdampingforce_front()->mutable_u0_axis();
    auto y0 = susp.mutable_shock()->mutable_shockdampingforce_front()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_shock()->mutable_shockdampingforce_front(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_shock()->mutable_shockdampingforce_front()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.shock().shockdampingforce_front())) {
      throw(std::runtime_error("FD_TABLE axle 1 table format error."));
      return false;
    }
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "FD_TABLE", tx_car::CarSimParData_DataType_MAP_1D, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    /*
    auto u0 = susp.mutable_shock()->mutable_shockdampingforce_rear()->mutable_u0_axis();
    auto y0 = susp.mutable_shock()->mutable_shockdampingforce_rear()->mutable_y0_axis();

    u0->mutable_data()->CopyFrom(parData.map1d().u0_axis().data());
    y0->mutable_data()->CopyFrom(parData.map1d().y0_axis().data());

    u0->set_data_source(tx_car::CarSim_PAR);
    y0->set_data_source(tx_car::CarSim_PAR);
    */
    copyMap1DData(*susp.mutable_shock()->mutable_shockdampingforce_rear(), parData.map1d(),
                  tx_car::DataSource::CarSim_PAR);
    susp.mutable_shock()->mutable_shockdampingforce_rear()->set_data_source(tx_car::CarSim_PAR);

    if (!tx_car::map1DFormatChecker(susp.shock().shockdampingforce_rear())) {
      throw(std::runtime_error("FD_TABLE axle 2 table format error."));
      return false;
    }
  } else {
    return false;
  }

  // motion ratio
  if (findParDataByKeyword(parDataSet, "CMP_SPR_SEAT_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_shock()->mutable_jounceratio_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_shock()->mutable_jounceratio_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CMP_SPR_SEAT_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_shock()->mutable_jounceratio_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_shock()->mutable_jounceratio_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // roll stiffness
  if (findParDataByKeyword(parDataSet, "MX_AUX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_roll()->mutable_rollstiff_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_roll()->mutable_rollstiff_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "MX_AUX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_roll()->mutable_rollstiff_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    susp.mutable_roll()->mutable_rollstiff_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // toe vs fx
  if (findParDataByKeyword(parDataSet, "CT_FX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_longitudinal()->mutable_toecomplianceperfx_front()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_toecomplianceperfx_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CT_FX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_longitudinal()->mutable_toecomplianceperfx_rear()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_toecomplianceperfx_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // steer vs fy
  if (findParDataByKeyword(parDataSet, "CS_FY_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_lateral()->mutable_toecomplianceperfy_front()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                        N_2_KN);
    susp.mutable_lateral()->mutable_toecomplianceperfy_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CS_FY_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_lateral()->mutable_toecomplianceperfy_rear()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                       N_2_KN);
    susp.mutable_lateral()->mutable_toecomplianceperfy_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // steer vs mz
  if (findParDataByKeyword(parDataSet, "CS_MZ_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_align()->mutable_aligntrqsteer_front()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                 Nm_2_KNm);
    susp.mutable_align()->mutable_aligntrqsteer_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CS_MZ_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_align()->mutable_aligntrqsteer_rear()->set_val(std::atof(parData.kvpair().value().c_str()) * Nm_2_KNm);
    susp.mutable_align()->mutable_aligntrqsteer_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // camber vs fx
  if (findParDataByKeyword(parDataSet, "CC_FX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_longitudinal()->mutable_cambercomplianceperfx_front()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_cambercomplianceperfx_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CC_FX_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_longitudinal()->mutable_cambercomplianceperfx_rear()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_cambercomplianceperfx_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // inclication vs fy
  if (findParDataByKeyword(parDataSet, "CI_FY_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_lateral()->mutable_cambercomplianceperfy_front()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                           N_2_KN);
    susp.mutable_lateral()->mutable_cambercomplianceperfy_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CI_FY_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_lateral()->mutable_cambercomplianceperfy_rear()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                          N_2_KN);
    susp.mutable_lateral()->mutable_cambercomplianceperfy_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // inclication vs mz
  if (findParDataByKeyword(parDataSet, "CI_MZ_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_align()->mutable_aligntrqcamber_front()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                  Nm_2_KNm);
    susp.mutable_align()->mutable_aligntrqcamber_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "CI_MZ_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_align()->mutable_aligntrqcamber_rear()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                 Nm_2_KNm);
    susp.mutable_align()->mutable_aligntrqcamber_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // long vs fx
  if (findParDataByKeyword(parDataSet, "C_LONG_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_longitudinal()->mutable_whldispcomplianceperfx_front()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_whldispcomplianceperfx_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "C_LONG_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_longitudinal()->mutable_whldispcomplianceperfx_rear()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_longitudinal()->mutable_whldispcomplianceperfx_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // lat vs fy
  if (findParDataByKeyword(parDataSet, "C_LAT_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1)) {
    susp.mutable_lateral()->mutable_whldispcomplianceperfy_front()->set_val(
        std::atof(parData.kvpair().value().c_str()) * N_2_KN);
    susp.mutable_lateral()->mutable_whldispcomplianceperfy_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "C_LAT_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2)) {
    susp.mutable_lateral()->mutable_whldispcomplianceperfy_rear()->set_val(std::atof(parData.kvpair().value().c_str()) *
                                                                           N_2_KN);
    susp.mutable_lateral()->mutable_whldispcomplianceperfy_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  LOG_2 << "suspension conversion done.\n";

  return true;
}

bool ParConverter_2016_1::convertWheelTire(const tx_car::CarSimParData& parDataSet, tx_car::WheelTireDD& wheelTire) {
  // par data
  tx_car::CarSimParData_ParData parData;

  // fluid time const for brake
  if (findParDataByKeyword(parDataSet, "TC_L1", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    wheelTire.mutable_brake()->mutable_const_fluid_time_front()->set_val(std::atof(parData.kvpair().value().c_str()));
    wheelTire.mutable_brake()->mutable_const_fluid_time_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "TC_L2", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    wheelTire.mutable_brake()->mutable_const_fluid_time_rear()->set_val(std::atof(parData.kvpair().value().c_str()));
    wheelTire.mutable_brake()->mutable_const_fluid_time_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "TL_L1", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    wheelTire.mutable_brake()->mutable_const_fluid_transport_time_front()->set_val(
        std::atof(parData.kvpair().value().c_str()));
    wheelTire.mutable_brake()->mutable_const_fluid_transport_time_front()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "TL_L2", tx_car::CarSimParData_DataType_KV_Type, parData)) {
    wheelTire.mutable_brake()->mutable_const_fluid_transport_time_rear()->set_val(
        std::atof(parData.kvpair().value().c_str()));
    wheelTire.mutable_brake()->mutable_const_fluid_transport_time_rear()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // rear brake
  double torquePerBar_Front = 350 * Bar_2_MPa;
  double torquePerBar_Rear = 150 * Bar_2_MPa;

  if (findParDataByKeyword(parDataSet, "MY_BRAKE_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_1, tx_car::CarSimParData_Side_SIDE_1)) {
    torquePerBar_Front = std::atof(parData.kvpair().value().c_str()) * Bar_2_MPa;

    auto rows = wheelTire.brake().brakemap().u0_axis().data_size();    // bar
    auto cols = wheelTire.brake().brakemap().u1_axis().data_size();    // speed, unit rpm
    auto y0Size = wheelTire.brake().brakemap().y0_axis().data_size();  // brake torque

    if (rows * cols != y0Size) {
      throw(std::runtime_error("brake map table size error."));
      return false;
    }

    auto y0 = wheelTire.mutable_brake()->mutable_brakemap()->mutable_y0_axis();

    for (auto c = 0; c < cols; c++) {
      for (auto r = 0; r < rows; ++r) {
        double currentBar = wheelTire.brake().brakemap().u0_axis().data().at(r);
        setValueOfMap2d(*wheelTire.mutable_brake()->mutable_brakemap(), r, c, torquePerBar_Front * currentBar);
      }
    }

    wheelTire.mutable_brake()->mutable_brakemap()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }
  if (findParDataByKeyword(parDataSet, "MY_BRAKE_COEFFICIENT", tx_car::CarSimParData_DataType_KV_Type, parData,
                           tx_car::CarSimParData_Axle_AXLE_2, tx_car::CarSimParData_Side_SIDE_1)) {
    torquePerBar_Rear = std::atof(parData.kvpair().value().c_str()) * Bar_2_MPa;
    wheelTire.mutable_brake()->mutable_rear_brk_pressure_ratio()->set_val(torquePerBar_Rear / torquePerBar_Front);
    wheelTire.mutable_brake()->mutable_rear_brk_pressure_ratio()->set_data_source(tx_car::CarSim_PAR);
  } else {
    return false;
  }

  // wheel radius, since txcar.json use .tir parameter, ignore tire data from carsim
  /*
  if (findParDataByKeyword(parDataSet, "RRE", tx_car::CarSimParData_DataType_KV_Type, parData)) {
          wheelTire.mutable_tire()->mutable_wheel_radius()->set_val(std::atof(parData.kvpair().value().c_str()) *
  MM_2_M); wheelTire.mutable_tire()->mutable_wheel_radius()->set_data_source(tx_car::CarSim_PAR);
  }
  else {
          return false;
  }
  */

  LOG_2 << "brake conversion done.\n";

  return true;
}
}  // namespace tx_car
