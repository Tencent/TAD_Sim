// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txcar_base.h"
#include "inc/car_log.h"

#include <algorithm>

#ifdef _WIN32
#  define NOMINMAX
#  include <Windows.h>
#else
#  include <unistd.h>
#endif  // _WIN32

namespace tx_car {
TxCarInit::TxCarInit(double x, double y, double z, double vx, double vy, double vz, double yaw, GearOperator gear_op) {
  init_x = x;
  init_y = y;
  init_z = z;
  init_vx = vx;
  init_vy = vy;
  init_vz = vz;
  init_yaw = yaw;
  init_gear_op = gear_op;

  loops_per_step = 10;
  param_path = "";
  enable_terrain = true;
  start_loc_payload = "";
  hadmap_path = "";
  mu = 0.9;
  vehicle_geometory_payload = "";
}

void TxCarInit::clear() {
  loops_per_step = 10;
  init_x = 0.0;
  init_y = 0.0;
  init_z = 0.0;
  init_vx = 0.0;
  init_vy = 0.0;
  init_vz = 0.0;
  init_roll = 0.0;
  init_pitch = 0.0;
  init_yaw = 0.0;
  init_gear_op = GearOperator::Netural;

  param_path = "";
  enable_terrain = true;
  start_loc_payload = "";
  hadmap_path = "";
  mu = 0.9;
  vehicle_geometory_payload = "";
}

void TxCarInit::debugShow(const TxCarInit& carInit) {
  LOG_0 << "vd | parameter file " << carInit.param_path << "\n";
  LOG_0 << "vd | init x:" << carInit.init_x << ", y:" << carInit.init_y << ", z:" << carInit.init_z
        << ", vx:" << carInit.init_vx << ", vy:" << carInit.init_vy << ", vz:" << carInit.init_vz
        << ", roll:" << carInit.init_roll << ", pitch:" << carInit.init_pitch << ", yaw:" << carInit.init_yaw << "\n";
}

///////////////////////
MsgWatchDog::MsgWatchDog() : m_prePayload(""), m_missingCounter(false), m_bMissing(false) {}

void MsgWatchDog::feed(const std::string& payload) {
  if (payload.size() == 0) {
    m_missingCounter++;
    m_stopCounter++;
    m_missingCounter = std::min(m_missingCounter, const_missingCounterThresh);
  } else {
    if (payload == m_prePayload) {
      m_missingCounter++;
      m_stopCounter++;
      m_missingCounter = std::min(m_missingCounter, const_missingCounterThresh);
    } else {
      m_prePayload = payload;
      m_missingCounter -= 10;
      m_stopCounter = 0;
      m_missingCounter = std::max(m_missingCounter, 0);
    }
  }
  if (m_missingCounter >= const_missingCounterThresh) {
    m_bMissing = true;
  } else {
    m_bMissing = false;
  }
}

bool MsgWatchDog::requestStop(int stopThresh) { return stopThresh > 0 && m_stopCounter >= stopThresh; }

TxCarBase::TxCarBase() : m_processID("None") { calProcessID(); }
std::string TxCarBase::getMsgByTopic(const std::string& topic) {
  auto iter = mMsgs.find(topic);
  if (iter != mMsgs.end()) return iter->second;
  return std::string("");
}
void TxCarBase::setMsgByTopic(const std::string& topic, const std::string& payload) { mMsgs[topic] = payload; }
void TxCarBase::setLogLevel(int flags_v) { FLAGS_v = flags_v; }
void TxCarBase::calProcessID() {
  uint64_t processID = 0;
#ifdef _WIN32
  auto id = GetCurrentProcessId();
  processID = static_cast<uint64_t>(id);
#else
  auto id = getpid();
  processID = static_cast<uint64_t>(id);
#endif
  m_processID = std::to_string(processID);
}
}  // namespace tx_car
