// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "par_converter.h"
#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
/***************************************/
void ParConverter::parsedParToProto(const ParsedData& parData, tx_car::CarSimParData& carsimParData) {
  carsimParData.Clear();
  carsimParData.set_version(parData.version.c_str());

  auto par = carsimParData.mutable_pardata();

  for (const auto& data : parData.parDatas) {
    auto parsed = par->Add();

    parsed->mutable_keyword()->set_mid(data.keyword.id.c_str());
    parsed->mutable_keyword()->mutable_parproperty()->set_axle(tx_car::CarSimParData_Axle(data.keyword.mProperty.axle));
    parsed->mutable_keyword()->mutable_parproperty()->set_side(tx_car::CarSimParData_Side(data.keyword.mProperty.side));

    parsed->set_datatype(tx_car::CarSimParData_DataType(data.dataType));

    parsed->mutable_kvpair()->set_key(data.keyValue.first.c_str());
    parsed->mutable_kvpair()->set_value(data.keyValue.second.c_str());

    parsed->mutable_map1d()->CopyFrom(data.map1D);
    parsed->mutable_map2d()->CopyFrom(data.map2D);
  }
}
void ParConverter::parsedJsonToFile(tx_car::CarSimParData& carsimParData, const std::string& parsedJsonPath) {
  std::string jsonContent;
  tx_car::protoToJson(carsimParData, jsonContent, false);
  tx_car::car_config::dumpToFile(jsonContent, parsedJsonPath);
}

void ParConverter::parsedJsonToFile(tx_car::car& txcar, const std::string& parsedJsonPath) {
  std::string jsonContent;
  tx_car::protoToJson(txcar, jsonContent, true);
  tx_car::car_config::dumpToFile(jsonContent, parsedJsonPath);
}

bool ParConverter::convertToTxCarJson(const tx_car::CarSimParData& carsimParData, tx_car::car& txcar) {
  mCar.Clear();
  mCar.CopyFrom(txcar);

  /* order is important, do not change */
  //*
  if (!convertCarType(carsimParData, *mCar.mutable_car_type(), *mCar.mutable_ecu())) {
    LOG_ERROR << "fail to convert cartype data from carsim.";
    return false;
  }

  if (!convertWheelTire(carsimParData, *mCar.mutable_wheel_tire())) {
    LOG_ERROR << "fail to convert wheel tire data from carsim.";
    return false;
  }

  if (!convertSprungMass(carsimParData, *mCar.mutable_sprung_mass())) {
    LOG_ERROR << "fail to convert sprung mass.";
    return false;
  }

  if (!convertBody(carsimParData, *mCar.mutable_body())) {
    LOG_ERROR << "fail to convert body.";
    return false;
  }

  if (!convertEngine(carsimParData, *mCar.mutable_propulsion()->mutable_engine_parameter())) {
    LOG_ERROR << "fail to convert engine.";
    return false;
  }
  if (!convertDriveLine(carsimParData, *mCar.mutable_driveline())) {
    LOG_ERROR << "fail to convert driveline.";
    return false;
  }
  if (!convertSteerMapped(carsimParData, *mCar.mutable_steer_system()->mutable_steermapped())) {
    LOG_ERROR << "fail to convert mapped-steer.";
    return false;
  }
  if (!convertDynamicSteer(carsimParData, *mCar.mutable_steer_system()->mutable_dynamicsteer())) {
    LOG_ERROR << "fail to convert dynamic steer.";
    return false;
  }
  if (!convertSuspension(carsimParData, *mCar.mutable_susp())) {
    LOG_ERROR << "fail to convert suspension.";
    return false;
  }
  //*/
  txcar.Clear();
  txcar.CopyFrom(mCar);

  return true;
}

