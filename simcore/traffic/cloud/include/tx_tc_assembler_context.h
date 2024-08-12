// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "road_realtimeinfo.pb.h"
#include "tx_assembler_context.h"
#include "tx_map_info.h"
#include "tx_tc_element_generator.h"
#include "tx_tc_marco.h"
TX_NAMESPACE_OPEN(Scene)
class TAD_Cloud_AssemblerContext : public Base::IAssemblerContext {
 public:
  TAD_Cloud_AssemblerContext() TX_DEFAULT;
  virtual ~TAD_Cloud_AssemblerContext() TX_DEFAULT;

  /**
   * @brief 初始化
   * @param[in] scene_loader 待加载的场景信息
   * @param[in] valid_map_range 合法的地图范围，包含起始点和结束点
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual Base::txBool Init(Base::ISceneLoaderPtr, const Base::map_range_t valid_map_range) TX_NOEXCEPT;

  /**
   * @brief 根据场景加载器生成场景
   *
   * 加载场景元素和属性，并将其组合成具有完整功能的场景实例。在生成场景过程中，需要考虑有效的映射范围。
   *
   * @param[in] scene_loader 用于加载场景元素的场景加载器
   * @param[in] valid_map_range 有效的地图范围，包括起始点和结束点
   * @return 成功生成场景返回 true，否则返回 false
   */
  virtual Base::txBool GenerateScene(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据场景加载器生成 ego 对象
   * @param[in] scene_loader 用于加载场景元素的场景加载器
   * @param[in] element_mgr 场景元素管理器
   * @return 生成 ego 成功返回 true，否则返回 false
   */
  virtual Base::txBool GenerateEgo(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 动态生成场景
   * @param[in] timeParamManager 时间参数管理器
   * @param[in] elementMgr 场景元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 生成成功返回 true，否则返回 false
   */
  virtual Base::txBool DynamicGenerateScene(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                            Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 动态更换场景
   * @param[in] timeParamManager 时间参数管理器
   * @param[in] elementMgr 场景元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 更换成功返回 true，否则返回 false
   */
  virtual Base::txBool DynamicChangeScene(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                          Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重新设置输入代理
   * @param vec_valid_sim_range 代理可以模拟的范围
   * @return 成功返回true, 失败返回false
   */
  virtual Base::txBool ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range) TX_NOEXCEPT;

  /**
   * @brief 检查序列化是否成功
   * @param timeParamManager 时间参数管理器
   * @param elementManager 元素管理器
   * @return 成功返回 true，失败返回 false
   */
  virtual Base::txBool TestSerialization(const Base::TimeParamManager&,
                                         Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;
#if __cloud_ditw__

  /**
   * @brief Generate3rdVehicle 生成第三方车辆
   * @param timeParamManager 时间参数管理器
   * @param _3rdData 第三方数据
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return void
   */
  virtual void Generate3rdVehicle(const Base::TimeParamManager&, const ObjectInfo& _3rdData, Base::IElementManagerPtr,
                                  Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 重置上下文，用于重置生成器的状态，以便重新开始生成车辆元素
   * @return 成功返回true，失败返回false
   */
  virtual Base::txBool ResetContext() TX_NOEXCEPT TX_OVERRIDE {
    mVehicleElementGenerator->ResetGenerator();
    return true;
  }
#endif /*__cloud_ditw__*/

 protected:
  /**
   * @brief DynamicGenerateTrafficFlowElement 动态生成交通流元素
   * @param[in] timeParamManager 时间参数管理器
   * @param[in] elementManager 元素管理器
   * @param[in] sceneLoader 场景加载器
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt DynamicGenerateTrafficFlowElement(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                                        Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 动态生成交通流元素，车辆
   * @param timeParamManager 时间参数管理器
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt DynamicGenerateTrafficFlowElement_Vehicle(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                                                Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 动态生成步行人流量元素
   * @param timeParamManager 时间参数管理器
   * @param elementManager 元素管理器
   * @param sceneLoader 场景加载器
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt DynamicGenerateTrafficFlowElement_Pedestrian(Base::TimeParamManager const&,
                                                                   Base::IElementManagerPtr,
                                                                   Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 动态生成交通环境元素
   *
   * 该函数通过一系列的步骤来动态生成交通环境元素。它需要时间参数管理器、元素管理器和场景加载器作为输入参数。此外，它还可以设置其他可选参数，例如交通环境元素的类型、材质等。
   *
   * @param timeParamManager 时间参数管理器，用于获取当前时间以及游戏速度等信息
   * @param elementManager 元素管理器，用于创建和管理交通环境元素
   * @param sceneLoader 场景加载器，用于加载交通环境元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt DynamicGenerateTrafficEnvironmentElement(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                                               Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 动态生成交通流量参考元素
   *
   * 该函数用于动态生成交通流量参考元素。它需要一个时间参数管理器、一个元素管理器和一个场景加载器作为输入参数。此外，它还可以设置其他可选参数，例如交通流量参考元素的类型、材质等。
   *
   * @param timeParamManager 时间参数管理器，用于获取当前时间以及游戏速度等信息
   * @param elementManager 元素管理器，用于创建和管理交通流量参考元素
   * @param sceneLoader 场景加载器，用于加载交通流量参考元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt DynamicGenerateTrafficReferenceElement(Base::TimeParamManager const&, Base::IElementManagerPtr,
                                                             Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 生成交通环境元素
   *
   * 该函数用于动态生成交通环境元素。它需要一个元素管理器和一个场景加载器作为输入参数，用于创建和管理交通环境元素。
   *
   * @param elementManager 元素管理器，用于创建和管理交通环境元素
   * @param sceneLoader 场景加载器，用于加载交通环境元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt GenerateTrafficEnvironmentElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 动态生成交通流量元素
   *
   * 该函数用于动态生成交通流量元素。它需要一个元素管理器和一个场景加载器作为输入参数，用于创建和管理交通流量元素。
   *
   * @param elementManager 元素管理器，用于创建和管理交通流量元素
   * @param sceneLoader 场景加载器，用于加载交通流量元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt GenerateTrafficFlowElement(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 生成交通流量元素 车辆
   *
   * 该函数用于动态生成交通流量元素。它需要一个元素管理器和一个场景加载器作为输入参数，用于创建和管理交通流量元素。
   *
   * @param elementManager 元素管理器，用于创建和管理交通流量元素
   * @param sceneLoader 场景加载器，用于加载交通流量元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Vehicle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 生成交通流量元素 行人
   *
   * 该函数用于动态生成交通流量元素。它需要一个元素管理器和一个场景加载器作为输入参数，用于创建和管理交通流量元素。
   *
   * @param elementManager 元素管理器，用于创建和管理交通流量元素
   * @param sceneLoader 场景加载器，用于加载交通流量元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Pedestrian(Base::IElementManagerPtr,
                                                            Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 生成交通流量元素 障碍物
   *
   * 该函数用于动态生成交通流量元素。它需要一个元素管理器和一个场景加载器作为输入参数，用于创建和管理交通流量元素。
   *
   * @param elementManager 元素管理器，用于创建和管理交通流量元素
   * @param sceneLoader 场景加载器，用于加载交通流量元素所需的资源
   *
   * @return 成功返回0，失败返回-1
   */
  virtual Base::txInt GenerateTrafficFlowElement_Obstacle(Base::IElementManagerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT;

 protected:
  Base::ISceneLoaderPtr mSceneLoader = nullptr;
  Base::ISceneElementGeneratorPtr mVehicleElementGenerator = nullptr;
};

using TAD_Cloud_AssemblerContextPtr = std::shared_ptr<TAD_Cloud_AssemblerContext>;
TX_NAMESPACE_CLOSE(Scene)
