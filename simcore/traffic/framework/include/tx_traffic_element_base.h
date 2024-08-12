// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <memory>
#include "tx_header.h"
#include "tx_scene_loader.h"
#ifdef max
#  undef max
#endif  // max
#ifdef min
#  undef min
#endif  // min
#include <boost/atomic.hpp>
#include "tbb/concurrent_hash_map.h"
#include "traffic.pb.h"
#include "tx_enum_def.h"
#include "tx_locate_info.h"
#include "tx_map_tracker.h"
#include "tx_mutex.h"
#include "tx_obb.h"
#include "tx_sim_point.h"
#include "tx_sim_time.h"
#include "tx_topo_def.h"
#include "tx_units.h"
#if USE_HashedRoadNetwork
#  include "HdMap/tx_hashed_lane_info.h"
#endif /*USE_HashedRoadNetwork*/
#include "tx_class_counter.h"
#include "tx_geometry_element_line.h"
#include "tx_hadmap_utils.h"
#include "tx_map_info.h"
#include "tx_serialization.h"
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/
TX_NAMESPACE_OPEN(Base)

// @brief The base class of simulator
class ISimulator {
 public:
  using AtomicSizeValueType = Base::txInt;
  using AtomicSize = std::atomic<AtomicSizeValueType>; /*protobuf repeatedfield index type*/
  using Elem_Id_Type = txSysId;
  struct KineticsInfo_t {
    Elem_Id_Type m_elemId;
    txFloat m_compute_velocity = 0.0;
    txFloat m_acc = 0.0;
    txFloat m_show_velocity = 0.0;
  };

 public:
  virtual ~ISimulator() TX_DEFAULT;

  /**
   * @brief 初始化当前对象
   *
   * @param viewerPtr 用于渲染的用户界面
   * @param sceneLoaderPtr 用于加载场景数据的加载器
   * @return txBool 返回是否成功初始化
   */
  virtual txBool Initialize(ISceneLoader::IViewerPtr, ISceneLoaderPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 更新仿真步
   *
   * @param timeMgr 时间参数
   * @return txBool
   */
  virtual txBool Update(TimeParamManager const &) TX_NOEXCEPT = 0;

  /**
   * @brief 获取动力学信息
   *
   * @param timeMgr 时间
   * @return KineticsInfo_t
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const &timeMgr) const TX_NOEXCEPT = 0;

  /**
   * @brief 释放资源
   *
   * @return txBool
   */
  virtual txBool Release() TX_NOEXCEPT = 0;

  /**
   * @brief PreFillingElement 预处理交通元素大小
   *
   * @param a_size 交通元素大小的引用
   * @return txBool 是否成功预处理交通元素大小
   */
  virtual txBool PreFillingElement(AtomicSize &a_size) TX_NOEXCEPT { return false; }

  /**
   * @brief 获取对象在pb中的索引位置
   *
   * @return AtomicSizeValueType
   */
  virtual AtomicSizeValueType IndexInPb() const TX_NOEXCEPT { return m_index_in_pb_without_serialize; }

  /**
   * @brief FillingElement 获取预计交通元素
   *
   * @param timeParamManager 时间参数管理器
   * @param traffic 交通对象
   * @return txBool 是否成功计算交通元素
   */
  virtual txBool FillingElement(Base::TimeParamManager const &, sim_msg::Traffic &) TX_NOEXCEPT = 0;

  /**
   * 函数 FillingTrajectory
   * @brief 用于获取车辆的预计路径
   * @param [in] timeParamManager 时间参数管理器
   * @param [out] trajectory 车辆的预计路径
   * @return txBool 返回是否成功计算路径
   */
  virtual txBool FillingTrajectory(Base::TimeParamManager const &, sim_msg::Trajectory &) TX_NOEXCEPT { return false; }

  /**
   * @brief 更新轨迹
   *
   * 更新轨迹信息，返回是否成功更新轨迹
   *
   * @param timeParamManager 时间参数管理器
   * @return txBool 是否成功更新轨迹
   */
  virtual txBool UpdateTrajectory(Base::TimeParamManager const &) TX_NOEXCEPT { return false; }

  /**
   * @brief 更新前预处理
   *
   * @param map_elemId2Kinetics
   * @return txBool
   */
  virtual txBool Pre_Update(const TimeParamManager &,
                            std::map<Elem_Id_Type, KineticsInfo_t> &map_elemId2Kinetics) TX_NOEXCEPT = 0;

  /**
   * @brief 更新后处理操作
   *
   * @return txBool
   */
  virtual txBool Post_Update(TimeParamManager const &) TX_NOEXCEPT = 0;

  /**
   * @brief 更新空间检索信息
   *
   * @return txBool
   */
  virtual txBool FillingSpatialQuery() TX_NOEXCEPT = 0;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ISimulator");
  }

 protected:
  AtomicSizeValueType m_index_in_pb_without_serialize;
};
using ISimulatorPtr = std::shared_ptr<ISimulator>;

class IIdentity;
using IIdentityPtr = std::shared_ptr<IIdentity>;

// @brief 系统身份接口
class IIdentity {
 public:
  using ElementType = Base::Enums::ElementType;
  using ElemId2SysIdMap = tbb::concurrent_hash_map<txSysId, txSysId>;
  virtual ~IIdentity() TX_DEFAULT;

  /**
   * @brief 获取当前类型中的id
   *
   * @return txSysId
   */
  virtual txSysId Id() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取类型
   *
   * @return ElementType
   */
  virtual ElementType Type() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取系统id
   *
   * @return txSysId
   */
  virtual txSysId SysId() const TX_NOEXCEPT = 0;
  virtual txString Name() const TX_NOEXCEPT { return ""; }

 public:
  /**
   * @brief 是否同一元素
   *
   * @param leftElemPtr 第一个元素
   * @param rightElemPtr 第二个元素
   * @return txBool true则相同
   */
  static txBool IsTheSameElement(const IIdentityPtr &leftElemPtr, const IIdentityPtr &rightElemPtr) TX_NOEXCEPT;