bool ParConverter::findParDataByKeyword(const tx_car::CarSimParData& parDataSet, const std::string& keyword,
                                        const tx_car::CarSimParData_DataType& dataType,
                                        tx_car::CarSimParData_ParData& parData, const tx_car::CarSimParData_Axle& axle,
                                        const tx_car::CarSimParData_Side& side) {
  bool bingo = false;
  parData.Clear();

  for (auto i = 0; i < parDataSet.pardata_size(); ++i) {
    const auto& parData_I = parDataSet.pardata().at(i);

    // check keyword
    if (parData_I.keyword().mid() == keyword) {
      // check data type
      if (dataType != parData_I.datatype()) {
        bingo = false;
        LOG_ERROR << "input dataType:" << tx_car::CarSimParData_DataType_Name(dataType)
                  << ", while par data set dataType:" << tx_car::CarSimParData_DataType_Name(parData_I.datatype())
                  << "\n";
        return bingo;
      }

      // check axle and side property
      if (axle == parData_I.keyword().parproperty().axle() && side == parData_I.keyword().parproperty().side()) {
        // bingo
        bingo = true;
        parData.CopyFrom(parData_I);
        return bingo;
      }
    }
  }

  if (!bingo) {
    LOG_ERROR << "unable to find par data by keyword " << keyword << "\n";
  }

  return bingo;
}

bool ParConverter::findAllParDataByKeyword(const tx_car::CarSimParData& parDataSet, const std::string& keyword,
                                           const tx_car::CarSimParData_DataType& dataType,
                                           std::vector<tx_car::CarSimParData_ParData>& parDatas,
                                           const tx_car::CarSimParData_Axle& axle,
                                           const tx_car::CarSimParData_Side& side) {
  parDatas.clear();

  for (auto i = 0; i < parDataSet.pardata_size(); ++i) {
    const auto& parData_I = parDataSet.pardata().at(i);

    // check keyword
    if (parData_I.keyword().mid() == keyword) {
      // check data type
      if (dataType != parData_I.datatype()) {
        LOG_ERROR << "input dataType:" << tx_car::CarSimParData_DataType_Name(dataType)
                  << ", while par data set dataType:" << tx_car::CarSimParData_DataType_Name(parData_I.datatype())
                  << "\n";
        continue;
      }

      // check axle and side property
      if (axle == parData_I.keyword().parproperty().axle() && side == parData_I.keyword().parproperty().side()) {
        // bingo
        parDatas.push_back(parData_I);
      }
    }
  }

  return parDatas.size() > 0;
}

void ParConverter::copyMap1DData(tx_car::Real1DMap& out, const tx_car::Real1DMap& in, tx_car::DataSource dataSource,
                                 double x0Scale, double y0Scale) {
  auto u0 = out.mutable_u0_axis();
  auto y0 = out.mutable_y0_axis();

  u0->mutable_data()->Clear();
  y0->mutable_data()->Clear();

  u0->mutable_data()->CopyFrom(in.u0_axis().data());
  y0->mutable_data()->CopyFrom(in.y0_axis().data());

  u0->set_data_source(dataSource);
  y0->set_data_source(dataSource);

  // unit conversion
  for (auto i = 0; i < u0->data_size(); ++i) {
    u0->set_data(i, u0->data().at(i) * x0Scale);
  }
  for (auto i = 0; i < y0->data_size(); ++i) {
    y0->set_data(i, y0->data().at(i) * y0Scale);
  }
}

void ParConverter::copyMap2DData(tx_car::Real2DMap& out, const tx_car::Real2DMap& in, tx_car::DataSource dataSource) {
  auto u0 = out.mutable_u0_axis();
  auto u1 = out.mutable_u1_axis();
  auto y0 = out.mutable_y0_axis();

  u0->mutable_data()->Clear();
  u1->mutable_data()->Clear();
  y0->mutable_data()->Clear();

  u0->mutable_data()->CopyFrom(in.u0_axis().data());
  u1->mutable_data()->CopyFrom(in.u1_axis().data());
  y0->mutable_data()->CopyFrom(in.y0_axis().data());

  u0->set_data_source(dataSource);
  u1->set_data_source(dataSource);
  y0->set_data_source(dataSource);
}
}  // namespace tx_car
