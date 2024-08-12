// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "utils/eval_common.h"

namespace eval {
// to make eval algorithm
typedef std::function<EvalBasePtr(void)> MakeEvalFunc;
typedef std::unordered_map<std::string, MakeEvalFunc> EvalAlgorithmMaker;

/**
 * @brief "评测指标组注册工厂基类", eval kpi factory base type, which can't be copied.
 */
class EvalFactory : public NoneCopyable {
 protected:
  EvalAlgorithmMaker _eval_algorithm_maker;

 public:
  EvalFactory();
  virtual ~EvalFactory();

  // register an eval kpi algorithm with its build function
  void Register(const std::string &kpi_name, MakeEvalFunc make_func);

  // make one eval kpi algorithm
  EvalBasePtr Make(const std::string &kpi_name);

  EvalAlgorithmMap MakeAll();
};
}  // namespace eval
