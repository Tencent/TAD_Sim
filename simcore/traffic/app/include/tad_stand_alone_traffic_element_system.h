// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_scene_sketch.h"
#include "tx_tadsim_flags.h"
#include "tx_traffic_element_system.h"
TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_EgoVehicleElement;
using TAD_EgoVehicleElementPtr = std::shared_ptr<TAD_EgoVehicleElement>;

class TAD_StandAlone_TrafficElementSystem : public Base::TrafficElementSystem {
  using ParentClass = Base::TrafficElementSystem;

 public:
  TAD_StandAlone_TrafficElementSystem() { InitSketch(); }
  virtual ~TAD_StandAlone_TrafficElementSystem() { Release(); }

  /**
   * @brief 系统初始化
   *
   * @return Base::txBool
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 系统更新
   *
   * @param timeMgr 事件管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool Update(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 资源释放
   *
   * @return Base::txBool 释放成功返回true
   */
  virtual Base::txBool Release() TX_NOEXCEPT TX_OVERRIDE { return ParentClass::Release(); }

  /**
   * @brief 是否支持所指定的场景类型
   *
   * @param _sceneType 所检查的场景类型
   * @return Base::txBool
   */
  virtual Base::txBool IsSupportSceneType(const Base::ISceneLoader::ESceneType _sceneType) const TX_NOEXCEPT
      TX_OVERRIDE;

 public:
  /**
   * @brief 更新规划车辆的数据
   *
   * 使用此函数更新规划车辆的数据，这些数据可能来自外部或自动化系统。
   *
   * @param[in] timeMgr 当前时间管理器
   * @param[in] _egoSubType 车辆子类型
   * @param[in] egoInfoStr 车辆信息字符串
   * @return Base::txBool 更新成功返回 true，否则返回 false
   */
  virtual Base::txBool UpdatePlanningCarData(Base::TimeParamManager const& timeMgr,
                                             const Base::Enums::EgoSubType _egoSubType,
                                             const Base::txString& egoInfoStr) TX_NOEXCEPT TX_OVERRIDE;
#if USE_EgoGroup
  virtual Base::txBool UpdatePlanningCarHighlight(Base::TimeParamManager const& timeMgr,
                                                  const Base::txString& highlightStr) TX_NOEXCEPT TX_OVERRIDE;
#endif

  /**
   * @brief 注册规划车辆
   *
   * 此函数用于注册一辆规划车辆。
   *
   * @return Base::txBool 函数执行成功返回 true，否则返回 false
   */
  virtual Base::txBool RegisterPlanningCar() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断是否存在ego
   *
   * 此函数用于检查当前车辆列表中是否存在ego。若存在，返回 true；否则，返回 false。
   *
   * @return Base::txBool 若存在车辆，则返回 true；否则，返回 false
   */
  virtual Base::txBool HasEgo() const TX_NOEXCEPT;

