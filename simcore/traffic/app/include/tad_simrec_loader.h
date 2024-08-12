// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tuple>
#include "tad_sim.h"
#include "tad_traffic.h"
#include "traffic.pb.h"
#include "tx_header.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"
#include "tx_tadsim_flags.h"
#include "tx_traj_manager_base.h"
TX_NAMESPACE_OPEN(SceneLoader)

// Simrec_Scene加载器
class Simrec_SceneLoader : public Base::ISceneLoader {
 public:
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;
  using txSysId = Base::txSysId;
  using txLaneID = Base::txLaneID;
  using txSpaceTimeCarList = TrafficFlow::txTrajManagerBase::txSpaceTimeCarList;
  using txSpaceTimeLocationList = TrafficFlow::txTrajManagerBase::txSpaceTimeLocationList;

 public:
  Simrec_SceneLoader();
  virtual ~Simrec_SceneLoader() TX_DEFAULT;
  virtual Base::txLpsz ClassName() const TX_NOEXCEPT TX_OVERRIDE { return _class_name; }

  /**
   * @brief 场景加载完成，是否可用
   *
   * @return Base::txBool
   */
  virtual Base::txBool IsValid() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 暂无实现
   *
   * @return Base::txBool
   */
  virtual Base::txBool Load(const txString&) TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 加载场景数据
   *
   * @param _sim_path 场景文件路径
   * @param strSimRecFilePath 回放数据pb路径
   * @return Base::txBool
   */
  virtual Base::txBool Load(const txString& _sim_path, const txString&, const txString&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 释放场景加载器中资源
   *
   * @return Base::txBool
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 暂无实现
   *
   * @return Base::txBool
   */
  virtual Base::txBool ParseSceneEvent() TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 随时间加载轨迹，cars，locations
   *
   * @return Base::txBool
   */
  virtual Base::txBool LoadObjects() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取当前状态
   *
   * @return EStatus
   */
  virtual EStatus Status() const TX_NOEXCEPT TX_OVERRIDE { return _status; }

  /**
   * @brief 获取ego类型
   *
   * @return EgoType
   */
  virtual EgoType GetEgoType() const TX_NOEXCEPT;

  /**
   * @brief 获取场景文件路径
   *
   * @return Base::txString
   */
  virtual Base::txString Source() const TX_NOEXCEPT TX_OVERRIDE { return _source; }

  /**
   * @brief 获取仿真场景类型
   *
   * @return ESceneType
   */
  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eSimrec; }

  /**
   * @brief 获取地图管理者初始化参数
   *
   * @param refParams
   * @return Base::txBool
   */
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取仿真开始的时间戳
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat SimrecStartTime() const TX_NOEXCEPT { return _simrec_ego_start_timestamp_ms; }

  /**
   * @brief 获取仿真结束的时间戳
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat SimrecEndTime() const TX_NOEXCEPT { return _simrec_ego_end_timestamp_ms; }

  /**
   * @brief 获取仿真时长
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat SimrecDuration() const TX_NOEXCEPT { return (SimrecEndTime() - SimrecStartTime()); }

  /**
   * @brief 获取ego结束的时间戳
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat SimrecEgoEndTime() const TX_NOEXCEPT { return _simrec_ego_end_timestamp_ms; }

  /**
   * @brief 获取traffic结束的时间戳
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat SimrecTrafficEndTime() const TX_NOEXCEPT { return _simrec_traffic_end_timestamp_ms; }

  /**
   * @brief 获取routing信息
   *
   * @param[in] refEgoData 接受取到的location
   * @return Base::txBool
   */
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取ego视图信息
   *
   * @return IVehiclesViewerPtr 当前视图
   */
  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取trailer视图信息
   *
   * @return IVehiclesViewerPtr 不存在null
   */
  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 预估ego某一时刻位置
   *
   * @param timeMgr 时间管理器
   * @param[out] refEgoLocation 计算的位置信息
   * @return Base::txBool
   */
  virtual Base::txBool InterpEgoLocation(Base::TimeParamManager const& timeMgr,
                                         sim_msg::Location& refEgoLocation) TX_NOEXCEPT;

