// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_assembler_context.h"

TX_NAMESPACE_OPEN(Scene)

class L2W_AssemblerContext : public Base::IAssemblerContext {
 public:
  L2W_AssemblerContext() TX_DEFAULT;
  virtual ~L2W_AssemblerContext() TX_DEFAULT;

 public:
  /**
   * @brief 初始化装配器
   * @param[in] loader_ptr 场景加载器指针
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool Init(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 根据指定的加载器生成场景
   *
   * @param[in] element_manager_ptr 元素管理器的指针，用于管理场景中的元素
   * @param[in] scene_loader_ptr 场景加载器的指针，用于加载场景数据
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool GenerateScene(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 生成自身元素
   *
   * @param[in] elementManagerPtr 元素管理器的指针，用于管理场景中的元素
   * @param[in] sceneLoaderPtr 场景加载器的指针，用于加载场景数据
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool GenerateEgo(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重置上下文
   *
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool ResetContext() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 动态生成场景
   *
   * @param[in] timeManager 时间参数管理器
   * @param[in] elementManager 元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool DynamicGenerateScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                            Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

  /**
   * @brief 动态更换场景
   *
   * @param[in] timeManager 时间参数管理器
   * @param[in] elementManager 元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool DynamicChangeScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                          Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

 protected:
  /**
   * @brief 根据场景加载器生成流量流元素
   * @param[in] elementManager 元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 成功返回生成的流量流元素数量, 失败返回 -1
   */
  virtual Base::txInt GenerateTrafficFlowElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 生成流量流元素
   *
   * 根据传入的场景加载器生成一个流量流元素，默认是车辆。
   * 用户可以通过覆盖此方法来自定义生成的流量流元素。
   *
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回生成的流量流元素数量，否则返回 -1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 根据场景加载器生成一个人群流量流元素
   *
   * 该方法用于生成一个新的人群流量流元素，通常适用于道路链路评估过程。
   *
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回生成的人群流量流元素数量，否则返回 -1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Pedestrian(Base::IElementManagerPtr,
                                                            Base::ISceneLoaderPtr) TX_NOEXCEPT {
    return 0;
  }

  /**
   * @brief 生成一个障碍物流量流元素
   *
   * 该方法用于生成一个新的障碍物流量流元素，通常适用于道路链路评估过程。
   *
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回生成的障碍物流量流元素数量，否则返回 -1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Obstacle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT {
    return 0;
  }

  /**
   * @brief 根据场景加载器生成一个障碍物流量流元素
   *
   * 该方法用于生成一个新的障碍物流量流元素，通常适用于道路链路评估过程。
   *
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回生成的障碍物流量流元素数量，否则返回 -1
   */
  virtual Base::txInt GenerateTrafficEnvironmentElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT {
    return 0;
  }

  /**
   * @brief 根据场景加载器生成指向某个场景中障碍物的引用
   *
   * 该方法用于生成一个新的障碍物引用元素，通常适用于道路链路评估过程。
   *
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回生成的障碍物引用元素数量，否则返回 -1
   */
  virtual Base::txInt GenerateTrafficReferenceElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT {
    return 0;
  }
};

using L2W_AssemblerContextPtr = std::shared_ptr<L2W_AssemblerContext>;

TX_NAMESPACE_CLOSE(Scene)
