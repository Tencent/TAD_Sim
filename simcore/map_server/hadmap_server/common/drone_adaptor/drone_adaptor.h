/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace drone_adaptor {
struct DetailType {
  int ID = 0;
  int Time = 0;
  int Airway = 0;
  double CenterX = 0.0;
  double CenterY = 0.0;
  double CenterZ = 0.0;
  double Head_a = 0.0;
  double Head_b = 0.0;
  double Body_L = 0.0;
  double Tail_a = 0.0;
  double Tail_b = 0.0;
  double Height = 0.0;
  double Heading = 0.0;
  double Pitch = 0.0;
  double Roll = 0.0;
  double Speed = 0.0;
};

struct CapsuleAndDroneBaseT {
  int DroneID = 0;
  int Priority = 0;
  std::string Name;
  std::string Owner;
  std::string Model;
};

struct DroneT : public CapsuleAndDroneBaseT {
  std::vector<DetailType> Tracs;
};

struct CapsuleT : public CapsuleAndDroneBaseT {
  std::vector<DetailType> Capsules;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DetailType, ID, Time, CenterX, CenterY, CenterZ, Head_a, Head_b, Body_L, Tail_a,
                                   Tail_b, Height, Heading, Pitch, Roll, Speed, Airway);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DroneT, DroneID, Priority, Name, Owner, Model, Tracs);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CapsuleT, DroneID, Priority, Name, Owner, Model, Capsules);

template <typename T>
nlohmann::json TypeDefaultJson() {
  nlohmann::json j;
  to_json(j, T());
  return std::move(j);
}

std::string TransDroneToSimrec(const std::string& drone, const std::string& capsule, int interval = 100 /* ms */);

constexpr int kDefaultFrameInterval = 100;  // ms
constexpr const char kDroneDetailKey[] = "Tracs";
constexpr const char kCapsuleDetailKey[] = "Capsules";

}  // namespace drone_adaptor
