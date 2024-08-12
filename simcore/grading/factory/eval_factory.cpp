// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory.h"

namespace eval {
EvalFactory::EvalFactory() {}
EvalFactory::~EvalFactory() {}

// register an eval kpi algorithm with its build function
void EvalFactory::Register(const std::string &kpi_name, MakeEvalFunc make_func) {
  _eval_algorithm_maker[kpi_name] = make_func;
}

// make one eval kpi algorithm
EvalBasePtr EvalFactory::Make(const std::string &kpi_name) {
  try {
    if (_eval_algorithm_maker.find(kpi_name) != _eval_algorithm_maker.end()) {
      return _eval_algorithm_maker[kpi_name]();
    }
  } catch (const std::exception &e) {
    LOG_ERROR << "eval | fail to make eval algorithm for " << kpi_name << ", " << e.what() << "\n";
  }
  return EvalBasePtr(nullptr);
}

EvalAlgorithmMap EvalFactory::MakeAll() {
  EvalAlgorithmMap eval_map;

  for (auto pair = _eval_algorithm_maker.begin(); pair != _eval_algorithm_maker.end(); ++pair) {
    try {
      auto algorithm = pair->second();
      if (algorithm) eval_map[pair->first] = algorithm;
    } catch (const std::exception &e) {
      LOG_ERROR << "fail to make algorithm for " << pair->first << ", " << e.what() << "\n";
    }
  }

  return eval_map;
}
}  // namespace eval
