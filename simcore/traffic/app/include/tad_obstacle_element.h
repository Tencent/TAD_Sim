// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_component.h"
#include "tx_header.h"
#include "tx_locate_info.h"
#include "tx_vehicle_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_ObstacleElement : public Base::IVehicleElement {
 public:
  using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;

 public:
  TAD_ObstacleElement() TX_DEFAULT;
  virtual ~TAD_ObstacleElement();

  // Virtual function declaration for initializing
  virtual txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放元素资源
   *
   * @return true 成功释放资源
   * @return false 资源未被释放（未被成功激活）
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间参数，填充交通元素
   *
   * @param[in] timeParamManager 时间参数管理器
   * @param[out] traffic 交通元素的信息
   * @return true 成功填充元素
   * @return false 无法填充元素
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查是否需要填充空间查询
   *
   * 如果需要填充空间查询，返回true，否则返回false。
   *
   * @return txBool 返回 true 表示需要填充空间查询；返回 false 表示不需要填充空间查询。
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 对障碍物元素进行预处理
   *
   * 此函数在填充元素之前调用，用于预处理障碍物元素。在此函数中，可以获取元素序号，并根据需要对元素进行调整。
   *
   * @param a_size 待填充的元素序号，用于标识所填充的元素位置
   * @return txBool 返回 true 表示允许填充元素；返回 false 表示不允许填充元素。
   */
  virtual txBool PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT TX_OVERRIDE {
    if (IsAlive()) {
      m_index_obstacle_in_pb_without_serialize = (a_size++);
    }
    return true;
  }

  /**
   * @brief 获取元素在不序列化PB时的索引
   *
   * 返回在不序列化PB时该元素的索引，即元素在PB的填充列表中的顺序。
   * 这个值在没有序列化PB时使用，例如更新或删除障碍物时。
   *
   * @return AtomicSizeValueType 返回元素在不序列化PB时的索引
   */
  virtual AtomicSizeValueType IndexInPb() const TX_NOEXCEPT TX_OVERRIDE {
    return m_index_obstacle_in_pb_without_serialize;
  }

 public:
  /**
   * @brief 获取元素的ID
   *
   * 返回一个表示元素ID的值，该值在对象具有唯一性时才有效。
   * 该方法仅适用于已读取数据的ObstacleElement对象。
   *
   * @return txSysId 返回表示元素ID的值
   */
  virtual txSysId Id() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief 获取元素的系统ID
   *
   * 返回一个表示元素系统ID的值，该值在对象具有唯一性时才有效。
   * 该方法仅适用于已读取数据的ObstacleElement对象。
   *
   * @return txSysId 返回表示元素系统ID的值
   */
  virtual txSysId SysId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.SysId(); }

 public:
  // These functions define the behavior of the object

  /**
   * @brief 检查元素是否存活
   *
   * 此方法检查当前元素是否存活，如果存活则返回 true，否则返回 false。
   * 当对象已被销毁或移除时，可以通过此方法检查元素是否仍然存在。
   *
   * @return txBool 如果元素存活则返回 true，否则返回 false
   */
  virtual txBool IsAlive() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 检查生命周期是否结束
   *
   * @return txBool
   */
  virtual txBool IsEnd() const TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 检查生命周期是否开始
   *
   * @return txBool
   */
  virtual txBool IsStart() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 检查元素是否有效
   *
   * @return txBool
   */
  virtual txBool IsValid() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 检查生命周期是否停止
   *
   * @return txBool
   */
  virtual txBool IsStop() const TX_NOEXCEPT TX_OVERRIDE { return false; }

  // These functions define the start and end conditions for the object

  /**
   * @brief 返回true
   *
   * @param timeMgr 时间管理器
   * @return txBool
   */
  virtual txBool CheckStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 此函数默认返回true
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  // These functions define the actions that should be performed when the object starts or ends

  virtual void OnStart(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {}
  virtual void OnEnd() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief kill 元素
   *
   * @return txBool
   */
  virtual txBool Kill() TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 获取元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Immovability; }

  /**
   * @brief 获取障碍物类型
   *
   * @return STATIC_ELEMENT_TYPE
   */
  virtual STATIC_ELEMENT_TYPE ObstacleType() const TX_NOEXCEPT { return mObstacleType; }

 public:
  /**
   * @brief 获取长度
   *
   * @return txFloat
   */
  virtual txFloat GetLength() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Length(); }

  /**
   * @brief 获取宽度
   *
   * @return txFloat
   */
  virtual txFloat GetWidth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Width(); }

  /**
   * @brief 获取高度
   *
   * @return txFloat
   */
  virtual txFloat GetHeigth() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Height(); }

  /**
   * @brief 获取包围盒
   *
   * @return const PolygonArray&
   */
  virtual const PolygonArray& Polygon() const TX_NOEXCEPT TX_OVERRIDE { return mGeometryData.Polygon(); }

 public:
  /**
   * @brief 获取当前道路元素的坐标
   *
   * 当道路元素位于一个空间坐标系中时，可以使用这个函数获取元素的坐标。
   *
   * @return 道路元素的坐标
   */
  virtual Coord::txWGS84 GetLocation() const TX_NOEXCEPT TX_OVERRIDE { return GeomCenter(); }

  /**
   * @brief 计算当前元素相对于道路坐标系中心的距离
   *
   * 返回的距离值表示当前道路元素的中心距离道路坐标系中心的距离。
   *
   * @return 返回当前元素相对于道路坐标系中心的距离
   */
  virtual txFloat DistanceAlongCurve() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.DistanceAlongCurve(); }

  /**
   * @brief 获取当前道路元素在当前道路中心与其自身中心之间的距离
   *
   * @return txFloat 当前道路元素在当前道路中心与其自身中心之间的距离
   */
  virtual txFloat LaneOffset() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneOffset(); }

  /**
   * @brief 是否在lanlink上
   *
   * @return txBool
   */
  virtual txBool IsOnLaneLink() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLaneLink(); }

  /**
   * @brief 是否在lane上
   *
   * @return txBool
   */
  virtual txBool IsOnLane() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.IsOnLane(); }

  /**
   * @brief 获取当前车道信息
   *
   * @return const Base::Info_Lane_t&
   */
  virtual const Base::Info_Lane_t& GetCurrentLaneInfo() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LaneLocInfo();
  }

  /**
   * @brief GetCrossRot
   * 获取当前道路元素与自身中心之间的夹角
   * @return Unit::txDegree 夹角值
   */
  virtual Unit::txDegree GetCrossRot() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.CrossRot(); }

  /**
   * @brief 获取当前道路元素的方向向量
   * @return Base::txVec3 当前道路元素的方向向量
   */
  virtual Base::txVec3 GetLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取局部坐标系的位置
   *
   * @return const std::array< Base::txVec3, 2 >
   */
  virtual const std::array<Base::txVec3, 2> LocalCoords() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.LocalCoords();
  }

 public:
  /**
   * @brief Get the Velocity object
   *
   * @return txFloat
   */
  virtual txFloat GetVelocity() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief Get the Acc object
   *
   * @return txFloat
   */
  virtual txFloat GetAcc() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief Get the Displacement object
   *
   * @return txFloat
   */
  virtual txFloat GetDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief Get the Lateral Velocity object
   *
   * @return txFloat
   */
  virtual txFloat GetLateralVelocity() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief Get the Lateral Displacement object
   *
   * @return txFloat
   */
  virtual txFloat GetLateralDisplacement() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat Compute_Displacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE {
    return 0.0;
  }

 public:
  // Declare the virtual functions in the class

  /**
   * 获取该物体元素的唯一ID
   *
   * 该方法用于获取当前物体元素的唯一ID，以便识别和管理物体元素。
   *
   * @return 物体元素的唯一ID
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT TX_OVERRIDE { return mIdentity.Id(); }

  /**
   * @brief TAD_ObstacleElement::StableGeomCenter 获取该物体元素的稳定的几何中心位置
   *
   * 该方法用于获取当前物体元素的稳定的几何中心位置，即在物体元素所有边的均匀分布的位置。
   *
   * @return 返回物体元素的稳定的几何中心位置
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.GeomCenter().GetENU(); }

  /**
   * @brief 获取该物体元素的稳定的后轮辆中心位置
   *
   * 该方法用于获取当前物体元素的稳定的后轮辆中心位置，即该物体元素驾驶时，车辆车辆靠近汽车的位置。
   *
   * @return 返回物体元素的稳定的后轮辆中心位置
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT TX_OVERRIDE { return StableGeomCenter(); }

  /**
   * @brief 获取该物体元素在道路中心线上的稳定位置
   *
   * 该方法用于获取当前物体元素在道路中心线上的稳定位置，即该物体元素在当前车辆驾驶时，距离车辆中心线的位置。
   *
   * @return 返回该物体元素在道路中心线上的稳定位置
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT TX_OVERRIDE {
    return mLocation.PosOnLaneCenterLinePos();
  }

  /**
   * @brief 获取该物体元素的稳定道路位置信息
   *
   * 该方法用于获取当前物体元素在道路上的稳定位置信息，包括道路坐标、速度以及当前所处的车道信息。
   *
   * @return 返回一个包含稳定道路位置信息的结构体
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Base::Info_Lane_t StableLaneInfo() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.LaneLocInfo(); }

  /**
   * @brief 获取该物体元素在道路上的稳定道路方向信息
   *
   * 该方法用于获取当前物体元素在道路上的稳定道路方向信息，包括道路方向。
   *
   * @return 返回一个包含稳定道路方向信息的向量
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Base::txVec3 StableLaneDir() const TX_NOEXCEPT TX_OVERRIDE { return mLocation.vLaneDir(); }

  /**
   * @brief 获取该物体元素在道路上的稳定方向信息
   *
   * 该方法用于获取当前物体元素在道路上的稳定方向信息，包括物体在道路上的平均方向。
   *
   * @return 返回一个表示稳定方向信息的角度值
   * @note 该方法的实现可能会在物体元素的位置发生变化时返回不同的结果
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT TX_OVERRIDE {
    return Utils::GetLaneAngleFromVectorOnENU(mLocation.vLaneDir());
  }

  /**
   * @brief 获取该障碍物元素的稳定速度
   *
   * 该方法用于获取当前障碍物元素在道路上的稳定速度，即障碍物的平均速度。
   *
   * @return 返回表示稳定速度的浮点数值
   * @note 该方法的实现可能会在障碍物元素的位置发生变化时返回不同的结果
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT TX_OVERRIDE { return GetVelocity(); }

  /**
   * @brief 获取当前对象加速度
   *
   * @return txFloat
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT TX_OVERRIDE { return GetAcc(); }

  /**
   * @brief 获取当前对象稳定状态
   *
   */
  virtual void SaveStableState() TX_NOEXCEPT TX_OVERRIDE {}

 public:
  /**
   * @brief Get the Kinetics Info
   *
   * @param timeMgr
   * @return KineticsInfo_t
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief Get the Switch Lane State
   *
   * @return VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT TX_OVERRIDE {
    return VehicleMoveLaneState::eStraight;
  }

  /**
   * @brief 获取障碍物的字符串表示
   *
   * 此函数用于获取障碍物元素的字符串表示，通常为空字符串。
   *
   * @return 返回表示障碍物的字符串
   * @note 此函数可能在障碍物元素位置发生变化时返回不同的结果
   */
  virtual txString Str() const TX_NOEXCEPT TX_OVERRIDE { return ""; }
