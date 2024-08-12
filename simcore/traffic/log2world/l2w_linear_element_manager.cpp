// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "l2w_linear_element_manager.h"
TX_NAMESPACE_OPEN(TrafficFlow)

L2W_LinearElementManager::L2W_LinearElementManager() { mShadowLayerPtr = std::make_shared<LinearElementManager>(); }

Base::txBool L2W_LinearElementManager::Initialize(Base::ISceneLoaderPtr _dataSource) TX_NOEXCEPT {
  using namespace Geometry::Topology;
  // 将数据源赋值给成员变量 m_DataSource
  m_DataSource = _dataSource;
  // 生成所有交通元素
  GenerateAllTrafficElement();
  // 初始化阴影层，使用数据源
  mShadowLayerPtr->Initialize(_dataSource);
  return true;
}

TX_NAMESPACE_CLOSE(TrafficFlow)
