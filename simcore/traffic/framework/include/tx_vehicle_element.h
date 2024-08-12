// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_component.h"
#include "tx_count_down_timer.h"
#include "tx_header.h"
#include "tx_nearest_object_info.h"
#include "tx_optional.h"
#include "tx_scene_sketch.h"
#include "tx_serialization.h"
#include "tx_sim_point.h"
#include "tx_traffic_element_base.h"
#include "tx_vehicle_coord.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

class ISignalLightElement;
using ISignalLightElementPtr = std::shared_ptr<ISignalLightElement>;
using WEAK_ISignalLightElementPtr = std::weak_ptr<ISignalLightElement>;

class IPedestrianElement;
using IPedestrianElementPtr = std::shared_ptr<IPedestrianElement>;
using WEAK_IPedestrianElementPtr = std::weak_ptr<IPedestrianElement>;

// @brief 路口信息接口
class IJunctionInfo {
 public:
  using SelfDrivingDirection = Base::Enums::VehicleInJunction_SelfDrivingDirection;
  using RelativeDirection = Base::Enums::Vehicle2Vehicle_RelativeDirection;

 public:
  virtual ~IJunctionInfo() TX_DEFAULT;

  /**
   * @brief 获取上一个路口处方向信息
   * @return 返回上一个路口方向
   */
  virtual txVec3 JunctionInfo_LastOnLaneDir() const TX_NOEXCEPT { return mLaneDir; }

  /**
   * @brief 获取上一个交叉口在道路上的位置信息
   * @return 返回上一个交叉口在道路上的位置
   */
  virtual Coord::txENU JunctionInfo_LastOnLanePos() const TX_NOEXCEPT { return mEnuPosOnLane; }

 protected:
  /**
   * @brief 更新下一个交叉口的行驶方向
   *
   * 该函数用于更新下一个交叉口的行驶方向，从而确定车辆的最终行驶方向。
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT = 0;

  /**
   * @brief 设置上一个路口的车道信息
   * @param _enuPos 车辆的ENU坐标
   * @param _laneDir 车辆当前车道的方向向量
   */
  virtual void JunctionInfo_SetLastOnLaneInfo(const Coord::txENU &_enuPos, const txVec3 &_laneDir) TX_NOEXCEPT {
    mEnuPosOnLane = _enuPos;
    mLaneDir = _laneDir;
  }

 protected:
  Base::txOptional<SelfDrivingDirection> m_op_self_next_driving_direction;
  Coord::txENU mEnuPosOnLane;
  txVec3 mLaneDir;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IJunctionInfo");
    archive(_MAKE_NVP_("op_self_next_driving_direction", m_op_self_next_driving_direction));
    archive(_MAKE_NVP_("EnuPosOnLane", mEnuPosOnLane));
    archive(_MAKE_NVP_("LaneDir", mLaneDir));
  }
};
#if __TX_Mark__("SceneSketch")
class ISceneSketchElement {
 public:
  using SketchEnumType = Base::Enums::SketchEnumType;

  ISceneSketchElement() TX_DEFAULT;
  virtual ~ISceneSketchElement() TX_DEFAULT;

  /**
   * @brief 更新车辆周围的草图
   *
   * 当车辆移动时，该函数将更新周围草图的围栏以保持其与车辆几何定义的关联。
   *
   * @param timeMgr 当前时间参数管理器
   */
  virtual void UpdateSketchSurroundingVehicle(const TimeParamManager &timeMgr) TX_NOEXCEPT;

  /**
   * @brief 获取周围草图信息
   *
   * 返回一个包含当前车辆周围草图的信息的对象。该信息包含距离、最近的交通要素和障碍物等信息。
   *
   * @return 包含周围草图信息的对象
   */
  const Base::Info_NearestObject &SketchSurroundingInfo() const TX_NOEXCEPT { return m_sketch_surrounding_info; }

  /**
   * @brief 获取周围草图信息
   *
   * 返回一个包含当前车辆周围草图的信息的对象。该信息包含距离、最近的交通要素和障碍物等信息。
   *
   * @return 包含周围草图信息的对象
   */
  Base::Info_NearestObject &SketchSurroundingInfo() TX_NOEXCEPT { return m_sketch_surrounding_info; }

  /**
   * @brief 获取草图圆形元素指针集合
   *
   * 返回一个包含当前车辆周围草图的圆形元素指针的集合。该集合按照从近到远的顺序排列。
   *
   * @return 包含草图圆形元素指针的集合
   */
  const std::vector<Base::ITrafficElementPtr> &GetSketchCircleElementPtrVec() const TX_NOEXCEPT {
    return m_sketch_circleElementPtrVec;
  }

  /**
   * @brief 获取草图圆形元素指针集合
   *
   * 返回一个包含当前车辆周围草图的圆形元素指针的集合。该集合按照从近到远的顺序排列。
   *
   * @return 包含草图圆形元素指针的集合
   */
  std::vector<Base::ITrafficElementPtr> &GetSketchCircleElementPtrVec() TX_NOEXCEPT {
    return m_sketch_circleElementPtrVec;
  }

