// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_scene_loader_factory.h"

TX_NAMESPACE_OPEN(Scene)

class TAD_SceneLoaderFactory : public Base::SceneLoaderFactory {
  using ParentClass = Base::SceneLoaderFactory;

 public:
  explicit TAD_SceneLoaderFactory(Base::txString const& strSceneType) : Base::SceneLoaderFactory(strSceneType) {}

  virtual ~TAD_SceneLoaderFactory() TX_DEFAULT;

  /**
   * @brief 创建场景加载器
   *
   * @return Base::ISceneLoaderPtr
   */
  virtual Base::ISceneLoaderPtr CreateSceneLoader() const TX_NOEXCEPT TX_OVERRIDE;
};

TX_NAMESPACE_CLOSE(Scene)
