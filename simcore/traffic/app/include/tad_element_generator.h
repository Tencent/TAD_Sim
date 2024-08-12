// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hashed_road_area.h"
#include "SceneLoader/tx_tadloader.h"
#include "tad_component.h"
#include "tad_route_group_agent.h"
#include "tx_dead_line_area_manager.h"
#include "tx_element_generator.h"
#include "tx_element_manager_base.h"
#include "tx_header.h"
#include "tx_sim_time.h"
TX_NAMESPACE_OPEN(Scene)

class TAD_ElementGenerator : public Base::ISceneElementGenerator {
 public:
  using LocationAgent = Base::LocationAgent;
  using VehicleInputeAgent = Base::VehicleInputeAgent;
  using VehicleExitAgent = Geometry::SpatialQuery::txHashedRoadArea; /*Base::VehicleExitAgent;*/
  using TAD_RouteGroupAgent = Scene::TAD_RouteGroupAgent;
  using ParentClass = Base::ISceneElementGenerator;

 public:
  virtual ~TAD_ElementGenerator() { Release(); }

  /**
   * @brief 初始化元素生成器
   *
   * @param[in] pSceneLoader 场景加载器
   * @param[in] valid_map_range 有效地图范围
   *
   * @return Base::txBool 是否初始化成功
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr,
                                  const Base::map_range_t& valid_map_range) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化元素生成器
   *
   * @param[in] pSceneLoader 场景加载器
   * @param[in] valid_map_range 有效地图范围
   *
   * @return Base::txBool 是否初始化成功
   */
  virtual Base::txBool Generate(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间参数和元素配置删除元素
   *
   * @param[in] param_manager 时间参数管理器
   * @param[in] elem_manager 元素管理器
   *
   * @return Base::txBool 是否成功删除
   */
  virtual Base::txBool Erase(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 实现重新路由
   *
   * @param[in] param_manager 时间参数管理器
   * @param[in] elem_manager 元素管理器
   *
   * @return 返回 true 表示重新路由成功
   * @return false 重新路由失败
   */
  virtual Base::txBool ReRoute(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放所有的资源
   *
   * 释放当前元素实例占用的所有资源。此方法在执行成功时返回true，否则返回false。
   *
   * @return Base::txBool 返回 true 表示释放成功，返回 false 表示释放失败
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重置生成器
   *
   * 重置生成器实例，以便从头开始重新生成元素。此方法返回重置操作是否成功。
   *
   * @return Base::txBool 如果重置操作成功返回 true，否则返回 false
   */
  virtual Base::txBool ResetGenerator() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 确定输入点是否安全
   *
   * 检查输入点是否在一个安全的区域内，从而决定是否允许用户执行相应的输入操作。
   *
   * @param inputPt 用户输入的点
   * @param nearestPt 最近的点
   * @return Base::txBool 如果输入点安全返回 true，否则返回 false
   */
  virtual Base::txBool SafeInputRegion(const Base::txVec2& inputPt,
                                       const Base::txVec2& nearestPt) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回输入区域的坐标列表
   *
   * @return std::vector< Coord::txWGS84 > 包含输入区域坐标的列表
   */
  virtual std::vector<Coord::txWGS84> InputRegionLocations() const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 返回输入区域的坐标列表
   *
   * 该函数返回一个包含输入区域中坐标的列表，这些坐标将用于判断用户输入的合法性和获取最近点等操作。
   * 在扩展该类的子类中，应重写此函数，以便为特定情况提供自定义输入区域。
   *
   * @return std::vector< Coord::txWGS84 > 包含输入区域坐标的列表
   */
  virtual Base::txSize SceneMaxVehicleSize() const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  std::vector<TAD_RouteGroupAgent> mRouteGroupVec;
  std::vector<VehicleInputeAgent> mVehicleInputeVec;
  std::vector<VehicleExitAgent> mVehicleExitVec;
};

using TAD_ElementGeneratorPtr = std::shared_ptr<TAD_ElementGenerator>;

TX_NAMESPACE_CLOSE(Scene)
