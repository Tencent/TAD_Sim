// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "its_sim.pb.h"
#include "parallel_simulation/tx_tc_ps_element_generator.h"
#include "tx_map_info.h"
#include "tx_tc_assembler_context.h"
TX_NAMESPACE_OPEN(Scene)

class ParallelSimulation_AssemblerContext : public TAD_Cloud_AssemblerContext {
 public:
  ParallelSimulation_AssemblerContext() TX_DEFAULT;
  virtual ~ParallelSimulation_AssemblerContext() TX_DEFAULT;

  /**
   * @brief 初始化方法
   *
   * 初始化某种场景的渲染上下文。该方法可能会被重复调用，用于反复渲染同一场景的不同帧。
   *
   * @param _loader 场景加载器，用于加载场景相关的资源
   * @param valid_map_range 可用的地图范围，一般可以简单地设置为[0, 1]
   * @return Base::txBool 初始化成功则返回 true，否则返回 false
   */
  virtual Base::txBool Init(Base::ISceneLoaderPtr _loader,
                            const Base::map_range_t valid_map_range) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 生成场景
   *
   * 从指定的场景加载器中加载场景数据，并将其分配给渲染上下文。
   * 初始化函数 GenerateScene 的扩展版本。
   *
   * @param elem_mgr 元素管理器，用于在场景中添加元素
   * @param loader 场景加载器，用于加载场景数据
   * @return Base::txBool 如果生成成功则返回 true，否则返回 false
   */
  virtual Base::txBool GenerateScene(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

  /**
   * @brief 创建交通信号灯元素
   *
   * 通过交通信号灯配置文件创建交通信号灯元素，并将其添加到元素管理器中。
   *
   * @param elem_mgr 元素管理器，用于添加交通信号灯元素
   * @param traffic_config_path 交通信号灯配置文件路径
   * @return Base::txBool 如果创建成功则返回 true，否则返回 false
   */
  virtual Base::txBool CreateTrafficLightElement(Base::IElementManagerPtr,
                                                 const Base::txString traffic_config_path) TX_NOEXCEPT;

  /**
   * @brief 动态生成场景
   *
   * 使用指定的静态信息来动态生成场景，包括汽车、道路、红绿灯等元素。
   *
   * @param info 含有静态信息的汽车数组
   * @param _loader 场景加载器，用于从外部资源加载场景元素
   * @return Base::txBool 生成成功则返回 true，否则返回 false
   */
  virtual Base::txBool DynamicGenerateScene(its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
    return m_ps_VehicleElementGenerator->Generate(info, _loader);
  }

 protected:
  ParallelSimulation_ElementGeneratorPtr m_ps_VehicleElementGenerator = nullptr;
};

using ParallelSimulation_AssemblerContextPtr = std::shared_ptr<ParallelSimulation_AssemblerContext>;
TX_NAMESPACE_CLOSE(Scene)
