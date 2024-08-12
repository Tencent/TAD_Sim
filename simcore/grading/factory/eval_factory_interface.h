// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @file EvalAlgorithmBuilder.h
 * @author DongYuanHu (dongyuanhu@tencent.com)
 * @brief EvalFactory class will register to this class.
 * This class can be used to make eval algorithms
 * @version 0.1
 * @date 2023-04-23
 *
 *
 */
#pragma once

#include <mutex>
#include "eval_factory.h"
#include "utils/eval_common.h"
#include "visibility.h"

/**
 * @brief define a global EvalFactory generator variable. must be call in .cpp file.
 * EvalFactory is automatically registerred to EvalFactoryInterface during the construct phase of this global variable.
 *
 */
#define DefineEvalFactory(EvalFactoryClass) EvalFactoryGenerator<EvalFactoryClass> global##EvalFactoryClass

namespace eval {
class EvalFactoryInterface;
extern std::shared_ptr<EvalFactoryInterface> globalFactory;
std::shared_ptr<EvalFactoryInterface> getGlobalFactory();

extern "C" {
// use it to get globalFactory from external library
TXSIM_API eval::EvalFactoryInterface* cGetGlobalFactory();
}  // extern "C"

class EvalFactoryInterface : public NoneCopyable {
 public:
  EvalFactoryInterface();
  virtual ~EvalFactoryInterface();

  /**
   * @brief push_back a eval factory
   *
   * @param evalFactory
   */
  void registerEvalFactory(const std::shared_ptr<eval::EvalFactory>& evalFactory);

  /**
   * @brief make eval algorithm by kpi name
   *
   * @param kpi_name
   * @return EvalBasePtr
   */
  EvalBasePtr Make(const std::string& kpi_name);

 private:
  std::vector<std::shared_ptr<EvalFactory>> m_factories;
};

/**
 * @brief EvalFactoryGenerator, automatically register EvalFactory class to EvalFactoryInterface
 * during construction phase.
 *
 * @tparam FactoryT
 */
template <class FactoryT>
class EvalFactoryGenerator {
 public:
  EvalFactoryGenerator() {
    auto factoryInst = std::make_shared<FactoryT>();
    auto gf = getGlobalFactory();
    gf->registerEvalFactory(factoryInst);
  }
};

}  // namespace eval
