// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/optional.hpp>
#include "tad_behavior_time_manager.h"
#include "tad_component.h"
#include "tad_fsm_user_defined.h"
#include "tad_planning.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_vehicle_kinetics_info.h"
#include "traffic.pb.h"
#include "tx_env_perception.h"
#include "tx_event_handler.h"
#include "tx_route_path_manager.h"
#include "tx_scene_loader.h"
#include "tx_vehicle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_UserDefined_VehicleElement : public Base::IVehicleElement,
                                       public IEventHandler,
                                       public SM::txUserDefinedState,
                                       public TrafficFlow::IKineticsHandler,
                                       public Base::IEnvPerception,
                                       public TAD_SceneEvent_1_0_0_0,
                                       public TAD_PlanningModule {
 public:
  using ParentClass = Base::IVehicleElement;
  using BehaviorTimeManager = TAD_BehaviorTimeManager;
  using DrivingStatus = IDrivingStatus::DrivingStatus;

 public:
  TAD_UserDefined_VehicleElement() TX_DEFAULT;
  virtual ~TAD_UserDefined_VehicleElement();

 public:
  /**
   * @brief 对象初始化函数
   *
   * @return txBool
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 动力学初始化
   *
   * @return txBool
   */
  virtual txBool Initialize_Kinetics(Base::ISceneLoader::IVehiclesViewerPtr) TX_NOEXCEPT;

  /**
   * @brief 元素更新函数
   *
   * @return txBool
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 资源释放函数
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前仿真步的元素输出的traffic信息
   *
   * @return txBool
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前仿真步输出的car信息
   *
   * @param timeStamp 时间对象
   * @param pSimVehicle 输出的pb对象指针
   * @return sim_msg::Car*
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前仿真步输出的trajectory信息
   *
   * @return txBool
   */
  virtual txBool FillingTrajectory(Base::TimeParamManager const&, sim_msg::Trajectory&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查是否可达
   *
   * @param _laneInfo 目标车道信息
   * @return Base::txBool
   */
  virtual Base::txBool CheckReachable(const Base::Info_Lane_t& _laneInfo) const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 更新前的预操作
   *
   * @param map_elemId2Kinetics
   * @return txBool
   */
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;
  // virtual txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查deadline
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 检查是否支持自定义行为
   *
   * 该函数用于判断当前车辆对象是否支持自定义行为。如果支持，则返回 true；否则返回 false。
   *
   * @param behavior 自定义行为
   * @return true 如果支持自定义行为，否则返回 false
   */
  virtual Base::txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VEHICLE_BEHAVIOR::eUserDefined) == behavior);
  }

 public:
  /**
   * @brief 检查是否允许启动某个特定的车辆元素
   *
   * 该函数用于检查特定的车辆元素是否可以在当前情况下开始运行。如果允许开始，则返回 true；否则返回 false。
   *
   * @param timeMgr 当前时间管理器
   * @return true 如果允许开始，否则返回 false
   */
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取车辆元素的字符串表示形式
   *
   * 该函数返回一个包含当前车辆元素的信息的字符串，便于在日志或其他地方打印该元素的详细信息。
   *
   * @return txString 返回一个包含当前车辆元素信息的字符串
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取元素的类型
   *
   * 该函数返回一个枚举值，表示当前车辆元素的类型，以便在日志或其他地方记录当前元素的类型。
   *
   * @return ElementType 返回一个表示当前元素类型的枚举值
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Vehicle; }

  /**
   * @brief 获取车辆的类型
   *
   * 该函数返回一个枚举值，表示当前车辆元素的类型，以便在日志或其他地方记录当前车辆元素的类型。
   *
   * @return VEHICLE_TYPE 返回一个表示当前车辆元素类型的枚举值
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle_type; }

  /**
   * @brief 获取车辆行为类型
   * @return eUserDefined 返回用户自定义车辆行为类型
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_BEHAVIOR::eUserDefined; }

 public:
  /**
   * @brief 获取车辆切换车道的状态
   * @return VehicleMoveLaneState 返回车辆切换车道的状态
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取事件处理器类型
   * @return EventHandlerType 返回事件处理器类型
   */
  virtual EventHandlerType GetEventHandlerType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件处理器描述
   * @return txString 返回事件处理器描述
   */
  virtual txString HandlerDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件
   *
   * 该方法会在检测到事件时被调用。
   * @param[in] eventPtr 事件指针
   * @return 事件处理结果
   */
  virtual txBool HandlerEvent(IEvent&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理eAcc事件
   *
   * 该方法会在检测到事件时被调用。
   * @param[in] eventPtr 事件指针
   * @return 事件处理结果
   */
  virtual void  HandlerEAcc(IEvent&) TX_NOEXCEPT;

  /**
   * @brief 处理eAccEvent事件
   *
   * 该方法会在检测到事件时被调用。
   * @param[in] eventPtr 事件指针
   * @return 事件处理结果
   */
  virtual void  HandlerEAccEvent(IEvent&) TX_NOEXCEPT;

  /**
   * @brief 处理eMerge事件
   *
   * 该方法会在检测到事件时被调用。
   * @param[in] eventPtr 事件指针
   * @return 事件处理结果
   */
  virtual void  HandlerEMerge(IEvent&) TX_NOEXCEPT;

  /**
   * @brief 处理eMergeEvent事件
   *
   * 该方法会在检测到事件时被调用。
   * @param[in] eventPtr 事件指针
   * @return 事件处理结果
   */
  virtual void  HandlerEMergeEvent(IEvent&) TX_NOEXCEPT;

  /**
   * @brief 获取事件触发工具信息
   *
   * @return EventHitUtilInfo_t
   */
  virtual EventHitUtilInfo_t GetEventHitUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取hander id
   *
   * @return txSysId
   */
  virtual txSysId HanderId() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取hander sys id
   *
   * @return txSysId
   */
  virtual txSysId HanderSysId() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 处理场景事件
   *
   * 此方法会在处理场景事件时被调用。
   * @param timeMgr 时间参数管理器
   * @param _eventPtr 事件视图指针
   * @return 事件处理结果
   */
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 判断当前车辆元素是否设置了跟随标识
   *
   * 判断当前车辆元素的 followId 是否被初始化，如果已初始化，则该元素具有跟随功能。
   *
   * @return true 当前车辆元素具有跟随功能，即其 followId 被初始化；false 当前车辆元素未设置跟随功能，其 followId
   * 未被初始化。
   */
  virtual txBool HasFollow() const TX_NOEXCEPT { return m_optional_followId.is_initialized(); }

  /**
   * @brief 获取当前车辆元素的跟随ID
   *
   * 该函数用于获取当前车辆元素的跟随ID。如果当前车辆元素已经设置了跟随功能，则返回其对应的跟随ID；否则返回0。
   *
   * @return 当前车辆元素的跟随ID，如果当前车辆元素没有设置跟随功能，则返回0。
   */
  virtual txInt GetFollow() const TX_NOEXCEPT { return (*m_optional_followId); }

  /**
   * @brief 获取当前车辆元素的拓扑关系
   *
   * 该函数用于获取当前车辆元素与其他车辆元素之间的拓扑关系，并返回一个包含这些关系的向量。
   *
   * @return 一个向量，包含当前车辆元素与其他车辆元素之间的拓扑关系。
   */
  virtual Geometry::Topology::txEdgeVec GetTopologyRelationShip() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 设置车辆的动力学信息
   * @param info 车辆动力学信息
   */
  virtual void SetKinetics(Base::TimeParamManager const& timeMgr, const KineticsInfo_t& info) TX_NOEXCEPT;

  /**
   * @brief 更新车辆的动力学信息
   *
   * 更新车辆的动力学信息，使其与时间参数管理器时间同步。
   *
   * @param timeMgr 时间参数管理器的引用，包含当前时间等相关信息。
   * @return txBool 更新成功则返回 true，否则返回 false。
   */
  virtual txBool Update_Kinetics(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

 public:
  using KineticsUtilInfo_t = TrafficFlow::IKineticsHandler::KineticsUtilInfo_t;

  /**
   * @brief 获取车辆动力学描述
   *
   * 该函数返回一个描述车辆动力学属性的字符串。
   *
   * @return txString 车辆动力学描述字符串
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_UserDefined_VehicleElement"; }

  /**
   * @brief 获取车辆动力学工具信息
   *
   * 该函数返回一个包含车辆动力学工具信息的结构体，包括各项动力学参数的有效性标志，各项动力学参数的值，及其对应的误差。
   *
   * @return KineticsUtilInfo_t 车辆动力学工具信息结构体
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算车辆动力学信息
   *
   * 此函数用于计算车辆的动力学信息，包括摩擦系数、角速度、位移等。
   *
   * @param timeMgr 时间参数管理器，包含当前时间等信息
   * @param _objInfo 车辆动力学实用信息结构体，包含车辆的各项参数信息
   * @return txBool 函数执行是否成功，返回 true 表示成功，false 表示失败
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                 const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 检查元素的waypoint
   *
   */
  virtual void CheckWayPoint() TX_NOEXCEPT;

  /**
   * @brief 移动车辆
   *
   * 该方法根据当前时间，将车辆移动到下一个允许的站点，并沿着一个方向行驶。
   *
   * @param timeMgr 时间参数管理器，用于获取当前时间和下一个站点的时间
   * @return true 表示可以实现直接移动，false 表示无法实现直接移动
   */
  virtual txBool MoveStraightOnS(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief SearchNextLinkByRouteInfo
   *
   * 根据给定的起点和终点车道信息，查找下一个可用的车道连接点。
   *
   * @param nextLaneLinks 下一个可用的车道连接点列表
   * @param toLaneId_neg 车道ID
   * @param wishLaneLinkId 希望的车道连接点ID
   * @return hadmap::txLaneLinkPtr 返回查找到的车道连接点指针，若未找到则返回空指针
   */
  virtual hadmap::txLaneLinkPtr SearchNextLinkByRouteInfo(const hadmap::txLaneLinks& nextLaneLinks,
                                                          const Base::txLaneID toLaneId_neg,
                                                          const Base::txLaneLinkID wishLaneLinkId) TX_NOEXCEPT;

  /**
   * @brief 查找下一个可用的车道连接点
   *
   * 根据给定的起点和终点车道信息，查找下一个可用的车道连接点。
   *
   * @param nextLaneLinks 下一个可用的车道连接点列表
   * @param toLaneId_neg 车道ID
   * @return hadmap::txLaneLinkPtr 返回查找到的车道连接点指针，若未找到则返回空指针
   */
  virtual hadmap::txLaneLinkPtr SearchNextLinkByRouteInfo(const hadmap::txLaneLinks& nextLaneLinks,
                                                          const Base::txLaneID toLaneId_neg) TX_NOEXCEPT;

  /**
   * @brief 换道后的后置操作
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool LaneChangePostOp(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 计算车辆在给定时间下的水平位移
   *
   * 该函数用于计算车辆在给定时间下的水平位移，该位移由车辆在当前车道上的角度、速度、车辆重量、车辆质量等因素决定。
   * 该函数的返回值为车辆在水平方向上的位移。
   *
   * @param timeMgr 时间管理器，包括时间、帧率等信息
   * @return 水平位移
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据经验时间计算车辆行驶至目标侧向角度
   *
   * 该函数用于根据经验时间计算车辆在车道内根据响应时间所需要行驶的侧向角度。
   * 根据车辆速度、车道宽度、车辆重量以及相关因素，可以得出车辆走向目标侧向的角度。
   *
   * @param exper_time 经验时间，表示从车辆当前位置到目标侧向的所需时间
   * @return 车辆行驶至目标侧向的角度
   */
  Base::txFloat Get_MoveTo_Sideway_Angle(Base::txFloat const exper_time);

  /**
   * @brief 车辆沿车道后移的方向角度计算
   *
   * 该函数计算沿车道后移的车辆侧向角度，需要输入时间和当前速度。
   * 该函数的输入为时间和当前速度，输出为沿车道后移的侧向角度。
   *
   * @param time 沿车道后移的时间
   * @param curSpeed 当前车速
   * @return 返回沿车道后移的侧向角度
   */
  Base::txFloat MoveBack_Sideway_Angle(const Base::txFloat time);

  /**
   * @brief 更新车辆切换车道中的功能状态
   *
   * 当车辆准备切换车道时，此函数用于更新车辆的切换车道功能。
   * 如果车辆尚未准备好切换车道，则返回false。
   *
   * @param timeMgr 时间管理器，提供当前时间信息
   * @return 车辆切换车道中的功能状态
   */
  virtual txBool UpdateSwitchLaneRunning(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 移动到边上车道
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat MoveTo_Sideway(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取换道时的速度
   *
   * @param exper_time 期望时间
   * @return txFloat
   */
  virtual txFloat Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT;

  /**
   * @brief 改变车辆车道
   *
   * 当车辆需要改变车道时，调用该函数。该函数的返回值指示车辆改变车道的结果。
   * @param state 车辆改变车道的状态
   * @param changeTime 车辆从当前车道改变到目标车道所需的时间
   * @param offset 车辆相对于车道中心的位置
   * @return 车辆改变车道的结果，如果成功则为true，否则为false
   */
  virtual txBool ChangeInLane(const Base::Enums::VehicleMoveLaneState state, txFloat changeTime,
                              const txFloat offset) TX_NOEXCEPT;

  /**
   * @brief 改变车道状态
   *
   * 当需要切换车道时，调用此函数。返回一个布尔值，表示车辆是否已成功切换车道。
   * @param state 车辆切换车道的状态
   * @param bCheckGoalLane 检查目标车道是否可用
   * @param bForce 强制切换车道，即使目标车道不可用
   * @param laneChangeTime 车辆从当前车道切换到目标车道所需的时间，默认值为短车道变更时间
   * @return 返回一个布尔值，表示车辆是否已成功切换车道
   */
  virtual txBool SwitchLane(const Base::Enums::VehicleMoveLaneState state, Base::txBool bCheckGoalLane,
                            Base::txBool bForce = false,
                            Base::txFloat laneChangeTime = FLAGS_Average_LaneChanging_Duration_Short) TX_NOEXCEPT;

  /**
   * @brief 计算最短的车道切换时间
   *
   * 当需要切换车道时，调用此函数。计算出最短的车道切换时间。
   * @param laneChangeTime 默认车道切换时间
   * @return 返回最短的车道切换时间
   */
  Base::txFloat ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT;

 public:
  /**
   * @brief 对环境感知的执行函数
   *
   * 在每次执行感知的时候都会调用这个函数
   * @param timeMgr 时间管理器，用于获取系统当前时间
   */
  virtual void ExecuteEnvPerception(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}

 protected:
  /**
   * @brief 更新下一个通过点的行驶方向
   *
   * 该函数用于更新下一个通过点的行驶方向，在进行任何车道改变或者行驶过程中车道需要改变时都需要调用此函数。
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 设置车辆的角度
   *
   * 当需要改变车辆的朝向时，使用此函数来设置车辆的角度。
   * @param _degree 车辆朝向的角度，以度为单位
   */
  virtual void SetAngle(Base::txFloat _degree) TX_NOEXCEPT { mLocation.fAngle().FromDegree(_degree); }

 public:
  /**
   * @brief 重置车道保持状态
   *
   * 此函数用于重置车辆的车道保持状态。
   */
  virtual void ResetLaneKeep() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 返回车辆路径的中点GPS坐标
   *
   * 此函数返回车辆路径的中点GPS坐标。中点是指车辆在路线中的所有交叉点的平均位置。返回的坐标类型为hadmap::txPoint。
   *
   * @return 车辆路径的中点GPS坐标
   */
  virtual std::vector<hadmap::txPoint> RouteMidGPS() const TX_NOEXCEPT TX_OVERRIDE { return mRoute.MidPts(); }

  /**
   * @brief 获取车辆路径的终点GPS坐标
   *
   * 此函数返回车辆路径的终点GPS坐标。返回的坐标类型为hadmap::txPoint。
   *
   * @return 车辆路径的终点GPS坐标
   */
  virtual hadmap::txPoint RouteEndGPS() const TX_NOEXCEPT TX_OVERRIDE { return mRoute.EndPt(); }

 public:
  /**
   * @brief 获取本地坐标系下点的x值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_horizontal();
  }

  /**
   * @brief 获取本地坐标系下的y值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_vertical();
  }

  /**
   * @brief 获取本地坐标系下的z值
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_elevation();
  }

 public:
  /**
   * @brief 计算与道路物体的距离
   *
   * @param target_element_geom_center 目标元素中心
   * @return FrenetProjInfo
   */
  virtual FrenetProjInfo ComputeRoadProjectDistance(const Coord::txENU& target_element_geom_center) const TX_NOEXCEPT
      TX_OVERRIDE;
#if __TX_Mark__("state machine interface")

 public:
  /**
   * @brief 当前元素是否左转
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsTurnLeft() const TX_NOEXCEPT TX_OVERRIDE { return IsInTurnLeft(); }

  /**
   * @brief 当前元素是否右转
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsTurnRight() const TX_NOEXCEPT TX_OVERRIDE { return IsInTurnRight(); }

  /**
   * @brief 当前元素是否直行
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsTurnStraight() const TX_NOEXCEPT TX_OVERRIDE { return IsLaneKeep(); }

  /**
   * @brief 当前元素是否处于换道
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneChange() const TX_NOEXCEPT TX_OVERRIDE { return IsInLaneChange(); }
#endif /*state machine interface*/

 protected:
  boost::optional<txInt> m_optional_followId;
  TrafficFlow::RoutePathManager roadInfoArray;
  BehaviorTimeManager m_LaneChangeTimeMgr;
  BehaviorTimeManager m_LaneAbortingTimeMgr;
  Base::txBool bNeedResetOffset = false;

  TrafficFlow::Component::Route mRoute;

#if __TX_Mark__("MergeActionManager")
  ActionManagerPtr m_MergeActionManagerPtr = nullptr;
  virtual void ClearMergeAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
#endif /*__TX_Mark__("MergeActionManager")*/

#if __TX_Mark__("LateralDistanceAction")
  class LateralDistanceActionManager {
    using txString = Base::txString;
    using txBool = Base::txBool;
    using txFloat = Base::txFloat;

   public:
    LateralDistanceActionManager() TX_DEFAULT;
    ~LateralDistanceActionManager() TX_DEFAULT;

    /**
     * @brief 初始化行驶目标车辆的 VehicleElement 对象
     *
     * 这个函数使用来初始化一个行驶目标车辆的 VehicleElement
     * 对象。在初始化期间，用户需要提供一些必要的元素信息和一个指向目标车辆的指针。
     *
     * @param _evId_actionId 指定此行驶目标车辆的事件 ID 和动作 ID
     * @param _self_elemType_elemId 指定此行驶目标车辆的元素类型和元素 ID
     * @param _target_elemType_elemId 指定目标车辆的元素类型和元素 ID
     * @param target_element_ptr 指向目标车辆的指针
     * @param _distance 行驶目标车辆到目标车辆的距离
     * @param _maxAcc 指定行驶目标车辆的最大加速度
     * @param _maxDec 指定行驶目标车辆的最大减速度
     * @param _maxSpeed 指定行驶目标车辆的最大速度
     * @param _freespace 指定是否需要考虑自由空间的信息
     * @param _continuous 指定行驶目标车辆的行驶是否是连续的
     * @return 如果成功初始化则返回 true，否则返回 false
     */
    Base::txBool Initialize(const txString _evId_actionId, const txString _self_elemType_elemId,
                            const txString _target_elemType_elemId, Base::ITrafficElementPtr target_element_ptr,
                            const txFloat _distance, const txFloat _maxAcc, const txFloat _maxDec,
                            const txFloat _maxSpeed, const txBool _freespace, const txBool _continuous) TX_NOEXCEPT {
      if (NonNull_Pointer(target_element_ptr) && _NonEmpty_(_evId_actionId) && _NonEmpty_(_self_elemType_elemId) &&
          _NonEmpty_(_target_elemType_elemId)) {
        m_evId_actionId = _evId_actionId;
        m_self_elemType_elemId = _self_elemType_elemId;
        m_target_elemType_elemId = _target_elemType_elemId;
        m_target_element_ptr = target_element_ptr;
        m_distance = _distance;
        m_maxAcc = (_maxAcc < 0.0) ? (FLT_MAX) : (_maxAcc);
        m_maxDec = (_maxDec < 0.0) ? (FLT_MAX) : (_maxDec);
        m_maxSpeed = (_maxSpeed < 0.0) ? (FLT_MAX) : (_maxSpeed);
        m_freespace = _freespace;
        m_continuous = _continuous;
        m_isValid = true;
      } else {
        m_isValid = false;
      }
      return IsValid();
    }

    /**
     * @brief 获取当前对象的有效性
     *
     * 该函数用于检查当前对象是否已经正确初始化，如果初始化成功，则返回 true，否则返回 false。
     *
     * @return true 如果当前对象已经正确初始化，则返回 true；否则返回 false。
     */
    Base::txBool IsValid() const TX_NOEXCEPT { return m_isValid; }

    /**
     * @brief 获取当前对象的事件 ID 和动作 ID
     *
     * 该函数用于获取当前对象的事件 ID 和动作 ID，通常用于存储事件信息。
     *
     * @return 当前对象的事件 ID 和动作 ID，格式为 "event_id:action_id"。
     */
    Base::txString evId_actionId() const TX_NOEXCEPT { return m_evId_actionId; }

    /**
     * @brief 获取当前对象的元素类型和元素ID
     *
     * 该函数用于获取当前对象的元素类型和元素ID，通常用于存储元素信息。
     *
     * @return 当前对象的元素类型和元素ID，格式为 "elemType_elemId"。
     */
    Base::txString self_elemType_elemId() const TX_NOEXCEPT { return m_self_elemType_elemId; }

    /**
     * @brief 获取当前对象的目标元素类型和目标元素ID
     *
     * 该函数用于获取当前对象的目标元素类型和目标元素ID，通常用于存储目标元素信息。
     *
     * @return 当前对象的目标元素类型和目标元素ID，格式为 "target_elemType_elemId"。
     */
    Base::txString target_elemType_elemId() const TX_NOEXCEPT { return m_target_elemType_elemId; }

    /**
     * @brief 获取当前对象的目标元素指针
     *
     * 该函数用于获取当前对象的目标元素指针，通常用于获取目标元素信息。
     *
     * @return 当前对象的目标元素指针
     */
    Base::ITrafficElementPtr target_element_ptr() const TX_NOEXCEPT { return m_target_element_ptr; }

    /**
     * @brief 获取当前对象的距离
     *
     * 该函数用于获取当前对象的距离，通常用于获取距离信息。
     *
     * @return 当前对象的距离
     */
    txFloat distance() const TX_NOEXCEPT { return m_distance; }

    /**
     * @brief 获取当前对象的最大加速度
     *
     * 该函数用于获取当前对象的最大加速度，通常用于获取最大加速度信息。
     *
     * @return 当前对象的最大加速度
     */
    txFloat maxAcc() const TX_NOEXCEPT { return m_maxAcc; }

    /**
     * @brief 获取当前对象的最大减速度
     *
     * 该函数用于获取当前对象的最大减速度，通常用于获取最大减速度信息。
     *
     * @return 当前对象的最大减速度
     */
    txFloat maxDec() const TX_NOEXCEPT { return m_maxDec; }

    /**
     * @brief 获取当前对象的最大速度
     *
     * 该函数用于获取当前对象的最大速度，通常用于获取最大速度信息。
     *
     * @return 当前对象的最大速度
     */
    txFloat maxSpeed() const TX_NOEXCEPT { return m_maxSpeed; }

    /**
     * @brief 判断当前车辆是否有空间
     *
     * 该函数用于判断当前车辆是否有足够的空间用于停车。
     *
     * @return 返回真（txTrue），表示有足够的空间；返回假（txFalse），表示没有足够的空间。
     */
    txBool freespace() const TX_NOEXCEPT { return m_freespace; }

    /**
     * @brief 判断当前车辆是否支持持续停车
     *
     * 该函数用于判断当前车辆是否支持持续停车。
     *
     * @return 返回真（txTrue），表示支持持续停车；返回假（txFalse），表示不支持持续停车。
     */
    txBool continuous() const TX_NOEXCEPT { return m_continuous; }

    /**
     * @brief 判断车辆当前的方向是向左还是向右
     *
     * 该函数用于判断车辆当前的方向是向左还是向右。返回值为true表示车辆向左行驶，返回值为false表示车辆向右行驶。
     *
     * @return true 表示车辆向左行驶；false 表示车辆向右行驶。
     */
    txBool left() const TX_NOEXCEPT { return distance() < 0.0; }

    /**
     * @brief 获取车辆当前的行驶方向
     *
     * 该函数用于获取车辆当前的行驶方向。根据车辆的行驶距离（返回值）的正负，返回true表示车辆向右行驶，返回false表示车辆向左行驶。
     *
     * @return 如果车辆向右行驶，返回true；否则返回false。
     */
    txBool right() const TX_NOEXCEPT { return distance() > 0.0; }

    /**
     * @brief 对象是否居中
     *
     * @return txBool
     */
    txBool center() const TX_NOEXCEPT { return Math::isZero(distance()); }

    /**
     * @brief 获取车辆上一次的侧向速度
     *
     * 该函数用于获取车辆上一次的侧向速度。
     *
     * @return 返回车辆上一次的侧向速度。
     */
    txFloat& last_v() TX_NOEXCEPT { return m_last_lateral_velocity; }
    txFloat last_v() const TX_NOEXCEPT { return m_last_lateral_velocity; }

    /**
     * @brief 距离目标sign的距离
     *
     * @return txFloat 距离值
     */
    txFloat sign_target_dist() const TX_NOEXCEPT { return m_sign_target_dist; }

    /**
     * @brief Set the sign target dist object
     *
     * @param dist
     */
    void set_sign_target_dist(const txFloat dist) TX_NOEXCEPT { m_sign_target_dist = dist; }

    /**
     * @brief 获取对象的格式化输出
     *
     * @return txString
     */
    txString Str() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << TX_VARS(self_elemType_elemId()) << TX_VARS(target_elemType_elemId()) << TX_VARS(evId_actionId())
          << TX_VARS(distance());
      return oss.str();
    }

   protected:
    Base::txBool m_isValid = false;
    Base::txString m_evId_actionId;
    Base::txString m_self_elemType_elemId;
    Base::txString m_target_elemType_elemId;
    Base::ITrafficElementPtr m_target_element_ptr = nullptr;
    txFloat m_distance = 0.0;
    txFloat m_maxAcc = 3.5;
    txFloat m_maxDec = 3.5;
    txFloat m_maxSpeed = 20;
    Base::txBool m_freespace = false;
    Base::txBool m_continuous = false;
    Base::txFloat m_last_lateral_velocity = 0.0;
    Base::txFloat m_sign_target_dist = 0.0;
  };
  using LateralDistanceActionManagerPtr = std::shared_ptr<LateralDistanceActionManager>;
  LateralDistanceActionManagerPtr m_LateralActionMgrPtr = nullptr;

  /**
   * @brief 更新侧向距离动作
   *
   * 更新车辆侧向距离的动作。该函数的时间管理器将传递给该函数，该函数根据当前车辆状态更新侧向距离。
   *
   * @param timeMgr 时间管理器，包含了当前时间、时间增量等信息
   */
  virtual void UpdateLateralDistanceAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 清除侧位距离动作
   *
   * 清除侧位距离动作，包括侧位检测、侧位处理等。在本函数中，如果用户有指定任何侧位处理的方式，则以用户指定的方式进行处理。
   *
   * @param timeMgr 时间参数管理器，包括当前时间、时间步长等信息
   */
  virtual void ClearLateralDistanceAction(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
#endif /*__TX_Mark__("LateralDistanceAction")*/
};
using TAD_UserDefined_VehicleElementPtr = std::shared_ptr<TAD_UserDefined_VehicleElement>;
TX_NAMESPACE_CLOSE(TrafficFlow)