  /**
   * @brief 根据sysId获取route信息
   *
   * @param id 给出的id
   * @return IRouteViewerPtr 返回route视图信息
   */
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取ego的route数据gps
   *
   * @return IRouteViewerPtr 视图
   */
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将每帧数据转成object数据
   *
   * @param baseTime 时间差
   * @param[out] locations_ location信息
   * @param[out] cars_ traffic cars信息
   * @return Base::txBool
   */
  virtual Base::txBool FrameData2ObjectData(
      const Base::txFloat baseTime, std::map<Base::txFloat, sim_msg::Location>& locations_,
      std::unordered_map<Base::txSysId, std::map<Base::txFloat, sim_msg::Car> >& cars_) TX_NOEXCEPT;

  /**
   * @brief 生成轨迹信息
   *
   * @param locations_ 输出的locations
   * @param cars_ 输出的traffic cars
   * @return Base::txBool
   */
  virtual Base::txBool GenerateTrajMgr(
      const std::map<Base::txFloat, sim_msg::Location>& locations_,
      const std::unordered_map<Base::txSysId, std::map<Base::txFloat, sim_msg::Car> >& cars_) TX_NOEXCEPT;

  /**
   * @brief 按照损失帧拆分车辆轨迹
   *
   * 按照损失帧拆分输入车辆轨迹列表 in_car_list 为多个子列表，每个子列表表示一个连续的损失帧区间内的车辆轨迹。
   *
   * @param in_car_list 输入车辆轨迹列表
   * @param out_car_list 输出拆分后的车辆轨迹列表
   */
  virtual void SpliteCarTrajByLossFrame(const std::map<Base::txFloat, sim_msg::Car>& in_car_list,
                                        std::vector<std::map<Base::txFloat, sim_msg::Car> >& out_car_list) const
      TX_NOEXCEPT;

  /**
   * @brief ComputeVehicle_Route
   *
   * 对输入车辆序列列表进行路线计算，返回计算结果的车辆结构体。
   *
   * @param vehicle_seq_list 输入车辆序列列表
   * @return SceneLoader::Traffic::Vehicle 计算结果的车辆结构体
   */
  virtual SceneLoader::Traffic::Vehicle ComputeVehicle_Route(const txSpaceTimeCarList& vehicle_seq_list) TX_NOEXCEPT;

  /**
   * @brief ComputeVehicle_Geometry
   *
   * 计算车辆序列列表的车辆几何。
   *
   * @param vehicle_seq_list 车辆序列列表
   * @return txVec3 车辆几何
   */
  virtual Base::txVec3 ComputeVehicle_Geometry(const txSpaceTimeCarList& vehicle_seq_list) const TX_NOEXCEPT;

  /**
   * @brief ComputeVehicleStartVelocity
   *
   * 计算车辆序列列表的起始速度。
   *
   * @param vehicle_seq_list 车辆序列列表
   * @return txFloat 起始速度
   */
  virtual Base::txFloat ComputeVehicleStartVelocity(const txSpaceTimeCarList& vehicle_seq_list) const TX_NOEXCEPT;

  /**
   * @brief 计算车辆序列列表的车辆类型
   *
   * 根据车辆序列列表的属性，计算并返回车辆类型。
   *
   * @param vehicle_seq_list 车辆序列列表
   * @return Base::Enums::VEHICLE_TYPE 车辆类型
   */
  virtual Base::Enums::VEHICLE_TYPE ComputeVehicle_Type(const txSpaceTimeCarList& vehicle_seq_list) const TX_NOEXCEPT;

  /**
   * @brief 计算车辆序列列表中的最大行驶速度
   *
   * 根据车辆序列列表的属性，计算并返回最大行驶速度。
   *
   * @param vehicle_seq_list 车辆序列列表
   * @return Base::txFloat 最大行驶速度
   */
  virtual Base::txFloat ComputeVehilce_MaxV(const txSpaceTimeCarList& vehicle_seq_list) const TX_NOEXCEPT;

