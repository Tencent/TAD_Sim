// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "HdMap/tx_hashed_road_area.h"
#include "tx_component.h"
#include "tx_dead_line_area_manager.h"
#include "tx_element_manager_base.h"
#include "tx_header.h"
#include "tx_map_info.h"
#include "tx_scene_loader.h"
#include "tx_sim_time.h"

TX_NAMESPACE_OPEN(Base)

class LocationAgent {
 public:
  using LocationAgentPtr = std::shared_ptr<LocationAgent>;
  LocationAgent() TX_DEFAULT;
  ~LocationAgent() TX_DEFAULT;

  /**
   * @brief 获取gps点坐标
   *
   * @return hadmap::txPoint
   */
  hadmap::txPoint GPS() const TX_NOEXCEPT { return mPos.WGS84(); }

  /**
   * @brief 获取当前元素的位置坐标
   *
   * 返回一个包含经度和纬度的浮点数组，表示当前元素的精确地理坐标。
   *
   * @return 包含经度和纬度的浮点数组
   */
  Coord::txWGS84 vPos() const TX_NOEXCEPT { return mPos; }

  /**
   * @brief 初始化组件的逻辑
   *
   * 这个函数的主要功能是初始化组件的一些基本参数，比如浮点数量、字符串长度等，并在完成之后，返回成功/失败的状态。
   *
   * @param xsdPtr 这个参数用于存储当前位置的信息，具体的类型是指向一个位置查询器的指针。
   * @return 这个函数返回一个布尔值，当成功执行时返回 true，失败时返回 false。
   */
  Base::txBool Init(Base::ISceneLoader::ILocationViewerPtr xsdPtr) TX_NOEXCEPT;

  /**
   * @brief txElementGenerator 初始化方法
   *
   * 这个方法用于初始化 txElementGenerator 类的一个实例对象，并设置其所在的全局位置。
   *
   * @param globalPos 全局位置坐标，是基于地图的一个局部坐标系统中的一个点。
   * @return 返回一个布尔值，如果初始化成功则返回 true，否则返回 false。
   */
  Base::txBool Init(const hadmap::txPoint& globalPos TX_MARK("__pingsn__")) TX_NOEXCEPT;

  /**
   * @brief 初始化txElementGenerator对象
   *
   * 这个方法用于初始化一个txElementGenerator类的实例对象，并设置其所在的全局位置以及相关信息。
   *
   * @param globalPos 全局位置坐标，是基于地图的一个局部坐标系统中的一个点。
   * @param _locInfo 一个包含相关信息的结构体，包括需要的道路信息。
   * @return 返回一个布尔值，如果初始化成功则返回true，否则返回false。
   */
  Base::txBool Init(const hadmap::txPoint& globalPos TX_MARK("__virtual_city__"),
                    const Base::Info_Lane_t& _locInfo) TX_NOEXCEPT;

  /**
   * @brief txElementGenerator 是否有效
   *
   * 判断此实例是否有效。当此实例的初始化过程完成并且成功时，返回true；否则，返回false。
   *
   * @return 返回一个布尔值，true表示有效，false表示无效。
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief GetLocationId 获取位置id。
   *
   * 返回一个整数值，该值代表在地理区域中的唯一标识符。如果这个框架不存在或无效，则返回 0。
   *
   * @return 返回一个整数值，代表框架的位置编号。
   */
  Base::txInt LocationId() const TX_NOEXCEPT { return (IsValid() ? (mRawXSDLocationPtr->id()) : (InvalidId)); }

  /**
   * @brief 获取车道信息
   *
   * @return Base::Info_Lane_t
   */
  Base::Info_Lane_t GetLaneInfo() const TX_NOEXCEPT { return mInfoLane; }

  /**
   * @brief 获取当前对象在st坐标系下的纵向距离
   *
   * @return Base::txFloat
   */
  Base::txFloat GetDistanceOnCurve() const TX_NOEXCEPT { return mST.x(); }

  /**
   * @brief 获取当前对象在st坐标系下的侧向偏移
   *
   * @return Base::txFloat
   */
  Base::txFloat GetLateralOffset() const TX_NOEXCEPT { return mST.y(); }

  /**
   * @brief 获取当前对象的字符串输出
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief 根据指定的位置ID获取对应的LocationAgent对象
   *
   * @param locationId 要查询的位置ID
   * @return 返回对应位置ID的LocationAgent对象，如果不存在则返回空指针
   */
  static LocationAgentPtr GetLocationAgentById(const Base::txInt locationId) TX_NOEXCEPT {
    return sMapLocation[locationId];
  }

