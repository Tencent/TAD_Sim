// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_assembler_context.h"
#include "tx_header.h"

TX_NAMESPACE_OPEN(Scene)

class AssemblerFactory {
 protected:
  AssemblerFactory() TX_DEFAULT;

 public:
  /**
   * @brief 获取指定类型场景的程序集装配上下文
   *
   * 该函数用于根据给定的场景类型，返回对应的程序装配上下文。
   *
   * @param _sceneType 场景类型，由 ISceencLoader 定义
   * @return 返回对应场景类型的程序装配上下文指针，如果没有对应场景类型，返回 nullptr
   */
  static Base::IAssemblerContextPtr GetAssemblerContext(Base::ISceneLoader::ESceneType) TX_NOEXCEPT;
};

TX_NAMESPACE_CLOSE(Scene)