  /**
   * @brief 根据车辆序列列表的属性，初始化车辆的位置信息
   *
   * 该函数根据车辆序列列表中的车辆属性，计算并设置车辆的初始位置信息。具体而言，它将返回车辆所在的道路编号、起始距离和沿道路的偏移量。
   *
   * @param vehicle_seq_list 车辆序列列表
   * @param refLaneID 引用的道路编号
   * @param refStartS 引用的起始距离
   * @param refLOffset 引用的沿道路的偏移量
   */
  virtual void ComputeVehilce_InitLocationInfo(const txSpaceTimeCarList& vehicle_seq_list, Base::txInt& refLaneID,
                                               Base::txFloat& refStartS, Base::txFloat& refLOffset) const TX_NOEXCEPT;

  /**
   * @brief 返回车辆位置信息的线性插值方式
   *
   * 该函数返回车辆位置信息的线性插值方式。若设置为 true，则在加载框架中该车辆位置信息的线性插值方式将被使用。
   *
   * @return Base::txBool 返回 true 表示启用线性插值，返回 false 表示不启用线性插值
   */
  virtual Base::txBool VehicleInterpolation() const TX_NOEXCEPT {
    return FLAGS_L2W_Vehicle_Position_Linear_Interpolation;
  }

  /**
   * @brief 是否启用对位置信息的线性插值
   *
   * 返回 true 表示启用对位置信息的线性插值，返回 false 表示不启用线性插值
   *
   * @return Base::txBool 返回 true 表示启用线性插值，返回 false 表示不启用线性插值
   */
  virtual Base::txBool LocationInterpolation() const TX_NOEXCEPT { return FLAGS_L2W_Ego_Position_Linear_Interpolation; }

  /**
   * @brief 是否避免突然出现
   *
   * 返回 true 表示避免突然出现，返回 false 表示不避免突然出现
   *
   * @return Base::txBool 返回 true 表示避免突然出现，返回 false 表示不避免突然出现
   */
  virtual Base::txBool AvoidSuddenlyAppear() const TX_NOEXCEPT { return FLAGS_L2W_AvoidSuddenlyAppear; }

  /**
   * @brief 丢帧阈值
   *
   * @return 返回丢帧阈值，当丢帧数超过此阈值时，视频数据可能会有轻微的偏差，但不会导致数据丢失
   */
  virtual Base::txFloat LoseFrameThreshold() const TX_NOEXCEPT { return FLAGS_L2W_LoseFrameThreshold; }

 public:
  struct L2W_EgoVehiclesViewer : public IVehiclesViewer {
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    L2W_EgoVehiclesViewer() TX_DEFAULT;
    virtual ~L2W_EgoVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取视图id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 视图id是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取route id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief route id是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取laneid
     *
     * @return Base::txLaneID
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief lane id 是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取开始时s
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时t
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时v
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大v
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车道偏移
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取经度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat lon() const TX_NOEXCEPT;

    /**
     * @brief 获取纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat lat() const TX_NOEXCEPT;

    /**
     * @brief 获取位置高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat alt() const TX_NOEXCEPT;

    /**
     * @brief 长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取元素宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取元素高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取acc事件id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief acc事件id是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取merge事件id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief merge事件id是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取behavior
     *
     * @return Base::txString
     */
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle类型
     *
     * @return Base::txString
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 模拟车辆跟随对象
     *
     * 根据车辆轨迹模拟器生成模拟车辆跟随对象。这个类将覆盖父类的 follow() 函数，以根据输入的车辆 ID
     * 返回相应的车辆跟随对象的 ID。
     *
     * @param srcVehicleId 车辆 ID
     * @param _ptr 指向 txTrajManagerBase 的指针
     *
     * @return 返回跟随对象的 ID
     */
    virtual Base::txSysId follow() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 合并时间
     *
     * 返回两个车辆的合并时间。这个函数会在每个时刻对两个车辆进行调度，直到其中一辆车辆不能再按照当前速度继续行驶。
     *
     * @return 返回合并时间，单位为秒
     */
    virtual Base::txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 计算偏移时间
     *
     * 返回车辆在模拟过程中偏移的时间。根据车辆初始速度和加速度，计算偏移时间。
     *
     * @return 返回偏移时间，单位为秒
     */
    virtual Base::txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 计算角度
     *
     * 计算当前车辆的方向角度。返回一个以弧度为单位的浮点数，表示当前车辆的方向角度。
     *
     * @return 返回当前车辆的方向角度，单位为弧度
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

