// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_pedestrian_trajectory.h"
#include "tad_scene_event_1_0_0_0.h"
#include "tad_vehicle_kinetics_info.h"
#include "tx_event_handler.h"
#include "tx_header.h"
#include "tx_pedestrian_element.h"
#include "tx_scene_loader.h"
#include "tx_sim_point.h"
#include "tx_vehicle_coord.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_PedestrianElement : public Base::IPedestrianElement,
                              public TrafficFlow::IEventHandler,
                              public TAD_SceneEvent_1_0_0_0,
                              public Base::VehicleCoord,
                              public TrafficFlow::IKineticsHandler {
 public:
  TAD_PedestrianElement() TX_DEFAULT;
  virtual ~TAD_PedestrianElement() TX_DEFAULT;

  /**
   * @brief 初始化函数
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 行人元素更新 用于更新行人状态
   *
   * @param timeMgr
   * @return Base::txBool
   */
  virtual Base::txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 元素资源释放
   *
   * @return Base::txBool
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief FillingElement 每次仿真后来获取当前元素的traffic信息
   *
   * @param[in] TimeParamManager const& paramMgr 时间对象
   * @param[out] sim_msg::Traffic& traffic 交通消息，包含交通流量数据和行人行为数据等
   * @return txBool 是否填充成功
   * @retval true 成功
   * @retval false 失败
   */
  virtual Base::txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新查询空间
   *
   * @return Base::txBool
   */
  virtual Base::txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取行人的动力学信息
   *
   * @param timeMgr
   * @return KineticsInfo_t
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化动力学
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize_Kinetics(Base::ISceneLoader::IPedestriansViewerPtr) TX_NOEXCEPT;

  /**
   * @brief 初始化位置
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize_Location() TX_NOEXCEPT;

  /**
   * @brief 获取元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Pedestrian; }

  /**
   * @brief 检查元素生命周期是否开始
   *
   * @param timeMgr
   * @return Base::txBool
   */
  virtual Base::txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查元素生命周期是否结束
   *
   * @param timeMgr
   * @return Base::txBool
   */
  virtual Base::txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 生命周期处于start
   *
   * @param timeMgr
   */
  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 生命周期处于end
   *
   */
  virtual void OnEnd() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 获取目标方向
   *
   * @return Base::txVec3
   */
  virtual Base::txVec3 GetTargetDir() const TX_NOEXCEPT TX_OVERRIDE { return m_vTargetDir; }
  virtual std::vector<Coord::txWGS84> GetRawWayPoints() const TX_NOEXCEPT { return m_polyline_traj.GetRawWayPoints(); }

 public:
  /**
   * @brief Base类的成员函数 计算侧向位移
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }

  /**
   * @brief 获取切换车道状态
   *
   * @return VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return VehicleMoveLaneState::eStraight;
  }

 public:
  /**
   * @brief 方向是否沿着道路
   *
   * @param _direction 一个整数参数_direction
   * @return Base::txBool 如果方向沿着道路，则返回true，否则返回false
   */
  Base::txBool IsAlongTheRoad(const Base::txInt _direction) const TX_NOEXCEPT;

  /**
   * @brief 方向是否不沿着道路
   *
   * @param _direction 一个整数参数_direction
   * @return Base::txBool 如果方向不沿着道路，则返回true，否则返回false
   */
  Base::txBool IsNotAlongTheRoad(const Base::txInt _direction) const TX_NOEXCEPT;

  /**
   * @brief 获取行人元素的运动方向
   *
   * 这个函数返回一个整数，表示行人元素的运动方向。
   *
   * @return 返回行人元素的运动方向，单位为整数
   */
  virtual Base::txInt GetDirection() const TX_NOEXCEPT { return mKinetics.m_direction; }

 protected:
  /**
   * @brief 根据给定的参数生成行人的路径
   *
   * 此函数根据给定的起点、途经点和终点，生成行人的行走路径。
   *
   * @param[in] startPos    起点的WGS84坐标
   * @param[in] midPoint    途经点的坐标列表，每个点由经度和纬度组成
   * @param[in] refEndPoint 终点的坐标，以WGS84坐标表示
   *
   * @return 无返回值
   */
  void GenerateWayPoints(const Coord::txWGS84& startPos,
                         const std::vector<std::pair<Base::txFloat, Base::txFloat> >& midPoint,
                         const std::pair<Base::txFloat, Base::txFloat>& refEndPoint) TX_NOEXCEPT;

 public:
#if __TX_Mark__("Base::IEventHandler")
  /**
   * @brief 获取事件处理器类型
   *
   * 返回表示事件处理器类型的枚举值。
   *
   * @return 事件处理器类型的枚举值
   */
  virtual EventHandlerType GetEventHandlerType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取事件处理器描述
   *
   * 返回表示事件处理器的描述信息。
   *
   * @return 事件处理器描述字符串
   */
  virtual txString HandlerDesc() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 处理事件
   *
   * 该函数用于处理驾驶员相关的事件。
   *
   * @param[in] event 事件信息，用于描述事件的相关属性
   * @return true 事件处理成功
   * @return false 事件处理失败
   */
  virtual txBool HandlerEvent(TrafficFlow::IEvent&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取行人视图指针
   * @return 行人视图指针，具体类型为 Base::ISceneLoader::IPedestriansViewerPtr
   */
  virtual Base::ISceneLoader::IPedestriansViewerPtr getPedestriansViewPtr() const TX_NOEXCEPT;

  /**
   * @brief 获取行人事件视图指针
   * @return Base::ISceneLoader::IPedestriansEventViewerPtr 行人事件视图指针
   */
  virtual Base::ISceneLoader::IPedestriansEventViewerPtr getPedestriansEventViewPtr() const TX_NOEXCEPT;

  /**
   * @brief 获取行人速度与时间关系视图指针
   * @return 行人速度与时间关系视图指针，具体类型为 Base::ISceneLoader::IPedestriansEventViewerPtr
   */
  virtual Base::ISceneLoader::IPedestriansEventViewerPtr getPedestriansTimeVelocityViewPtr() const TX_NOEXCEPT;

  /**
   * @brief 获取行人事件速度视图指针
   *
   * @return Base::ISceneLoader::IPedestriansEventViewerPtr
   */
  virtual Base::ISceneLoader::IPedestriansEventViewerPtr getPedestriansEventVelocityViewPtr() const TX_NOEXCEPT;

  /**
   * @brief 获取行人碰撞事件实用信息
   * @return 行人碰撞事件实用信息
   */
  virtual EventHitUtilInfo_t GetEventHitUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取处理器id
   *
   * @return txSysId 处理器的id
   */
  virtual txSysId HanderId() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取处理器的系统id
   *
   * @return txSysId
   */
  virtual txSysId HanderSysId() const TX_NOEXCEPT TX_OVERRIDE;

#endif  // __TX_Mark__("Base::IEventHandler")
#if __TX_Mark__("ISimulationConsistency")

 public:
  // Declare the virtual functions in the class
  // These functions provide access to specific information about the object

  /**
   * @brief get the Consistency ID
   *
   * @return txSysId
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取稳定的几何中心坐标
   * @return 稳定的几何中心坐标
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastGeomCenter(); }

  /**
   * @brief 获取稳定的后车辆车辆中心坐标
   * @return 稳定的后车辆车辆中心坐标
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastRearAxleCenter(); }

  /**
   * @brief 获取稳定在道路上的行人的WGS84坐标
   * @return 稳定在道路上的行人的WGS84坐标
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.PosOnLaneCenterLinePos();
  }

  /**
   * @brief 获取稳定在道路上的行人的行驶车道信息
   * @return 稳定在道路上的行人的行驶车道信息
   */
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取稳定在道路上的行人的方向信息
   * @return 稳定在道路上的行人的方向信息
   */
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取行人的head角
   *
   * @return Unit::txDegree 度
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE {
    return Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());
  }

  /**
   * @brief 获取稳定在道路上的行人的稳定速度
   * @return 稳定在道路上的行人的稳定速度
   * @details 此函数返回一个表示稳定速度的浮点值，该值表示行人在当前车道中的平均速度。
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return GetVelocity(); }

  /**
   * @brief 获取稳定的行人加速度
   * @return 行人加速度
   * @details 返回一个表示稳定行人加速度的浮点数，该加速度在当前车道中平均值。
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return GetAcc(); }

  /**
   * @brief 获取稳定的行人在曲线上的距离的倒数
   * @return 稳定的行人在曲线上的距离的倒数，单位为米/秒²
   * @details 返回一个表示稳定行人在曲线上的距离的倒数的浮点数，该距离在当前车道中平均值。
   */
  virtual txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastInvertDistanceAlongCurve();
  }

  /**
   * @brief 获取稳定的行人在曲线上的距离
   * @return 稳定的行人在曲线上的距离，单位为米
   * @details 返回一个表示稳定行人在曲线上的距离的浮点数，该距离在当前车道中平均值。
   */
  virtual txFloat StableDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastDistanceAlongCurve();
  }

  /**
   * @brief 保存稳定状态
   *
   * 保存当前行人元素的稳定状态，包括位置、速度、方向等。
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief get the stable hashed lane information of the object
   *
   * @return const HashedLaneInfo&
   */
  virtual const HashedLaneInfo& StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_curHashedLaneInfo; }