  /**
   * @brief 添加一个LocationAgent到指定的位置
   * @param pLocationAgent 指向要添加的LocationAgent对象的指针
   */
  static void AddLocationAgent(LocationAgentPtr) TX_NOEXCEPT;

  /**
   * @brief 清除所有 LocationAgent 对象
   *
   * 删除容器 sMapLocation 中所有已存储的 LocationAgent 对象。
   */
  static void ClearLocationAgent() TX_NOEXCEPT { sMapLocation.clear(); }

 protected:
  Base::ISceneLoader::ILocationViewerPtr mRawXSDLocationPtr;
  Base::txBool mValid = false;
  Coord::txWGS84 mPos;
  Base::Info_Lane_t mInfoLane;
  Base::txVec2 mST;
  static std::unordered_map<Base::txInt /* RouteGroup id*/, LocationAgentPtr> sMapLocation;
};

struct VehicleInputeAgent {
 public:
  using Distribution = Base::ISceneLoader::ITrafficFlowViewer::IVehInput::Distribution;
  using TimeVaryingDistributionParam = Base::ISceneLoader::ITrafficFlowViewer::IVehInput::TimeVaryingDistributionParam;
  using TimeVaryingDistributionParamVec =
      Base::ISceneLoader::ITrafficFlowViewer::IVehInput::TimeVaryingDistributionParamVec;
  using HashedLaneInfo = Geometry::SpatialQuery::HashedLaneInfo;
  using HashedLaneInfoOrthogonalListPtr = Geometry::SpatialQuery::HashedLaneInfoOrthogonalListPtr;
  struct VehicleInitParam_t {
    struct RandomKinectInfo_t {
      Base::txFloat mStartV = 0.0;
      Base::txFloat mMaxV = 0.0;
    };
    Base::ISceneLoader::ITrafficFlowViewer::VehTypePtr mVehTypePtr = nullptr;
    Base::ISceneLoader::ITrafficFlowViewer::BehPtr mBehPtr = nullptr;
    Base::txInt mPercentage = 0;
    Base::txFloat mAggress = 0.5;
    LocationAgent::LocationAgentPtr mLocationPtr = nullptr;
    Base::ISceneLoader::ITrafficFlowViewer::VehCompPtr mVehCompPtr = nullptr;
    Base::txOptional<txFloat> mForceStartV;

    /**
     * @brief 获取开始速度
     *
     * @return Base::txFloat 返回开始时的速度
     */
    Base::txFloat StartV() const TX_NOEXCEPT { return mRndKinectInfo.mStartV; }

    /**
     * @brief 获取最大速度
     *
     * @return Base::txFloat 返回最大速度
     */
    Base::txFloat MaxV() const TX_NOEXCEPT { return mRndKinectInfo.mMaxV; }

    /**
     * @brief 设置mStartV
     *
     * @param start_v 要设置的值
     */
    void SetStartV_Cloud(const txFloat start_v) TX_NOEXCEPT { mRndKinectInfo.mStartV = start_v; }

    /**
     * @brief 设置mMaxV
     *
     * @param max_v 要设置的最大速度
     */
    void SetMaxV_Cloud(const txFloat max_v) TX_NOEXCEPT { mRndKinectInfo.mMaxV = max_v; }

   protected:
    RandomKinectInfo_t mRndKinectInfo;
  };

  struct VehicleInitParamWithRandomKinect : public VehicleInitParam_t {
    Base::txFloat mStartV_native = 0.0;
    Base::txFloat mMaxV_native = 0.0;
    Base::txFloat mHalfRange_native = 0.0;

    /**
     * @brief 重置随机Kinect数据索引
     *
     * 重置用于获取随机Kinect数据的索引，以便从Kinect数据集中随机选择数据。
     */
    void ResetRndKinect() TX_NOEXCEPT { mRndKinectIdx = 0; }

    /**
     * @brief 更新随机Kinect数据
     *
     * 这个函数会更新随机Kinect数据索引，以便从Kinect数据集中随机选择数据。
     */
    void UpateRndKinect() TX_NOEXCEPT;