   protected:
    friend class Simrec_SceneLoader;
    void Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    sim_msg::Location m_ego;
    Base::txInt m_egoId = -1;
  };
  using L2W_EgoVehiclesViewerPtr = std::shared_ptr<L2W_EgoVehiclesViewer>;
#if Use_TruckEgo
  struct L2W_TruckVehiclesViewer : public L2W_EgoVehiclesViewer {
   public:
    L2W_TruckVehiclesViewer() TX_DEFAULT;
    virtual ~L2W_TruckVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取视图ID
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图中长度属性值
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图中宽度属性值
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图中高度属性值
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using L2W_TruckVehiclesViewerPtr = std::shared_ptr<L2W_TruckVehiclesViewer>;
#endif /*Use_TruckEgo*/
  struct L2W_TrailerVehiclesViewer : public L2W_EgoVehiclesViewer {
   public:
    L2W_TrailerVehiclesViewer() TX_DEFAULT;
    virtual ~L2W_TrailerVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取视图id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图中长度属性
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图宽度属性
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取视图高度属性
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using L2W_TrailerVehiclesViewerPtr = std::shared_ptr<L2W_TrailerVehiclesViewer>;

  struct L2W_VehiclesViewer : public IVehiclesViewer {
    using txTrajManagerBasePtr = TrafficFlow::txTrajManagerBasePtr;
    L2W_VehiclesViewer() TX_DEFAULT;
    virtual ~L2W_VehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取vehicle id
     *
     * @return txSysId
     */
    virtual txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle.id; }

    /**
     * @brief 返回是否初始化
     *
     * @return txBool
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

    /**
     * @brief 获取路由ID
     *
     * 该函数用于获取当前模块的路由ID，以便在路由操作中使用。
     *
     * @return txSysId 返回路由ID
     */
    virtual txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车道id
     *
     * @return txLaneID
     */
    virtual txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时的s
     *
     * @return txFloat
     */
    virtual txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时的t
     *
     * @return txFloat
     */
    virtual txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时的v
     *
     * @return txFloat
     */
    virtual txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * @return txFloat
     */
    virtual txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车道上的偏移
     *
     * @return txFloat
     */
    virtual txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取长度属性
     *
     * @return txFloat
     */
    virtual txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取宽度属性
     *
     * @return txFloat
     */
    virtual txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取高度属性
     *
     * @return txFloat
     */
    virtual txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取acc事件的id
     *
     * @return txSysId
     */
    virtual txSysId accID() const TX_NOEXCEPT TX_OVERRIDE { return -1; }

    /**
     * @brief 获取merge事件的ID
     *
     * @return txSysId
     */
    virtual txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE { return -1; }

    /**
     * @brief 获取behavior
     *
     * @return txString
     */
    virtual txString behavior() const TX_NOEXCEPT TX_OVERRIDE { return "L2W"; }
    virtual txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle的字符串类型
     *
     * @return txString
     */
    virtual txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle的behavior枚举类型
     *
     * @return VEHICLE_BEHAVIOR
     */
    virtual VEHICLE_BEHAVIOR behaviorEnum() const TX_NOEXCEPT { return VEHICLE_BEHAVIOR::eL2W; }
    virtual txSysId follow() const TX_NOEXCEPT TX_OVERRIDE { return -1; }

