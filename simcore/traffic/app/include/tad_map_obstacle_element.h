// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_obstacle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_MapObstacleElement : public TAD_ObstacleElement {
 public:
  TAD_MapObstacleElement() TX_DEFAULT;
  virtual ~TAD_MapObstacleElement() TX_DEFAULT;

  /**
   * @brief 初始化元素
   *
   * @param obj_ptr 地图物体指针
   * @return txBool 初始化是否成功
   */
  virtual txBool Initialize_txObject(hadmap::txObjectPtr obj_ptr) TX_NOEXCEPT;

  /**
   * @brief 返回元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Map_Immovability; }

  /**
   * @brief 填充空间查询
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 获取元素的名称
   *
   * @return txString
   */
  virtual txString GetObjectName() const TX_NOEXCEPT { return m_obj_name; }

 protected:
  /**
   * @brief 是否支持的地图物体类型
   *
   * @param strTypeName 物体类型名称
   * @return txBool
   */
  virtual txBool IsSupportMapObjectType(const Base::txString strTypeName) const TX_NOEXCEPT;

 protected:
  hadmap::txObjectPtr m_raw_tx_object_ptr = nullptr;
  Base::txString m_obj_name;
};

using TAD_MapObstacleElementPtr = std::shared_ptr<TAD_MapObstacleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