    /**
     * @brief 随机生成 Kinect 数据
     *
     * 这个函数会随机生成 Kinect 数据，并将生成的数据存储在 mData 数组中。
     * 输入参数 v 指定数据的值，max_v 指定数据的最大值，variance 指定数据的方差。
     */
    void GenerateRndKinect(const txFloat v, const txFloat max_v, const txFloat variance) TX_NOEXCEPT;

   protected:
    /**
     * @brief 获取随机Kinect数据的信息
     *
     * 获取Kinect数据的信息，即生成随机Kinect数据的起始值和最大值。
     *
     * @return RandomKinectInfo_t 返回生成随机Kinect数据的信息
     */
    RandomKinectInfo_t NativeKinect() const TX_NOEXCEPT { return RandomKinectInfo_t{mStartV_native, mMaxV_native}; }
    std::vector<RandomKinectInfo_t> mVecRandomKinectInfo;
    Base::txSize mRndKinectIdx = 0;
  };

 public:
  VehicleInputeAgent() TX_DEFAULT;
  ~VehicleInputeAgent() TX_DEFAULT;

  /**
   * @brief 初始化工具
   *
   * 初始化工具，需要传入场景加载器和Vehicle Input信息。
   *
   * @param _loader 场景加载器的智能指针
   * @param xsdPtr Vehicle Input信息的智能指针
   * @return 返回执行结果，true表示成功，false表示失败
   */
  Base::txBool Init(Base::ISceneLoaderPtr _loader,
                    Base::ISceneLoader::ITrafficFlowViewer::VehInputPtr xsdPtr) TX_NOEXCEPT;

  /**
   * @brief 判断当前元素是否有效
   *
   * @return Base::txBool 如果当前元素有效，返回true，否则返回false
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 检查当前的持续时间是否有效
   *
   * @return Base::txBool 如果当前的持续时间有效，返回 true，否则返回 false
   */
  Base::txBool IsValidDuration() const TX_NOEXCEPT { return (RawDuration()) < 0 || (CurDuration() <= RawDuration()); }

  /**
   * @brief 获取当前元素的输入 ID
   *
   * 该函数用于获取当前元素的输入 ID，该 ID 是一个 32 位整数。
   *
   * @return Base::txInt 返回当前元素的输入 ID
   */
  Base::txInt VehInputId() const TX_NOEXCEPT;

  /**
   * @brief 更新时间周期
   *
   * 此函数用于根据给定的时间步长更新时间周期。如果更新成功，则返回true；否则，返回false。
   *
   * @param timeStepInSecond 时间步长，以秒为单位
   * @return Base::txBool 如果更新成功，则返回true；否则，返回false
   */
  Base::txBool UpdatePeriod(const Base::txFloat timeStepInSecond) TX_NOEXCEPT;

  /**
   * @brief 重置时间周期
   *
   * 此函数用于重置时间周期。如果重置成功，则返回true；否则，返回false。
   *
   * @return Base::txBool 如果重置成功，则返回true；否则，返回false
   */
  Base::txBool ResetPeriod() TX_NOEXCEPT;

  /**
   * @brief 获取下一个交通状态的初始化参数
   *
   * 该函数用于获取下一个交通状态的初始化参数，包括交通状态的初始化参数、路径ID、哈希的道路信息和哈希的道路信息组列表指针。
   *
   * @return 一个元组，包含交通状态的初始化参数、路径ID、哈希的道路信息和哈希的道路信息组列表指针
   */
  std::tuple<VehicleInitParam_t, Base::txLaneID, HashedLaneInfo, HashedLaneInfoOrthogonalListPtr>
  GetNextProbabilityVehicleInitParam() TX_NOEXCEPT;

  /**
   * @brief 获取原始持续时间
   *
   * @return txFloat 原始持续时间
   */
  Base::txFloat RawDuration() const TX_NOEXCEPT { return mSrcDuration; }

  /**
   * @brief 获取当前持续时间
   *
   * 返回当前的持续时间，单位为秒。
   *
   * @return Base::txFloat 当前持续时间
   */
  Base::txFloat CurDuration() const TX_NOEXCEPT { return mCurDudation; }

  /**
   * @brief 获取当前周期
   *
   * 获取当前的计时周期，单位为秒。
   *
   * @return Base::txFloat 当前周期
   */
  Base::txFloat CurPeriod() const TX_NOEXCEPT { return mCurPeriod; }

  /**
   * @brief 获取车头时距
   *
   * @return Base::txFloat
   */
  Base::txFloat TimeHeadway() const TX_NOEXCEPT { return mCurTimeHeadway; }

