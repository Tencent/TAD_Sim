/**
 * @file asn1msg_rsm2020.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "asn1msg_rsm2020.h"
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <fstream>
#include "json/json.h"
#include "merge_proto.h"
#include "v2x_asn1_2020.pb.h"

/**
 * @brief cover rsm message
 *
 * @param msg message to cover
 * @return true on success
 * @return false
 */
bool CAsn1MsgRSM2020::Convert(google::protobuf::Message *msg) {
  v2x_asn1_2020::V2X_ASN1 *v2x_asn1_2020 = (v2x_asn1_2020::V2X_ASN1 *)msg;
  for (const auto &rsu : data.rsu) {
    v2x_asn1_2020::V2XRSMMsg *prsmmsg = v2x_asn1_2020->mutable_rsm()->Add();
    // recover from prelabelled json string
    CoverRSM(rsu.PreRsu, prsmmsg);
    // if has no prelabel
    if (prsmmsg->participantlist_size() == 0) {
      if (rsu.participant_cars.empty() && rsu.participant_dynamicObstacles.empty()) {
        continue;
      }
      Convert_RSM(prsmmsg, rsu);
    }
  }
  // std::cout << "CAsn1MsgRSM2020 Convert end" << std::endl;
  return true;
}

/**
 * @brief load car information into protobuf
 *
 * @param pbsmmsg message out
 * @param car car to load
 */
void CAsn1MsgRSM2020::LoadCarInfo(v2x_asn1_2020::V2XRSMMsg *pbsmmsg, const sim_msg::Car &car,
                                  const Eigen::Vector3d &pos) {
  v2x_asn1_2020::ParticipantData *participantData_;
  participantData_ = pbsmmsg->add_participantlist();
  // set type, id
  participantData_->set_participanttype(0);  // Car - 0; DynamicObstacle - 1;
  participantData_->set_paticipantid(car.id());
  participantData_->set_sourcetype(car.type());
  participantData_->set_vehicleid(car.id());
  participantData_->set_plateno(0);
  participantData_->set_dsecond((std::size_t)car.t() % 60000);
  // set position
  // double -> int
  v2x_asn1_2020::Position2D *p = participantData_->mutable_positionoffsetllv()->mutable_positionoffsetll()->Add();
  p->set_latitude((car.y() - pos.y()) * 1E7);
  p->set_longitude((car.x() - pos.x()) * 1E7);
  participantData_->mutable_positionoffsetllv()->set_verticaloffset((car.z() - pos.z()) * 10);
  // set velocity
  participantData_->set_transmissionstate(1);
  participantData_->set_speed(sqrt(car.v() * car.v() + car.vl() * car.vl()) * 50);
  participantData_->set_heading(
      ((90 + 360 - static_cast<int>(std::round(car.heading() * 180. / 3.141592653589793238462643383))) % 360) * 80);
  participantData_->set_steeringwheelangle(0);
  // set vehicle size
  // unit: cm
  participantData_->mutable_vehiclesize()->set_vehiclelength(car.length() * 100);
  participantData_->mutable_vehiclesize()->set_vehiclewidth(car.width() * 100);
  participantData_->mutable_vehiclesize()->set_vehicleheight(car.height() * 100);
}

/**
 * @brief load dynamic obstacle information into protobuf
 *
 * @param pbsmmsg message out
 * @param dynamicObstacle dynamic obstacle to load
 */