    /**
     * @brief 合并时间
     *
     * 获取合并时间。当前暂时没有合并操作，因此返回0。
     *
     * @return 返回浮点型的合并时间
     */
    virtual txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取偏移时间
     *
     * 获取加载器中的偏移时间。当前暂时没有偏移操作，因此返回0。
     *
     * @return 返回浮点型的偏移时间
     */
    virtual txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取驾驶员数据
     *
     * 从驾驶员数据管理器中获取一个指定驾驶员的数据。
     *
     * @param driverId 指定的驾驶员 ID
     * @return 返回指定驾驶员的数据，如果不存在则返回空指针
     */
    virtual txTrajManagerBasePtr trajMgr() const TX_NOEXCEPT { return m_trajMgrPtr; }
    /*virtual std::vector< txSysId > eventId() const TX_NOEXCEPT { return std::vector< txSysId >(); }*/

    /**
     * @brief 获取当前模块的角度
     *
     * 该函数用于获取当前模块的角度值，以便在其他模块中使用。
     *
     * @return txFloat 返回当前模块的角度值
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }

   protected:
    friend class Simrec_SceneLoader;

    /**
     * @brief 初始化 Vehicle
     *
     * 根据输入的 @p srcVehicle 和 @p _ptr 初始化 @p this 对象。
     *
     * @param srcVehicle 待初始化的车辆
     * @param _ptr 指向 txTrajManagerBase 的指针
     */
    void Init(SceneLoader::Traffic::Vehicle const& srcVehicle, txTrajManagerBasePtr _ptr) TX_NOEXCEPT {
      m_vehicle = srcVehicle;
      m_trajMgrPtr = _ptr;
      m_inited = true;
    }

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Vehicle m_vehicle;
    txTrajManagerBasePtr m_trajMgrPtr = nullptr;
  };
  using L2W_VehiclesViewerPtr = std::shared_ptr<L2W_VehiclesViewer>;

  struct L2W_RouteViewer : public IRouteViewer {
    /**
     * @brief 检查是否已经初始化
     *
     * @return Base::txBool 已经初始化返回true
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    L2W_RouteViewer() TX_DEFAULT;
    virtual ~L2W_RouteViewer() TX_DEFAULT;

    /**
     * @brief 获取route id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return m_route_id; }

    /**
     * @brief 获取route类型
     *
     * @return ROUTE_TYPE
     */
    virtual ROUTE_TYPE type() const TX_NOEXCEPT TX_OVERRIDE { return ROUTE_TYPE::ePos; }

    /**
     * @brief 获取开始点的经度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat startLon() const TX_NOEXCEPT TX_OVERRIDE { return __Lon__(m_control_path.front().waypoint); }

    /**
     * @brief 获取开始点的纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat startLat() const TX_NOEXCEPT TX_OVERRIDE { return __Lat__(m_control_path.front().waypoint); }

    /**
     * @brief 获取终点的经度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat endLon() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取终点的纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat endLat() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取route的中间点集合
     *
     * @return std::vector< std::pair< Base::txFloat, Base::txFloat > >
     */
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > midPoints() const TX_NOEXCEPT TX_OVERRIDE {
      return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
    }

    /**
     * @brief 获取roadId
     *
     * @return Base::txRoadID
     */
    virtual Base::txRoadID roidId() const TX_NOEXCEPT TX_OVERRIDE { return -1; }

    /**
     * @brief 获取sectionID
     *
     * @return Base::txSectionID
     */
    virtual Base::txSectionID sectionId() const TX_NOEXCEPT TX_OVERRIDE { return -1; }

    /**
     * @brief 获取控制路径
     *
     * 该函数用于获取控制路径，以便在其他模块中使用。
     *
     * @return tx_path_node_vec 返回控制路径的向量
     */
    virtual control_path_node_vec control_path() const TX_NOEXCEPT TX_OVERRIDE { return m_control_path; }

