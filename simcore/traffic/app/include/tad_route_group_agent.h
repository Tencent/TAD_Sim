// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/hashed_lane_info_orthogonal_list.h"
#include "HdMap/tx_hashed_road_area.h"
#include "tad_component.h"
#include "tx_element_generator.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Scene)

class TAD_RouteGroupAgent {
 public:
  using LocationAgentPtr = Base::LocationAgent::LocationAgentPtr;
  using RouteAI = TrafficFlow::Component::RouteAI;
  using RouteGroupPtr = Base::ISceneLoader::ITrafficFlowViewer::RouteGroupPtr;
  using txHashedRoadArea = Geometry::SpatialQuery::txHashedRoadArea;
  using HashedLaneInfoOrthogonalList = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList;
  using VehicleContainer = HashedLaneInfoOrthogonalList::VehicleContainer;

 public:
  TAD_RouteGroupAgent() TX_DEFAULT;
  ~TAD_RouteGroupAgent() TX_DEFAULT;

  /**
   * @brief 路由组组件初始化
   *
   * @param xsdPtr 路由组指针
   * @return Base::txBool
   */
  Base::txBool Init(RouteGroupPtr xsdPtr) TX_NOEXCEPT;

  /**
   * @brief 当前agent是否有效
   *
   * @return Base::txBool
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 获取路由组ID，返回一个整数
   *
   * @return Base::txInt 数类型的RouteGroupId
   */
  Base::txInt RouteGroupId() const TX_NOEXCEPT;

  /**
   * @brief 重置路由
   */
  void ResetRoute() TX_NOEXCEPT;

  /**
   * @brief 获取下一个概率路由
   *
   * @param subRouteId 用于存储子路由ID的引用
   * @return std::tuple< LocationAgentPtr, LocationAgentPtr, RouteAI > 一个包含LocationAgentPtr,
   * LocationAgentPtr和RouteAI类型的元组
   */
  std::tuple<LocationAgentPtr, LocationAgentPtr, RouteAI> GetNextProbabilityRoute(Base::txInt& subRouteId) TX_NOEXCEPT;

  /**
   * @brief 获取特殊路由
   *
   * @param subRouteId 用于存储子路由ID的常量引用
   * @return std::tuple< LocationAgentPtr, LocationAgentPtr, RouteAI > 一个包含LocationAgentPtr,
   * LocationAgentPtr和RouteAI类型的元组
   */
  std::tuple<LocationAgentPtr, LocationAgentPtr, RouteAI> GetSpecialRoute(const Base::txInt subRouteId) TX_NOEXCEPT;

  /**
   * @brief 获取当前agent的字符串表示
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief 查询所有vehicles
   *
   * @param refVehicleContainer vehicle容器
   * @return Base::txBool
   */
  virtual Base::txBool QueryVehicles(VehicleContainer& refVehicleContainer) TX_NOEXCEPT;

 protected:
  RouteGroupPtr mRawXSDRouteGroupPtr = nullptr;
  std::vector<Base::txInt> mProbability;
  Base::txInt mIndex = 0;
  Base::txBool mValid = false;
  std::vector<std::tuple<LocationAgentPtr, LocationAgentPtr, RouteAI> > mCandidateVector;
  txHashedRoadArea mStartHashArea;
};

TX_NAMESPACE_CLOSE(Scene)