  /**
   * @brief 是否不是同一元素
   *
   * @param leftElemPtr 第一个元素
   * @param rightElemPtr 第二个元素
   * @return txBool 不是同一元素返回true
   */
  static txBool IsNotTheSameElement(const IIdentityPtr &leftElemPtr, const IIdentityPtr &rightElemPtr) TX_NOEXCEPT;

  /**
   * @brief 创建系统id
   *
   * @param elemId 元素id
   * @return txSysId
   */
  static txSysId CreateSysId(const txSysId elemId) TX_NOEXCEPT;

  /**
   * @brief 系统中已注册元素个数
   *
   * @return txSysId
   */
  static txSysId SysIdCount() TX_NOEXCEPT;
  static txSysId SysRootId() TX_NOEXCEPT;

  /**
   * @brief 重置系统id
   *
   */
  static void ResetSysId() TX_NOEXCEPT;

  /**
   * @brief 注册元素
   *
   * @param _sysId
   * @param elemId
   * @return txSysId
   */
  static txSysId RegisterElementId(const txSysId _sysId, const txSysId elemId) TX_NOEXCEPT;

  /**
   * @brief 根据元素id获取其系统id
   *
   * @param[in] elemId 元素id
   * @param[out] resSysId 返回的系统id
   * @return txBool
   */
  static txBool GetSysIdByElemId(const txSysId elemId, txSysId &resSysId) TX_NOEXCEPT;

 protected:
  static boost::atomic<txSysId> s_sys_id_base;
  static ElemId2SysIdMap s_elemId2SysId;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IIdentity");
  }
};

// @brief 声明周期接口类
class ILiftCycle {
 public:
  virtual ~ILiftCycle() TX_DEFAULT;

  /**
   * @brief 是否活跃
   *
   * @return txBool
   */
  virtual txBool IsAlive() const TX_NOEXCEPT = 0;

  /**
   * @brief 生命是否结束
   *
   * @return txBool
   */
  virtual txBool IsEnd() const TX_NOEXCEPT = 0;

  /**
   * @brief 生命是否开始
   *
   * @return txBool
   */
  virtual txBool IsStart() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否有效
   *
   * @return txBool
   */
  virtual txBool IsValid() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否停止
   *
   * @return txBool
   */
  virtual txBool IsStop() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否满足开始仿真条件
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT = 0;

  // 只有障碍物有endtime(落石、轮胎等)。
  /**
   * @brief 是否满足仿真结束条件
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckEnd(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 被动结束仿真
   *
   * @return txBool
   */
  virtual txBool Kill() TX_NOEXCEPT = 0;
  virtual void OnStart(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT = 0;
  virtual void OnEnd() TX_NOEXCEPT = 0;

  /**
   * @brief 检查是否激活
   *
   * @param timeMgr
   * @return txBool
   */
  virtual txBool CheckAlive(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT { return IsAlive(); }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ILiftCycle");
  }
};

// @brief 几何信息管理接口
class IGeometry {
 public:
  using PolygonArray = Geometry::OBB2D::PolygonArray;
  virtual ~IGeometry() TX_DEFAULT;

  /**
   * @brief Get the Length object
   *
   * @return txFloat
   */
  virtual txFloat GetLength() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Width object
   *
   * @return txFloat
   */
  virtual txFloat GetWidth() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Heigth object
   *
   * @return txFloat
   */
  virtual txFloat GetHeigth() const TX_NOEXCEPT = 0;
  virtual const PolygonArray &Polygon() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Polygon object获取包围盒
   *
   * @return const PolygonArray&
   */
  virtual const PolygonArray &GetPolygon() const TX_NOEXCEPT { return Polygon(); }

  /**
   * @brief 是否转为上级对象
   *
   * @return txBool
   */
  virtual txBool IsTransparentObject() const TX_NOEXCEPT { return false; }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IGeometry");
  }
};

// @brief 交通元素与地图交互接口
class IMapLocation {
 public:
  virtual ~IMapLocation() TX_DEFAULT;

  /**
   * @brief 获取元素经纬度
   *
   * @return Coord::txWGS84
   */
  virtual Coord::txWGS84 GetLocation() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取沿曲线的距离
   *
   * 该函数用于计算当前对象沿曲线的沿线位置
   *
   * @return txFloat 当前对象沿曲线的距离
   */
  virtual txFloat DistanceAlongCurve() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车道的偏移
   *
   * @return txFloat
   */
  virtual txFloat LaneOffset() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否在lanlink
   *
   * @return txBool
   */
  virtual txBool IsOnLaneLink() const TX_NOEXCEPT = 0;

  /**
   * @brief 是否在lane
   *
   * @return txBool
   */
  virtual txBool IsOnLane() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Current Lane Info object
   *
   * @return const Info_Lane_t&
   */
  virtual const Info_Lane_t &GetCurrentLaneInfo() const TX_NOEXCEPT = 0;
  virtual const Info_Road_t GetCurrentRoadInfo() const TX_NOEXCEPT { return GetCurrentLaneInfo().toRoadLocInfo(); }
  virtual Unit::txDegree GetCrossRot() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车道线方向
   *
   * @return txVec3
   */
  virtual txVec3 GetLaneDir() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前对象的高度
   *
   * @return txFloat
   */
  virtual txFloat Altitude() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取当前元素的局部坐标范围
   *
   * 该函数用于获取当前元素在其父元素内的局部坐标范围。坐标范围是由两个向量表示的，第一个向量表示局部坐标系中的左下角坐标，第二个向量表示右上角的坐标。
   *
   * @return std::array<txVec3, 2> 当前元素的局部坐标范围，以左下角为第一个向量，右上角为第二个向量
   */
  virtual const std::array<txVec3, 2> LocalCoords() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取车道方向角度信息
   *
   * 该函数获取车辆当前所在车道的方向角度，并以度为单位返回。函数中使用了一个辅助函数
   * `GetLaneAngleFromVectorOnENU`，该函数接收一个向量作为输入，表示车道向量在参考极坐标系下的沿横向向量和纵向向量。
   *
   * @param GetLaneDir 获取车道向量信息的方法
   * @return 车道方向角度
   */
  virtual Unit::txDegree GetHeadingWithAngle() const TX_NOEXCEPT {
    return Utils::GetLaneAngleFromVectorOnENU(GetLaneDir());
  }

