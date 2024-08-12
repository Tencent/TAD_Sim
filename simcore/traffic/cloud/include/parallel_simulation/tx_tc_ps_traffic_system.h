// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "parallel_simulation/tx_tc_ps_assembler_context.h"
#include "parallel_simulation/tx_tc_ps_vehicle_element.h"
#include "tx_map_info.h"
#include "tx_tc_traffic_system.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class ParallelSimulation_TrafficElementSystem : public CloudTrafficElementSystem {
 public:
  /**
   * @brief Initialize 初始化函数
   *
   * 使用提供的加载器和有效地图范围对象来初始化交通系统，并根据给定参数设置。
   *
   * @param loader 地图数据加载器指针
   * @param valid_map_range 有效地图范围
   * @return true 初始化成功
   * @return false 初始化失败
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr loader,
                                  const Base::map_range_t valid_map_range) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief DynamicGenerateScene 动态生成场景
   *
   * 使用提供的地图数据加载器对象，对场景进行动态生成，并将信息存储在指定的车辆信息对象中。
   *
   * @param info 车辆信息对象的引用
   * @param _loader 地图数据加载器对象的指针
   * @return true 生成成功
   * @return false 生成失败
   */
  virtual Base::txBool DynamicGenerateScene(its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT {
    return m_ps_AssemblerCtx->DynamicGenerateScene(info, _loader);
  }

  /**
   * @brief 根据指定的路径创建一个交通信号灯元素
   *
   * 根据提供的路径，在交通信号灯管理器中创建一个新的交通信号灯元素，并读取配置文件中的信息。
   *
   * @param traffic_config_path 交通信号灯配置文件路径
   * @return true 创建成功
   * @return false 创建失败
   */
  virtual Base::txBool CreateTrafficLightElement(const Base::txString traffic_config_path) TX_NOEXCEPT {
    return m_ps_AssemblerCtx->CreateTrafficLightElement(ElemMgr(), traffic_config_path);
  }

 public:
  struct SimulationEvent {
   public:
    /**
     * @brief 判断当前实例是否有效
     *
     * @return true 当前实例有效
     * @return false 当前实例无效
     */
    Base::txBool IsValid() const TX_NOEXCEPT { return mIsValid; }

    /**
     * @brief 设置事件
     *
     * 根据给定的事件数据，设置事件。
     *
     * @param ev 待设置的事件数据
     * @return true 事件设置成功
     * @return false 事件设置失败
     */
    Base::txBool SetEvent(const its::txEvent& ev) TX_NOEXCEPT {
      mEvent = ev;
      for (const auto& al : mEvent.affected_objs().affected_lane()) {
        laneUid2txAffectedLane[Base::txLaneUId(al.road_id(), al.seg_id(), al.lane_id())] = al;
      }
      return true;
    }

    /**
     * @brief 更新模拟事件
     *
     * 根据给定的当前时间戳和车辆列表，更新模拟事件。
     *
     * @param curTimeStamp 当前时间戳
     * @param vecVehiclePtr 车辆列表
     * @return true 更新成功
     * @return false 更新失败
     */
    Base::txBool UpdateSimulationEvent(const uint64_t curTimeStamp,
                                       std::vector<Base::ITrafficElementPtr> vecVehiclePtr) TX_NOEXCEPT {
      if ((mEvent.starting_time() <= curTimeStamp) && (curTimeStamp < (mEvent.starting_time() + mEvent.duration()))) {
        LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent)
            << "[True]" << TX_VARS(mEvent.starting_time()) << TX_VARS(curTimeStamp)
            << TX_VARS((mEvent.starting_time() + mEvent.duration()));
        mIsValid = true;
        for (const auto vehiclePtr : vecVehiclePtr) {
          HandlerEvent(vehiclePtr);
        }
      } else {
        LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent)
            << "[False]" << TX_VARS(mEvent.starting_time()) << TX_VARS(curTimeStamp)
            << TX_VARS((mEvent.starting_time() + mEvent.duration()));
        mIsValid = false;
      }
      return IsValid();
    }

   protected:
    /**
     * @brief 处理事件
     *
     * 当事件发生时，响应处理该事件。
     *
     * @param vehiclePtr 车辆指针
     * @return true 事件处理成功
     * @return false 事件处理失败
     */
    Base::txBool HandlerEvent(Base::ITrafficElementPtr vehiclePtr) const TX_NOEXCEPT {
      if (NonNull_Pointer(vehiclePtr)) {
        const Base::Info_Lane_t laneLocInfo = vehiclePtr->GetCurrentLaneInfo();
        if (CallFail(laneLocInfo.isOnLaneLink)) {
          if (_Contain_(laneUid2txAffectedLane, laneLocInfo.onLaneUid)) {
            TrafficFlow::PS_VehicleElementPtr psVehiclePtr =
                std::dynamic_pointer_cast<TrafficFlow::PS_VehicleElement>(vehiclePtr);
            if (NonNull_Pointer(psVehiclePtr)) {
              LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent)
                  << TX_VARS_NAME(VehId, vehiclePtr->Id()) << "set vehicle speed "
                  << (laneUid2txAffectedLane.at(laneLocInfo.onLaneUid).affected_speed());
              return psVehiclePtr->HandleSimulationEvent(
                  laneUid2txAffectedLane.at(laneLocInfo.onLaneUid).affected_speed());
            } else {
              LOG(WARNING) << "vehicle do not support HandleSimulationEvent.";
              return false;
            }
          } else {
            LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent)
                << TX_VARS_NAME(VehId, vehiclePtr->Id()) << "vehicle is not at event region.";
            return false;
          }
        } else {
          LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent)
              << TX_VARS_NAME(VehId, vehiclePtr->Id()) << "vehicle is on lanelink.";
          return false;
        }
      } else {
        LOG_IF(WARNING, FLAGS_LogLevel_SimulationEvent) << "vehiclePtr is nullptr.";
        return false;
      }
    }

   protected:
    Base::txBool mIsValid = false;
    its::txEvent mEvent;
    std::unordered_map<Base::txLaneUId, its::txEvent::txAffectedLane, Utils::LaneUIdHashCompare> laneUid2txAffectedLane;
  };

  /**
   * @brief 注册模拟事件
   *
   * 将要模拟的事件注册到事件系统中，以便在模拟运行期间依据该事件触发相应处理
   *
   * @param evts 要注册的事件列表
   * @return true 注册成功
   * @return false 注册失败
   */
  virtual Base::txBool RegisterSimulationEvent(const its::txEvents& evts) TX_NOEXCEPT;

  /**
   * @brief 更新模拟事件
   *
   * 根据当前时间，更新模拟事件的状态。若事件已经完成，则返回true；否则返回false。
   *
   * @param timeMgr 当前时间信息
   * @return true 模拟事件更新成功
   * @return false 模拟事件更新失败
   */
  virtual Base::txBool UpdateSimulationEvent(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 计算道路车辆信息
   *
   * 计算并返回一个包含所有道路车辆信息的字典。键是道路ID，值是另一个字典，该字典包含车辆ID和对应的当前速度。
   *
   * @param refRoad2VehicleVelocityMap 一个字典，包含道路ID和它们对应的车辆ID和速度信息
   * @return Base::txBool 如果计算成功则返回true，否则返回false
   */
  virtual Base::txBool ComputeRoadVehicleInfo(
      std::map<Base::txRoadID, std::map<Base::txInt /*veh id*/, Base::txFloat /*velocity*/> >&
          refRoad2VehicleVelocityMap) TX_NOEXCEPT;
  std::vector<SimulationEvent> mSimulationEventVec;
  std::vector<Base::ITrafficElementPtr> mTrafficAccidentVec;

 protected:
  /**
   * @brief 创建装配器上下文
   *
   */
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 创建元素管理器
   *
   */
  virtual void CreateElemMgr() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Scene::ParallelSimulation_AssemblerContextPtr m_ps_AssemblerCtx = nullptr;
};

using ParallelSimulation_TrafficElementSystemPtr = std::shared_ptr<ParallelSimulation_TrafficElementSystem>;

TX_NAMESPACE_CLOSE(TrafficFlow)
