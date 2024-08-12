// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <tuple>
#include "tad_sim.h"
#include "tad_traffic.h"
#include "tx_header.h"
#include "tx_scene_loader.h"

TX_NAMESPACE_OPEN(SceneLoader)

class TAD_SceneLoader : public Base::ISceneLoader {
 public:
  using txULong = Base::txULong;
  using txUInt = Base::txUInt;
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txInt = Base::txInt;
  using txString = Base::txString;

  using Location = SceneLoader::Traffic::Location;
  using VehType = SceneLoader::Traffic::VehType;
  using VehComp = SceneLoader::Traffic::VehComp;
  using VehInput = SceneLoader::Traffic::VehInput;
  using Beh = SceneLoader::Traffic::Beh;
  using VehExit = SceneLoader::Traffic::VehExit;
  using RouteGroup = SceneLoader::Traffic::RouteGroup;
  using TrafficFlowXML = SceneLoader::Traffic::TrafficFlow;

 public:
  TAD_SceneLoader();
  virtual ~TAD_SceneLoader() TX_DEFAULT;

  /**
   * @brief 获取类名
   *
   * 获取当前对象的类名，用于区分不同类型的对象。
   *
   * @return 返回当前对象的类名
   */
  virtual Base::txLpsz ClassName() const TX_NOEXCEPT TX_OVERRIDE { return _class_name; }

