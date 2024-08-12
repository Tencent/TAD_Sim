/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/utils/xsd_validator.h"

#include <thread>

#include "gtest/gtest.h"

#include "common/utils/status.h"

namespace utils {
class XsdValidatorTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { XsdValidator::Instance().Initialize("./hdserver/cloud/resources/OpenX"); }
};

TEST_F(XsdValidatorTest, v1) {
  auto status = XsdValidator::Validate("./hdserver/cloud/hdserver/test/data/BUG_110033741.xosc",
                                       "./hdserver/cloud/resources/OpenX/OpenSCENARIO_1_0.xsd");
  ASSERT_FALSE(status.ok());
  std::cout << status.ToString() << std::endl;
}

TEST_F(XsdValidatorTest, v2) {
  auto status = XsdValidator::Instance().Validate("./hdserver/cloud/hdserver/test/data/SimCity_01.xosc",
                                                  XsdValidator::XsdType::kOpenScenario_1_0);
  ASSERT_TRUE(status.ok());
  std::cout << status.ToString() << std::endl;
}

TEST_F(XsdValidatorTest, multi_v) {
  std::vector<std::thread> workers;
  for (int i = 0; i < 4; i++) {
    workers.emplace_back(std::thread([&]() {
      for (int j = 0; j < 100; j++) {
        auto status = XsdValidator::Instance().Validate("./hdserver/cloud/hdserver/test/data/BUG_110033741.xosc",
                                                        XsdValidator::XsdType::kOpenScenario_1_0);
        ASSERT_FALSE(status.ok());
        std::cout << "thread " << i << ": " << status.ToString() << std::endl;
      }
    }));
  }

  for (int i = 0; i < 4; i++) {
    workers[i].join();
  }
}

TEST_F(XsdValidatorTest, odr) {
  auto status = XsdValidator::Instance().Validate("./hdserver/cloud/hdserver/test/data/哈哈哈.xodr",
                                                  XsdValidator::XsdType::kOpenDrive_1_4);
  EXPECT_TRUE(status.ok()) << status.ToString();
}

}  // namespace utils