 protected:
  Base::Info_NearestObject m_sketch_surrounding_info;
  std::vector<Base::ITrafficElementPtr> m_sketch_circleElementPtrVec;
};
using ISceneSketchElementPtr = std::shared_ptr<ISceneSketchElement>;
#endif /*SceneSketch*/

class IVehicleElement;
using IVehicleElementPtr = std::shared_ptr<IVehicleElement>;

class IVehicleElement : public ITrafficElement /*, public ISimulationConsistency*/,
                        public IJunctionInfo,
                        public Base::VehicleCoord,
                        public ISceneSketchElement {
 public:
  using ISceneLoader = Base::ISceneLoader;
  using ISceneLoaderPtr = Base::ISceneLoaderPtr;
  using TimeParamManager = Base::TimeParamManager;
  using VEHICLE_BEHAVIOR = Base::ISceneLoader::VEHICLE_BEHAVIOR;
  using VEHICLE_TYPE = Base::ISceneLoader::VEHICLE_TYPE;
  using PolygonArray = Geometry::OBB2D::PolygonArray;
  enum class VehicleSource : txInt { unknown, scene_cfg, veh_input, ego, _3rd, world_mgr, hadmap };

  virtual ~IVehicleElement() TX_DEFAULT;

  /**
   * @brief 获取vehcile的具体类型
   *
   * @return VEHICLE_TYPE
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT = 0;
  virtual txString VehicleTypeStr() const TX_NOEXCEPT { return m_vehicle_type_str; }
  virtual txInt VehicleTypeId() const TX_NOEXCEPT { return m_vehicle_type_id; }

  /**
   * @brief 返回当前车辆行为类型
   *
   * @return VEHICLE_BEHAVIOR
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT = 0;

  /**
   * @brief 停车
   *
   */
  virtual void StopVehicle() TX_NOEXCEPT;

  /**
   * @brief vehicel是否停止
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsVehicleStop() const TX_NOEXCEPT { return mLifeCycle.IsStop(); }

  /**
   * @brief 是否支持behavior行为类型
   *
   * @param behavior 行为类型
   * @return Base::txBool
   */
  virtual Base::txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT = 0;

 public:
  /**
   * @brief 获取vehicle id
   *
   * @return txSysId
   */
  virtual txSysId Id() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取vehicle sysid
   *
   * @return txSysId
   */
  virtual txSysId SysId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.SysId(); }

 public:
  /**
   * @brief 是否激活
   *
   * @return txBool
   */
  virtual txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 是否结束
   *
   * @return txBool
   */
  virtual txBool IsEnd() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsEnd(); }

  /**
   * @brief 是否开始
   *
   * @return txBool
   */
  virtual txBool IsStart() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStart(); }

  /**
   * @brief 是否有效
   *
   * @return txBool
   */
  virtual txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsAlive(); }

  /**
   * @brief 是否停止
   *
   * @return txBool
   */
  virtual txBool IsStop() const TX_NOEXCEPT TX_OVERRIDE { return mLifeCycle.IsStop(); }

  /**
   * @brief 检查是否启动
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查是否结束
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckEnd(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 结束元素生命
   *
   * @return txBool
   */
  virtual txBool Kill() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查deadline
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT = 0;

  /**
   * @brief 是否onstart
   *
   * @param timeMgr
   */
  virtual void OnStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 是否处于end
   *
   */
  virtual void OnEnd() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取vehicle长度
   *
   * @return txFloat
   */
  virtual txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Length(); }

  /**
   * @brief Get the Width object
   *
   * @return txFloat
   */
  virtual txFloat GetWidth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Width(); }

  /**
   * @brief Get the Heigth object
   *
   * @return txFloat
   */
  virtual txFloat GetHeigth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Height(); }

  /**
   * @brief 获取vehicel包围盒
   *
   * @return const PolygonArray&
   */
  virtual const PolygonArray &Polygon() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Polygon(); }

 public:
  /**
   * @brief 获取几何中心的经纬度
   *
   * @return Coord::txWGS84
   */
  virtual Coord::txWGS84 GetLocation() const TX_NOEXCEPT TX_OVERRIDE { return GeomCenter(); }
  virtual txFloat DistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.DistanceAlongCurve(); }
  virtual txFloat GeomCurveLength() const TX_NOEXCEPT;
  virtual txFloat InvertDistanceAlongCurve() const TX_NOEXCEPT {
    return (GeomCurveLength() - mLocation.DistanceAlongCurve());
  }

  /**
   * @brief 获取当前的车道偏移
   *
   * @return txFloat
   */
  virtual txFloat LaneOffset() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneOffset(); }

  /**
   * @brief 是否在路口连接上
   *
   * @return txBool
   */
  virtual txBool IsOnLaneLink() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLaneLink(); }

  /**
   * @brief 是否在车道上
   *
   * @return txBool
   */
  virtual txBool IsOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLane(); }

  /**
   * @brief 获取当前的车道信息
   *
   * @return const Info_Lane_t&
   */
  virtual const Info_Lane_t &GetCurrentLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }
  virtual Unit::txDegree GetCrossRot() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.CrossRot(); }

  /**
   * @brief 获取车道方向
   *
   * @return txVec3
   */
  virtual txVec3 GetLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }
  virtual Unit::txDegree GetHeadingWithAngle() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.heading(); }
  virtual const std::array<txVec3, 2> LocalCoords() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LocalCoords(); }
  virtual txFloat Altitude() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.Altitude(); }

 public:
  /**
   * @brief 获取vehicel的速度
   *
   * @return txFloat
   */
  virtual txFloat GetVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_velocity; }

  /**
   * @brief 获取vehicel的加速度
   *
   * @return txFloat
   */
  virtual txFloat GetAcc() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_acceleration; }

  /**
   * @brief 获取vehicle的横向加速度
   *
   * @return txFloat
   */
  virtual txFloat GetLateralAcc() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_lateral_acceleration; }
  virtual txFloat GetDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_displacement; }

  /**
   * @brief 获取vehicle的侧向速度
   *
   * @return txFloat
   */
  virtual txFloat GetLateralVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_LateralVelocity; }
  virtual txFloat GetLateralDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_LateralDisplacement; }
  virtual txFloat Compute_Displacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    mKinetics.m_displacement = GetVelocity() * timeMgr.RelativeTime();
    return GetDisplacement();
  }

  /**
   * @brief 计算中心线偏移
   *
   * @return txFloat
   */
  virtual txFloat Compute_CenterLine_Offset() TX_NOEXCEPT;
  virtual void SetVelocityDesired(const txFloat _new_velocity_desired) TX_NOEXCEPT TX_OVERRIDE;
  virtual txFloat GetRawVelocityDesired() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.raw_velocity_max; }
  virtual txFloat GetNowVelocityDesired() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.m_velocity_max; }
  virtual void ResetVelocityDesired() TX_NOEXCEPT TX_OVERRIDE;

 public:
  virtual txFloat GetAggress() const TX_NOEXCEPT { return m_aggress; }

 public:
  /**
   * @brief 更新空间检索
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 添加元素与操作
   *
   * @param a_size
   * @return txBool
   */
  virtual txBool PreFillingElement(AtomicSize &a_size) TX_NOEXCEPT TX_OVERRIDE {
    if (IsAlive()) {
      m_index_in_pb_without_serialize = (a_size++);
    }
    return true;
  }

  /**
   * @brief 加入元素
   *
   * @return txBool
   */
  virtual txBool FillingElement(Base::TimeParamManager const &, sim_msg::Traffic &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将元素数据填充到车辆
   *
   * 该函数根据时间戳和模拟车辆实例，填充元素数据到模拟车辆中。
   *
   * @param timeStamp 当前时间戳
   * @param pSimVehicle 模拟车辆实例指针
   * @return sim_msg::Car* 填充元素后的模拟车辆实例指针
   */
  virtual sim_msg::Car *FillingElement(txFloat const timeStamp, sim_msg::Car *pSimVehicle) TX_NOEXCEPT;

  /**
   * @brief 根据时间戳填充车辆位置信息
   *
   * 该函数根据给定的时间戳，从模拟车辆中获取对应的位置信息。
   *
   * @param timeStamp_ms 时间戳（以毫秒为单位）
   * @param refLoc 用于存储车辆位置信息的 Location 实例
   * @return txBool 是否成功获取到位置信息
   */
  virtual txBool FillingLocation(const Base::txFloat timeStamp_ms, sim_msg::Location &refLoc) TX_NOEXCEPT {
    return false;
  }

  /**
   * @brief 获取车辆的动力学信息
   * @param timeMgr 时间参数管理器
   * @return KineticsInfo_t 车辆的动力学信息
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const &timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间参数管理器填充车辆轨迹信息
   * @param timeMgr 时间参数管理器，用于获取当前时间、速度和加速度等参数信息
   * @param trajectory 输出的车辆轨迹信息，存储车辆的坐标、速度、加速度等参数信息
   * @return true 成功填充车辆轨迹信息
   * @return false 填充车辆轨迹信息失败
   */
  virtual txBool FillingTrajectory(Base::TimeParamManager const &, sim_msg::Trajectory &) TX_NOEXCEPT;

 public:
  /**
   * @brief 获取车辆的一致性 ID
   * @return 返回车辆的一致性 ID
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取车辆后轴中心车辆坐标
   * @return 返回后轴中心车辆坐标
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastRearAxleCenter(); }

  /**
   * @brief 获取车辆的稳定中心坐标
   * @return 返回车辆稳定中心坐标
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastGeomCenter(); }

  /**
   * @brief 获取车辆稳定在道路上的位置
   * @return 返回车辆在道路上的稳定位置
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastOnLanePos(); }

  /**
   * @brief 获取车辆稳定在道路上的信息
   * @return 返回车辆稳定在道路上的信息
   */
  virtual Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastLaneInfo(); }

  /**
   * @brief 获取车辆稳定在道路上的信息
   * @return 返回车辆稳定在道路上的信息
   */
  virtual txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastLaneDir(); }

  /**
   * @brief 获取车辆稳定在道路上的方向
   * @return 返回车辆稳定在道路上的方向
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LastHeading(); }

  /**
   * @brief 获取车辆稳定在道路上的速度
   * @return 返回车辆稳定在道路上的速度
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.LastVelocity(); }

  /**
   * @brief 获取车辆稳定在道路上的加速度
   * @return 返回车辆稳定在道路上的加速度
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return mKinetics.LastAcceleration(); }

  /**
   * @brief 获取车辆稳定在道路上的反向距离
   * @return 返回车辆稳定在道路上的反向距离
   */
  virtual txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastInvertDistanceAlongCurve();
  }

  /**
   * @brief 获取车辆稳定在曲线上的距离
   * @return 返回车辆稳定在曲线上的距离
   */
  virtual txFloat StableDistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LastDistanceAlongCurve();
  }

  /**
   * @brief 保存车辆的稳定状态
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前稳定的哈希后的车道信息
   * @return 返回当前稳定的哈希后的车道信息
   */
  virtual const HashedLaneInfo &StableHashedLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return m_curHashedLaneInfo; }
  virtual HashedLaneInfo &StableHashedLaneInfo() TX_NOEXCEPT { return m_curHashedLaneInfo; }

  /**
   * @brief 计算实时的车辆加速度信息
   * @param timeMgr 时间参数管理器
   * @return 无返回值
   */
  virtual void ComputeRealAcc(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT;

 public:
  /**
   * @brief 获取当前目标信号灯指针
   * @return 当前目标信号灯指针
   */
  virtual Base::ISignalLightElementPtr GetCurTargetSignLightPtr() const TX_NOEXCEPT {
    return Weak2SharedPtr(m_weak_TargetSignLightPtr);
  }

  /**
   * @brief 获取当前目标信号灯距离
   * @return 当前目标信号灯距离
   */
  virtual Base::txFloat GetCurDistanceToTargetSignLight() const TX_NOEXCEPT { return mDistanceToTargetSignLight; }

  /**
   * @brief 设置当前距离目标信号灯的距离
   * @param dist 当前距离目标信号灯的距离
   */
  virtual void SetCurDistanceToTargetSignLight(Base::txFloat dist) TX_NOEXCEPT;

  /**
   * @brief 设置当前目标信号灯指针
   * @param tlPtr 当前目标信号灯指针
   */
  virtual void SetCurTargetSignLightPtr(Base::ISignalLightElementPtr tlPtr) TX_NOEXCEPT;

  /**
   * @brief 设置当前目标信号灯指针及当前距离
   * @param tlPtr 目标信号灯指针
   * @param dist 当前距离
   */
  virtual void SetCurTargetSignLight(Base::ISignalLightElementPtr tlPtr, Base::txFloat dist) TX_NOEXCEPT;

  /**
   * @brief 获取当前前方行人指针
   *
   * 获取当前前方行人的智能指针
   * @return Base::IPedestrianElementPtr 当前前方行人指针
   */
  virtual Base::IPedestrianElementPtr GetCurFrontPedestrianPtr() const TX_NOEXCEPT {
    return Weak2SharedPtr(m_weak_PedestrianPtr);
  }

  /**
   * @brief 获取当前行人距离
   * @return 当前行人距离
   */
  virtual Base::txFloat GetDistanceToPedestrian() const TX_NOEXCEPT { return mDistanceToPedestrian; }

  /**
   * @brief 设置当前行人距离
   *
   * @param dist 要设置的距离
   */
  virtual void SetDistanceToPedestrian(Base::txFloat dist) TX_NOEXCEPT;

  /**
   * @brief 设置当前前方行人指针
   * @param pedePtr 要设置的前方行人指针
   */
  virtual void SetCurFrontPedestrianPtr(Base::IPedestrianElementPtr pedePtr) TX_NOEXCEPT;

  /**
   * @brief 获取预测的路径点
   *
   * 该函数通过输入的距离，返回该距离后的路径点位置。
   *
   * @param distance 输入距离
   * @param prePoint 输出预测路径点的位置
   * @return true 成功获取路径点
   * @return false 无法获取路径点
   */
  virtual txBool GetPredictionPos(const Base::txFloat distance, Coord::txENU &prePoint) const TX_NOEXCEPT;

 public:
  /**
   * @brief 下个路口是否右转
   *
   * @return txBool
   */
  virtual txBool IsTurnRightAtNextIntersection() const TX_NOEXCEPT;

  /**
   * @brief 下个路口是否左转
   *
   * @return txBool
   */
  virtual txBool IsTurnLeftAtNextIntersection() const TX_NOEXCEPT;

  /**
   * @brief 下个路口是否直行
   *
   * @return txBool
   */
  virtual txBool IsGoStraightAtNextIntersection() const TX_NOEXCEPT;

  /**
   * @brief 下个路口是否掉头
   *
   * @return txBool
   */
  virtual txBool IsUTurnAtNextIntersection() const TX_NOEXCEPT;

 protected:
  /**
   * @brief 在随机下一车道link的情况下，返回一个符合要求的车道link
   *
   * 当调用此函数时，找到车辆所在的车道下一车道link，
   * 并在符合条件的情况下返回该车道link，否则返回空指针。
   *
   * @param nextLanes 车辆当前位置的所有下一车道link
   * @return hadmap::txLanePtr 返回符合条件的下一车道link指针，如果没有符合条件的下一车道link则返回空指针
   */
  virtual hadmap::txLaneLinkPtr RandomNextLink(const hadmap::txLaneLinks &nextLaneLinks) TX_NOEXCEPT;

  /**
   * @brief 在随机下一车道的情况下，返回一个符合要求的车道
   *
   * 当调用此函数时，找到车辆所在的车道下一车道，
   * 并在符合条件的情况下返回该车道，否则返回空指针。
   *
   * @param nextLanes 车辆当前位置的所有下一车道
   * @return hadmap::txLanePtr 返回符合条件的下一车道指针，如果没有符合条件的下一车道则返回空指针
   */
  virtual hadmap::txLanePtr RandomNextLane(const hadmap::txLanes &nextLanes) TX_NOEXCEPT;
#if USE_HashedRoadNetwork

 protected:
  /**
   * @brief 在初始化时注册车辆元素信息
   *
   * 当车辆元素的位置信息发生变化时，调用此函数以更新其哈希车辆元素信息。
   *
   * @param _location 车辆元素新的位置信息
   * @return void
   */
  virtual void RegisterInfoOnInit(const Component::Location &_location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新哈希车辆元素信息
   *
   * 当车辆元素位置信息发生变化时，调用此函数以更新其哈希车辆元素信息。
   *
   * @param _location 车辆元素新的位置信息
   * @return void
   */
  virtual void UpdateHashedLaneInfo(const Component::Location &_location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 取消注册车辆元素相关信息
   *
   * 当车辆元素对象的生命周期结束时，可以调用此函数来取消注册与该车辆元素相关的信息。
   *
   * @return void
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_HashedRoadNetwork*/

 public:
  /**
   * @brief 获取指向VehicleElement的智能指针
   *
   * 此函数返回一个智能指针，指向当前对象的VehicleElement的实例。
   *
   * @return 返回一个智能指针，指向当前对象的VehicleElement的实例。
   */
  virtual std::shared_ptr<IVehicleElement> GetVehicleElementPtr() TX_NOEXCEPT {
    return std::dynamic_pointer_cast<IVehicleElement>(GetBasePtr());
  }

  /**
   * @brief 获取0到1之间的随机浮点数
   *
   * 这个函数返回一个介于0到1之间的随机浮点数，每次调用都会生成不同的随机值。
   *
   * @return 返回一个介于0到1之间的随机浮点数
   */
  virtual txFloat GetRnd_0_1() TX_NOEXCEPT { return mPRandom.GetRandomValue(); }

 public:
  /**
   * @brief 重置车道保持状态
   *
   * 该函数重置车辆当前车道保持状态，可以在车辆启动、加速、刹车等场景调用，使车辆恢复到正常的行驶状态。
   *
   * @return 无
   */
  virtual void ResetLaneKeep() TX_NOEXCEPT {}

 public:
  /**
   * @brief 获取车辆在通道连接的时间戳
   *
   * 该函数返回车辆在通道连接时的时间戳，包括对应的车道的时间戳，方便后续的操作。
   *
   * @return 车辆在通道连接的时间戳
   */
  virtual Base::txFloat GetOnLaneLinkTimeStamp() const TX_NOEXCEPT { return mLocation.TimeStampOnLanelink(); }

  /**
   * @brief GetVehicleComeFrom 获取车辆出现的来源
   *
   * 该函数返回车辆出现的来源，包括VEHICLE_COME_FROM_GUIDE_LANE、
   * VEHICLE_COME_FROM_GUIDE_LINE等。
   *
   * @return VehicleSource 车辆出现的来源
   */
  virtual VehicleSource Vehicle_ComeFrom() const TX_NOEXCEPT { return m_vehicle_come_from; }

  /**
   * @brief ResetFCW 重置车辆FCW(Field of View, Maneuver)
   *
   * 该函数将重置车辆的FCW值。
   *
   * @param 无需传入参数
   * @return 无返回值
   */
  virtual void ResetFCW() TX_NOEXCEPT;

  /**
   * @brief 对车辆进行FCW计算
   *
   * 此函数用于计算车辆与目标交通元素之间的FCW（Field of View, Maneuver），即车辆各摄像头的视野和对抗行为监测的准确性。
   *
   * @param target_ptr 目标交通元素的指针
   * @return void 返回空值
   */
  virtual void Compute_FCW(Base::ITrafficElementPtr target_ptr) TX_NOEXCEPT;

 public:
  /**
   * @brief 获取车辆的中间GPS坐标
   *
   * 此函数用于获取车辆的中间GPS坐标，也就是穿越所有路线节点后的坐标。
   * @return std::vector<hadmap::txPoint> 车辆的中间GPS坐标列表
   */
  virtual std::vector<hadmap::txPoint> RouteMidGPS() const TX_NOEXCEPT { return std::vector<hadmap::txPoint>(); }

  /**
   * @brief 获取车辆的路线结束GPS坐标
   *
   * 此函数用于获取车辆的路线结束GPS坐标。
   * @return hadmap::txPoint 路线结束GPS坐标
   */
  virtual hadmap::txPoint RouteEndGPS() const TX_NOEXCEPT { return hadmap::txPoint(); }
#if __TX_Mark__("VehicleCoord")

 public:
  /**
   * @brief 位置同步
   *
   * @param _passTime 时间戳
   * @return txBool
   */
  virtual txBool SyncPosition(const txFloat _passTime) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取车辆的中心点坐标
   *
   * 此函数用于获取车辆的中心点坐标。
   * @return Coord::txWGS84 & 车辆的中心点坐标
   */
  virtual Coord::txWGS84 &GeomCenter() TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }
  virtual const Coord::txWGS84 &GeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter(); }

  /**
   * @brief 获取后轴线中心点坐标（WGS84）
   *
   * 此函数用于获取当前车辆元素后轴线中心点的WGS84坐标。
   * @return const Coord::txWGS84& 后轴线中心点坐标（WGS84）
   */
  virtual Coord::txWGS84 &RearAxleCenter() TX_NOEXCEPT TX_OVERRIDE { return mLocation.RearAxleCenter(); }
  virtual const Coord::txWGS84 &RearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.RearAxleCenter(); }

 protected:
  /**
   * @brief 获取车辆元素的类型
   *
   * 此函数用于获取车辆元素的类型。返回值可以是车辆元素的枚举值。
   * @return Base::Enums::ElementType 车辆元素类型枚举值
   */
  virtual Base::Enums::ElementType VehicleCoordElementType() const TX_NOEXCEPT TX_OVERRIDE { return Type(); }

  /**
   * @brief 获取车辆元素的行为
   *
   * 此函数用于获取车辆元素的行为。返回值可以是车辆行为的枚举值。
   * @return Enums::VEHICLE_BEHAVIOR 车辆行为枚举值
   */
  virtual Base::Enums::VEHICLE_BEHAVIOR VehicleCoordElementBehavior() const TX_NOEXCEPT TX_OVERRIDE {
    return VehicleBehavior();
  }
