/**
 * @file msg.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "msg.h"
#include <google/protobuf/util/json_util.h>
#include <Eigen/Eigen>
#include "catalog.h"
#include "asn1msg/asn1msg_bsm2020.h"
#include "asn1msg/asn1msg_map2020.h"
#include "asn1msg/asn1msg_rsi2020.h"
#include "asn1msg/asn1msg_rsm2020.h"
#include "asn1msg/asn1msg_spat2020.h"
#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "pblog/tx_sim_log_common.h"
#include "settings.pb.h"
#include "v2x_asn1_2020.pb.h"

std::shared_ptr<Msg> msgMaker;

#define TOPIC_ASN1 "V2XASN1"

/**
 * @brief init of msg
 *
 * @param helper helper of txsim
 */
void Msg::Init(tx_sim::InitHelper &helper) {
  helper.Publish(TOPIC_ASN1);

  mMessages.clear();
  auto v2x_version = helper.GetParameter("V2xVersion");
  if (v2x_version == "2020" || v2x_version == "") {  // default
    mMessages[MN_BSM] = std::make_shared<CAsn1MsgBSM2020>();
    mMessages[MN_MAP] = std::make_shared<CAsn1MsgMAP2020>();
    mMessages[MN_RSI] = std::make_shared<CAsn1MsgRSI2020>();
    mMessages[MN_RSM] = std::make_shared<CAsn1MsgRSM2020>();
    mMessages[MN_SPAT] = std::make_shared<CAsn1MsgSPAT2020>();
  } else if (v2x_version == "2019") {
    /* code */
  } else if (v2x_version == "2017") {
    /* code */
  } else {
    LOG(ERROR) << "V2X VERSION NOT SUPPORT: " << v2x_version;
  }
  for (const auto &msg : mMessages) {
    msg.second->Init(helper);
  }
  m_data_logger.Init(helper);
  std::string sInputJson = helper.GetParameter("json");
  if (sInputJson == "false") m_bJson = false;
}

/**
 * @brief reset of msg
 *
 * @param helper helper of txsim
 */
void Msg::Reset(tx_sim::ResetHelper &helper) {
  for (const auto &msg : mMessages) {
    msg.second->Reset(helper);
  }
  m_data_logger.Reset(helper);
}

/**
 * @brief step of msg
 *
 * @param helper helper of txsim
 */
void Msg::Step(tx_sim::StepHelper &helper) {
  v2x_asn1_2020::V2X_ASN1 v2xMsgs;
  for (const auto &msg : mMessages) {
    msg.second->Convert(&v2xMsgs);
  }

  // if (v2xMsgs.bsm(0).v2xbsms_size() > 0) {
  //   std::cout << "Msg::Step: bsm Publish tracking: Total publish="
  //             << "; bsm_size=" << v2xMsgs.bsm(0).v2xbsms_size()
  //             << "  bsm(0).v2xbsms(0).x()=" << v2xMsgs.bsm(0).v2xbsms(0).position3d().latitude() << std::endl;
  // }

  // if (v2xMsgs.rsm_size() > 0 && v2xMsgs.rsm(0).participantlist_size()>0) {
  //   std::cout << "Msg::Step: rsm Publish tracking: Total publish="
  //               << "; rsm_size=" << v2xMsgs.rsm_size()
  //               << ";rsm(0).participantlist(0).y=" << v2xMsgs.rsm(0).participantlist(0).position3d().latitude() <<
  //               std::endl;
  // }

  std::string buf;
  v2xMsgs.SerializeToString(&buf);
  helper.PublishMessage(TOPIC_ASN1, buf);

  if (m_bJson) {
    buf.clear();
    google::protobuf::util::MessageToJsonString(v2xMsgs, &buf);
  }
  m_data_logger.ProducePBLogEvent(TOPIC_ASN1, buf, helper.timestamp());

  // update data logger one step
  m_data_logger.Step(helper);
  // debug
  // std::cout << "[v2xMsgs is]: " << v2xMsgs.DebugString();
}

/**
 * @brief stop of msg
 *
 * @param helper helper of txsim
 */
void Msg::Stop(tx_sim::StopHelper &helper) { m_data_logger.Stop(helper); }

