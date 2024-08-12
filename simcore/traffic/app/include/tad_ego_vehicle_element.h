// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_vehicle_kinetics_info.h"
#include "tad_vehicle_scan_region.h"
#include "tx_env_perception.h"
#include "tx_header.h"
#include "tx_obb.h"
#include "tx_scene_loader.h"
#include "tx_scene_sketch.h"
#include "tx_vehicle_element.h"
#include "vehicle_geometry.pb.h"
TX_NAMESPACE_OPEN(TrafficFlow)
class TAD_EgoVehicleElement : public Base::IVehicleElement, public TrafficFlow::IKineticsHandler {
 public:
  using ParentClass = Base::IVehicleElement;
  using EgoSubType = Base::Enums::EgoSubType;
  using txBool = Base::txBool;
  using txFloat = Base::txFloat;

 public:
  TAD_EgoVehicleElement() TX_DEFAULT;
  virtual ~TAD_EgoVehicleElement();

 public:
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;
  virtual txBool Initialize_Kinetics(ISceneLoader::IVehiclesViewerPtr) TX_NOEXCEPT;
#ifdef ON_CLOUD
  virtual txBool InitializeOnCloud(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT;
#endif /*ON_CLOUD*/
  /**
   * @brief Update method is called before processing the time parameters.
   * @brief The method takes a const reference to TimeParamManager as its argument.
   * @brief It returns a txBool value indicating the success or failure of the operation.
   *
   * @return txBool
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief Post_Update method is called after processing the time parameters.
   * @brief The method takes a const reference to TimeParamManager as its argument.
   *
   * @return txBool It returns a txBool value indicating the success or failure of the operation.
   */
  virtual txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief Pre_Update method is called before processing the time parameters.
   *
   * @param TimeParamManager TimeParamManager
   * @param map_elemId2Kinetics a reference to a map containing Elem_Id_Type as the key and KineticsInfo_t as the value.
   * @return txBool It returns a txBool value indicating the success or failure of the operation.
   */
  virtual txBool Pre_Update(const TimeParamManager&,
                            std::map<Elem_Id_Type, KineticsInfo_t>& map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief Release method is called when the object is no longer needed.
   *
   * @return txBool It returns a txBool value indicating the success or failure of the operation.
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 使用当前时间信息填充交通信息结构体
   *
   * 该函数用于根据当前时间信息填充交通信息结构体，并在每次模拟时间步骤中调用。
   *
   * @param timeParamMgr 一个包含当前时间信息的引用
   * @param traffic 交通信息结构体的引用，用于存放填充后的信息
   *
   * @return 如果函数执行成功则返回 true，否则返回 false
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 更新哈希车道信息
   *
   * 该函数用于根据当前位置更新哈希车道信息，并在每次模拟时间步骤中调用。
   *
   * @param _location 当前位置
   * @return 无
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取车辆行为
   *
   * 获取车辆的行为，用于确定车辆在路径规划和执行过程中的行为。
   *
   * @return 车辆行为枚举，表示车辆的行为类型
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_BEHAVIOR::ePlanning; }
  /**
   * @brief 获取车辆类型
   *
   * 获取车辆的类型，用于区分不同类型的车辆。
   *
   * @return 车辆类型枚举，表示车辆的类型
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return _plus_(ElementType::TAD_Ego); }
  /**
   * @brief 检查当前元素是否start
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return IsAlive(); }
  /**
   * @brief 检查当前元素是否end
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return IsAlive(); }
  /**
   * @brief 检查当前元素是否到达其 deadline.
   *
   * @return txBool
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE { return true; }
  /**
   * @brief 销毁此实体
   *
   * 对于其他类型的实体，这个函数通常仅用于标记实体已被销毁，使其在其他代码中变为无效。对于载船车辆，这个函数通常将删除载船车辆的所有成员变量，并释放内存。
   *
   * @return 如果此函数成功销毁实体，则返回 true，否则返回 false
   */
  virtual txBool Kill() TX_NOEXCEPT TX_OVERRIDE { return IsAlive(); }

  /**
   * @brief 判断是否支持特定行为
   *
   * 该函数用于检查车辆是否支持指定的行为。
   *
   * @param behavior 车辆行为类型
   * @return 返回值表示当前车辆是否支持指定的行为
   */
  virtual Base::txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VEHICLE_BEHAVIOR::ePlanning) == behavior);
  }

 public:
  /**
   * @brief 获取车辆类型
   *
   * 获取当前车辆的类型。
   *
   * @return 返回车辆类型
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 计算车辆的横向位置
   *
   * 根据车辆的运动状态，计算车辆在当前时间下的横向位置。
   *
   * @param timeMgr 时间管理器，包含当前时间等相关信息
   * @return 返回当前时间下的横向位置
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }
  /**
   * @brief 获取车辆切换车道的状态
   *
   * 获取车辆在当前时间下的切换车道状态，可能返回普通直行、左转、右转等。
   *
   * @return 返回车辆切换车道的状态
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return VehicleMoveLaneState::eStraight;
  }
  /**
   * @brief 返回当前对象的字符串表示
   *
   * 返回当前对象的字符串表示，用于调试和打印信息。
   *
   * @return txString 当前对象的字符串表示
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return "TAD_EgoVehicleElement"; }

  virtual txString Name() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Name(); }

 public:
  /**
   * @brief 设置子类型
   *
   * 设置当前对象的子类型。
   *
   * @param _subType 要设置的子类型
   */
  void SetSubType(EgoSubType _subType) TX_NOEXCEPT { m_SubType = _subType; }
  /**
   * @brief 获取子类型
   *
   * 获取当前对象的子类型。
   *
   * @return EgoSubType 返回当前对象的子类型
   */
  EgoSubType GetSubType() const TX_NOEXCEPT { return m_SubType; }
  /**
   * @brief 获取自车类型
   *
   * 获取当前对象的自车类型。
   *
   * @return Base::Enums::EgoType 返回当前对象的自车类型
   */
  Base::Enums::EgoType GetEgoType() const TX_NOEXCEPT { return m_EgoType; }
  /**
   * @brief 设置自车类型
   *
   * 设置当前对象的自车类型。
   *
   * @param _egoType 要设置的自车类型
   */
  void SetEgoType(Base::Enums::EgoType _egoType) TX_NOEXCEPT { m_EgoType = _egoType; }
  /**
   * @brief 检查当前自车是否为领先车辆
   *
   * 检查当前自车的子类型是否为领先车辆子类型
   *
   * @return true 当前自车为领先车辆
   * @return false 当前自车不为领先车辆
   */
  Base::txBool IsLeader() const TX_NOEXCEPT { return (_plus_(EgoSubType::eLeader) == GetSubType()); }
  /**
   * @brief 检查当前车辆是否为追随车辆
   *
   * 通过调用 IsLeader() 函数并逻辑非操作符，检查当前车辆是否为追随车辆。
   *
   * @return true 当前车辆为追随车辆
   * @return false 当前车辆不为追随车辆
   */
  Base::txBool IsFollower() const TX_NOEXCEPT { return !(IsLeader()); }
  // virtual Base::txFloat ComputeProjectDistance(const Geometry::OBB2D::PolygonArray& vecq) TX_NOEXCEPT;

  /**
   * @brief 设置pb缓冲区字符串
   *
   * 通过该函数，可以将原型缓冲区字符串设置为所给字符串。
   *
   * @param strPB 原型缓冲区字符串
   */
  virtual void SetProtoBufferStr(const Base::txString& strPB) TX_NOEXCEPT;
  /**
   * @brief 获取原型缓冲区节点
   *
   * 通过该函数，可以获取当前实例中的原型缓冲区节点，用于获取该节点的相关信息。
   *
   * @return ::sim_msg::Location 原型缓冲区节点
   */
  virtual ::sim_msg::Location GetProtoBufferNode() const TX_NOEXCEPT;

 public:
  using KineticsUtilInfo_t = TrafficFlow::IKineticsHandler::KineticsUtilInfo_t;
  /**
   * @brief 获取车辆动力学描述
   *
   * 此函数用于获取车辆的动力学描述。
   * 返回的动力学描述将用于监控系统的动力学模型。
   *
   * @return txString 车辆动力学描述
   */
  virtual txString KineticsDesc() const TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 获取车辆动力学信息
   *
   * 该函数用于获取车辆的动力学信息，包括重量、转矩、动力、扭矩等。
   *
   * @return KineticsUtilInfo_t 车辆动力学信息
   */
  virtual KineticsUtilInfo_t GetKineticsUtilInfo() const TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 计算车辆动力学信息
   *
   * 该函数用于计算车辆的动力学信息，包括重量、转矩、动力、扭矩等。
   * 通过参数 "timeMgr" 和 "_objInfo" 进行计算。
   *
   * @param timeMgr 时间管理器
   * @param _objInfo 车辆动力学信息，包括车辆的位置、速度、加速度等
   * @return txBool 计算成功返回 true，否则返回 false
   */
  virtual txBool ComputeKinetics(Base::TimeParamManager const& timeMgr,
                                 const KineticsUtilInfo_t& _objInfo) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 更新自车对象
   *
   * 该函数更新自车对象的状态，包括车辆的位置、速度、加速度等。
   * 使用传入的时间管理器作为参数，提供时间信息。
   *
   * @param timeMgr 时间管理器，包含了自车对象所在的时间域
   * @return txBool 若更新成功，则返回 true，否则返回 false
   */
  Base::txBool UpdateEgoVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;
  /**
   * @brief 计算自车的速度向量
   *
   * 通过输入时间管理器和自车质心，计算并更新自车速度向量和速度大小。
   * 速度向量使用物理惯例表示，即带有方向信息的向量。
   *
   * @param timeMgr 时间管理器，包含自车所在的时间域信息
   * @param egoMassCenter 自车质心的坐标值，使用二维向量表示
   * @return 无
   */
  void ComputeScalarVelocity(Base::TimeParamManager const& timeMgr, const Base::txVec2& egoMassCenter) TX_NOEXCEPT;
  // void ComputeProjectionMat(const Base::txVec2& p0, const Base::txVec2& p1) TX_NOEXCEPT;
  // Base::txBool IsPtInSegment(const Base::txVec2& pt, const Base::txVec2& segHead, const Base::txVec2& segTrail)
  // TX_NOEXCEPT; Base::txVec2 ComputeProjectPt(const Base::txVec2& q) TX_NOEXCEPT;

  /**
   * @brief 设置自车生命周期开始
   *
   * 设置自车生命周期开始，即自车初始化完成。
   *
   * @return 是否设置成功，若设置成功则表示自车已初始化完成
   */
  Base::txBool SetEgoStart() TX_NOEXCEPT {
    mLifeCycle.SetStart();
    return IsAlive();
  }

 public:
  /**
   * @brief 获取自车当前的速度值
   *
   * 获取自车当前的速度值，包括转向速度、加速度和减速度
   *
   * @return 当前速度
   */
  virtual Base::txFloat GetRawVelocity() const TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief Get the Show Velocity object
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetShowVelocity() const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 判断当前实例是否有效
   *
   * @return TXBool 如果实例有效，则返回true，否则返回false
   */
  virtual Base::txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE { return m_isValid; }
  /**
   * @brief 设置实例的有效性
   *
   * 设置实例的有效性，若为true则表示实例有效，否则表示实例无效。
   * 此函数不会引发任何异常。
   *
   * @param f 是否有效
   */
  virtual void SetValid(Base::txBool f) TX_NOEXCEPT { m_isValid = f; }

 protected:
  /**
   * @brief 更新下一个转向方向
   *
   * 在EgoVehicleElement类中，该函数用于更新下一个转向方向。
   * 通过判断当前汽车的状态，例如行驶状态、停车状态等，来决定下一个转向方向。
   * 该函数不会引发任何异常。
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 同步车辆位置
   * @param[in] _passTime 时间间隔，单位：秒
   * @return 是否同步成功
   * @details 该函数用于同步目标车辆的位置，可以在停留、行驶和靠近目标的过程中调用。
   *          在同步成功时返回 true，否则返回 false。
   */
  virtual txBool SyncPosition(const txFloat _passTime) TX_NOEXCEPT TX_OVERRIDE;
  /**
   * @brief 获取稳定的原始车辆坐标
   * @return 稳定的原始车辆坐标
   * @details
   * 该函数用于获取目标车辆在稳定的原始坐标系中的坐标。此函数在计算车辆的坐标时，会考虑到目标车辆的前后和左右的障碍物，以确保其坐标的稳定性。
   */
  virtual Coord::txENU StableRawVehicleCoord() const TX_NOEXCEPT;
