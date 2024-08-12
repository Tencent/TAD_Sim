/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "S3ClientWrap.h"

#include "gtest/gtest.h"

namespace utils {

class S3ClientWrapTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { S3ClientWrap::Instance().Initialize(); }
};

TEST_F(S3ClientWrapTest, TravelDir) {
  for (auto& f : misc::TravelDir("./hdserver/cloud/hdserver/test/data")) {
    // std::cout << f << std::endl;
  }

  EXPECT_TRUE(true);
}

TEST_F(S3ClientWrapTest, DownloadDir) {
  EXPECT_TRUE(S3ClientWrap::Instance().DownloadDir("hadmap-only-1311246188", "map/", "/tmp"));
  EXPECT_TRUE(S3ClientWrap::Instance().DownloadDir("hadmap-only-1311246188", "scenes/editor/1684859668",
                                                   "/home/hdserver/scene"));
}

TEST_F(S3ClientWrapTest, UploadDir) {
  EXPECT_TRUE(S3ClientWrap::Instance().UploadDir("hadmap-only-1311246188", "tmp/", "/home/hdserver/scene"));
}

TEST_F(S3ClientWrapTest, DeleteObj) {
  EXPECT_TRUE(S3ClientWrap::Instance().Delete("hadmap-only-1311246188", "model/Catalogs/Dynamics/dynamic_9527.json"));
}

}  // namespace utils