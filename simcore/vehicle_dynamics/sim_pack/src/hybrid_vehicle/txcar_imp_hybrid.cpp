// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txcar_imp_hybrid.h"

#include <cmath>
#include <exception>

#include "TxCar_Hybrid.h"

#include "car_common.h"
#include "car_log.h"
#include "coord_trans.h"
#include "location.pb.h"
#include "proto_helper.h"

#include "car.pb.h"
#include "control.pb.h"
#include "control_v2.pb.h"
#include "vehState.pb.h"

#include "boost/filesystem.hpp"

namespace fs = boost::filesystem;

#ifndef M_PI
#  define M_PI (3.1415926)
#endif  // !M_PI

#define RAD_S_2_RPM (30.0 / M_PI)
#define RAD_2_DEG (180.0 / M_PI)
#define DEG_2_RAD (M_PI / 180.0)
#define ENABLE_STABLE_OUTPUT_AT_START 1

namespace tx_car {
// global steer ratio
double const_steerRatio_Hybrid = 18.0;
const double const_flipSteer_Hybrid = -1.0;
const int const_flipTerrain_Hybrid = -1;

// stablelize output at beginning of simulation
const double const_StableT_ms_Hybrid = 200.0;

// vehicle model global data
std::shared_ptr<TxCar_Hybrid> g_carModel_Hybrid;
ExtU_TxCar_Hybrid_T g_carU_Hybrid;
ExtY_TxCar_Hybrid_T g_carY_Hybrid;

///////////////////////
TxCarImp_Hybrid::TxCarImp_Hybrid() {}
TxCarImp_Hybrid::~TxCarImp_Hybrid() {}

void TxCarImp_Hybrid::setCarInit(const TxCarInit &car_init) { m_carParam = car_init; }

void TxCarImp_Hybrid::resetTerrain() {
  if (m_carParam.enable_terrain && tx_car::isFileExist(m_carParam.hadmap_path)) {
    // wgs84 to enu
    terrain::Vector3d ego_init_enu = {mStartLocation.position().x(), mStartLocation.position().y(),
                                      mStartLocation.position().z()};
    coord_trans_api::lonlat2enu(ego_init_enu.x, ego_init_enu.y, ego_init_enu.z, mMapOrigin.x, mMapOrigin.y,
                                mMapOrigin.z);

    terrain::Vector3d maporigin = {mMapOrigin.x, mMapOrigin.y, mMapOrigin.z};
    mTerrain.releaseRoad();
    mTerrain.initVDRoad(m_carParam.enable_terrain, false, m_carParam.mu);

    if (mTerrain.resetRoad(m_carParam.hadmap_path.c_str(), maporigin, ego_init_enu)) {
      LOG_0 << "terrain enabled.\n";
      return;
    }
  }

  // default choice
  m_carParam.enable_terrain = false;
  LOG_0 << "terrain not enabled.\n";
}
void TxCarImp_Hybrid::resetVehicle() {
  if (!tx_car::isFileExist(m_carParam.param_path)) {
    LOG_0 << "parameter file does not exist, " << m_carParam.param_path << "\n";
    std::abort();
  }

  // load car parameter
  std::string content;
  if (!tx_car::car_config::loadFromFile(content, m_carParam.param_path)) {
    LOG_0 << "parameter file does not exist, " << m_carParam.param_path << "\n";
    std::abort();
  }

  // parse car parameter
  tx_car::car car_param;
  if (!tx_car::jsonToProto(content, car_param)) {
    LOG_0 << "fail to parse vehicle parameter, " << m_carParam.param_path << "\n";
    std::abort();
  }

  // replace init condition in vehicle model param file
  auto sprung_mass = car_param.mutable_sprung_mass();
  auto wheel_tire = car_param.mutable_wheel_tire();

  // replace .tir path
  {
    fs::path param_dir(m_carParam.param_path);
    fs::path tno_file(wheel_tire->mutable_tire()->tno_tir_path().str_var());
    fs::path tir_path = param_dir.parent_path() / tno_file.filename();
    wheel_tire->mutable_tire()->mutable_tno_tir_path()->set_str_var(tir_path.string().c_str());
  }

  // wheel base
  mCgToFrontAxle = sprung_mass->frontaxlepositionfromcg().val();
  mCgToRearAxle = sprung_mass->rearaxlepositionfromcg().val();
  mWheelRadius = mCarParam.wheel_tire().tire().wheel_radius().val();
  mCgHeight = sprung_mass->heightcg().val() + mWheelRadius;
  mWheelBase = sprung_mass->wheelbase().val();
  mHalfTrack = sprung_mass->trackwidthfront().val() / 2.0;

  // ego rotation
  m_ROT_ego_ENU = Eigen::AngleAxisd(mStartLocation.rpy().z(), Eigen::Vector3d::UnitZ()).toRotationMatrix();

  // init position and velocity
  {
    m_carParam.init_x = mStartLocation.position().x();
    m_carParam.init_y = mStartLocation.position().y();
    m_carParam.init_z = mStartLocation.position().z();
    m_carParam.init_vx = std::hypot(mStartLocation.velocity().x(), mStartLocation.velocity().y());
    m_carParam.init_vy = 0.0;
    m_carParam.init_vz = 0.0;

    // wgs84 to enu
    coord_trans_api::lonlat2enu(m_carParam.init_x, m_carParam.init_y, m_carParam.init_z, mMapOrigin.x, mMapOrigin.y,
                                mMapOrigin.z);
    LOG_INFO << "ego statlocation x:" << m_carParam.init_x << ", y:" << m_carParam.init_y << ", z:" << m_carParam.init_z
             << "\n";

    // use vehicle geometory
    if (m_carParam.vehicle_geometory_payload.size() > 0) {
      const sim_msg::VehicleGeometory &geometry = m_geometry_comb.front();
      if (geometry.coord_type() == sim_msg::VehicleGeometory_VehicleCoordType_RearAxle_SnappedToGround) {
        Eigen::Vector3d offset = {mCgToRearAxle, 0.0, 0.0};
        Eigen::Vector3d &&offset_ENU = m_ROT_ego_ENU * offset;
        m_carParam.init_x += offset_ENU[0];
        m_carParam.init_y += offset_ENU[1];
        m_carParam.init_z += offset_ENU[2];
      } else if (geometry.coord_type() == sim_msg::VehicleGeometory_VehicleCoordType_WheelBaseCenter_SnappedToGround) {
        Eigen::Vector3d offset = {mCgToRearAxle - mWheelBase / 2.0, 0.0, 0.0};
        Eigen::Vector3d &&offset_ENU = m_ROT_ego_ENU * offset;
        m_carParam.init_x += offset_ENU[0];
        m_carParam.init_y += offset_ENU[1];
        m_carParam.init_z += offset_ENU[2];
      }
    }

    // ego displacement
    m_Pos_ego_ENU = {m_carParam.init_x, m_carParam.init_y, m_carParam.init_z};

    // body init z
    if (mTerrain.isUsingTerrain()) {
      double dzdy = 0.0, dzdx = 0.0;
      terrain::CTerrainInterface::getHeightAtWorldPosition(m_carParam.init_x, m_carParam.init_y, m_carParam.init_z,
                                                           &(m_carParam.init_z), &dzdy, &dzdx, &m_mu);
      LOG_2 << "vd | body init z is " << m_carParam.init_z << ".\n";
      mTerrain.updateRoad(m_carParam.init_x, m_carParam.init_y, m_carParam.init_z);
    }
  }

  // set init height of body and wheel
  if (mTerrain.isUsingTerrain()) {
    // wheel z
    m_ROT_ego_ENU = Eigen::AngleAxisd(mStartLocation.rpy().z(), Eigen::Vector3d::UnitZ()).toRotationMatrix();
    m_Pos_ego_ENU = {m_carParam.init_x, m_carParam.init_y, m_carParam.init_z};

    double fl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, mHalfTrack, m_mu)[2];
    double fr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, -mHalfTrack, m_mu)[2];
    double rl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, mHalfTrack, m_mu)[2];
    double rr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, -mHalfTrack, m_mu)[2];

    wheel_tire->mutable_fl_init_height()->set_val(fl_z * const_flipTerrain_Hybrid);  // fl init z
    wheel_tire->mutable_fr_init_height()->set_val(fr_z * const_flipTerrain_Hybrid);  // fr init z
    wheel_tire->mutable_rl_init_height()->set_val(rl_z * const_flipTerrain_Hybrid);  // rl init z
    wheel_tire->mutable_rr_init_height()->set_val(rr_z * const_flipTerrain_Hybrid);  // rr init z

    LOG_2 << "vd | fl wheel init z is " << wheel_tire->fl_init_height().val() << ".\n";
    LOG_2 << "vd | fr wheel init z is " << wheel_tire->fr_init_height().val() << ".\n";
    LOG_2 << "vd | rl wheel init z is " << wheel_tire->rl_init_height().val() << ".\n";
    LOG_2 << "vd | rr wheel init z is " << wheel_tire->rr_init_height().val() << ".\n";
  }

  // flu to frd of init position
  Eigen::Vector3d flu_pos = {m_carParam.init_x, m_carParam.init_y, m_carParam.init_z};
  Eigen::Vector3d frd_pos = m_math.FLU_To_FRD(flu_pos);
  m_carParam.init_x = frd_pos[0];
  m_carParam.init_y = frd_pos[1];
  m_carParam.init_z = frd_pos[2];

  // flu to frd of init rpy
  double pitch = std::atan2(wheel_tire->fl_init_height().val() - wheel_tire->rl_init_height().val(),
                            mCgToFrontAxle + mCgToRearAxle);
  TxCarEuler flu_euler = {mStartLocation.rpy().x(), pitch, mStartLocation.rpy().z()};
  TxCarEuler frd_euler = m_math.FLU_To_FRD(flu_euler);

  m_carParam.init_roll = 0.0;               // frd_euler.roll; //@dhu
  m_carParam.init_pitch = frd_euler.pitch;  // frd_euler.pitch; //@dhu
  m_carParam.init_yaw = frd_euler.yaw;

  // init state
  sprung_mass->mutable_initiallongposition()->set_val(m_carParam.init_x);
  sprung_mass->mutable_initiallatposition()->set_val(m_carParam.init_y);
  sprung_mass->mutable_initialvertposition()->set_val(m_carParam.init_z);
  sprung_mass->mutable_initiallongvel()->set_val(m_carParam.init_vx);
  sprung_mass->mutable_initiallatvel()->set_val(0.0);
  sprung_mass->mutable_initialvertvel()->set_val(0.0);
  sprung_mass->mutable_initialrollangle()->set_val(m_carParam.init_roll);
  sprung_mass->mutable_initialpitchangle()->set_val(m_carParam.init_pitch);
  sprung_mass->mutable_initialyawangle()->set_val(m_carParam.init_yaw);

  mPreVelocity = {m_carParam.init_vx, 0.0, 0.0};

  // create new car parameter file and reload
