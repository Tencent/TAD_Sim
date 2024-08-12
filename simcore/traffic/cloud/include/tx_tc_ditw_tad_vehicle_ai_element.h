// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "road_realtimeinfo.pb.h"
#include "tx_enum_def.h"
#include "tx_tc_cloud_ditw_trajectory_manager.h"
#include "tx_tc_tad_vehicle_ai_element.h"

TX_NAMESPACE_OPEN(TrafficFlow)
class DITW_TAD_Cloud_AI_VehicleElement : public TAD_Cloud_AI_VehicleElement {
 public:
  using DITW_Status = Base::Enums::DITW_Status;
  using ParentClass = TAD_Cloud_AI_VehicleElement;

 public:
  /**
   * @brief 向该对象注入第三方车辆信息
   * @param ref3rdCar 第三方车辆信息的引用
   * @details 本函数不会负责对第三方车辆信息对象的内存释放，但它会使当前对象中的信息变为空。
   */
  virtual void Injection3rdData(const ObjectInfo& ref3rdCar) TX_NOEXCEPT {
    m_optional_object_info = ObjectInfo();
    m_optional_object_info->CopyFrom(ref3rdCar);
  }

  /**
   * @brief 初始化时间管理器和第三方车辆信息
   * @param timeMgr 时间参数管理器
   * @param ref3rdCar 第三方车辆信息
   * @param sceneLoaderPtr 场景加载器，可选
   * @return 成功返回 true, 失败返回 false
   */
  virtual Base::txBool Initialize(TimeParamManager const& timeMgr, const ObjectInfo& ref3rdCar,
                                  Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 判断当前状态是否是logsim
   *
   * @return Base::txBool 是logsim返回true
   */
  virtual Base::txBool IsLogSim() const TX_NOEXCEPT { return _plus_(DITW_Status::eLog) == m_ditw_status; }

  /**
   * @brief 判断是否是worldsim
   *
   * @return Base::txBool 是worldsim返回true
   */
  virtual Base::txBool IsWorldSim() const TX_NOEXCEPT { return _plus_(DITW_Status::eWorld) == m_ditw_status; }

  /**
   * @brief 检查该汽车对象是否受到速度控制
   * @return 如果是速度控制的汽车对象则返回 true, 否则返回 false
   */
  virtual txBool IsVelocityControled() const TX_NOEXCEPT TX_OVERRIDE { return mVelocityControled; }

  /**
   * @brief 设置汽车对象是否受到速度控制
   * @param _f 如果为 true，表示汽车对象受到速度控制；否则表示未受到速度控制
   */
  virtual void SetVelocityControled(txBool _f) TX_NOEXCEPT TX_OVERRIDE { mVelocityControled = _f; }

  /**
   * @brief 返回是否支持速度控制
   * @return true:支持速度控制, false:不支持速度控制
   */
  virtual txBool SupportVelocityControled() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 判断第三方信息是否已准备就绪
   * @return true: 第三方信息准备就绪, false: 第三方信息未准备就绪
   */
  virtual Base::txBool Is3rdReady() const TX_NOEXCEPT { return m_optional_object_info.has_value(); }

  /**
   * @brief 向当前类添加车辆的路径信息
   *
   * 这个函数向当前类中添加车辆的路径信息。它接受一个时间参数管理器和一个包含车辆信息的对象引用，不返回任何值。
   *
   * @param timeMgr 一个引用，指向一个时间参数管理器的对象
   * @param ref3rdCar 一个引用，指向一个包含车辆信息的对象
   * @return 无
   */
  virtual void AddTraj(Base::TimeParamManager const& timeMgr, const ObjectInfo& ref3rdCar) TX_NOEXCEPT;

  /**
   * @brief 更新车辆AI元素的状态.
   *
   * 此函数用于更新车辆AI元素的状态，并在需要时采取行动。它需要一个时间参数管理器对象以及一个车辆的引用作为输入，不返回任何值。
   *
   * @param timeMgr 一个指向时间参数管理器对象的引用
   * @param ref3rdCar 一个指向车辆的引用
   * @return 无
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新logsim
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool UpdateLogSim(TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新logsim的vehicles
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool UpdateLogSimVehicle(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 计算累积速度
   *
   * 根据提供的时间参数管理器和自行车中心的坐标，计算累积速度。
   *
   * @param timeMgr 时间参数管理器的引用
   * @param egoMassCenter 自行车中心的坐标
   * @return 无
   */
  virtual void ComputeScalarVelocity(Base::TimeParamManager const& timeMgr,
                                     const Base::txVec2& egoMassCenter) TX_NOEXCEPT;

  /**
   * @brief 计算注入相对时间
   *
   * 计算辅助函数的相对时间，该函数会根据传入的参数计算出注入相对时间。
   *
   * @return 返回计算得到的注入相对时间值
   */
  virtual txFloat ComputeInjectionRelativeTime() const TX_NOEXCEPT;

  /**
   * @brief 切换log与world的映射
   *
   * 切换日志系统与世界系统之间的映射关系。
   */
  virtual void SwitchLog2World() TX_NOEXCEPT;

  /**
   * @brief 检查实体的存活状态
   *
   * 这个函数用于检查实体的存活状态。通常情况下，如果实体的存活状态发生变化，将会返回 true。
   *
   * @param[in] timeMgr 时间参数管理器
   * @return true 如果实体存活，false 如果实体不存活
   */
  virtual txBool CheckAlive(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取最大空闲时间
   *
   * @return float 最大空闲时间，单位为秒
   */
  virtual Base::txFloat GetMaxIdleTimeInSecond() const TX_NOEXCEPT { return FLAGS_cloud_ditw_max_idle_time_in_second; }

  /**
   * @brief 用于填充交通元素的函数
   *
   * 当调用此函数时，将会用于处理与交通元素相关的数据
   *
   * @param timeParamManager 时间参数管理器，包含与当前时间相关的信息
   * @param traffic 交通信息结构体，包含与交通系统相关的信息
   * @return txBool 返回 true 表示已成功填充交通元素，返回 false 表示填充失败
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 提前填充元素
   *
   * 在使用该函数之前，用于处理与元素相关的预填充操作。
   * 如果预填充成功，该函数应返回 true，否则返回 false。
   *
   * @param a_size 一个引用参数，用于返回处理后的大小值
   * @return txBool 如果预填充成功，则返回 true，否则返回 false
   */
  virtual txBool PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  DITW_Status m_ditw_status = _plus_(DITW_Status::eLog);
  static Base::txSize s3rdVehicleSize;
  Base::txBool mVelocityControled = false;
  boost::optional<ObjectInfo> m_optional_object_info;
  Cloud::txCloudTrajManagerPtr m_traj_mgr = nullptr;
};

using DITW_TAD_Cloud_AI_VehicleElementPtr = std::shared_ptr<DITW_TAD_Cloud_AI_VehicleElement>;

class TAD_Cloud_Obstacle_VehicleElement : public TAD_Cloud_AI_VehicleElement {
  using ParentClass = TAD_Cloud_AI_VehicleElement;

 public:
  /**
   * @brief 初始化斜向车辆的情景模拟元素
   *
   * 初始化斜向车辆的情景模拟元素，包括车辆的ID、起始车道信息、起始距离、起始方向、结束时间和场景加载器。
   * 这个函数在情景模拟中起到了关键作用，因为它初始化了我们正在模拟的斜向车辆。
   *
   * @param _vehId 车辆的ID
   * @param StartLaneInfo 起始车道的信息
   * @param _s 起始距离
   * @param _heading 起始方向
   * @param endTime 结束时间
   * @param _loader 用于加载情景模拟元素的场景加载器
   *
   * @return true 如果成功初始化斜向车辆情景模拟元素
   * @return false 如果初始化斜向车辆情景模拟元素失败
   */
  virtual Base::txBool Initialize_Obstacle(const Base::txSysId _vehId, const Base::Info_Lane_t StartLaneInfo,
                                           const Base::txFloat& _s, const Base::txFloat _heading, const int64_t endTime,
                                           Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 更新函数
   *
   * 此函数用于在每帧更新时进行操作。如果返回true，则更新成功；否则，更新失败。
   *
   * @param timeMgr 时间管理器的引用，包含当前时间信息
   *
   * @return true 更新成功
   * @return false 更新失败
   */
  virtual txBool Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 对指定车辆进行模拟填充
   *
   * 该函数根据指定的时间戳和车辆模拟数据，对车辆进行模拟填充。
   *
   * @param timeStamp 当前时间戳，单位为秒
   * @param pSimVehicle 指向车辆模拟对象的指针
   *
   * @return 返回填充后的车辆模拟对象指针
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 检查当前事件是否已经结束
   *
   * 该函数用于检查当前事件是否已经结束。如果当前事件已经结束，返回true；否则返回false。
   *
   * @param timeMgr 时间管理器的引用，包含当前时间信息
   *
   * @return true 当前事件已经结束
   * @return false 当前事件未结束
   */
  virtual Base::txBool CheckEnd(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;
};
using TAD_Cloud_Obstacle_VehicleElementPtr = std::shared_ptr<TAD_Cloud_Obstacle_VehicleElement>;

class TAD_Virtual_Obstacle_VehicleElement : public TAD_Cloud_Obstacle_VehicleElement {
 public:
  /**
   * @brief 初始化一个固定的障碍物
   *
   * 这个函数用于初始化一个固定的障碍物。它需要接收一个车辆
   * ID、当前的卫星信息、一个偏移量、固定障碍物的结束时间和一个场景加载器。 如果成功初始化，它将返回 true，否则返回
   * false。
   *
   * @param _vehId 车辆 ID
   * @param StartLaneInfo 当前卫星信息
   * @param _s 当前偏移量
   * @param endTime_s 固定障碍物的结束时间
   * @param _sceneLoader 场景加载器
   * @return true 成功初始化
   * @return false 初始化失败
   */
  virtual Base::txBool Initialize_Closure_Obstacle(const Base::txSysId _vehId, const Base::Info_Lane_t StartLaneInfo,
                                                   const Base::txFloat& _s, const int64_t endTime_s,
                                                   Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 填充traffic信息
   *
   * @param timeMgr 一个包含当前时间信息的参数管理器
   * @param traffic 待填充的Traffic数据结构
   *
   * @return txBool
   */
  virtual txBool FillingElement(Base::TimeParamManager const&, sim_msg::Traffic&) TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }

  /**
   * @brief 元素预处理
   *
   * 该函数用于在预处理阶段对元素进行相关的预处理操作。
   *
   * @param a_size 一个包含元素的大小信息的引用
   * @return true 预处理成功
   * @return false 预处理失败，可能是由于未实现或其他错误导致的
   */
  virtual txBool PreFillingElement(AtomicSize& a_size) TX_NOEXCEPT TX_OVERRIDE { return false; }
};
using TAD_Virtual_Obstacle_VehicleElementPtr = std::shared_ptr<TAD_Virtual_Obstacle_VehicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)
