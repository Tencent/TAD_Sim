// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/writer_driveline.h"
#include "inc/proto_helper.h"

DrivelineParamWriter::DrivelineParamWriter() {}

void DrivelineParamWriter::fillDriveline(tx_car::car &m_car) {
  auto driveline = m_car.mutable_driveline();

  // RealScalar motor_gear_ratio =3; //gear ratio for motor drive type without fd ratio
  // tx_car::setRealScalar(driveline->mutable_front_motor_ratio()->mutable_front_motor_gear_ratio(),"motor gear
  // ratio","front gear ratio for motor drive type without fd ratio","null",3.7);
  // tx_car::setRealScalar(driveline->mutable_rear_motor_ratio()->mutable_rear_motor_gear_ratio(),"motor gear
  // ratio","rear gear ratio for motor drive type without fd ratio","null",3.7);
  tx_car::setRealScalar(driveline->mutable_front_motor_ratio()->mutable_front_motor_gear_ratio(), "motor gear ratio",
                        "front gear ratio for motor drive type without fd ratio", "null", 1);
  tx_car::setRealScalar(driveline->mutable_rear_motor_ratio()->mutable_rear_motor_gear_ratio(), "motor gear ratio",
                        "rear gear ratio for motor drive type without fd ratio", "null", 1);

  // MCU parameter
  // Int32Scalar gear_init_num = 4; // Initial gear, GearInit  1
  auto tcu = driveline->mutable_tcu();
  tx_car::setInt32Scalar(tcu->mutable_gear_init_num(), "Initial gear number", "", "null", 0);

  real_T upgearNum[] = {1, 2, 3, 4, 5, 6, 7};
  real_T upAccLoad[] = {0, 0.4, 0.75, 1};
  real_T muc_up_shift_spd_table[] = {20.0, 25.0, 30.0,  45.0,  35.0, 45.0, 40.0,  55.0, 40.0, 55.0,
                                     55.0, 65.0, 50.0,  60.0,  65.0, 75.0, 60.0,  65.0, 80.0, 90.0,
                                     70.0, 75.0, 120.0, 150.0, 85.0, 85.0, 150.0, 185.0};
  auto upShitTable = tcu->mutable_muc_up_shift_spd_table();
  upShitTable->set_comment(
      "TCU gear box downshift map table,u0 axis is acc load[0-1], u1 axis is current gear num,  y0 axis is vehicle "
      "speed[km/h]");
  upShitTable->set_disp_name("Gear box upshift table");
  auto uoAxis = upShitTable->mutable_u0_axis();
  auto u1Axis = upShitTable->mutable_u1_axis();
  auto y0Axis = upShitTable->mutable_y0_axis();

  for (size_t i = 0; i < 4; i++) uoAxis->add_data(upAccLoad[i]);
  uoAxis->set_unit("null");
  uoAxis->set_disp_name("mcu upshit table acc load");
  uoAxis->set_comment("value should between 0-1");
  for (size_t i = 0; i < 7; i++) u1Axis->add_data(upgearNum[i]);
  u1Axis->set_unit("null");
  u1Axis->set_disp_name("mcu upshit table gear state (current gear num such as 1,2...)");
  u1Axis->set_comment("gear number should be strictly int number such as 1,2,3,4...");
  for (size_t i = 0; i < 28; i++) y0Axis->add_data(muc_up_shift_spd_table[i]);
  y0Axis->set_unit("km/h");
  y0Axis->set_disp_name("vehicle speed[km/h]");
  y0Axis->set_comment("vehicle speed[km/h] not engine speed or wheel speed");

  // Real2DMap muc_down_shift_spd_table=8; //Downshift speeds, DnShSpd [speed unit]: [10 15 25 30 40 55 75; 10 18 25 35
  // 50 65 80; 12 18 21 55 65 75 85; 15 20 25 55 70 80 95]
  real_T downgearNum[] = {2, 3, 4, 5, 6, 7, 8};
  real_T downAccLoad[] = {0, 0.4, 0.75, 1};
  real_T muc_down_shift_spd_table[] = {10.0, 10.0, 12.0, 15.0, 15.0, 18.0, 18.0, 20.0, 25.0, 25.0,
                                       21.0, 25.0, 30.0, 35.0, 55.0, 55.0, 40.0, 50.0, 65.0, 70.0,
                                       55.0, 65.0, 75.0, 80.0, 75.0, 80.0, 85.0, 95.0};
  auto downShitTable = tcu->mutable_muc_down_shift_spd_table();
  downShitTable->set_comment(
      "TCU gear box downshift map table,u0 axis is acc load[0-1], u1 axis is current gear num,  y0 axis is vehicle "
      "speed[km/h]");
  downShitTable->set_disp_name("Gear box downshift table");
  uoAxis = downShitTable->mutable_u0_axis();
  u1Axis = downShitTable->mutable_u1_axis();
  y0Axis = downShitTable->mutable_y0_axis();

  for (size_t i = 0; i < 4; i++) uoAxis->add_data(downAccLoad[i]);
  uoAxis->set_unit("N/A");
  uoAxis->set_disp_name("mcu downshit table acc load");
  uoAxis->set_comment("value should between 0-1");
  for (size_t i = 0; i < 7; i++) u1Axis->add_data(downgearNum[i]);
  u1Axis->set_unit("N/A");
  u1Axis->set_disp_name("mcu downshit table gear state (current gear num such as 1,2...)");
  u1Axis->set_comment("gear number should be strictly int number such as 1,2,3,4...");
  for (size_t i = 0; i < 28; i++) y0Axis->add_data(muc_down_shift_spd_table[i]);
  y0Axis->set_unit("km/h");
  y0Axis->set_disp_name("vehicle speed[km/h]");
  y0Axis->set_comment("vehicle speed[km/h] not engine speed or wheel speed");

  // RealScalar t_cluth_down=9; //Time required to upshift, tClutchUp [s]
  tx_car::setRealScalar(tcu->mutable_t_cluth_down(), "Time required to upshift",
                        "Time required to upshift, tClutchUp [s]", "s", 1e-3);
  // RealScalar t_cluth_up=10; //Time required to downshift, tClutchDn [s]
  tx_car::setRealScalar(tcu->mutable_t_cluth_up(), "Time required to downshift",
                        "Time required to downshift, tClutchDn [s]", "s", 1e-3);

  // //AT parameters
  auto tm = driveline->mutable_tm();
  real_T gearNum[] = {-1, 0, 1, 2, 3, 4, 5, 6, 7, 8};
  real_T tr_gear_ratio_table[] = {-3.8200, 4.5600, 4.5600, 2.9700, 2.0800, 1.69, 1.27, 1.0, 0.85, 0.65};
  real_T tr_gear_num_efficien_table[] = {0.9, 0.9, 0.92, 0.92, 0.95, 0.95, 0.98, 0.99, 0.99, 0.99};
  real_T tr_gear_num_damping_coeffs_table[] = {0.003, 0.001, 0.003, 0.0025, 0.002, 0.001, 0.001, 0.001, 0.001, 0.001};
  real_T tr_gear_num_inertias_table[] = {0.034, 0.034, 0.037, 0.034, 0.042, 0.04, 0.04, 0.04, 0.04, 0.04};

  auto gearRatioTable = tm->mutable_tr_gear_ratio_table();
  gearRatioTable->set_comment(
      "the gear number of gear ratio, gear eff,"
      "gear damping and gear inertia table should be same with each other!!");
  gearRatioTable->set_disp_name("Gear box gear ratio table");
  auto u0Axis = gearRatioTable->mutable_u0_axis();
  u0Axis->set_unit("N/A");
  u0Axis->set_disp_name("gear number");
  u0Axis->set_comment("gear number should be strictly int number such as-1,0,1,2,3,4...");
  auto y0_Axis = gearRatioTable->mutable_y0_axis();
  y0_Axis->set_unit("N/A");
  y0_Axis->set_disp_name("gear ratio");
  for (size_t i = 0; i < 10; i++) {
    u0Axis->add_data(gearNum[i]);
    y0_Axis->add_data(tr_gear_ratio_table[i]);
  }

  auto gearEffTable = tm->mutable_tr_gear_num_efficien_table();
  gearEffTable->set_comment(
      "the gear number of gear ratio, gear eff,"
      "gear damping and gear inertia table should be same with each other!!");
  gearEffTable->set_disp_name("Gear box gear efficient table");
  u0Axis = gearEffTable->mutable_u0_axis();
  u0Axis->set_unit("null");
  u0Axis->set_disp_name("gear number");
  u0Axis->set_comment("gear number should be strictly int number such as-1,0,1,2,3,4...");
  y0_Axis = gearEffTable->mutable_y0_axis();
  y0_Axis->set_unit("null");
  y0_Axis->set_disp_name("gear efficient 0-1");
  for (size_t i = 0; i < 10; i++) {
    u0Axis->add_data(gearNum[i]);
    y0_Axis->add_data(tr_gear_num_efficien_table[i]);
  }

  auto gearDampleTable = tm->mutable_tr_gear_num_damping_coeffs_table();
  gearDampleTable->set_comment(
      "the gear number of gear ratio, gear eff,"
      "gear damping and gear inertia table should be same with each other!!");
  gearDampleTable->set_disp_name("Gear box gear damping table");
  u0Axis = gearDampleTable->mutable_u0_axis();
  u0Axis->set_unit("null");
  u0Axis->set_disp_name("gear number");
  u0Axis->set_comment("gear number should be strictly int number such as-1,0,1,2,3,4...");
  y0_Axis = gearDampleTable->mutable_y0_axis();
  y0_Axis->set_unit("N*m*s/rad");
  y0_Axis->set_disp_name("Damping vector, bout [N*m*s/rad]");
  for (size_t i = 0; i < 10; i++) {
    u0Axis->add_data(gearNum[i]);
    y0_Axis->add_data(tr_gear_num_damping_coeffs_table[i]);
  }

  auto gearInertiasTable = tm->mutable_tr_gear_num_inertias_table();
  gearInertiasTable->set_comment(
      "the gear number of gear ratio, gear eff,"
      "gear damping and gear inertia table should be same with each other!!");
  gearInertiasTable->set_disp_name("Gear box gear inertia table");
  u0Axis = gearInertiasTable->mutable_u0_axis();
  u0Axis->set_unit("N/A");
  u0Axis->set_disp_name("gear number");
  u0Axis->set_comment("gear number should be strictly int number such as-1,0,1,2,3,4...");
  y0_Axis = gearInertiasTable->mutable_y0_axis();
  y0_Axis->set_unit("kg*m^2");
  y0_Axis->set_disp_name("Inertia vector, Jout [kg*m^2]");
  for (size_t i = 0; i < 10; i++) {
    u0Axis->add_data(gearNum[i]);
    y0_Axis->add_data(tr_gear_num_inertias_table[i]);
  }

  // RealScalar gear_shift_tau=13; //Shift time constant, tau_s [s]:
  tx_car::setRealScalar(tm->mutable_gear_shift_tau(), "Shift time constant", "Shift time constant, tau_s [s]", "s",
                        0.2);

  // Final Diff Paramters
  // RealScalar ratio_diff_front=19; // front differential gear ratio  driveshaft ratio (NC/ND), Ndiff []: 2.77
  tx_car::setRealScalar(driveline->mutable_front_fd()->mutable_ratio_diff_front(), "front differential gear ratio",
                        "front differential gear ratio  driveshaft ratio (NC/ND), Ndiff []:", "N/A", 2.65);
  // RealScalar ratio_diff_rear=20; // rear differential gear ratio    driveshaft ratio (NC/ND), Ndiff []: 2.77
  tx_car::setRealScalar(driveline->mutable_rear_fd()->mutable_ratio_diff_rear(), "rear differential gear ratio",
                        "rear differential gear ratio    driveshaft ratio (NC/ND), Ndiff []", "N/A", 2.65);
}
