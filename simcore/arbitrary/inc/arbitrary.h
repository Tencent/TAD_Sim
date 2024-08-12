#pragma once

#include "txsim_module.h"

#include <memory>
#include <string>

namespace arbitrary {
class CArbitraryImp;
using CArbitraryImpPtr = std::shared_ptr<CArbitraryImp>;

namespace topic {
const std::string CONTROL_DRIVER = "CONTROL_DRIVER";
const std::string CONTROL_ALGORITHM = "CONTROL_ALGORITHM";
const std::string CONTROL = "CONTROL_V2";
}  // namespace topic
class CArbitrary final : public tx_sim::SimModule {
 public:
  CArbitrary();
  virtual ~CArbitrary();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 private:
  std::string name_;
  CArbitraryImpPtr m_arbitrary;
};
}  // namespace arbitrary
