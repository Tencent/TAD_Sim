// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "manager/report_helper.h"
#include "unittest/unittest.h"

namespace eval {
// 测试用例：查找不在分数映射中的负值
TEST(LookUpFuncTest_BadMap_0, GradingTest) {
  // 创建一个空的分数映射对象
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  // 设置要查询的值
  double u = 0;
  // 使用ReportHelper类调用lookupScoreFromMap1d函数
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, u);
  // 检查返回结果是否小于INT32_MIN+1（表示分数映射中不存在该值的情况）
  EXPECT_LT(ret, INT32_MIN + 1);
}

// 测试用例：查找从map中获取分数时，传入的分数不在范围内（负值）
TEST(LookUpFuncTest_BadMap_1, GradingTest) {
  // 创建一个包含两个元素的分数映射对象
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  // 添加两个分数到映射中
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_y()->Add(0);
  // 设置要查询的分数为0
  double u = 0;
  // 使用ReportHelper类中的lookupScoreFromMap1d方法进行查询
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, u);
  // 检查返回结果是否小于INT32_MIN+1
  EXPECT_LT(ret, INT32_MIN + 1);
}

// 测试用例：查找分数在map中不存在的情况
TEST(LookUpFuncTest_BadMap_2, GradingTest) {
  // 创建一个包含两个分数的map
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  // 添加分数到map中
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  // 设置对应的得分
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  // 查找分数
  double u = 0;
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, u);
  // 检查返回值是否小于INT32_MIN+1（表示分数不存在）
  EXPECT_LT(ret, INT32_MIN + 1);
}

// 测试用例：查找分数在map中不存在的情况
TEST(LookUpFuncTest_BadMap_3, GradingTest) {
  // 创建一个包含两个分数的map
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  // 添加第一个分数
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_y()->Add(100);
  // 添加第二个分数
  scoreMap.mutable_y()->Add(80);
  // 设置要查询的分数
  double u = 0;
  // 使用lookupScoreFromMap1d函数尝试从map中获取分数
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, u);
  // 检查返回值是否小于INT32_MIN+1（表示分数不存在）
  EXPECT_LT(ret, INT32_MIN + 1);
}

// 测试用例：查找分数在map中不存在的情况
TEST(LookUpFuncTest_BadMap_4, GradingTest) {
  // 创建一个包含两个分数的map
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  // 添加分数到map中
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(1);
  // 添加对应的得分
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  // 设置要查询的分数
  double u = 0;
  // 使用lookupScoreFromMap1d函数从map中查找分数
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, u);
  // 检查返回结果是否小于INT32_MIN+1（表示分数不存在）
  EXPECT_LT(ret, INT32_MIN + 1);
}

TEST(LookUpFuncTest_1, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 0);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 100, 1e-6);
}

TEST(LookUpFuncTest_2, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, -1);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 100, 1e-6);
}

TEST(LookUpFuncTest_3, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 5);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 0, 1e-6);
}

TEST(LookUpFuncTest_4, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 6);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 0, 1e-6);
}

TEST(LookUpFuncTest_5, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 2);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 60, 1e-6);
}

TEST(LookUpFuncTest_6, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(60);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 3.5);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 30, 1e-6);
}

TEST(LookUpFuncTest_7, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(0);

  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 3.5);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 50, 1e-6);
}

TEST(LookUpFuncTest_8, GradingTest) {
  // 创建一个示例的评分映射对象，包含四个元素（成绩）。
  sim_msg::GradingKpiGroup_GradingKpi_Parameters_ScoreMap1D scoreMap;
  scoreMap.mutable_u()->Add(0);
  scoreMap.mutable_u()->Add(1);
  scoreMap.mutable_u()->Add(2);
  scoreMap.mutable_u()->Add(5);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(80);
  scoreMap.mutable_y()->Add(100);
  scoreMap.mutable_y()->Add(0);

  // 使用示例评分映射对象调用 lookupScoreFromMap1d 函数，传入一个数作为查询的键。
  auto ret = ReportHelper::lookupScoreFromMap1d(scoreMap, 1.5);
  // 检查返回结果是否接近预期的值。
  EXPECT_NEAR(ret, 90, 1e-6);
}
}  // namespace eval