#endif /*__TX_Mark__("ISimulationConsistency")*/

#if __TX_Mark__("Log.Debug")
  virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
  friend std::ostream& operator<<(std::ostream& os, TAD_PedestrianElement const& v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }
#endif  // #if __TX_Mark__("Log.Debug")
#if USE_HashedRoadNetwork

 protected:
  /**
   * @brief 注册初始化信息
   *
   * 该函数用于在初始化过程中注册相关信息，将当前对象与给定的位置关联起来。
   *
   * @param _location 给定的位置
   */
  virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 更新哈希的车道信息
   *
   * 该函数用于在更新道路信息时使用哈希算法，将当前对象与给定的位置关联起来。
   *
   * @param _location 给定的位置
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief kill时注销信息
   *
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}
#endif /*USE_HashedRoadNetwork*/

 public:
  // The following methods are part of a class that implements a handler for scene events

  /**
   * @brief 处理场景事件，返回事件处理结果
   *
   * 此函数处理传入的场景事件，并根据事件类型进行处理，处理结果将根据不同的事件类型返回 true 或 false。
   * @param timeMgr 时间管理器，包含当前时间、时间步长等信息
   * @param _eventPtr 事件指针，包含当前需要处理的事件信息
   * @return Base::txBool 事件处理结果，返回 true 表示事件处理成功，返回 false 表示事件处理失败
   */
  virtual Base::txBool HandlerEvent(Base::TimeParamManager const& timeMgr,
                                    ISceneEventViewerPtr _eventPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新行人时的前置操作
   *
   * @param timeMgr 事件参数管理器
   * @return Base::txBool 前置更新成功返回true
   */
  virtual Base::txBool Pre_UpdatePedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新动力学
   *
   * @param timeMgr 事件参数管理器
   * @return txBool 更新成功返回true
   */
  virtual txBool Update_Kinetics(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取对象是否是透明的
   *
   * 该函数返回一个布尔值，表示该对象是否是透明的。透明对象具有一定的特殊性，比如文字，需要
   * 在绘制其他对象时使用特殊的渲染方式，避免出现绘制错误。
   *
   * @return true 如果对象是透明的，否则返回 false
   */
  virtual Base::txBool IsTransparentObject() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定类型的行人对象是否为透明对象
   *
   * 根据传入的行人类型参数，判断该行人对象是否为透明对象。透明对象具有特殊的渲染属性，需要特别处理，以避免绘制错误。
   *
   * @param _ped_type 指定的行人类型
   * @return true 如果该行人对象是透明对象，否则返回 false
   */
  static Base::txBool IsTransparentObject(const Base::Enums::PEDESTRIAN_TYPE& _ped_type) TX_NOEXCEPT;
  static Base::txBool IsTransparentObject(const Base::txString& _ped_type) TX_NOEXCEPT;
#if __TX_Mark__("VehicleCoord")
  // VehicleCoord class declaration

 public:
  /**
   * @brief 位置同步
   *
   * @param _passTime 期望同步的时间
   * @return txBool 如果行人元素在给定时间内成功同步到一致位置，则返回 true。否则，返回 false。
   */
  virtual txBool SyncPosition(const txFloat _passTime) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取行人元素的几何中心
   *
   * 此函数返回行人元素的几何中心点的WGS84坐标系，以经度和纬度表示。
   *
   * @return mLocation.GeomCenter() 返回行人元素的几何中心点的WGS84坐标系，以经度和纬度表示。
   */
  virtual Coord::txWGS84& GeomCenter() TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }
  virtual const Coord::txWGS84& GeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }

  /**
   * @brief 获取行人元素后中心点的WGS84坐标
   *
   * 该函数返回行人元素后中心点的WGS84坐标，即行人元素沿着行人元素方向行驶一定距离的位置。
   *
   * @return txWGS84& 行人元素后中心点的WGS84坐标
   */
  virtual Coord::txWGS84& RearAxleCenter() TX_NOEXCEPT TX_OVERRIDE { return mLocation.RearAxleCenter(); }

  /**
   * @brief 获取行人元素后中心点的WGS84坐标
   *
   * 该函数返回行人元素后中心点的WGS84坐标，即行人元素沿着行人元素方向行驶一定距离的位置。
   *
   * @return const Coord::txWGS84& 行人元素后中心点的WGS84坐标
   */
  virtual const Coord::txWGS84& RearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.RearAxleCenter(); }

 protected:
  /**
   * @brief 获取行人元素类型
   *
   * 返回行人元素的类型。
   *
   * @return 返回行人元素的类型
   */
  virtual Base::Enums::ElementType VehicleCoordElementType() const TX_NOEXCEPT TX_OVERRIDE { return Type(); }

  /**
   * @brief 获取行人元素行为
   *
   * 返回行人元素的行为类型。
   *
   * @return 行人元素的行为类型
   */
  virtual Base::Enums::VEHICLE_BEHAVIOR VehicleCoordElementBehavior() const TX_NOEXCEPT TX_OVERRIDE {
    return _plus_(Base::Enums::VEHICLE_BEHAVIOR::undefined);
  }