  /**
   * @brief flush scene events
   *
   * @param timeMgr 时间管理器
   */
  virtual void FlushSceneEvents(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 执行环境感知功能
   *
   * 该函数用于执行当前TAD（Traffic Element System）环境中的任何需要被感知的环境数据，例如：摄像头数据、传感器数据等。
   * 它将通过时间管理器获取当前的时间，并且可以将感知到的数据存储到当前TAD的对象中。
   *
   * @param timeMgr Base::TimeParamManager 时间参数管理器，用于获取当前时间。
   */
  virtual void ExecuteEnvPerception(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间戳填充交通数据
   *
   * 这个函数用于根据时间戳填充交通数据，包括摄像头数据、传感器数据等。
   * 它会获取当前时间戳，并将感知到的数据存储到交通对象中。
   *
   * @param timeStamp Base::TimeParamManager 当前时间戳
   * @param outTraffic sim_msg::Traffic 用于存储交通数据的对象
   * @return Base::txBool 如果成功填充数据，则返回 true，否则返回 false
   */
  virtual Base::txBool FillingTrafficData(Base::TimeParamManager const& timeStamp,
                                          sim_msg::Traffic& outTraffic) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 初始化Assemble对象
   *
   */
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 初始化element manager对象
   *
   */
  virtual void CreateElemMgr() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  // Declare the virtual functions that will be implemented by the derived classes

  /**
   * @brief 更新信号
   *
   * 这个函数用于更新信号，以便系统能够更好地执行相关任务。
   *
   * @param[in] timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return bool 如果信号更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Signal(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 检查元素生命周期是否需要更新
   * @param timeMgr 时间参数管理器
   * @return 如果需要更新返回true, 否则返回false
   */
  virtual Base::txBool Update_CheckLifeCycle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 预模拟车辆更新
   *
   * 这个函数用于在模拟开始前执行车辆预模拟，可以用于对车辆信息进行更新。
   *
   * @param[in] timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return bool 如果车辆预模拟更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Vehicle_PreSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 车辆模拟更新
   *
   * 这个函数用于在模拟过程中更新车辆的状态。
   *
   * @param[in] timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return bool 如果车辆模拟更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Vehicle_Simulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 车辆仿真更新后处理函数
   *
   * 这个函数用于在仿真后过程中更新车辆的状态。
   *
   * @param[in] timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return Base::txBool 如果车辆后置处理更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Vehicle_PostSimulation(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 计算vehicles的动力学信息
   *
   * @param timeMgr 事件管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool Update_ComputeKineticsInfo(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新行人元素
   *
   * 更新行人元素的状态和属性，并在必要时触发相应的事件。
   *
   * @param timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return Base::txBool 如果更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Pedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新障碍物元素
   *
   * 根据当前时间更新障碍物元素的状态和属性，并在必要时触发相应的事件。
   *
   * @param timeMgr 时间参数管理器，包含当前时间戳等信息
   * @return Base::txBool 如果更新成功，则返回 true；否则返回 false
   */
  virtual Base::txBool Update_Obstacle(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新FCW状态
   *
   * @param timeMgr 事件参数管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool Update_FCW(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

  /**
   * @brief 更新车辆的额外信息
   *
   * @param timeMgr 时间参数管理器
   * @return Base::txBool 更新成功返回true
   */
  virtual Base::txBool Update_CarExternalInfo(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT;

 protected:
  // This code snippet defines three functions within the Base class.
  // Each function is a const member function and returns a value of a specific type.

  /**
   * @brief 启用交通信号灯过滤器功能
   *
   * 返回交通信号灯过滤器是否启用的状态。
   *
   * @return Base::txBool 交通信号灯过滤器是否启用
   */
  Base::txBool EnableTrafficVisionFilter() const TX_NOEXCEPT { return FLAGS_EnableTrafficVisionFilter; }

  /**
   * @brief 获取交通信号灯过滤器半径
   *
   * 获取交通信号灯过滤器的半径值。半径值表示在检测交通信号灯时，车辆与交通信号灯之间的最大距离。
   *
   * @return Base::txFloat 交通信号灯过滤器的半径值
   */
  Base::txFloat TrafficVisionFilterRadius() const TX_NOEXCEPT { return FLAGS_TrafficVisionFilterRadius; }

  /**
   * @brief 获取交通信号灯视野范围高度差
   *
   * 获取交通信号灯视野范围的高度差，用于过滤掉与自身车辆高度差较大的交通信号灯。
   *
   * @return Base::txFloat 交通信号灯视野范围高度差
   */
  Base::txFloat TrafficVisionFilterAltitudeDiff() const TX_NOEXCEPT { return FLAGS_TrafficVisionFilterAltitudeDiff; }
#if __TX_Mark__("SceneSketch")

 public:
  enum { version = 20230730 };

  struct SketchNode {
    SketchEnumType type = _plus_(SketchEnumType::default_scene);
    Base::ITrafficElementPtr source_ptr = nullptr;
    std::vector<Base::ITrafficElementPtr> participants;

    /**
     * @brief 结构体格式化字符串输出
     *
     * @return Base::txString
     */
    Base::txString Str() const TX_NOEXCEPT;
    friend std::ostream& operator<<(std::ostream& os, const SketchNode& v) TX_NOEXCEPT {
      os << v.Str();
      return os;
    }
  };

  struct SketchNodeCollection {
    Base::txFloat passTime = -1.0;
    tbb::concurrent_vector<SketchNode> conVec_sketch_nodes;
    sim_msg::Traffic snapshot_traffic;
    sim_msg::Location snapshot_ego;
    friend std::ostream& operator<<(std::ostream& os, const SketchNodeCollection& v) TX_NOEXCEPT {
      os << "{";
      os << TX_VARS_NAME(passTime, v.passTime);
      for (const auto& refNode : v.conVec_sketch_nodes) {
        os << refNode;
      }
      os << "}";
      return os;
    }
  };

  using SketchNodeCollectionArray = std::vector<SketchNodeCollection>;

 public:
  /**
   * @brief sketch初始化
   */
  virtual void InitSketch() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief sketch资源释放
   */
  virtual void ReleaseSketch() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief sketch 更新
   *
   * @param timeMgr 事件参数管理器
   * @param refTraffic traffic信息输出
   * @return Base::txSize
   */
  virtual Base::txSize UpdateSketch(const Base::TimeParamManager& timeMgr,
                                    const sim_msg::Traffic& refTraffic) TX_NOEXCEPT TX_OVERRIDE;
  virtual SketchEnumType SketchVoting() const TX_NOEXCEPT TX_OVERRIDE;
  static void CheckSketch(const SketchEnumType _type, SketchNodeCollection& refCollection,
                          Base::IElementManagerPtr _elemMgr, Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
#  if 1
  static Base::IVehicleElementPtr QueryVehiclePtr(Base::IElementManagerPtr _elemMgr,
                                                  const Base::txSysId vehId) TX_NOEXCEPT;
  static void CheckSketch_1(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_2(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_3(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_4(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_5(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_6(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_7(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_8(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_9(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                            Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_10(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_11(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_12(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_13(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_14(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_15(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_16(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static std::vector<Base::ITrafficElementPtr> CheckSketch_Obs(SketchNodeCollection& refCollection,
                                                               Base::IElementManagerPtr _elemMgr,
                                                               Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_17(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_18(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_19(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_20(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_21(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_22(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  static void CheckSketch_23(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                             Base::IVehicleElementPtr ego_elem_ptr) TX_NOEXCEPT;
  using PedCatalogFunc = std::function<Base::txBool(const Base::Enums::PEDESTRIAN_TYPE)>;
  static void CheckSketch_Cross(SketchNodeCollection& refCollection, Base::IElementManagerPtr _elemMgr,
                                Base::IVehicleElementPtr ego_elem_ptr, PedCatalogFunc call_func,
                                const SketchEnumType cur_enum_type) TX_NOEXCEPT;
#  endif

 protected:
  SketchNodeCollectionArray m_KeyFrameVec;
  std::array<SketchEnumType, Base::Enums::szSketchEnumType> m_array_enum_type;

  // 用于解析ego的位置信息
  sim_msg::Location m_pb_location;
#endif /*SceneSketch*/
};

using TAD_StandAlone_TrafficElementSystemPtr = std::shared_ptr<TAD_StandAlone_TrafficElementSystem>;

TX_NAMESPACE_CLOSE(TrafficFlow)
