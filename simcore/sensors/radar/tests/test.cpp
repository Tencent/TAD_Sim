/**
 * @file test.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-25
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <fstream>
#include "txsim_messenger.h"
#include "txsim_module_service.h"
#include "../radar.h"
#include "../sensor.h"
#include "gtest/gtest.h"


/**
 * @brief Construct a new TEST object
 *
 */
TEST(SensorTest, Fun) {
  Sensor sensor(1);

  sensor.setRotation(0, 0, 0);
  sensor.setPosition(Eigen::Vector3d(0, 0, 0));
  sensor.setCarPosition(Eigen::Vector3d(0, 0, 0));
  sensor.setCarRoatation(0, 0, 0);
  sensor.MaxDistance() = 100;

  std::vector<Eigen::Vector3d> wps;
  wps.push_back(Eigen::Vector3d(30, 0, 0));

  EXPECT_EQ(sensor.ID(), 1);
  EXPECT_TRUE(sensor.inFov(wps));
}

/**
 * @brief Construct a new TEST object
 *
 */
TEST(RadarTest, init) {
  radar_bit::radar_parameter para;
  para.F0_GHz = 24;
  radar_bit::output out = radar_bit::Radar_Initialization(para, 0);
  para.F0_GHz = 77;
  out = radar_bit::Radar_Initialization(para, 1);
  para.F0_GHz = 79;
  out = radar_bit::Radar_Initialization(para, 2);

  for (int i = 0; i < 3; i++) {
    std::cout << " delta_R=" << out.delta_R[i] << " delta_V=" << out.delta_V[i] << " delta_A=" << out.delta_A[i]
              << " hfov=" << out.hfov[i] << " R_max=" << out.R_max[i] << " sigma_R=" << out.sigma_R[i]
              << " sigma_V=" << out.sigma_V[i] << " sigma_A=" << out.sigma_A[i] << std::endl;
  }

  EXPECT_TRUE(out.R_max[2] > 50);
}

TEST(RadarTest, signal) {
  radar_bit::radar_parameter para;
  para.F0_GHz = 79;
  radar_bit::output out = radar_bit::Radar_Initialization(para, 0);

  std::cout << " delta_R=" << out.delta_R[0] << " delta_V=" << out.delta_V[0] << " delta_A=" << out.delta_A[0]
            << " hfov=" << out.hfov[0] << " R_max=" << out.R_max[0] << " sigma_R=" << out.sigma_R[0]
            << " sigma_V=" << out.sigma_V[0] << " sigma_A=" << out.sigma_A[0] << std::endl;

  radar_bit::tar_info_in tar[3];
  tar[0].tar_range = 20;
  tar[0].tar_speed = -5;
  tar[0].tar_angle = -30;
  tar[0].tar_ID = 1;
  tar[0].tar_rcs = 4;
  tar[1].tar_range = 20.1;
  tar[1].tar_speed = -5;
  tar[1].tar_angle = -30;
  tar[1].tar_ID = 2;
  tar[1].tar_rcs = 4;
  tar[2].tar_range = 20;
  tar[2].tar_speed = 5;
  tar[2].tar_angle = 30;
  tar[2].tar_ID = 3;
  tar[2].tar_rcs = 4;

  auto tar_out = radar_bit::Rar_signal_pro(0, tar, 3, out, 0, 0);

  EXPECT_TRUE(out.R_max[0] > 50);
}

TEST(RadarTest, radar) {
  try {
    std::shared_ptr<tx_sim::SimModule> module = std::make_shared<SimRadar>();
    // tx_sim::SimModuleService service;
    // service.Serve("Ego_001-camera", module, "");
    // service.Wait();
    // exit(0);
  } catch (const std::exception &e) {
    std::cerr << "Serving module error:\n" << e.what() << std::endl;
  }

  EXPECT_TRUE(1);
}