  /**
   * @brief 根据车道方向和偏移量计算车辆在车道中的位置
   *
   * 该函数根据车道方向和偏移量计算车辆在车道中的位置。输入参数包括车辆当前的坐标、车道方向向量和偏移量，输出为计算后的坐标。
   *
   * @param srcPos   车辆当前的坐标
   * @param vLaneDir 车道方向向量
   * @param offset   偏移量
   * @return Coord::txWGS84 计算后的坐标
   */
  static Coord::txWGS84 ComputeLaneOffset(Coord::txWGS84 srcPos, const Base::txVec3 &vLaneDir,
                                          const Base::txFloat offset) TX_NOEXCEPT;

 protected:
  static Base::txFloat GenerateST(const txFloat _s, const txFloat _distanceCurve) TX_NOEXCEPT;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IMapLocation");
  }
};

#if USE_HashedRoadNetwork
class IVehicleElement;
using IVehicleElementPtr = std::shared_ptr<IVehicleElement>;
class ISpatialRegister {
 public:
  using VehicleContainer = std::map<Base::txSysId, Base::IVehicleElementPtr>;
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoPtr = Geometry::SpatialQuery::HashedLaneInfoPtr;
  using MapTrackerPtr = HdMap::MapTrackerPtr;
  ISpatialRegister() {
    /*eFront = 0, eBack = 1, eLeft = 2, eLeftFront = 3, eLeftBack = 4, eRight = 5, eRightFront = 6, eRightBack = 7)*/
    /*
    |------------|------------|------------|
    |    0  0    |   1 0      |   2 0      |
    | eLeftFront | eFront     | eRightFront|
    |------------|------------|------------|
    |    0 1     |   1 1      |   2 1      |
    |   eLeft    | eFront     | eRight     |
    |------------|------------|------------|
    |    0 2     |   1 2      |   2 2      |
    | eLeftBack  |   eBack    | eRightBack |
    |------------|------------|------------|
    */
    using Direction = Base::Enums::NearestElementDirection;
    m_xy2jdir_without_serialize[0][0] = _plus_(Direction::eLeftFront);
    m_xy2jdir_without_serialize[1][0] = _plus_(Direction::eFront);
    m_xy2jdir_without_serialize[2][0] = _plus_(Direction::eRightFront);

    m_xy2jdir_without_serialize[0][1] = _plus_(Direction::eLeft);
    m_xy2jdir_without_serialize[1][1] = _plus_(Direction::eFront);
    m_xy2jdir_without_serialize[2][1] = _plus_(Direction::eRight);

    m_xy2jdir_without_serialize[0][2] = _plus_(Direction::eLeftBack);
    m_xy2jdir_without_serialize[1][2] = _plus_(Direction::eBack);
    m_xy2jdir_without_serialize[2][2] = _plus_(Direction::eRightBack);
  }
  virtual ~ISpatialRegister() TX_DEFAULT;

  /**
   * @brief 初始化本地坐标系
   *
   * 这个函数用于初始化当前道路的本地坐标系，以方便后续计算。
   *
   * @param _length 当前道路的长度，单位为米
   * @param _width 当前道路的宽度，单位为米
   */
  virtual void InitLocalCoord_SUDOKU_GRID(const txFloat _length, const txFloat _width) TX_NOEXCEPT {
    m_local_coord_x_axis[0] = -1.5 * FLAGS_default_lane_width;
    m_local_coord_x_axis[1] = -0.5 * FLAGS_default_lane_width;
    m_local_coord_x_axis[2] = 0.5 * FLAGS_default_lane_width;
    m_local_coord_x_axis[3] = 1.5 * FLAGS_default_lane_width;

    m_local_coord_y_axis[0] = 0.5 * (_length + FLAGS_DETECT_OBJECT_DISTANCE);
    m_local_coord_y_axis[1] = 0.5 * _length;
    m_local_coord_y_axis[2] = -0.5 * _length;
    m_local_coord_y_axis[3] = -0.5 * (_length + FLAGS_DETECT_OBJECT_DISTANCE);
  }

  /**
   * @brief 初始化空间注册
   *
   * @param _location
   */
  virtual void RegisterInfoOnInit(const Component::Location &_location) TX_NOEXCEPT = 0;

  /**
   * @brief 定位发生变化更新定位
   *
   * @param _location
   */
  virtual void UpdateHashedLaneInfo(const Component::Location &_location) TX_NOEXCEPT = 0;

  /**
   * @brief 元素结束仿真反注册接口
   *
   */
  virtual void UnRegisterInfoOnKill() TX_NOEXCEPT = 0;

 protected:
  HashedLaneInfo m_curHashedLaneInfo;
  boost::optional<HashedLaneInfo> m_op_LaneChangeHashedLaneInfo;

  std::array<Base::txFloat, 4> m_local_coord_x_axis;
  std::array<Base::txFloat, 4> m_local_coord_y_axis;
  std::array<std::array<Base::Enums::NearestElementDirection, 3>, 3> m_xy2jdir_without_serialize;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ISpatialRegister");
    archive(_MAKE_NVP_("curHashedLaneInfo", (m_curHashedLaneInfo)),
            _MAKE_NVP_("op_LaneChangeHashedLaneInfo", (m_op_LaneChangeHashedLaneInfo)),
            _MAKE_NVP_("m_local_coord_x_axis", (m_local_coord_x_axis)),
            _MAKE_NVP_("m_local_coord_y_axis", (m_local_coord_y_axis)));
  }
};
#endif /*USE_HashedRoadNetwork*/

class IKinetics {
 public:
  virtual ~IKinetics() TX_DEFAULT;
  virtual txFloat GetShowVelocity() const TX_NOEXCEPT { return GetVelocity(); }

  /**
   * @brief Get the Velocity
   *
   * @return txFloat
   */
  virtual txFloat GetVelocity() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Raw Velocity
   *
   * @return txFloat
   */
  virtual txFloat GetRawVelocity() const TX_NOEXCEPT { return GetVelocity(); }

  /**
   * @brief Get the Acc
   *
   * @return txFloat
   */
  virtual txFloat GetAcc() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Lateral Acc
   *
   * @return txFloat
   */
  virtual txFloat GetLateralAcc() const TX_NOEXCEPT { return 0.0; }

