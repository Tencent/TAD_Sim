/**
 * @file asn1msg_bsm2020.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "asn1msg_bsm2020.h"
#include <cmath>
#include <glog/logging.h>
#include <google/protobuf/util/json_util.h>
#include <json/json.h>
#include <fstream>
#include "merge_proto.h"
#include "v2x_asn1_2020.pb.h"

/**
 * @brief Convert BSM2020 to protobuf message
 *
 * @param msg input pb msg
 * @return true on success, false otherwise
 * @return false
 */
bool CAsn1MsgBSM2020::Convert(google::protobuf::Message *msg) {
  v2x_asn1_2020::V2X_ASN1 *v2x_asn1_2020 = (v2x_asn1_2020::V2X_ASN1 *)msg;
  v2x_asn1_2020::V2XBSMMsg *pbsmmsg = v2x_asn1_2020->mutable_bsm()->Add();
  Convert_Traffic(pbsmmsg);
  return true;
}

/**
 * @brief write previous json example to file
 *
 * @param fpath file path to save the data
 */
void CAsn1MsgBSM2020::WritePreJsonExample(const std::string &fpath) {
  v2x_asn1_2020::V2XBSM pV2XBSM;
  // use radom number to fill up the data
  FillupProto(&pV2XBSM);
  std::string buffer;
  // message to json
  google::protobuf::util::MessageToJsonString(pV2XBSM, &buffer);
  // write json file
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(buffer.c_str(), buffer.c_str() + buffer.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre BSM faild: " << err;
    return;
  }
  Json::Value bsmjson;
  bsmjson["BSM"] = tree;
  Json::StreamWriterBuilder builder;
  const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
  std::ofstream emp(fpath);
  writer->write(bsmjson, &emp);
  emp.close();
}

/**
 * @brief convert traffic to protobuf
 *
 * @param pbsmmsg input pb msg
 */
void CAsn1MsgBSM2020::Convert_Traffic(v2x_asn1_2020::V2XBSMMsg *pbsmmsg) {
  pbsmmsg->set_minuteoftheyear(std::round(data.timestamp / 60000));
  for (const auto &carbuf : data.cars) {
    // convert car to protobuf
    auto &car = carbuf.car;
    v2x_asn1_2020::V2XBSM *pV2XBSM = pbsmmsg->mutable_v2xbsms()->Add();
    pV2XBSM->set_msgcount(carbuf.frameid);
    pV2XBSM->set_vehicleid(car.id());
    pV2XBSM->set_dsecond((std::size_t)carbuf.timestampe % 60000);
    // lonlat is integer, so need to multiply by 1E7
    pV2XBSM->mutable_position3d()->set_longitude(car.x() * 1E7);
    pV2XBSM->mutable_position3d()->set_latitude(car.y() * 1E7);
    pV2XBSM->mutable_position3d()->set_elevation(car.z() * 10);
    pV2XBSM->set_speed(std::sqrt(car.v() * car.v() + car.vl() * car.vl()) * 50);
    // 0 is north
    pV2XBSM->set_heading(
        ((90 + 360 - static_cast<int>(std::round(car.heading() * 180. / 3.141592653589793238462643383))) % 360) * 80);
    pV2XBSM->set_motionconfidencesetflag(false);
    pV2XBSM->set_positionaccuracyflag(false);
    pV2XBSM->mutable_accelerationset4way()->set_longacceleration(car.acc() * 100);
    pV2XBSM->mutable_accelerationset4way()->set_latacceleration(car.acc_l() * 100);
    // m -> cm
    pV2XBSM->mutable_vehiclesize()->set_vehiclewidth(car.width() * 100);    // m -> cm
    pV2XBSM->mutable_vehiclesize()->set_vehicleheight(car.height() * 100);  // m -> cm
    pV2XBSM->mutable_vehiclesize()->set_vehiclelength(car.length() * 100);  // m -> cm
    pV2XBSM->set_vehiclesafetyextensionsflag(false);
    // merge prevous car data with new one
    CoverCar(carbuf.preBSM, *pV2XBSM);
  }
}

/**
 * @brief merge privious car data to bsm
 *
 * @param preBSM previous bsm data
 * @param pV2XBSM merged car data
 */
void CAsn1MsgBSM2020::CoverCar(const std::string &preBSM, v2x_asn1_2020::V2XBSM &pV2XBSM) {
  if (preBSM.empty()) return;
  // parse bsm from prebsm string
  Json::Value tree;
  Json::CharReaderBuilder reader;
  std::string err;
  std::unique_ptr<Json::CharReader> const json_read(reader.newCharReader());
  if (!json_read->parse(preBSM.c_str(), preBSM.c_str() + preBSM.length(), &tree, &err)) {
    LOG(WARNING) << "Read pre BSM faild: " << err;
    return;
  }
  Json::Value bsmjson = tree["BSM"];
  if (!bsmjson.isObject()) {
    return;
  }
  v2x_asn1_2020::V2XBSM preV2XBSM;
  // json to message
  if (!google::protobuf::util::JsonStringToMessage(bsmjson.toStyledString(), &preV2XBSM).ok()) {
    LOG(WARNING) << "CANNOT read json from preBSM";
    return;
  }
  MergeProto(pV2XBSM, &preV2XBSM);
  pV2XBSM = preV2XBSM;
}
