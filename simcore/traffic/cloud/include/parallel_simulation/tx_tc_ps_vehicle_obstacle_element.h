// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_tc_tad_vehicle_ai_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

struct obs_vehicle_info {
  Base::txSysId vehId;
  Base::txFloat startTimeStamp = 0.0;
  Base::txFloat endTimeStamp = 0.0;
  Base::txLaneUId laneUid;
  Base::txVec3 vLaneDir;
  Base::txFloat s = 0.0;
  Coord::txWGS84 vPos;
  Base::txFloat length = 1.0;
  Base::txFloat width = 1.0;
  Base::txFloat height = 2.0;
  static Base::txFloat SampleStep() TX_NOEXCEPT { return 1.5; }
};

class PS_ObsVehicleElement : public TAD_Cloud_AI_VehicleElement {
 public:
  /**
   * @brief 初始化障碍物车辆信息
   * @param obsVehInfo 障碍物车辆信息
   * @param sceneLoaderPtr 场景加载器指针
   * @return 成功返回true，失败返回false
   *
   * 使用传入的障碍物车辆信息和场景加载器初始化障碍物车辆对象。
   */
  virtual Base::txBool Initialize(const obs_vehicle_info& obsVehInfo, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 检查元素是否end
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool
   */
  virtual Base::txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新当前障碍物元素
   * @param timeMgr 当前时间管理器
   * @return 成功返回true，失败返回false
   *
   * 该方法用于更新当前障碍物元素的状态，包括位置、速度等参数。
   */
  virtual Base::txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 根据时间管理器填充交通元素
   * @param timeMgr 当前时间管理器
   * @param traffic 交通信息
   * @return true 填充成功；false 填充失败
   *
   * 该方法根据当前时间管理器填充交通元素，包括交通方向、速度、位置等信息。
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const& timeMgr, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;
};

using PS_ObsVehicleElementPtr = std::shared_ptr<PS_ObsVehicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
