// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_behavior_time_manager.h"
#include "tad_component.h"
#include "tad_fsm_ai.h"
#include "tad_vehicle_kinetics_info.h"
#include "traffic.pb.h"
#include "tx_env_perception.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_vehicle_element.h"
#if USE_SUDOKU_GRID
#  include "tad_vehicle_scan_region.h"
#endif /*USE_SUDOKU_GRID*/
#include "tad_behavior_time_manager.h"
#include "tad_planning.h"
#include "tx_driving_change_lane.h"
#include "tx_driving_follower.h"
#include "tx_element_generator.h"
#include "tx_serialization.h"
#include "tx_traffic_junction_rules.h"
#include "tx_traffic_rules.h"
#ifdef InjectMode
#  include "vehicle_element_inject_mode.h"
#endif /*InjectMode*/
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(TrafficFlow)

class IReRoute {
 public:
  using RouteAI = TrafficFlow::Component::RouteAI;
  /**
   * @brief 用于获取当前路由组的ID
   *
   * @return Base::txSysId
   */
  virtual Base::txSysId GetCurRouteGroupId() const TX_NOEXCEPT = 0;

  /**
   * @brief 用于获取子路由ID
   *
   * @return Base::txInt
   */
  virtual Base::txInt SubRouteId() const TX_NOEXCEPT = 0;

  /**
   * @brief 用于检查是否支持重新路由
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsSupportReRoute() const TX_NOEXCEPT = 0;

  /**
   * @brief 用于重新路由
   *
   * @param routeGroupId 路线分组的ID
   * @param subRouteId 子路线的ID
   * @param newRouteAI 指定的新路线AI
   * @return Base::txBool
   */
  virtual Base::txBool ReRoute(const Base::txSysId routeGroupId, const Base::txInt subRouteId,
                               const RouteAI &newRouteAI) TX_NOEXCEPT = 0;

  /**
   * @brief 用于检查是否有路由
   *
   * @return Base::txBool
   */
  virtual Base::txBool HasRoute() const TX_NOEXCEPT = 0;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IReRoute");
  }
};
#if __TX_Mark__(txVehicleBehavior)
class txVehicleBehavior {
 public:
  txVehicleBehavior();
  virtual ~txVehicleBehavior() TX_DEFAULT;
  /**
   * @brief 获取 NoCheckLCAbortThreshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat NoCheckLCAbortThreshold() const TX_NOEXCEPT { return mNoCheckLCAbortThreshold; }

  /**
   * @brief 获取 ChangeTopologyThreshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat ChangeTopologyThreshold() const TX_NOEXCEPT { return mChangeTopologyThreshold; }

  /**
   * @brief 获取 LaneKeep 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat LaneKeep() const TX_NOEXCEPT { return mLaneKeep; }

  /**
   * @brief 获取 Driving_Parameters 参数的值，根据 idx 选择
   *
   * @param idx
   * @return Base::txFloat
   */
  Base::txFloat Driving_Parameters(const Base::txInt idx) const TX_NOEXCEPT { return mDriving_Parameters.at(idx); }

  /**
   * @brief 获取 Max_Deceleration 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Max_Deceleration() const TX_NOEXCEPT { return mMax_Deceleration; }

  /**
   * @brief 获取 Max_Ped_Reaction_Distance 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Max_Ped_Reaction_Distance() const TX_NOEXCEPT { return mMax_Ped_Reaction_Distance; }

  /**
   * @brief 获取 MAX_SPEED 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat MAX_SPEED() const TX_NOEXCEPT { return mMAX_SPEED; }

  /**
   * @brief 获取 ComfortGap 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat ComfortGap() const TX_NOEXCEPT { return mComfortGap; }

  /**
   * @brief 获取 MAX_REACTION_GAP 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat MAX_REACTION_GAP() const TX_NOEXCEPT { return mMAX_REACTION_GAP; }

  /**
   * @brief 获取 SpeedDenominatorOffset 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat SpeedDenominatorOffset() const TX_NOEXCEPT { return mSpeedDenominatorOffset; }

  /**
   * @brief 获取 Tolerance_Threshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Tolerance_Threshold() const TX_NOEXCEPT { return mTolerance_Threshold; }

  /**
   * @brief 获取 LaneChanging_Aggressive_Threshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat LaneChanging_Aggressive_Threshold() const TX_NOEXCEPT { return mLaneChanging_Aggressive_Threshold; }

  /**
   * @brief 获取 Yielding_Aggressive_Threshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Yielding_Aggressive_Threshold() const TX_NOEXCEPT { return mYielding_Aggressive_Threshold; }

  /**
   * @brief 获取 SafetyGap_Front 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat SafetyGap_Front() const TX_NOEXCEPT { return mSafetyGap_Front; }

  /**
   * @brief 获取 SafetyGap_Rear 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat SafetyGap_Rear() const TX_NOEXCEPT { return mSafetyGap_Rear; }

  /**
   * @brief 获取 SafetyGap_Side 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat SafetyGap_Side() const TX_NOEXCEPT { return mSafetyGap_Side; }

  /**
   * @brief 获取 EmptyTargetLaneGapThreshold 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat EmptyTargetLaneGapThreshold() const TX_NOEXCEPT { return mEmptyTargetLaneGapThreshold; }

  /**
   * @brief 获取 Signal_Reaction_Gap 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Signal_Reaction_Gap() const TX_NOEXCEPT { return mSignal_Reaction_Gap; }

  /**
   * @brief 获取 Max_Comfort_Deceleration 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Max_Comfort_Deceleration() const TX_NOEXCEPT { return mMax_Comfort_Deceleration; }

  /**
   * @brief 获取 W74_DEFAULT_ax 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_ax() const TX_NOEXCEPT { return mW74_DEFAULT_ax; }

  /**
   * @brief 获取 W74_DEFAULT_bx_add 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_bx_add() const TX_NOEXCEPT { return mW74_DEFAULT_bx_add; }

  /**
   * @brief 获取 W74_DEFAULT_bx_mult 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_bx_mult() const TX_NOEXCEPT { return mW74_DEFAULT_bx_mult; }

  /**
   * @brief 获取 W74_DEFAULT_cx 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_cx() const TX_NOEXCEPT { return mW74_DEFAULT_cx; }

  /**
   * @brief 获取 W74_DEFAULT_ex_add 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_ex_add() const TX_NOEXCEPT { return mW74_DEFAULT_ex_add; }

  /**
   * @brief 获取 W74_DEFAULT_ex_mult 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_ex_mult() const TX_NOEXCEPT { return mW74_DEFAULT_ex_mult; }

  /**
   * @brief 获取 W74_DEFAULT_opdv_mult 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_opdv_mult() const TX_NOEXCEPT { return mW74_DEFAULT_opdv_mult; }

  /**
   * @brief 获取 W74_DEFAULT_bnull_mult 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat W74_DEFAULT_bnull_mult() const TX_NOEXCEPT { return mW74_DEFAULT_bnull_mult; }

  /**
   * @brief 获取 W74_DEFAULT_v2cav 参数的值
   *
   * @return Base::txBool
   */
  Base::txBool W74_DEFAULT_v2cav() const TX_NOEXCEPT { return mW74_DEFAULT_v2cav; }