    /**
     * @brief 返回对象的字符串表示
     *
     * @return Base::txString
     */
    virtual Base::txString Str() const TX_NOEXCEPT { return ""; }

   protected:
    friend class Simrec_SceneLoader;
    void Init(Base::txSysId _id, const control_path_node_vec& cp) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    Base::txSysId m_route_id = 0;
    control_path_node_vec m_control_path;
  };
  using L2W_RouteViewerPtr = std::shared_ptr<L2W_RouteViewer>;

 public:
  /**
   * @brief 获取所有车辆数据
   *
   * 该函数用于获取所有车辆数据，以便在其他模块中使用。
   *
   * @return std::unordered_map<Base::txSysId, IVehiclesViewerPtr> 返回所有车辆数据的哈希表
   */
  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定车辆数据
   *
   * 该函数用于获取指定车辆ID的数据，返回车辆信息的句柄。
   *
   * @param id 要查询的车辆ID
   * @return IVehiclesViewerPtr 返回车辆信息的句柄
   */
  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief 获取指定行人数据
   *
   * 该函数用于获取指定行人ID的数据，返回行人信息的句柄。
   *
   * @param id 要查询的行人ID
   * @return IPedestriansViewerPtr 返回行人信息的句柄
   */
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief 获取所有行人数据
   *
   * 该函数用于获取系统中所有行人的数据，返回一个包含所有行人信息的哈希表。
   *
   * @return std::unordered_map<Base::txSysId, IPedestriansViewerPtr> 包含所有行人信息的哈希表
   */
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, IPedestriansViewerPtr>();
  }

