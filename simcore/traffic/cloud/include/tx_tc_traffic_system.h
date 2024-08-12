// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <set>
#include "its_sim.pb.h"
#include "regionBasic.pb.h"
#include "road_realtimeinfo.pb.h"
#include "tx_assembler_context.h"
#include "tx_map_info.h"
#include "tx_sim_point.h"
#include "tx_traffic_element_system.h"
#include "tx_tc_assembler_context.h"
#include "tx_tc_cloud_element_manager.h"
#include "tx_tc_inject_event_system.h"
#include "tx_tc_marco.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class CloudTrafficElementSystem : public Base::TrafficElementSystem, public InjectEventHandler {
  using ParentClass = Base::TrafficElementSystem;

 public:
  CloudTrafficElementSystem();
  virtual ~CloudTrafficElementSystem() TX_DEFAULT;

  /**
   * @brief traffic元素系统初始化
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 更新规划车辆的数据
   * @param timeMgr 时间管理器，包括当前时间、速度等信息
   * @param _egoSubType 车辆子类型，具体信息参考Base::Enums::EgoSubType
   * @param egoInfoStr 车辆的详细信息，字符串格式
   * @return 返回值为true表示更新成功，为false表示更新失败
   */
  virtual Base::txBool UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                             const Base::Enums::EgoSubType _egoSubType,
                                             const Base::txString& egoInfoStr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 注册规划车辆
   * @return 返回值为true表示注册成功，为false表示注册失败
   */
  virtual Base::txBool RegisterPlanningCar() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化交通系统
   * @param scene_loader 场景加载器的指针
   * @param valid_map_range 一个包含有效地图范围的映射，键是地图名称，值是一个包含最小和最大坐标（包含）的元组
   * @return 返回初始化是否成功，true表示成功，false表示失败
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr, const Base::map_range_t valid_map_range) TX_NOEXCEPT;

  /**
   * @brief 更新车流系统的状态
   * @param timeMgr 时间管理器，包括当前时间、速度等信息
   * @return 返回值为 true 表示更新成功，为 false 表示更新失败
   */
  virtual Base::txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放资源
   * @return 返回值为 true 表示释放成功，为 false 表示释放失败
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE { return ParentClass::Release(); }

  /**
   * @brief 判断是否支持指定的场景类型
   * @param _sceneType 指定的场景类型
   * @return 返回值为 true 表示支持，为 false 表示不支持
   */
  virtual Base::txBool IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const TX_NOEXCEPT
      TX_OVERRIDE;

  /**
   * @brief 清空场景事件队列
   * @param timeMgr 时间管理器，包含当前时间和速度等信息
   */
  virtual void FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief ExecuteEnvPerception 虚拟执行环境感知操作
   * @param timeMgr 时间管理器，包含当前时间和速度等信息
   * @return 无返回值
   * @note TX_NOEXCEPT TX_OVERRIDE 表示异常处理和重载的声明
   */
  virtual void ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief RegisterEgoOnCloud 向云端注册参与者
   * @param ego_id 参与者的唯一ID
   * @param pos 参与者当前的位置
   * @param bForce 是否强制更新，即使位置没有发生变化
   * @return 是否成功注册
   */
  virtual Base::txBool RegisterEgoOnCloud(const Base::txSysId ego_id, ::sim_msg::Location pos,
                                          const Base::txBool bForce = false) TX_NOEXCEPT;

  /**
   * @brief 移除指定ID的参与者
   * @param ego_id 要移除的参与者的唯一ID
   * @return 是否成功移除
   *
   * 该函数移除指定ID的参与者，如果参与者不存在或已被移除，则返回false。
   */
  virtual Base::txBool EraseEgoById(const Base::txSysId ego_id) TX_NOEXCEPT;

  /**
   * @brief 更新指定ID的参与者位置信息
   * @param ego_id 参与者的唯一ID
   * @param info 新的位置信息
   * @return 是否成功更新
   *
   * 该函数用于更新指定ID的参与者的位置信息。如果指定ID的参与者不存在，则返回false。
   */
  virtual Base::txBool UpdateEgoOnCloud(const Base::txSysId ego_id, const sim_msg::Location& info) TX_NOEXCEPT;

  /**
   * @brief 通过半径查询指定ID范围内的交通元素
   * @param egoId 指定ID
   * @param radius 半径
   * @param queryResultElementVec 查询结果
   * @return 是否成功查询
   *
   * 该函数通过半径查询指定ID范围内的交通元素。如果指定ID不存在，则返回false。
   */
  virtual Base::txBool QueryTrafficDataByCircle(const Base::txSysId egoId, const Base::txFloat radius,
                                                std::vector<Base::ITrafficElementPtr>& queryResultElementVec)
      TX_NOEXCEPT;

  /**
   * @brief 向交通系统中发送特殊类型的交通数据
   * @param timeMgr 时间管理器参数
   * @param vec_send_element_ptr 待发送的交通元素向量
   * @param outTraffic 发送结果的交通数据对象
   * @return true 成功发送特殊类型的交通数据，false 发送失败
   *
   * 该函数向交通系统中发送特殊类型的交通数据，并更新发送结果的交通数据对象。
   * 当发送失败时，函数返回false，否则返回true。
   */
  virtual Base::txBool SendSpecialTrafficData(Base::TimeParamManager const& timeMgr,
                                              std::vector<Base::ITrafficElementPtr>& vec_send_element_ptr,
                                              sim_msg::Traffic& outTraffic) TX_NOEXCEPT;

  /**
   * @brief 重新设置输入代理的有效范围
   * @param vec_valid_sim_range 输入代理的有效范围
   * @return true 重新设置成功，false 重新设置失败
   *
   * 这个函数用于重新设置输入代理的有效范围，使其在新范围内生效。
   * 如果重新设置成功，函数返回true，否则返回false。
   */
  virtual Base::txBool ReSetInputAgent(const std::vector<Base::map_range_t>& vec_valid_sim_range) TX_NOEXCEPT;

  /**
   * @brief 填充模拟状态
   * @param timeStamp 当前时间戳
   * @param outSceneStatue 输出模拟状态
   * @return 填充成功返回 true，否则返回 false
   *
   * 该函数用于填充模拟状态。如果填充成功，函数返回 true，否则返回 false。
   */
  virtual Base::txBool FillingSimStatus(Base::TimeParamManager const& timeStamp,
                                        its::txSimStatus& outSceneStatue) TX_NOEXCEPT;

  /**
   * @brief 获取云元素管理器
   * @return 返回云元素管理器的智能指针
   *
   * 该函数用于获取云元素管理器的智能指针。
   */
  virtual CloudElementManagerPtr CloudElemMgr() TX_NOEXCEPT { return m_Cloud_ElementMgr; }

  /**
   * @brief 获取云元素整合器上下文
   * @return 返回云元素整合器上下文的智能指针
   *
   * 该函数用于获取云元素整合器上下文的智能指针，用于管理云元素整合器。
   */
  virtual Scene::TAD_Cloud_AssemblerContextPtr CloudAssemblerCtx() TX_NOEXCEPT { return m_Cloud_AssemblerCtx; }

  /**
   * @brief 填充空间查询
   * @return true 填充成功，false 填充失败
   *
   * 该函数用于填充空间查询，用于管理空间查询。
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 注册自身的状态
   * @return true: 自身已注册, false: 自身未注册
   *
   * 该函数用于在时间复杂度 O(1) 的时间内判断一个系统是否已经注册。
   */
  Base::txBool RegEgo() const TX_NOEXCEPT { return m_register_ego; }

  /**
   * @brief 注册自身的状态
   * @return true: 已注册, false: 未注册
   *
   * 该函数用于在时间复杂度 O(1) 的时间内判断一个系统是否已经注册。
   */
  Base::txBool& RegEgo() TX_NOEXCEPT { return m_register_ego; }
