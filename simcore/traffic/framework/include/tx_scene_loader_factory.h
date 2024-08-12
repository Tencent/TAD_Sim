// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(Base)

class SceneLoaderFactory {
 public:
  explicit SceneLoaderFactory(txString const& strSceneType) : _sceneType(strSceneType) {}
  virtual ~SceneLoaderFactory() TX_DEFAULT;

  /**
   * @brief 创建场景加载器
   *
   * 创建一个用于加载场景的实例。
   *
   * @return ISceneLoaderPtr 返回一个指向场景加载器实例的指针
   */
  virtual Base::ISceneLoaderPtr CreateSceneLoader() const TX_NOEXCEPT;

  /**
   * @brief 通过路径字符串将外部名称名称映射到场景类型
   *
   * 从给定的路径字符串中获取场景类型。根据不同的文件扩展名匹配不同的场景类型。
   *
   * @param pPath 待解析的文件路径字符串指针
   * @return txString 返回对应的场景类型，如果没有匹配到则返回空字符串
   */
  static txString ExternNameToSceneTypeByPath(txString const&) TX_NOEXCEPT;

 protected:
  /**
   * @brief 根据文件路径评估场景类型
   *
   * 根据给定的文件路径，评估并返回与该文件类型对应的场景类型。
   *
   * @return ISceneLoader::ESceneType 返回与文件类型对应的场景类型
   */
  ISceneLoader::ESceneType EvaluteSceneType() const TX_NOEXCEPT;

 protected:
  const txString _sceneType;
};

TX_NAMESPACE_CLOSE(Base)