#if USE_HashedRoadNetwork

 protected:
  // virtual void RegisterInfoOnInit(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 更新哈希键的道路信息
   *
   * 此函数用于更新哈希键的道路信息，以便在当前位置定位道路元素。
   *
   * @param _location 当前位置坐标
   * @return 无
   * @note 此函数可能在道路元素位置发生变化时返回不同的结果
   */
  virtual void UpdateHashedLaneInfo(const Base::Component::Location& _location) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 注销关于即将被终止元素的信息
   *
   * 当元素即将被终止时，此函数将注销与此元素相关的信息。
   *
   * @return 无
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT TX_OVERRIDE {}
#endif /*USE_HashedRoadNetwork*/

  /**
   * @brief 获取车辆行为
   *
   * 该方法返回当前车辆的行为类型。该方法被覆盖为自定义行为。
   *
   * @return 车辆行为类型 (eUserDefined)
   */
  virtual VEHICLE_BEHAVIOR VehicleBehavior() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_BEHAVIOR::eUserDefined; }

  /**
   * @brief 判断当前车辆是否支持特定的行为
   *
   * 此方法用于检查当前车辆是否支持特定的行为。子类可以根据需要覆盖此方法并返回 true，以表示车辆支持该行为。
   *
   * @param behavior 需要检查的行为
   * @return 如果车辆支持该行为，则返回 true；否则返回 false。
   */
  virtual txBool IsSupportBehavior(const VEHICLE_BEHAVIOR behavior) const TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 检查当前是否已经到达死线
   * @return 如果已经到达死线，则返回 true，否则返回 false
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 更新下一个越界元素的交叉方向
   *
   * 此函数用于更新下一个越界元素的交叉方向，以响应本元素的变化。
   */
  virtual void UpdateNextCrossDirection() TX_NOEXCEPT TX_OVERRIDE {}

  /**
   * @brief 获取车辆类型
   *
   * 获取车辆类型，可以用于对车辆进行分类和判断
   *
   * @return VEHICLE_TYPE 返回车辆类型，默认为 undefined
   */
  virtual VEHICLE_TYPE VehicleType() const TX_NOEXCEPT TX_OVERRIDE { return VEHICLE_TYPE::undefined; }

 public:
  /**
   * @brief 获取稳定驾驶状态
   * @return 返回稳定驾驶状态
   */
  virtual IDrivingStatus::DrivingStatus StableDriving_Status() const TX_NOEXCEPT TX_OVERRIDE {
    return IDrivingStatus::DrivingStatus::stop_crash;
  }

 protected:
  // The SubdivideGeom function subdivides a given geometry based on a given subdivision level

  /**
   * @brief 将此障碍物划分为若干个更小的子障碍物
   *
   * 此函数将此障碍物中的某些点或轴进行逐个采样，并将其分割成若干个更小的子障碍物。
   * 可以根据需要调整子障碍物的总数量，但要保证子障碍物的尺寸和各自之间的距离都可以接受。
   * 可能会对其他对象造成影响，所以需要确保对其他对象进行适当的处理。
   */
  virtual void SubdivideGeom() TX_NOEXCEPT;

  /**
   * @brief 将此障碍物划分为若干个更小的子障碍物
   *
   * 此函数将此障碍物中的某些点或轴进行逐个采样，并将其分割成若干个更小的子障碍物。
   * 可以根据需要调整子障碍物的总数量，但要保证子障碍物的尺寸和各自之间的距离都可以接受。
   * 可能会对其他对象造成影响，所以需要确保对其他对象进行适当的处理。
   */
  virtual txBool IsTransparentObject() const TX_NOEXCEPT TX_OVERRIDE;
  static txBool IsTransparentObject(const Base::Enums::STATIC_ELEMENT_TYPE& _obs_type) TX_NOEXCEPT;
  static txBool IsTransparentObject(const Base::txString& _obs_type) TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取当前障碍物在笛卡尔坐标系中的x坐标
   *
   * 返回当前障碍物在笛卡尔坐标系中的x坐标。由于障碍物可能是任意形状，
   * 这个函数将返回一个默认值，但实际应用中可能需要根据某些条件来计算x坐标。
   *
   * @return 当前障碍物在笛卡尔坐标系中的x坐标
   */
  virtual Base::txFloat local_cartesian_point_x() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取当前障碍物在笛卡尔坐标系中的y坐标
   *
   * 返回当前障碍物在笛卡尔坐标系中的y坐标。由于障碍物可能是任意形状，
   * 这个函数将返回一个默认值，但实际应用中可能需要根据某些条件来计算y坐标。
   *
   * @return 当前障碍物在笛卡尔坐标系中的y坐标
   */
  virtual Base::txFloat local_cartesian_point_y() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 获取当前障碍物在笛卡尔坐标系中的z坐标
   *
   * 返回当前障碍物在笛卡尔坐标系中的z坐标。由于障碍物可能是任意形状，
   * 这个函数将返回一个默认值，但实际应用中可能需要根据某些条件来计算z坐标。
   *
   * @return 当前障碍物在笛卡尔坐标系中的z坐标
   */
  virtual Base::txFloat local_cartesian_point_z() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

  /**
   * @brief 设置障碍物的旋转属性。
   *
   * 根据提供的渲染器视图指针，设置障碍物的旋转属性。
   *
   * @param _view_ptr 渲染器视图指针。
   */
  virtual void SetRot(Base::ISceneLoader::IObstacleViewerPtr _view_ptr) TX_NOEXCEPT;

 protected:
  Base::ISceneLoader::IObstacleViewerPtr _elemAttrViewPtr = nullptr;
  Base::ISceneLoader::IRouteViewerPtr _elemRouteViewPtr = nullptr;
  /*Base::Component::Identity mIdentity;
  Base::Component::GeometryData mGeometryData;
  Base::Component::Location mLocation;*/
  Base::Enums::STATIC_ELEMENT_TYPE mObstacleType = _plus_(Base::Enums::STATIC_ELEMENT_TYPE::Box);
  Base::txString mObstacleTypeStr = "Box";
  Base::txInt mObstacleTypeId = 1;
  Unit::txDegree m_rot;
  std::vector<Base::Component::GeometryData::PolygonArray::value_type> m_vec_subdivide_vertex;

 protected:
  AtomicSizeValueType m_index_obstacle_in_pb_without_serialize;
};
using TAD_ObstacleElementPtr = std::shared_ptr<TAD_ObstacleElement>;