  /**
   * @brief Get the Displacement
   *
   * @return txFloat
   */
  virtual txFloat GetDisplacement() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Lateral Velocity
   *
   * @return txFloat
   */
  virtual txFloat GetLateralVelocity() const TX_NOEXCEPT = 0;

  /**
   * @brief Get the Lateral Displacement
   *
   * @return txFloat
   */
  virtual txFloat GetLateralDisplacement() const TX_NOEXCEPT = 0;

  /**
   * @brief 计算对象的位移
   *
   * @param timeMgr
   * @return txFloat
   */
  virtual txFloat Compute_Displacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 计算对象的横向位移
   * @param timeMgr 时间参数管理器
   * @return 返回横向位移的值
   */
  virtual txFloat Compute_LateralDisplacement(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT = 0;

  /**
   * @brief 获取元素的原始速度愿望
   * @return 返回元素的原始速度愿望值，单位是元素速度的单位
   */
  virtual txFloat GetRawVelocityDesired() const TX_NOEXCEPT { return 0.0; }

  /**
   * @brief 获取现在的速度愿望
   * @return 返回元素现在的速度愿望值，单位是元素速度的单位
   */
  virtual txFloat GetNowVelocityDesired() const TX_NOEXCEPT { return 0.0; }

  /**
   * @brief SetVelocityDesired
   * @param _new_velocity_desired 新的速度期望值
   */
  virtual void SetVelocityDesired(const txFloat _new_velocity_desired) TX_NOEXCEPT {}

  /**
   * @brief 重置速度愿望
   *
   * 该方法会将速度愿望重置为初始值，通常在应用程序启动或重启时使用。
   */
  virtual void ResetVelocityDesired() TX_NOEXCEPT {}

  /**
   * @brief 是否支持速度控制
   * @return true 支持速度控制；false 不支持速度控制
   */
  virtual txBool SupportVelocityControled() const TX_NOEXCEPT { return false; }

  /**
   * @brief 获取元素是否可以控制速度
   * @return true 可以控制速度；false 不可以控制速度
   */
  virtual txBool IsVelocityControled() const TX_NOEXCEPT { return false; }
  virtual void SetVelocityControled(txBool _f) TX_NOEXCEPT {}

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IKinetics");
  }
};

class ISwitchLane {
 public:
  using VehicleMoveLaneState = Base::Enums::VehicleMoveLaneState;
  virtual ~ISwitchLane() TX_DEFAULT;

  /**
   * @brief 获取切换车道状态
   *
   * 该函数用于获取车辆的切换车道状态。
   *
   * @return 切换车道状态，具体取值见 #VehicleMoveLaneState
   */
  virtual VehicleMoveLaneState GetSwitchLaneState() const TX_NOEXCEPT = 0;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ISwitchLane");
  }
};

class ISerialize {
 public:
  virtual ~ISerialize() TX_DEFAULT;

  /**
   * @brief txTrafficElementBase::binary_archive
   * @return 返回二进制归档的字符串表示形式。
   */
  virtual txString binary_archive() const TX_NOEXCEPT { return ""; }

  /**
   * @brief 返回xml归档字符串表示形式
   *
   * @return txString
   */
  virtual txString xml_archive() const TX_NOEXCEPT { return ""; }

  /**
   * @brief 返回json归档字符串表示形式
   *
   * @return txString
   */
  virtual txString json_archive() const TX_NOEXCEPT { return ""; }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ISerialize");
  }
};

class IStream {
 public:
  virtual ~IStream() TX_DEFAULT;

  /**
   * @brief 获取字符串形式的对象
   *
   * 此函数用于获取对象的字符串形式。
   *
   * @return 返回一个字符串对象，通常用于日志记录和输出
   */
  virtual txString Str() const TX_NOEXCEPT = 0;
  friend std::ostream &operator<<(std::ostream &os, const IStream &v) TX_NOEXCEPT {
    os << v.Str();
    return os;
  }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IStream");
  }
};

class ISimulationConsistency {
 public:
  using DrivingStatus = Base::Enums::DrivingStatus;

 public:
  virtual ~ISimulationConsistency() TX_DEFAULT;

  /**
   * @brief 获取一致性ID
   * @return 返回一致性ID
   */
  virtual txSysId ConsistencyId() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定的几何中心
   * @return 返回稳定的几何中心坐标
   */
  virtual Coord::txENU StableGeomCenter() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定的后车辆几何中心坐标
   * @return 返回稳定的后车辆几何中心坐标
   */
  virtual Coord::txENU StableRearAxleCenter() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定在道路上的位置
   * @return 返回稳定在道路上的坐标
   */
  virtual Coord::txWGS84 StablePositionOnLane() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定在道路上的信息
   * @return 返回稳定在道路上的坐标信息
   */
  virtual Info_Lane_t StableLaneInfo() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定在道路上的方向
   * @return 返回稳定在道路上的方向向量
   */
  virtual txVec3 StableLaneDir() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定的方向
   * @return 稳定的方向值，以度数表示
   */
  virtual Unit::txDegree StableHeading() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定的速度值
   * @return 稳定的速度值，以小数表示
   */
  virtual txFloat StableVelocity() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定的加速度值
   * @return 稳定的加速度值，以小数表示
   */
  virtual txFloat StableAcc() const TX_NOEXCEPT = 0;

  /**
   * @brief 保存稳定状态
   *
   * 保存当前对象的稳定状态，以便在不同的时间点恢复使用。
   *
   */
  virtual void SaveStableState() TX_NOEXCEPT = 0;

  /**
   * @brief 获取一致性元素类型
   *
   * 这个函数返回当前对象的一致性元素类型。这个类型是由 IIdentity::ElementType 枚举定义的。
   *
   * @return 当前对象的一致性元素类型
   */
  virtual IIdentity::ElementType ConsistencyElementType() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定驾驶状态
   *
   * 该函数返回当前对象的稳定驾驶状态。该状态的取值类型为 DrivingStatus，该类型是由枚举定义的。
   *
   * @return 当前对象的稳定驾驶状态
   */
  virtual DrivingStatus StableDriving_Status() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取稳定驾驶状态
   *
   * 该函数返回当前对象的稳定驾驶状态。该状态的取值类型为布尔值，即 txBool，表示当前驾驶状态是否稳定。
   *
   * @return txBool 当前对象的稳定驾驶状态
   */
  virtual txBool IsStableLaneKeep() const TX_NOEXCEPT { return true; }