/**
 * @brief get instance
 *
 * @return Msg&
 */
Msg &Msg::getInstance() {
  static Msg msg;
  return msg;
}

/**
 * @brief clear all message
 *
 */
void Msg::Clear() {
  for (const auto &msg : mMessages) {
    msg.second->Clear();
  }
}

/**
 * @brief get bsm message
 *
 * @return CAsn1MsgBSM*
 */
CAsn1MsgBSM *Msg::getBSM() { return reinterpret_cast<CAsn1MsgBSM *>(mMessages[MN_BSM].get()); }

/**
 * @brief get map message
 *
 * @return CAsn1MsgMAP*
 */
CAsn1MsgMAP *Msg::getMAP() { return reinterpret_cast<CAsn1MsgMAP *>(mMessages[MN_MAP].get()); }

/**
 * @brief get RSI message
 *
 * @return CAsn1MsgRSI*
 */
CAsn1MsgRSI *Msg::getRSI() { return reinterpret_cast<CAsn1MsgRSI *>(mMessages[MN_RSI].get()); }

/**
 * @brief get RSM message
 *
 * @return CAsn1MsgRSM*
 */
CAsn1MsgRSM *Msg::getRSM() { return reinterpret_cast<CAsn1MsgRSM *>(mMessages[MN_RSM].get()); }

/**
 * @brief get SPAT message
 *
 * @return CAsn1MsgSPAT*
 */
CAsn1MsgSPAT *Msg::getSPAT() { return reinterpret_cast<CAsn1MsgSPAT *>(mMessages[MN_SPAT].get()); }

sim_msg::Car location2car(const sim_msg::Location &loc, std::int64_t id) {
  sim_msg::Car car;
  car.set_id(-id);
  car.set_t(loc.t() * 1000);
  car.set_x(loc.position().x());
  car.set_y(loc.position().y());
  car.set_z(loc.position().z());
  car.set_heading(loc.rpy().z());
  car.set_theta(loc.rpy().z());
  Eigen::Quaterniond q = Eigen::AngleAxisd(loc.rpy().z(), Eigen::Vector3d::UnitZ()) *
                         Eigen::AngleAxisd(loc.rpy().y(), Eigen::Vector3d::UnitY()) *
                         Eigen::AngleAxisd(loc.rpy().x(), Eigen::Vector3d::UnitX());
  Eigen::Vector3d velo(loc.velocity().x(), loc.velocity().y(), loc.velocity().z());
  Eigen::Vector3d acc(loc.acceleration().x(), loc.acceleration().y(), loc.acceleration().z());
  car.set_v(velo.dot(q * Eigen::Vector3d(1, 0, 0)));
  car.set_acc(acc.dot(q * Eigen::Vector3d(1, 0, 0)));
  Eigen::Quaterniond ql = Eigen::AngleAxisd(EIGEN_PI * 0.5, Eigen::Vector3d::UnitZ()) * q;
  car.set_vl(velo.dot(ql * Eigen::Vector3d(1, 0, 0)));
  car.set_acc_l(acc.dot(ql * Eigen::Vector3d(1, 0, 0)));
  auto fd = Catalog::getInstance()._objects.find(std::make_pair(-1, static_cast<int>(id)));
  if (fd != Catalog::getInstance()._objects.end()) {
    double minx = std::numeric_limits<double>::max();
    double miny = std::numeric_limits<double>::max();
    double minz = std::numeric_limits<double>::max();
    double maxx = std::numeric_limits<double>::min();
    double maxy = std::numeric_limits<double>::min();
    double maxz = std::numeric_limits<double>::min();

    for (auto &p : fd->second.models) {
      minx = std::min(minx, p.cen.x() - p.l * 0.5);
      miny = std::min(miny, p.cen.y() - p.w * 0.5);
      minz = std::min(minz, p.cen.z() - p.h * 0.5);
      maxx = std::max(maxx, p.cen.x() + p.l * 0.5);
      maxy = std::max(maxy, p.cen.y() + p.w * 0.5);
      maxz = std::max(maxz, p.cen.z() + p.h * 0.5);
    }
    car.set_width(maxy - miny);
    car.set_length(maxx - minx);
    car.set_height(maxz - minz);
  }
  return car;
}