#ifdef _WIN32
  std::string replace_param_path = "C:\\Windows\\Temp\\tx_car_" + getProcessID() + ".json";
#else
  std::string replace_param_path = "/tmp/tx_car_" + getProcessID() + ".json";
#endif  // _WIN32
  content.clear();
  tx_car::protoToJson<tx_car::car>(car_param, content);
  if (tx_car::car_config::dumpToFile(content, replace_param_path)) {
    m_carParam.param_path = replace_param_path;
    LOG_0 << "temp car param saved into " << replace_param_path << "\n";
  } else {
    LOG_ERROR << "fail to dump temp car param into " << replace_param_path << "\n";
    std::abort();
  }
  mCarParam.CopyFrom(car_param);

  // get max brake pressure
  mMaxBrakePressure = mCarParam.wheel_tire().brake().max_main_cylinder_pressure().val() < 1e6
                          ? 10e6
                          : mCarParam.wheel_tire().brake().max_main_cylinder_pressure().val();
  LOG_0 << "mMaxBrakePressure[Pa]:" << mMaxBrakePressure << "\n";

  // pass param file path to vd module
  tx_car::car_config::setParPath(m_carParam.param_path, true);

  // debug show
  tx_car::TxCarInit::debugShow(m_carParam);

  // steer ratio
  const_steerRatio_Hybrid = std::max(mCarParam.sprung_mass().steeringratio().val(), 18.0);

  // init
  g_carModel_Hybrid = std::make_shared<TxCar_Hybrid>();
  memset(&g_carU_Hybrid, 0x00, sizeof(ExtU_TxCar_Hybrid_T));
  g_carModel_Hybrid->initialize();
}