  /**
   * @brief 判断当前对象是否有效
   *
   * 检查当前对象是否符合有效的要求。一般来说，对象可能在磁盘或网络中，并可能因为有限资源、文件损坏等原因导致无法正常读取。
   *
   * @return 如果当前对象有效，返回 true，否则返回 false。
   */
  virtual Base::txBool IsValid() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 加载文件
   *
   * @param [in] filePath 文件路径，用于指定加载文件的位置。
   * @return 如果成功加载文件，则返回 true，否则返回 false。
   */
  virtual Base::txBool Load(const txString&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 从文件中加载资源
   *
   * 使用指定的文件路径、资源类型和扩展名来加载资源。
   *
   * @param filePath 文件路径，用于加载scene data
   * @param resourceType 文件路径，用于加载veh data
   * @param extension 扩展名，用于指定要加载的资源的扩展名。
   * @return 如果加载成功，则返回 true，否则返回 false。
   */
  virtual Base::txBool Load(const txString&, const txString&, const txString&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 解析场景事件
   *
   * @return Base::txBool
   */
  virtual Base::txBool ParseSceneEvent() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 加载物体
   *
   * @return Base::txBool
   */
  virtual Base::txBool LoadObjects() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 获取当前仿真的状态
   *
   * @return EStatus
   */
  virtual EStatus Status() const TX_NOEXCEPT TX_OVERRIDE { return _status; }

  /**
   * @brief 获取主车的类型，ego或trailer
   *
   * @return EgoType
   */
  virtual EgoType GetEgoType() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取scene源文件路径
   *
   * @return Base::txString
   */
  virtual Base::txString Source() const TX_NOEXCEPT TX_OVERRIDE { return _source; }

  /**
   * @brief 获取场景类型
   *
   * @return ESceneType
   */
  virtual ESceneType GetSceneType() const TX_NOEXCEPT TX_OVERRIDE { return ESceneType::eTAD; }

  /**
   * @brief 根据给定的 sim 文件路径，加载 txTAD 相关的场景文件和地图文件。
   *
   * @param _sim_path 来自 txTAD 文件的相对路径。
   * @param _scene_data_file 返回加载的场景数据文件路径。
   * @param _map_file 返回加载的地图文件路径。
   * @return 返回处理成功与否的结果。
   */
  static Base::txBool Sim2SceneFile(const Base::txString& _sim_path, Base::txString& _scene_data_file,
                                    Base::txString& _map_file) TX_NOEXCEPT;

 public:
  struct TAD_RouteViewer : public IRouteViewer {
    /**
     * @brief 判断当前加载器是否已经初始化。
     *
     * @return 返回 true 表示已经初始化，false 表示未初始化。
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_RouteViewer() TX_DEFAULT;
    virtual ~TAD_RouteViewer() TX_DEFAULT;

    /**
     * @brief 获取加载器的唯一标识符。
     *
     * @return 返回加载器的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief Get the route type
     *
     * @return ROUTE_TYPE type of route
     */
    virtual ROUTE_TYPE type() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线的起始点京都
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat startLon() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线的起始点纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat startLat() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线的的终点经度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat endLon() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线的终点纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat endLat() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路线的中间点
     *
     * @return std::vector< std::pair< Base::txFloat, Base::txFloat > >
     */
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > midPoints() const TX_NOEXCEPT TX_OVERRIDE;
    /*virtual Base::txFloat midLon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat midLat() const TX_NOEXCEPT TX_OVERRIDE;*/

    /**
     * @brief 获取道路的道路ID
     *
     * @return 返回道路的道路ID
     */
    virtual Base::txRoadID roidId() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取路段id
     *
     * @return Base::txSectionID
     */
    virtual Base::txSectionID sectionId() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取控制路径
     *
     * @return 控制路径节点向量
     */
    virtual control_path_node_vec control_path() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取相对路径
     *
     * @return txString 返回相对路径字符串
     */
    virtual txString relativePath() const TX_NOEXCEPT TX_OVERRIDE { return m_route.relativePath; }

    /**
     * @brief 获取绝对路径
     *
     * @return txString 返回绝对路径字符串
     */
    virtual txString absolutePath() const TX_NOEXCEPT TX_OVERRIDE { return m_route.absolutePath; }

    /**
     * @brief 获取字符串形式的数据
     *
     * 获取当前对象的字符串形式的数据。
     *
     * @return txString 返回当前对象的字符串形式数据
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化车辆路线信息
     *
     * 初始化车辆路线信息，提供一个源路线数据，这将被转化为适用于当前场景的路线数据。
     *
     * @param srcRoute 源路线数据
     */
    void Init(SceneLoader::Traffic::Route const& srcRoute) TX_NOEXCEPT;

    /**
     * @brief 初始化交通线路信息
     *
     * 通过设置交通线路的起点和终点坐标来初始化交通线路信息，适应于不同的场景。
     *
     * @param _id 线路ID，用于唯一识别
     * @param _type 线路类型
     * @param _startLon 起点经度坐标
     * @param _startLat 起点纬度坐标
     * @param _endLon 终点经度坐标
     * @param _endLat 终点纬度坐标
     */
    void Init(Base::txUInt _id, ROUTE_TYPE _type, Base::txFloat _startLon, Base::txFloat _startLat,
              Base::txFloat _endLon, Base::txFloat _endLat) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Route m_route;
  };
  using TAD_RouteViewerPtr = std::shared_ptr<TAD_RouteViewer>;

  struct TAD_AccelerationViewer : public IAccelerationViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_AccelerationViewer() TX_DEFAULT;
    virtual ~TAD_AccelerationViewer() TX_DEFAULT;

    /**
     * @brief 获取系统ID
     *
     * 获取当前实例对象的系统ID。
     *
     * @return Base::txSysId 当前实例对象的系统ID
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前实例的配置信息
     *
     * 当前实例的配置信息通常包括配置文件路径、设备绑定配置等信息。
     *
     * @return Base::txString 当前实例的配置信息
     */
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取结束条件
     *
     * 获取当前实例的结束条件，通常用于循环迭代中判断循环是否继续执行。
     *
     * @return Base::txString 结束条件
     */
    virtual Base::txString endCondition() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件动作类型
     *
     * 获取当前事件动作的类型，通常用于判断事件动作的类型，以便进行不同的操作。
     *
     * @return EventActionType 事件动作类型
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取时间戳与加速度对应的关联映射
     *
     * 获取一个包含时间戳与加速度关联的映射，用于存储加速度及其时间戳的对应关系。
     *
     * @return std::vector< std::pair< Base::txFloat, Base::txFloat > > 时间戳与加速度对应的关联映射
     */
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_acceleration_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;

    /**
     * @brief 获取事件结束条件向量
     *
     * 获取一个包含事件结束条件的向量，用于存储事件结束条件。
     *
     * @return std::vector< EventEndCondition_t > 事件结束条件向量
     */
    virtual std::vector<EventEndCondition_t> end_condition_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取时间加速向量
     *
     * 获取一个包含时间加速项的向量，用于表示加速项及其时间跨度的关系。
     *
     * @return std::vector< std::tuple< txFloat, txFloat, DistanceProjectionType, txUInt > > 时间加速向量
     */
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_acceleration_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取时间加速向量
     *
     * 获取一个包含时间加速项的向量，用于表示加速项及其时间跨度的关系。
     *
     * @return std::vector< std::tuple< Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt > >
     * 时间加速向量
     */
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_acceleration_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化指定交通信号信息的允许减速设置
     *
     * 初始化目标结构体中的允许减速设置为给定的减速信息，并更新数据结构。
     *
     * @param srcAcc 源减速信息，包括交通信号信息的允许减速设置
     */
    void Init(SceneLoader::Traffic::Acc const& srcAcc) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Acc m_acc;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using TAD_AccelerationViewerPtr = std::shared_ptr<TAD_AccelerationViewer>;

  struct TAD_MergeViewer : public IMergesViewer {
    /**
     * @brief txTADLoader 类的 IsInited 方法
     * 检查 txTADLoader 类对象是否已被初始化。
     * @return 如果 txTADLoader 类对象已被初始化，返回 true；否则返回 false。
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_MergeViewer() TX_DEFAULT;
    virtual ~TAD_MergeViewer() TX_DEFAULT;
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前实例的配置信息
     *
     * 当前实例的配置信息通常包括配置文件路径、设备绑定配置等信息。
     *
     * @return Base::txString 当前实例的配置信息
     */
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件动作类型
     *
     * 获取当前事件动作的类型，通常用于判断事件动作的类型，以便进行不同的操作。
     *
     * @return EventActionType 事件动作类型
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取时间戳方向对应的时间戳方向对
     *
     * 此函数返回一个包含时间戳方向对的向量。
     * 每个对中都包含一个时间戳、方向以及一个偏移量和持续时间。
     *
     * @return vector<tuple<Base::txFloat, Base::txInt, Base::txFloat, Base::txFloat>>
     */
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat /*Duration*/, Base::txFloat /*offset*/> >
    timestamp_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief ttc_direction_pair_vector 获取时间戳方向对应的时间戳方向对
     *
     * @return 返回包含时间戳方向对的向量。
     *         每个对中包含一个时间戳、方向、偏移量和持续时间。
     */
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    ttc_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief ttc_direction_pair_vector 获取时间戳方向对应的时间戳方向对
     *
     * @return 返回包含时间戳方向对的向量。
     *         每个对中包含一个时间戳、方向、偏移量和持续时间。
     */
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, DistanceProjectionType, Base::txFloat /*Duration*/,
                                   Base::txFloat /*offset*/, Base::txUInt> >
    egodistance_direction_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化允许的合并信息设置
     *
     * 初始化目标结构体中的允许合并信息设置为给定的合并信息，并更新数据结构。
     *
     * @param srcMerge 源合并信息，包括允许的合并信息设置
     */
    void Init(SceneLoader::Traffic::Merge const& srcMerge) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Merge m_merge;

    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using TAD_MergeViewerPtr = std::shared_ptr<TAD_MergeViewer>;

  struct TAD_VelocityViewer : public IVelocityViewer {
    /**
     * @brief txTADLoader 类的 IsInited 方法
     * 检查 txTADLoader 类对象是否已被初始化。
     * @return 如果 txTADLoader 类对象已被初始化，返回 true；否则返回 false。
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_VelocityViewer() TX_DEFAULT;
    virtual ~TAD_VelocityViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前实例的配置信息
     *
     * 当前实例的配置信息通常包括配置文件路径、设备绑定配置等信息。
     *
     * @return Base::txString 当前实例的配置信息
     */
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件动作类型
     *
     * 获取当前事件动作的类型，通常用于判断事件动作的类型，以便进行不同的操作。
     *
     * @return EventActionType 事件动作类型
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化指定速度信息
     *
     * 从给定的速度信息结构体中获取速度数据并初始化当前结构体。
     *
     * @param srcMerge 给定的速度信息结构体
     * @return void
     * @exception 无
     */
    void Init(const SceneLoader::Traffic::Velocity& srcMerge) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Velocity m_velocity;

    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using TAD_VelocityViewerPtr = std::shared_ptr<TAD_VelocityViewer>;

  struct TAD_PedestriansEvent_time_velocity_Viewer : public IPedestriansEventViewer {
   public:
    using txFloat = Base::txFloat;
    using txInt = Base::txInt;
    using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_PedestriansEvent_time_velocity_Viewer() TX_DEFAULT;
    virtual ~TAD_PedestriansEvent_time_velocity_Viewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前实例的配置信息
     *
     * 当前实例的配置信息通常包括配置文件路径、设备绑定配置等信息。
     *
     * @return Base::txString 当前实例的配置信息
     */
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件动作类型
     *
     * 获取当前事件动作的类型，通常用于判断事件动作的类型，以便进行不同的操作。
     *
     * @return EventActionType 事件动作类型
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > timestamp_direction_speed_tuple_vector()
        const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化时间和速度信息
     *
     * 根据给定的时间和速度信息结构体，初始化当前结构体。
     *
     * @param srcTimeVelocity 给定的时间和速度信息结构体
     * @return void
     * @exception 无
     */
    void Init(SceneLoader::Traffic::Time_velocity const& srcTimeVelocity) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Time_velocity m_time_velocity;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using TAD_PedestriansEvent_time_velocity_ViewerPtr = std::shared_ptr<TAD_PedestriansEvent_time_velocity_Viewer>;

  struct TAD_PedestriansEvent_event_velocity_Viewer : public IPedestriansEventViewer {
   public:
    using txFloat = Base::txFloat;
    using txInt = Base::txInt;
    using DistanceProjectionType = Base::ISceneLoader::DistanceProjectionType;

    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_PedestriansEvent_event_velocity_Viewer() TX_DEFAULT;
    virtual ~TAD_PedestriansEvent_event_velocity_Viewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前实例的配置信息
     *
     * 当前实例的配置信息通常包括配置文件路径、设备绑定配置等信息。
     *
     * @return Base::txString 当前实例的配置信息
     */
    virtual Base::txString profile() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件动作类型
     *
     * 获取当前事件动作的类型，通常用于判断事件动作的类型，以便进行不同的操作。
     *
     * @return EventActionType 事件动作类型
     */
    virtual EventActionType ActionType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<txFloat, txInt, txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_threshold_direction_velocity_tuple_vector() const TX_NOEXCEPT TX_OVERRIDE;

    virtual std::vector<std::pair<Base::txFloat, Base::txFloat> > timestamp_speed_pair_vector() const TX_NOEXCEPT
        TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat> > timestamp_direction_speed_tuple_vector()
        const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    ttc_direction_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<std::tuple<Base::txFloat, Base::txInt, Base::txFloat, DistanceProjectionType, Base::txUInt> >
    egodistance_direction_speed_pair_vector() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化时间和速度信息
     *
     * 根据给定的时间和速度信息结构体，初始化当前结构体。
     *
     * @param srcEventVelocity 给定的时间和速度信息结构体
     * @return void
     * @exception 无
     */
    void Init(SceneLoader::Traffic::Event_velocity const& srcEventVelocity) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Event_velocity m_event_velocity;
    EventActionType m_type = EventActionType::ActionTypeUndefined;
  };
  using TAD_PedestriansEvent_event_velocity_ViewerPtr = std::shared_ptr<TAD_PedestriansEvent_event_velocity_Viewer>;

  struct TAD_VehiclesViewer : public IVehiclesViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_VehiclesViewer() TX_DEFAULT;
    virtual ~TAD_VehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的唯一标识符是否有效。
     * @return 如果当前对象的唯一标识符有效则返回 true，否则返回 false。
     */
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取routeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的路由标识是否有效
     * @return 如果路由标识有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象lanid
     * @return 返回当前对象lanid
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的laneid是否有效
     * @return 如果laneid有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取开始的距离
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前对象的起始时间
     * @return 返回当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;
    /*virtual Base::txFloat lon() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat lat() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat alt() const TX_NOEXCEPT TX_OVERRIDE;*/

    /**
     * @brief 获取长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取acc的id
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 返回accid是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取mergeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 返回mergeid是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象的行为
     * @return 返回一个字符串，表示当前对象的行为
     */
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle的类型
     *
     * @return Base::txString
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txSysId follow() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat mergeTime() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat offsetTime() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<Base::txSysId> eventId() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat angle() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txString catalog() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat start_angle() const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化一个新的车辆
     *
     * 根据给定的车辆结构体初始化一个新的车辆。
     *
     * @param srcVehicle 给定的车辆结构体
     * @return void
     * @exception 无
     */
    void Init(SceneLoader::Traffic::Vehicle const& srcVehicle) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Vehicle m_vehicle;
  };
  using TAD_VehiclesViewerPtr = std::shared_ptr<TAD_VehiclesViewer>;

  struct TAD_EgoVehiclesViewer : public TAD_VehiclesViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_EgoVehiclesViewer() TX_DEFAULT;
    virtual ~TAD_EgoVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的唯一标识符是否有效。
     * @return 如果当前对象的唯一标识符有效则返回 true，否则返回 false。
     */
    virtual Base::txBool IsIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取routeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的路由标识是否有效
     * @return 如果路由标识有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象lanid
     * @return 返回当前对象lanid
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的laneid是否有效
     * @return 如果laneid有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsLaneIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取开始的距离
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前对象的起始时间
     * @return 返回当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 经度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat lon() const TX_NOEXCEPT;

    /**
     * @brief 纬度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat lat() const TX_NOEXCEPT;

    /**
     * @brief 高度
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
     * @brief 宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief accid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId accID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief accid是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsAccIdValid() const TX_NOEXCEPT;

    /**
     * @brief mergeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId mergeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief mergeid是否有效
     *
     * @return Base::txBool
     */
    virtual Base::txBool IsMergeIdValid() const TX_NOEXCEPT;
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat aggress() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat angle() const TX_NOEXCEPT TX_OVERRIDE { return 0.0; }

    /**
     * @brief 获取catalog描述
     *
     * @return txString
     */
    virtual txString catalog() const TX_NOEXCEPT TX_OVERRIDE { return ""; }
    virtual txString name() const TX_NOEXCEPT TX_OVERRIDE { return m_groupName; }

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化场景数据
     *
     * 使用输入参数srcEgo初始化场景数据。
     *
     * @param srcEgo 初始化参数，指定场景中的Ego车辆位置
     * @return 无
     */
    void Init(sim_msg::Location const& srcEgo) TX_NOEXCEPT;
    void Init(sim_msg::Location const& srcEgo, Base::txString groupName, Base::txInt egoId) TX_NOEXCEPT;
    void Init(sim_msg::Location const& srcEgo, Base::txString groupName, Base::txInt ego_id, Base::txFloat start_v,
              Base::txFloat max_v) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    sim_msg::Location m_ego;
    Base::txInt m_egoId = -1;
    Base::txString m_groupName = "";
    Base::txFloat m_startV = 0.0;
    Base::txFloat m_maxV = 0.0;
  };
  using TAD_EgoVehiclesViewerPtr = std::shared_ptr<TAD_EgoVehiclesViewer>;
#if Use_TruckEgo
  struct TAD_TruckVehiclesViewer : public TAD_EgoVehiclesViewer {
   public:
    TAD_TruckVehiclesViewer() TX_DEFAULT;
    virtual ~TAD_TruckVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using TAD_TruckVehiclesViewerPtr = std::shared_ptr<TAD_TruckVehiclesViewer>;
#endif /*Use_TruckEgo*/
  struct TAD_TrailerVehiclesViewer : public TAD_EgoVehiclesViewer {
   public:
    TAD_TrailerVehiclesViewer() TX_DEFAULT;
    virtual ~TAD_TrailerVehiclesViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;
  };
  using TAD_TrailerVehiclesViewerPtr = std::shared_ptr<TAD_TrailerVehiclesViewer>;

  struct TAD_PedestriansViewer : public IPedestriansViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_PedestriansViewer() TX_DEFAULT;
    virtual ~TAD_PedestriansViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取routeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的路由标识是否有效
     * @return 如果路由标识有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象lanid
     * @return 返回当前对象lanid
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始的距离
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前对象的起始时间
     * @return 返回当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 结束时间
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat end_t() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 视图类型
     *
     * @return Base::txString
     */
    virtual Base::txString type() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txBool hadDirection() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取起始速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取最大速度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString behavior() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取事件id集合
     *
     * @return std::vector< Base::txSysId >
     */
    virtual std::vector<Base::txSysId> eventId() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txFloat angle() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取catalog字符串描述
     *
     * @return txString
     */
    virtual txString catalog() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时角度大小
     *
     * @return txFloat
     */
    virtual txFloat start_angle() const TX_NOEXCEPT TX_OVERRIDE;
    virtual IVehiclesViewerPtr vehicle_view_ptr() const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化步行者数据
     *
     * 使用输入参数srcPedestrian初始化步行者数据。
     *
     * @param srcPedestrian 初始化参数，指定步行者的基本信息
     */
    void Init(SceneLoader::Traffic::Pedestrian const& srcPedestrian) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Pedestrian m_pedestrian;
  };
  using TAD_PedestriansViewerPtr = std::shared_ptr<TAD_PedestriansViewer>;

  struct TAD_ObstacleViewer : public IObstacleViewer {
   public:
    using STATIC_ELEMENT_TYPE = Base::Enums::STATIC_ELEMENT_TYPE;

    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_ObstacleViewer() TX_DEFAULT;
    virtual ~TAD_ObstacleViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取routeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的路由标识是否有效
     * @return 如果路由标识有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象lanid
     * @return 返回当前对象lanid
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始的距离
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 长度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 宽度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 高度
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取vehicle类型
     *
     * @return Base::txString
     */
    virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取静态元素类型
     *
     * @return STATIC_ELEMENT_TYPE
     */
    virtual Base::txString type() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat direction() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txString behavior() const TX_NOEXCEPT TX_OVERRIDE;
    virtual std::vector<Base::txSysId> eventId() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取角度
     *
     * @return txFloat
     */
    virtual txFloat angle() const TX_NOEXCEPT TX_OVERRIDE;
    virtual txString catalog() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始时角度
     *
     * @return txFloat
     */
    virtual txFloat start_angle() const TX_NOEXCEPT TX_OVERRIDE;

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化场景数据
     *
     * 使用输入参数srcVehicle初始化场景数据。
     *
     * @param srcVehicle 初始化参数，指定场景中的障碍物信息
     * @return 无返回值
     */
    void Init(SceneLoader::Traffic::Obstacle const& srcVehicle) TX_NOEXCEPT;
    STATIC_ELEMENT_TYPE str2type(Base::txString const& strType) const TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Obstacle m_obstacle;
  };
  using TAD_ObstacleViewerPtr = std::shared_ptr<TAD_ObstacleViewer>;

  struct TAD_SignlightsViewer : public ISignlightsViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_SignlightsViewer() TX_DEFAULT;
    virtual ~TAD_SignlightsViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取routeid
     *
     * @return Base::txSysId
     */
    virtual Base::txSysId routeID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 判断当前对象的路由标识是否有效
     * @return 如果路由标识有效则返回 true，否则返回 false
     */
    virtual Base::txBool IsRouteIdValid() const TX_NOEXCEPT;

    /**
     * @brief 获取当前对象lanid
     * @return 返回当前对象lanid
     */
    virtual Base::txLaneID laneID() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取开始的距离
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat start_s() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取当前对象的起始时间
     * @return 返回当前对象的起始时间
     */
    virtual Base::txFloat start_t() const TX_NOEXCEPT TX_OVERRIDE;

    virtual Base::txFloat l_offset() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取绿灯时间
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat time_green() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取黄灯时间
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat time_yellow() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取红灯时间
     *
     * @return Base::txFloat
     */
    virtual Base::txFloat time_red() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txFloat direction() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT { return Base::txString("UnImplenment"); }
#if USE_SignalByLane
    virtual Base::txFloat compliance() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString lane() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString phase() const TX_NOEXCEPT TX_OVERRIDE;
    virtual Base::txString status() const TX_NOEXCEPT TX_OVERRIDE;
#endif /*USE_SignalByLane*/

#if __TX_Mark__("signal control configure")
    virtual txString plan() const TX_NOEXCEPT TX_OVERRIDE;

    virtual txString junction() const TX_NOEXCEPT TX_OVERRIDE;

    virtual txString phaseNumber() const TX_NOEXCEPT TX_OVERRIDE;

    virtual txString signalHead() const TX_NOEXCEPT TX_OVERRIDE;

    virtual txString eventId() const TX_NOEXCEPT TX_OVERRIDE;
#endif /*signal control configure*/

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化路灯信息
     *
     * 初始化路灯信息，并使用输入参数srcSignlight。
     *
     * @param srcSignlight 源路灯信息，包含路灯相关参数和属性信息
     * @return 无返回值
     */
    void Init(SceneLoader::Traffic::Signlight const& srcSignlight) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    SceneLoader::Traffic::Signlight m_signlight;
  };
  using TAD_SignlightsViewerPtr = std::shared_ptr<TAD_SignlightsViewer>;
#if 1
  struct TAD_LocationViewer : public ILocationViewer {
    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual Base::txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

   public:
    TAD_LocationViewer();
    ~TAD_LocationViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE;

    virtual hadmap::txPoint PosGPS() const TX_NOEXCEPT TX_OVERRIDE;

    virtual Base::txString info() const TX_NOEXCEPT TX_OVERRIDE;

    /**
     * @brief 获取对象的字符串表示
     *
     * 获取当前对象的字符串格式化表示
     *
     * @return txString 当前字符串对象的字符串表示
     */
    virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

    friend std::ostream& operator<<(std::ostream& os, const TAD_LocationViewer& v) { return os << v.Str(); }

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief 初始化
     *
     * @param srcLocation 源Location
     * @return 无
     */
    void Init(const Location& srcLocation) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    Location m_location;
  };
  using TAD_LocationViewerPtr = std::shared_ptr<TAD_LocationViewer>;

  struct TAD_TrafficFlowViewer : public ITrafficFlowViewer {
    struct TAD_VehType : public IVehType {
      TAD_VehType();
      ~TAD_VehType() TX_DEFAULT;
      // <VehType id="101" Type="Sedan" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      virtual Base::txString vehicleType() const TX_NOEXCEPT TX_OVERRIDE;

      virtual VEHICLE_BEHAVIOR behavior() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat length() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat width() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat height() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_VehType& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化函数
       *
       * @param srcVehType veh类型
       */
      void Init(const VehType& srcVehType) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehType m_VehType;
    };
    using TAD_VehTypePtr = std::shared_ptr<TAD_VehType>;
    virtual VehTypePtr GetVehType(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;
    struct TAD_VehComp : public IVehComp {
      TAD_VehComp();
      ~TAD_VehComp() TX_DEFAULT;
      // <VehComp id = "1001" Type1 = "101" Percentage1 = "80" Behavior1 = "1" Aggress1 = "0.5" Type2 = "102"
      // Percentage2 = "15" Behavior2 = "1" Aggress2 = "0.6" Type3 = "103" Percentage3 = "50" Behavior3 = "1" Aggress3 =
      // "0.1" Info = "Platoon1" / >

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;
      virtual txInt Type(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;
      virtual txInt Percentage(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;
      virtual txInt Behavior(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;
      virtual txFloat Aggress(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      virtual txBool IsValid(const txInt idx) const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt PropCnt() const TX_NOEXCEPT TX_OVERRIDE { return m_VehComp.CompNodeArray.size(); }

      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_VehComp& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化函数
       *
       * @param srcVehComp vehComp
       */
      void Init(const VehComp& srcVehComp) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehComp m_VehComp;
    };
    using TAD_VehCompPtr = std::shared_ptr<TAD_VehComp>;

    virtual VehCompPtr GetVehComp(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;

    struct TAD_VehInput : public IVehInput {
      TAD_VehInput();
      ~TAD_VehInput() TX_DEFAULT;
      // <VehInput id="10011" Location="800011" Composition="1001" start_v="12" max_v="15" Distribution="Fixed"
      // TimeHeadway="3.0" Duration="250" Cover="100" Info="Platoon10"/>

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt location() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt composition() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取起始速度
       *
       * @return Base::txFloat
       */
      virtual txFloat start_v() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取最大速度
       *
       * @return Base::txFloat
       */
      virtual txFloat max_v() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat halfRange_v() const TX_NOEXCEPT TX_OVERRIDE;

      virtual Distribution distribution() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat timeHeadway() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt duration() const TX_NOEXCEPT TX_OVERRIDE;

      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txString cover_str() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_VehInput& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化函数
       *
       * @param srcVehInput vehInput
       */
      void Init(const VehInput& srcVehInput) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehInput m_VehInput;
    };
    using TAD_VehInputPtr = std::shared_ptr<TAD_VehInput>;

    struct TAD_Beh : public IBeh {
      // <Beh id="1" Type="Freeway" cc0="1.5" cc1="1.3" cc2="4"/>
      // <Beh id = "2" Type = "Arterial" AX = "2" BX_Add = "2" BX_Mult = "3" / >
      TAD_Beh();
      ~TAD_Beh() TX_DEFAULT;

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      virtual BehType type() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat cc0() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat cc1() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat cc2() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat AX() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat BX_Add() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat BX_Mult() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txFloat LCduration() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_Beh& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化
       *
       * @param srcBeh Beh类型
       */
      void Init(const Beh& srcBeh) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      Beh m_Beh;
    };
    using TAD_BehPtr = std::shared_ptr<TAD_Beh>;

    virtual BehPtr GetBeh(const txInt id) const TX_NOEXCEPT TX_OVERRIDE;
    struct TAD_VehExit : public IVehExit {
      TAD_VehExit();
      ~TAD_VehExit() TX_DEFAULT;
      // <VehExit id = "20001" Location = "200001" Cover = "0" Info = "Exit1" / >

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt location() const TX_NOEXCEPT TX_OVERRIDE;

      virtual std::set<Base::txLaneID> cover() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txString cover_str() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txString Info() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_VehExit& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化函数
       *
       * @param srcVehExit veh exit
       */
      void Init(const VehExit& srcVehExit) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      VehExit m_VehExit;
    };
    using TAD_VehExitPtr = std::shared_ptr<TAD_VehExit>;

    struct TAD_RouteGroup : public IRouteGroup {
      TAD_RouteGroup();
      ~TAD_RouteGroup() TX_DEFAULT;
      // <RouteGroup id="8002" Start="800020" Mid1="0" End1="800021" Percentage1="30" Mid2="0" End2="800022"
      // Percentage2="40" Mid3="0" End3="800023" Percentage3="30" />

      /**
       * @brief 获取当前对象的唯一标识符。
       * @return 当前对象的唯一标识符。
       */
      virtual txInt id() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt start() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt mid1() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt end1() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt percentage1() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt mid2() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt end2() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt percentage2() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt mid3() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt end3() const TX_NOEXCEPT TX_OVERRIDE;

      virtual txInt percentage3() const TX_NOEXCEPT TX_OVERRIDE;

      /**
       * @brief 获取对象的字符串表示
       *
       * 获取当前对象的字符串格式化表示
       *
       * @return txString 当前字符串对象的字符串表示
       */
      virtual Base::txString Str() const TX_NOEXCEPT TX_OVERRIDE;

      friend std::ostream& operator<<(std::ostream& os, const TAD_RouteGroup& v) { return os << v.Str(); }

     protected:
      friend class TAD_TrafficFlowViewer;

      /**
       * @brief 对象初始化函数
       *
       * @param srcRouteGroup routegroup
       */
      void Init(const RouteGroup& srcRouteGroup) TX_NOEXCEPT;

     protected:
      Base::txBool m_inited = false;
      RouteGroup m_RouteGroup;
    };
    using TAD_RouteGroupPtr = std::shared_ptr<TAD_RouteGroup>;

    TAD_TrafficFlowViewer() TX_DEFAULT;
    virtual ~TAD_TrafficFlowViewer() TX_DEFAULT;

    /**
     * @brief 获取当前对象的唯一标识符。
     * @return 当前对象的唯一标识符。
     */
    virtual Base::txSysId id() const TX_NOEXCEPT TX_OVERRIDE { return 0; }

    /**
     * @brief 判断是否已初始化
     *
     * 检查当前对象是否已经被初始化，返回布尔值。
     *
     * @return true 当前对象已初始化
     * @return false 当前对象未初始化
     */
    virtual txBool IsInited() const TX_NOEXCEPT TX_OVERRIDE { return m_inited; }

    virtual std::unordered_map<Base::txInt, VehInputPtr> GetAllVehInputData() const TX_NOEXCEPT TX_OVERRIDE;
    // virtual std::unordered_map<Base::txInt, PedInputPtr> GetAllPedInputData() const TX_NOEXCEPT TX_OVERRIDE;

    virtual std::unordered_map<Base::txInt, RouteGroupPtr> GetAllRouteGroupData() const TX_NOEXCEPT TX_OVERRIDE;

    virtual std::unordered_map<Base::txInt, VehExitPtr> GetAllVehExitData() const TX_NOEXCEPT TX_OVERRIDE;
    // virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT TX_OVERRIDE { return (m_inited &&
    // _NonEmpty_(m_trafficflow.RuleCompliance)) ? std::stod(m_trafficflow.RuleCompliance[0].proportion) :
    // FLAGS_DefaultRuleComplianceProportion; }

   protected:
    friend class TAD_SceneLoader;

    /**
     * @brief txTADLoader 类对象初始化
     *
     * 使用此函数初始化 txTADLoader 类对象。提供源文件路径等相关参数，以准备开始解析和使用文件数据。
     *
     * @param srcTrafficFlow 来自源文件的 TrafficFlowXML 类型数据
     * @return 无返回值
     */
    void Init(const TrafficFlowXML& srcTrafficFlow) TX_NOEXCEPT;

   protected:
    Base::txBool m_inited = false;
    TrafficFlowXML m_trafficflow;
  };
  using TAD_VehTypePtr = TAD_TrafficFlowViewer::TAD_VehTypePtr;
  using TAD_VehCompPtr = TAD_TrafficFlowViewer::TAD_VehCompPtr;
  using TAD_VehInputPtr = TAD_TrafficFlowViewer::TAD_VehInputPtr;
  using TAD_BehPtr = TAD_TrafficFlowViewer::TAD_BehPtr;
  using TAD_VehExitPtr = TAD_TrafficFlowViewer::TAD_VehExitPtr;
  using TAD_RouteGroupPtr = TAD_TrafficFlowViewer::TAD_RouteGroupPtr;
  using TAD_TrafficFlowViewerPtr = std::shared_ptr<TAD_TrafficFlowViewer>;
  using ISceneEventViewer = Base::ISceneLoader::ISceneEventViewer;
  using kvMap = Base::ISceneLoader::ISceneEventViewer::kvMap;
  using kvMapVec = Base::ISceneLoader::ISceneEventViewer::kvMapVec;
  using EventParam_t = Base::ISceneLoader::ISceneEventViewer::EventParam_t;
  using EventGroupParam_t = Base::ISceneLoader::ISceneEventViewer::EventGroupParam_t;
#endif

 public:
  /**
   * @brief 获取路由开始点信息
   * @param[out] refEgoData 当前车辆的位置信息
   * @return true 表示获取成功，false 表示获取失败
   */
  virtual Base::txBool GetRoutingInfo(sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取地图管理器初始化参数
   *
   * @param refParams 地图管理器初始化参数
   * @return true 表示获取成功，false 表示获取失败
   */
  virtual Base::txBool GetMapManagerInitParams(HdMap::HadmapCacheConCurrent::InitParams_t& refParams)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定路由ID的路由数据
   * @param id 路由ID
   * @return 返回路由数据视图对象，用于查询和展示路由信息
   */
  virtual IRouteViewerPtr GetRouteData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Ego Route Data object
   *
   * @return IRouteViewerPtr
   */
  virtual IRouteViewerPtr GetEgoRouteData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Accs Data object
   *
   * @param id
   * @return IAccelerationViewerPtr
   */
  virtual IAccelerationViewerPtr GetAccsData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Accs Event Data object
   *
   * @param id
   * @return IAccelerationViewerPtr
   */
  virtual IAccelerationViewerPtr GetAccsEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Merges Data object
   *
   * @param id
   * @return IMergesViewerPtr
   */
  virtual IMergesViewerPtr GetMergesData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Merges Event Data object
   *
   * @param id
   * @return IMergesViewerPtr
   */
  virtual IMergesViewerPtr GetMergesEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Velocity Data object
   *
   * @param id
   * @return IVelocityViewerPtr
   */
  virtual IVelocityViewerPtr GetVelocityData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Velocity Event Data object
   *
   * @param id
   * @return IVelocityViewerPtr
   */
  virtual IVelocityViewerPtr GetVelocityEventData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定id的行人事件数据视图
   * @param id 输入的id
   * @return 返回行人事件数据视图对象，用于查询和展示行人事件信息
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_TimeEvent(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取指定速度的行人事件数据视图
   * @param id ID
   * @return 返回行人事件数据视图对象，用于查询和展示行人事件信息
   */
  virtual IPedestriansEventViewerPtr GetPedestriansEventData_VelocityEvent(Base::txSysId const id)
      TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the All Vehicle Data object
   *
   * @return std::unordered_map<Base::txSysId, IVehiclesViewerPtr>
   */
  virtual std::unordered_map<Base::txSysId, IVehiclesViewerPtr> GetAllVehicleData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the All Pedestrian Data object
   *
   * @return std::unordered_map<Base::txSysId, IPedestriansViewerPtr>
   */
  virtual std::unordered_map<Base::txSysId, IPedestriansViewerPtr> GetAllPedestrianData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the All Signlight Data object
   *
   * @return std::unordered_map<Base::txSysId, ISignlightsViewerPtr>
   */
  virtual std::unordered_map<Base::txSysId, ISignlightsViewerPtr> GetAllSignlightData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the All Obstacle Data object
   *
   * @return std::unordered_map<Base::txSysId, IObstacleViewerPtr>
   */
  virtual std::unordered_map<Base::txSysId, IObstacleViewerPtr> GetAllObstacleData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取随机种子
   *
   * @return Base::txInt
   */
  virtual Base::txInt GetRandomSeed() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Ego Data object
   *
   * @return IVehiclesViewerPtr
   */
  virtual IVehiclesViewerPtr GetEgoData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Ego Trailer Data object
   *
   * @return IVehiclesViewerPtr
   */
  virtual IVehiclesViewerPtr GetEgoTrailerData() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Rule Compliance Proportion object
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat GetRuleComplianceProportion() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Location Data as id
   *
   * @param id
   * @return ILocationViewerPtr
   */
  virtual ILocationViewerPtr GetLocationData(const Base::txUInt id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Traffic Flow object
   *
   * @return ITrafficFlowViewerPtr
   */
  virtual ITrafficFlowViewerPtr GetTrafficFlow() TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the All Location Data object
   *
   * @return std::unordered_map<Base::txInt, ILocationViewerPtr>
   */
  virtual std::unordered_map<Base::txInt, ILocationViewerPtr> GetAllLocationData() TX_NOEXCEPT TX_OVERRIDE;

 private:
  /**
   * @brief Get the Vehicle Data as id
   *
   * @param id ID
   * @return IVehiclesViewerPtr
   */
  virtual IVehiclesViewerPtr GetVehicleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Pedestrian Data as id
   *
   * @param id ID
   * @return IPedestriansViewerPtr
   */
  virtual IPedestriansViewerPtr GetPedestrianData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Signlight Data as id
   *
   * @param id ID
   * @return ISignlightsViewerPtr
   */
  virtual ISignlightsViewerPtr GetSignlightData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief Get the Obstacle Data as id
   *
   * @param id ID
   * @return IObstacleViewerPtr
   */
  virtual IObstacleViewerPtr GetObstacleData(Base::txSysId const id) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief Get the Sim Simulation Planner Theta
   *
   * @return Base::txFloat
   */
  Base::txFloat GetSimSimulationPlannerTheta() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真地图文件经度
   *
   * @return Base::txFloat
   */
  Base::txFloat GetSimSimulationMapfileLongitude() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真地图文件经度
   *
   * @return Base::txFloat
   */
  Base::txFloat GetSimSimulationMapfileLatitude() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真地图文件高度
   *
   * @return Base::txFloat
   */
  Base::txFloat GetSimSimulationMapfileAltitude() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真规划路径开始点
   *
   * @param res
   * @return Base::txBool
   */
  Base::txBool GetSimSimulationPlannerRouteStart(std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT;

  /**
   * @brief 获取仿真规划开始速度
   *
   * @return Base::txFloat
   */
  Base::txFloat GetSimSimulationPlannerStartV() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真的traffic信息
   *
   * @return Base::txString
   */
  Base::txString GetSimSimulationTraffic() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真地图文件路径
   *
   * @return Base::txString
   */
  Base::txString GetSimSimulationMapFile() const TX_NOEXCEPT;

  /**
   * @brief 获取仿真规划的ego类型
   *
   * @return Base::txString
   */
  Base::txString GetSimSimulationPlannerEgoType() const TX_NOEXCEPT;
#if USE_EgoGroup
  Base::txFloat GetSimSimulationPlannerTheta(Base::txString egoGroup) const TX_NOEXCEPT;
  Base::txBool GetSimSimulationPlannerRouteStart(Base::txString egoGroup,
                                                 std::tuple<Base::txFloat, Base::txFloat>& res) const TX_NOEXCEPT;
  Base::txFloat GetSimSimulationPlannerStartV(Base::txString egoGroup) const TX_NOEXCEPT;
  Base::txString GetSimSimulationPlannerEgoType(Base::txString egoGroup) const TX_NOEXCEPT;
#endif

 public:
  using ParentClass = Base::ISceneLoader;

  /**
   * @brief 获取规划开始速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat PlannerStartV() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取规划开始theta
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat PlannerTheta() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取规划开始的最大速度
   *
   * @return Base::txFloat
   */
  virtual Base::txFloat PlannerVelocityMax() const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txString activePlan() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取规划轨迹是否可用
   *
   * @return Base::txBool
   */
  virtual Base::txBool Planner_Trajectory_Enabled() const TX_NOEXCEPT TX_OVERRIDE;
  virtual IRouteViewer::control_path_node_vec ControlPath() const TX_NOEXCEPT TX_OVERRIDE;

#if USE_EgoGroup

 public:
  virtual EgoType GetEgoType(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txInt GetEgoId(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual std::vector<Base::txString> GetAllEgoGroups() const TX_NOEXCEPT;
  virtual Base::txBool GetRoutingInfo(Base::txString egoGroup, sim_msg::Location& refEgoData) TX_NOEXCEPT TX_OVERRIDE;
  virtual IVehiclesViewerPtr GetEgoData(Base::txString egoGroup) TX_NOEXCEPT TX_OVERRIDE;
  virtual IVehiclesViewerPtr GetEgoTrailerData(Base::txString egoGroup) TX_NOEXCEPT TX_OVERRIDE;
  virtual IRouteViewerPtr GetEgoRouteData(Base::txString egoGroup) TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerStartV(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerTheta(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txFloat PlannerVelocityMax(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool Planner_Trajectory_Enabled(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
  virtual IRouteViewer::control_path_node_vec ControlPath(Base::txString egoGroup) const TX_NOEXCEPT TX_OVERRIDE;
#endif

 protected:
  SceneLoader::Sim::simulation_ptr m_DataSource_Scene = nullptr;
  SceneLoader::Traffic::traffic_ptr m_DataSource_Traffic = nullptr;
  Base::txLpsz _class_name;
};

using TAD_SceneLoaderPtr = std::shared_ptr<TAD_SceneLoader>;

TX_NAMESPACE_CLOSE(SceneLoader)
