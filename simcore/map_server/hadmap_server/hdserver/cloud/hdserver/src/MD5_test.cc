/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "MD5.h"
#include "gtest/gtest.h"

TEST(MD5Test, GenMD5) {
  // echo -n 123456 |md5sum
  EXPECT_EQ(GenMD5("123456"), "e10adc3949ba59abbe56e057f20f883e");
}

TEST(MD5Test, GenMD5Sum) { EXPECT_EQ(GenMD5Sum(".clang-format"), "1970ddb46e179627e403af9745e2439e"); }