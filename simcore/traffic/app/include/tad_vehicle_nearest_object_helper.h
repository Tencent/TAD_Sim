// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_traffic_element_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

struct TAD_VehicleNearestObjectHelper {
 public:
  TAD_VehicleNearestObjectHelper() TX_DEFAULT;
  virtual ~TAD_VehicleNearestObjectHelper() TX_DEFAULT;

  /**
   * @brief 获取当前道路中最前方的车辆及其所在的道路
   *
   * @return 返回一个包含两个元素的 std::tuple:
   *          第一个元素为最前方车辆的远程对象指针，
   *          第二个元素为距离当前车辆的距离。
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> FrontVeh_CurrentLane() const TX_NOEXCEPT = 0;

  /**
   * @brief RearVeh_CurrentLane 获取最近的后车辆信息
   *
   * @return 返回一个包含两个元素的 std::tuple:
   *         - 第一个元素为最近后车辆的智能指针，
   *         - 第二个元素为该后车辆与当前车辆之间的距离。
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> RearVeh_CurrentLane() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最前面的前车辆信息及目标道路
   *
   * @return 返回一个包含两个元素的 std::tuple:
   *         - 第一个元素为最前面的前车辆智能指针，
   *         - 第二个元素为该前车辆所在的目标道路。
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> FrontVeh_TargetLane() const TX_NOEXCEPT = 0;

  /**
   * @brief FrontVeh_TargetLane 获取最前面的前车辆及其所在的目标道路
   *
   * @param dest_move_state 目标道路的状态
   * @return 包含两个元素的 std::tuple: 第一个元素为最前面的前车辆智能指针，第二个元素为该前车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> FrontVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最近的车辆及其所在的目标道路
   *
   * @return 包含两个元素的 std::tuple: 第一个元素为最近的车辆智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> RearVeh_TargetLane() const TX_NOEXCEPT = 0;

  /**
   * @brief RearVeh_TargetLane 获取最近的后方车辆及其所在的目标道路
   * @param dest_move_state 目标道路的状态
   * @return 包含两个元素的 std::tuple: 第一个元素为最近的后方车辆智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> RearVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最近的一侧车辆及其所在的目标道路
   * @return 包含两个元素的 std::tuple: 第一个元素为最近的一侧车辆智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> Any_SideVeh_TargetLane() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最近的对面车辆及其所在的目标道路
   * @return 包含两个元素的 std::tuple: 第一个元素为最近的对面车辆智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> SideVeh_TargetLane() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取指定移动状态的最近车辆及其所在目标道路
   * @param dest_move_state 指定的车辆移动状态
   * @return 包含两个元素的 tuple：第一个元素为最近车辆的智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> SideVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最左侧的车辆和它所在的目标道路
   * @return 包含两个元素的 tuple: 第一个元素为最左侧车辆的智能指针, 第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> LeftVeh_TargetLane() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取最右侧车辆及其所在目标道路
   * @return 包含两个元素的 tuple：第一个元素为最右侧车辆的智能指针，第二个元素为该车辆所在的目标道路
   */
  virtual std::tuple<Base::ITrafficElementPtr, Base::txFloat> RightVeh_TargetLane() const TX_NOEXCEPT = 0;
};

TX_NAMESPACE_CLOSE(TrafficFlow)