  /**
   * @brief 检查当前方向是否为稳定左转
   *
   * 此函数返回一个布尔值，表示当前对象的方向是否稳定在左转状态。
   * 当当前对象的状态表明它正在左转时，该函数返回 `true`；否则返回 `false`。
   *
   * @return txBool 当前对象的方向是否稳定在左转状态
   */
  virtual txBool IsNotStableTurnLeft() const TX_NOEXCEPT { return true; }

  /**
   * @brief 判断当前方向是否为稳定右转
   *
   * 该函数返回一个布尔值，表示当前对象的方向是否稳定在右转状态。
   * 当当前对象的状态表明它正在右转时，该函数返回 `true`；否则返回 `false`。
   *
   * @return txBool 当前对象的方向是否稳定在右转状态
   */
  virtual txBool IsNotStableTurnRight() const TX_NOEXCEPT { return true; }

  /**
   * @brief 获取稳定的哈希的道路信息
   *
   * 该函数返回一个包含道路信息的哈希值，表示当前对象的方向是否稳定。
   * 当道路的信息确定且不会改变时，该函数返回一个哈希值；否则返回0。
   *
   * @return const Geometry::SpatialQuery::HashedLaneInfo & 稳定的哈希的道路信息
   */
  virtual const Geometry::SpatialQuery::HashedLaneInfo &StableHashedLaneInfo() const TX_NOEXCEPT = 0;

#if __Meeting__

 public:
  /**
   * @brief 获取稳定的跟随前车ID
   *
   * 该函数返回一个整数值，表示当前对象的稳定跟随前车ID。
   *
   * @return txSysId 稳定的跟随前车ID
   */
  virtual txSysId StableFollowFrontId() const TX_NOEXCEPT { return mFollowFrontId; }

  /**
   * @brief 保存稳定跟随前车 ID
   *
   * 此函数用于保存当前对象的稳定跟随前车 ID。
   *
   * @param id 前端 ID，即跟随的前车 ID
   */
  virtual void SaveStableFollowFrontId(txSysId id) TX_NOEXCEPT { mFollowFrontId = id; }

 protected:
  txSysId mFollowFrontId = -1;
#endif /*__Meeting__*/

#if __JunctionYield__

 public:
  /**
   * @brief 获取稳定的轨迹对象
   *
   * 该函数用于获取当前对象的稳定轨迹对象，当前对象的稳定轨迹是指在代表此对象中描述的时间范围内的最稳定轨迹。
   *
   * @return 返回一个指向稳定轨迹对象的指针，该对象的类型为
   * Geometry::Element::txGeomElementBasePtr，通常用于日志记录和输出
   */
  virtual Geometry::Element::txGeomElementBasePtr StableTrajectory() const TX_NOEXCEPT { return mGeomTrajPtr; }

  /**
   * @brief 获取稳定的长轴轨迹线段
   *
   * 该函数用于获取当前对象的稳定长轴轨迹线段，当前对象的稳定长轴轨迹是指在代表此对象中描述的时间范围内的最稳定长轴轨迹。
   *
   * @return 返回一个指向稳定长轴轨迹线段的指针，该线段的类型为 Geometry::Element::txLinePtr，通常用于日志记录和输出
   */
  virtual Geometry::Element::txLinePtr StableLongitudinalSkeleton() const TX_NOEXCEPT {
    return mGeomLongSkeletionLinePtr;
  }

  /**
   * @brief 获取稳定的反向距离
   *
   * 该函数用于获取当前对象的稳定反向距离。这个值可以表示在代表此对象中描述的时间范围内的最稳定的反向距离。
   *
   * @return 返回一个表示稳定反向距离的txFloat浮点数，通常用于日志记录和输出
   */
  virtual txFloat StableInvertDistanceAlongCurve() const TX_NOEXCEPT = 0;

  /**
   * @brief 返回当前对象在代表此对象中描述的时间范围内的最稳定轨迹的浮点数
   *
   * 返回一个表示当前对象在代表此对象中描述的时间范围内的最稳定轨迹的浮点数，
   * 通常用于日志记录和输出。具体来说，该值表示当前对象以某个速度沿着曲线运动时，
   * 该曲线的长度不变的距离，在代表此对象中描述的时间范围内。
   *
   * @return 当前对象在代表此对象中描述的时间范围内的最稳定轨迹的浮点数
   * @retval 大于等于零的正数 当前对象以某个速度沿着曲线运动时，
   *         该曲线的长度不变的距离。
   * @retval -1 表示某些错误情况发生。
   */
  virtual txFloat StableDistanceAlongCurve() const TX_NOEXCEPT = 0;

  /**
   * @brief 清除当前对象的几何轨迹和长轨迹骨架线。
   *
   * 清除当前对象存储的几何轨迹和长轨迹骨架线指针，并将指针置为空。
   */
  virtual void ClearGeomCurve() TX_NOEXCEPT {
    mGeomTrajPtr = nullptr;
    mGeomLongSkeletionLinePtr = nullptr;
  }

 protected:
  Geometry::Element::txGeomElementBasePtr mGeomTrajPtr = nullptr;
  Geometry::Element::txLinePtr mGeomLongSkeletionLinePtr = nullptr;
#endif /*__JunctionYield__*/
  virtual void ConsistencyClear() TX_NOEXCEPT { m_ConsistencyStr.str(""); }

  /**
   * @brief 返回一个字符串，表示当前对象的一致性信息
   *
   * 返回一个字符串，表示当前对象的一致性信息。具体来说，这个字符串应该反映该对象在代表该对象的时间范围内，与其他对象的关系的一致性。
   *
   * @return 返回一个字符串，表示当前对象的一致性信息
   */
  virtual txString ConsistencyStr() const TX_NOEXCEPT;

  /**
   * @brief 添加一个字符串到当前对象的一致性数据
   *
   * 在当前对象的一致性数据中添加一个字符串。这个字符串必须在代表该对象的时间范围内与其他对象的关系保持一致。
   *
   * @param str 要添加的字符串
   */
  virtual void ConsistencyAppend(const txString &str) TX_NOEXCEPT;

