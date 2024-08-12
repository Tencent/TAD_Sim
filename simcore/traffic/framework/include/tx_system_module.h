// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"

TX_NAMESPACE_OPEN(Base)

class ISystemModule {
 public:
  virtual ~ISystemModule() TX_DEFAULT;
  /**
   * @brief 初始化加载场景
   * @param loader 场景加载器
   * @return 初始化成功返回true，失败返回false
   */
  virtual txBool Initialize(ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief Update 更新操作
   * @param timeParamManager 时间参数管理器
   * @return 更新成功返回true，失败返回false
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT = 0;

  /**
   * @brief Release 释放对象
   * @return 释放成功返回 true，否则返回 false
   */
  virtual txBool Release() TX_NOEXCEPT = 0;
};

using ISystemModulePtr = std::shared_ptr<ISystemModule>;

TX_NAMESPACE_CLOSE(Base)
