// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/proto_helper.h"
#include "inc/car_log.h"

namespace tx_car {
bool protoToJson(const google::protobuf::Message &message, std::string &json_str) {
  google::protobuf::util::JsonPrintOptions options;
  options.add_whitespace = true;
  options.always_print_primitive_fields = true;
  options.preserve_proto_field_names = true;

  json_str.clear();
  return google::protobuf::util::MessageToJsonString(message, &json_str, options).ok();
}

bool jsonToProto(const std::string &json_str, google::protobuf::Message &message) {
  return google::protobuf::util::JsonStringToMessage(json_str, &message).ok();
}

// set value of real scalar data
void setRealScalar(tx_car::RealScalar *real_scalar, const std::string &disp_name, const std::string &comment,
                   const std::string &unit, double val) {
  real_scalar->mutable_disp_name()->assign(disp_name.c_str());
  real_scalar->mutable_comment()->assign(comment.c_str());
  real_scalar->mutable_unit()->assign(unit.c_str());
  real_scalar->set_val(val);
}

// set value of real array data
void setRealArray(tx_car::RealArray *real_array, const std::string &disp_name, const std::string &comment,
                  const std::string &unit, double arr_in[], double size) {
  real_array->mutable_disp_name()->assign(disp_name.c_str());
  real_array->mutable_comment()->assign(comment.c_str());
  real_array->mutable_unit()->assign(unit.c_str());
  real_array->mutable_data()->Clear();
  for (auto i = 0; i < size; ++i) {
    real_array->mutable_data()->Add(arr_in[i]);
  }
}

// set value of real axis data
void setRealAxis(tx_car::RealAxis *real_axis, const std::string &disp_name, const std::string &comment,
                 const std::string &unit, double arr_in[], double size) {
  real_axis->mutable_disp_name()->assign(disp_name.c_str());
  real_axis->mutable_comment()->assign(comment.c_str());
  real_axis->mutable_unit()->assign(unit.c_str());
  real_axis->mutable_data()->Clear();
  for (auto i = 0; i < size; ++i) {
    real_axis->mutable_data()->Add(arr_in[i]);
  }
}

// set value of 1-d map info
void set1DMapInfo(tx_car::Real1DMap *map, const std::string &disp_name, const std::string &comment) {
  map->mutable_disp_name()->assign(disp_name.c_str());
  map->mutable_comment()->assign(comment.c_str());
}

// set value of int32 scalar data
void setInt32Scalar(tx_car::Int32Scalar *int_scalar, const std::string &disp_name, const std::string &comment,
                    const std::string &unit, int val) {
  int_scalar->mutable_disp_name()->assign(disp_name.c_str());
  int_scalar->mutable_comment()->assign(comment.c_str());
  int_scalar->mutable_unit()->assign(unit.c_str());
  int_scalar->set_val(val);
}

// set value of string scalar data
void setStringScalar(tx_car::StringVariable *string_variable, const std::string &disp_name, const std::string &comment,
                     const std::string &val) {
  string_variable->mutable_disp_name()->assign(disp_name.c_str());
  string_variable->mutable_comment()->assign(comment.c_str());
  string_variable->mutable_str_var()->assign(val.c_str());
}

// set veh param
void initVehParam(struct_hFtzkwIyDan9Uw0DEd8s7D &veh, const tx_car::SprungMassDD &sprung) {
  {
    /*
    veh.StaticNormalFrontLoad.FL = sprung.staticnormalfrontload().fl().val();
    veh.StaticNormalFrontLoad.FR = sprung.staticnormalfrontload().fr().val();
    veh.StaticNormalRearLoad.RR = sprung.staticnormalrearload().rr().val();
    veh.StaticNormalRearLoad.RL = sprung.staticnormalrearload().rl().val();
    */
    real_T sprung_mass = sprung.sprungmass().val();
    real_T mass = sprung.mass().val();
    real_T wheel_base = sprung.wheelbase().val();
    real_T a = sprung.frontaxlepositionfromcg().val();
    real_T b = sprung.rearaxlepositionfromcg().val();
    const int ratio = 1;
    veh.StaticNormalFrontLoad.FL = b / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalFrontLoad.FR = b / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalRearLoad.RL = a / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalRearLoad.RR = a / wheel_base * sprung_mass / 2 * ratio;
  }
  veh.Mass = sprung.mass().val();
  veh.SprungMass = sprung.sprungmass().val();
  veh.WheelBase = sprung.wheelbase().val();
  veh.FrontAxlePositionfromCG = sprung.frontaxlepositionfromcg().val();
  veh.RearAxlePositionfromCG = sprung.rearaxlepositionfromcg().val();
  veh.HeightCG = sprung.heightcg().val();
  veh.FrontalArea = sprung.frontalarea().val();
  veh.DragCoefficient = sprung.dragcoefficient().val();
  veh.NumberOfWheelsPerAxle = sprung.numberofwheelsperaxle().val();
  veh.PitchMomentInertia = sprung.pitchmomentinertia().val();
  veh.RollMomentInertia = sprung.rollmomentinertia().val();
  veh.YawMomentInertia = sprung.yawmomentinertia().val();
  veh.SteeringRatio = sprung.steeringratio().val();
  veh.TrackWidth = sprung.trackwidthfront().val();
  veh.InitialLongPosition = sprung.initiallongposition().val();
  veh.InitialLatPosition = sprung.initiallatposition().val();
  veh.InitialVertPosition = sprung.initialvertposition().val();
  veh.InitialRollAngle = sprung.initialrollangle().val();
  veh.InitialPitchAngle = sprung.initialpitchangle().val();
  veh.InitialYawAngle = sprung.initialyawangle().val();
  veh.InitialLongVel = sprung.initiallongvel().val();
  veh.InitialLatVel = sprung.initiallatvel().val();
  veh.InitialVertVel = sprung.initialvertvel().val();
  veh.InitialRollRate = sprung.initialrollrate().val();
  veh.InitialPitchRate = sprung.initialpitchrate().val();
  veh.InitialYawRate = sprung.initialyawrate().val();
  veh.UnsprungMassFrontAxle = sprung.unsprungmassfrontaxle().val();
  veh.UnsprungMassRearAxle = sprung.unsprungmassrearaxle().val();
}

// set veh param
void initVehParam(struct_WPo6c0btEjgdkiBqVilJ2B &veh, const tx_car::SprungMassDD &sprung) {
  {
    /*
    veh.StaticNormalFrontLoad.FL = sprung.staticnormalfrontload().fl().val();
    veh.StaticNormalFrontLoad.FR = sprung.staticnormalfrontload().fr().val();
    veh.StaticNormalRearLoad.RR = sprung.staticnormalrearload().rr().val();
    veh.StaticNormalRearLoad.RL = sprung.staticnormalrearload().rl().val();
    */
    real_T sprung_mass = sprung.sprungmass().val();
    real_T mass = sprung.mass().val();
    real_T wheel_base = sprung.wheelbase().val();
    real_T a = sprung.frontaxlepositionfromcg().val();
    real_T b = sprung.rearaxlepositionfromcg().val();
    const int ratio = 1;
    veh.StaticNormalFrontLoad.FL = b / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalFrontLoad.FR = b / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalRearLoad.RL = a / wheel_base * sprung_mass / 2 * ratio;
    veh.StaticNormalRearLoad.RR = a / wheel_base * sprung_mass / 2 * ratio;
  }
  veh.Mass = sprung.mass().val();
  veh.SprungMass = sprung.sprungmass().val();
  veh.WheelBase = sprung.wheelbase().val();
  veh.FrontAxlePositionfromCG = sprung.frontaxlepositionfromcg().val();
  veh.RearAxlePositionfromCG = sprung.rearaxlepositionfromcg().val();
  veh.HeightCG = sprung.heightcg().val();
  veh.FrontalArea = sprung.frontalarea().val();
  veh.DragCoefficient = sprung.dragcoefficient().val();
  veh.NumberOfWheelsPerAxle = sprung.numberofwheelsperaxle().val();
  veh.PitchMomentInertia = sprung.pitchmomentinertia().val();
  veh.RollMomentInertia = sprung.rollmomentinertia().val();
  veh.YawMomentInertia = sprung.yawmomentinertia().val();
  veh.SteeringRatio = sprung.steeringratio().val();
  veh.TrackWidthFront = sprung.trackwidthfront().val();
  veh.TrackWidthRear = sprung.trackwidthrear().val();
  veh.InitialLongPosition = sprung.initiallongposition().val();
  veh.InitialLatPosition = sprung.initiallatposition().val();
  veh.InitialVertPosition = sprung.initialvertposition().val();
  veh.InitialRollAngle = sprung.initialrollangle().val();
  veh.InitialPitchAngle = sprung.initialpitchangle().val();
  veh.InitialYawAngle = sprung.initialyawangle().val();
  veh.InitialLongVel = sprung.initiallongvel().val();
  veh.InitialLatVel = sprung.initiallatvel().val();
  veh.InitialVertVel = sprung.initialvertvel().val();
  veh.InitialRollRate = sprung.initialrollrate().val();
  veh.InitialPitchRate = sprung.initialpitchrate().val();
  veh.InitialYawRate = sprung.initialyawrate().val();
  veh.UnsprungMassFrontAxle = sprung.unsprungmassfrontaxle().val();
  veh.UnsprungMassRearAxle = sprung.unsprungmassrearaxle().val();
}

void initArray3(real_T out[], const tx_car::RealArray &in) {
  const size_t size = 3;
  if (in.data_size() != size) {
    LOG_ERROR << in.disp_name() << " array size must be size of " << size << "\n";
    std::abort();
  }
  for (auto i = 0; i < size; ++i) {
    out[i] = in.data().at(i);
  }
}
void initArray9(real_T out[], const tx_car::RealArray &in) {
  const size_t size = 9;
  if (in.data_size() != size) {
    LOG_ERROR << in.disp_name() << " array size must be size of " << size << "\n";
    std::abort();
  }
  for (auto i = 0; i < size; ++i) {
    out[i] = in.data().at(i);
  }
}
void initArray(real_T out[], const tx_car::RealArray &in, size_t size) {
  for (auto i = 0; i < in.data_size() && i < size; ++i) {
    out[i] = in.data().at(i);
  }
}
void initAxis(real_T out[], const tx_car::RealAxis &in, size_t size) {
  for (auto i = 0; i < in.data_size() && i < size; ++i) {
    out[i] = in.data().at(i);
  }
}

// 1-d map checker
bool map1DFormatChecker(const tx_car::Real1DMap &map_1d) {
  auto u0_size = map_1d.u0_axis().data_size();
  auto y0_size = map_1d.y0_axis().data_size();

  auto ret_state =
      (u0_size >= 2) && (u0_size == y0_size) && (u0_size <= kMap1dSize) && (y0_size <= kMap1dSize);

  if (!ret_state) {
    LOG_ERROR << "map check failed, name:" << map_1d.disp_name() << ", u0 axis size:" << map_1d.u0_axis().data_size()
              << ", y0 axis size:" << map_1d.y0_axis().data_size() << ".\n";
  }

  return ret_state;
}

// 2-d map checker
bool map2DFormatChecker(const tx_car::Real2DMap &map_2d) {
  auto u0_size = map_2d.u0_axis().data_size();
  auto u1_size = map_2d.u1_axis().data_size();
  auto y0_size = map_2d.y0_axis().data_size();

  auto ret_state = (u0_size >= 2) && (u1_size >= 2) && (u0_size * u1_size == y0_size);

  if (!ret_state) {
    LOG_ERROR << "map check failed, name:" << map_2d.disp_name() << ", u0 axis size:" << map_2d.u0_axis().data_size()
              << ", u1 axis size:" << map_2d.u1_axis().data_size() << ", y0 axis size:" << map_2d.y0_axis().data_size()
              << ".\n";
  }

  return ret_state;
}

void initMap1D(const tx_car::Real1DMap &map_1d, real_T bp[], real_T table[], uint32_t &max_index) {
  // map checker
  if (!tx_car::map1DFormatChecker(map_1d)) {
    std::abort();
  }

  if (map_1d.y0_axis().data_size() >= tx_car::kMap1dSize) {
    LOG_ERROR << map_1d.disp_name() << " max map axis size is " << kMap1dSize << ".\n";
    std::abort();
  }

  max_index = map_1d.y0_axis().data_size() - 1;
  if (max_index < 1) {
    LOG_ERROR << map_1d.disp_name() << " map data size error.\n";
    std::abort();
  }

  // fl
  tx_car::initAxis(bp, map_1d.u0_axis(), map_1d.u0_axis().data_size());
  tx_car::initAxis(table, map_1d.y0_axis(), map_1d.y0_axis().data_size());
}

// print map 2d
MODULE_API void printMap2d(const tx_car::Real2DMap &map) {
  double data[kMap1dSize][kMap1dSize];

  data[0][0] = 0.0;

  size_t row = 0, col = 0;

  row = map.u0_axis().data_size();
  col = map.u1_axis().data_size();

  for (auto r = 0ul; r < row; ++r) {
    data[r + 1][0] = map.u0_axis().data().at(r);
  }
  for (auto c = 0ul; c < col; ++c) {
    data[0][c + 1] = map.u1_axis().data().at(c);
  }

  for (auto c = 0ul; c < col; ++c) {
    for (auto r = 0ul; r < row; ++r) {
      data[r + 1][c + 1] = getValueFromMap2d(map, r, c);
    }
  }
  /*
  for (auto c = 0ul; c < col; ++c) {
      for (auto r = 0ul; r < row; ++r) {
          data[r + 1][c + 1] = map.y0_axis().data().at(c * row + r);
      }
  }
  */

  for (auto r = 0ul; r <= row; ++r) {
    std::string outStr;
    for (auto c = 0ul; c <= col; ++c) {
      outStr += std::to_string(data[r][c]) + ",";
    }
    LOG_2 << outStr << "\n";
  }
}

// transpose map 2d
MODULE_API tx_car::Real2DMap transposeMap2d(const tx_car::Real2DMap &map) {
  tx_car::Real2DMap map2d;

  if (!tx_car::map2DFormatChecker(map)) {
    std::string error = "map2d format error, " + map.disp_name();
    LOG_ERROR << error << "\n";
    throw(std::runtime_error(error.c_str()));
    return map2d;
  }

  map2d.mutable_u0_axis()->CopyFrom(map.u1_axis());
  map2d.mutable_u1_axis()->CopyFrom(map.u0_axis());

  size_t axle1Size = map.u0_axis().data_size();
  size_t axle2Size = map.u1_axis().data_size();

  // transpose
  for (auto a1 = 0ul; a1 < axle1Size; ++a1) {
    for (auto a2 = 0ul; a2 < axle2Size; ++a2) {
      map2d.mutable_y0_axis()->add_data(getValueFromMap2d(map, a1, a2));
    }
  }

  map2d.mutable_y0_axis()->set_comment(map.y0_axis().comment());
  map2d.mutable_y0_axis()->set_data_source(map.y0_axis().data_source());
  map2d.mutable_y0_axis()->set_disp_name(map.y0_axis().disp_name());

  return map2d;
}

// get y by row and col index
MODULE_API double getValueFromMap2d(const tx_car::Real2DMap &map, size_t r, size_t c) {
  if (!tx_car::map2DFormatChecker(map)) {
    std::string error = "map2d format error, " + map.disp_name();
    LOG_ERROR << error << "\n";
    throw(std::runtime_error(error.c_str()));
    return 0.0;
  }

  size_t axle1Size = map.u0_axis().data_size();

  return map.y0_axis().data().at(c * axle1Size + r);
}

// set y by row and col index
MODULE_API void setValueOfMap2d(tx_car::Real2DMap &map, size_t r, size_t c, double value) {
  if (!tx_car::map2DFormatChecker(map)) {
    std::string error = "map2d format error, " + map.disp_name();
    LOG_ERROR << error << "\n";
    throw(std::runtime_error(error.c_str()));
    return;
  }

  size_t axle1Size = map.u0_axis().data_size();

  map.mutable_y0_axis()->mutable_data()->Set(c * axle1Size + r, value);
}
// get one column of map 2d
MODULE_API tx_car::RealAxis getColumnOfMap2d(const tx_car::Real2DMap &map, size_t c) {
  tx_car::RealAxis axis;

  if (!tx_car::map2DFormatChecker(map)) {
    std::string error = "map2d format error, " + map.disp_name();
    LOG_ERROR << error << "\n";
    throw(std::runtime_error(error.c_str()));
    return axis;
  }

  size_t axle1Size = map.u0_axis().data_size();

  for (auto i = 0ul; i < axle1Size; ++i) {
    axis.add_data(map.y0_axis().data().at(c * axle1Size + i));
  }

  return axis;
}

// get max in real axis
MODULE_API double getMaxValueOf(const tx_car::RealAxis &axis) {
  size_t axle1Size = axis.data_size();
  double maxValue = -1e12;

  for (auto i = 0ul; i < axle1Size; ++i) {
    maxValue = axis.data().at(i) > maxValue ? axis.data().at(i) : maxValue;
  }

  return maxValue;
}

MODULE_API double getMinValueOf(const tx_car::RealAxis &axis) {
  size_t axle1Size = axis.data_size();
  double minValue = 1e12;

  for (auto i = 0ul; i < axle1Size; ++i) {
    minValue = axis.data().at(i) < minValue ? axis.data().at(i) : minValue;
  }

  return minValue;
}

// flip each element in axis and exchange element (i) with (n-i)
MODULE_API void flipAndExchangeAxisData(tx_car::RealAxis &yAxis) {
  const int l_flip = -1;

  // flip axis data
  for (auto i = 0ul; i < yAxis.data_size(); ++i) {
    yAxis.set_data(i, yAxis.data(i) * l_flip);
  }

  // inverse axis data
  int l = 0, r = yAxis.data_size() - 1;
  double tmp = 0.0;
  while (l < r) {
    tmp = yAxis.data(l);
    yAxis.set_data(l, yAxis.data(r));
    yAxis.set_data(r, tmp);
    l++;
    r--;
  }
}
}  // namespace tx_car