void TxCarImp_Hybrid::initMDL() {
  // init values
  {
    mMsgs.clear();
    mCgToFrontAxle = 1.6;
    mCgToRearAxle = 1.4;
    mWheelBase = mCgToFrontAxle + mCgToRearAxle;
    mWheelRadius = 0.36;
    mCgHeight = mWheelRadius + 0.166;
    mHalfTrack = 0.8;
    m_mu = 0.9;
    mStartLocation.Clear();
    mMaxBrakePressure = 10e6;
  }

  // get ego start location
  mStartLocation.ParseFromString(m_carParam.start_loc_payload);

  // map origin
  {
    mMapOrigin.x = mStartLocation.position().x();
    mMapOrigin.y = mStartLocation.position().y();
    mMapOrigin.z = mStartLocation.position().z();

    LOG_0 << "map origin:" << mMapOrigin.x << ", " << mMapOrigin.y << ", " << mMapOrigin.z << "\n";
  }

  // vehicle geometory
  if (m_carParam.vehicle_geometory_payload.size() > 0) {
    m_geometry_comb.ParseFromString(m_carParam.vehicle_geometory_payload);
  }

  // init terrain
  resetTerrain();

  // init car model
  resetVehicle();
}
void TxCarImp_Hybrid::releaseMDL() {
  // clear mdl init data
  m_carParam.clear();
  g_carModel_Hybrid.reset();
  LOG_0 << "TxCar mdl released.\n";
}

bool TxCarImp_Hybrid::isModelValid() { return (g_carModel_Hybrid.get() != nullptr); }

std::string TxCarImp_Hybrid::getMsgByTopic(const std::string &topic) {
  auto iter = mMsgs.find(topic);
  if (iter != mMsgs.end()) return iter->second;
  return std::string("");
}
void TxCarImp_Hybrid::setMsgByTopic(const std::string &topic, const std::string &payload) { mMsgs[topic] = payload; }

void TxCarImp_Hybrid::init(const TxCarInit &car_init) {
  setCarInit(car_init);
  initMDL();
  m_ctrl_watch_dog = MsgWatchDog();
  m_ctrlV2_watch_dog = MsgWatchDog();
}
void TxCarImp_Hybrid::step(double t_ms) {
  // get control msg. CONTROL_V2 has higher priority
  std::string ctrlV2Payload = getMsgByTopic(tx_car::topic::CONTROL_V2);
  std::string ctrlV1Payload = getMsgByTopic(tx_car::topic::CONTROL);

  m_ctrl_watch_dog.feed(ctrlV1Payload);
  m_ctrlV2_watch_dog.feed(ctrlV2Payload);

  // LOG_IF(WARNING, m_ctrl_watch_dog.m_missingCounter > 1) << "vd | mControlMonitor.m_missingCounter:" <<
  // m_ctrl_watch_dog.m_missingCounter << "\n";
  LOG_IF(WARNING, m_ctrlV2_watch_dog.m_missingCounter > 1)
      << "vd | mControlV2Monitor.m_missingCounter:" << m_ctrlV2_watch_dog.m_missingCounter << "\n";

  if (m_ctrlV2_watch_dog.requestStop(MsgWatchDog::const_missingCounterThresh_Stop) &&
      m_ctrl_watch_dog.requestStop(MsgWatchDog::const_missingCounterThresh_Stop)) {
    LOG_ERROR << "both control and control_v2 msg missing more than 3 minutes, stop simulation!.\n";
    throw std::runtime_error("both control and control_v2 are msg missing more than 3 minutes, stop simulation!");
    return;
  }

  if (!m_ctrlV2_watch_dog.m_bMissing) {
    controlV2ToVehicle(g_carU_Hybrid, m_ctrlV2_watch_dog.m_prePayload);
  } else if (!m_ctrl_watch_dog.m_bMissing) {
    LOG_2 << "control_v2 message payload is empty, use control.proto instead.\n";
    controlToVehicle(g_carU_Hybrid, m_ctrl_watch_dog.m_prePayload);
  }

  if (m_ctrlV2_watch_dog.m_bMissing && m_ctrl_watch_dog.m_bMissing) {
    LOG_EVERY_N(WARNING, 1000)
        << "both control and control_v2 message payload are empty. switch to Netural gear and slide until stop.\n";
    g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 0;
    g_carU_Hybrid.DriverIn.acc_control_mode = 0;
    g_carU_Hybrid.DriverIn.target_acc_m_s2 = 0.0;
    g_carU_Hybrid.DriverIn.brake_control_mode = 0;
    g_carU_Hybrid.DriverIn.steer_control_mode = 0;
    g_carU_Hybrid.DriverIn.steer_cmd_rad = 0.0;
  }

  // update terrain
  mTerrain.updateRoad(m_Pos_ego_ENU[0], m_Pos_ego_ENU[1], m_Pos_ego_ENU[2]);
  double z = 0.0, dzdy = 0.0, dzdx = 0.0;
  terrain::CTerrainInterface::getHeightAtWorldPosition(m_Pos_ego_ENU[0], m_Pos_ego_ENU[1], m_Pos_ego_ENU[2], &z, &dzdy,
                                                       &dzdx, &m_mu);
  LOG_2 << "vd | ego enu x:" << m_Pos_ego_ENU[0] << ", y:" << m_Pos_ego_ENU[1] << ", z:" << z << ".\n";

  // terrain
  if (mTerrain.isUsingTerrain()) {
    // calculate wheel z
    double fl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, mHalfTrack, m_mu)[2];
    double fr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, -mHalfTrack, m_mu)[2];
    double rl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, mHalfTrack, m_mu)[2];
    double rr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, -mHalfTrack, m_mu)[2];

    g_carU_Hybrid.Height.G_FL_z = fl_z;
    g_carU_Hybrid.Height.G_FR_z = fr_z;
    g_carU_Hybrid.Height.G_RL_z = rl_z;
    g_carU_Hybrid.Height.G_RR_z = rr_z;

    g_carU_Hybrid.Friction.mu_FL = m_mu;
    g_carU_Hybrid.Friction.mu_FR = m_mu;
    g_carU_Hybrid.Friction.mu_RL = m_mu;
    g_carU_Hybrid.Friction.mu_RR = m_mu;

    //*
    LOG_2 << "vd | fl z:" << g_carU_Hybrid.Height.G_FL_z << ", fr z:" << g_carU_Hybrid.Height.G_FR_z
          << ", rl z:" << g_carU_Hybrid.Height.G_RL_z << ", rr z:" << g_carU_Hybrid.Height.G_RR_z << ", mu:" << m_mu
          << "\n";
    //*/
  }

  // update vehicle model
  for (auto i = 0; i < m_carParam.loops_per_step; ++i) {
    // input of vehicle model
    g_carModel_Hybrid->setExternalInputs(&g_carU_Hybrid);

    // update vehicle model
    g_carModel_Hybrid->step();

    // get output of vehicle model
    g_carY_Hybrid = g_carModel_Hybrid->getExternalOutputs();

    if (std::isnan(g_carY_Hybrid.Body.BdyFrm.Cg.Acc.xddot)) {
      LOG(ERROR) << "ax is NaN, vehicle dynamics quit.\n";
      throw std::runtime_error("ax is NaN, vehicle dynamics quit.");
      return;
    }
  }

  LOG_2 << "vd | softecu brake cmd[0-1]:" << g_carY_Hybrid.SoftEcu_Out_Brake_BrkPrsCmd01
        << ", softecu torque cmd[Nm]:" << g_carY_Hybrid.SoftEcu_Out_ECU_TrqCmd << ".\n";

  // convert to chassis and location
  std::string chassis_payload, loc_payload;
  vehicleToChassis(t_ms, g_carU_Hybrid, g_carY_Hybrid, chassis_payload);
  vehicleToLocation(t_ms, g_carY_Hybrid, loc_payload);
  setMsgByTopic(tx_car::topic::VEHICLE_STATE, chassis_payload);
  setMsgByTopic(tx_car::topic::LOCATION, loc_payload);

  LOG_2 << "vd | chassis_payload size is " << chassis_payload.size() << ", loc_payload size is " << loc_payload.size()
        << "\n";
}
void TxCarImp_Hybrid::stop() {
  g_carModel_Hybrid->terminate();
  mTerrain.releaseRoad();
}

