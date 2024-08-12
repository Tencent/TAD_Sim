/**
 * @file age.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief test for age
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "../age.h"
#include "gtest/gtest.h"

/**
 * @brief Construct a new TEST object
 *
 */
TEST(AgeTest, main) {
  Age age;
  EXPECT_EQ(age(0), 1);
  EXPECT_EQ(age(1), 1);
  EXPECT_EQ(age(2), 1);
  EXPECT_EQ(age(0), 1);
  EXPECT_EQ(age(1), 1);
  EXPECT_EQ(age(2), 1);

  age.YearEnd();
  EXPECT_EQ(age(0), 2);
  EXPECT_EQ(age(1), 2);
  EXPECT_EQ(age(0), 2);
  EXPECT_EQ(age(1), 2);

  age.YearEnd();
  EXPECT_EQ(age(0), 3);
  EXPECT_EQ(age(1), 3);
  EXPECT_EQ(age(2), 1);
}
