// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_linear_element_manager.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class L2W_LinearElementManager : public LinearElementManager {
 public:
  L2W_LinearElementManager();

  /**
   * @brief 初始化游戏世界中的线性元素管理器
   *
   * 初始化线性元素管理器，加载场景中的线性元素。
   *
   * @param sceneLoader 用于加载场景的加载器
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取元素管理器指针
   *
   * @return Base::IElementManagerPtr
   */
  virtual Base::IElementManagerPtr ShadowLayer() TX_NOEXCEPT { return mShadowLayerPtr; }

 protected:
  Base::IElementManagerPtr mShadowLayerPtr = nullptr;
};
using L2W_LinearElementManagerPtr = std::shared_ptr<L2W_LinearElementManager>;

TX_NAMESPACE_CLOSE(TrafficFlow)
