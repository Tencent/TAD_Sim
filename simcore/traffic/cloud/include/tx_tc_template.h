// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "location.pb.h"
#include "parallel_simulation/tx_tc_ps_traffic_record.h"
#include "parallel_simulation/tx_tc_ps_traffic_system.h"
#include "tx_tc_init_params.h"
#include "tx_tc_map_range_utils.h"
#include "tx_tc_traffic_system.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class PluginTemplate {
 public:
  PluginTemplate() TX_DEFAULT;
  virtual ~PluginTemplate() TX_DEFAULT;

 public:
  /**
   * @brief 创建环境
   *
   * 根据输入的是否有自主车辆的信息来创建环境。如果存在自主车辆，则该函数会根据自主车辆的位置创建环境，否则该函数会创建一个简单的环境。
   *
   * @param res_ego 是否存在自主车辆
   * @param loc 用于保存自主车辆位置信息的地点结构体
   * @return 无
   */
  virtual void CreateEnv(const Base::txBool res_ego, sim_msg::Location& loc) TX_NOEXCEPT;

  /**
   * @brief 获取云端元素系统指针
   *
   * @return TrafficFlow::CloudTrafficElementSystemPtr 返回云端元素系统指针
   */
  virtual TrafficFlow::CloudTrafficElementSystemPtr CloudTrafficSystemPtr() TX_NOEXCEPT {
    return m_TrafficElementSystemPtr;
  }

  /**
   * @brief 获取并行模拟流量系统指针
   * @return 返回并行模拟流量系统指针
   */
  virtual TrafficFlow::ParallelSimulation_TrafficElementSystemPtr ParallelSimulationSystemPtr() TX_NOEXCEPT {
    return m_ps_TrafficElementSystemPtr;
  }

  /**
   * @brief 实现仿真运行函数
   *
   * 这个函数可以用来实现模拟相关时间运行的功能。它根据传递给它的参数（如时间管理器对象）决定模拟流量系统的指针的行为。
   *
   * @param timeMgr 时间管理器对象，包含了当前时间、任务及其优先级等信息
   */
  virtual void Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 重置HD图缓存
   *
   * 使用此功能重置HD图缓存。当参数为true时，系统将清除缓存并重新加载该缓存。
   *
   * @param _f 表示重置HD图缓存的标志，如果为true则重置，如果为false则不重置
   * @return 返回重置后的标志，如果为true则表示成功重置，否则表示未成功重置
   */
  virtual Base::txBool ResetHdMapCache(const Base::txBool _f) TX_NOEXCEPT {
    resetHdMapCache = _f;
    return resetHdMapCache;
  }

  /**
   * @brief 判断是否重置HD图缓存
   *
   * 此函数用于判断是否需要重置HD图缓存。
   *
   * @return 如果需要重置，返回true；否则返回false。
   */
  virtual Base::txBool IsResetHdMapCache() const TX_NOEXCEPT { return resetHdMapCache; }

  /**
   * @brief 获取输入参数包装器
   *
   * 这个函数用于获取输入参数包装器。
   *
   * @return 返回输入参数包装器
   */
  Scene::InitInfoWrap& InputeParams() TX_NOEXCEPT { return inputeParams; }

  /**
   * @brief 获取输入参数包装器
   *
   * 该函数用于获取输入参数包装器。
   *
   * @return 返回输入参数包装器
   */
  const Scene::InitInfoWrap& InputeParams() const TX_NOEXCEPT { return inputeParams; }

  /**
   * @brief 使用相对时间（秒）创建一个时间参数管理器
   *
   * 该函数接受一个相对时间（秒）作为参数，并使用它来创建一个时间参数管理器实例。
   *
   * @param relativeTimeInSec 相对时间（秒）
   * @return 返回一个新的时间参数管理器实例
   */
  virtual Base::TimeParamManager MakeTimeMgrRelativeTimeInSecond(const Base::txFloat relativeTimeInSec) TX_NOEXCEPT;

  /**
   * @brief 创建一个绝对时间（毫秒）的时间参数管理器
   *
   * 该函数根据给定的绝对时间（毫秒）创建一个时间参数管理器实例。
   *
   * @param abs_time_ms 绝对时间（毫秒）
   * @return 返回一个新的时间参数管理器实例
   */
  virtual Base::TimeParamManager MakeTimeMgrAbsTimeInMillisecondSecond(const uint64_t abs_time_ms) TX_NOEXCEPT;

  /**
   * @brief 获取时间参数管理器
   *
   * 这个方法用于获取一个基于绝对时间的时间参数管理器实例。
   *
   * @return 返回一个新的时间参数管理器实例
   */
  virtual Base::TimeParamManager GetTimeMgr() const TX_NOEXCEPT { return m_cloud_time_mgr; }

  /**
   * @brief traffic仿真执行函数
   *
   * @param timeMgr 时间参数管理器
   */
  virtual void SimulationTraffic(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 初始化L2W模拟
   *
   * 这个函数用于初始化L2W模拟，传入时间参数管理器对象，进行输入输出相关的处理逻辑。
   *
   * @param traffic_pb_record 交通记录，包含模拟中的交通信息
   * @param status_pb_record 状态记录，包含模拟中的状态信息
   * @param _event_json 事件的JSON格式数据
   * @return 返回 true 表示初始化成功，返回 false 表示初始化失败
   * @exception 无
   */
  virtual Base::txBool initialize_l2w_sim(const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                          const its::txStatusRecord& status_pb_record /* size = 1 */,
                                          const std::string _event_json) TX_NOEXCEPT;

  /**
   * @brief 注入traffic事件
   *
   * 该函数负责向模拟环境中注入一个traffic事件，返回值为 true 表示注入成功，返回值为 false 表示注入失败。
   *
   * @param _event_json 一个包含需要注入的traffic事件信息的 JSON 字符串
   * @return 返回 true 表示注入成功，返回 false 表示注入失败
   * @exception 无
   */
  virtual Base::txBool inject_traffic_event(const std::string _event_json) TX_NOEXCEPT;

  /**
   * @brief 重置插件的变量
   *
   * 重置变量以便于重新使用插件。如果没有特定的重置操作，将此函数视为不做任何操作。
   *
   * @exception 无
   */
  virtual void ResetVarsPlugin() TX_NOEXCEPT;

 protected:
  /**
   * @brief 创建场景加载器
   *
   * 创建一个新的场景加载器实例，用于加载和管理场景资源。
   *
   * @return 返回一个智能指针，指向新创建的场景加载器实例
   * @exception 无
   */
  virtual Base::ISceneLoaderPtr CreateSceneLoader() const TX_NOEXCEPT;

  /**
   * @brief 创建系统插件
   *
   * 创建一个新的系统插件实例，用于实现和管理系统级别的功能。
   *
   * @return 无
   * @exception 无
   */
  virtual void CreateSystemPlugin() TX_NOEXCEPT;

 protected:
  /**
   * @brief 检查是否进行并行模拟
   * @return 如果进行并行模拟，返回true，否则返回false
   */
  Base::txBool isParallelSimulation() const TX_NOEXCEPT { return inputeParams.isParallelSimulation(); }

  /**
   * @brief 判断是否为虚拟城市
   * @return 如果是虚拟城市，则返回true；否则返回false
   */
  Base::txBool isVirtualCity() const TX_NOEXCEPT { return inputeParams.isVirtualCity(); }
#if __pingsn__

 protected:
  /**
   * @brief 从原始的pb结构中加载车辆信息并启动车辆
   *
   * 使用这个函数可以从指定的pb中读取车辆信息，并根据该信息初始化车辆。
   *
   * @param startStatus 模拟的初始状态
   * @return 如果成功加载车辆信息并启动车辆，则返回true；否则返回false
   */
  Base::txBool loadVehicles(const its::txSimStatus& startStatus) TX_NOEXCEPT;

 public:
  /**
   * @brief 部分清除traffic记录
   *
   * 该函数用于清除部分traffic记录。
   */
  void TrafficRecord_PartialClear() TX_NOEXCEPT { trafficRecord_.PartialClear(); }

  /**
   * @brief 记录输入车辆信息
   * @param[in] frame_id 当前帧编号
   * @param[in] info 车辆信息
   */
  void TrafficRecord_RecordInputVehicles(const int64_t frame_id, its::txVehicles& info) TX_NOEXCEPT {
    trafficRecord_.RecordInputVehicles(info);
  }

  /**
   * @brief 获取当前traffic状态
   *
   * 获取当前帧（frame_id）对应的traffic状态（sceneSnapshot）。
   *
   * @param[in] frame_id 当前帧编号
   * @param[out] sceneSnapshot 当前帧的流量状态
   */
  void TrafficRecord_GetCurTrafficState(const int64_t frame_id, its::txSimStatus& sceneSnapshot) TX_NOEXCEPT;

  /**
   * @brief 获取指定帧的交通信息
   * @param frame_id 待查询的帧编号
   * @param traffic 存储查询结果的交通信息对象
   * @return 查询是否成功
   */
  Base::txBool GetMapTrafficInfo(const int64_t frame_id, sim_msg::Traffic& traffic) TX_NOEXCEPT;

  /**
   * @brief 记录输入的交通车辆信息
   * @param vehicleMsg 交通车辆信息结构体
   */
  void TrafficRecord_RecordInputVehicle(const its::txVehicle& vehicleMsg) TX_NOEXCEPT;

  /**
   * @brief 将车辆添加到元素管理器
   * @param vehId 车辆ID
   * @param vehType 车辆类型
   * @param vehicleStartLaneId 车辆开始的泊车道ID
   * @param roadIds 车道ID列表
   * @param startV 车辆开始的速度
   * @param roadStartPointX 车道起始点X坐标
   * @param roadStartPointY 车道起始点Y坐标
   * @param roadStartPointZ 车道起始点Z坐标
   * @return 操作成功返回true，否则返回false
   */
  Base::txBool addVehicleToElementManager(Base::txSysId vehId, Base::txInt vehType, Base::txLaneID vehicleStartLaneId,
                                          const std::vector<int64_t>& roadIds, Base::txFloat startV,
                                          double roadStartPointX, double roadStartPointY,
                                          double roadStartPointZ) TX_NOEXCEPT;

  /**
   * @brief 获取仿真状态信息
   * @return txSimStat 仿真状态信息结构体
   */
  its::txSimStat GetSimStatMsg() const TX_NOEXCEPT;
#endif /*__pingsn__*/

 protected:
  TrafficFlow::CloudTrafficElementSystemPtr m_TrafficElementSystemPtr = nullptr;
  TrafficFlow::ParallelSimulation_TrafficElementSystemPtr m_ps_TrafficElementSystemPtr = nullptr;
  Base::txFloat m_lastStepTime = 0.0;
  Base::txFloat m_passTime = 0.0;
  Base::txSize m_step_count_ = 0;
  Base::txSize m_max_step_count_ = std::numeric_limits<Base::txSize>::max();
  Base::txString m_payload_;
  Base::txString m_payload_trailer_;
  sim_msg::Traffic m_outputTraffic;
  Base::txBool resetHdMapCache = true;
  Scene::InitInfoWrap inputeParams;
  sim_msg::VehicleGeometoryList vehGeomList;
  TrafficFlow::TrafficRecord trafficRecord_;
  Base::TimeParamManager m_cloud_time_mgr;
#if __pingsn__
  sim_msg::Traffic trafficMsg_;
  uint64_t cur_AbsTime = 0; /*unix timestamp*/
#endif                      /*__pingsn__*/
};

using PluginTemplatePtr = std::shared_ptr<PluginTemplate>;

TX_NAMESPACE_CLOSE(TrafficFlow)
