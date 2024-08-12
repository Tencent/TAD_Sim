// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tuple>
#include "SceneLoader/tx_tadloader.h"
#include "tad_traffic.h"
#include "tx_header.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(SceneLoader)
class TAD_Cloud_SceneLoader : public Base::ISceneLoader {
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;

  using TAD_SignlightsViewer = TAD_SceneLoader::TAD_SignlightsViewer;
  using TAD_RouteViewer = TAD_SceneLoader::TAD_RouteViewer;
  using TAD_RouteViewerPtr = TAD_SceneLoader::TAD_RouteViewerPtr;

  using Location = SceneLoader::Traffic::Location;
  using VehType = SceneLoader::Traffic::VehType;
  using VehComp = SceneLoader::Traffic::VehComp;
  using VehInput = SceneLoader::Traffic::VehInput;
  using Beh = SceneLoader::Traffic::Beh;
  using VehExit = SceneLoader::Traffic::VehExit;
  using RouteGroup = SceneLoader::Traffic::RouteGroup;
  using TrafficFlowXML = SceneLoader::Traffic::TrafficFlow;
  using ISceneEventViewer = Base::ISceneLoader::ISceneEventViewer;
  using kvMap = Base::ISceneLoader::ISceneEventViewer::kvMap;
  using kvMapVec = Base::ISceneLoader::ISceneEventViewer::kvMapVec;

 public:
  TAD_Cloud_SceneLoader();
  virtual ~TAD_Cloud_SceneLoader() TX_DEFAULT;

 public:
  /**
   * @brief 获取主车类型
   *
   * @return EgoType 主车类型 ego or trailer
   */
  virtual EgoType GetEgoType() const TX_NOEXCEPT TX_OVERRIDE { return EgoType::eCloud; }

