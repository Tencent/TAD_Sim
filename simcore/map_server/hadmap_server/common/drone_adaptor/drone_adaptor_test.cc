/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "drone_adaptor.h"

#include "gtest/gtest.h"

#include "traffic.pb.h"

namespace drone_adaptor {

TEST(DroneAdaptorTest, DetailT_trans) {
  nlohmann::json j = TypeDefaultJson<DetailType>();
  j.update(nlohmann::json::parse(R"( {"ID":1,"Speed":600} )"));
  DetailType detail;
  from_json(j, detail);
  EXPECT_EQ(detail.ID, 1);
  EXPECT_DOUBLE_EQ(detail.Speed, 600.0);
}

TEST(DroneAdaptorTest, DroneT_trans) {
  nlohmann::json j = TypeDefaultJson<DroneT>();
  DroneT drone;
  from_json(j, drone);
  EXPECT_TRUE(drone.Tracs.empty());

  j = TypeDefaultJson<DroneT>();
  j.update(nlohmann::json::parse(R"( {"DroneID":101,"Tracs":[{"ID":1,"Speed":550}]} )"));
  for (auto& trac : j["Tracs"]) {
    nlohmann::json trac_j = TypeDefaultJson<DetailType>();
    trac_j.update(trac);
    trac = trac_j;
  }
  from_json(j, drone);
  EXPECT_EQ(drone.DroneID, 101);
  EXPECT_TRUE(drone.Tracs.size() > 0);
  EXPECT_EQ(drone.Tracs[0].ID, 1);
  EXPECT_DOUBLE_EQ(drone.Tracs[0].Speed, 550.0);
}

TEST(DroneAdaptorTest, TransDroneToSimrec) {
  EXPECT_EQ(TransDroneToSimrec("", ""), "");
  EXPECT_EQ(TransDroneToSimrec("[]", "[]"), "");

  std::string drone_s = R"(
[
    {
        "DroneID":1,
        "Name":"MT_002",
        "Owner":"Meituan",
        "Model":"gen-4",
        "Priority":1,
        "Tracs":[
            {
                "ID":1,
                "Time":600,
                "CenterX":-595.0563298642301,
                "CenterY":85.45063891384098,
                "CenterZ":120,
                "Heading":1.446441332248135,
                "Pitch":0,
                "Roll":0,
                "Speed":8,
                "AirWay":1
            },
            {
                "ID":2,
                "Time":620,
                "CenterX":-593.4686852614882,
                "CenterY":85.64909448918371,
                "CenterZ":120,
                "Heading":1.446441332248135,
                "Pitch":0,
                "Roll":0,
                "Speed":8,
                "AirWay":1
            }
        ]
    }
]
  )";

  std::string capsule_s = R"(
  )";
  auto res = TransDroneToSimrec(drone_s, capsule_s);

  sim_msg::TrafficRecords4Logsim simrec;
  simrec.ParseFromString(res);
  // std::cout << simrec.ShortDebugString() << std::endl;
}

}  // namespace drone_adaptor