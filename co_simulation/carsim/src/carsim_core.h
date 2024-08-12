// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef TADSIM_CO_SIMULATION_CARSIM_CARSIM_CORE_H_
#define TADSIM_CO_SIMULATION_CARSIM_CARSIM_CORE_H_

#include <cmath>
#include <map>
#include <string>

#define MS_2_KMH(x) ((x) * 3.6)
#define KMH_2_MS(x) ((x) / 3.6)

#define RAD_2_DEG(rad) ((rad) / M_PI * 180.0)
#define DEG_2_RAD(deg) ((deg) / 180.0 * M_PI)

#define MAX_FILENAME_LENGTH 256

#define To_Char_Ptr(x) const_cast<char *>(x)

#define SPLIT_LINE "************************************\n"

// #define _OPEN_LOOP_ 1

namespace tx_carsim {
class CarSimState;
class CarSimAgent;
class CTxCarsim;

// topics
namespace topic {
const char CONTROL[] = "CONTROL";
const char VEHICLE_STATE[] = "VEHICLE_STATE";
const char LOCATION[] = "LOCATION";
const char LOCATION_TRAILER[] = "LOCATION_TRAILER";
}  // namespace topic

struct Vector3d {
  double x;
  double y;
  double z;

  explicit Vector3d(double xo = 0.0, double yo = 0.0, double zo = 0.0) : x(xo), y(yo), z(zo) {}
};

struct SCarsimInit {
  double x = 0.0;                         // m
  double y = 0.0;                         // m
  double z = 0.0;                         // m
  double vx = 0.0;                        // km/h
  double vy = 0.0;                        // km/h
  double vz = 0.0;                        // km/h
  double yaw = 0.0;                       // deg
  double steer_ratio = 18.0;              // steer ratio
  double wheel_radius = 0.36;             // wheel radius
  double maxCylinderBrakePressure = 8.0;  // MPa

  tx_carsim::Vector3d mapOrigin;
  std::string startLocationPayload;
  std::string carsimSimFile;
};
struct SCarsimIn {
  double steer;      // rad
  double driveMode;  // gear lever, 1 forward
  double throttle;   // throttle, [0,1]
  double brake;      // brake, [0,1]
};
struct SCarsimOut {
  double t;
  double xo;
  double yo;
  double zo;
  double yaw;
  double pitch;
  double roll;
  double vx;
  double vy;
  double vz;
  double avx;
  double avy;
  double avz;
  double ax;
  double ay;
  double az;
  // wheel speed, km/h
  double v_whl_l1, v_whl_l2, v_whl_r1, v_whl_r2;
  // current gear 5678--->DRPN
  double gearLever;
  // motor torque
  double motor_trq_f, motor_trq_r1, motor_trq_r2;
  // front wheel angle, deg
  double steer_l1, steer_l2, steer_r1, steer_r2;
  // steer wheel angle, deg
  double steerWheelAngle;
  // brake main cylinder pressure, bar
  double brakePressure;
};
enum SCarsimState { CERROR, COK };
}  // namespace tx_carsim
#endif  // TADSIM_CO_SIMULATION_CARSIM_CARSIM_CORE_H_
