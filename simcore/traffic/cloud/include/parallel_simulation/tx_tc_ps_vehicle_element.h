// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_serialization.h"
#include "tx_tc_gflags.h"
#include "tx_tc_tad_vehicle_ai_element.h"

#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/

TX_NAMESPACE_OPEN(TrafficFlow)

class PS_VehicleElement : public TAD_Cloud_AI_VehicleElement {
 public:
  /**
   * @brief 添加车辆路径
   * @param startPoint 起始点坐标
   * @param roadIds 道路ID列表
   * @return true 成功添加; false 添加失败
   *
   * 添加车辆从起始点出发，遵循roadIds指定的道路序列，构建车辆路径。
   * 若成功添加，返回true；若添加失败，返回false。
   */
  virtual Base::txBool AddVehiclePath(hadmap::txPoint startPoint, const std::vector<int64_t>& roadIds) TX_NOEXCEPT;

  /**
   * @brief 处理模拟事件
   * @param v 传入模拟事件参数
   * @return true 成功处理事件; false 处理事件失败
   *
   * 在事件处理器中处理传入的模拟事件v，并返回处理结果。
   */
  virtual Base::txBool HandleSimulationEvent(const Base::txFloat v) TX_NOEXCEPT;

  /**
   * @brief 检查deadline
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE { return true; }

 public:
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("PS_VehicleElement");
    archive(cereal::base_class<TrafficFlow::TAD_Cloud_AI_VehicleElement>(this));
  }
};

using PS_VehicleElementPtr = std::shared_ptr<PS_VehicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef SerializeInfo

CEREAL_REGISTER_TYPE(TrafficFlow::PS_VehicleElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::TAD_Cloud_AI_VehicleElement, TrafficFlow::PS_VehicleElement);