 public:
  /**
   * @brief 返回当前对象的一致性快照
   *
   * 返回一个字符串，表示当前对象的一致性快照。该快照包含了当前对象在代表该对象的时间范围内与其他对象的关系信息。
   *
   * @return 返回一个字符串，表示当前对象的一致性快照
   */
  virtual txString ConsistencySnapshot() const TX_NOEXCEPT { return m_ConsistencyStr.str(); }

  /**
   * @brief 重置一致性状态
   *
   * 当需要重置一致性状态时，调用此函数。它会清除当前对象与其他对象之间的一致性关系。
   *
   */
  virtual void ResetConsistency() TX_NOEXCEPT {
    ConsistencyClear();
    ConsistencyAppend(ConsistencyStr());
  }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ISimulationConsistency");
  }
  std::ostringstream m_ConsistencyStr;
};
using SimulationConsistencyPtr = std::shared_ptr<ISimulationConsistency>;
using WEAK_SimulationConsistencyPtr = std::weak_ptr<ISimulationConsistency>;

class ILocalCoord {
 public:
  struct FrenetProjInfo {
    Base::txBool mIsValid = false;
    Base::Enums::Element_Spatial_Relationship mTarget2Source =
        _plus_(Base::Enums::Element_Spatial_Relationship::eOverlap); /*eFront:target front of source*/
    Base::txFloat mProjDistance = 0;
  };

 public:
  ILocalCoord() { m_TransMatInv.setZero(); }
  virtual ~ILocalCoord() TX_DEFAULT;

  /**
   * @brief ComputeProjectionMat 根据两个点计算投影矩阵
   *
   * 此函数计算以两个点（p0, p1）为顶点的矩阵，将其存储在公共成员变量 m_projectionMat 中。
   *
   * @param p0 第一个点的坐标
   * @param p1 第二个点的坐标
   */
  virtual void ComputeProjectionMat(const Base::txVec2 &p0, const Base::txVec2 &p1) TX_NOEXCEPT;

  /**
   * @brief IsPtInSegment 判断给定点是否在给定线段上
   *
   * 返回 true 表示给定点在线段上，false 表示在线段外。
   *
   * @param pt 待判断的点
   * @param segHead 线段的起点
   * @param segTrail 线段的终点
   * @return 返回 true 表示给定点在线段上，false 表示在线段外
   */
  virtual Base::txBool IsPtInSegment(const Base::txVec2 &pt, const Base::txVec2 &segHead,
                                     const Base::txVec2 &segTrail) const TX_NOEXCEPT;

  /**
   * @brief ComputeProjectPt 计算点到线段的投影点
   *
   * 此函数计算点q在线段上的投影点。
   *
   * @param q 要计算投影点的点
   * @return 返回点q在线段上的投影点
   */
  virtual Base::txVec2 ComputeProjectPt(const Base::txVec2 &q) const TX_NOEXCEPT;

  /**
   * @brief 计算投影点到目标多边形集合的投影点的最小距离
   *
   * 此函数计算源多边形集合中某一个点的投影点到目标多边形集合的投影点的最小距离。
   *
   * @param target_vecq 目标多边形集合
   * @param source_vecq 源多边形集合
   * @return 返回源多边形集合中某一个点的投影点到目标多边形集合的投影点的最小距离
   */
  virtual Base::txFloat ComputeProjectDistance(const Geometry::OBB2D::PolygonArray &target_vecq,
                                               const Geometry::OBB2D::PolygonArray &source_vecq) const TX_NOEXCEPT;

  /**
   * @brief ComputeRoadProjectDistance 计算目标元素在当前元素上的投影点
   *
   * 该函数通过将目标元素的坐标坐标点投影到当前元素的基准矩形上计算投影点，并返回投影点的投影坐标和投影方向。
   *
   * @param target_element_geom_center 目标元素的坐标点
   * @return FrenetProjInfo 返回投影点的投影坐标和投影方向
   */
  virtual FrenetProjInfo ComputeRoadProjectDistance(const Coord::txENU &target_element_geom_center) const TX_NOEXCEPT {
    return FrenetProjInfo();
  }

  /**
   * @brief txTrafficElementBase 的 TransMatInv 函数
   * 返回当前元素的坐标变换矩阵的逆矩阵。
   *
   * @return 返回当前元素的坐标变换矩阵的逆矩阵.
   */
  virtual const Base::txMat2 &TransMatInv() const TX_NOEXCEPT { return m_TransMatInv; }

  /**
   * @brief RelativePosition 返回与一个中心点对应的轮廓对象在坐标变换矩阵的逆矩阵下的相对位置.
   *
   * 该函数接受两个输入:
   * - center2d: 中心点的坐标 (x, y)
   * - obj2d: 需要计算相对位置的轮廓对象的坐标 (x, y)
   * - tranMatInv: 坐标变换矩阵的逆矩阵
   *
   * 函数返回一个包含两个值的向量:
   * - x: 相对于中心点的x坐标
   * - y: 相对于中心点的y坐标
   *
   * @param center2d 中心点的坐标
   * @param obj2d 轮廓对象的坐标
   * @param tranMatInv 坐标变换矩阵的逆矩阵
   * @return Base::txVec2 相对位置的向量
   */
  static Base::txVec2 RelativePosition(const Base::txVec2 &center2d, const Base::txVec2 obj2d,
                                       const Base::txMat2 &tranMatInv) TX_NOEXCEPT;

 protected:
  /**
   * @brief 计算局部坐标变换矩阵的逆矩阵
   *
   * 该函数计算局部坐标变换矩阵的逆矩阵，使得_dir指向的局部坐标系中的点能够被映射到坐标系中心点。
   *
   * @param[in] _geo_center 坐标系中心点的坐标
   * @param[in] _dir 指向局部坐标系中心点的坐标
   * @return 无返回值
   */
  virtual void ComputeLocalCoordTransMatInv(const Base::txVec2 &_geo_center, const Base::txVec3 &_dir) TX_NOEXCEPT;