void TxCarImp_Hybrid::controlToVehicle(ExtU_TxCar_Hybrid_T &car_u, const std::string &ctrl_payload) {
  sim_msg::Control ctrl;
  ctrl.ParseFromString(ctrl_payload);
  car_u.DriverIn.acc_control_mode = 0;
  car_u.DriverIn.brake_control_mode = 0;
  car_u.DriverIn.steer_control_mode = 0;

  if (ctrl.contrl_type() == sim_msg::Control_CONTROL_TYPE_ACC_CONTROL) {
    car_u.DriverIn.target_acc_m_s2 = ctrl.acc_cmd().acc();
    car_u.DriverIn.steer_cmd_rad =
        ctrl.acc_cmd().front_wheel_angle() * const_steerRatio_Hybrid * const_flipSteer_Hybrid;
    car_u.DriverIn.gear_cmd__0N1D2R3P = 0;
    if (ctrl.gear_cmd() == sim_msg::Control_GEAR_MODE_REVERSE) {
      car_u.DriverIn.gear_cmd__0N1D2R3P = 2;
    } else if (ctrl.gear_cmd() == sim_msg::Control_GEAR_MODE_DRIVE) {
      car_u.DriverIn.gear_cmd__0N1D2R3P = 1;
    }
  } else {
    LOG_ERROR << "only support acc control now.\n";
  }

  LOG_2 << "vd | control target acc[m|s2]:" << car_u.DriverIn.target_acc_m_s2
        << ", steer_cmd_rad:" << car_u.DriverIn.steer_cmd_rad
        << ", gear_cmd__101:" << static_cast<int32_t>(car_u.DriverIn.gear_cmd__0N1D2R3P) << ".\n";
}
void TxCarImp_Hybrid::controlV2ToVehicle(ExtU_TxCar_Hybrid_T &car_u, const std::string &ctrl_payload) {
  sim_msg::Control_V2 ctrl;
  ctrl.ParseFromString(ctrl_payload);

  // acceleration control
  std::string accControlTypeName =
      sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_Name(ctrl.control_type().acc_control_type());
  if (ctrl.control_type().acc_control_type() ==
      sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_TARGET_ACC_CONTROL) {
    car_u.DriverIn.acc_control_mode = 0;
    car_u.DriverIn.target_acc_m_s2 = ctrl.control_cmd().request_acc();
    car_u.DriverIn.request_torque = 0.0;
  } else if (ctrl.control_type().acc_control_type() ==
             sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_REQUEST_TORQUE_CONTROL) {
    car_u.DriverIn.acc_control_mode = 1;
    car_u.DriverIn.request_torque = ctrl.control_cmd().request_torque();
    LOG_ERROR << "Only Acc control and throttle control are supported in Hybrid vehicle."
              << "\n";
    throw std::runtime_error("Only Acc control and throttle control are supported in Hybrid vehicle.");
  } else if (ctrl.control_type().acc_control_type() ==
             sim_msg::Control_V2_CONTROL_TYPE_ACCEL_CONTROL_TYPE_REQUEST_THROTTLE) {
    // reuse request_torque as throttle in this mode, only works for ICE Car and hybrid car
    if (mCarParam.car_type().propulsion_type() == tx_car::CarType_Propulsion_Type_ICE_Engine) {
      car_u.DriverIn.acc_control_mode = 2;
      car_u.DriverIn.request_torque = ctrl.control_cmd().request_throttle();
    } else {
      LOG_EVERY_N(ERROR, 100) << "only support throttle in ICE and hybrid mode.\n";
    }
  } else {
    LOG_1 << "vd | do not support acceleration control type " << accControlTypeName << "\n";
  }

  // brake control type
  std::string brakeControlTypeName =
      sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_Name(ctrl.control_type().brake_control_type());
  if (ctrl.control_type().brake_control_type() ==
      sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_BRAKE_TARGET_ACC_CONTROL) {
    car_u.DriverIn.brake_control_mode = 0;
    car_u.DriverIn.target_acc_m_s2 = ctrl.control_cmd().request_acc();
    car_u.DriverIn.brake_pressure_01 = 0.0;
  } else if (ctrl.control_type().brake_control_type() ==
             sim_msg::Control_V2_CONTROL_TYPE_BRAKE_CONTROL_TYPE_REQUEST_BRAKE_PRESSURE_CONTROL) {
    car_u.DriverIn.brake_control_mode = 1;
    car_u.DriverIn.brake_pressure_01 = ctrl.control_cmd().request_brake_pressure() * 1e6 / mMaxBrakePressure;
  } else {
    LOG_1 << "vd | do not support brake control type " << brakeControlTypeName << "\n";
  }

  // steer control
  car_u.DriverIn.steer_control_mode = 0;
  std::string steerControlTypeName =
      sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_Name(ctrl.control_type().steer_control_type());
  if (ctrl.control_type().steer_control_type() ==
      sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_TARGET_STEER_ANGLE) {
    car_u.DriverIn.steer_control_mode = 0;
    car_u.DriverIn.steer_cmd_rad = ctrl.control_cmd().request_steer_wheel_angle() * DEG_2_RAD * const_flipSteer_Hybrid;
    car_u.DriverIn.steer_request_torque = 0.0;
  } else if (ctrl.control_type().steer_control_type() ==
             sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_REQUEST_STEER_TORQUE_CONTROL) {
    car_u.DriverIn.steer_control_mode = 1;
    car_u.DriverIn.steer_cmd_rad = 0;
    car_u.DriverIn.steer_request_torque = ctrl.control_cmd().request_steer_torque() * const_flipSteer_Hybrid;
  } else if (ctrl.control_type().steer_control_type() ==
             sim_msg::Control_V2_CONTROL_TYPE_STEER_CONTROL_TYPE_REQUEST_FRONT_WHEEL_ANGLE) {
    car_u.DriverIn.steer_control_mode = 0;
    car_u.DriverIn.steer_cmd_rad =
        ctrl.control_cmd().request_steer_wheel_angle() * DEG_2_RAD * const_flipSteer_Hybrid * const_steerRatio_Hybrid;
    car_u.DriverIn.steer_request_torque = 0.0;
  } else {
    LOG_2 << "vd | do not support steer control type " << steerControlTypeName << "\n";
  }

  if (ctrl.control_flags().flag_1() == 2) {
    LOG_2 << "ctrl.control_flags().flag_1 is " << ctrl.control_flags().flag_1() << ", emergency brake is on.\n";
    g_carU_Hybrid.DriverIn.brake_pressure_01 = 1.0;
  }

  // gear command
  std::string gearCommandName = sim_msg::Control_V2_GEAR_MODE_Name(ctrl.gear_cmd());
  switch (ctrl.gear_cmd()) {
    case sim_msg::Control_V2_GEAR_MODE_NEUTRAL: {
      g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 0;
      break;
    }
    case sim_msg::Control_V2_GEAR_MODE_DRIVE: {
      g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 1;
      break;
    }
    case sim_msg::Control_V2_GEAR_MODE_REVERSE: {
      g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 2;
      break;
    }
    case sim_msg::Control_V2_GEAR_MODE_PARK: {
      g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 3;
      break;
    }
    default: {
      g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P = 0;
      break;
    }
  }

  LOG_2 << "vd | accel control name:" << accControlTypeName
        << ", accel control type:" << static_cast<uint32_T>(g_carU_Hybrid.DriverIn.acc_control_mode)
        << ", request acc[m|s2]:" << g_carU_Hybrid.DriverIn.target_acc_m_s2
        << ", request torque[Nm]:" << g_carU_Hybrid.DriverIn.request_torque << "\n";

  LOG_2 << "vd | brake control name:" << brakeControlTypeName
        << ", brake control type:" << static_cast<uint32_T>(g_carU_Hybrid.DriverIn.brake_control_mode)
        << ", request acc[m|s2]:" << g_carU_Hybrid.DriverIn.target_acc_m_s2
        << ", request brake pressure[01]:" << g_carU_Hybrid.DriverIn.brake_pressure_01 << "\n";

  LOG_2 << "vd | gear mode name:" << gearCommandName
        << ", gear mode value:" << static_cast<uint32_T>(g_carU_Hybrid.DriverIn.gear_cmd__0N1D2R3P) << "\n";

  LOG_2 << "vd | steer type name:" << steerControlTypeName
        << ", steer control type:" << static_cast<uint32_T>(g_carU_Hybrid.DriverIn.steer_control_mode)
        << ", steer wheel angle[rad]:" << g_carU_Hybrid.DriverIn.steer_cmd_rad
        << ", steer request torque[Nm]:" << g_carU_Hybrid.DriverIn.steer_request_torque << "\n";
}