class TAD_VertexElement : public TAD_ObstacleElement {
 public:
  TAD_VertexElement() TX_DEFAULT;
  virtual ~TAD_VertexElement() TX_DEFAULT;

 public:
  /**
   * @brief 获取元素类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT TX_OVERRIDE { return ElementType::TAD_Ignore; }

  /**
   * 初始化物体元素
   *
   * 对于实现在 TAD_ObstacleElement 类中的 Initialize 虚函数，用于对物体元素进行初始化操作，包括设置 ID、经度和纬度。
   *
   * @param pViewer 观察者指针
   * @param pSceneLoader 场景加载器指针
   * @return 如果函数调用成功，则返回 true；否则，返回 false
   *
   * @note TX_NOEXCEPT 表示该函数不会抛出异常
   * @note TX_OVERRIDE 表示该函数覆盖了基类的同名虚函数
   */
  virtual txBool Initialize(Base::ISceneLoader::IViewerPtr, Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化物体元素
   *
   * 对于实现在 TAD_ObstacleElement 类中的 Initialize 虚函数，用于对物体元素进行初始化操作，包括设置 ID、经度和纬度。
   *
   * @param _id 物体元素的系统 ID
   * @param _lon 物体元素的经度坐标
   * @param _lat 物体元素的纬度坐标
   * @return 如果函数调用成功，则返回 true；否则，返回 false
   *
   * @note TX_NOEXCEPT 表示该函数不会抛出异常
   */
  virtual txBool Initialize(const Base::txSysId _id, const Base::txFloat _lon, const Base::txFloat _lat) TX_NOEXCEPT;

  /**
   * @brief 更新元素
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool Update(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 释放资源
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 填充Traffic元素
   *
   * @return txBool
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 物体空间查询更新
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * PreFillingElement()
   * 是一个虚函数，在物体互动模块中使用。在初始化子类中需要实现该方法，并在填充物体之前执行预处理操作。
   * 该方法的作用是在物体创建完成后，执行物体填充之前的必要工作。
   *
   * @param[in]  _sizeSize 参数用于设置填充物体的大小。
   * @return    返回true表示物体填充执行成功，返回false表示物体填充执行失败。
   *
   * 注意:
   *  - TX_NOEXCEPT: 表示此函数不会抛出异常
   *  - TX_OVERRIDE: 表示此函数覆盖了基类的同名虚函数
   */
  virtual txBool PreFillingElement(AtomicSize&) TX_NOEXCEPT TX_OVERRIDE { return true; }
};
using TAD_VertexElementPtr = std::shared_ptr<TAD_VertexElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
