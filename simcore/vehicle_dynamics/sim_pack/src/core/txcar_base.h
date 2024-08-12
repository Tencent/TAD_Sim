// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <map>
#include <memory>
#include <string>
#include <type_traits>

#include "model_hooks.h"

namespace tx_car {
// gravity
const double constGravity = 9.80665;

// gear lever
enum GearOperator { Parking = 0, Reverse = 1, Netural = 2, Drive = 3 };

// init of vehicle
struct TxCarInit {
  size_t loops_per_step;      // vehicle model loop times per step
  double init_x;              // unit, m
  double init_y;              // unit, m
  double init_z;              // unit, m
  double init_vx;             // unit, m/s
  double init_vy;             // unit, m/s
  double init_vz;             // unit, m/s
  double init_roll;           // unit, rad
  double init_pitch;          // unit, rad
  double init_yaw;            // unit, rad
  GearOperator init_gear_op;  // init gear operator

  std::string param_path;
  bool enable_terrain;
  std::string start_loc_payload;
  std::string hadmap_path;
  double mu;
  std::string vehicle_geometory_payload;

  TxCarInit(double x = 0.0, double y = 0.0, double z = 0.0, double vx = 0.0, double vy = 0.0, double vz = 0.0,
            double yaw = 0.0, GearOperator gear_op = GearOperator::Netural);

  void clear();

  static void debugShow(const TxCarInit &carInit);
};

// point3d
struct Point3d {
  double x;
  double y;
  double z;

  explicit Point3d(double xo = 0.0, double yo = 0.0, double zo = 0.0) : x(xo), y(yo), z(zo) {}
};

// topics
namespace topic {
const char CONTROL[] = "CONTROL";
const char CONTROL_V2[] = "CONTROL_V2";
const char VEHICLE_STATE[] = "VEHICLE_STATE";
const char LOCATION[] = "LOCATION";
}  // namespace topic

// constants
namespace Constants {
#ifdef _WIN32
const char const_mdl_ICE_Name[] = "txcar_ice.dll";
const char const_mdl_EV_Name[] = "txcar_ev.dll";
const char const_mdl_Hybrid_Name[] = "txcar_hybrid.dll";
#else
const char const_mdl_ICE_Name[] = "libtxcar_ice.so";
const char const_mdl_EV_Name[] = "libtxcar_ev.so";
const char const_mdl_Hybrid_Name[] = "libtxcar_hybrid.so";
#endif
}  // namespace Constants

/**
 * @brief control payload watch dog
 */
struct MsgWatchDog {
  std::string m_prePayload = "";
  int m_missingCounter = 0;
  int m_stopCounter = 0;
  bool m_bMissing = false;

  MsgWatchDog();
  void feed(const std::string &payload);
  bool requestStop(int stopThresh = 18000);

  const static int const_missingCounterThresh = 50;
  const static int const_missingCounterThresh_Stop = 18000;
};

// vehicle model
class TxCarBase;
using TxCarBasePtr = std::shared_ptr<TxCarBase>;

// vehicle base model
class TxCarBase {
 public:
  TxCarBase();
  virtual ~TxCarBase() {}

  TxCarBase(const TxCarBase &) = delete;
  TxCarBase &operator=(const TxCarBase &) = delete;

  virtual void init(const TxCarInit &car_init) = 0;
  virtual void step(double t_ms) = 0;
  virtual void stop() = 0;
  virtual bool isModelValid() = 0;

 public:
  // get/set protobuf message
  virtual std::string getMsgByTopic(const std::string &topic);
  virtual void setMsgByTopic(const std::string &topic, const std::string &payload);

  virtual void setLogLevel(int flags_v);

 public:
  using VehicleModelBaseType = TxCarBase;
  inline std::string getProcessID() { return m_processID; }

 private:
  void calProcessID();

 protected:
  // vehicle init
  TxCarInit m_carParam;

  // protobuf messages
  std::map<std::string, std::string> mMsgs;

  // process id
  std::string m_processID;
};
}  // namespace tx_car
