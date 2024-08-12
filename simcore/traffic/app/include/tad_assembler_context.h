// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_element_generator.h"
#include "tx_assembler_context.h"
#include "tx_event_dispatcher.h"
TX_NAMESPACE_OPEN(Scene)

class TAD_AssemblerContext : public Base::IAssemblerContext {
 public:
  using txEventDispatcher = TrafficFlow::txEventDispatcher;
  TAD_AssemblerContext() TX_DEFAULT;
  virtual ~TAD_AssemblerContext() TX_DEFAULT;
  /**
   * @brief 用于初始化场景加载器
   *
   * @return Base::txBool
   */
  virtual Base::txBool Init(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于生成场景
   *
   * @return Base::txBool
   */
  virtual Base::txBool GenerateScene(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于生成自身（Ego）
   *
   * @return Base::txBool
   */
  virtual Base::txBool GenerateEgo(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于重置上下文
   *
   * @return Base::txBool
   */
  virtual Base::txBool ResetContext() TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于动态生成场景
   *
   * @return Base::txBool
   */
  virtual Base::txBool DynamicGenerateScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                            Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于动态更改场景
   *
   * @return Base::txBool
   */
  virtual Base::txBool DynamicChangeScene(const Base::TimeParamManager &, Base::IElementManagerPtr,
                                          Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 用于测试序列化
   *
   * @return Base::txBool
   */
  virtual Base::txBool TestSerialization(const Base::TimeParamManager &,
                                         Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief GenerateTrafficFlowElement，该函数接受两个指针参数，并返回 Base::txInt 类型的值
   *
   * @param managerPtr 一个指向 Base::IElementManagerPtr 类型的指针，表示元素管理器的指针
   * @param loaderPtr 一个指向 Base::ISceneLoaderPtr 类型的指针，表示场景加载器的指针
   * @return Base::txInt
   */
  virtual Base::txInt GenerateTrafficFlowElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  /**
   * @brief 生成车流元素的车辆数据
   *
   * @param managerPtr 一个指向 Base::IElementManagerPtr 类型的指针，表示元素管理器的指针
   * @param loaderPtr 一个指向 Base::ISceneLoaderPtr 类型的指针，表示场景加载器的指针
   * @return Base::txInt
   */
  virtual Base::txInt GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  /**
   * @brief 生成流量模拟中的行人数据
   *
   * @param pElementManager 元素管理器指针
   * @param pSceneLoader 场景加载器指针
   * @return 成功时返回行人数
   */
  virtual Base::txInt GenerateTrafficFlowElement_Pedestrian(Base::IElementManagerPtr,
                                                            Base::ISceneLoaderPtr) TX_NOEXCEPT;
  /**
   * @brief 生成车流模拟中的障碍物数据
   *
   * @param pElementManager 元素管理器指针
   * @param pSceneLoader 场景加载器指针
   * @return 成功时返回障碍物数据总量
   */
  virtual Base::txInt GenerateTrafficFlowElement_Obstacle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  /**
   * @brief 生成交通环境元素
   *
   * @param pElementManager 元素管理器指针
   * @param pSceneLoader 场景加载器指针
   * @return 成功时返回环境元素数量
   */
  virtual Base::txInt GenerateTrafficEnvironmentElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  /**
   * @brief 生成流量参考元素，目前暂无实现
   *
   * @param pElementManager 元素管理器指针
   * @param pSceneLoader 场景加载器指针
   * @return 0
   */
  virtual Base::txInt GenerateTrafficReferenceElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

 protected:
  Scene::TAD_ElementGeneratorPtr m_TAD_VehicleElementGenerator = nullptr;
};

using TAD_AssemblerContextPtr = std::shared_ptr<TAD_AssemblerContext>;
TX_NAMESPACE_CLOSE(Scene)