  /**
   * @brief 获取 Friction_Factor 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat Friction_Factor() const TX_NOEXCEPT { return mFriction_Factor; }

  /**
   * @brief 获取 SideGapMinVelocity 参数的值
   *
   * @return Base::txFloat
   */
  Base::txFloat SideGapMinVelocity() const TX_NOEXCEPT { return mSideGapMinVelocity; }

 protected:
  Base::txFloat mFriction_Factor = 0.6;
  Base::txFloat mLaneKeep = 5.0;
  Base::txFloat mMAX_SPEED = 27.8;
  Base::txFloat mChangeTopologyThreshold = 0.76;
  Base::txFloat mNoCheckLCAbortThreshold = 0.8;
  Base::txFloat mEmptyTargetLaneGapThreshold = 50;
  Base::txFloat mSpeedDenominatorOffset = 1.0;
  Base::txFloat mSafetyGap_Rear = 3;
  Base::txFloat mSafetyGap_Front = 3;
  Base::txFloat mSafetyGap_Side = 1;
  Base::txFloat mComfortGap = 40.0;
  Base::txFloat mMax_Ped_Reaction_Distance = 30.0;
  Base::txFloat mMAX_REACTION_GAP = 100.0;
  Base::txFloat mLaneChanging_Aggressive_Threshold = 0.5;
  Base::txFloat mTolerance_Threshold = 0.8;
  Base::txFloat mMax_Deceleration = -10.0;
  Base::txFloat mSignal_Reaction_Gap = 20.0;
  Base::txFloat mMax_Comfort_Deceleration = -10.0;
  Base::txFloat mYielding_Aggressive_Threshold = 0.5;

  Base::txFloat mW74_DEFAULT_ax = 2;
  Base::txFloat mW74_DEFAULT_bx_add = 2;
  Base::txFloat mW74_DEFAULT_bx_mult = 3;
  Base::txFloat mW74_DEFAULT_cx = 40;
  Base::txFloat mW74_DEFAULT_ex_add = 1.5;
  Base::txFloat mW74_DEFAULT_ex_mult = 0.55;
  Base::txFloat mW74_DEFAULT_opdv_mult = 1.5;
  Base::txFloat mW74_DEFAULT_bnull_mult = 0.25;
  Base::txBool mW74_DEFAULT_v2cav = false;

  std::vector<Base::txFloat> mDriving_Parameters = {1.50, 1.30, 4.00, -12.00, -0.25, 1.35, 0.0006, 0.25, 2.00, 1.50};
  Base::txFloat mSideGapMinVelocity = 0.2;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("txVehicleBehavior");
    archive(_MAKE_NVP_("NoCheckLCAbortThreshold", mNoCheckLCAbortThreshold));
    archive(_MAKE_NVP_("ChangeTopologyThreshold", mChangeTopologyThreshold));
    archive(_MAKE_NVP_("LaneKeep", mLaneKeep));
    archive(_MAKE_NVP_("Driving_Parameters", mDriving_Parameters));
    archive(_MAKE_NVP_("Max_Deceleration", mMax_Deceleration));
    archive(_MAKE_NVP_("Max_Ped_Reaction_Distance", mMax_Ped_Reaction_Distance));
    archive(_MAKE_NVP_("MAX_SPEED", mMAX_SPEED));
    archive(_MAKE_NVP_("ComfortGap", mComfortGap));
    archive(_MAKE_NVP_("MAX_REACTION_GAP", mMAX_REACTION_GAP));
    archive(_MAKE_NVP_("SpeedDenominatorOffset", mSpeedDenominatorOffset));
    archive(_MAKE_NVP_("Tolerance_Threshold", mTolerance_Threshold));
    archive(_MAKE_NVP_("LaneChanging_Aggressive_Threshold", mLaneChanging_Aggressive_Threshold));
    archive(_MAKE_NVP_("Yielding_Aggressive_Threshold", mYielding_Aggressive_Threshold));
    archive(_MAKE_NVP_("SafetyGap_Front", mSafetyGap_Front));
    archive(_MAKE_NVP_("SafetyGap_Rear", mSafetyGap_Rear));
    archive(_MAKE_NVP_("SafetyGap_Side", mSafetyGap_Side));
    archive(_MAKE_NVP_("EmptyTargetLaneGapThreshold", mEmptyTargetLaneGapThreshold));
    archive(_MAKE_NVP_("Signal_Reaction_Gap", mSignal_Reaction_Gap));
    archive(_MAKE_NVP_("Max_Comfort_Deceleration", mMax_Comfort_Deceleration));
    archive(_MAKE_NVP_("W74_DEFAULT_ax", mW74_DEFAULT_ax));
    archive(_MAKE_NVP_("W74_DEFAULT_bx_add", mW74_DEFAULT_bx_add));
    archive(_MAKE_NVP_("W74_DEFAULT_bx_mult", mW74_DEFAULT_bx_mult));
    archive(_MAKE_NVP_("W74_DEFAULT_cx", mW74_DEFAULT_cx));
    archive(_MAKE_NVP_("W74_DEFAULT_ex_add", mW74_DEFAULT_ex_add));
    archive(_MAKE_NVP_("W74_DEFAULT_ex_mult", mW74_DEFAULT_ex_mult));
    archive(_MAKE_NVP_("W74_DEFAULT_opdv_mult", mW74_DEFAULT_opdv_mult));
    archive(_MAKE_NVP_("W74_DEFAULT_bnull_mult", mW74_DEFAULT_bnull_mult));
    archive(_MAKE_NVP_("W74_DEFAULT_v2cav", mW74_DEFAULT_v2cav));
    archive(_MAKE_NVP_("Friction_Factor", mFriction_Factor));
    archive(_MAKE_NVP_("SideGapMinVelocity", mSideGapMinVelocity));
  }
};
using txVehicleBehaviorPtr = std::shared_ptr<txVehicleBehavior>;
#endif /*txVehicleBehavior*/