 protected:
  Base::txVec2 m_p0, m_p1;
  Base::txMat2 m_invMat;
  Base::txFloat m_rhs_1;
  Base::txMat2 m_TransMat_UnSerialize, m_TransMatInv;

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ILocalCoord");
    /*archive(m_p0, m_p1, m_invMat, m_rhs_1);*/
    archive(_MAKE_NVP_("p0", m_p0));
    archive(_MAKE_NVP_("p1", m_p1));
    archive(_MAKE_NVP_("invMat", m_invMat));
    archive(_MAKE_NVP_("rhs_1", m_rhs_1));
    archive(_MAKE_NVP_("TransMatInv", m_TransMatInv));
  }
};
using LocalCoordPtr = std::shared_ptr<ILocalCoord>;

class IDrivingStatus {
 public:
  using DrivingStatus = Base::Enums::DrivingStatus;

 public:
  virtual ~IDrivingStatus() TX_DEFAULT;

  /**
   * @brief 获取车辆的行驶状态
   *
   * 该函数用于获取车辆的当前行驶状态。
   *
   * @return 返回当前行驶状态
   */
  virtual DrivingStatus Driving_Status() const TX_NOEXCEPT { return mDrivingStatus; }

  /**
   * @brief 设置车辆的行驶状态
   *
   * 此函数用于设置车辆的行驶状态，通常在通信链路连接正常时使用。
   *
   * @param _status 要设置的行驶状态
   * @return 返回设置成功的行驶状态
   */
  virtual DrivingStatus SetDrivingStatus(const DrivingStatus _status) TX_NOEXCEPT {
    mDrivingStatus = _status;
    return Driving_Status();
  }

  /**
   * @brief 检查驾驶状态
   * @param  _status 待检查的驾驶状态
   * @return 如果当前驾驶状态与待检查的驾驶状态相同，则返回 true，否则返回 false
   */
  virtual txBool CheckDrivingStatus(const DrivingStatus _status) TX_NOEXCEPT { return (_status == mDrivingStatus); }

  /**
   * @brief 获取驾驶状态字符串
   *
   * 获取当前驾驶状态，并返回相应的字符串表示。
   * @return 包含驾驶状态字符串的引用
   */
  txString StrDrivingStatus() const TX_NOEXCEPT { return txString(__enum2lpsz__(DrivingStatus, mDrivingStatus)); }

  /**
   * @brief 清除驾驶状态
   *
   * 清除当前驾驶状态，将其设置为未知状态。
   */
  virtual void ClearDrivingStatus() TX_NOEXCEPT { mDrivingStatus = _plus_(DrivingStatus::unknown); }

 protected:
  DrivingStatus mDrivingStatus = _plus_(DrivingStatus::unknown);
  DrivingStatus mLastDrivingStatus = _plus_(DrivingStatus::unknown);

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("IDrivingStatus");
    archive(_MAKE_NVP_("DrivingStatus", mDrivingStatus));
  }
};

class IMapRange {
 public:
  /**
   * @brief 判断经纬度是否在指定范围内
   *
   * 该函数用于判断给定的经纬度（dLng，dLat）是否在指定的经纬度范围内。
   *
   * @param[in] dLng 经度值
   * @param[in] dLat 纬度值
   * @return 如果经纬度在指定范围内则返回 true，否则返回 false
   */
  static Base::txBool InRanges(const Base::txFloat dLng, const Base::txFloat dLat) TX_NOEXCEPT;

  /**
   * @brief 设置地图范围
   *
   * 此函数用于设置一组地图范围。
   *
   * @param[in] _map_ranges 待设置的地图范围列表
   */
  static void SetMapRanges(const std::vector<Base::map_range_t> &_map_ranges) TX_NOEXCEPT {
    s_vec_map_range = _map_ranges;
  }

 protected:
  static std::vector<Base::map_range_t> s_vec_map_range;
};

class ITrafficElement;
TX_MARK("forward declaration");
using ITrafficElementPtr = std::shared_ptr<ITrafficElement>;
using ITrafficElementConstPtr = std::shared_ptr<const ITrafficElement>;
using WEAK_ITrafficElementPtr = std::weak_ptr<ITrafficElement>;

#if __FocusVehicle__
class IFocusElement {
 public:
  using FocusVehicleContainer = std::map<Base::txSysId, WEAK_ITrafficElementPtr>;
  virtual Base::txSize AddFocusVehicle(const Base::txSysId _sysId, WEAK_ITrafficElementPtr weak_ptr) TX_NOEXCEPT {
    m_weak_external_focus_vehicle_map[_sysId] = weak_ptr;
    return m_weak_external_focus_vehicle_map.size();
  }

  /**
   * @brief 获取关注车辆集合
   *
   * 此函数用于获取关注度的车辆集合。
   *
   * @return 关注度的车辆集合
   */
  virtual FocusVehicleContainer GetFocusVehicles() const TX_NOEXCEPT { return m_weak_external_focus_vehicle_map; }

  /**
   * @brief 清除关注车辆
   *
   * 此函数用于清除关注度的车辆集合。
   */
  virtual void ClearFocusVehicles() TX_NOEXCEPT { m_weak_external_focus_vehicle_map.clear(); }

  /**
   * @brief 计算关注车辆
   *
   * 此函数用于计算具有关注度的车辆。
   *
   * @param[in] timeMgr 时间参数管理器
   */
  virtual void ComputeFocusVehicles(Base::TimeParamManager const &timeMgr) TX_NOEXCEPT {}

 protected:
  FocusVehicleContainer m_weak_external_focus_vehicle_map;
};
using IFocusElementPtr = std::shared_ptr<IFocusElement>;
#endif /*__FocusVehicle__*/

class ITrafficElement : public ISimulator,
                        public IIdentity,
                        public ILiftCycle,
                        public IGeometry,
                        public IKinetics,
                        public ISwitchLane,
                        public IMapLocation,
                        public ISerialize,
                        public IStream,
                        public IMutexTBB,
                        public ISimulationConsistency,
                        public ILocalCoord,
#if USE_HashedRoadNetwork
                        public ISpatialRegister,
#endif /*USE_HashedRoadNetwork*/
                        public std::enable_shared_from_this<ITrafficElement>,
                        public IDrivingStatus,
#if __FocusVehicle__
                        public IFocusElement,
