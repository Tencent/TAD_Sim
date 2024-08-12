// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_obstacle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_Cloud_ObstacleElement : public TAD_ObstacleElement {
 public:
  /**
   * @brief 将当前障碍物元素初始化
   *
   * @param[in] obsId 碰撞体元素的标识符
   * @param[in] wgs84_pt 碰撞体元素在WGS84平面上的坐标
   * @param[in] loaderPtr 场景加载器的智能指针
   *
   * @return TX_TRUE 初始化成功
   * @return TX_FALSE 初始化失败
   */
  virtual txBool InitializeOnCloud(const Base::txSysId obsId, Coord::txWGS84 wgs84_pt,
                                   Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 注册初始化信息
   *
   * 根据需要，在组件初始化时注册组件所需的信息。
   *
   * @param _location 组件的位置信息
   */
  virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新哈希键的道路信息
   *
   * 根据需要，在组件初始化时更新组件所需的哈希键信息。
   *
   * @param _location 组件的位置信息
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 死亡时注销信息
   *
   * 当某个障碍物被消除后，执行此函数注销信息。
   *
   * @return void
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE;
};

using TAD_Cloud_ObstacleElementPtr = std::shared_ptr<TAD_Cloud_ObstacleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