#if __FocusVehicle__

 public:
  /**
   * @brief 计算关注的车辆
   * @param[in] timeMgr 时间参数管理器
   * @details
   * 该函数用于计算当前感知范围内的关注车辆。会根据时间管理器中的参数调整感知范围，同时也会考虑车辆自身的状态，如速度、方向等来确定关注车辆的列表。
   */
  virtual void ComputeFocusVehicles(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
#endif  // __FocusVehicle__

#if __TX_Mark__("UA_ES")

 public:
  /**
   * @brief Search_Front_Rear_Vehicle_HLane 用于查找同车道前后车辆
   * @param timeMgr 时间参数管理器
   * @param front_info 一个包含前车系统ID和与前车之间距离的元组
   * @param rear_info 一个包含后车系统ID和与后车之间距离的元组
   *
   * 此函数用于查找同车道前后车辆，并返回相关信息。它接受时间参数管理器作为输入，并返回包含前后车辆信息的元组。
   */
  virtual void Search_Front_Rear_Vehicle_HLane(const Base::TimeParamManager& timeMgr,
                                               std::tuple<Base::txSysId, Base::txFloat>& front_info,
                                               std::tuple<Base::txSysId, Base::txFloat>& rear_info) const TX_NOEXCEPT;
  /**
   * 获取当前车辆在局部坐标系下的 X 轴坐标
   *
   * @return 当前车辆在局部坐标系下的 X 轴坐标值
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
  /**
   * @brief 获取当前车辆在局部坐标系下的 Y 轴坐标
   *
   * @return 当前车辆在局部坐标系下的 Y 轴坐标值
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
  /**
   * @brief 获取当前车辆在局部坐标系下的 Z 轴坐标
   *
   * @return 当前车辆在局部坐标系下的 Z 轴坐标值
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

 protected:
#endif /*__TX_Mark__("UA_ES")*/

#if __JunctionYield__

 public:
  /**
   * @brief 更新车辆的轨迹
   *
   * @param[in] TimeParamManager const& 参数管理器的引用，包含当前的时间参数
   * @return txBool 更新轨迹是否成功，如果成功则返回true，否则返回false
   */
  virtual txBool UpdateTrajectory(Base::TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 生成连接轨迹的方法
   *
   * 使用给定的时间参数和连接点的向量，生成车辆从一组连接点连接到另一组连接点的路径
   *
   * @param TimeParamManager const& 参数管理器的引用，包含当前的时间参数
   * @param[out] std::vector< Base::txVec3 >& 轨迹点的向量，用于存储生成的轨迹点
   * @return txBool 生成轨迹是否成功，如果成功则返回true，否则返回false
   */
  virtual txBool MakeJuntionTrajectory(Base::TimeParamManager const&, std::vector<Base::txVec3>&) TX_NOEXCEPT;
#endif /*__JunctionYield__*/

 protected:
  /**
   * 计算车辆当前速度对应的车道坡度
   *
   * @return 返回车道坡度值，单位为以1为底的倒数
   */
  Base::txFloat ComputeSlope() const TX_NOEXCEPT;
#if __TX_Mark__("evn perception sketch")

 public:
  using Info_NearestObject = Base::Info_NearestObject;
  /**
   * 初始化SUDOKU格式的数字网格
   */
  virtual void Initialize_SUDOKU_GRID() TX_NOEXCEPT;
  /**
   * @brief 根据枚举值获取相应的相邻元素的顺时针紧密闭合区域
   *
   * @param eValue  枚举值，表示相邻元素方向
   * @param retRegion 返回值，包含顺时针紧密闭合区域的点集合
   */
  virtual void Get_Region_Clockwise_Close_By_Enum(const Base::Enums::NearestElementDirection eValue,
                                                  std::vector<Base::txVec2>& retRegion) const TX_NOEXCEPT;
  /**
   * @brief Compute_Region_Clockwise_Close 计算与车辆当前位置相邻的闭合区域，并沿着顺时针方向提供点的集合
   *
   * @param vehPos 车辆当前的ENU坐标
   * @param laneDir 车辆当前所在道路的方向向量
   * @param headingDegree 车辆当前的方向，以度数表示
   * @return
   */
  virtual void Compute_Region_Clockwise_Close(const Coord::txENU& vehPos, const Base::txVec3& laneDir,
                                              const Unit::txDegree& headingDegree) TX_NOEXCEPT;
  /**
   * @brief SnapshotHistoryTrajectory 将当前车辆的驾驶历史记录快照并存储到车辆的历史记录中
   * @param timeMgr 时间管理器，带有当前时间、时间步长等信息
   */
  virtual void SnapshotHistoryTrajectory(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;
  struct EgoRoadSTInfo {
    Base::txBool IsValid() const TX_NOEXCEPT { return (locInfo.IsOnLane()) && (st_valid); }
    Base::txBool IsValid(const Base::Info_Lane_t& dstLocInfo) const TX_NOEXCEPT {
      return IsValid() && (dstLocInfo.IsOnLane()) && (dstLocInfo.onLaneUid.roadId == locInfo.onLaneUid.roadId);
    }
    Base::txFloat passTime;
    Base::Info_Lane_t locInfo;
    Coord::txWGS84 ego_pos;
    Base::txVec3 velocity3d;
    Unit::txRadian heading;
    Base::txBool st_valid = false;
    Base::txVec2 st;
  };
  using EgoRoadSTInfoVec = std::vector<EgoRoadSTInfo>;
  using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
  /**
   * @brief 获取车辆历史轨迹信息
   * @return 车辆历史轨迹信息的 vector 容器，包含了车辆历史轨迹的信息
   */
  virtual const EgoRoadSTInfoVec& GetHistoryTrajectoryInfo() const TX_NOEXCEPT { return m_history_trajectory_info; }
  /**
   * @brief CheckEgoLaneChange 检查车辆对应的车道是否可以改变，以及其他可能的约束条件。
   *
   * @param timeMgr 时间管理器，带有当前时间、时间步长等信息
   * @param traj_info 当前车辆历史轨迹信息的 vector 容器，包含了车辆历史轨迹的信息
   */
  virtual void CheckEgoLaneChange(const Base::TimeParamManager& timeMgr, const EgoRoadSTInfoVec& traj_info) TX_NOEXCEPT;
  /**
   * @brief 更新车辆周围的沿线绘制元素
   *
   * 根据车辆当前位置和周围的其他元素，更新沿线绘制元素（如行人、马路标线等）的状态。
   *
   * @param timeMgr 时间管理器，包含当前时间和时间步长等信息
   */
  virtual void UpdateSketchSurroundingVehicle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
#  if __TX_Mark__("state machine interface")

 public:
  /**
   * @brief 获取车辆是否需要向左变道的状态
   *
   * 该函数用于获取车辆是否需要向左变道。
   * 当车辆在变道状态时，车辆需要向左变道，此函数返回true；否则返回false。
   *
   * @return 如果车辆需要向左变道，则返回true；否则返回false。
   */
  virtual Base::txBool IsTurnLeft() const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VehicleMoveLaneState::eLeft) == m_ego_LaneChangeState) ||
           (_plus_(VehicleMoveLaneState::eLeftInLane) == m_ego_LaneChangeState);
  }
  /**
   * @brief 判断车辆是否需要向右变道
   *
   * 该函数用于判断车辆是否需要向右变道。
   * 当车辆在变道状态时，车辆需要向右变道，此函数返回true；否则返回false。
   *
   * @return 如果车辆需要向右变道，则返回true；否则返回false。
   */
  virtual Base::txBool IsTurnRight() const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VehicleMoveLaneState::eRight) == m_ego_LaneChangeState) ||
           (_plus_(VehicleMoveLaneState::eRightInLane) == m_ego_LaneChangeState);
  }
  /**
   * @brief 判断车辆是否需要保持直行的行驶状态
   *
   * 该函数用于判断车辆是否需要保持直行的行驶状态。
   * 当车辆在变道状态时，车辆需要保持直行的行驶状态，此函数返回true；否则返回false。
   *
   * @return 如果车辆需要保持正直的行驶状态，则返回true；否则返回false。
   */
  virtual Base::txBool IsTurnStraight() const TX_NOEXCEPT TX_OVERRIDE {
    return (_plus_(VehicleMoveLaneState::eStraight) == m_ego_LaneChangeState);
  }
  /**
   * @brief 判断车辆是否换道状态
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsLaneChange() const TX_NOEXCEPT TX_OVERRIDE { return IsTurnLeft() || IsTurnRight(); }
#  endif /*state machine interface*/

 protected:
  /**
   * @brief 计算车辆目标位置
   *
   * @param vPos 车辆的当前位置
   * @param vehicleLaneDir 车辆的当前方向
   * @return std::array< Base::txVec3, Info_NearestObject::N_Dir >
   */
  virtual std::array<Base::txVec3, Info_NearestObject::N_Dir> ComputeTargetPos(
      const Base::txVec3& vPos, const Base::txVec3& vehicleLaneDir) const TX_NOEXCEPT;

  Scene::VehicleScanRegion2D mVehicleScanRegion2D;
  EgoRoadSTInfoVec m_history_trajectory_info;

  Base::Enums::VehicleMoveLaneState m_ego_LaneChangeState = _plus_(VehicleMoveLaneState::eStraight);
#endif /*"evn perception sketch"*/

 protected:
  Base::ISceneLoader::IVehiclesViewerPtr _elemAttrViewPtr = nullptr;
  Base::Enums::EgoType m_EgoType = Base::Enums::EgoType::eVehicle;
  Base::Enums::EgoSubType m_SubType = Base::Enums::EgoSubType::eLeader;
  Base::txString m_pb_str;
  sim_msg::Location m_egoData, m_last_egoData;
  Base::txBool m_isValid = true;
};

using TAD_EgoVehicleElementPtr = std::shared_ptr<TAD_EgoVehicleElement>;
TX_NAMESPACE_CLOSE(TrafficFlow)