#endif /*VehicleCoord*/

#if __TX_Mark__("UA_ES")

 public:
  enum class spatial_relationship_with_ego : Base::txInt{no_relationship = 0, ego_front_veh = 1, ego_rear_veh = 2};

  /**
   * @brief 获取当前车辆在局部坐标系中的坐标点X值
   *
   * 该函数用于获取当前车辆在局部坐标系中的坐标点的X值。该点的坐标系相对于车辆的自身坐标系，而不是全局坐标系。
   * @return txFloat 当前车辆在局部坐标系中的坐标点X值
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前车辆在局部坐标系中的坐标点Y值
   *
   * 此函数用于获取当前车辆在局部坐标系中的坐标点的Y值。该点的坐标系相对于车辆的自身坐标系，而不是全局坐标系。
   * @return txFloat 当前车辆在局部坐标系中的坐标点Y值
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前车辆的局部坐标系中的坐标点Z值
   *
   * 这个函数用于获取当前车辆在局部坐标系中的坐标点的Z值。该点的坐标系相对于车辆的自身坐标系，而不是全局坐标系。
   * @return txFloat 当前车辆在局部坐标系中的坐标点Z值
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前车辆的局部坐标点
   *
   * 此函数用于获取当前车辆的局部坐标系中的某一点的坐标。该坐标点将基于车辆的自身坐标系，而不是全局坐标系。
   * @return sim_msg::Vec3 当前车辆的局部坐标点
   */
  virtual sim_msg::Vec3 local_cartesian_point() const TX_NOEXCEPT;

  /**
   * @brief 设置前车辆状态
   *
   * 此函数用于设置当前车辆的前车辆状态。它将当前车辆与指定的前车辆进行关联，并记录它们之间的距离。
   * @param dist 前车辆与当前车辆的距离 (单位: 米)
   */
  virtual void set_front_vehicle(const Base::txFloat dist) TX_NOEXCEPT {
    m_ego_relationship = spatial_relationship_with_ego::ego_front_veh;
    m_ego_dist = dist;
  }

  /**
   * @brief 设置后车辆状态
   *
   * 这个函数用于设置当前车辆的后车辆状态。它将当前车辆与指定的后车辆进行关联，并记录它们之间的距离。
   * @param dist 后车辆与当前车辆的距离 (单位: 米)
   */
  virtual void set_rear_vehicle(const Base::txFloat dist) TX_NOEXCEPT {
    m_ego_relationship = spatial_relationship_with_ego::ego_rear_veh;
    m_ego_dist = dist;
  }

  /**
   * @brief 设置无关系状态
   *
   * 该函数将不关联任何对象，并将与自身的关系状态设置为 "无关系"。
   * 注意: 本函数仅设置与自身的关系，而不是对象之间的关系。
   */
  virtual void set_no_relationship() TX_NOEXCEPT {
    m_ego_relationship = spatial_relationship_with_ego::no_relationship;
  }

  /**
   * @brief 计算外部信息
   *
   * 该函数根据自身的状态计算外部对象的关系和距离信息，并将其存储在 extra_info 结构体中。
   *
   * @param _extern_info[out] 存储外部对象关系和距离信息的结构体指针
   */
  virtual void ComputeExternInfo(sim_msg::Extra_Info *_extern_info) const TX_NOEXCEPT;

  /**
   * @brief 清除外部信息
   *
   * 该函数将清除与外部对象的关系信息和距离信息。
   */
  virtual void ClearExternInfo() TX_NOEXCEPT {
    m_ego_dist = 0.0;
    m_ego_relationship = spatial_relationship_with_ego::no_relationship;
  }

 protected:
  spatial_relationship_with_ego m_ego_relationship = spatial_relationship_with_ego::no_relationship;
  Base::txFloat m_ego_dist = 0.0;