  /**
   * @brief 获取场景类型
   *
   * @return ESceneType 返回的场景类型eCloud
   */
  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eCloud; }

  /**
   * @brief 获取ego数据相关
   *
   * @return IVehiclesViewerPtr 返回ego的vehicel视图
   */
  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 返回类名
   *
   * @return Base::txLpsz 当前类名称
   */
  virtual Base::txLpsz ClassName() const TX_NOEXCEPT TX_OVERRIDE { return _class_name; }

  /**
   * @brief 场景加载器是否有效
   *
   * @return txBool 有效返回true
   */
  virtual txBool IsValid() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 加载对象
   *
   * 从文件中加载对象并存储到内存中。
   *
   * @return 返回 true 表示加载成功，false 表示加载失败。
   */
  virtual txBool LoadObjects() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 获取当前状态
   *
   * 返回当前状态，表示当前组件的状态。
   *
   * @return EStatus 返回当前组件的状态
   */
  virtual EStatus Status() const TX_NOEXCEPT TX_OVERRIDE { return _status; }

  /**
   * @brief 获取当前场景的源路径
   *
   * @return txString 场景文件的路径
   */
  virtual txString Source() const TX_NOEXCEPT TX_OVERRIDE { return _source; }

  /**
   * @brief 解析场景事件
   *
   * @return Base::txBool 场景事件解析成功返回true
   */
  virtual Base::txBool ParseSceneEvent() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取路由信息
   *
   * 从Ego路由管理器中获取路由信息，并将其拷贝到输入参数refEgoData中。
   *
   * @param refEgoData 一个用于存放Ego路由信息的Location对象
   * @return 返回获取路由信息是否成功
   */
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE {
    refEgoData.CopyFrom(m_EgoData);
    return true;
  }

  /**
   * @brief 获取路由数据
   *
   * 通过给定的 Base::txSysId 类型的 id 参数，从 CloudLoader 中获取对应的路由数据。
   *
   * @param id 一个 Base::txSysId 类型的参数，表示要获取路由数据的 id。
   * @return 返回一个 IRouteViewerPtr 类型的指针，表示获取到的路由数据。
   * @since 1.0.0
   */
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取accs的数据
   *
   * @param id 要获取视图的视图id
   * @return IAccelerationViewerPtr 返回一个视图指针
   */
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IAccelerationViewerPtr();
  }

  /**
   * @brief 获取accsevent的数据
   *
   * @param id 要获取视图的视图id
   * @return IAccelerationViewerPtr 返回一个视图指针
   */
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IAccelerationViewerPtr();
  }

  /**
   * @brief 获取merges数据
   *
   * @param id 要获取视图的视图id
   * @return IMergesViewerPtr 返回一个视图指针
   */
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE { return IMergesViewerPtr(); }

  /**
   * @brief 获取mergesEvent数据
   *
   * @param id 要获取视图的视图id
   * @return IMergesViewerPtr 返回一个视图指针
   */
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IMergesViewerPtr();
  }

  /**
   * @brief 获取时间事件相关的行人数据
   *
   * 通过提供的 Base::txSysId 类型的 id 参数，从 CloudLoader 中获取对应的时间事件相关的行人数据。
   *
   * @param id 一个 Base::txSysId 类型的参数，表示要获取行人数据的 id。
   * @return 返回一个 IPedestriansEventViewerPtr 类型的指针，表示获取到的行人数据。
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IPedestriansEventViewerPtr();
  }

  /**
   * @brief 获取速度事件相关的行人数据
   *
   * 通过提供的 Base::txSysId 类型的 id 参数，从 CloudLoader 中获取对应的速度事件相关的行人数据。
   *
   * @param id 一个 Base::txSysId 类型的参数，表示要获取行人数据的 id。
   * @return 返回一个 IPedestriansEventViewerPtr 类型的指针，表示获取到的行人数据。
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id)
      TX_NOEXCEPT TX_OVERRIDE {
    return IPedestriansEventViewerPtr();
  }

  /**
   * @brief 获取指定ID的车辆数据信息
   *
   * 通过提供的 txSysId 类型的 id 参数，从系统中获取对应的车辆数据信息。
   *
   * @param id txSysId 类型的参数，表示需要获取的车辆ID。
   * @return IVehiclesViewerPtr 返回一个 IVehiclesViewerPtr 类型的智能指针，指向获取到的车辆数据信息。
   */
  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IVehiclesViewerPtr();
  };

  /**
   * @brief 获取自主路径数据
   *
   * 通过调用此函数，可以获取当前自主车辆的路径数据。
   *
   * @return IRouteViewerPtr 返回一个指向 IRougeViewerPtr 类型的智能指针，该智能指针指向自主路径数据。
   */
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT TX_OVERRIDE { return IRouteViewerPtr(); }

  /**
   * @brief 获取指定ID的车辆速度数据
   *
   * 通过调用此函数，可以获取指定ID的车辆速度数据。
   *
   * @param id 需要获取速度数据的车辆ID
   * @return IVelocityViewerPtr 返回一个指向IVelocityViewerPtr类型的智能指针，指向获取到的速度数据
   */
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IVelocityViewerPtr();
  }

  /**
   * @brief 获取指定ID的速度事件数据
   *
   * 通过调用此函数，可以获取指定ID的速度事件数据。
   *
   * @param id 需要获取速度事件数据的车辆ID
   * @return IVelocityViewerPtr 返回一个指向IVelocityViewerPtr类型的智能指针，指向获取到的速度事件数据
   */
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IVelocityViewerPtr();
  }

  /**
   * @brief 获取trailer数据
   *
   * @return IVehiclesViewerPtr 返回一个指向IVehiclesViewerPtr类型的智能指针
   */
  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT TX_OVERRIDE { return IVehiclesViewerPtr(); }

  /**
   * @brief 获取所有车辆数据
   *
   * 通过调用此函数，可以获取系统中所有车辆的数据
   *
   * @return std::unordered_map<Base::txSysId, IVehiclesViewerPtr>
   * 返回一个包含所有车辆数据的哈希表，键为车辆ID，值为IVehiclesViewerPtr类型的智能指针
   */
  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定ID的行人数据
   *
   * 通过调用此函数，可以获取指定ID的行人数据
   *
   * @param id 指定ID
   * @return IPedestriansViewerPtr 返回指定ID的行人数据的智能指针
   */
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取所有行人数据
   *
   * 通过调用此函数，可以获取系统中所有行人数据的智能指针
   *
   * @return std::unordered_map<Base::txSysId, IPedestriansViewerPtr>
   * 返回所有行人数据的哈希表，键为行人ID，值为IPedestriansViewerPtr类型的智能指针
   */
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定ID的障碍物数据
   *
   * 通过调用此函数，可以获取系统中指定ID的障碍物数据
   *
   * @param id 指定ID
   * @return 返回障碍物数据的智能指针
   */
  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE {
    return IObstacleViewerPtr();
  }

  /**
   * @brief 获取所有障碍物数据
   *
   * 通过调用此函数，可以获取系统中所有障碍物数据
   *
   * @return 返回一个包含所有障碍物数据的哈希表，键为障碍物ID，值为智能指针指向的障碍物数据
   */
  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT TX_OVERRIDE;

  struct TAD_Cloud_RouteViewer : public TAD_SceneLoader::TAD_RouteViewer {
   public:
    TAD_Cloud_RouteViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_RouteViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return m_route.id; }

    /**
     * @brief 获取当前路由的类型
     *
     * @return ROUTE_TYPE 返回路由类型
     */
    virtual ROUTE_TYPE type() const TX_NOEXCEPT TX_OVERRIDE { return ROUTE_TYPE::ePos; }

    /**
     * @brief 获取起点的经度
     *
     * @return Base::txFloat 返回起点的经度
     */
    virtual Base::txFloat startLon() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起点纬度
     *
     * @return Base::txFloat 返回起点纬度
     */
    virtual Base::txFloat startLat() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取终点经度
     *
     * @return Base::txFloat 返回终点经度
     */
    virtual Base::txFloat endLon() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取终点纬度
     *
     * @return Base::txFloat 返回终点纬度
     */
    virtual Base::txFloat endLat() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路由中间的点
     *
     * @return std::vector< std::pair< Base::txFloat, Base::txFloat > > 返回一个经纬度点的集合
     */
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > midPoints() const TX_NOEXCEPT TX_OVERRIDE {
      return std::vector<std::pair<Base::txFloat, Base::txFloat> >();
    }

    /**
     * @brief 返回当前对象的字符串描述
     *
     * @return Base::txString
     */
    virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE { return "SceneLoader::Cloud::Route"; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化路由信息
     *
     * 使用传入的路由信息初始化路由数据，以便进行后续的路由分发等操作。
     *
     * @param srcRoute 传入的路由信息
     */
    void Init(SceneLoader::Traffic::Route const& srcRoute) TX_NOEXCEPT {
      m_route = srcRoute;
      m_inited = true;
    }

   protected:
    SceneLoader::Traffic::Route m_route;
  };
  using TAD_Cloud_RouteViewerPtr = std::shared_ptr<TAD_Cloud_RouteViewer>;

  struct TAD_Cloud_SignlightsViewer : public TAD_SceneLoader::TAD_SignlightsViewer {
   public:
    TAD_Cloud_SignlightsViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_SignlightsViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路由ID
     *
     * 获取路由信息ID，用于路由数据分发。
     *
     * @return 路由ID
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断路由ID是否有效
     *
     * 返回真（true）表示路由ID有效，返回假（false）表示路由ID无效。
     *
     * @return 路由ID是否有效
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的车道id
     *
     * @return Base::txLaneID 返回车道id
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时的s
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始的时间
     *
     * @return Base::txFloat 返回开始的时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取l_offset属性值
     *
     * 获取l_offset属性值，它是一个浮点型数据，表示当前的偏移量。
     *
     * @return 返回当前的l_offset属性值，以浮点型表示
     */
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取绿灯时间
     *
     * @return Base::txFloat 返回绿灯时间
     */
    virtual Base::txFloat time_green() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取黄灯时间
     *
     * @return Base::txFloat 返回黄灯时间
     */
    virtual Base::txFloat time_yellow() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取红灯的时间
     *
     * @return Base::txFloat 返回红灯时间
     */
    virtual Base::txFloat time_red() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前的方向
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat direction() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前signal对象的字符串表示
     *
     * @return Base::txString
     */
    virtual Base::txString Str() const TX_NOEXCEPT { return Base::txString("UnImplenment"); }
#if USE_SignalByLane

    /**
     * @brief 获取当前对象的compliance属性值
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat compliance() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前信号对象的车道
     *
     * @return Base::txString
     */
    virtual Base::txString lane() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前信号对象的phase
     *
     * @return Base::txString 返回当前时间的相位，以字符串表示
     */
    virtual Base::txString phase() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前信号对象的status
     *
     * @return Base::txString 返回字符串表示
     */
    virtual Base::txString status() const TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_SignalByLane*/

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化交通信号灯
     *
     * 根据给定的交通信号灯对象初始化当前对象。
     * 其中，输入参数 `srcSignlight` 是一个 `SceneLoader::Traffic::Signlight` 类型的常量引用，表示源交通信号灯对象。
     *
     * @param srcSignlight 源交通信号灯对象
     */
    void Init(SceneLoader::Traffic::Signlight const& srcSignlight) TX_NOEXCEPT;

   protected:
    SceneLoader::Traffic::Signlight m_signlight;
  };
  using TAD_Cloud_SignlightsViewerPtr = std::shared_ptr<TAD_Cloud_SignlightsViewer>;

  struct TAD_Cloud_VehiclesViewer : public IVehiclesViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 返回一个布尔值，表示当前对象是否已经初始化。
     * 如果已经初始化，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象是否已经初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_Cloud_VehiclesViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_VehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断ID是否有效
     *
     * 返回一个布尔值，表示当前对象的ID是否有效。
     * 如果ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的ID是否有效
     */
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取路线ID
     *
     * 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID。
     *
     * @return 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断路线ID是否有效
     *
     * 返回一个布尔值，表示当前对象的路线ID是否有效。
     * 如果路线ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的路线ID是否有效
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的车道id
     *
     * @return Base::txLaneID 返回车道id
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断车道ID是否有效
     *
     * 返回一个布尔值，表示当前对象的车道ID是否有效。
     * 如果车道ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的车道ID是否有效
     */
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取起始距离
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取左侧偏移量
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量
     */
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取acc事件id
     *
     * @return Base::txSysId 返回事件id
     */
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取accid是否有效
     *
     * @return Base::txBool 有效返回true
     */
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取mergeid
     *
     * @return Base::txSysId 返回merge事件id
     */
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断mergeid是否有效
     *
     * @return Base::txBool 有效返回true
     */
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取行为
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的行为。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的行为
     */
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前对象的激进程度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取跟随ID
     *
     * 返回一个 `Base::txSysId` 类型的值，表示当前对象的跟随ID。
     *
     * @return 返回一个 `Base::txSysId` 类型的值，表示当前对象的跟随ID
     */
    virtual Base::txSysId follow() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取合并时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的合并时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的合并时间
     */
    virtual Base::txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取偏移时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的偏移时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的偏移时间
     */
    virtual Base::txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件ID
     *
     * 返回一个 `std::vector<Base::txSysId>` 类型的值，表示当前对象的事件ID列表。
     *
     * @return 返回一个 `std::vector<Base::txSysId>` 类型的值，表示当前对象的事件ID列表
     */
    virtual std::vector<Base::txSysId> eventId() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取角度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle.angle; }

    /**
     * @brief 获取对象的catalog
     *
     * @return Base::txString
     */
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return m_vehicle.catalog; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化
     *
     * 使用给定的 `SceneLoader::Traffic::Vehicle` 对象初始化当前对象。
     *
     * @param srcVehicle 一个 `SceneLoader::Traffic::Vehicle` 类型的常量引用，表示用于初始化的源对象
     */
    void Init(SceneLoader::Traffic::Vehicle const& srcVehicle) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Vehicle m_vehicle;
  };
  using TAD_Cloud_VehiclesViewerPtr = std::shared_ptr<TAD_Cloud_VehiclesViewer>;

  struct TAD_Cloud_ObstacleViewer : public IObstacleViewer {
   public:
    using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_Cloud_ObstacleViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_ObstacleViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线ID
     *
     * 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID。
     *
     * @return 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断路线ID是否有效
     *
     * 返回一个布尔值，表示当前对象的路线ID是否有效。
     * 如果路线ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的路线ID是否有效
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的车道id
     *
     * @return Base::txLaneID 返回车道id
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始距离
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取左侧偏移量
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量
     */
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取元素类型
     *
     * @return STATIC_ELEMENT_TYPE 返回具体的类型
     */
    virtual Base::txString type() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txFloat direction() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取角度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return m_obstacle.angle; }

    /**
     * @brief 获取对象的catalog
     *
     * @return Base::txString
     */
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return m_obstacle.catalog; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化
     *
     * 使用给定的 `SceneLoader::Traffic::Obstacle` 对象初始化当前对象。
     *
     * @param srcVehicle 一个 `SceneLoader::Traffic::Obstacle` 类型的常量引用，表示用于初始化的源对象
     */
    void Init(SceneLoader::Traffic::Obstacle const& srcVehicle) TX_NOEXCEPT;
    STATIC_ELEMENT_TYPE str2type(Base::txString const& strType) const TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Obstacle m_obstacle;
  };
  using TAD_Cloud_ObstacleViewerPtr = std::shared_ptr<TAD_Cloud_ObstacleViewer>;

  struct TAD_Cloud_PedestriansViewer : public IPedestriansViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_Cloud_PedestriansViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_PedestriansViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线ID
     *
     * 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID。
     *
     * @return 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断路线ID是否有效
     *
     * 返回一个布尔值，表示当前对象的路线ID是否有效。
     * 如果路线ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的路线ID是否有效
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的车道id
     *
     * @return Base::txLaneID 返回车道id
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始距离
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间
     */
    virtual Base::txFloat end_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取左侧偏移量
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量
     */
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断是否已初始化
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化
     */
    virtual Base::txString type() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断是否有方向
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象是否有方向。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象是否有方向
     */
    virtual Base::txBool hadDirection() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取行为
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的行为。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的行为
     */
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件ID
     *
     * 返回一个 `std::vector<Base::txSysId>` 类型的值，表示当前对象的事件ID列表。
     *
     * @return 返回一个 `std::vector<Base::txSysId>` 类型的值，表示当前对象的事件ID列表
     */
    virtual std::vector<Base::txSysId> eventId() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取角度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return m_pedestrian.angle; }

    /**
     * @brief 获取对象的catalog
     *
     * @return Base::txString
     */
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return m_pedestrian.catalog; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化
     *
     * 根据给定的 `SceneLoader::Traffic::Pedestrian` 类型的参数 `srcPedestrian`，初始化当前对象。
     *
     * @param srcPedestrian 一个 `SceneLoader::Traffic::Pedestrian` 类型的参数，表示要初始化的行人对象
     */
    void Init(SceneLoader::Traffic::Pedestrian const& srcPedestrian) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Pedestrian m_pedestrian;
  };
  using TAD_Cloud_PedestriansViewerPtr = std::shared_ptr<TAD_Cloud_PedestriansViewer>;

  struct TAD_Cloud_EgoVehiclesViewer : public IVehiclesViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_Cloud_EgoVehiclesViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_EgoVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE {
      return (Base::txInt)m_ego.t();
      TAG("trick");
    }

    /**
     * @brief 判断ID是否有效
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象的ID是否有效。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象的ID是否有效
     */
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取路线ID
     *
     * 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID。
     *
     * @return 返回一个 `Base::txSysId` 类型的值，表示当前对象的路线ID
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断路线ID是否有效
     *
     * 返回一个布尔值，表示当前对象的路线ID是否有效。
     * 如果路线ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的路线ID是否有效
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的车道id
     *
     * @return Base::txLaneID 返回车道id
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断车道ID是否有效
     *
     * 返回一个布尔值，表示当前对象的车道ID是否有效。
     * 如果车道ID有效，则返回 `true`，否则返回 `false`。
     *
     * @return 返回一个布尔值，表示当前对象的车道ID是否有效
     */
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取起始距离
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始距离
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始时间
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取左侧偏移量
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的左侧偏移量
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
     * @brief 获取高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat alt() const TX_NOEXCEPT;

    /**
     * @brief 获取对象长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取acc事件id
     *
     * @return Base::txSysId 返回事件id
     */
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取accid是否有效
     *
     * @return Base::txBool 有效返回true
     */
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取mergeid
     *
     * @return Base::txSysId 返回merge事件id
     */
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断mergeid是否有效
     *
     * @return Base::txBool 有效返回true
     */
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取行为
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的行为。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的行为
     */
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆的激进程度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle的类型
     *
     * @return Base::txString
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txSysId follow() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual Base::txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取车辆类型
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的车辆类型
     */
    virtual sim_msg::Location& egoLocation() TX_NOEXCEPT { return m_ego; }

    /**
     * @brief 获取角度
     *
     * 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度。
     *
     * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的角度
     */
    virtual Base::txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取对象的catalog
     *
     * @return Base::txString
     */
    virtual Base::txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return "un-impl"; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    void Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT {
      m_ego = srcEgo;
      m_inited = true;
    }

   protected:
    Base::txBool m_inited = false;
    sim_msg::Location m_ego;
    Base::txInt m_egoId = -1;
  };
  using TAD_Cloud_EgoVehiclesViewerPtr = std::shared_ptr<TAD_Cloud_EgoVehiclesViewer>;

  struct TAD_Cloud_LocationViewer : public ILocationViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化。
     *
     * @return 返回一个 `Base::txBool` 类型的值，表示当前对象是否已初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_Cloud_LocationViewer();
    ~TAD_Cloud_LocationViewer() TX_DEFAULT;

    /**
     * @brief 获取当前视图对象的id
     *
     * @return Base::txSysId 返回对象的id
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取GPS坐标
     *
     * 返回一个 `hadmap::txPoint` 类型的值，表示当前对象的GPS坐标。
     *
     * @return 返回一个 `hadmap::txPoint` 类型的值，表示当前对象的GPS坐标
     */
    virtual hadmap::txPoint PosGPS() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取信息
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的信息。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的信息
     */
    virtual Base::txString info() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取字符串表示
     *
     * 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示。
     *
     * @return 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
    friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_LocationViewer& v) { return os << v.Str(); }
    // protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化
     *
     * 根据给定的 `Location` 类型的参数 `srcLocation`，初始化当前对象。
     *
     * @param srcLocation 一个 `Location` 类型的参数，表示要初始化的位置对象
     */
    void Init(const Location& srcLocation) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    Location m_location;
  };
  using TAD_Cloud_LocationViewerPtr = std::shared_ptr<TAD_Cloud_LocationViewer>;

  struct TAD_Cloud_TrafficFlowViewer : public ITrafficFlowViewer {
    struct TAD_Cloud_VehType : public IVehType {
      TAD_Cloud_VehType();
      ~TAD_Cloud_VehType() TX_DEFAULT;
      // <VehType id="101" Type="Sedan" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />

      /**
       * @brief 获取当前视图对象的id
       *
       * @return Base::txSysId 返回对象的id
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取vehicle类型
       *
       * @return VEHICLE_TYPE
       */
      virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取行为
       *
       * 返回一个 `Base::txString` 类型的值，表示当前对象的行为。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示当前对象的行为
       */
      virtual VEHICLE_BEHAVIOR behavior() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象长度
       *
       * @return Base::txFloat
       */
      virtual txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的宽度
       *
       * @return Base::txFloat
       */
      virtual txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的高度
       *
       * @return Base::txFloat
       */
      virtual txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取字符串表示
       *
       * 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_VehType& v) { return os << v.Str(); }

      friend class TAD_Cloud_TrafficFlowViewer;

      /**
       * @brief 获取字符串表示
       *
       * 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示当前对象的字符串表示
       */
      void Init(const VehType& srcVehType) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehType m_VehType;
    };
    using TAD_Cloud_VehTypePtr = std::shared_ptr<TAD_Cloud_VehType>;

    /**
     * @brief 获取车辆类型
     *
     * 根据给定的 `txInt` 类型的参数 `id`，返回一个 `VehTypePtr` 类型的值，表示对应的车辆类型。
     *
     * @param id 一个 `txInt` 类型的参数，表示要获取的车辆类型的ID
     * @return 返回一个 `VehTypePtr` 类型的值，表示对应的车辆类型
     */
    virtual VehTypePtr GetVehType(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;
    struct TAD_Cloud_VehComp : public IVehComp {
      TAD_Cloud_VehComp();
      ~TAD_Cloud_VehComp() TX_DEFAULT;
      // <VehComp id = "1001" Type1 = "101" Percentage1 = "80" Behavior1 = "1" Aggress1 = "0.5" Type2 = "102"
      // Percentage2 = "15" Behavior2 = "1" Aggress2 = "0.6" Type3 = "103" Percentage3 = "50" Behavior3 = "1" Aggress3 =
      // "0.1" Info = "Platoon1" / >

      /**
       * @brief 获取当前视图对象的id
       *
       * @return Base::txSysId 返回对象的id
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取类型
       *
       * 根据给定的索引 `idx`，返回对应的类型值。
       *
       * @param idx 一个 `txInt` 类型的参数，表示要获取类型的索引
       * @return 返回一个 `txInt` 类型的值，表示对应索引的类型值
       */
      virtual txInt Type(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取类型
       *
       * 根据给定的索引 `idx`，返回对应的类型值。
       *
       * @param idx 一个 `txInt` 类型的参数，表示要获取类型的索引
       * @return 返回一个 `txInt` 类型的值，表示对应索引的类型值
       */
      virtual txString TypeStr(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取百分比
       *
       * 根据给定的索引 `idx`，返回对应的百分比值。
       *
       * @param idx 一个 `txInt` 类型的参数，表示要获取百分比的索引
       * @return 返回一个 `txInt` 类型的值，表示对应索引的百分比值
       */
      virtual txInt Percentage(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取百分比
       *
       * 根据给定的索引 `idx`，返回对应的百分比值。
       *
       * @param idx 一个 `txInt` 类型的参数，表示要获取百分比的索引
       * @return 返回一个 `txInt` 类型的值，表示对应索引的百分比值
       */
      virtual txInt Behavior(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 根据索引获取veh的激进程度
       *
       * @param idx 提供的索引
       * @return txFloat
       */
      virtual txFloat Aggress(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 判断是否有效
       *
       * 根据给定的索引 `idx`，判断对应的值是否有效。
       *
       * @param idx 一个 `txInt` 类型的参数，表示要判断的索引
       * @return 返回一个 `txBool` 类型的值，表示对应索引的值是否有效
       */
      virtual txBool IsValid(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取属性数量
       *
       * 返回当前车辆组件的属性数量。
       *
       * @return 返回一个 `txInt` 类型的值，表示车辆组件的属性数量
       */
      virtual txInt PropCnt() const TX_NOEXCEPT TX_OVERRIDE { return m_VehComp.CompNodeArray.size(); }

      /**
       * @brief 获取信息
       *
       * 返回当前车辆组件的信息。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件的信息
       */
      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取字符串
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_VehComp& v) { return os << v.Str(); }

     protected:
      friend class TAD_Cloud_TrafficFlowViewer;

      /**
       * @brief 初始化
       *
       * 使用给定的 `VehComp` 对象初始化当前对象。
       *
       * @param srcVehComp 一个 `const VehComp &` 类型的参数，表示要使用的 `VehComp` 对象
       */
      void Init(const VehComp& srcVehComp) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehComp m_VehComp;
    };
    using TAD_Cloud_VehCompPtr = std::shared_ptr<TAD_Cloud_VehComp>;

    /**
     * @brief 获取车辆组件
     *
     * 根据给定的 ID 获取对应的车辆组件。
     *
     * @param id 一个 `txInt` 类型的参数，表示要获取的车辆组件的 ID
     * @return 返回一个 `VehCompPtr` 类型的值，表示获取到的车辆组件
     */
    virtual VehCompPtr GetVehComp(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;

    struct TAD_Cloud_VehInput : public IVehInput {
      TAD_Cloud_VehInput();
      ~TAD_Cloud_VehInput() TX_DEFAULT;
      // <VehInput id="10011" Location="800011" Composition="1001" start_v="12" max_v="15" Distribution="Fixed"
      // TimeHeadway="3.0" Duration="250" Cover="100" Info="Platoon10"/>

      /**
       * @brief 获取当前视图对象的id
       *
       * @return Base::txSysId 返回对象的id
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取位置
       *
       * 返回当前车辆组件的位置。
       *
       * @return 返回一个 `txInt` 类型的值，表示车辆组件的位置
       */
      virtual txInt location() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取组合
       *
       * 返回当前车辆组件的组合。
       *
       * @return 返回一个 `txInt` 类型的值，表示车辆组件的组合
       */
      virtual txInt composition() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取起始速度
       *
       * 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度。
       *
       * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的起始速度
       */
      virtual txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取最大速度
       *
       * 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度。
       *
       * @return 返回一个 `Base::txFloat` 类型的值，表示当前对象的最大速度
       */
      virtual txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取半范围
       *
       * 返回当前车辆组件的半范围。
       *
       * @return 返回一个 `txFloat` 类型的值，表示车辆组件的半范围
       */
      virtual txFloat halfRange_v() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取分布
       *
       * 返回当前车辆组件的分布。
       *
       * @return 返回一个 `Distribution` 类型的值，表示车辆组件的分布
       */
      virtual Distribution distribution() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取时间变化分布
       *
       * 返回当前车辆组件的时间变化分布。
       *
       * @return 返回一个 `TimeVaryingDistributionParamVec` 类型的值，表示车辆组件的时间变化分布
       */
      virtual TimeVaryingDistributionParamVec TimeVaryingDistribution() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取车头时距
       *
       * @return txFloat
       */
      virtual txFloat timeHeadway() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取持续时间
       *
       * @return txInt
       */
      virtual txInt duration() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取覆盖的车道ID集合
       *
       * 返回当前车辆组件覆盖的车道ID集合。
       *
       * @return 返回一个 `std::set<Base::txLaneID>` 类型的值，表示车辆组件覆盖的车道ID集合
       */
      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取覆盖的车道ID集合的字符串表示
       *
       * 返回当前车辆组件覆盖的车道ID集合的字符串表示。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件覆盖的车道ID集合的字符串表示
       */
      virtual txString cover_str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取信息
       *
       * 返回当前车辆组件的信息。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件的信息
       */
      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;
      /**
       * @brief 获取字符串表示
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示车辆组件的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_VehInput& v) { return os << v.Str(); }

     protected:
      friend class TAD_Cloud_TrafficFlowViewer;
      /**
       * @brief 初始化
       *
       * 使用给定的 `VehInput` 对象初始化当前车辆组件。
       *
       * @param srcVehInput 一个 `const VehInput &` 类型的参数，表示用于初始化的车辆输入对象
       */
      void Init(const VehInput& srcVehInput) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehInput m_VehInput;
    };
    using TAD_Cloud_VehInputPtr = std::shared_ptr<TAD_Cloud_VehInput>;

    struct TAD_Cloud_Beh : public IBeh {
      // <Beh id="1" Type="Freeway" cc0="1.5" cc1="1.3" cc2="4"/>
      // <Beh id = "2" Type = "Arterial" AX = "2" BX_Add = "2" BX_Mult = "3" / >
      TAD_Cloud_Beh();
      ~TAD_Cloud_Beh() TX_DEFAULT;

      /**
       * @brief 获取当前视图对象的id
       *
       * @return Base::txSysId 返回对象的id
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取行为类型
       *
       * 返回当前车辆组件的行为类型。
       *
       * @return 返回一个 `BehType` 类型的值，表示车辆组件的行为类型
       */
      virtual BehType type() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取cc0
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat cc0() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取cc1
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat cc1() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取cc2
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat cc2() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取AX属性
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat AX() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取BX_Add属性
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat BX_Add() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取BX_Mult属性
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat BX_Mult() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取LCduration属性
       *
       * @return txFloat 浮点数类型
       */
      virtual txFloat LCduration() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 返回当前对象的字符串表示
       *
       * @return Base::txString
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_Beh& v) { return os << v.Str(); }

     protected:
      friend class TAD_Cloud_TrafficFlowViewer;

      /**
       * @brief 初始化
       *
       * 使用给定的 `Beh` 对象初始化当前车辆组件。
       *
       * @param srcBeh 一个 `const Beh &` 类型的参数，表示用于初始化的车辆行为对象
       */
      void Init(const Beh& srcBeh) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      Beh m_Beh;
    };
    using TAD_Cloud_BehPtr = std::shared_ptr<TAD_Cloud_Beh>;

    /**
     * @brief 获取车辆行为对象
     *
     * 根据给定的 `id` 获取对应的车辆行为对象。
     *
     * @param id 一个 `const txInt` 类型的参数，表示要获取的车辆行为对象的 ID
     * @return 返回一个 `BehPtr` 类型的值，表示对应 ID 的车辆行为对象
     */
    virtual BehPtr GetBeh(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;
    struct TAD_Cloud_VehExit : public IVehExit {
      TAD_Cloud_VehExit();
      ~TAD_Cloud_VehExit() TX_DEFAULT;
      // <VehExit id = "20001" Location = "200001" Cover = "0" Info = "Exit1" / >

      /**
       * @brief 获取当前对象的id
       *
       * @return txInt 返回整数
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取位置
       *
       * 返回当前车辆组件的位置。
       *
       * @return 返回一个 `txInt` 类型的值，表示车辆组件的位置
       */
      virtual txInt location() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取覆盖的车道ID集合
       *
       * 返回当前车辆组件覆盖的车道ID集合。
       *
       * @return 返回一个 `std::set<Base::txLaneID>` 类型的值，表示车辆组件覆盖的车道ID集合
       */
      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取覆盖的车道ID集合的字符串表示
       *
       * 返回当前车辆组件覆盖的车道ID集合的字符串表示。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件覆盖的车道ID集合的字符串表示
       */
      virtual txString cover_str() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取信息
       *
       * 返回当前车辆组件的信息。
       *
       * @return 返回一个 `txString` 类型的值，表示车辆组件的信息
       */
      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取字符串表示
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示车辆组件的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_VehExit& v) { return os << v.Str(); }

     protected:
      friend class TAD_Cloud_TrafficFlowViewer;

      /**
       * @brief 获取字符串表示
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示车辆组件的字符串表示
       */
      void Init(const VehExit& srcVehExit) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehExit m_VehExit;
    };
    using TAD_Cloud_VehExitPtr = std::shared_ptr<TAD_Cloud_VehExit>;

    struct TAD_Cloud_RouteGroup : public IRouteGroup {
      TAD_Cloud_RouteGroup();
      ~TAD_Cloud_RouteGroup() TX_DEFAULT;
      // <RouteGroup id="8002" Start="800020" Mid1="0" End1="800021" Percentage1="30" Mid2="0" End2="800022"
      // Percentage2="40" Mid3="0" End3="800023" Percentage3="30" />

      /**
       * @brief 获取 ID
       *
       * 返回当前车辆组件的 ID。
       *
       * @return 返回一个 `txInt` 类型的值，表示车辆组件的 ID
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 启动
       *
       * 启动当前车辆组件。
       *
       * @return 返回一个 `txInt` 类型的值，表示启动操作的结果
       */
      virtual txInt start() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取mid1
       *
       * @return txInt 返回整数值
       */
      virtual txInt mid1() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取end1
       *
       * @return txInt 返回整数值
       */
      virtual txInt end1() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取percentage1
       *
       * @return txInt 返回整数值
       */
      virtual txInt percentage1() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取mid2
       *
       * @return txInt 返回整数值
       */
      virtual txInt mid2() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取end2
       *
       * @return txInt 返回整数值
       */
      virtual txInt end2() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取percentage2
       *
       * @return txInt 返回整数值
       */
      virtual txInt percentage2() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取mid3
       *
       * @return txInt 返回整数值
       */
      virtual txInt mid3() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取end3
       *
       * @return txInt 返回整数值
       */
      virtual txInt end3() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取percentage3
       *
       * @return txInt 返回整数值
       */
      virtual txInt percentage3() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取字符串表示
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示车辆组件的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;
      friend std::ostream& operator<<(std::ostream& os, const TAD_Cloud_RouteGroup& v) { return os << v.Str(); }

     protected:
      friend class TAD_Cloud_TrafficFlowViewer;

      /**
       * @brief 获取字符串表示
       *
       * 返回当前车辆组件的字符串表示。
       *
       * @return 返回一个 `Base::txString` 类型的值，表示车辆组件的字符串表示
       */
      void Init(const RouteGroup& srcRouteGroup) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      RouteGroup m_RouteGroup;
    };
    using TAD_Cloud_RouteGroupPtr = std::shared_ptr<TAD_Cloud_RouteGroup>;

    /**
     * @brief 构造函数
     *
     * 创建一个 `TAD_Cloud_TrafficFlowViewer` 对象。
     */
    TAD_Cloud_TrafficFlowViewer() TX_DEFAULT;
    virtual ~TAD_Cloud_TrafficFlowViewer() TX_DEFAULT;

    /**
     * @brief 构造函数
     *
     * 创建一个 `TAD_Cloud_TrafficFlowViewer` 对象。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return 0; }

    /**
     * @brief 检查是否已初始化
     *
     * 返回一个布尔值，表示当前车辆组件是否已初始化。
     *
     * @return 返回一个 `txBool` 类型的值，表示车辆组件是否已初始化
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

    /**
     * @brief 获取所有车辆输入数据
     *
     * 返回一个包含所有车辆输入数据的 `std::unordered_map` 对象。
     *
     * @return 返回一个 `std::unordered_map<Base::txInt, VehInputPtr>` 类型的值，表示所有车辆输入数据
     */
    virtual std::unordered_map<Base::txInt, VehInputPtr> GetAllVehInputData() const TX_NOEXCEPT TX_OVERRIDE;
    // virtual std::unordered_map<Base::txInt, PedInputPtr> GetAllPedInputData() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取所有路线组数据
     *
     * 返回一个包含所有路线组数据的 `std::unordered_map` 对象。
     *
     * @return 返回一个 `std::unordered_map<Base::txInt, RouteGroupPtr>` 类型的值，表示所有路线组数据
     */
    virtual std::unordered_map<Base::txInt, RouteGroupPtr> GetAllRouteGroupData() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取所有车辆出口数据
     *
     * 返回一个包含所有车辆出口数据的 `std::unordered_map` 对象。
     *
     * @return 返回一个 `std::unordered_map<Base::txInt, VehExitPtr>` 类型的值，表示所有车辆出口数据
     */
    virtual std::unordered_map<Base::txInt, VehExitPtr> GetAllVehExitData() const TX_NOEXCEPT TX_OVERRIDE;
    // virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT TX_OVERRIDE { return (m_inited &&
    // _NonEmpty_(m_trafficflow.RuleCompliance)) ? std::stod(m_trafficflow.RuleCompliance[0].proportion) :
    // FLAGS_DefaultRuleComplianceProportion; }

   protected:
    friend class TAD_Cloud_SceneLoader;

    /**
     * @brief 初始化
     *
     * 使用给定的 `TrafficFlowXML` 对象初始化当前对象。
     *
     * @param srcTrafficFlow 一个 `const TrafficFlowXML &` 类型的参数，表示用于初始化的 `TrafficFlowXML` 对象
     */
    void Init(const TrafficFlowXML& srcTrafficFlow) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    TrafficFlowXML m_trafficflow;
  };
  using TAD_Cloud_VehTypePtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehTypePtr;
  using TAD_Cloud_VehCompPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehCompPtr;
  using TAD_Cloud_VehInputPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehInputPtr;
  // using TAD_Cloud_PedInputPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_PedInputPtr;
  using TAD_Cloud_BehPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_BehPtr;
  using TAD_Cloud_VehExitPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_VehExitPtr;
  using TAD_Cloud_RouteGroupPtr = TAD_Cloud_TrafficFlowViewer::TAD_Cloud_RouteGroupPtr;
  using TAD_Cloud_TrafficFlowViewerPtr = std::shared_ptr<TAD_Cloud_TrafficFlowViewer>;

  /**
   * @brief 获取位置数据
   *
   * 根据给定的 ID 获取对应的位置数据。
   *
   * @param id 一个 `const Base::txUInt` 类型的参数，表示要获取的位置数据的 ID
   * @return 返回一个 `ILocationViewerPtr` 类型的值，表示获取到的位置数据
   */
  virtual ILocationViewerPtr GetLocationData(const Base::txUInt id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取交通流数据
   *
   * 返回一个包含交通流数据的 `ITrafficFlowViewerPtr` 对象。
   *
   * @return 返回一个 `ITrafficFlowViewerPtr` 类型的值，表示交通流数据
   */
  virtual ITrafficFlowViewerPtr GetTrafficFlow() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取所有位置数据
   *
   * 返回一个包含所有位置数据的 `std::unordered_map<Base::txInt, ILocationViewerPtr>` 对象。
   *
   * @return 返回一个 `std::unordered_map<Base::txInt, ILocationViewerPtr>` 类型的值，表示所有位置数据
   */
  virtual std::unordered_map<Base::txInt, ILocationViewerPtr> GetAllLocationData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 加载数据
   *
   * 根据给定的文件路径加载数据。
   *
   * @param file_path 一个 `Base::txString const &` 类型的参数，表示要加载的数据文件的路径
   * @return 返回一个 `Base::txBool` 类型的值，表示加载操作是否成功
   */
  virtual Base::txBool Load(Base::txString const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取地图管理器初始化参数
   *
   * 获取地图管理器的初始化参数，并将其存储在给定的 `HdMap::HadmapCacheConCurrent::InitParams_t` 类型的引用参数中。
   *
   * @param refParams 一个 `HdMap::HadmapCacheConCurrent::InitParams_t` 类型的引用参数，表示要获取的地图管理器初始化参数
   * @return 返回一个 `Base::txBool` 类型的值，表示操作是否成功
   */
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取交通信号灯数据
   *
   * 根据给定的系统 ID 获取交通信号灯数据。
   *
   * @param id 一个 `Base::txSysId const` 类型的参数，表示要获取交通信号灯数据的系统 ID
   * @return 返回一个 `ISignlightsViewerPtr` 类型的值，表示交通信号灯数据
   */
  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取所有交通信号灯数据
   *
   * 返回一个包含所有交通信号灯数据的 `std::unordered_map<Base::txSysId, ISignlightsViewerPtr>` 对象。
   *
   * @return 返回一个 `std::unordered_map<Base::txSysId, ISignlightsViewerPtr>` 类型的值，表示所有交通信号灯数据
   */
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取随机数种子
   *
   * 返回一个 `Base::txInt` 类型的随机数种子值。
   *
   * @return 返回一个 `Base::txInt` 类型的值，表示随机数种子
   */
  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取规则遵循比例
   *
   * 返回一个 `Base::txFloat` 类型的规则遵循比例值。
   *
   * @return 返回一个 `Base::txFloat` 类型的值，表示规则遵循比例
   */
  virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT TX_OVERRIDE;

 protected:
  SceneLoader::Traffic::traffic_ptr m_traffic_ptr = nullptr;
  Base::txLpsz _class_name;
};
using TAD_Cloud_SceneLoaderPtr = std::shared_ptr<TAD_Cloud_SceneLoader>;
TX_NAMESPACE_CLOSE(SceneLoader)
