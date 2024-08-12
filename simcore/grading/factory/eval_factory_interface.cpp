// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "eval_factory_interface.h"

namespace eval {
std::shared_ptr<EvalFactoryInterface> globalFactory;
// std::mutex globalFactoryLock;

std::shared_ptr<EvalFactoryInterface> getGlobalFactory() {
  // std::lock_guard<std::mutex> guard(globalFactoryLock);
  if (globalFactory.get() == nullptr) {
    globalFactory = std::make_shared<EvalFactoryInterface>();
  }
  return globalFactory;
}

extern "C" {
eval::EvalFactoryInterface* cGetGlobalFactory() {
  if (globalFactory.get() == nullptr) {
    globalFactory = std::make_shared<EvalFactoryInterface>();
  }
  return globalFactory.get();
}
}  // extern "C"

EvalFactoryInterface::EvalFactoryInterface() {
  m_factories.reserve(64);
  std::cout << "globalFactory Constructed.\n";
}
EvalFactoryInterface::~EvalFactoryInterface() {}

void EvalFactoryInterface::registerEvalFactory(const std::shared_ptr<EvalFactory>& evalFactory) {
  if (evalFactory.get() != nullptr) {
    m_factories.push_back(evalFactory);
  }
}
// make one eval kpi algorithm
EvalBasePtr EvalFactoryInterface::EvalFactoryInterface::Make(const std::string& kpi_name) {
  for (size_t i = 0; i < m_factories.size(); ++i) {
    if (m_factories.at(i)) {
      auto algorithm_general = m_factories.at(i)->Make(kpi_name);
      if (algorithm_general.get()) return algorithm_general;
    }
  }
  return EvalBasePtr(nullptr);
}
}  // namespace eval
