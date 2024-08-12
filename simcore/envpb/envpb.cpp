/**
 * @file envpb.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "envpb.h"
#include <stdio.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <map>
#include "environment.pb.h"
#include "scene.pb.h"

template <typename type>
type interpolation(type a, type b, double interp) {
  return (type)(a + interp * (b - a));
}

envpb::envpb() {}

envpb::~envpb() {}

void envpb::Init(tx_sim::InitHelper &helper) {
  auto str = helper.GetParameter("interp_method");
  if (!str.empty()) {
    interpMethod = str;
  }
  helper.Publish("ENVIRONMENTAL");
  std::cout << "interp_method = " << interpMethod << std::endl;
}

void envpb::Reset(tx_sim::ResetHelper &helper) {
  envdata.clear();
  cur_idx = 0;
  auto scenebuf = helper.scene_pb();
  sim_msg::Scene scene;
  if (!scene.ParseFromString(helper.scene_pb())) {
    std::cout << "cannnot parse scene\n";
    return;
  }
  for (const auto &env : scene.environment()) {
    envdata.push_back(std::make_pair(static_cast<double>(env.first), std::move(env.second)));  //
  }
  std::sort(envdata.begin(), envdata.end(),
            [](const std::pair<double, sim_msg::EnvironmentalConditions> &rh1,
               const std::pair<double, sim_msg::EnvironmentalConditions> &rh2) { return rh1.first < rh2.first; });
  std::cout << "read env size is " << envdata.size() << std::endl;
}

void envpb::Step(tx_sim::StepHelper &helper) {
  std::cout << helper.timestamp() << " ";
  if (envdata.empty()) {
    std::cout << "the env data is empty\n";
    return;
  }

  while (cur_idx + 1 < envdata.size() && envdata[cur_idx + 1].first < helper.timestamp()) {
    cur_idx += 1;
  }
  if (cur_idx + 1 >= envdata.size()) {
    cur_idx = envdata.size() - 1;
  }
  auto env = envdata[cur_idx].second;
  double time_off = 0;
  if (cur_idx == envdata.size() - 1) {
    time_off = helper.timestamp() - envdata.back().first;
  } else {
    const auto &env1 = envdata[cur_idx].second;
    const auto &env2 = envdata[cur_idx + 1].second;
    double interp =
        (helper.timestamp() - envdata[cur_idx].first) / (envdata[cur_idx + 1].first - envdata[cur_idx].first);
    time_off = interp * (env2.unix_timestamp() - env1.unix_timestamp());
    if (time_off < 0) {
      std::cout << helper.timestamp() << ": the time is warning! off=" << time_off << std::endl;
    }
    if (interpMethod == "linear") {
    } else if (interpMethod == "nearest") {
      interp = interp < 0.5 ? 0 : 1;
    } else if (interpMethod.length() > 6 && interpMethod.substr(0, 6) == "corner") {
      double cn = std::atof(interpMethod.substr(6).c_str());
      cn = std::max(0.001, std::min(1.0, cn));
      if (interp < (1.0 - cn)) {
        interp = 0;
      } else {
        interp = (interp - 1.0 + cn) / cn;
      }
    } else {
      interp = -1;
    }
    if (interp >= -1e-4) {
      env.set_ambient_illumination(sim_msg::EnvironmentalConditions_AmbientIllumination(
          interpolation<int>(env1.ambient_illumination(), env2.ambient_illumination(), interp)));
      env.set_atmospheric_pressure(interpolation(env1.atmospheric_pressure(), env2.atmospheric_pressure(), interp));
      env.set_temperature(interpolation(env1.temperature(), env2.temperature(), interp));
      env.set_relative_humidity(interpolation(env1.relative_humidity(), env2.relative_humidity(), interp));
      env.mutable_precipitation()->set_type(sim_msg::EnvironmentalConditions_Precipitation_Type(
          interpolation<int>(env1.precipitation().type(), env2.precipitation().type(), interp)));
      env.mutable_precipitation()->set_intensity(
          interpolation(env1.precipitation().intensity(), env2.precipitation().intensity(), interp));
      env.mutable_fog()->set_visibility(interpolation(env1.fog().visibility(), env2.fog().visibility(), interp));
      env.mutable_clouds()->set_fractional_cloud_cover(sim_msg::EnvironmentalConditions_CloudLayer_FractionalCloudCover(
          interpolation<int>(env1.clouds().fractional_cloud_cover(), env2.clouds().fractional_cloud_cover(), interp)));
      env.mutable_wind()->set_speed(interpolation(env1.wind().speed(), env2.wind().speed(), interp));
      env.mutable_wind()->set_origin_direction(
          interpolation(env1.wind().origin_direction(), env2.wind().origin_direction(), interp));
      env.mutable_sun()->set_azimuth(interpolation(env1.sun().azimuth(), env2.sun().azimuth(), interp));
      env.mutable_sun()->set_elevation(interpolation(env1.sun().elevation(), env2.sun().elevation(), interp));
      env.mutable_sun()->set_intensity(interpolation(env1.sun().intensity(), env2.sun().intensity(), interp));
      env.set_temperature_c(interpolation(env1.temperature_c(), env2.temperature_c(), interp));
    }
  }

  env.set_unix_timestamp(env.unix_timestamp() + (std::int64_t)std::floor(time_off));
  env.mutable_time_of_day()->set_seconds_since_midnight((env.unix_timestamp() % 86400000) / 1000);

  std::string buf;
  env.SerializeToString(&buf);
  helper.PublishMessage("ENVIRONMENTAL", buf);

  std::cout << ": wind=" << env.wind().speed() << ", fog=" << env.fog().visibility()
            << ", cloud=" << static_cast<int>(env.clouds().fractional_cloud_cover())
            << ", unix=" << env.unix_timestamp() << ", precipitation=" << env.precipitation().type() << "\n";
}

void envpb::Stop(tx_sim::StopHelper &helper) {}

TXSIM_MODULE(envpb)
