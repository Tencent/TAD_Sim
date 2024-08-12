// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/writer_susp.h"
#include "inc/proto_helper.h"

void SuspWriter::fillSusp(tx_car::car &m_car) {
  tx_car::SuspensionDD *susp = m_car.mutable_susp();
  susp->Clear();

  // shock
  {
    real_T front_table[12] = {-5008.0, -3436.0, -2324.0, -1792.0, -1008.0, -228.0,
                              228.0,   596.0,   784.0,   1100.0,  1796.0,  2560.0};
    real_T front_bp[12] = {-1410.0, -720.0, -390.0, -210.0, -90.0, -20.0, 20.0, 90.0, 200.0, 390.0, 760.0, 1160.0};
    real_T rear_table[12] = {-5008.0, -3436.0, -2324.0, -1792.0, -1008.0, -228.0,
                             228.0,   596.0,   784.0,   1100.0,  1796.0,  2560.0};
    real_T rear_bp[12] = {-1410.0, -720.0, -390.0, -210.0, -90.0, -20.0, 20.0, 90.0, 200.0, 390.0, 760.0, 1160.0};
    tx_car::set1DMapInfo(susp->mutable_shock()->mutable_shockdampingforce_front(),
                         "Front Shock force vs shock compression rate", "");
    tx_car::setRealAxis(susp->mutable_shock()->mutable_shockdampingforce_front()->mutable_u0_axis(), "Compression_Rate",
                        "Shock compression rate", "mm/s", front_bp, 12);
    tx_car::setRealAxis(susp->mutable_shock()->mutable_shockdampingforce_front()->mutable_y0_axis(), "Shock_Force",
                        "Shock force", "N", front_table, 12);

    tx_car::set1DMapInfo(susp->mutable_shock()->mutable_shockdampingforce_rear(),
                         "Rear Shock force vs shock compression rate", "");
    tx_car::setRealAxis(susp->mutable_shock()->mutable_shockdampingforce_rear()->mutable_u0_axis(), "Compression_Rate",
                        "Shock compression rate", "mm/s", front_bp, 12);
    tx_car::setRealAxis(susp->mutable_shock()->mutable_shockdampingforce_rear()->mutable_y0_axis(), "Shock_Force",
                        "Shock force", "N", front_table, 12);

    tx_car::setRealScalar(susp->mutable_shock()->mutable_jounceratio_front(), "Front Bounce Ratio",
                          "ratio of spring movement per unit of jounce", "N/A", 0.8613);
    tx_car::setRealScalar(susp->mutable_shock()->mutable_jounceratio_rear(), "Rear Bounce Ratio",
                          "ratio of spring movement per unit of jounce", "N/A", 0.797);
  }

  // bounce
  {
    // toe
    real_T front_bumptoe_bp[15] = {-0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01, 0.0,
                                   0.01,  0.02,  0.03,  0.04,  0.05,  0.06,  0.07};
    real_T front_bumptoe_tb[15] = {0.292407,  0.237822,  0.187561,  0.141604,  0.099916,  0.062452,  0.029164, 0.0,
                                   -0.025093, -0.046165, -0.063264, -0.076429, -0.085696, -0.091086, -0.09261};
    real_T rear_bumptoe_bp[15] = {-0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01, 0.0,
                                  0.01,  0.02,  0.03,  0.04,  0.05,  0.06,  0.07};
    real_T rear_bumptoe_tb[15] = {-1.02057, -0.828098, -0.65623, -0.501756, -0.361641, -0.233037, -0.113294, 0.0,
                                  0.108974, 0.215401,  0.320605, 0.425353,  0.529714,  0.632927,  0.733253};

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumptoe_front(), "front toe vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumptoe_front()->mutable_u0_axis(), "Compression",
                        "compression", "m", front_bumptoe_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumptoe_front()->mutable_y0_axis(), "Toe", "toe", "deg",
                        front_bumptoe_tb, 15);

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumptoe_rear(), "rear toe vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumptoe_rear()->mutable_u0_axis(), "Compression", "compression",
                        "m", rear_bumptoe_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumptoe_rear()->mutable_y0_axis(), "Toe", "toe", "deg",
                        rear_bumptoe_tb, 15);

    // camber
    real_T front_bumpcamber_bp[15] = {-0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01, 0.0,
                                      0.01,  0.02,  0.03,  0.04,  0.05,  0.06,  0.07};
    real_T front_bumpcamber_tb[15] = {1.08171,   0.996827,  0.88789,   0.755642, 0.600569, 0.422934, 0.222791, 0.0,
                                      -0.245764, -0.515012, -0.808436, -1.12692, -1.47156, -1.84365, -2.24474};
    real_T rear_bumpcamber_bp[15] = {-0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01, 0.0,
                                     0.01,  0.02,  0.03,  0.04,  0.05,  0.06,  0.07};
    real_T rear_bumpcamber_tb[15] = {1.66479,   1.43877,   1.21143,   0.981031, 0.746032, 0.505036, 0.25676, 0.0,
                                     -0.266403, -0.543604, -0.832791, -1.13524, -1.45237, -1.78583, -2.13754};

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumpcamber_front(), "front camber vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcamber_front()->mutable_u0_axis(), "Compression",
                        "compression", "m", front_bumpcamber_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcamber_front()->mutable_y0_axis(), "Camber", "camber",
                        "deg", front_bumpcamber_tb, 15);

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumpcamber_rear(), "rear camber vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcamber_rear()->mutable_u0_axis(), "Compression",
                        "compression", "m", rear_bumpcamber_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcamber_rear()->mutable_y0_axis(), "Camber", "camber", "deg",
                        rear_bumpcamber_tb, 15);

    // caster
    real_T bumpcaster_bp[15] = {-0.07, -0.06, -0.05, -0.04, -0.03, -0.02, -0.01, 0.0,
                                0.01,  0.02,  0.03,  0.04,  0.05,  0.06,  0.07};
    real_T bumpcaster_tb[15] = {2.07166,   1.77237,   1.47431,   1.17742,  0.881618, 0.586832, 0.292987, 0.0,
                                -0.292218, -0.583766, -0.874758, -1.16532, -1.45561, -1.7458,  -2.03611};
    real_T bumpcaster_tb_rear[15];
    memset(bumpcaster_tb_rear, 0x00, 15 * sizeof(real_T));

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumpcaster(), "front caster vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcaster()->mutable_u0_axis(), "Compression", "compression",
                        "m", bumpcaster_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcaster()->mutable_y0_axis(), "Caster", "caster", "deg",
                        bumpcaster_tb, 15);

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_bumpcaster_rear(), "rear caster vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcaster_rear()->mutable_u0_axis(), "Compression",
                        "compression", "m", bumpcaster_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_bumpcaster_rear()->mutable_y0_axis(), "Caster", "caster", "deg",
                        bumpcaster_tb_rear, 15);

    // lateral wheel displacement
    real_T front_latdisp_bp[15] = {-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
                                   10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0};
    real_T front_latdisp_tb[15] = {4.74662,  3.12302, 1.82181, 0.837522, 0.165882, -0.196302, -0.251197, 0.0,
                                   0.557037, 1.42059, 2.59226, 4.07461,  5.87116,  7.98647,   10.4262};
    real_T rear_latdisp_bp[15] = {-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
                                  10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0};
    real_T rear_latdisp_tb[15] = {5.34878, 3.82136,  2.55534, 1.54581, 0.788587, 0.280222, 0.018009, 0.0,
                                  0.22505, 0.692872, 1.40412, 2.36046, 3.56474,  5.02106,  6.73495};

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_latwhlctrdisplacement_front(),
                         "front wheel center lateral displacement vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_latwhlctrdisplacement_front()->mutable_u0_axis(), "Compression",
                        "compression", "mm", front_latdisp_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_latwhlctrdisplacement_front()->mutable_y0_axis(),
                        "Wheel_Lateral_Displacement", "wheel center lateral displacement", "mm", front_latdisp_tb, 15);

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_latwhlctrdisplacement_rear(),
                         "rear wheel center lateral displacement vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_latwhlctrdisplacement_rear()->mutable_u0_axis(), "Compression",
                        "compression", "mm", rear_latdisp_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_latwhlctrdisplacement_rear()->mutable_y0_axis(),
                        "Wheel_Lateral_Displacement", "wheel center lateral displacement", "mm", rear_latdisp_tb, 15);

    // longitudinal wheel displacement
    real_T front_lngdisp_bp[15] = {-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
                                   10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0};
    real_T front_lngdisp_tb[15] = {-0.729904, -0.634454, -0.536096, -0.434813, -0.330589, -0.2234,  -0.113217, 0.0,
                                   0.116308,  0.235783,  0.358522,  0.484654,  0.61434,   0.747787, 0.885253};
    real_T rear_lngdisp_bp[15] = {-70.0, -60.0, -50.0, -40.0, -30.0, -20.0, -10.0, 0.0,
                                  10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0};
    real_T rear_lngdisp_tb[15] = {2.6892,   1.76827, 1.01123, 0.426265, 0.022328, -0.190752, -0.202151, 0.0,
                                  0.428603, 1.09748, 2.02114, 3.21439,  4.69179,  6.46687,   8.5513};

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_lngwhlctrdisp_front(),
                         "front wheel center longitudinal displacement vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_lngwhlctrdisp_front()->mutable_u0_axis(), "Compression",
                        "compression", "mm", front_lngdisp_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_lngwhlctrdisp_front()->mutable_y0_axis(),
                        "Wheel_Longitudinal_Displacement", "wheel center lateral displacement", "mm", front_lngdisp_tb,
                        15);

    tx_car::set1DMapInfo(susp->mutable_bounce()->mutable_lngwhlctrdisp_rear(),
                         "rear wheel center longitudinal displacement vs compression", "");
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_lngwhlctrdisp_rear()->mutable_u0_axis(), "Compression",
                        "compression", "mm", rear_lngdisp_bp, 15);
    tx_car::setRealAxis(susp->mutable_bounce()->mutable_lngwhlctrdisp_rear()->mutable_y0_axis(),
                        "Wheel_Longitudinal_Displacement", "wheel center lateral displacement", "mm", rear_lngdisp_tb,
                        15);

    // spring rate
    tx_car::setRealScalar(susp->mutable_bounce()->mutable_nrmlwhlrates_front(), "Front_Spring_Rate",
                          "linear spring rate for front suspension spring", "N/mm", 34);
    tx_car::setRealScalar(susp->mutable_bounce()->mutable_nrmlwhlrates_rear(), "Rear_Spring_Rate",
                          "linear spring rate for rear suspension spring", "N/mm", 46);
  }

  // roll
  {
    tx_car::setRealScalar(susp->mutable_roll()->mutable_rollstiff_front(), "Front_Roll_Stiffness",
                          "front suspension auxilary roll moment", "Nm/deg", 384);
    tx_car::setRealScalar(susp->mutable_roll()->mutable_rollstiff_rear(), "Rear_Roll_Stiffness",
                          "rear suspension auxilary roll moment", "Nm/deg", 344);
  }

  // longitudinal compliance
  {
    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_toecomplianceperfx_front(), "Front_Toe_Compliance_Fx",
                          "front toe deflection per unit of tire Fx ", "deg/KN", -1.9e-3);
    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_toecomplianceperfx_rear(), "Rear_Toe_Compliance_Fx",
                          "rear toe deflection per unit of tire Fx", "deg/KN", -6.3e-3);

    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_cambercomplianceperfx_front(),
                          "Front_Camber_Compliance_Fx", "front camber deflection per unit of tire Fx ", "deg/KN", 0.0);
    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_cambercomplianceperfx_rear(),
                          "Rear_Camber_Compliance_Fx", "rear camber deflection per unit of tire Fx", "deg/KN", 0.0);

    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_castercompliancefx_front(),
                          "Front_Caster_Compliance_Fx", "front caster deflection per unit of tire Fx ", "deg/KN", 0.0);
    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_castercompliancefx_rear(), "Rear_Caster_Compliance_Fx",
                          "rear caster deflection per unit of tire Fx", "deg/KN", 0.0);

    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_whldispcomplianceperfx_front(),
                          "Longitudinal_WheelCenter_Compliance",
                          "front longitudinal wheel center deflection per unit of tire Fx ", "mm/KN", 0.2);
    tx_car::setRealScalar(susp->mutable_longitudinal()->mutable_whldispcomplianceperfx_rear(),
                          "Longitudinal_WheelCenter_Compliance",
                          "rear longitudinal wheel center deflection per unit of tire Fx", "mm/KN", 0.036);
  }

  // lateral compliance
  {
    tx_car::setRealScalar(susp->mutable_lateral()->mutable_toecomplianceperfy_front(), "Front_Steer_Compliance_Fy",
                          "front steer deflection per unit of tire Fy", "deg/KN", -0.0330);
    tx_car::setRealScalar(susp->mutable_lateral()->mutable_toecomplianceperfy_rear(), "Rear_Steer_Compliance_Fy",
                          "rear steer deflection per unit of tire Fy", "deg/KN", -0.0083);

    tx_car::setRealScalar(susp->mutable_lateral()->mutable_cambercomplianceperfy_front(),
                          "Front_Inclination_Compliance_Fy", "front inclination deflection per unit of tire Fy",
                          "deg/KN", 0.0500);
    tx_car::setRealScalar(susp->mutable_lateral()->mutable_cambercomplianceperfy_rear(),
                          "Rear_Inclination_Compliance_Fy", "rear inclination deflection per unit of tire Fy", "deg/KN",
                          -0.0330);

    tx_car::setRealScalar(susp->mutable_lateral()->mutable_whldispcomplianceperfy_front(),
                          "Lateral_WheelCenter_Compliance",
                          "front longitudinal wheel center deflection per unit of tire Fy", "mm/KN", 0.0220);
    tx_car::setRealScalar(susp->mutable_lateral()->mutable_whldispcomplianceperfy_rear(),
                          "Lateral_WheelCenter_Compliance",
                          "rear longitudinal wheel center deflection per unit of tire Fy", "mm/KN", 0.0940);
  }

  // alignment
  {
    tx_car::setRealScalar(susp->mutable_align()->mutable_aligntrqsteer_front(), "Front_Toe_Compliance_Mz",
                          "front toe deflection per unit of tire Mz", "deg/KNm", 0.1400);
    tx_car::setRealScalar(susp->mutable_align()->mutable_aligntrqsteer_rear(), "Rear_Toe_Compliance_Mz",
                          "rear toe deflection per unit of tire Mz", "deg/KNm", -0.1200);

    tx_car::setRealScalar(susp->mutable_align()->mutable_aligntrqcamber_front(), "Front_Inclination_Compliance_Mz",
                          "front inclination deflection per unit of tire Mz", "deg/KNm", -0.0530);
    tx_car::setRealScalar(susp->mutable_align()->mutable_aligntrqcamber_rear(), "Rear_Inclination_Compliance_Mz",
                          "rear inclination deflection per unit of tire Mz", "deg/KNm", -0.0500);
  }

  // static
  {
    tx_car::setRealScalar(susp->mutable_static_()->mutable_statictoe_front(), "Front_Toe_Static",
                          "static front toe angle", "deg", 0.2);
    tx_car::setRealScalar(susp->mutable_static_()->mutable_statictoe_rear(), "Rear_Toe_Static", "static rear toe angle",
                          "deg", 0.1);

    tx_car::setRealScalar(susp->mutable_static_()->mutable_staticcamber_front(), "Front_Camber_Static",
                          "static front camber angle", "deg", -0.14);
    tx_car::setRealScalar(susp->mutable_static_()->mutable_staticcamber_rear(), "Rear_Camber_Static",
                          "static rear camber angle", "deg", 0.0);
  }
}
