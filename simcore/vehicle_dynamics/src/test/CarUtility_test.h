// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "gtest/gtest.h"
#include "inc/TNOTireParser.h"
#include "inc/car_common.h"

class StringSplitTest : public testing::Test {
 protected:
  void SetUp() override { tx_car::splitString(m_str1, m_splitted, "="); }
  void TearDown() override {}

 protected:
  std::vector<std::string> m_splitted;
  const std::string m_str1 = "name=1.2345";
};

TEST_F(StringSplitTest, VDTest) {
  EXPECT_EQ(m_splitted.size(), 2);
  EXPECT_STREQ(m_splitted.at(0).c_str(), "name");
  EXPECT_STREQ(m_splitted.at(1).c_str(), "1.2345");
}

TEST(StringSpliter1_Test, VDTest) {
  std::string content = "VXLOW=1";
  std::vector<std::string> splitted;
  tx_car::splitString(content, splitted, "=");
  EXPECT_EQ(splitted.size(), 2);
  EXPECT_STREQ(splitted.at(0).c_str(), "VXLOW");
  EXPECT_STREQ(splitted.at(1).c_str(), "1");
}

TEST(StringSplitTest_SpliterPrefix, VDTest) {
  std::vector<std::string> splitted2;
  std::string content2 = "=1.2";
  tx_car::splitString(content2, splitted2);
  EXPECT_EQ(splitted2.size(), 1);
  EXPECT_STREQ(splitted2.at(0).c_str(), "1.2");
}

TEST(StringSplitTest_SpliterPostfix, VDTest) {
  std::vector<std::string> splitted3;
  std::string content3 = "key=";
  tx_car::splitString(content3, splitted3);
  EXPECT_EQ(splitted3.size(), 1);
  EXPECT_STREQ(splitted3.at(0).c_str(), "key");
}

TEST(StringSplitTest_NoSpliter, VDTest) {
  std::vector<std::string> splitted3;
  std::string content3 = "key";
  tx_car::splitString(content3, splitted3);
  EXPECT_EQ(splitted3.size(), 1);
  EXPECT_STREQ(splitted3.at(0).c_str(), "key");
}

TEST(stringSpecialCharTest, VDTest) {
  std::string content = "key    = -1.23   $comment 1 1 1\n";

  tx_car::trim(content);
  EXPECT_STREQ(content.c_str(), "key    = -1.23   $comment 1 1 1");

  tx_car::trim(content);
  EXPECT_STREQ(content.c_str(), "key    = -1.23   $comment 1 1 1");

  tx_car::rmCharFromString(content);
  EXPECT_STREQ(content.c_str(), "key=-1.23$comment111");

  tx_car::trim(content, '$');
  EXPECT_STREQ(content.c_str(), "key=-1.23");

  std::vector<std::string> splitted;
  tx_car::splitString(content, splitted);
  EXPECT_EQ(splitted.size(), 2);
  EXPECT_STREQ(splitted.at(0).c_str(), "key");
  EXPECT_STREQ(splitted.at(1).c_str(), "-1.23");

  content = "!comment 1 2 3 ";
  tx_car::trim(content, '!');
  EXPECT_STREQ(content.c_str(), "");

  content = "[key]";
  EXPECT_EQ(tx_car::beginWithChar(content, '['), true);
}

TEST(fileIO, VDTest) {
  std::string content = "hello world.\nhello world.\n";
  std::string file_path = "./tmp.txt";
  std::string content_rd;
  std::list<std::string> content_rd1;

  tx_car::car_config::dumpToFile(content, file_path);
  EXPECT_EQ(tx_car::isFileExist(file_path), true);

  tx_car::car_config::loadFromFile(content_rd, file_path);
  EXPECT_STREQ(content.c_str(), content_rd.c_str());

  tx_car::car_config::loadFromFile(content_rd1, file_path);
  EXPECT_EQ(content_rd1.size(), 2);
  EXPECT_STREQ(content_rd1.begin()->c_str(), "hello world.");
  EXPECT_STREQ((++content_rd1.begin())->c_str(), "hello world.");
}

TEST(TNOTest, VDTest) {
  tx_car::TNOTireParser tno_parser;

  tno_parser.setTirPath("./param/C100_Continental_225_50R18.tir");
  EXPECT_EQ(tno_parser.parseTNOTir(), true);
}

TEST(fileIOProtoJson, VDTest) {
  const std::string file_path = "E:\\work\\VehicleDynamics\\param\\txcar.json";
  std::string content_rd;
  EXPECT_EQ(tx_car::car_config::loadFromFile(content_rd, file_path), 1);
  EXPECT_GT(content_rd.size(), 0);
}

TEST(DerivedTest, VDTest) {
  class Foo {
   protected:
    int m_var;

   public:
    Foo() : m_var(0) {}
    int getVar() { return m_var; }
  };

  class Bar : public Foo {
   public:
    int getVar() {
      m_var = 1;
      return Foo::getVar();
    }
  };

  Bar bar;
  EXPECT_EQ(bar.getVar(), 1);
}
