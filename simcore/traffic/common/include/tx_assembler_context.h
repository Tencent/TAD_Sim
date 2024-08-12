// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_element_manager_base.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(Base)

// @brief 场景元素创建接口
class IAssemblerContext {
 public:
  IAssemblerContext() TX_DEFAULT;
  virtual ~IAssemblerContext() TX_DEFAULT;

  /**
   * @brief 初始化
   *
   * @param pSceneLoader 场景加载器
   * @return txBool
   */
  virtual txBool Init(Base::ISceneLoaderPtr) TX_NOEXCEPT { return true; }

  /**
   * @brief GenerateScene 函数用于将场景元素加载至存储器
   *
   * @param[in] pElementManager 当前场景的元素管理器
   * @param[in] pSceneLoader 场景加载器
   * @return true 成功；false 失败
   */
  virtual txBool GenerateScene(IElementManagerPtr, ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief GenerateEgo 函数用于将ego元素加载至存储器
   *
   * @param[in] pElementManager 当前场景的元素管理器
   * @param[in] pSceneLoader 场景加载器
   * @return true 成功；false 失败
   */
  virtual txBool GenerateEgo(IElementManagerPtr, ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 重置context对象
   *
   * @return txBool
   */
  virtual txBool ResetContext() TX_NOEXCEPT { return true; }

  /**
   * @brief DynamicGenerateScene 函数用于动态加载场景元素
   *
   * @param timeParamManager 当前场景的时间参数管理器
   * @param pElementManager 当前场景的元素管理器
   * @param pSceneLoader 场景加载器
   * @return txBool 成功时返回 true，失败时返回 false
   */
  virtual txBool DynamicGenerateScene(const TimeParamManager &, IElementManagerPtr, ISceneLoaderPtr) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief DynamicChangeScene 函数用于动态切换场景
   *
   * @param timeParamManager 当前场景的时间参数管理器
   * @param pElementManager 当前场景的元素管理器
   * @param pSceneLoader 场景加载器
   * @return txBool 成功时返回 true，失败时返回 false
   */
  virtual txBool DynamicChangeScene(const TimeParamManager &, IElementManagerPtr, ISceneLoaderPtr) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief TestSerialization 测试对象序列化的功能
   *
   * @param timeParamManager 当前场景的时间参数管理器
   * @param pElementManager 当前场景的元素管理器
   * @return txBool 成功时返回 true，失败时返回 false
   */
  virtual txBool TestSerialization(const TimeParamManager &, IElementManagerPtr) TX_NOEXCEPT { return false; }
};

using IAssemblerContextPtr = std::shared_ptr<IAssemblerContext>;
TX_NAMESPACE_CLOSE(Base)
