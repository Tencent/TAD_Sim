// Copyright 2024 Tencent Inc. All rights reserved.
//

#ifndef VEHICLEDYNAMICS_INC_PROTO_HELPER_H_
#define VEHICLEDYNAMICS_INC_PROTO_HELPER_H_

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

#include "Chassis_types.h"
#include "TX_Transmission_types.h"
#include "car.pb.h"
#include "inc/car_common.h"

#include <string>

namespace tx_car {
// protobuf message to json string
MODULE_API bool protoToJson(const google::protobuf::Message &message, std::string &json_str);

// json string to protobuf message
MODULE_API bool jsonToProto(const std::string &json_str, google::protobuf::Message &message);

// protobuf message to json string
template <class MsgTy>
MODULE_API bool protoToJson(const MsgTy &message, std::string &json_str, bool always_print_primitive_fields = true) {
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = always_print_primitive_fields;

  json_str.clear();
  return google::protobuf::util::MessageToJsonString(message, &json_str, options).ok();
}

// json string to protobuf message
template <class MsgTy>
MODULE_API bool jsonToProto(const std::string &json_str, MsgTy &message) {
  return google::protobuf::util::JsonStringToMessage(json_str, &message).ok();
}

// set value of real scalar data
MODULE_API void setRealScalar(tx_car::RealScalar *real_scalar, const std::string &disp_name, const std::string &comment,
                              const std::string &unit, double val);

// set value of int32 scalar data
MODULE_API void setInt32Scalar(tx_car::Int32Scalar *int_scalar, const std::string &disp_name,
                               const std::string &comment, const std::string &unit, int val);

// set value of string scalar data
MODULE_API void setStringScalar(tx_car::StringVariable *string_variable, const std::string &disp_name,
                                const std::string &comment, const std::string &val);

// set value of real array data
MODULE_API void setRealArray(tx_car::RealArray *real_array, const std::string &disp_name, const std::string &comment,
                             const std::string &unit, double arr_in[], double size);

// set value of real axis data
MODULE_API void setRealAxis(tx_car::RealAxis *real_array, const std::string &disp_name, const std::string &comment,
                            const std::string &unit, double arr_in[], double size);

// set value of 1-d map info
MODULE_API void set1DMapInfo(tx_car::Real1DMap *map, const std::string &disp_name, const std::string &comment);

// init veh param
MODULE_API void initVehParam(struct_hFtzkwIyDan9Uw0DEd8s7D &veh, const tx_car::SprungMassDD &sprung);
MODULE_API void initVehParam(struct_WPo6c0btEjgdkiBqVilJ2B &veh, const tx_car::SprungMassDD &sprung);

// init array 3/9/any
MODULE_API void initArray3(real_T out[], const tx_car::RealArray &in);
MODULE_API void initArray9(real_T out[], const tx_car::RealArray &in);
MODULE_API void initArray(real_T out[], const tx_car::RealArray &in, size_t size = tx_car::kMap1dSize);
MODULE_API void initAxis(real_T out[], const tx_car::RealAxis &in, size_t size = tx_car::kMap1dSize);

// 1-d map checker
MODULE_API bool map1DFormatChecker(const tx_car::Real1DMap &map_1d);

// 2-d map checker
MODULE_API bool map2DFormatChecker(const tx_car::Real2DMap &map_2d);

// init map, used to set 1-d map used in simulink
MODULE_API void initMap1D(const tx_car::Real1DMap &map_1d, real_T bp[], real_T table[], uint32_t &max_index);

// print map 2d, map2d is column order
MODULE_API void printMap2d(const tx_car::Real2DMap &map);

// transpose map 2d, map2d is column order
MODULE_API tx_car::Real2DMap transposeMap2d(const tx_car::Real2DMap &map);

// get y by row and col index, map2d is column order
MODULE_API double getValueFromMap2d(const tx_car::Real2DMap &map, size_t r, size_t c);

// set y by row and col index, map2d is column order
MODULE_API void setValueOfMap2d(tx_car::Real2DMap &map, size_t r, size_t c, double value);

// get one column of map 2d
MODULE_API tx_car::RealAxis getColumnOfMap2d(const tx_car::Real2DMap &map, size_t c);

// get max in real axis
MODULE_API double getMaxValueOf(const tx_car::RealAxis &axis);

// get min in real axis
MODULE_API double getMinValueOf(const tx_car::RealAxis &axis);

// flip each element in axis and exchange element (i) with (n-i)
MODULE_API void flipAndExchangeAxisData(tx_car::RealAxis &yAxis);
}  // namespace tx_car

#endif  // VEHICLEDYNAMICS_INC_PROTO_HELPER_H_