  /**
   * @brief 更新车头时距
   *
   * @return Base::txFloat
   */
  Base::txFloat UpdateTimeHeadway() TX_NOEXCEPT;

  /**
   * @brief 获取当前对象的字符串表示
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT;

  /**
   * @brief 重置输入代理
   *
   * 重置输入代理，将其恢复到初始状态。
   *
   * @return Base::txBool 重置成功与否
   */
  Base::txBool ResetInputAgent() TX_NOEXCEPT;

  /**
   * @brief 判断元素是否激活
   *
   * 该函数用于判断元素是否处于激活状态。
   *
   * @return Base::txBool 如果激活则返回 true，否则返回 false
   */
  Base::txBool IsActive() const TX_NOEXCEPT { return m_active; }

  /**
   * @brief 设置元素的激活状态
   *
   * 当元素被激活时，可以对其进行相应的操作。
   *
   * @param[in] _f 要设置的激活状态，true 为激活，false 为非激活
   * @return txBool 返回设置后的激活状态
   */
  Base::txBool SetActive(const Base::txBool _f) TX_NOEXCEPT {
    m_active = _f;
    return IsActive();
  }

  /**
   * @brief 获取区域的位置指针
   *
   * 返回区域的位置指针，这个指针可以用于查询和操作区域的位置信息。
   *
   * @return LocationAgent::LocationAgentPtr 返回区域的位置指针
   */
  LocationAgent::LocationAgentPtr RegionLocationPtr() const TX_NOEXCEPT { return mInputLocationPtr; }

 protected:
  Base::ISceneLoader::ITrafficFlowViewer::VehInputPtr mRawXSDVehInputPtr = nullptr;
  enum { VehParams = 0, AppearLaneId = 1 };
  std::array<std::vector<Base::txInt>, 2> mProbability;
  Base::txInt mIndex = 0;
  Base::txBool mValid = false;
  std::vector<VehicleInitParamWithRandomKinect> mCandidateVehicleInitParamVector;
  std::vector<Base::txInt> mCandidateLaneIdVector;
  std::vector<std::tuple<HashedLaneInfo, HashedLaneInfoOrthogonalListPtr> > m_vec_HashedRoadInfoCache;
  Base::txFloat mCurTimeHeadway = 0.0;
  Base::txFloat mSrcTimeHeadway = 0.0;
  Base::txFloat mSrcDuration = FLT_MAX;
  Base::txFloat mCurDudation = 0.0;
  Distribution mTimeDistribution = Distribution::Undef;
  Base::txFloat mCurPeriod = 0.0;
  Base::txBool m_active = true;
  TX_MARK(" historical reason true");
  LocationAgent::LocationAgentPtr mInputLocationPtr = nullptr;
  TimeVaryingDistributionParamVec mSrcTimeVaryDistributionParams;
  Base::txOptional<TimeVaryingDistributionParam> mCurTimeVaryDistribution;
};

struct VehicleExitAgent {
 public:
  VehicleExitAgent() TX_DEFAULT;
  ~VehicleExitAgent() TX_DEFAULT;

  /**
   * @brief 初始化函数
   *
   * 初始化流量代理，获取传入的车辆退出信息。
   *
   * @param[in] _loader 场景加载器指针
   * @param[in] xsdPtr 车辆退出指针
   * @return Base::txBool 初始化是否成功
   */
  Base::txBool Init(Base::ISceneLoaderPtr _loader,
                    Base::ISceneLoader::ITrafficFlowViewer::VehExitPtr xsdPtr) TX_NOEXCEPT;

  /**
   * @brief 判断当前元素是否有效
   *
   * @return 如果当前元素有效，则返回 true；否则返回 false。
   */
  Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

  /**
   * @brief 获取当前元素的输入 ID
   *
   * 该函数用于获取当前元素的输入 ID，该 ID 在整个交通系统中具有唯一性。
   *
   * @return 当前元素的输入 ID，如果当前元素无效，则返回 -1。
   */
  Base::txInt VehInputId() const TX_NOEXCEPT;

  /**
   * @brief 检查元素是否到达deadline区域
   *
   * 该函数用于检查传入的元素是否到达deadline区域，如果传入的元素位于deadline区域，则函数返回 true。
   *
   * @param elemLaneUid 要检查的元素所在的道路 ID
   * @param elemDir 要检查的元素的方向向量
   * @param elementPt 要检查的元素在空间中的坐标点
   * @return 如果传入的元素位于deadline区域，则返回 true；否则返回 false。
   */
  Base::txBool ArriveAtDeadlineArea(const Base::txLaneUId& elemLaneUid, const Base::txVec3& elemDir,
                                    const Coord::txENU& elementPt) const TX_NOEXCEPT;

