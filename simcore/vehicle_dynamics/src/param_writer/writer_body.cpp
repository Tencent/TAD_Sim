// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/writer_body.h"
#include "Chassis.h"
#include "inc/proto_helper.h"

void VehBodyWriter::fillSprungMass(tx_car::car &m_car) {
  tx_car::SprungMassDD *sprung_mass = m_car.mutable_sprung_mass();
  tx_car::setRealScalar(sprung_mass->mutable_mass(), "Mass", "vehicle mass", "kg", 1830);
  /*
  tx_car::setRealScalar(sprung_mass->mutable_staticnormalfrontload()->mutable_fl(), "StaticNormalFrontLoad_FL",
      "static normal front left load", "kg", 446.31);
  tx_car::setRealScalar(sprung_mass->mutable_staticnormalfrontload()->mutable_fr(), "StaticNormalFrontLoad_FR",
      "static normal front right load", "kg", 446.31);
  tx_car::setRealScalar(sprung_mass->mutable_staticnormalrearload()->mutable_rr(), "StaticNormalRearLoad_RR",
      "static normal rear right load", "kg", 378.69);
  tx_car::setRealScalar(sprung_mass->mutable_staticnormalrearload()->mutable_rl(), "StaticNormalRearLoad_RL",
      "static normal rear left load", "kg", 378.69);
  */
  tx_car::setRealScalar(sprung_mass->mutable_wheelbase(), "Wheel_Base",
                        "distance between front wheel center and read wheel center", "m", 3.05);
  tx_car::setRealScalar(sprung_mass->mutable_frontaxlepositionfromcg(), "Front_Axle_Position_from_CG",
                        "Longitudinal distance from center of mass to front axle", "m", 1.4);
  tx_car::setRealScalar(sprung_mass->mutable_rearaxlepositionfromcg(), "Rear_Axle_Position_from_CG",
                        "Longitudinal distance from center of mass to rear axle", "m", 1.65);
  tx_car::setRealScalar(sprung_mass->mutable_heightcg(), "Height_CG",
                        "Vertical distance from center of mass to axle plane", "m", 0.21);
  tx_car::setRealScalar(sprung_mass->mutable_frontalarea(), "Af", "Longitudinal drag are", "m^2", 2.11);
  tx_car::setRealScalar(sprung_mass->mutable_dragcoefficient(), "Cd", "Longitudinal drag coefficient", "[]", 0.33);
  tx_car::setRealScalar(sprung_mass->mutable_numberofwheelsperaxle(), "Wheels_Per_Axle", "number of wheels per axle",
                        "[]", 2);
  tx_car::setRealScalar(sprung_mass->mutable_pitchmomentinertia(), "Pitch_Inertia", "pitch moment inertia", "kg*m^2",
                        2788.5);
  tx_car::setRealScalar(sprung_mass->mutable_rollmomentinertia(), "Roll_Inertia", "roll moment inertia", "kg*m^2",
                        928.1);
  tx_car::setRealScalar(sprung_mass->mutable_yawmomentinertia(), "Yaw_Inertia", "yaw moment inertia", "kg*m^2", 3234.0);
  tx_car::setRealScalar(sprung_mass->mutable_steeringratio(), "Steering_Ratio", "steering ratio", "[]", 20.0);
  tx_car::setRealScalar(sprung_mass->mutable_trackwidthfront(), "Track_Width_Front", "track width front", "[]", 1.6);
  tx_car::setRealScalar(sprung_mass->mutable_trackwidthrear(), "Track_Width_Rear", "track width rear", "[]", 1.6);
  tx_car::setRealScalar(sprung_mass->mutable_sprungmass(), "Sprung_Mass", "sprung mass", "kg", 1650.0);
  tx_car::setRealScalar(sprung_mass->mutable_initiallongposition(), "Init_Long_Position", "initial longtitude position",
                        "m", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initiallatposition(), "Init_Lat_Position", "initial latitude position",
                        "m", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialvertposition(), "Init_Ver_Position", "initial vertical position",
                        "m", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialrollangle(), "Init_Roll_Angle", "initial roll angle", "rad", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialpitchangle(), "Init_Pitch_Angle", "initial pitch angle", "rad",
                        0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialyawangle(), "Init_Yaw_Angle", "initial yaw angle", "rad", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initiallongvel(), "Init_Long_Vel", "initial longtitude velocity", "m/s",
                        60 / 3.6);
  tx_car::setRealScalar(sprung_mass->mutable_initiallatvel(), "Init_Lat_Vel", "initial latitude velocity", "m/s", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialvertvel(), "Init_Ver_Vel", "initial vertical velocity", "m/s", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialrollrate(), "Init_Roll_Rate", "initial roll velocity", "rad/s",
                        0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialpitchrate(), "Init_Pitch_Rate", "initial pitch velocity", "rad/s",
                        0.0);
  tx_car::setRealScalar(sprung_mass->mutable_initialyawrate(), "Init_Yaw_Rate", "initial yaw velocity", "rad/s", 0.0);
  tx_car::setRealScalar(sprung_mass->mutable_unsprungmassfrontaxle(), "Unsprung_Mass_Front_Axle",
                        "unsprung mass of front axle", "kg", 90);
  tx_car::setRealScalar(sprung_mass->mutable_unsprungmassrearaxle(), "Unsprung_Mass_Rear_Axle",
                        "unsprung mass of rear axle", "kg", 90);
}