#endif

 public:
  /**
   * @brief 在给定的道路链接和时间戳下重新定位车辆跟踪器
   *
   * @param pLane 给定的道路链接指针
   * @param _timestamp 给定的时间戳
   * @return Base::txBool 重新定位车辆跟踪器的成功与否
   */
  virtual Base::txBool RelocateTracker(hadmap::txLanePtr pLane, const txFloat &_timestamp) TX_NOEXCEPT {
    SetDrivingStatus(_plus_(Base::IDrivingStatus::DrivingStatus::unknown));
    return mLocation.RelocateTracker(pLane, _timestamp);
  }

  /**
   * @brief 通过给定的pLaneLink在指定的时间戳下重新定位车辆跟踪器
   *
   * @param pLaneLink 指定的道路链接指针
   * @param _timestamp 指定的时间戳
   * @return Base::txBool 重新定位车辆跟踪器的成功与否
   */
  virtual Base::txBool RelocateTracker(hadmap::txLaneLinkPtr pLaneLink, const txFloat &_timestamp) TX_NOEXCEPT {
    return mLocation.RelocateTracker(pLaneLink, _timestamp);
  }

  /**
   * @brief 获取速度限制阈值
   *
   * @return const Base::txFloat& 速度限制阈值
   */
  Base::txFloat SpeedLimitThreshold() const TX_NOEXCEPT { return FLAGS_SpeedLimitThreshold; }

 public:
  /**
   * @brief 检查指定车辆类型是否为不透明物体
   *
   * 该函数通过检查传入的车辆类型，判断是否为不透明物体。当传入的车辆类型是不透明物体时，返回 true，否则返回 false。
   *
   * @param _vehicle_type 需要检查的车辆类型
   * @return true 传入的车辆类型为不透明物体
   * @return false 传入的车辆类型不是不透明物体
   */
  static Base::txBool IsTransparentObject(const Base::Enums::VEHICLE_TYPE &_vehicle_type) TX_NOEXCEPT;
  static Base::txBool IsTransparentObject(const Base::txString &_vehicle_type) TX_NOEXCEPT;

 protected:
  /**
   * @brief 检查当前的车辆是否为不透明物体
   *
   * 如果当前车辆是不透明物体，则返回true，否则返回false。
   *
   * @return true 当前车辆为不透明物体
   * @return false 当前车辆不是不透明物体
   */
  virtual Base::txBool IsTransparentObject() const TX_NOEXCEPT { return IsTransparentObject(m_vehicle_type); }

  /**
   * @brief 检查当前的车辆是否只沿着一条轨迹走
   *
   * 如果当前车辆的行驶方式只包含沿一条轨迹行驶的行为，则返回true，否则返回false。
   *
   * @param _vehicle_type 当前车辆的类型
   * @return true 当前车辆只沿着一条轨迹行驶
   * @return false 当前车辆沿着多条轨迹行驶
   */
  virtual Base::txBool IsOnlyTrajectoryFollow(const Base::Enums::VEHICLE_TYPE &_vehicle_type) const TX_NOEXCEPT {
    return IsTransparentObject(_vehicle_type);
  }
  virtual Base::txBool IsOnlyTrajectoryFollow(const Base::txString &_vehicle_type) const TX_NOEXCEPT {
    return IsTransparentObject(_vehicle_type);
  }

