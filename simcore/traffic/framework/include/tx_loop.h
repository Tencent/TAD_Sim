// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"

TX_NAMESPACE_OPEN(Base)

class txLoop {
 public:
  virtual ~txLoop() TX_DEFAULT;

  /**
   * @brief 计算场景类型
   *
   * @param scene_path 场景文件路径
   * @return txString
   */
  virtual txString ComputeSceneType(const txString scene_path) TX_NOEXCEPT;

  /**
   * @brief xosc转换为sim
   *
   * @param xosc_scene_name xosc文件的路径
   * @return Base::txString
   */
  virtual Base::txString ConvertFromXOSC2Sim(const Base::txString xosc_scene_name) const TX_NOEXCEPT;
};

TX_NAMESPACE_CLOSE(Base)
