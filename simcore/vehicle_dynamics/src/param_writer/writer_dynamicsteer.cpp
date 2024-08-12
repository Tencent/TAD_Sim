// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/writer_dynamicsteer.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

#include <cmath>

void DynamicSteerWriter::fillDynamicSteer(tx_car::car &m_car) {
  auto dynamicSteer = m_car.mutable_steer_system()->mutable_dynamicsteer();

  // geometry
  {
    auto steerGeometry = dynamicSteer->mutable_steergeometry();

    tx_car::setRealScalar(steerGeometry->mutable_steertrackwidth(), "steering track width",
                          "steering track width, distance from one end of steer arm to the other", "m", 1.37);
    tx_car::setRealScalar(steerGeometry->mutable_steeringwheelanglerange(),
                          "absolute value of max steering wheel angle", "absolute value of max steering wheel angle",
                          "rad", 5.0 * M_PI);

    double StrgAngBpts[] = {-6.2832, -5.0265, -3.7699, -2.5133, -1.2566, 0, 1.2566, 2.5133, 3.7699, 5.0265, 6.2832};
    double PnnRadiusTbl[] = {0.0055, 0.0055, 0.0056, 0.0057, 0.0057, 0.0057, 0.0058, 0.0057, 0.0056, 0.0055, 0.0055};
    tx_car::set1DMapInfo(steerGeometry->mutable_pinradiustable(), "pinion radius vs steering wheel angle", "");
    tx_car::setRealAxis(steerGeometry->mutable_pinradiustable()->mutable_u0_axis(), "steering wheel angle",
                        "steering wheel angle", "rad", StrgAngBpts, 11);
    tx_car::setRealAxis(steerGeometry->mutable_pinradiustable()->mutable_y0_axis(), "pinion radius", "pinion radius",
                        "m", PnnRadiusTbl, 11);

    tx_car::setRealScalar(steerGeometry->mutable_rackcasinglength(), "steering rack length",
                          "steering rack length, distance from one end of tie road to the other", "m", 0.48);
    tx_car::setRealScalar(steerGeometry->mutable_tierodlength(), "steering tie rod length", "steering tie rod length",
                          "m", 0.435);
    tx_car::setRealScalar(steerGeometry->mutable_steerarmlength(), "steering arm length", "steering arm length", "m",
                          0.117);

    tx_car::setRealScalar(steerGeometry->mutable_dist_rackandfrontaxle_longitudinal(),
                          "longitudinal distance from front axle to rack",
                          "longitudinal distance from front axle to rack", "m", 0.117);
  }

  // dynamic
  {
    auto dynamic = dynamicSteer->mutable_steerdynamics();

    tx_car::setRealScalar(dynamic->mutable_steerwheelinertia(), "steering wheel inertia", "steering wheel inertia",
                          "kg*m2", 0.1);
    tx_car::setRealScalar(dynamic->mutable_steerwheeldamping(), "steering wheel damping", "steering wheel damping",
                          "N*m/(rad/s)", 1);

    tx_car::setRealScalar(dynamic->mutable_steercolumninertia(), "steering column inertia", "steering column inertia",
                          "kg*m2", 0.01);
    tx_car::setRealScalar(dynamic->mutable_steercolumndamping(), "steering column damping", "steering column damping",
                          "N*m/(rad/s)", 0.001);

    tx_car::setRealScalar(dynamic->mutable_hysteresisstiffness(), "hysteres stiffness",
                          "hysteres stiffness between steering wheel and column", "N*m/rad", 60);
    tx_car::setRealScalar(dynamic->mutable_hysteresisdamping(), "hysteres damping",
                          "hysteres damping between steering wheel and column", "N*m/(rad/s)", 0.001);

    tx_car::setRealScalar(dynamic->mutable_steerfrictorque(), "steer friction torque", "steer friction torque", "N*m",
                          0.1);
  }

  // power assist
  {
    auto powerAssist = dynamicSteer->mutable_powerassist();

    const double maxAssistTorque = 20.0, maxSpeed = 65.0;
    tx_car::setRealScalar(powerAssist->mutable_maxassisttorque(), "max assit torque", "max assit torque", "N*m",
                          maxAssistTorque);
    tx_car::setRealScalar(powerAssist->mutable_maxassistpower(), "max assit power", "max assit power", "w",
                          maxAssistTorque * 10.0);

    const uint32_t u0Size = 9, u1Size = 14, y0Size = u0Size * u1Size;
    double TrqBpts[u0Size] = {-20, -15, -10, -5, 0, 5, 10, 15, 20};
    double VehSpdBpts[u1Size] = {0.0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65};
    double TrqTbl[y0Size];

    // column order
    for (auto j = 0; j < u1Size; ++j) {
      for (auto i = 0; i < u0Size; ++i) {
        double assistRatio = 10 * (maxSpeed - VehSpdBpts[j]) / maxSpeed + 1.0;
        assistRatio = assistRatio < 1.0 ? 1.0 : assistRatio;
        TrqTbl[j * u0Size + i] = TrqBpts[i] * std::log10(assistRatio);
      }
    }

    auto assistTorqueMap = powerAssist->mutable_powerassisttable();

    assistTorqueMap->set_disp_name("power assist map");
    assistTorqueMap->set_comment("map 2d for power assist, assist torque vs torque input and vehicle speed");

    tx_car::setRealAxis(assistTorqueMap->mutable_u0_axis(), "input steering wheel torque",
                        "user input steering wheel torque", "N*m", TrqBpts, u0Size);
    tx_car::setRealAxis(assistTorqueMap->mutable_u1_axis(), "vehicle speed", "vehicle speed", "m/s", VehSpdBpts,
                        u1Size);
    tx_car::setRealAxis(assistTorqueMap->mutable_y0_axis(), "assist torque", "output assist torque", "N*m", TrqTbl,
                        y0Size);
  }
}