#if __TX_Mark__("state machine interface")

 public:
  /**
   * @brief 判断车辆是否可以转向左侧
   *
   * 该函数返回当前车辆是否可以转向左侧，如果可以则返回true，否则返回false。
   * @return true 当前车辆可以转向左侧
   * @return false 当前车辆不能转向左侧
   */
  virtual Base::txBool IsTurnLeft() const TX_NOEXCEPT { return false; }

  /**
   * @brief 检查车辆是否可以转向右侧
   *
   * 该函数返回当前车辆是否可以转向右侧，如果可以则返回true，否则返回false。
   * @return true 当前车辆可以转向右侧
   * @return false 当前车辆不能转向右侧
   */
  virtual Base::txBool IsTurnRight() const TX_NOEXCEPT { return false; }

  /**
   * @brief 判断车辆是否可以朝直线行驶
   *
   * 该函数返回当前车辆是否可以朝直线行驶，如果可以则返回true，否则返回false。
   * @return true 当前车辆可以朝直线行驶
   * @return false 当前车辆不能朝直线行驶
   */
  virtual Base::txBool IsTurnStraight() const TX_NOEXCEPT { return false; }

  /**
   * @brief 获取车辆是否可以进行lane change（变道）操作
   *
   * 该函数返回当前车辆是否可以进行lane change，如果可以则返回true，否则返回false。
   * @return true 当前车辆可以进行lane change
   * @return false 当前车辆不能进行lane change
   */
  virtual Base::txBool IsLaneChange() const TX_NOEXCEPT { return false; }

  /**
   * @brief 是否在距离范围内的lanlink
   *
   * @param dist 指定距离
   * @return Base::txBool
   */
  virtual Base::txBool IsNearLink(const txFloat dist) const TX_NOEXCEPT;

  /**
   * @brief txVehicleElement::IsOnJunction 判断车辆是否位于路口
   *
   * 该函数检查车辆的当前位置是否位于路口。如果在路口，则返回 `true`；否则返回 `false`。
   * @return Base::txBool 如果车辆在路口，则返回 `true`；否则返回 `false`
   */
  virtual Base::txBool IsOnJunction() TX_NOEXCEPT;
