// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hashed_road_area.h"
#include "tad_component.h"
#include "tad_route_group_agent.h"
#include "tx_dead_line_area_manager.h"
#include "tx_element_generator.h"
#include "tx_element_manager_base.h"
#include "tx_map_info.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#include "tx_tc_cloud_loader.h"
#include "tx_tc_marco.h"
TX_NAMESPACE_OPEN(Scene)

class TAD_Cloud_ElementGenerator : public Base::ISceneElementGenerator {
 public:
  using LocationAgent = Base::LocationAgent;
  using VehicleInputeAgent = Base::VehicleInputeAgent;
  using VehicleExitAgent = Geometry::SpatialQuery::txHashedRoadArea;
  using TAD_RouteGroupAgent = Scene::TAD_RouteGroupAgent;
  using ParentClass = Base::ISceneElementGenerator;

 public:
  virtual ~TAD_Cloud_ElementGenerator() { Release(); }

  /**
   * @brief 初始化类实例
   *
   * 使用场景加载器初始化类实例，并设置有效的地图范围。
   *
   * @param scene_loader 指向场景加载器的智能指针
   * @param valid_map_range 有效地图范围
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr,
                                  const Base::map_range_t& valid_map_range) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 使用指定的时间参数管理器和元素管理器生成的场景元素
   *
   * 这个函数会根据给定的时间参数管理器和元素管理器生成随机的场景元素，并将它们添加到元素管理器中。
   *
   * @param time_param_manager 时间参数管理器的引用
   * @param element_manager 元素管理器的智能指针
   * @return 如果成功生成元素，则返回 true，否则返回 false
   */
  virtual Base::txBool Generate(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 移除指定时间参数管理器和元素管理器中的所有场景元素
   *
   * 此函数会移除指定时间参数管理器和元素管理器中的所有场景元素，并返回执行结果。
   *
   * @param time_param_manager 时间参数管理器的引用
   * @param element_manager 元素管理器的智能指针
   * @return 成功删除元素时返回 true，否则返回 false
   */
  virtual Base::txBool Erase(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 使用指定的时间参数管理器和元素管理器重新路由场景元素
   *
   * 此函数使用指定的时间参数管理器和元素管理器重新路由场景元素，并返回执行结果。
   *
   * @param time_param_manager 时间参数管理器的引用
   * @param element_manager 元素管理器的智能指针
   * @return 成功重新路由元素时返回 true，否则返回 false
   */
  virtual Base::txBool ReRoute(Base::TimeParamManager const&, Base::IElementManagerPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放当前的元素生成器
   *
   * 该函数释放当前的元素生成器对象，并将所有关联的资源释放。
   *
   * @return 返回 true，表示成功释放；返回 false，表示释放失败。
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重置元素生成器
   *
   * 该函数重置元素生成器，将其状态恢复到初始化状态，并清除已生成的元素。
   *
   * @return 返回 true，表示成功重置；返回 false，表示重置失败。
   */
  virtual Base::txBool ResetGenerator() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查输入点是否安全
   *
   * 函数检查输入点是否在安全范围内。如果输入点在最近点附近的安全范围内，则认为是安全的。
   *
   * @param inputPt 输入点
   * @param nearestPt 最近点
   * @return true 如果输入点安全，即在最近点附近的安全范围内。
   * @return false 如果输入点不安全，即不在最近点附近的安全范围内。
   */
  virtual Base::txBool SafeInputRegion(const Base::txVec2& inputPt,
                                       const Base::txVec2& nearestPt) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 重新设置输入代理
   *
   * 重新设置输入代理，允许在满足给定范围的情况下重置输入代理。
   *
   * @param vec_valid_sim_range 重置输入代理的范围。
   * @return 返回 true 表示重置成功；返回 false 表示重置失败。
   */
  virtual Base::txBool ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range) TX_NOEXCEPT;

  /**
   * @brief 获取输入区域的坐标列表
   *
   * 该函数返回一个包含输入区域坐标的向量，其中每个坐标都是一个 WGS84 坐标。
   *
   * @return 返回一个包含输入区域坐标的向量
   */
  virtual std::vector<Coord::txWGS84> InputRegionLocations() const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 获取场景中的最大车辆数量
   *
   * 该函数返回场景中的最大车辆数量。
   *
   * @return 返回场景中的最大车辆数量
   */
  virtual Base::txSize SceneMaxVehicleSize() const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  std::vector<TAD_RouteGroupAgent> mRouteGroupVec;
  std::vector<VehicleInputeAgent> mVehicleInputeVec;
  std::vector<VehicleExitAgent> mVehicleExitVec;
};

using TAD_Cloud_ElementGeneratorPtr = std::shared_ptr<TAD_Cloud_ElementGenerator>;

TX_NAMESPACE_CLOSE(Scene)
