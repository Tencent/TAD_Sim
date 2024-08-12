// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_assembler_context.h"

TX_NAMESPACE_OPEN(Scene)

class TAD_ManualAssemblerContext : public TAD_AssemblerContext {
 public:
  using txEventDispatcher = TrafficFlow::txEventDispatcher;
  TAD_ManualAssemblerContext() TX_DEFAULT;
  virtual ~TAD_ManualAssemblerContext() TX_DEFAULT;
  virtual Base::txBool DynamicGenerateScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                            Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }
  virtual Base::txBool DynamicChangeScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                          Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

 protected:
  virtual Base::txInt GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr,
                                                         Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Scene::TAD_ElementGeneratorPtr m_TAD_VehicleElementGenerator = nullptr;
};

using TAD_ManualAssemblerContextPtr = std::shared_ptr<TAD_ManualAssemblerContext>;
TX_NAMESPACE_CLOSE(Scene)