  /**
   * @brief 获取deadline区的顺时针逆序的点集
   *
   * 返回一个向量，表示deadline区的顺时针逆序的点集。
   *
   * @return deadline区的顺时针逆序的点集
   */
  std::vector<Base::txVec2> DeadlineAreaClockWiseClose() const TX_NOEXCEPT {
    return mDeadlineArea.DeadlineAreaClockWiseClose();
  }

 protected:
  Base::ISceneLoader::ITrafficFlowViewer::VehExitPtr mRawXSDVehExitPtr = nullptr;
  Base::txBool mValid = false;
  Base::DeadLineAreaManager mDeadlineArea;
};

class ISceneElementGenerator {
 public:
  virtual ~ISceneElementGenerator() TX_DEFAULT;

  /**
   * @brief 初始化辅助器
   *
   * 使用场景加载器初始化辅助器，并将有效地图范围设置为 \a valid_map_range。
   *
   * @param loader 场景加载器的智能指针
   * @param valid_map_range 有效地图范围
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual txBool Initialize(ISceneLoaderPtr, const Base::map_range_t& valid_map_range) TX_NOEXCEPT = 0;

  /**
   * @brief 生成元素
   *
   * 根据提供的TimeParamManager和IElementManagerPtr，使用当前类的实现生成元素。
   *
   * @param managerParam 时间参数管理器
   * @param elementManager 元素管理器的智能指针
   * @return 生成成功返回 true，否则返回 false
   */
  virtual txBool Generate(TimeParamManager const&, IElementManagerPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 删除元素
   *
   * 根据提供的 TimeParamManager 和 IElementManagerPtr，使用当前类的实现删除元素。
   *
   * @param managerParam 时间参数管理器
   * @param elementManager 元素管理器的智能指针
   * @return 删除成功返回 true，否则返回 false
   */
  virtual txBool Erase(TimeParamManager const&, IElementManagerPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 重新路由元素
   *
   * 根据提供的 TimeParamManager 和 IElementManagerPtr，使用当前类的实现重新路由元素。
   *
   * @param managerParam 时间参数管理器
   * @param elementManager 元素管理器的智能指针
   * @return 重新路由成功返回 true，否则返回 false
   */
  virtual txBool ReRoute(TimeParamManager const&, IElementManagerPtr) TX_NOEXCEPT = 0;

  /**
   * @brief 释放元素
   *
   * 释放由派生类实现的元素资源，并且将元素状态设置为未分配。
   *
   * @return 释放成功返回 true，否则返回 false
   */
  virtual txBool Release() TX_NOEXCEPT;

  /**
   * @brief 重置生成器
   *
   * 重置生成器的内部状态，使其能够重新开始生成元素。
   *
   * @return 重置成功返回 true，否则返回 false
   */
  virtual txBool ResetGenerator() TX_NOEXCEPT { return true; }

  /**
   * @brief 检查输入点是否安全
   *
   * 使用最近点算法检查输入点是否位于元素生成器的生成范围内。
   * 如果输入点是安全的，函数将返回true，否则返回false。
   *
   * @param inputPt 要检查的输入点
   * @param nearestPt 输入点最近的元素生成点
   * @return 检查结果，如果输入点安全则返回true，否则返回false
   */
  virtual Base::txBool SafeInputRegion(const Base::txVec2& inputPt,
                                       const Base::txVec2& nearestPt) const TX_NOEXCEPT = 0;

  /**
   * @brief 获取场景中车辆最多数量
   *
   * @return Base::txSize
   */
  virtual Base::txSize SceneMaxVehicleSize() const TX_NOEXCEPT = 0;

  /**
   * @brief 获取输入区域的坐标位置
   *
   * 此函数用于获取输入区域的坐标位置，即参与计算过程的点集中的坐标位置。
   *
   * @return std::vector< Coord::txWGS84 > 返回输入区域的坐标位置集合
   */
  virtual std::vector<Coord::txWGS84> InputRegionLocations() const TX_NOEXCEPT = 0;

 protected:
};

using ISceneElementGeneratorPtr = std::shared_ptr<ISceneElementGenerator>;

TX_NAMESPACE_CLOSE(Base)
