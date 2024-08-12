// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_element_manager_base.h"
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(Utils)

using ElementType = Base::Enums::ElementType;
using ConditionBoundaryType = Base::Enums::ConditionBoundaryType;
using ConditionDistanceType = Base::Enums::ConditionDistanceType;
using ConditionSpeedType = Base::Enums::ConditionSpeedType;
using ConditionEquationOp = Base::Enums::ConditionEquationOp;
using DistanceProjectionType = Base::Enums::DistanceProjectionType;

/**
 * @brief DistType2ProjType 将 ConditionDistanceType 转换为 DistanceProjectionType 类型
 *
 * @param _dist_type   条件下的距离类型
 * @param scene_loader_ptr 地图加载器指针
 * @return 转换后的 DistanceProjectionType 类型
 */
DistanceProjectionType DistType2ProjType(const ConditionDistanceType _dist_type,
                                         Base::ISceneLoaderPtr scene_loader_ptr) TX_NOEXCEPT;
Base::txBool IsValidProj(Base::ITrafficElementPtr src_ptr, Base::ITrafficElementPtr target_ptr,
                         const DistanceProjectionType projType) TX_NOEXCEPT;

/**
 * @brief QueryTrafficElementPtr 根据给定的参数，从元素管理器中获取相应的流量元素指针。
 *
 * @param _type 要查询的流量元素类型。
 * @param _elemId 要查询的流量元素的 ID。
 * @param _elemPtr 指向元素管理器的智能指针。
 * @return 返回一个智能指针，指向查询到的流量元素。
 */
Base::ITrafficElementPtr QueryTrafficElementPtr(const ElementType _type, const Base::txSysId _elemId,
                                                Base::IElementManagerPtr _elemPtr) TX_NOEXCEPT;

/**
 * @brief EquationOperator 根据给定的运算符、左值和右值执行数学运算
 * @param _op 运算符
 * @param left_v 数学表达式左侧的值
 * @param right_v 数学表达式右侧的值
 * @return true 如果运算符合法，则返回true
 * @return false 如果运算符不合法，则返回false
 */
Base::txBool EquationOperator(const ConditionEquationOp _op, const Base::txFloat left_v,
                              const Base::txFloat right_v) TX_NOEXCEPT;

/**
 * @brief BoundaryOperator 根据给定的边界条件、上一个值、当前值和阈值，判断当前值是否满足边界条件
 * @param _boundary 边界条件类型
 * @param opLastValue 上一个值（如果有）
 * @param value 当前值
 * @param _threshold 阈值
 * @return true 当前值满足边界条件
 * @return false 当前值不满足边界条件
 */
Base::txBool BoundaryOperator(const ConditionBoundaryType _boundary,
                              const boost::optional<std::list<Base::txFloat> > opLastValue, const Base::txFloat value,
                              const Base::txFloat _threshold) TX_NOEXCEPT;

/**
 * @brief Compute_EGO_Distance 计算两个交通元素间的距离
 * @param ego_ptr 代表我们感兴趣的交通元素的指针
 * @param element_ptr 我们要计算距离的目标交通元素的指针
 * @param projType 定义预期的距离投影方式
 * @return ego_ptr 到 element_ptr 之间的距离，以米为单位
 * @note 该函数不会抛出任何异常
 */
Base::txFloat Compute_EGO_Distance(Base::ITrafficElementPtr ego_ptr, Base::ITrafficElementPtr element_ptr,
                                   const DistanceProjectionType projType) TX_NOEXCEPT;

/**
 * @brief Compute_TTC_Distance 计算两个交通元素之间的TTC，即它们相距多远以及所需的时间
 *
 * @param ego_ptr 代表我们感兴趣的交通元素的指针
 * @param element_ptr 我们要计算距离的目标交通元素的指针
 * @param projType 定义预期的距离投影方式
 * @return Base::txFloat 返回一个 txFloat 类型的变量，表示 ego_ptr 到 element_ptr 之间的时间-距离消耗
 *
 * @note 该函数不会抛出任何异常
 */
Base::txFloat Compute_TTC_Distance(Base::ITrafficElementPtr ego_ptr, Base::ITrafficElementPtr element_ptr,
                                   const DistanceProjectionType projType) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(Utils)