#endif /*__FocusVehicle__*/
                        public IMapRange {
  Utils::Counter<ITrafficElement> _c;

 public:
  using ElementType = Base::Enums::ElementType;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txFloat = Base::txFloat;
  using txSysId = Base::txSysId;
  using txString = Base::txString;
  using KineticsInfo_t = ISimulator::KineticsInfo_t;

 public:
  virtual ~ITrafficElement() TX_DEFAULT;
  virtual Geometry::Topology::txEdgeVec GetTopologyRelationShip() const TX_NOEXCEPT;

  /**
   * @brief 更新之前预处理
   *
   * @param map_elemId2Kinetics
   * @return txBool
   */
  virtual txBool Pre_Update(const TimeParamManager &,
                            std::map<Elem_Id_Type, KineticsInfo_t> &map_elemId2Kinetics) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新后操作
   *
   * @return txBool
   */
  virtual txBool Post_Update(TimeParamManager const &) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取动力学信息
   *
   * @param timeMgr
   * @return KineticsInfo_t
   */
  virtual KineticsInfo_t GetKineticsInfo(Base::TimeParamManager const &timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前对象的另一个const shared指针
   *
   * @return ITrafficElementConstPtr
   */
  virtual ITrafficElementConstPtr GetBaseConstPtr() const TX_NOEXCEPT { return shared_from_this(); }

  /**
   * @brief 获取当前对象的一个shared指针
   *
   * @return ITrafficElementPtr
   */
  virtual ITrafficElementPtr GetBasePtr() TX_NOEXCEPT { return shared_from_this(); }

  /**
   * @brief 获取元素一致性类型
   *
   * @return IIdentity::ElementType
   */
  virtual IIdentity::ElementType ConsistencyElementType() const TX_NOEXCEPT TX_OVERRIDE { return Type(); }
  virtual txFloat GetRnd_0_1() TX_NOEXCEPT { return 0.5; }

  /**
   * @brief 获取最后一次行驶状态
   *
   * @return IDrivingStatus::DrivingStatus
   */
  virtual IDrivingStatus::DrivingStatus StableDriving_Status() const TX_NOEXCEPT TX_OVERRIDE {
    return mLastDrivingStatus;
  }

  /**
   * @brief 是否需要输出轨迹信息
   *
   * @return txBool
   */
  virtual txBool NeedOutputTrajectory() const TX_NOEXCEPT { return FLAGS_VehicleOutputTrajectory; }

 public:
  template <class Archive>
  void serialize(Archive &archive) {
    SerializeInfo("ITrafficElement");
    archive(
        /*cereal::base_class<ISimulator>(this),*/
        /*cereal::base_class<IIdentity>(this),*/
        /*cereal::base_class<ILiftCycle>(this),*/
        /*cereal::base_class<IGeometry>(this),*/
        /*cereal::base_class<IKinetics>(this),*/
        /*cereal::base_class<ISwitchLane>(this),*/
        /*cereal::base_class<IMapLocation>(this),*/
        /*cereal::base_class<ISerialize>(this),*/
        /*cereal::base_class<IStream>(this),*/
        /*cereal::base_class<IMutexTBB>(this),*/
        /*cereal::base_class<ISimulationConsistency>(this),*/
        cereal::base_class<ILocalCoord>(this), cereal::base_class<ISpatialRegister>(this),
        cereal::base_class<IDrivingStatus>(this));
  }
};

/**
 * @brief GetSelfDrivingDirection 获取自驾方向的枚举值
 *
 * @param fromAngle 角度值 fromAngle (单位: 度)
 * @param toAngle 角度值 toAngle (单位: 度)
 * @return Base::Enums::VehicleInJunction_SelfDrivingDirection 返回自驾方向的枚举值
 */
Base::Enums::VehicleInJunction_SelfDrivingDirection GetSelfDrivingDirection(const Base::txFloat fromAngle,
                                                                            const Base::txFloat toAngle) TX_NOEXCEPT;

/**
 * @brief GetSelfDrivingDirection 获取自驾方向的枚举值
 *
 * @param fromLaneDir 从原始方向向量
 * @param toLaneDir 目标方向向量
 * @return Base::Enums::VehicleInJunction_SelfDrivingDirection 返回自驾方向的枚举值
 */
Base::Enums::VehicleInJunction_SelfDrivingDirection GetSelfDrivingDirection(const Base::txVec3 &fromLaneDir,
                                                                            const Base::txVec3 &toLaneDir) TX_NOEXCEPT;

/**
 * @brief ComputeVehicleInterchangeRelationship 计算两辆交通车之间的相对方向
 *
 * @param SelfVehicleDir 自车方向向量
 * @param otherVehicleDir 其他车辆方向向量
 * @return Base::Enums::Vehicle2Vehicle_RelativeDirection 返回两辆交通车之间的相对方向枚举值
 */
Base::Enums::Vehicle2Vehicle_RelativeDirection ComputeVehicleInterchangeRelationship(
    const Base::txVec3 &SelfVehicleDir, const Base::txVec3 &otherVehicleDir) TX_NOEXCEPT;

/**
 * @brief TX::ComputeVehicleInterchangeRelationship 计算两辆交通车之间的相对方向
 *
 * @param SelfVehicleAngle 自车的角度
 * @param otherVehicleAngle 其他车辆的角度
 * @return TX::Base::Enums::Vehicle2Vehicle_RelativeDirection 返回两辆交通车之间的相对方向枚举值
 */
Base::Enums::Vehicle2Vehicle_RelativeDirection ComputeVehicleInterchangeRelationship(
    const Base::txFloat SelfVehicleAngle, const Base::txFloat otherVehicleAngle) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(Base)

#undef SerializeInfo

CEREAL_REGISTER_TYPE(Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ISimulator, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IIdentity, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ILiftCycle, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IGeometry, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IKinetics, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ISwitchLane, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IMapLocation, Base::ITrafficElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ISerialize, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IStream, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IMutexTBB, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ISimulationConsistency, Base::ITrafficElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ILocalCoord, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::ISpatialRegister, Base::ITrafficElement);
CEREAL_REGISTER_POLYMORPHIC_RELATION(Base::IDrivingStatus, Base::ITrafficElement);