class TAD_AI_VehicleElement : public Base::IVehicleElement,
                              public Base::IEnvPerception_TAD_AI_Hashed,
                              public SM::txAITrafficState,
                              public TrafficFlow::IKineticsHandler,
                              public Base::IDrivingFollower,
                              public Base::IDrivingChangeLane,
                              public Base::ITrafficRules,
                              public IReRoute,
                              public Base::ITrafficJunctionRules,
                              public txVehicleBehavior,
                              public TAD_PlanningModule {
 public:
  using ParentClass = Base::IVehicleElement;
  using BehaviorTimeManager = TAD_BehaviorTimeManager;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  using VehicleInitParam_t = Base::VehicleInputeAgent::VehicleInitParam_t;

 public:
  TAD_AI_VehicleElement() { ConfigureFocusTypes(); }
  virtual ~TAD_AI_VehicleElement();

 public:
  /**
   * @brief 初始化场景元素
   *
   * 初始化场景元素的类型、位置、速度等属性
   * @param viewerPtr 场景查看器的智能指针
   * @param sceneLoaderPtr 场景加载器的智能指针
   * @return 初始化成功返回true
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化车辆元素
   *
   * 初始化车辆元素的类型、位置、速度等属性
   * @param vehId 车辆ID
   * @param veh_init_param 车辆初始化参数
   * @param startLaneId 车辆初始所在的道路ID
   * @param _sceneLoader 场景加载器的智能指针
   * @return 初始化成功返回true，否则返回false
   */
  virtual txBool Initialize(const Base::txSysId vehId,
                            const Base::VehicleInputeAgent::VehicleInitParam_t &veh_init_param,
                            const Base::txLaneID startLaneId, Base::ISceneLoaderPtr _sceneLoader) TX_NOEXCEPT;

  /**
   * @brief 初始化随机动力学
   *
   * 使用随机方法初始化车辆动力学参数
   * @param mStartV 初始速度
   * @param mMaxV 最大速度
   * @param variance 随机方法中使用的方差
   */
  virtual void InitializeRandomKinetics(Base::txFloat mStartV, Base::txFloat mMaxV, Base::txFloat variance) TX_NOEXCEPT;

  /**
   * @brief 初始化随机动力学
   *
   * 使用随机方法初始化车辆动力学参数
   * @param veh_init_param 包含车辆动力学参数的结构体
   */
  virtual void InitializeRandomKinetics(VehicleInitParam_t veh_init_param) TX_NOEXCEPT;

  /**
   * @brief 初始化车辆动力学
   *
   * 使用车辆动力学初始化设定值，如起始速度、最大速度等
   * @param pVehicleSceneLoaderPtr 一个车辆场景加载器的智能指针
   * @return 初始化成功返回 true
   */
  virtual txBool Initialize_Kinetics(Base::ISceneLoader::IVehiclesViewerPtr) TX_NOEXCEPT;

  /**
   * @brief 更新车辆元素信息
   *
   * 更新车辆元素的位置、速度等信息，并返回更新成功与否
   * @param tpMgr 时间参数管理器
   * @return 更新成功返回true
   */
  virtual txBool Update(TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放当前对象
   *
   * @return 释放成功与否
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据给定的时间参数，执行车辆对象的填充操作
   *
   * @param pTimeParamManager 时间参数管理器
   * @param pSimTraffic 模拟的交通信息
   * @return 返回成功或失败的布尔值
   */
  virtual txBool FillingElement(Base::TimeParamManager const &, sim_msg::Traffic &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据给定的时间戳，填充车辆对象。
   *
   * @param timeStamp 当前时间戳（单位：秒）
   * @param pSimVehicle 模拟车辆对象指针
   * @return 指向填充后的车辆对象（Car类型）的指针
   */
  virtual sim_msg::Car *FillingElement(txFloat const timeStamp, sim_msg::Car *pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 填充驾驶路径
   *
   * @param pTimeParamManager 时间参数管理器
   * @param pTrajectory 驾驶路径
   * @return 返回是否填充成功的布尔值
   */
  virtual txBool FillingTrajectory(Base::TimeParamManager const &, sim_msg::Trajectory &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查是否可达
   *
   * @param _laneInfo 交通道路信息
   * @return true 可达
   */
  virtual Base::txBool CheckReachable(const Base::Info_Lane_t &_laneInfo) const TX_NOEXCEPT TX_OVERRIDE {
    return mRouteAI.CheckReachable(_laneInfo);
  }

  /**
   * @brief 填充空间查询信息函数
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PreUpdate 预更新，接受一个时间参数管理器和一个元素ID到车辆动力学信息的映射
   *
   * @param[in] _paramManager 时间参数管理器
   * @param[in/out] map_elemId2Kinetics 元素ID与运动属性关联表
   * @return 处理结果
   */
  virtual txBool Pre_Update(const TimeParamManager &,
                            std::map<Elem_Id_Type, KineticsInfo_t> &map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PostUpdate 更新完成后的处理
   * @param[in] _paramManager 时间参数管理器
   * @return bool
   */
  virtual txBool Post_Update(TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 保存稳定状态
   * @details 当对象的状态已经稳定时，可以调用该方法保存当前状态。
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 判断是否支持指定的行为
   * @param behavior 要判断的行为
   * @return true 支持
   */
  virtual txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 检查是否可以开始驾驶
   * @param timeMgr 时间管理器
   * @return true 可以开始驾驶
   */
  virtual txBool CheckStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 用于检查是否结束
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 返回元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Vehicle; }

  /**
   * @brief 返回车辆类型
   *
   * @return VEHICLE_TYPE
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle_type; }

  /**
   * @brief 返回车辆行为
   *
   * @return VEHICLE_BEHAVIOR
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_BEHAVIOR::eTadAI; }

 public:
  /**
   * @brief 返回字符串表示形式
   *
   * @return txString
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

 public:
  /**
   * @brief 返回车辆移动车道状态
   *
   * @return VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  TX_MARK("IKineticsHandler")
  using KineticsUtilInfo_t = TrafficFlow::IKineticsHandler::KineticsUtilInfo_t;

  /**
   * @brief 获取车辆元素的Kinetics描述
   *
   * @return txString
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_AI_VehicleElement"; }

  /**
   * @brief 获取Kinetics实用信息
   *
   * @return KineticsUtilInfo_t
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算车辆元素的Kinetics属性
   *
   * @param timeMgr 时间管理器
   * @param _objInfo 对象相关信息
   * @return txBool
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const &timeMgr,
                                 const KineticsUtilInfo_t &_objInfo) TX_NOEXCEPT TX_OVERRIDE;

 public:
  TX_MARK("IEnvPerception")
  using Info_NearestObject = Base::Info_NearestObject;
  using Info_NearestDynamic = Base::Info_NearestDynamic;
  using Info_NearestCrossRoadVehicle = Base::Info_NearestCrossRoadVehicle;

  /**
   * @brief 用于配置焦点类型
   *
   */
  virtual void ConfigureFocusTypes() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 执行环境感知
   *
   * @param timeMgr 时间管理器
   */
  virtual void ExecuteEnvPerception(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索周围的车辆
   *
   * @param timeMgr 时间管理器
   */
  virtual void SearchSurroundingVehicle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索周围的车辆（使用 RTree）
   *
   * @param timeMgr 时间管理器
   * @return Info_NearestObject
   */
  virtual Info_NearestObject SearchSurroundingVehicle_RTree(const Base::TimeParamManager &timeMgr)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索周围的车辆（使用高速道路线）
   *
   * @param timeMgr 时间管理器
   * @return Info_NearestObject
   */
  virtual Info_NearestObject SearchSurroundingVehicle_HLane(const Base::TimeParamManager &timeMgr)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索前方车辆
   *
   * @param timeMgr 时间管理器
   */
  virtual void Search_Follow_Front(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索前方车辆（使用 RTree）
   *
   * @param timeMgr 时间管理器
   */
  virtual void Search_Follow_Front_RTree(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 搜索前方车辆 meeting场景（使用Hash路网）
   *
   * @param timeMgr 时间管理器
   */
  virtual void Search_Follow_Front_HLane_Meeting(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 搜索前方车辆（使用Hash路网）
   *
   * @param timeMgr 时间管理器
   */
  virtual void Search_Follow_Front_HLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 搜索周围的行人
   *
   * @param timeMgr 时间管理器
   */
  virtual void SearchSurroundingPedestrian(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索周围的障碍物
   *
   * @param timeMgr 时间管理器
   */
  virtual void SearchSurroundingObstacle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 搜索下一个交叉车辆
   *
   * @param timeMgr 时间管理器
   */
  virtual void SearchNextCrossVehicle(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取前车的当前车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和当前车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_CurrentLane() const TX_NOEXCEPT
      TX_OVERRIDE;

  /**
   * @brief 获取后车的当前车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和当前车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_CurrentLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取前车的目标车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取前车的目标车道信息，根据目标移动状态
   *
   * @param dest_move_state 目标移动状态
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> FrontVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取后车的目标车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取后车的目标车道信息，根据目标移动状态
   *
   * @param dest_move_state 目标移动状态
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RearVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取任意一侧车辆的目标车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> Any_SideVeh_TargetLane() const TX_NOEXCEPT
      TX_OVERRIDE;

  /**
   * @brief 获取同侧车辆的目标车道信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 一个包含模拟一致性指针和目标车道的元组
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取沿着目标车辆的路径信息
   *
   * @param dest_move_state 目标车辆的移动状态
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 包含目标车辆的路径状态和角度
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> SideVeh_TargetLane(
      const Base::Enums::VehicleMoveLaneState dest_move_state) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取左侧车辆的目标道路信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 返回左侧车辆的目标道路及需要转向的角度
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> LeftVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取左侧车辆的目标道路信息
   *
   * @return std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> 返回右侧车辆的目标道路及需要转向的角度
   */
  virtual std::tuple<Base::SimulationConsistencyPtr, Base::txFloat> RightVeh_TargetLane() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取对方侧车辆的下一个可以通过的交叉口信息
   *
   * @return std::tuple<Base::IVehicleElementPtr, Base::txFloat> 返回对方侧车辆的下一个可以通过的交叉口和需要转向的角度
   */
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> OpposeSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取相同侧的车辆的下一个可以通过的交叉口信息
   *
   * @return std::tuple<Base::IVehicleElementPtr, Base::txFloat> 返回相同侧车辆的下一个可以通过的交叉口和需要转向的角度
   */
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> SameSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取左侧车辆的下一个可以通过的交叉口信息
   *
   * @return std::tuple<Base::IVehicleElementPtr, Base::txFloat> 返回左侧车辆下一个可以通过的交叉口和需要转向的角度
   */
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> LeftSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前车辆右侧下一个路口信息
   *
   * @return std::tuple<Base::IVehicleElementPtr, Base::txFloat> 包含路口对象及其与当前车辆之间的距离
   */
  virtual std::tuple<Base::IVehicleElementPtr, Base::txFloat> RightSideVeh_NextCross() const TX_NOEXCEPT TX_OVERRIDE;

  virtual Base::txBool ValidSampling(const Base::TimeParamManager &) const TX_NOEXCEPT TX_OVERRIDE;
#if USE_SUDOKU_GRID

  /**
   * @brief 用于初始化一个表示数独网格的数据结构
   *
   */
  virtual void Initialize_SUDOKU_GRID() TX_NOEXCEPT;

  /**
   * @brief 用于根据指定的方向枚举，获取区域的顺时针相邻的一组
   *
   * @param eValue 指定方向参考
   * @param retRegion
   * 返回结果包含可能有车辆在紧邻的元素区域（包括其他车辆、道路等）的元素的顺时针逼近的顺序，每个元素都用txVec2表示。
   */
  virtual void Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                  std::vector<Base::txVec2> &retRegion) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 用于计算根据给定的车辆位置、车辆所在道路的方向和车辆当前的朝向，获取对应的区域的顺时针相邻的一组
   * @param vehPos 目标的坐标(地理纬度, 地理经度)
   * @param laneDir 目标坐标下的一个车道的方向向量（相对于目标坐标）
   * @param headingDegree 目标坐标下的方向（以度为单位）
   */
  virtual void Compute_Region_Clockwise_Close(const Coord::txENU &vehPos, const Base::txVec3 &laneDir,
                                              const Unit::txDegree &headingDegree) TX_NOEXCEPT TX_OVERRIDE;

#endif /*USE_SUDOKU_GRID*/

 protected:
#if USE_HashedRoadNetwork
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using txSurroundVehiclefo = Base::txSurroundVehiclefo;

  /**
   * @brief 用于搜索旋转列表，并在后向搜索时返回结果
   * @param cmd 搜索指令,包含给定的车道方向向量及搜索范围的边界参数
   * @param res_gap_vehPtr 指向最近侧向附近车辆的指针，返回为nullptr表示找不到符合条件的车辆
   * @return 搜索成功时返回true
   */
  virtual Base::txBool SearchOrthogonalListBack(const IEnvPerception_TAD_AI_Hashed::OrthogonalListSearchCmd &cmd,
                                                txSurroundVehiclefo &res_gap_vehPtr) const TX_NOEXCEPT;

  using VehicleContainer = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::VehicleContainer;
  using AdjoinContainerType = Geometry::SpatialQuery::HashedLaneInfoOrthogonalList::AdjoinContainerType;

#endif /*USE_HashedRoadNetwork*/

  /**
   * @brief 在倾斜状态下移动（只能在道路的斜率为正值时使用）
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool MoveStraightOnS(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 执行道路变更后的操作
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool LaneChangePostOp(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 计算相对于当前速度的横向位移
   *
   * @param timeMgr 时间管理器
   * @return txFloat
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取移动到指定车道的角度
   *
   * @param exper_time 期望时间
   * @return txFloat
   */
  virtual txFloat Get_MoveTo_Sideway_Angle(const txFloat exper_time) TX_NOEXCEPT;

  /**
   * @brief 获取移动到指定车道的角度，考虑车道宽度
   *
   * @param exper_time 期望时间
   * @param laneWidth 车道宽度
   * @return txFloat 返回实际沿着侧向角度移动的时间
   */
  virtual txFloat Get_MoveTo_Sideway_Angle(const txFloat exper_time, const txFloat laneWidth) TX_NOEXCEPT;

  /**
   * @brief 计算撤回时的角度
   *
   * @param time 当前的时间(s)
   * @return txFloat 返回车辆在指定时间内逐渐停止的总距离(m)
   */
  virtual txFloat MoveBack_Sideway_Angle(const txFloat time) TX_NOEXCEPT;

  /**
   * @brief 更新交叉车道运行状态
   * @param timeMgr 时间管理器
   * @return 返回是否正在切换车道运行状态
   */
  virtual txBool UpdateSwitchLaneRunning(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT;

 protected:
  void CheckWayPoint() TX_NOEXCEPT;
  using DrivingFollowType = Base::IDrivingFollower::DrivingFollowType;

  /**
   * @brief 用于控制车辆在跟随道路行驶时的行为
   *
   * @param timeMgr 时间管理器
   * @return txFloat 返回DrivingFollow状态
   */
  virtual txFloat DrivingFollow(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算汽车车队的行为模式（普通模式）
   *
   * @param timeMgr 时间管理器
   * @param frontElementPtr 顶层车辆指针
   * @param frontDist 相对前车的距离
   * @param bHurryToExitState 是否逼近退出状态
   * @return 返回车辆行驶的距离
   */
  virtual txFloat DrivingFollowNormal(const Base::TimeParamManager &timeMgr,
                                      Base::SimulationConsistencyPtr frontElementPtr, const txFloat frontDist,
                                      const Base::txBool bHurryToExitState = false) TX_NOEXCEPT;

  /**
   * @brief 计算汽车车队的行为模式（车道道路模式）
   * @param timeMgr 时间管理器
   * @param frontElementPtr 相对前车的车辆指针
   * @param frontDist 相对前车的距离
   * @param isHurryToExit 是否逼近退出状态
   * @return 返回汽车行驶的距离
   */
  virtual txFloat DrivingFollowArterial(const Base::TimeParamManager &timeMgr,
                                        Base::SimulationConsistencyPtr frontElementPtr,
                                        const txFloat frontDist) TX_NOEXCEPT;

  /**
   * @brief 用于控制车辆在改变车道时的行为
   *
   * @param timeMgr 时间管理器
   * @return txFloat
   */
  virtual txFloat DrivingChangeLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据道路状况，在当前车道和目标车道之间切换车道，同时考虑前后车辆信息以确保行驶安全。
   * @param timeMgr 时间管理器
   * @param FrontVeh_CurrentLane_Base 当前车道的前车辆信息
   * @param RearVeh_CurrentLane_Base 当前车道的后车辆信息
   * @param LeftVeh_TargetLane_Base 目标车道的左侧车辆信息
   * @param RightVeh_TargetLane_Base 目标车道的右侧车辆信息
   * @return 返回切换车道后车辆行驶的距离
   */
  virtual txFloat Driving_SwitchLaneOrg(const Base::TimeParamManager &timeMgr,
                                        Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
                                        Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
                                        Base::SimulationConsistencyPtr LeftVeh_TargetLane_Base,
                                        Base::SimulationConsistencyPtr RightVeh_TargetLane_Base) TX_NOEXCEPT;

  /**
   * @brief 用于控制车辆在行驶到相对位置时的行为
   *
   * @param timeMgr 时间管理器
   * @return txFloat
   */
  virtual txFloat MoveBack_Sideway(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;
  virtual txFloat Get_MoveBack_Sideway(const txFloat exper_time) TX_NOEXCEPT;

  virtual txFloat MoveTo_Sideway(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;
  virtual txFloat Get_MoveTo_Sideway(const txFloat exper_time) TX_NOEXCEPT;

  /**
   * @brief 判断车辆是否处于跟随道路行驶的状态
   *
   * @return txBool
   */
  virtual txBool IsUnderLaneKeep() const TX_NOEXCEPT;

  /**
   * @brief 用于检查车辆是否可以切换到右侧车道
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckCanSwitchRightLaneState(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 用于检查车辆是否可以切换到左侧车道
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckCanSwitchLeftLaneState(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前平均车道改变时间
   *
   * @return txFloat
   */
  virtual txFloat fCurAverage_LaneChanging_Duration() const TX_NOEXCEPT { return FLAGS_Average_LaneChanging_Duration; }

  /**
   * @brief 获取当前方差车道改变时间
   *
   * @return txFloat
   */
  virtual txFloat fCurVariance_LaneChanging_Duration() const TX_NOEXCEPT {
    return FLAGS_Variance_LaneChanging_Duration;
  }

  /**
   * @brief 判断当前车辆是否需要改变车道，并确定改变车道的目标车道，同时考虑前后车辆信息以确保行驶安全。
   * @param FrontVeh_CurrentLane_Base 前车辆当前所在的车道
   * @param RearVeh_CurrentLane_Base 后车辆当前所在的车道
   * @param FrontVeh_TargetLane_Base 前车辆期望的目标车道
   * @param RearVeh_TargetLane_Base 后车辆期望的目标车道
   * @param FrontGap_Current 当前车辆与前车辆的距离
   * @param RearGap_Current 当前车辆与后车辆的距离
   * @param FrontGap_TargetLane 期望的车道中，当前车辆与前车辆的距离
   * @param RearGap_TargetLane 期望的车道中，当前车辆与后车辆的距离
   * @param SideGap_TargetLane 期望的车道中，当前车辆与目标车辆在目标车道两侧的距离
   * @param targetType 需要查找的行人搜索类型
   * @return 包含行走人搜索目标车道、车道变更意图和相应车道距离的结构体
   */
  virtual LaneChangeIntention LaneChanging(Base::SimulationConsistencyPtr FrontVeh_CurrentLane_Base,
                                           Base::SimulationConsistencyPtr RearVeh_CurrentLane_Base,
                                           Base::SimulationConsistencyPtr FrontVeh_TargetLane_Base,
                                           Base::SimulationConsistencyPtr RearVeh_TargetLane_Base,
                                           const txFloat FrontGap_Current, const txFloat RearGap_Current,
                                           txFloat FrontGap_TargetLane, const txFloat RearGap_TargetLane,
                                           const txFloat SideGap_TargetLane,
                                           const PedestrianSearchType targetType) TX_NOEXCEPT;

  /**
   * @brief 用于检查是否可以更改车道
   *
   * @param state 切换车道后是否成功，若成功则返回true
   * @return txBool 切换车道后是否成功，若成功则返回true
   */
  virtual txBool SwitchLane(const PedestrianSearchType state) TX_NOEXCEPT;

  /**
   * @brief 用于在改变车道时计算需要的时间
   *
   * @param laneChangeTime
   * @return txFloat
   */
  virtual txFloat ComputeMinLaneChangeTime(const Base::txFloat laneChangeTime) TX_NOEXCEPT;

  /**
   * @brief 用于检查在指定的车道内是否存在行人
   *
   * @param relative_time
   * @param refLateralVelocity
   * @return txBool
   */
  virtual txBool CheckPedestrianWalking(const txFloat relative_time, txFloat &refLateralVelocity) TX_NOEXCEPT;

  virtual txBool CheckJunctionYieldConflict() TX_NOEXCEPT;

  /**
   * @brief 用于检查在前方行人走路时，前方行人是否在视线范围内
   *
   * @param targetType
   * @param distance
   * @return txFloat
   */
  virtual txFloat FrontPedWalkingInSight(const PedestrianSearchType targetType,
                                         const Base::txFloat distance) TX_NOEXCEPT;

 protected:
  /**
   * @brief 用于更新下一个交叉方向
   *
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 定义一个受保护的用于检查信号灯
   *
   * @param singalId 要检查的信号灯的ID。
   * @param distance 车辆到信号灯的距离。
   * @param lightType 信号灯类型，如红绿灯、隔雨灯、一般灯等。
   * @return txFloat
   */
  virtual txFloat CheckSignLight(const txSysId singalId, const txFloat distance,
                                 const SIGN_LIGHT_COLOR_TYPE lightType) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 定义一个受保护的用于对比驾车是否宜转向，如果驾车需要转向，则返回转向所需的时间
   * 这个方法可以计算逆向行驶的时间。
   *
   * @param pOpposingVeh 对应的交通物体指针。
   * @param distance 车辆与对应交通物体之间的距离。
   * @param acceleration 车辆的加速度。
   * @return 返回计算出的逆向行驶时间。
   */
  virtual txFloat TurnLeft_vs_GoStraight(const Base::IVehicleElementPtr pOpposingVeh, const txFloat distance,
                                         const txFloat acceleration) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 定义一个受保护的用于比较两种情况下的时间：1. 转向左 2. 与左侧前车相比较的时间
   *
   * @param pOpposingVeh 对应的交通物体指针。
   * @param pLeftApproachingVeh 左侧近逼的交通物体指针。
   * @param distance2signal 车辆与待定交通物体之间的距离。
   * @param acceleration 车辆的加速度。
   * @return 返回与待定交通物体的相对方向，可以是左转、右转或保持原来的方向。
   */
  virtual txFloat TurnRight_vs_TurnLeft_GoStraight(const Base::IVehicleElementPtr pOpposingVeh,
                                                   const Base::IVehicleElementPtr pLeftApproachingVeh,
                                                   const txFloat distance2signal,
                                                   const txFloat acceleration) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 定义一个受保护的用于判断指定颜色的信号灯是否存在
   *
   * @param _color 信号灯颜色
   * @return Base::txBool
   */
  virtual Base::txBool IsSignalLightAhead(const Base::Enums::SIGN_LIGHT_COLOR_TYPE _color) const TX_NOEXCEPT;

 protected:
  /**
   * 获取一个随机浮点数，参数strMark为一个用于确定随机数种子的字符串
   * @param strMark 用于确定随机数种子的字符串
   * @return 返回一个随机浮点数
   */
  Base::txFloat Random(const Base::txString &strMark) TX_NOEXCEPT;
  Base::txFloat GetRandomValue_NegOne2PosOne(Base::txString strMark) TX_NOEXCEPT;

 public:
  /**
   * 获取当前的路线组ID
   * @return 返回当前的路线组ID
   */
  virtual Base::txSysId GetCurRouteGroupId() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.RouteId(); }

  /**
   * @brief 获取子路由id
   *
   * @return Base::txInt
   */
  virtual Base::txInt SubRouteId() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.SubRouteId(); }
  virtual Base::txBool IsSupportReRoute() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 对车辆进行路线替换
   * @param routeGroupId 路线组ID
   * @param subRouteId 子路线ID
   * @param newRouteAI 新的路线信息
   * @return true表示替换成功
   */
  virtual Base::txBool ReRoute(const Base::txSysId routeGroupId, const Base::txInt subRouteId,
                               const RouteAI &newRouteAI) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool HasRoute() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.IsValid(); }

 protected:
  /**
   * 更新哈希后的道路信息
   *
   * @param _location 车辆所在的位置
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location &_location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * 注销监听信息
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual void ResetLaneKeep() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 对车辆进行路线替换
   *
   * @param routeGroupId 路线组ID
   * @param subRouteId 子路线ID
   * @param newRouteAI 新的路线信息
   * @return true表示替换成功，false表示替换失败
   */
  virtual hadmap::txLaneLinkPtr RandomNextLink(const hadmap::txLaneLinks &nextLaneLinks) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断当前车辆是否处于稳定的车道保持状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsStableLaneKeep() const TX_NOEXCEPT TX_OVERRIDE { return IsLaneKeep(LastFSMState()); }

  /**
   * @brief IsNotStableTurnLeft 判断车辆是否处于非稳定转向左状态
   *
   * @return txBool 如果车辆处于非稳定转向左状态返回 true，否则返回 false
   */
  virtual txBool IsNotStableTurnLeft() const TX_NOEXCEPT TX_OVERRIDE { return CallFail(IsInTurnLeft(LastFSMState())); }

  /**
   * @brief IsNotStableTurnRight 判断车辆是否处于非稳定转向右状态
   *
   * @return txBool 如果车辆处于非稳定转向右状态返回 true，否则返回 false
   */
  virtual txBool IsNotStableTurnRight() const TX_NOEXCEPT TX_OVERRIDE {
    return CallFail(IsInTurnRight(LastFSMState()));
  }

  /**
   * @brief 检查道路前方是否阻塞
   * @return true 表示有阻塞，false 表示没有阻塞
   */
  virtual Base::txBool BlockAhead() TX_NOEXCEPT;

 public:
  /**
   * @brief 获取当前车辆路径中间点的GPS坐标
   * @return 当前车辆路径中间点的GPS坐标向量
   */
  virtual std::vector<hadmap::txPoint> RouteMidGPS() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.MidPts(); }

  /**
   * @brief 获取车辆当前路线的终点的经纬度坐标
   * @return 返回一个hadmap::txPoint类型的对象，表示路线终点的经纬度坐标
   */
  virtual hadmap::txPoint RouteEndGPS() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.EndPt(); }

 public:
  /**
   * @brief 返回当前车辆的局部坐标系下的x轴坐标
   * @return 返回一个Base::txFloat类型的值，表示当前车辆的局部坐标系下的x轴坐标
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_horizontal();
  }

  /**
   * @brief 返回当前车辆的局部坐标系下的y轴坐标
   * @return 返回一个Base::txFloat类型的值，表示当前车辆的局部坐标系下的y轴坐标
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_vertical();
  }

  /**
   * @brief 返回当前车辆的局部坐标系下的z轴坐标
   *
   * @return Base::txFloat 类型的值，表示当前车辆的局部坐标系下的z轴坐标
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE {
    return show_relative_dist_elevation();
  }

 public:
  /**
   * @brief 重新定位车辆跟踪器
   *
   * @param pLane 指向目标车道的指针
   * @param _timestamp 当前时间戳
   * @return 返回一个Base::txBool类型的值，表示重新定位是否成功
   */
  virtual Base::txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat &_timestamp) TX_NOEXCEPT TX_OVERRIDE;
  // virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat &_timestamp) TX_NOEXCEPT
  // TX_OVERRIDE { return ParentClass::RelocateTracker(pLaneLink, _timestamp); }

  /**
   * @brief 重新定位车辆跟踪器
   * @param pLaneLink 指向目标车道连接的指针
   * @param _timestamp 当前时间戳
   * @return 返回一个Base::txBool类型的值，表示重新定位是否成功
   */
  virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink,
                                       const txFloat &_timestamp) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 更新车辆的车道保持状态
   * @param timeMgr 时间管理器
   */
  virtual void UpdateLaneKeep(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 初始化行为模块
   * @param _beh_ptr 行为模块指针
   */
  virtual void InitializeBehavior(Base::ISceneLoader::ITrafficFlowViewer::BehPtr _beh_ptr) TX_NOEXCEPT;

  /**
   * @brief 计算车辆信号反应间隔
   * @return 车辆信号反应间隔
   */
  virtual Base::txFloat SignalReactionGap() const TX_NOEXCEPT;

#if __JunctionYield__
 public:
  virtual txBool UpdateTrajectory(Base::TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  virtual txBool MakeJuntionTrajectory(Base::TimeParamManager const &, std::vector<Base::txVec3> &) TX_NOEXCEPT;
#endif /*__JunctionYield__*/

#if __TX_Mark__("state machine interface")
 public:
  /**
   * 获取是否左转
   * @return 是否左转
   */
  virtual Base::txBool IsTurnLeft() const TX_NOEXCEPT TX_OVERRIDE { return IsInTurnLeft(); }

  /**
   * @brief 获取是否右转
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsTurnRight() const TX_NOEXCEPT TX_OVERRIDE { return IsInTurnRight(); }

  /**
   * @brief 获取是否直行
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsTurnStraight() const TX_NOEXCEPT TX_OVERRIDE { return IsLaneKeep(); }

  /**
   * @brief 获取是否变道
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneChange() const TX_NOEXCEPT TX_OVERRIDE { return IsInLaneChange(); }
#endif /*state machine interface*/
 protected:
  /**
   * @brief 计算曲率对应的速度变化量
   * @param curv 当前曲率
   * @param cur_acc 当前加速度
   * @return 返回一个Base::txFloat类型的值，表示速度变化量
   */
  virtual Base::txFloat Curvature2decc(const Base::txFloat curv, const Base::txFloat cur_acc) const TX_NOEXCEPT;

 protected:
  std::set<Base::Enums::ElementType> m_vehicle_type_filter_set;
  std::set<Base::Enums::ElementType> m_pedestrian_type_filter_set;
  Base::txFloat m_stop_on_junction_second = 0.0;
  Base::txFloat m_stop_second = 0.0;

  /*Unit::txDegree heading_for_front_region_on_ENU;*/
  Scene::VehicleScanRegion2D mVehicleScanRegion2D;
  Info_NearestObject m_NearestObject;
  Info_NearestDynamic m_NearestDynamic;
  Info_NearestCrossRoadVehicle m_NearestCrossRoadVehicle;
  txSurroundVehiclefo m_YieldConflict;

  TAD_BehaviorTimeManager m_LaneAbortingTimeMgr;
  TAD_BehaviorTimeManager m_LaneChangeTimeMgr;
  ExitHighWayType m_Need_To_Exit = ExitHighWayType::eFollower;
  Base::txFloat m_DistanceToExit = FLT_MAX;

 public:
  /**
   * 获取当前对象的二进制存档.
   *
   * @return 当前对象的二进制存档的字符串表示
   */
  virtual txString binary_archive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前对象的XML存档.
   *
   * @return txString
   */
  virtual txString xml_archive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前对象的JSON存档.
   *
   * @return txString
   */
  virtual txString json_archive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 对象序列化的模板函数
   *
   * @tparam Archive 类型
   * @param archive 对象引用
   */
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("TAD_AI_VehicleElement");
    archive(cereal::base_class<Base::IVehicleElement>(this));
    archive(cereal::base_class<Base::IEnvPerception_TAD_AI_Hashed>(this));
    archive(cereal::base_class<SM::txAITrafficState>(this));
    /*archive(cereal::base_class<TrafficFlow::IKineticsHandler>(this));*/
    archive(cereal::base_class<Base::IDrivingFollower>(this));
    archive(cereal::base_class<Base::IDrivingChangeLane>(this));
    /*archive(cereal::base_class<Base::ITrafficRules>(this));*/
    /*archive(cereal::base_class<IReRoute>(this));*/
    /*archive(cereal::base_class<Base::ITrafficJunctionRules>(this));*/
    archive(cereal::base_class<TrafficFlow::txVehicleBehavior>(this));
    archive(cereal::base_class<TrafficFlow::TAD_PlanningModule>(this));

    archive(_MAKE_NVP_("vehicle_type_filter_set", m_vehicle_type_filter_set));
    archive(_MAKE_NVP_("pedestrian_type_filter_set", m_pedestrian_type_filter_set));
    archive(_MAKE_NVP_("stop_on_junction_second", m_stop_on_junction_second));
    archive(_MAKE_NVP_("stop_second", m_stop_second));
    archive(_MAKE_NVP_("VehicleScanRegion2D", mVehicleScanRegion2D));
    archive(_MAKE_NVP_("LaneAbortingTimeMgr", m_LaneAbortingTimeMgr));
    archive(_MAKE_NVP_("LaneChangeTimeMgr", m_LaneChangeTimeMgr));

    archive(_MAKE_NVP_("Need_To_Exit", m_Need_To_Exit));
    archive(_MAKE_NVP_("DistanceToExit", m_DistanceToExit));

    /*archive(_MAKE_NVP_("FCW", mFCW));*/
  }
};

using TAD_AI_VehicleElementPtr = std::shared_ptr<TAD_AI_VehicleElement>;

#ifdef InjectMode

class InjectModeVehicleElement : public TAD_AI_VehicleElement {
  using ParentClass = TAD_AI_VehicleElement;

 public:
  InjectModeVehicleElement();
  virtual ~InjectModeVehicleElement() TX_DEFAULT;
  virtual txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat DrivingFollow(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat DrivingChangeLane(const Base::TimeParamManager &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  std::shared_ptr<InjectImpl> pImpl;
};
using InjectModeVehicleElementPtr = std::shared_ptr<InjectModeVehicleElement>;
#endif /*InjectModeVehicleElement*/

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef SerializeInfo

CEREAL_REGISTER_TYPE(TrafficFlow::TAD_AI_VehicleElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IVehicleElement, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IEnvPerception_TAD_AI_Hashed, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::SM::txAITrafficState, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::IKineticsHandler, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IDrivingFollower, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IDrivingChangeLane, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ITrafficRules, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::IReRoute, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ITrafficJunctionRules, TrafficFlow::TAD_AI_VehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::TAD_PlanningModule, TrafficFlow::TAD_AI_VehicleElement);