void TxCarImp_Hybrid::vehicleToChassis(double t_ms, const ExtU_TxCar_Hybrid_T &car_u, const ExtY_TxCar_Hybrid_T &vehOut,
                                       std::string &chassis_payload) {
  // lambda function, set wheel information
  auto setWheelInfo = [](sim_msg::VehicleState::Wheel4 *ptrWheel4, const double wheelInfo[4]) {
    ptrWheel4->set_frontleft(wheelInfo[0]);
    ptrWheel4->set_frontright(wheelInfo[1]);
    ptrWheel4->set_rearleft(wheelInfo[2]);
    ptrWheel4->set_rearright(wheelInfo[3]);
  };

  sim_msg::VehicleState vehState;

  vehState.set_time_stamp(t_ms * 0.001);

  // get wheel radius
  double wheel_radius = mCarParam.wheel_tire().tire().wheel_radius().val();
  // set wheel speed
  double wheel_spd_m_s[4];
  for (auto i = 0; i < 4; ++i) {
    wheel_spd_m_s[i] = vehOut.WheelsOut.TireFrame.Omega[i] * wheel_radius;
  }

  // powertrain state
  {
    auto power = vehState.mutable_powertrain_state();

    // set current gear
    if (car_u.DriverIn.gear_cmd__0N1D2R3P == 1) {
      int gearEngaged = vehOut.DriveLineBus_Out_Gear < 0.5 ? 1 : static_cast<int>(vehOut.DriveLineBus_Out_Gear);
      power->set_gear_engaged(gearEngaged);
    }
    if (car_u.DriverIn.gear_cmd__0N1D2R3P == 0) {
      power->set_gear_engaged(0);
    }

    // set engine speed
    power->set_engine_speed(RAD_S_2_RPM * vehOut.DriveLineBus_Out_EngSpd);

    // set gear
    power->set_gead_mode(sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_NEUTRAL);
    if (static_cast<int64_t>(car_u.DriverIn.gear_cmd__0N1D2R3P) == 1) {
      power->set_gead_mode(sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_DRIVE);
    }
    if (static_cast<int64_t>(car_u.DriverIn.gear_cmd__0N1D2R3P) == 2) {
      power->set_gead_mode(sim_msg::VehicleState::GEAR_MODE::VehicleState_GEAR_MODE_REVERSE);
    }

    // set throttle pedal
    const auto &carType = mCarParam.car_type();
    double maxTorque = 300.0;
    if (carType.propulsion_type() == tx_car::CarType_Propulsion_Type_Electric_Drive) {
      maxTorque = mCarParam.ecu().soft_vcu().motormaxtrq().val();
    } else if (carType.propulsion_type() == tx_car::CarType_Propulsion_Type_ICE_Engine) {
      maxTorque = mCarParam.ecu().soft_ems().engmaxtrq().val();
    }
    power->set_accpedal_position(0.0);
    if (vehOut.SoftEcu_Out_ECU_TrqCmd > 1e-3)
      power->set_accpedal_position(vehOut.SoftEcu_Out_ECU_TrqCmd / maxTorque * 100.0);

    // set motor torque and energy source level
    power->set_motorrequesttorque(0.0);
    power->set_motoractualtorque(0.0);
    power->set_energysourcelevel(90.0);
    if (carType.propulsion_type() == tx_car::CarType_Propulsion_Type_Electric_Drive) {
      power->set_motorrequesttorque(vehOut.SoftEcu_Out_ECU_TrqCmd);
      power->set_motoractualtorque(vehOut.PowerBus_Out_mot_front_trqNm + vehOut.PowerBus_Out_mot_rear_trqNm);
      power->set_energysourcelevel(vehOut.PowerBus_Out_battery_info_batt_soc_null * 100.0);  // battery soc, [0,1]
    }

    // transmission output spd or motor spd
    power->set_omegagearspeed(vehOut.DriveLineBus_Out_PropShftSpd);
    if (carType.propulsion_type() == tx_car::CarType_Propulsion_Type_Electric_Drive) {
      power->set_omegagearspeed(vehOut.DriveLineBus_Out_RearMotSpd);
      if (mCarParam.car_type().drivetrain_type() == tx_car::CarType_DriveTrain_Type_FrontWheel_Drive) {
        power->set_omegagearspeed(vehOut.DriveLineBus_Out_FrontMotSpd);
      }
    }
  }

  // chassis state
  {
    auto chassis = vehState.mutable_chassis_state();

    // wheel speed
    setWheelInfo(chassis->mutable_wheel_speed(), wheel_spd_m_s);

    // wheel rot speed
    setWheelInfo(chassis->mutable_wheelrotspd(), vehOut.WheelsOut.TireFrame.Omega);

    // wheel rotation direction
    if (std::abs(wheel_spd_m_s[0]) <= 0.01) {
      chassis->mutable_wheelspindir()->set_fl(sim_msg::VehicleState_Direction_Dir_StandStill);
      chassis->mutable_wheelspindir()->set_fr(sim_msg::VehicleState_Direction_Dir_StandStill);
      chassis->mutable_wheelspindir()->set_rl(sim_msg::VehicleState_Direction_Dir_StandStill);
      chassis->mutable_wheelspindir()->set_rr(sim_msg::VehicleState_Direction_Dir_StandStill);
    } else if (wheel_spd_m_s[0] > 0.01) {
      chassis->mutable_wheelspindir()->set_fl(sim_msg::VehicleState_Direction_Dir_Forward);
      chassis->mutable_wheelspindir()->set_fr(sim_msg::VehicleState_Direction_Dir_Forward);
      chassis->mutable_wheelspindir()->set_rl(sim_msg::VehicleState_Direction_Dir_Forward);
      chassis->mutable_wheelspindir()->set_rr(sim_msg::VehicleState_Direction_Dir_Forward);
    } else {
      chassis->mutable_wheelspindir()->set_fl(sim_msg::VehicleState_Direction_Dir_Backward);
      chassis->mutable_wheelspindir()->set_fr(sim_msg::VehicleState_Direction_Dir_Backward);
      chassis->mutable_wheelspindir()->set_rl(sim_msg::VehicleState_Direction_Dir_Backward);
      chassis->mutable_wheelspindir()->set_rr(sim_msg::VehicleState_Direction_Dir_Backward);
    }

    chassis->set_steeringwheelspd(0.0);
    chassis->set_steeringwheeltorque(0.0);
    double brakePedalPos = vehOut.SoftEcu_Out_Brake_BrkPrsCmd01 * 100.0;
    brakePedalPos = brakePedalPos < 0.1 ? 0.0 : brakePedalPos;
    chassis->set_brakepedalpos(brakePedalPos);

    // set steer wheel angle, in haval h7, chassis feedback steerwheelangle is front wheel angle
    chassis->set_steeringwheelangle(const_flipSteer_Hybrid *
                                    (vehOut.WheelsOut.Steering.WhlAng[0] + vehOut.WheelsOut.Steering.WhlAng[1]) / 2.0);
    chassis->set_steeringwheelanglesign(const_flipSteer_Hybrid * car_u.DriverIn.steer_cmd_rad * RAD_2_DEG);

    LOG_2 << "vd | chassis front wheel angle[rad]:" << chassis->steeringwheelangle()
          << ", chassis front wheel angle[deg]:" << chassis->steeringwheelanglesign() << "\n";

    // set esp master cylinder brake pressure, unit MPa
    chassis->set_esp_mastercylindbrakepress(vehOut.SoftEcu_Out_Brake_brake_pressure_Pa[0] / 1e6);

    // set yaw_rate
    Eigen::Vector3d flu_angular = {vehOut.Body.BdyFrm.Cg.AngVel.p, -vehOut.Body.BdyFrm.Cg.AngVel.q,
                                   -vehOut.Body.BdyFrm.Cg.AngVel.r};
    chassis->set_vehdynyawrate(flu_angular[2]);

    // vehicle curvature
    chassis->set_vehcurvature(1.0 / 4.5);

    // set wheel contact
    {
      auto wheel_contact = chassis->mutable_wheel_contact();
      auto SetWheelContact = [this](sim_msg::Vec3 *wheel_contact, const ExtY_TxCar_Hybrid_T &vehOut,
                                    Eigen::Vector3d &pos) {
        coord_trans_api::enu2lonlat(pos[0], pos[1], pos[2], this->mMapOrigin.x, this->mMapOrigin.y, this->mMapOrigin.z);
        wheel_contact->set_x(pos[0]);
        wheel_contact->set_y(pos[1]);
        wheel_contact->set_z(pos[2]);
      };
      Eigen::Vector3d &&fl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, mHalfTrack, m_mu);
      Eigen::Vector3d &&fr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, mCgToFrontAxle, -mHalfTrack, m_mu);
      Eigen::Vector3d &&rl_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, mHalfTrack, m_mu);
      Eigen::Vector3d &&rr_z = calWheelsZ(m_Pos_ego_ENU, m_ROT_ego_ENU, -mCgToRearAxle, -mHalfTrack, m_mu);

      // sin slope, body frame, sin value of pitch angle, front higher means positive
      vehState.mutable_body_state()->set_sinslope(std::sin((fl_z[2] - rl_z[2]) / mWheelBase));

      SetWheelContact(wheel_contact->mutable_fl(), vehOut, fl_z);
      SetWheelContact(wheel_contact->mutable_fr(), vehOut, fr_z);
      SetWheelContact(wheel_contact->mutable_rl(), vehOut, rl_z);
      SetWheelContact(wheel_contact->mutable_rr(), vehOut, rr_z);
    }
  }

  // body state
  {
    auto body = vehState.mutable_body_state();

    // body frame velocity
    body->mutable_bodyframevelocity()->set_x(vehOut.Body.BdyFrm.Cg.Vel.xdot);
    body->mutable_bodyframevelocity()->set_y(-vehOut.Body.BdyFrm.Cg.Vel.ydot);
    body->mutable_bodyframevelocity()->set_z(0.0);

    // vehicle direction
    if (std::abs(wheel_spd_m_s[0]) <= 0.01) {
      body->set_vehdirection(sim_msg::VehicleState_Direction_Dir_StandStill);
    } else if (wheel_spd_m_s[0] > 0.01) {
      body->set_vehdirection(sim_msg::VehicleState_Direction_Dir_Forward);
    } else {
      body->set_vehdirection(sim_msg::VehicleState_Direction_Dir_Backward);
    }

    // slip angle
    body->set_slipangle(-vehOut.Body.BdyFrm.Geom.Ang.Beta);

    // brake pedal state
    body->set_brakepedsts(false);
    if (vehOut.SoftEcu_Out_Brake_BrkPrsCmd01 > 1e-3) body->set_brakepedsts(true);
  }

  LOG_2 << "vd | engine trq[Nm]:" << vehOut.PowerBus_Out_eng_trqNm
        << ", front motor trq[Nm]:" << vehOut.PowerBus_Out_mot_front_trqNm
        << ", rear motor trq[Nm]:" << vehOut.PowerBus_Out_mot_rear_trqNm
        << ", batter soc[0-1]:" << vehOut.PowerBus_Out_battery_info_batt_soc_null
        << ", battery voltage[V]:" << vehOut.PowerBus_Out_battery_info_batt_volt_V << ".\n";

  LOG_2 << "vd | engaged gear:" << vehOut.DriveLineBus_Out_Gear
        << ", engine spd[rad|s]:" << vehOut.DriveLineBus_Out_EngSpd
        << ", front motor spd[rad|s]:" << vehOut.DriveLineBus_Out_FrontMotSpd
        << ", rear motor spd[rad|s]:" << vehOut.DriveLineBus_Out_RearMotSpd << ".\n";

  chassis_payload.clear();
  vehState.SerializeToString(&chassis_payload);
}
void TxCarImp_Hybrid::vehicleToLocation(double t_ms, const ExtY_TxCar_Hybrid_T &vehOut, std::string &loc_payload) {
  sim_msg::Location location;
  sim_msg::Vec3 *ptrLoc = location.mutable_position();

  // set t
  location.set_t(t_ms * 0.001);

  double speedFlat = std::hypot(vehOut.Body.InertFrm.Cg.Vel.Xdot, vehOut.Body.InertFrm.Cg.Vel.Ydot);

  // update roll pitch yaw
  sim_msg::Vec3 *ptrRPY = location.mutable_rpy();

  TxCarEuler flu_euler = {vehOut.Body.InertFrm.Cg.Ang.phi, -vehOut.Body.InertFrm.Cg.Ang.theta,
                          -vehOut.Body.InertFrm.Cg.Ang.psi};  // m_math.FRD_To_FLU(frd_euler);
  m_ROT_ego_ENU = TxCarMath::toQuaternion(flu_euler).toRotationMatrix();
  flu_euler.roll = std::atan2(std::sin(flu_euler.roll), std::cos(flu_euler.roll));
  flu_euler.pitch = std::atan2(std::sin(flu_euler.pitch), std::cos(flu_euler.pitch));
  flu_euler.yaw = std::atan2(std::sin(flu_euler.yaw), std::cos(flu_euler.yaw));
  flu_euler.yaw = flu_euler.yaw < 0.0 ? 2 * M_PI + flu_euler.yaw : flu_euler.yaw;
  /*
  tx_car::TxCarMath::saturation(flu_euler.roll);
  tx_car::TxCarMath::saturation(flu_euler.pitch);
  tx_car::TxCarMath::saturation(flu_euler.yaw);
  */
  ptrRPY->set_x(flu_euler.roll);
  ptrRPY->set_y(flu_euler.pitch);
  ptrRPY->set_z(flu_euler.yaw);  // [0,2*pi]
  LOG_2 << "vd | flu roll:" << flu_euler.roll << ", flu pitch:" << flu_euler.pitch << ", flu yaw:" << flu_euler.yaw
        << ".\n";

  // FRD position
  double xo = 0.0, yo = 0.0, zo = 0.0;
  xo = vehOut.Body.InertFrm.Cg.Disp.X;
  yo = vehOut.Body.InertFrm.Cg.Disp.Y;
  zo = vehOut.Body.InertFrm.Cg.Disp.Z;

  // get position
  Eigen::Vector3d flu_pos = {xo, -yo, -zo};  // m_math.FRD_To_FLU(frd_pos);
  m_Pos_ego_ENU = flu_pos;
  LOG_2 << "vd | enu pos x:" << flu_pos[0] << ", frd y:" << flu_pos[1] << ", frd z:" << flu_pos[2] << ".\n";

  // update angular speed(rad/s)
  sim_msg::Vec3 *ptrAngular = location.mutable_angular();

  Eigen::Vector3d flu_angular = {vehOut.Body.BdyFrm.Cg.AngVel.p, -vehOut.Body.BdyFrm.Cg.AngVel.q,
                                 -vehOut.Body.BdyFrm.Cg.AngVel.r};
  ptrAngular->set_x(flu_angular[0]);
  ptrAngular->set_y(flu_angular[1]);
  ptrAngular->set_z(flu_angular[2]);

  // velocity in inertia coordinate
  Eigen::Vector3d enu_velocity = {vehOut.Body.InertFrm.Cg.Vel.Xdot, -vehOut.Body.InertFrm.Cg.Vel.Ydot,
                                  -vehOut.Body.InertFrm.Cg.Vel.Zdot};

  // update velocity, use marco to detemine if using z info
  sim_msg::Vec3 *ptrVel = location.mutable_velocity();

  TxCarMath::saturation(enu_velocity[0], -1e-3, 1e-3);
  ptrVel->set_x(enu_velocity[0]);
  ptrVel->set_y(enu_velocity[1]);
  ptrVel->set_z(enu_velocity[2]);
  LOG_2 << "vd | enu, vx is " << enu_velocity[0] << ", vy is " << enu_velocity[1] << ", vz is " << enu_velocity[2]
        << "\n";

  Eigen::Vector3d flu_velocity = {vehOut.Body.BdyFrm.Cg.Vel.xdot, -vehOut.Body.BdyFrm.Cg.Vel.ydot,
                                  -vehOut.Body.BdyFrm.Cg.Vel.zdot};
  LOG_2 << "vd | flu, vx is " << flu_velocity[0] << ", vy is " << flu_velocity[1] << ", vz is " << flu_velocity[2]
        << "\n";

  // TxCarMath::saturation(enu_velocity, -0.01, 0.01);

  // acc
  sim_msg::Vec3 *ptrAccel = location.mutable_acceleration();

  Eigen::Vector3d frd_acc = {vehOut.Body.BdyFrm.Cg.Acc.ax, vehOut.Body.BdyFrm.Cg.Acc.ay, vehOut.Body.BdyFrm.Cg.Acc.az};
  ptrAccel->set_x(frd_acc[0] * constGravity);
  ptrAccel->set_y(-1.0 * frd_acc[1] * constGravity);
  ptrAccel->set_z(-1.0 * frd_acc[2] * constGravity);

  // use vehicle geometory and get velocity/acc of ego certain point offset from CG
  {
    Eigen::Vector3d offsetFromCG = getOffsetFromCG();
    LOG_2 << "vd | offsetFromCG:[" << offsetFromCG[0] << ", " << offsetFromCG[1] << ", " << offsetFromCG[2] << "].\n";
    offsetFromCG[2] = 0.0;

    Eigen::Vector3d bodyVelocity = {vehOut.Body.BdyFrm.Cg.Vel.xdot, -vehOut.Body.BdyFrm.Cg.Vel.ydot,
                                    -vehOut.Body.BdyFrm.Cg.Vel.zdot};
    Eigen::Vector3d bodyAcc = {ptrAccel->x(), ptrAccel->y(), ptrAccel->z()};
    Eigen::Vector3d bodyOmega = {ptrAngular->x(), ptrAngular->y(), ptrAngular->z()};
    Eigen::Vector3d bodyOmegaAcc = {vehOut.Body.BdyFrm.Cg.AngAcc.pdot, -vehOut.Body.BdyFrm.Cg.AngAcc.qdot,
                                    -vehOut.Body.BdyFrm.Cg.AngAcc.rdot};

    bodyVelocity = bodyVelocity + bodyOmega.cross(offsetFromCG);
    bodyAcc = bodyAcc + bodyOmegaAcc.cross(offsetFromCG) + bodyOmega.cross(bodyOmega.cross(offsetFromCG));
    // flu_pos = flu_pos + m_ROT_ego_ENU * offsetFromCG;

    LOG_2 << "vd | bodyVelocity vx is " << bodyVelocity[0] << ", vy is " << bodyVelocity[1] << ", vz is "
          << bodyVelocity[2] << "\n";

    // set position
    double mu = 0.9;
    flu_pos[2] = calWheelsZ(flu_pos, m_ROT_ego_ENU, 0.0, 0.0, mu)[2];
    LOG_2 << "vd | location ENU x: " << flu_pos[0] << ", y: " << flu_pos[1] << ", z: " << flu_pos[2] << "\n";
    coord_trans_api::enu2lonlat(flu_pos[0], flu_pos[1], flu_pos[2], mMapOrigin.x, mMapOrigin.y, mMapOrigin.z);
    LOG_2 << "vd | location GPS x: " << flu_pos[0] << ", y: " << flu_pos[1] << ", z: " << flu_pos[2] << "\n";

    ptrLoc->set_x(flu_pos[0]);
    ptrLoc->set_y(flu_pos[1]);
    if (mTerrain.isUsingTerrain())
      ptrLoc->set_z(flu_pos[2]);
    else
      ptrLoc->set_z(mMapOrigin.z);

    // set velocity
    Eigen::Vector3d inertVelocity = m_ROT_ego_ENU * bodyVelocity;
    ptrVel->set_x(inertVelocity[0]);
    ptrVel->set_y(inertVelocity[1]);
    ptrVel->set_z(inertVelocity[2]);

    // set acc
    ptrAccel->set_x(bodyAcc[0]);
    ptrAccel->set_y(bodyAcc[1]);
    ptrAccel->set_z(bodyAcc[2]);

    /// stable acc when speed lower than 1.0 m/s
    auto speed = std::hypot(enu_velocity[0], enu_velocity[1]);
    if (speed < 1.0 || t_ms < const_StableT_ms_Hybrid) {
      ptrAccel->set_x((bodyVelocity[0] - mPreVelocity[0]) / 0.01);
      ptrAccel->set_y((bodyVelocity[1] - mPreVelocity[1]) / 0.01);
      ptrAccel->set_z((bodyVelocity[2] - mPreVelocity[2]) / 0.01);
    }
    mPreVelocity = bodyVelocity;
  }

  // print
  LOG_2 << "vd | location, avx:" << location.angular().x() << ", avy:" << location.angular().y()
        << ", avz:" << location.angular().z() << "\n";
  LOG_2 << "vd | location, roll:" << location.rpy().x() << ", pitch:" << location.rpy().y()
        << ", yaw:" << location.rpy().z() << "\n";
  LOG_2 << "vd | location, vx is " << location.velocity().x() << ", vy is " << location.velocity().y() << ", vz is "
        << location.velocity().z() << "\n";
  LOG_2 << "vd | location, ax is " << location.acceleration().x() << ", ay is " << location.acceleration().y()
        << ", az:" << location.acceleration().z() << ".\n";
  LOG_2 << "vd | location, vflat is " << std::hypot(location.velocity().x(), location.velocity().y()) << "\n";
  LOG_2 << "vd | location, ax is " << location.acceleration().x() << ", ay is " << location.acceleration().y()
        << ", az:" << location.acceleration().z() << ".\n";
  LOG_2 << "vd | location, aflat is " << std::hypot(location.acceleration().x(), location.acceleration().y()) << ".\n";

  loc_payload.clear();
  location.SerializeToString(&loc_payload);
}