void VehBodyWriter::fillVehbody(tx_car::car &m_car) {
  const tx_car::SprungMassDD &sprung = m_car.sprung_mass();

  // set aerodynamics
  {
    tx_car::AeroDynamics *aero = m_car.mutable_body()->mutable_aerodynamics();
    real_T beta_w[31] = {0.0,
                         0.01,
                         0.02,
                         0.03,
                         0.04,
                         0.05,
                         0.060000000000000005,
                         0.069999999999999993,
                         0.08,
                         0.09,
                         0.099999999999999992,
                         0.11,
                         0.12,
                         0.13,
                         0.14,
                         0.15000000000000002,
                         0.15999999999999998,
                         0.16999999999999998,
                         0.18,
                         0.19,
                         0.19999999999999998,
                         0.21,
                         0.21999999999999997,
                         0.22999999999999998,
                         0.24,
                         0.25,
                         0.26,
                         0.27,
                         0.27999999999999997,
                         0.29,
                         0.3};
    real_T cs[31] = {0.0,
                     0.01,
                     0.02,
                     0.03,
                     0.04,
                     0.05,
                     0.060000000000000005,
                     0.069999999999999993,
                     0.08,
                     0.09,
                     0.099999999999999992,
                     0.11,
                     0.12,
                     0.13,
                     0.14,
                     0.15000000000000002,
                     0.15999999999999998,
                     0.16999999999999998,
                     0.18,
                     0.19,
                     0.19999999999999998,
                     0.21,
                     0.21999999999999997,
                     0.22999999999999998,
                     0.24,
                     0.25,
                     0.26,
                     0.27,
                     0.27999999999999997,
                     0.29,
                     0.3};
    real_T cym[31] = {0.0,
                      1.0E-6,
                      0.010001,
                      0.020001,
                      0.030001,
                      0.040001,
                      0.050001000000000004,
                      0.060001,
                      0.070001000000000008,
                      0.080001,
                      0.090001,
                      0.100001,
                      0.110001,
                      0.120001,
                      0.130001,
                      0.14000100000000001,
                      0.15000099999999994,
                      0.16000099999999995,
                      0.17000099999999996,
                      0.18000099999999997,
                      0.19000099999999995,
                      0.20000099999999996,
                      0.21000099999999994,
                      0.22000099999999995,
                      0.23000099999999996,
                      0.24000099999999996,
                      0.250001,
                      0.26000099999999993,
                      0.27000099999999994,
                      0.28000099999999994,
                      0.29000099999999995};
    tx_car::setRealScalar(aero->mutable_vehiclebody6dof_cd(), "Cd", "Longitudinal drag coefficient", "[]", 0.3);
    tx_car::setRealScalar(aero->mutable_vehiclebody6dof_cl(), "Cl", "Longitudinal lift coefficient", "[]", 0.1);
    tx_car::setRealScalar(aero->mutable_vehiclebody6dof_cpm(), "Cl", "Longitudinal drag pitch moment", "[]", 0.1);

    tx_car::set1DMapInfo(aero->mutable_beta_w_cs_1dmap(), "Relative wind angle vs Side force coefficient", "");
    tx_car::setRealAxis(aero->mutable_beta_w_cs_1dmap()->mutable_u0_axis(), "Beta_W", "Relative wind angle vector",
                        "rad", beta_w, 31);
    tx_car::setRealAxis(aero->mutable_beta_w_cs_1dmap()->mutable_y0_axis(), "Cs", "Side force coefficient vector", "",
                        cs, 31);

    tx_car::set1DMapInfo(aero->mutable_beta_w_cym_1dmap(), "Relative wind angle vs Yaw moment coefficient", "");
    tx_car::setRealAxis(aero->mutable_beta_w_cym_1dmap()->mutable_u0_axis(), "Beta_W", "Relative wind angle vector",
                        "rad", beta_w, 31);
    tx_car::setRealAxis(aero->mutable_beta_w_cym_1dmap()->mutable_y0_axis(), "Cym", "Yaw moment coefficient vector", "",
                        cym, 31);

    tx_car::setRealScalar(aero->mutable_vehiclebody6dof_pabs(), "Pabs", "Absolute Pressure", "Pa", 101325);
    tx_car::setRealScalar(aero->mutable_vehiclebody6dof_tair(), "Tair", "Air temperature", "K", 273);
  }

  // set simulation
  {
    /*
    tx_car::VehBodySim *sim = m_car.mutable_body()->mutable_simulation();
    sim->mutable_vehiclebody6dof_xdot_tol()->set_val(0.1);
    sim->mutable_vehiclebody6dof_longoff()->set_val(0.0);
    sim->mutable_vehiclebody6dof_latoff()->set_val(0.0);
    sim->mutable_vehiclebody6dof_vertoff()->set_val(0.0112);
    */
  }
}
