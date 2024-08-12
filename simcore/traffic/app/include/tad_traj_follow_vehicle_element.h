// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/optional.hpp>
#include "HdMap/tx_lane_geom_info.h"
#include "tad_behavior_time_manager.h"
#include "tad_component.h"
#include "tad_fsm_user_defined.h"
#include "tad_planning.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_vehicle_kinetics_info.h"
#include "traffic.pb.h"
#include "tx_centripetal_cat_mull.h"
#include "tx_env_perception.h"
#include "tx_scene_loader.h"
#include "tx_trajectory_follower.h"
#include "tx_vehicle_element.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_TrajFollow_VehicleElement : public Base::IVehicleElement,
                                      public TrafficFlow::IKineticsHandler,
                                      public SM::txUserDefinedState,
                                      public TAD_PlanningModule,
                                      public TAD_SceneEvent_1_0_0_0 {
 public:
  using ParentClass = Base::IVehicleElement;
  using controlPoint = HdMap::txLaneInfoInterface::controlPoint;
  using controlPointVec = HdMap::txLaneInfoInterface::controlPointVec;
  using control_path_node_vec = CentripetalCatMull::control_path_node_vec;
  using ControlPathGear = Base::SegmentKinectParam::ControlPathGear;
  using SegmentKinectParam = Base::SegmentKinectParam;
  using SegmentKinectParamList = Base::SegmentKinectParamList;
  using BehaviorTimeManager = TAD_BehaviorTimeManager;

 public:
  TAD_TrajFollow_VehicleElement() TX_DEFAULT;
  virtual ~TAD_TrajFollow_VehicleElement() TX_DEFAULT;

 public:
  /**
   * @brief 初始化函数，接受一个视图对象和一个场景加载器对象，并返回一个布尔值
   *
   * @param 视图对象 视图对象，包含渲染相关信息
   * @param 场景加载器对象 场景加载器对象，包含场景相关信息
   * @return txBool 初始化成功返回 txTrue，否则返回 txFalse
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化行人，接受一个行人对象和一个场景加载器对象，并返回一个布尔值
   *
   * @param 行人对象 行人对象，包含行人相关信息
   * @param 场景加载器对象 场景加载器对象，包含场景相关信息
   * @return txBool 初始化成功返回 txTrue，否则返回 txFalse
   */
  virtual txBool InitializePedestrian(Base::ISceneLoader::IPedestriansViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 初始化动力学
   *
   * @param vehiclesViewerPtr 指向行驶元素的指针
   * @return txBool
   */
  virtual txBool Initialize_Kinetics(Base::ISceneLoader::IVehiclesViewerPtr) TX_NOEXCEPT;

  /**
   * @brief 更新行车轨迹跟踪功能的车辆元素
   *
   * 更新行车轨迹跟踪功能的车辆元素状态。函数获取当前时间参数，并使用该参数更新行车轨迹跟踪功能的车辆元素。
   *
   * @param const TimeParamManager& 当前时间参数
   * @return txBool 更新成功则返回txTrue, 否则返回txFalse
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放车辆元素
   *
   * 释放车辆元素占用的资源，例如关闭文件、停止线程等。此函数通常在车辆元素不再使用时被调用。
   *
   * @return txBool 释放成功则返回txTrue，否则返回txFalse
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 对车辆元素进行填充
   *
   * 这个函数用于对车辆元素进行填充。在填充车辆元素之前，该函数会确保所需的数据和资源已被准备好。
   *
   * @param timeParamManager 当前时间参数管理器
   * @param traffic 需要进行车辆元素填充的消息对象
   * @return txBool 如果填充成功则返回txTrue，否则返回txFalse
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间戳和车辆对象信息，填充车辆元素
   *
   * 该函数根据给定的时间戳和车辆对象信息，执行车辆元素的填充操作。该函数首先确保所需的数据和资源已准备好，然后对车辆元素进行填充。
   *
   * @param timeStamp 当前时间戳
   * @param pSimVehicle 需要进行填充的车辆对象指针
   * @return sim_msg::Car* 填充完成的车辆对象指针
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 对给定的时间戳和车辆元素信息，填充车辆的运动轨迹
   *
   * 该函数将对给定的时间戳和车辆元素信息，填充车辆的运动轨迹。函数首先检查所需的数据和资源是否已准备好，然后对车辆的运动轨迹进行填充。
   *
   * @param timeStamp 当前时间戳
   * @param vehicle 需要填充车辆元素的车辆对象
   * @return 返回填充完成的车辆运动轨迹信息
   */
  virtual txBool FillingTrajectory(Base::TimeParamManager const&, sim_msg::Trajectory&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查车辆是否能够走到目标路段
   *
   * 此函数检查车辆是否能够顺利走到指定的路段，如果可以则返回 true，否则返回 false。
   *
   * @param lane 目标路段信息
   * @return 车辆是否能够走到目标路段
   */
  virtual Base::txBool CheckReachable(const Base::Info_Lane_t&) const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief Pre-update 前置更新
   *
   * 该函数在数据更新之前被调用，用于进行相关元素状态的预处理。
   * 若更新成功，返回 true，否则返回 false。
   *
   * @param[in] timeParamManager 时间参数管理器
   * @param[in] map_elemId2Kinetics 元素 ID 到动力学信息的映射
   * @return 更新是否成功
   */
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;
  // virtual txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查生命周期
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算车辆元素的位移
   *
   * 此函数根据当前时间和可能需要的车辆参数计算车辆元素的位移。
   *
   * @param timeMgr 时间管理器，包含当前时间
   * @return 计算出的车辆元素的位移
   */
  virtual txFloat Compute_Displacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算横向位移
   *
   * 该函数计算车辆元素的横向位移。根据传入的时间参数和可能需要的车辆参数，
   * 计算车辆元素在指定时间内的横向位移值。
   *
   * @param timeMgr 时间管理器，包含当前时间
   * @return 计算出的横向位移
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新下一个通过路口的方向
   *
   * 该函数用于更新车辆元素下一个通过路口的方向。根据当前路口的状态以及可能需要的车辆参数，
   * 计算出车辆元素在接下来的行驶过程中应该采取的通过路口的方向。
   *
   * @param TX_NOEXCEPT TX_OVERRIDE {}
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 检查当前对象是否处于活跃状态
   *
   * 此函数检查当前对象是否存在，以及其关联的路径跟踪管理器是否存在。
   * 如果对象存在并且关联的路径跟踪管理器也存在，则返回 true，表示对象处于活跃状态；否则，返回 false。
   *
   * @return 当前对象是否处于活跃状态
   */
  virtual txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE {
    return ParentClass::IsAlive() && NonNull_Pointer(m_trajMgr);
  }

 public:
  /**
   * @brief 判断该对象是否支持指定的行为类型
   *
   * 此函数判断该对象是否支持指定的行为类型。如果支持，则返回true；否则返回false。
   *
   * @param behavior 指定的行为类型
   * @return 是否支持指定的行为类型
   */
  virtual Base::txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VEHICLE_BEHAVIOR::eTrajectoryFollow == behavior));
  }

 public:
  /**
   * @brief 检查是否可以开始执行该功能
   *
   * 当一个车辆元素需要实现具体的车辆轨迹跟踪功能时，需要先检查当前是否可以开始执行该功能。
   *
   * @param timeMgr 时间管理器，提供当前时间
   * @return true 如果可以开始执行该功能；false 否则
   */
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 在开始执行该功能之前进行准备
   *
   * 当一个车辆元素需要实现具体的车辆轨迹跟踪功能时，需要先检查当前是否可以开始执行该功能。
   *
   * @param timeMgr 时间管理器，提供当前时间
   */
  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取车辆元素的字符串表示形式
   *
   * @return txString 车辆元素的字符串表示形式
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Vehicle; }

  /**
   * @brief 获取车辆类型
   *
   * @return VEHICLE_TYPE
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle_type; }

  /**
   * @brief 获取车辆的行为类型
   *
   * @return VEHICLE_BEHAVIOR
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE {
    return VEHICLE_BEHAVIOR::eTrajectoryFollow;
  }

 public:
  /**
   * @brief 获取车辆切换车道的状态
   *
   * @return VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return _plus_(VehicleMoveLaneState::eStraight);
  }

 public:
  /**
   * @brief 判断车辆是否支持轨迹跟踪
   *
   * @return txBool
   */
  virtual txBool HasFollow() const TX_NOEXCEPT { return false; }

  /**
   * @brief 获取车辆当前跟随的目标ID（为-1时表示没有跟随目标）
   *
   * @return txInt
   */
  virtual txInt GetFollow() const TX_NOEXCEPT { return -1; }
  virtual Geometry::Topology::txEdgeVec GetTopologyRelationShip() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 设置车辆的动力学信息
   *
   * 设置车辆的动力学信息，包括车辆的运动特性、速度和加速度等。
   *
   * @param info 包含车辆动力学信息的结构体
   * @return 无
   */
  virtual void SetKinetics(const KineticsInfo_t& info) TX_NOEXCEPT;

  /**
   * @brief 更新车辆的动力学信息
   *
   * 根据给定的时间参数，更新车辆的动力学信息，例如速度和加速度等。
   *
   * @param timeMgr 包含当前时间参数的管理器
   * @return 如果成功更新动力学信息，则返回 true，否则返回 false
   */
  virtual txBool Update_Kinetics(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

 public:
  using KineticsUtilInfo_t = TrafficFlow::IKineticsHandler::KineticsUtilInfo_t;

  /**
   * @brief 获取kinetics描述
   *
   * @return txString
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_TrajFollow_VehicleElement"; }

  /**
   * @brief GetKineticsUtilInfo 获取车辆跟随动力学工具信息
   *
   * 此函数用于获取车辆跟随算法的动力学工具信息，用于统计和分析算法的性能。
   *
   * @return KineticsUtilInfo_t 返回包含动力学工具信息的结构体
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算车辆的动力学信息
   *
   * 此函数用于计算车辆的动力学信息，包括速度、加速度等，以及与其他车辆之间的动力学相关信息。
   *
   * @param timeMgr 时间管理器，包含当前时间等信息
   * @param _objInfo 车辆的动力学工具信息
   * @return txBool 计算成功返回 true，否则返回 false
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                 const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief HandlerEvent 处理事件
   *
   * 此函数用于处理车辆元素的事件。当需要车辆元素响应某个事件时，会调用此函数。
   * 当此函数执行成功时返回 true，否则返回 false。
   *
   * @param timeMgr 时间管理器，包含当前时间等信息
   * @param _eventPtr 事件指针，包含事件的信息
   * @return txBool 执行成功返回 true，否则返回 false
   */
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 在道路改变时，车辆元素的交通状态更新
   *
   * 当车辆需要在道路切换时改变路线时，调用此函数。
   * 函数返回 true 表示路线切换成功，否则表示路线切换失败。
   *
   * @param state 车辆道路改变状态
   * @param changeTime 道路切换所需时间
   * @param offset 道路切换时改变道路的偏移量
   * @return txBool 路线切换成功返回 true，否则返回 false
   */
  virtual Base::txBool ChangeInLane(const Base::Enums::VehicleMoveLaneState state, Base::txFloat changeTime,
                                    const Base::txFloat offset) TX_NOEXCEPT;

  /**
   * @brief 计算最短车道变更时间
   *
   * 该函数用于计算车辆在指定车道变更时间情况下，能够完成车道变更所需的最短时间。
   *
   * @param laneChangeTime 指定的车道变更时间
   * @return txFloat 计算得出的最短车道变更时间
   */
  virtual Base::txFloat ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT {
    return laneChangeTime;
  }

  /**
   * @brief 获取到轨道末端的距离
   *
   * 该函数用于获取车辆到轨道末端的剩余距离，包括需要走的轨道长度和之前已经行驶的距离。
   *
   * @return txFloat 车辆到轨道末端的距离
   */
  virtual Base::txFloat DistanceToEnd() const TX_NOEXCEPT {
    return (NonNull_Pointer(m_trajMgr)) ? (m_trajMgr->GetLength() - m_real_displacement) : (0.0);
  }

  /**
   * @brief 获取deadline半径
   *
   * 即当车辆已经接近某个栅栏时，仍然允许其继续前进的距离。
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat DeadLineRadius() const TX_NOEXCEPT { return 3.0; }

  /**
   * @brief 驾驶员在车道边缘处移动
   *
   * 当驾驶员所在车辆需要移动到车道边缘时，该函数被调用，以调整车辆的位置以避免与其他道路障碍物发生碰撞。
   *
   * @param timeMgr 时间参数管理器，包含当前时间、速度和持续时间等信息
   * @return txFloat 返回实际移动的距离
   */
  virtual Base::txFloat MoveTo_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取车辆在变道中的速度
   *
   * @param exper_time 期望时间
   * @return Base::txFloat
   */
  virtual Base::txFloat Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT;

  /**
   * @brief 计算车辆在更换车道后的行为
   *
   * 该函数用于在车辆更换车道后，计算并返回车辆在一段时间内应执行的行为。
   *
   * @param timeMgr 时间参数管理器，包含了当前时间、持续时间等信息
   * @return txBool 车辆在更换车道后是否需要执行上述操作，可选值为 true 或 false
   */
  virtual Base::txBool LaneChangePostOp(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取车辆变道过程中角度
   *
   * @param exper_time
   * @return Base::txFloat
   */
  virtual Base::txFloat Get_MoveTo_Sideway_Angle(Base::txFloat const exper_time) TX_NOEXCEPT;

  /**
   * @brief 更新车辆在执行切换车道操作时的状态。
   *
   * @param[in] timeMgr 包含当前时间和持续时间等信息的参数管理器对象。
   *
   * @return 返回一个布尔值，指示车辆是否正在执行切换车道操作。
   */
  virtual Base::txBool UpdateSwitchLaneRunning(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

 protected:
  /**
   * @brief 初始化路径规划管理器.
   *
   * @param[in] ref_control_path 待规划的控制路径，由控制路径节点组成的向量。
   * @param[in] startTime 规划开始时间.
   * @param[in] startAngle 规划开始时的方向角度.
   *
   * @return 成功则返回 true，否则返回 false.
   */
  virtual txBool InitializeTrajMgr(const control_path_node_vec& ref_control_path, const txFloat startTime,
                                   const txFloat startAngle) TX_NOEXCEPT;

  /**
   * @brief 计算车辆在沿着特定路径行驶时的加速度和持续时间.
   *
   * 该函数用于根据车辆当前的速度、目标速度和路径长度，计算出车辆在沿着特定路径行驶时的加速度和持续时间.
   *
   * @param[in] init_v 车辆当前的速度.
   * @param[in] final_v 车辆目标的速度.
   * @param[in] s 特定路径的长度.
   *
   * @return 一个包含两个元素的元组：第一个元素是计算出的加速度，第二个元素是车辆沿路径行驶的持续时间。
   */
  virtual std::tuple<txFloat /*acc*/, txFloat /*duration*/> ComputeTrajKinetics(const txFloat init_v,
                                                                                const txFloat final_v,
                                                                                const txFloat s) const TX_NOEXCEPT;

 public:
  /**
   * @brief 当前笛卡尔坐标系的x值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_horizontal();
  }

  /**
   * @brief 当前笛卡尔坐标系的y值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_vertical();
  }

  /**
   * @brief 当前点在笛卡尔坐标系的z值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_elevation();
  }

 protected:
  /**
   * @brief 计算车辆的行驶方向。
   *
   * 该函数计算车辆行驶的方向，以弧度制表示，返回一个范围为[0, 2π)的实数。
   *
   * @param _passTime 驾驶时间，以秒为单位。
   * @return 车辆的行驶方向，单位为弧度。
   */
  virtual Base::txFloat ComputeHeading(const txFloat _passTime) TX_NOEXCEPT;

 protected:
  HdMap::txLaneInfoInterfacePtr m_trajMgr = nullptr;
  SegmentKinectParamList m_KinectParamList;
  txFloat m_real_displacement = 0.0;
  BehaviorTimeManager m_LaneChangeTimeMgr;
  BehaviorTimeManager m_LaneAbortingTimeMgr;
  Base::txBool bNeedResetOffset = false;

#if __TX_Mark__("MergeActionManager")
  ActionManagerPtr m_MergeActionManagerPtr = nullptr;
  virtual void ClearMergeAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
#endif /*__TX_Mark__("MergeActionManager")*/
  Base::txFloat m_start_angle_degree = -1.0;
  Base::txFloat m_start_angle_action_duration_ms = 0.0;
  Base::txFloat m_sim_step_ms = 20.0;
};
using TAD_TrajFollow_VehicleElementPtr = std::shared_ptr<TAD_TrajFollow_VehicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