#endif /*VehicleCoord*/

 public:
  /**
   * @brief 计算行人元素与目标点之间的Frenet坐标
   *
   * 根据给定的ENU坐标点（target_element_geom_center），
   * 计算该点与该行人元素相对应的Frenet坐标信息。
   *
   * @param[in] target_element_geom_center 目标点的ENU坐标
   * @return FrenetProjInfo 包含Frenet坐标信息的结构体
   */
  virtual FrenetProjInfo ComputeRoadProjectDistance(const Coord::txENU& target_element_geom_center) const TX_NOEXCEPT
      TX_OVERRIDE;

 public:
  TX_MARK("IKineticsHandler")
#if USE_VehicleKinectInfo
  using KineticsUtilInfo_t = TrafficFlow::IKineticsHandler::KineticsUtilInfo_t;

  /**
   * @brief 返回行人元素的动力学描述信息
   *
   * 该函数返回一个包含行人元素动力学描述信息的字符串。
   *
   * @return 行人元素动力学描述信息的字符串
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_PedestrianElement"; }

  /**
   * @brief 获取行人元素的动力学信息
   *
   * 该函数返回一个包含行人元素动力学信息的结构体。
   *
   * @return 行人元素动力学信息的结构体
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 计算行人元素的动力学信息
   *
   * 该函数接收一个包含行人元素动力学信息的结构体，并计算并返回同样结构体类型的数据。
   *
   * @param[in] timeMgr 时间参数管理器
   * @param[in] _objInfo 行人元素动力学信息结构体
   * @return 返回计算后的行人元素动力学信息结构体
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                 const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_VehicleKinectInfo*/

 protected:
  virtual Base::txFloat ComputeHeading(const txFloat _passTime) TX_NOEXCEPT;

 protected:
  enum class PedestrianBehavior : Base::txInt { WayPoints, Direction };
  PedestrianBehavior m_pedestrian_behavior = PedestrianBehavior::Direction;
  Base::ISceneLoader::IPedestriansViewerPtr _elemAttrViewPtr = nullptr;
  Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = nullptr;
  Base::ISceneLoader::IPedestriansEventViewerPtr _elemPedestriansEventViewerPtr = nullptr;
  Base::ISceneLoader::IPedestriansEventViewerPtr _elemPedestriansEvent_time_velocity_ViewerPtr = nullptr;
  Base::ISceneLoader::IPedestriansEventViewerPtr _elemPedestriansEvent_event_velocity_ViewerPtr = nullptr;
  PolyLineTraj m_polyline_traj;
  Base::txVec3 m_vTargetDir;
  Base::txFloat m_start_angle_degree = -1.0;
  Base::txFloat m_start_angle_action_duration_ms = 0.0;
  Base::txFloat m_sim_step_ms = 20.0;
};
using TAD_PedestrianElementPtr = std::shared_ptr<TAD_PedestrianElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