Eigen::Vector3d TxCarImp_Hybrid::getOffsetFromCG() {
  Eigen::Vector3d offset = {0.0, 0.0, 0.0};

  if (m_carParam.vehicle_geometory_payload.size() > 0) {
    const sim_msg::VehicleGeometory &geometry = m_geometry_comb.front();
    if (geometry.coord_type() == sim_msg::VehicleGeometory_VehicleCoordType_RearAxle_SnappedToGround) {
      offset = {-mCgToRearAxle, 0.0, -mCgHeight};
    } else if (geometry.coord_type() == sim_msg::VehicleGeometory_VehicleCoordType_WheelBaseCenter_SnappedToGround) {
      offset = {-(mCgToRearAxle - mWheelBase / 2.0), 0.0, -mCgHeight};
    }
  }

  return offset;
}

Eigen::Vector3d TxCarImp_Hybrid::calWheelsZ(const Eigen::Vector3d &FLU_Pos, const Eigen::Matrix3d &FLU_Rot,
                                            double l_offset, double w_offset, double &m_mu) {
  Eigen::Vector3d pos_with_z = FLU_Rot * Eigen::Vector3d(l_offset, w_offset, 0.0) + FLU_Pos;
  double dzdy = 0.0, dzdx = 0.0;
  terrain::CTerrainInterface::getHeightAtWorldPosition(pos_with_z[0], pos_with_z[1], pos_with_z[2], &pos_with_z[2],
                                                       &dzdy, &dzdx, &m_mu);
  return pos_with_z;
}
}  // namespace tx_car

// declare vehicle model funtion
Declare_Vehicle_Model(tx_car::TxCarImp_Hybrid)