  /**
   * @brief 获取指定信号灯数据
   *
   * 此函数用于获取指定信号灯数据，返回一个信号灯的指针。
   *
   * @param id 标识符，用于唯一标识一个信号灯视图
   * @return ISignlightsViewerPtr 返回一个信号灯视图的指针
   */
  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief 获取所有信号灯数据
   *
   * 获取系统中所有标志灯数据的哈希表，包含唯一的标识符及其对应的信号灯数据查看器指针。
   *
   * @return std::unordered_map<Base::txSysId, ISignlightsViewerPtr> 包含所有信号灯数据的集合
   */
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, ISignlightsViewerPtr>();
  }

  /**
   * @brief 获取指定标识符的障碍物数据
   *
   * 此函数用于获取指定标识符的障碍物数据。
   * 输入参数为标识符，表示唯一的障碍物。
   * 函数返回障碍物数据查看器指针，用于查询障碍物数据信息。
   *
   * @param id 标识符，用于唯一标识一个障碍物
   * @return IObstacleViewerPtr 包含指定标识符的障碍物数据的查看器指针
   */
  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief 获取所有障碍物数据
   *
   * 此函数用于获取所有障碍物的数据，返回一个包含所有障碍物数据的查看器的哈希表。
   *
   * @return std::unordered_map<Base::txSysId, IObstacleViewerPtr> 包含所有障碍物数据的查看器的集合
   */
  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT TX_OVERRIDE {
    return std::unordered_map<Base::txSysId, IObstacleViewerPtr>();
  }

  /**
   * @brief 获取随机种子
   *
   * 此函数返回一个用于生成随机数的种子，默认为55。
   *
   * @return 随机种子的整数表示
   */
  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT TX_OVERRIDE { return 55; }

  /**
   * @brief 获取指定时间的轨迹数据
   *
   * 根据输入的时间戳，返回对应的轨迹数据。这个函数通常在Simulation模块中使用。
   *
   * @param[in] time_stamp_s 指定的时间戳（单位：秒）
   * @return sim_msg::Trajectory 返回对应时间的轨迹数据
   */
  virtual sim_msg::Trajectory GetTrajectory(const txFloat time_stamp_s) const TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取模拟任务的Ego类型
   * @return 返回Ego类型的字符串
   */
  Base::txString GetSimSimulationPlannerEgoType() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟任务的地图文件路径
   * @return 返回映射文件路径的字符串
   */
  Base::txString GetSimSimulationMapFile() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟任务的地图文件的经度
   * @return 返回经度的浮点数
   */
  Base::txFloat GetSimSimulationMapfileLongitude() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟任务的地图文件的纬度
   * @return 返回纬度的浮点数
   */
  Base::txFloat GetSimSimulationMapfileLatitude() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟任务的地图文件的高度
   * @return 返回高度的浮点数
   */
  Base::txFloat GetSimSimulationMapfileAltitude() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟交通数据
   *
   * 该函数用于获取模拟交通数据。
   *
   * @return std::string 返回模拟交通数据
   */
  Base::txString GetSimSimulationTraffic() const TX_NOEXCEPT;

  /**
   * @brief 获取模拟交通规划启动信息
   *
   * 此函数用于获取模拟交通规划的起始信息。
   *
   * @param[out] res 用于保存起始坐标的二元组
   * @return bool 当模拟交通规划启动时返回 true，否则返回 false
   */
  Base::txBool GetSimSimulationPlannerRouteStart(std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT;

  /**
   * @brief 获取L2W中动作距离。
   *
   * 返回在L2W中计算的动作间距离。
   *
   * @return Base::txFloat L2W中动作间距离的值
   */
  Base::txFloat L2W_Motion_Dist() const TX_NOEXCEPT { return FLAGS_L2W_Motion_Dist; }

 public:
  /**
   * @brief Get the Accs Data object
   *
   * @param id 指定的id
   * @return IAccelerationViewerPtr 要获取的视图指针
   */
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief Get the Accs Event Data object
   *
   * @param id 指定的id
   * @return IAccelerationViewerPtr
   */
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief Get the Merges Data object
   *
   * @param id 指定的id
   * @return IMergesViewerPtr 要获取的视图指针
   */
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief Get the Merges Event Data object
   *
   * @param id 指定的id
   * @return IMergesViewerPtr 要获取的视图指针
   */
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief Get the Velocity Data object
   *
   * @param id 指定的id
   * @return IVelocityViewerPtr 要获取的视图指针
   */
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief Get the Velocity Event Data object
   *
   * @param id 指定的id
   * @return IVelocityViewerPtr 要获取的视图指针
   */
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return nullptr; }

  /**
   * @brief 获取行人事件数据时间事件视图指针
   *
   * @param id 指定的id
   * @return IPedestriansEventViewerPtr 要获取的视图指针
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return nullptr;
  }

  /**
   * @brief 获取行人事件数据速度事件视图指针
   *
   * @param id 指定的id
   * @return IPedestriansEventViewerPtr 要获取的视图指针
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id)
      TX_NOEXCEPT TX_OVERRIDE {
    return nullptr;
  }

 protected:
  SceneLoader::Sim::simulation_ptr m_DataSource_Scene = nullptr;
  Base::txLpsz _class_name;
  sim_msg::TrafficRecords4Logsim trafficRecords_;
  Base::txFloat _simrec_ego_start_timestamp_ms = 0.0;
  Base::txFloat _simrec_ego_end_timestamp_ms = 0.0;
  Base::txFloat _simrec_traffic_end_timestamp_ms = 0.0;

  std::vector<std::tuple<SceneLoader::Traffic::Vehicle, TrafficFlow::txTrajManagerBasePtr> >
      mSpaceTimeCarListWithSameId;
  std::unordered_map<Base::txSysId, L2W_RouteViewerPtr> m_map_rid2routePtr;
  Base::txSysId m_route_base = 1;
  TrafficFlow::txTrajManagerBasePtr mSpaceTimeEgoPtr = nullptr;
};

using Simrec_SceneLoaderPtr = std::shared_ptr<Simrec_SceneLoader>;

TX_NAMESPACE_CLOSE(SceneLoader)
