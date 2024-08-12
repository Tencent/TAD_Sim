// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_component.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_vehicle_kinetics_info.h"
#include "traffic.pb.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_vehicle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_RelativeTrajectoryFollowElement : public Base::IVehicleElement,
                                            public TAD_SceneEvent_1_0_0_0,
                                            public TrafficFlow::IKineticsHandler {
 public:
  using txString = Base::txString;
  using ParentClass = Base::IVehicleElement;
  enum class ObstacleBehaviorType : Base::txInt { Unknown = 0, eRelative = 1, eAbsolute = 2 };
  struct RelativeActionItemInfo {
    /**
     * @brief 结构体初始化
     *
     * @param[in] kv_map 参数键值对，用于配置结构体属性
     * @return 返回 true 表示初始化成功，false 表示失败
     */
    txBool Initialize(const std::map<txString, txString>& kv_map) TX_NOEXCEPT;

    /**
     * @brief 返回当前结构体的字符串表示
     *
     * @return 返回当前结构体的字符串表示
     */
    txString Str() const TX_NOEXCEPT;
    Base::txVec3 dStep; /*dx,dy,dz*/
    Base::txFloat time;
    Base::txSysId elemId;
    ElementType elemType;
  };
  using RelativeActionItemInfoVec = std::vector<RelativeActionItemInfo>;

  struct AbsoluteActionItemInfo {
    /**
     * @brief 结构体初始化
     *
     * @param[in] kv_map 参数键值对，用于配置结构体属性
     * @return 返回 true 表示初始化成功，false 表示失败
     */
    txBool Initialize(const std::map<txString, txString>& kv_map) TX_NOEXCEPT;

    /**
     * @brief 返回当前结构体的字符串表示
     *
     * @return 返回当前结构体的字符串表示
     */
    txString Str() const TX_NOEXCEPT;
    hadmap::txPoint gps;
    Base::txFloat height;
    Base::txFloat time;
  };
  using AbsoluteActionItemInfoVec = std::vector<AbsoluteActionItemInfo>;

 public:
  TAD_RelativeTrajectoryFollowElement() TX_DEFAULT;
  virtual ~TAD_RelativeTrajectoryFollowElement() TX_DEFAULT;

 public:
  /**
   * @brief 初始化跟随相对轨迹元素
   *
   * @param viewer 指向Viewer对象的指针
   * @param loader 指向SceneLoader对象的指针
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新相对轨迹元素
   * 更新相对轨迹元素的状态和位置
   * @param timeParamManager 时间参数管理器对象
   * @return 更新成功返回 true，否则返回 false
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新相对轨迹元素
   *
   * @param timeParamManager 时间参数管理器对象
   * @param followElemPtr 需要跟随的元素
   * @return 更新成功返回 true，否则返回 false
   */
  virtual txBool Update(TimeParamManager const&, Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT;

  /**
   * @brief 资源释放
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * 获取指定元素的traffic信息。
   *
   * 该函数用于将指定元素的相对轨迹信息填充到交通管理系统的输入中，以便进行交通管理。
   *
   * @param timeParamManager 时间参数管理器对象，用于获取相关时间参数。
   * @param traffic 交通对象的引用，用于存储填充后的相对轨迹信息。
   * @return 如果填充成功，则返回 true；否则返回 false。
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * 填充指定元素的Car信息。
   *
   * 该函数用于将指定元素的相对轨迹信息填充到交通管理系统的输入中，以便进行交通管理。
   *
   * @param timeStamp 当前时间戳，单位为秒。
   * @param pSimVehicle 交通对象的指针，用于存储填充后的Car信息。
   * @return 如果填充成功，则返回指向填充后的交通对象的指针；否则返回 nullptr。
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新空间查询
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief PreUpdate 函数，该函数用于在执行 Kinetics 更新之前进行相关操作。
   * 该函数可以在 Update 函数之前进行调用，并将 PreUpdate 函数的执行结果返回。
   * @param[in] TimeParamManager 时间参数管理器。
   * @param[in,out] map_elemId2Kinetics 一个映射表，用于存储元素 ID 和其对应的 Kinetics 信息。
   * @return true 表示函数执行成功；false 表示函数执行失败。
   */
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 结束 Update 操作后执行的处理逻辑
   * @param[in] TimeParamManager 时间参数管理器
   * @return 如果函数执行成功，则返回 true；否则，返回 false
   * @details 该函数将在 Update 操作之后执行，用于进行相关的后续处理。
   */
  virtual txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 保存稳定状态
   * @details 在启动执行轨迹跟随模块之前，需要保存该模块的稳定状态。
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 判断当前元素是否支持指定的行为
   *
   * @param behavior 要检查的行为
   * @return txBool 是否支持
   */
  virtual txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE {
    return _plus_(VEHICLE_BEHAVIOR::eRelativeTrajectoryFollow) == behavior;
  }

 public:
  /**
   * @brief 检查元素生命周期是否开始
   *
   * @param timeMgr 时间管理器对象
   * @return txBool 开始则true
   */
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查元素是否活跃
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取元素的类型
   *
   * @return ElementType 元素类型
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Vehicle; }

  /**
   * @brief 获取车辆的具体类型
   *
   * @return VEHICLE_TYPE
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return _plus_(VEHICLE_TYPE::RelativeObstacle); }

  /**
   * @brief 获取当前行为类型
   * @return 当前行为类型
   * @details 返回当前行为为 eRelativeTrajectoryFollow
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE {
    return VEHICLE_BEHAVIOR::eRelativeTrajectoryFollow;
  }

 public:
  /**
   * @brief 计算横向距离
   *
   * @param timeMgr 时间管理器
   * @return txFloat
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }

  /**
   * @brief 获取车辆切换车道的状态
   *
   * @return VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return _plus_(VehicleMoveLaneState::eStraight);
  }

  /**
   * @brief 获取元素对象描述
   *
   * @return txString
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_RelativeTrajectoryFollowElement"; }

  /**
   * @brief 更新下个路口方向
   *
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE { return; }

  /**
   * @brief 返回元素的动力学描述
   *
   * @return txString
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_RelativeTrajectoryFollowElement"; }

  /**
   * @brief 获取KineticsUtilInfo_t
   *
   * @return KineticsUtilInfo_t
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_self_KineticsUtilInfo; }

  /**
   * @brief 计算相对轨迹追踪元素的动力学信息
   * @param timeMgr Base::TimeParamManager 类型的参数管理器，用于获取计算时间
   * @param _objInfo KineticsUtilInfo_t 类型的对象，包含元素相关信息
   * @return txBool 类型，表示是否成功计算动力学信息
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                 const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT TX_OVERRIDE {
    return true;
  }

 public:
  /**
   * @brief 获取相对reference的元素信息
   *
   * @return std::tuple<Base::Enums::ElementType, Base::txSysId> 元素类型和对应的sysid
   */
  virtual std::tuple<Base::Enums::ElementType, Base::txSysId> RelativeReferenceElementInfo() const TX_NOEXCEPT;

  /**
   * @brief 根据相对obs还是绝对obs设置开始时间
   *
   * @param timeMgr 时间管理器
   * @return txFloat 返回生命周期的开始时间
   */
  virtual txFloat SetStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

 public:
  /**
   * @brief HandlerEvent 处理观察者事件并返回处理结果
   *
   * @param timeMgr 时间参数管理器
   * @param _eventPtr 场景事件观察者指针
   * @return TxBool 处理结果
   */
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 是否使用相对障碍物行为
   *
   * @return Base::txBool
   */
  Base::txBool IsRelativeObs() const TX_NOEXCEPT { return ObstacleBehaviorType::eRelative == mObstacleBehaviorType; }

  /**
   * @brief 是否使用绝对障碍物行为
   *
   * @return Base::txBool
   */
  Base::txBool IsAbsoluteObs() const TX_NOEXCEPT { return ObstacleBehaviorType::eAbsolute == mObstacleBehaviorType; }

 protected:
  /**
   * @brief 获取障碍物行为类型
   *
   * @return ObstacleBehaviorType
   */
  ObstacleBehaviorType ObsBehaviorType() const TX_NOEXCEPT { return mObstacleBehaviorType; }

  /**
   * @brief 更新相对障碍物行为
   *
   * @param timeMgr 时间参数管理器
   * @param relativePassTime 相对障碍物行为的持续时间
   * @param followElemPtr 被跟随的障碍物元素指针
   * @return txBool 如果成功更新相对障碍物行为则返回true，否则返回false
   */
  virtual txBool UpdateRelative(TimeParamManager const&, const Base::txFloat relativePassTime,
                                Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT;

  /**
   * @brief UpdateAbsolute 更新相对障碍物行为，使其在相对障碍物行为的持续时间内紧跟着其他障碍物
   *
   * @param[in] timeMgr 时间参数管理器
   * @param[in] relativePassTime 相对障碍物行为的持续时间
   * @param[in] followElemPtr 被跟随的障碍物元素指针
   * @return txBool 如果成功更新相对障碍物行为则返回true，否则返回false
   */
  virtual txBool UpdateAbsolute(TimeParamManager const&, const Base::txFloat relativePassTime,
                                Base::ITrafficElementPtr followElemPtr) TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取笛卡尔坐标系x
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取笛卡尔坐标系y
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取笛卡尔坐标系z
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

 protected:
  RelativeActionItemInfoVec m_relative_action_vec;
  AbsoluteActionItemInfoVec m_absolute_action_vec;
  Base::txVec3 m_cur_step;
  Base::txFloat m_event_start_passTime = 0.0;
  Base::Enums::STATIC_ELEMENT_TYPE mObstacleType = _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box);
  Base::txString mObstacleTypeStr = "Box";
  Base::txInt mObstacleTypeId = 1;
  ObstacleBehaviorType mObstacleBehaviorType = ObstacleBehaviorType::Unknown;
};

using TAD_RelativeTrajectoryFollowElementPtr = std::shared_ptr<TAD_RelativeTrajectoryFollowElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