#endif /*state machine interface*/

 protected:
  Base::Component::Identity mIdentity;
  Base::Component::LifeCycle mLifeCycle;
  Base::Component::GeometryData mGeometryData;
  Base::Component::Location mLocation;
  Base::Component::Kinetics mKinetics;

  Base::Component::Pseudorandom mPRandom;
  Base::txCountDownTimer mLaneKeepMgr;
  Base::txFloat m_aggress = 0.5;
  Base::txBool mRuleCompliance = true;
  Base::txBool m_lastLaneLink = false;
  Base::Component::Location m_LastLocation;

  WEAK_ISignalLightElementPtr m_weak_TargetSignLightPtr; /*Base::ISignalLightElementPtr mTargetSignLightPtr = nullptr;*/
  Base::txFloat mDistanceToTargetSignLight = FLT_MAX;

  WEAK_IPedestrianElementPtr m_weak_PedestrianPtr;
  Base::txFloat mDistanceToPedestrian = 999.0;
  tbb::mutex m_tbbMutex;

  VehicleSource m_vehicle_come_from = VehicleSource::unknown;

  sim_msg::ForwardCollisionWarning mFCW;
  VEHICLE_TYPE m_vehicle_type = VEHICLE_TYPE::Sedan;
  Base::txString m_vehicle_type_str = "Sedan";
  Base::txInt m_vehicle_type_id = -1;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IVehicleElement");
    archive(cereal::base_class<ITrafficElement>(this));
    archive(cereal::base_class<IJunctionInfo>(this));

    archive(_MAKE_NVP_("Identity", mIdentity));
    archive(_MAKE_NVP_("LifeCycle", mLifeCycle));
    archive(_MAKE_NVP_("GeometryData", mGeometryData));
    archive(_MAKE_NVP_("Location", mLocation));
    archive(_MAKE_NVP_("Kinetics", mKinetics));
    archive(_MAKE_NVP_("PRandom", mPRandom));
    archive(_MAKE_NVP_("LaneKeepMgr", mLaneKeepMgr));

    archive(_MAKE_NVP_("aggress", m_aggress));
    archive(_MAKE_NVP_("ruleCompliance", mRuleCompliance));
    archive(_MAKE_NVP_("vehicle_come_from", m_vehicle_come_from));
    archive(_MAKE_NVP_("vehicle_type", m_vehicle_type));
    archive(_MAKE_NVP_("vehicle_type_str", m_vehicle_type_str));
    archive(_MAKE_NVP_("vehicle_type_id", m_vehicle_type_id));
    /*archive(_MAKE_NVP_("FCW", mFCW));*/
  }
};

using WEAK_IVehicleElementPtr = std::weak_ptr<IVehicleElement>;
TX_NAMESPACE_CLOSE(Base)

#undef SerializeInfo

CEREAL_REGISTER_TYPE(Base::IVehicleElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ITrafficElement, Base::IVehicleElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IJunctionInfo, Base::IVehicleElement);