#if __cloud_ditw__

  /**
   * @brief 更新DITW数据
   * @param timeMgr    时间管理器
   * @param _input_frame 输入帧数据
   * @return true: 更新成功, false: 更新失败
   *
   * 该函数用于根据输入帧数据更新DITW数据，返回更新结果。
   */
  virtual Base::txBool UpdateDITWData(Base::TimeParamManager const& timeMgr,
                                      const PerceptionMessageData& _input_frame) TX_NOEXCEPT;

  /**
   * @brief 更新DITW数据的后置操作
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 操作成功返回true
   */
  virtual Base::txBool PostOpDITWData(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 检查Log2World
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool
   */
  virtual Base::txBool CheckL2W(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取vehicle输入的启动状态
   *
   * @return Base::txBool 返回启用状态
   */
  virtual Base::txBool EnableVehicleInput() const TX_NOEXCEPT { return m_enable_vehicle_input; }

  /**
   * @brief 开启/关闭车辆输入功能
   * @param _f 开启 true，关闭 false
   * @return 无
   * @note 修改车辆输入功能的状态，一般会在车辆的启动与关闭过程中使用
   */
  virtual void EnableVehicleInput(const Base::txBool _f) TX_NOEXCEPT { m_enable_vehicle_input = _f; }

  /**
   * @brief 重置预测模拟场景
   *
   * 重置预测模拟场景的接口
   * @param status_pb_record 模拟场景启用/禁用状态
   * @param traffic_pb_record 模拟场景流量记录
   * @param _event_json 模拟场景事件JSON字符串
   * @return 重置成功返回 true，否则返回 false
   * @note 该函数用于重置预测模拟场景的接口
   */
  virtual Base::txBool ResetForecastSimScene(const its::txStatusRecord& status_pb_record /* size = 1 */,
                                             const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                             const std::string _event_json) TX_NOEXCEPT;

  /**
   * @brief 调用Fog()方法
   * @return 返回函数结果，如果成功返回true，否则返回false
   * @note 此函数用于调用Fog()方法
   */
  virtual Base::txBool Fog() TX_NOEXCEPT;

  /**
   * @brief 碰撞检测函数
   *
   * @return 如果发生碰撞返回true, 否则返回false
   *
   * @note 此函数用于检测当前车辆状态是否发生碰撞，如果发生碰撞则返回true，否则返回false。
   */
  virtual Base::txBool Crash() TX_NOEXCEPT;
#endif /*__cloud_ditw__*/

 protected:
  /**
   * @brief 更新信号
   *
   * 根据时间参数管理器，更新信号。
   *
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 如果更新成功则返回 true，否则返回 false
   */
  virtual Base::txBool Update_Signal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 检查实例生命周期
   *
   * 检查实例是否已经超出其生命周期范围，如果超出则设置为不可用。
   *
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 如果检查成功则返回 true，否则返回 false
   */
  virtual Base::txBool Update_CheckLifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新车辆模拟系统
   *
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 更新成功返回 true，否则返回 false
   *
   * @note 此函数用于更新当前系统中的车辆模拟系统，包括车辆位置、速度、状态等，返回更新成功与否。
   */
  virtual Base::txBool Update_Vehicle_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief vehicle仿真更新后置操作
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 操作成功返回true
   */
  virtual Base::txBool Update_Vehicle_PostSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新行人信息
   *
   * @param timeMgr 当前时间参数管理器
   * @return Base::txBool 更新成功返回true，否则返回false
   *
   * @note 此函数用于更新当前系统中的行人信息，包括位置、速度、状态等，返回更新成功与否。
   */
  virtual Base::txBool Update_Pedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新障碍物
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool Update_Obstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 创建装配器对象
   *
   */
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 创建元素管理器
   *
   */
  virtual void CreateElemMgr() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Scene::TAD_Cloud_AssemblerContextPtr m_Cloud_AssemblerCtx = nullptr;
  CloudElementManagerPtr m_Cloud_ElementMgr = nullptr;
#if __cloud_ditw__
  Coord::txWGS84 m_l2w_point;
  Base::Info_Lane_t m_l2w_laneInfo;
  Base::txFloat m_l2w_s;
  Base::txBool m_enable_vehicle_input = true;
  enum class event_type : int { eNone, eFog, eCrash };
  event_type m_event_type = event_type::eNone;
  Base::txFloat m_fog_max_speed;
  std::set<Base::txRoadID> m_fog_roads_set;

  Coord::txWGS84 m_crash_point;
  Base::Info_Lane_t m_crash_laneInfo;

  Base::txFloat m_crash_s;

  std::list<Geometry::SpatialQuery::HashedLaneInfo> m_crash_speed_limit_start_hashedNodeList;
  std::list<Geometry::SpatialQuery::HashedLaneInfo> m_crash_speed_limit_end_hashedNodeList;
  Base::txFloat m_crash_max_speed;
#endif /*__cloud_ditw__*/
  Base::txBool m_register_ego = false;
};

using CloudTrafficElementSystemPtr = std::shared_ptr<CloudTrafficElementSystem>;

TX_NAMESPACE_CLOSE(TrafficFlow)
