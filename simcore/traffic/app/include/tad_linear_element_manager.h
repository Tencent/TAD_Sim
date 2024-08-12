// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_linear_element_manager.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_LinearElementManager : public LinearElementManager {
 public:
  /**
   * @brief 初始化线性元素管理器
   *
   * 此方法用于初始化线性元素管理器，并使其准备好加载扫描场景。
   * @param sceneLoader 扫描场景加载器指针
   * @return txBool true 表示初始化成功，返回 false 表示初始化失败。
   */
  virtual txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