void CAsn1MsgRSM2020::LoadDynamicObstacleInfo(v2x_asn1_2020::V2XRSMMsg *pbsmmsg,
                                              const sim_msg::DynamicObstacle &dynamicObstacle,
                                              const Eigen::Vector3d &pos) {
  v2x_asn1_2020::ParticipantData *participantData_;
  participantData_ = pbsmmsg->add_participantlist();
  // set id, type...
  participantData_->set_participanttype(1);  // Car - 0; DynamicObstacle - 1;
  participantData_->set_paticipantid(dynamicObstacle.id());
  participantData_->set_sourcetype(dynamicObstacle.type());
  participantData_->set_vehicleid(dynamicObstacle.id());
  participantData_->set_plateno(0);
  participantData_->set_dsecond((std::size_t)dynamicObstacle.t() % 60000);
  // set position
  // double -> int
  v2x_asn1_2020::Position2D *p = participantData_->mutable_positionoffsetllv()->mutable_positionoffsetll()->Add();
  p->set_latitude((dynamicObstacle.y() - pos.y()) * 1E7);
  p->set_longitude((dynamicObstacle.x() - pos.x()) * 1E7);
  participantData_->mutable_positionoffsetllv()->set_verticaloffset((dynamicObstacle.z() - pos.z()) * 10);
  // set velocity
  participantData_->set_transmissionstate(1);
  participantData_->set_speed(dynamicObstacle.v() * 50);
  participantData_->set_heading(
      ((90 + 360 - static_cast<int>(std::round(dynamicObstacle.heading() *
      180. / 3.141592653589793238462643383))) %
       360) *
      80);
  participantData_->set_steeringwheelangle(0);
  // set vehicle size
  // unit: cm
  participantData_->mutable_vehiclesize()->set_vehiclelength(dynamicObstacle.length() * 100);
  participantData_->mutable_vehiclesize()->set_vehiclewidth(dynamicObstacle.width() * 100);
  participantData_->mutable_vehiclesize()->set_vehicleheight(dynamicObstacle.height() * 100);
}

/**
 * @brief recover RSM message from traffic
 *
 * @param pbsmmsg message out
 * @param buffer car and dynamic obstacle data
 */
void CAsn1MsgRSM2020::Convert_RSM(v2x_asn1_2020::V2XRSMMsg *pbsmmsg, const V2XDataRSM::RsuBuff &buffer) {
  // std::cout << "CAsn1MsgRSM2020 Convert: rsm_size = "
  //           << buffer.participant_cars.size() +
  //                  buffer.participant_dynamicObstacles.size()
  //           << std::endl;
  pbsmmsg->set_msgcount(0);
  pbsmmsg->set_rsuid(0);
  // double -> int
  pbsmmsg->mutable_position3d()->set_latitude(buffer.pos.x() * 1E7);
  pbsmmsg->mutable_position3d()->set_longitude(buffer.pos.x() * 1E7);
  pbsmmsg->mutable_position3d()->set_elevation(buffer.pos.z() * 10);
  // load cars
  for (const auto &participant_car : buffer.participant_cars) {
    const auto &car = participant_car;
    LoadCarInfo(pbsmmsg, car, buffer.pos);
  }
  // load dynamic obstacles
  for (const auto &participant_dynamicObstacle : buffer.participant_dynamicObstacles) {
    const auto &dynamicObstacle = participant_dynamicObstacle;
    LoadDynamicObstacleInfo(pbsmmsg, dynamicObstacle, buffer.pos);
  }
}

/**
 * @brief write prelabel example
 *
 * @param fpath path to save
 */
void CAsn1MsgRSM2020::WritePreJsonExample(const std::string &fpath) {
  v2x_asn1_2020::V2XRSMMsg msg;
  // fill by random
  FillupProto(&msg);
  std::string buffer;
  // protobuf to json
  google::protobuf::util::MessageToJsonString(msg, &buffer);
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(buffer.c_str(), buffer.c_str() + buffer.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSM faild: " << err;
    return;
  }
  Json::Value prejson;
  prejson["RSM"] = tree;
  Json::StreamWriterBuilder builder;
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::ofstream emp(fpath);
  writer->write(prejson, &emp);
  emp.close();
}

/**
 * @brief recover from prelabel buffer
 *
 * @param preRSU prelabel rsu buffer
 * @param pMsg to be recover
 */
void CAsn1MsgRSM2020::CoverRSM(const std::string &preRSU, v2x_asn1_2020::V2XRSMMsg *pMsg) {
  if (preRSU.empty()) return;
  // json to message
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(preRSU.c_str(), preRSU.c_str() + preRSU.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre RSU faild: " << err;
    return;
  }
  Json::Value jsonv = tree["RSM"];
  if (!jsonv.isObject()) {
    return;
  }
  if (!google::protobuf::util::JsonStringToMessage(jsonv.toStyledString(), pMsg).ok()) {
    LOG(WARNING) << "CANNOT read RSM from preRSU";
    return;
  }
}
