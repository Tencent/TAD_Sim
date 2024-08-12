// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_ai_vehicle_element.h"
#include "its_sim.pb.h"
#include "tx_serialization.h"
#include "tx_tc_tad_element_generator.h"

#include <vector>
#if _OutputSerialLog_
#  define SerializeInfo(_MSG_) LOG_IF(INFO, FLAGS_LogLevel_Serialize) << "[Serialize] " << _MSG_
#else /*_OutputSerialLog_*/
#  define SerializeInfo(_MSG_)
#endif /*_OutputSerialLog_*/

TX_NAMESPACE_OPEN(TrafficFlow)

class TAD_Cloud_AI_VehicleElement : public TAD_AI_VehicleElement {
 public:
  using ParentClass = TAD_AI_VehicleElement;

 public:
  /**
   * @brief 初始化指定的车辆
   * @param[in] vehId 车辆ID
   * @param[in] veh_init_param 车辆的初始化参数
   * @param[in] startLaneId 车辆的起始道路ID
   * @param[in] pSceneLoader 场景加载器指针
   * @return 初始化成功返回 true，否则返回 false
   */
  virtual Base::txBool Initialize(const Base::txSysId vehId, const VehicleInitParam_t& veh_init_param,
                                  const Base::txLaneID startLaneId, Base::ISceneLoaderPtr) TX_NOEXCEPT;
  using TAD_AI_VehicleElement::Initialize;

#if __TX_Mark__("Dynamic Route")

  /**
   * @brief 获取当前路由组ID
   * @return 当前路由组ID
   */
  virtual Base::txSysId GetCurRouteGroupId() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.RouteId(); }

  /**
   * @brief 获取当前子路由ID
   * @return 当前子路由ID
   */
  virtual Base::txInt SubRouteId() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.SubRouteId(); }

  /**
   * @brief 是否支持重定向
   * @return true 支持重定向；false 不支持重定向
   */
  virtual Base::txBool IsSupportReRoute() const TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 路由重定向
   * @param routeGroupId 路由组ID
   * @param subRouteId 子路由ID
   * @param routeAI 路由AI对象
   * @return true 成功重定向；false 重定向失败
   */
  virtual Base::txBool ReRoute(const Base::txSysId routeGroupId, const Base::txInt subRouteId,
                               const TrafficFlow::Component::RouteAI&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 是否存在路径
   * @return true 存在有效路径；false 无效路径
   */
  virtual Base::txBool HasRoute() const TX_NOEXCEPT TX_OVERRIDE { return mRouteAI.IsValid(); }
#endif /*__TX_Mark__("Dynamic Route")*/

  /**
   * @brief 检查是否超过deadline
   * @return 是否超过deadline
   */
  virtual txBool CheckDeadLine() TX_NOEXCEPT TX_OVERRIDE { return true; }

  /**
   * @brief 更新云中的物体
   *
   * 根据veh对象的位置和视野范围，更新云中的物体。
   *
   * @param timeMgr            时间管理器
   * @param _veh                待识别的车辆
   * @param maxErrDist          识别准确性，用于计算物体与veh之间的最大距离
   * @return                   返回true表示更新成功，否则表示更新失败
   */
  virtual Base::txBool UpdateCloud(Base::TimeParamManager const& timeMgr, const its::txVehicle& _veh,
                                   const double maxErrDist) TX_NOEXCEPT;
  struct Cloud_MultiRegion_Params {
   public:
    /**
     * @brief 结构体初始化
     *
     * @param _veh 原始的pb数据，用于初始化结构体
     */
    void Initialize(const its::txVehicle& _veh) TX_NOEXCEPT;

    /**
     * @brief 结构体初始化
     *
     * @param _car 原始的Car pb结构，用于初始化
     */
    void Initialize(const sim_msg::Car& _car) TX_NOEXCEPT;

    /**
     * @brief 获取经度
     *
     * @return Base::txFloat
     */
    Base::txFloat lng() const TX_NOEXCEPT { return __Lon__(vehicle_pos); }

    /**
     * @brief 获取纬度
     *
     * @return Base::txFloat
     */
    Base::txFloat lat() const TX_NOEXCEPT { return __Lat__(vehicle_pos); }

    /**
     * @brief 道路id
     *
     * @return Base::txRoadID
     */
    Base::txRoadID road_id() const TX_NOEXCEPT {
      return (lane_loc_info.IsOnLane() ? (lane_loc_info.onLaneUid.roadId) : (lane_loc_info.onLinkFromLaneUid.roadId));
    }

    /**
     * @brief section id
     *
     * @return Base::txSectionID
     */
    Base::txSectionID section_id() const TX_NOEXCEPT {
      return (lane_loc_info.IsOnLane() ? (lane_loc_info.onLaneUid.sectionId)
                                       : (lane_loc_info.onLinkFromLaneUid.sectionId));
    }

    /**
     * @brief 车道id
     *
     * @return Base::txLaneID
     */
    Base::txLaneID lane_id() const TX_NOEXCEPT {
      return (lane_loc_info.IsOnLane() ? (lane_loc_info.onLaneUid.laneId) : (lane_loc_info.onLinkFromLaneUid.laneId));
    }

    /**
     * @brief 所连接的下条路id
     *
     * @return Base::txRoadID
     */
    Base::txRoadID to_road_id() const TX_NOEXCEPT { return lane_loc_info.onLinkToLaneUid.roadId; }

    /**
     * @brief 连接的下个section id
     *
     * @return Base::txSectionID
     */
    Base::txSectionID to_section_id() const TX_NOEXCEPT { return lane_loc_info.onLinkToLaneUid.sectionId; }

    /**
     * @brief 连接的下个lane id
     *
     * @return Base::txLaneID
     */
    Base::txLaneID to_lane_id() const TX_NOEXCEPT { return lane_loc_info.onLinkToLaneUid.laneId; }

    /**
     * @brief 获取是否在link上
     *
     * @return Base::txBool
     */
    Base::txBool on_link() const TX_NOEXCEPT { return lane_loc_info.IsOnLaneLink(); }
    Base::txFloat vehicle_length;
    Base::txFloat vehicle_width;
    Base::txFloat vehicle_height;
    Base::txSysId vehicle_id;
    Base::Enums::VEHICLE_TYPE vehicle_type;
    Base::Info_Lane_t lane_loc_info;
    hadmap::txPoint vehicle_pos;
    Base::txFloat vehicle_speed;
    Base::txFloat vehicle_yaw;
  };

  /**
   * @brief 云端区域初始化
   *
   * @param _veh 用于初始化的原始Vehicle数据
   * @param sceneLoader 场景加载器，用于加载云数据等
   * @return Base::txBool
   */
  virtual Base::txBool Initialize_Cloud_MultiRegion(const its::txVehicle& _veh, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 云端区域初始化
   *
   * @param _veh 用于初始化的原始Car数据
   * @param sceneLoader 场景加载器，用于加载云数据等
   * @return Base::txBool
   */
  virtual Base::txBool Initialize_Cloud_MultiRegion(const sim_msg::Car& _car, Base::ISceneLoaderPtr) TX_NOEXCEPT;

  /**
   * @brief 云端区域初始化
   *
   * @param _veh 用于初始化的原始参数信息
   * @param sceneLoader 场景加载器，用于加载云数据等
   * @return Base::txBool
   */
  virtual Base::txBool Initialize_Cloud_MultiRegion(const Cloud_MultiRegion_Params& _info,
                                                    Base::ISceneLoaderPtr) TX_NOEXCEPT;

 public:
  /**
   * @brief 填充仿真状态
   * @param timeMgr 时间管理器
   * @param outSceneStatue 输出场景状态
   * @return true: 成功, false: 失败
   */
  virtual Base::txBool FillingSimStatus(Base::TimeParamManager const& timeMgr,
                                        its::txSimStatus& outSceneStatue) TX_NOEXCEPT;

  /**
   * @brief 对模拟的车辆进行元素计算
   * @param timeStamp 当前时间戳
   * @param pTxVehicle 存放模拟车辆元素计算结果的缓存指针
   * @return 返回模拟车辆的指针
   */
  virtual its::txVehicle* FillingSimStatus(txFloat const timeStamp, its::txVehicle* pTxVehicle) TX_NOEXCEPT;

 protected:
  /**
   * @brief 获取云车辆类型
   * @return 云车辆类型
   */
  virtual its::txVehicleType CloudVehicleType() const TX_NOEXCEPT;

  /**
   * @brief 查找周围行人
   * @param[in] timeMgr 时间管理器
   */
  virtual void SearchSurroundingPedestrian(const Base::TimeParamManager& timeMgr) TX_NOEXCEPT {}

  /**
   * @brief 检查行人是否在步行
   * @param relative_time 相对时间
   * @param refLateralVelocity 侧向速度
   * @return true 检测到行人在步行，false 未检测到行人在步行
   */
  virtual Base::txBool CheckPedestrianWalking(const txFloat relative_time,
                                              txFloat& refLateralVelocity) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  /**
   * @brief 停止车辆
   *
   * 此函数用于停止车辆的运动。在停止车辆后，车辆的速度应该为零，旋转方向应该是停止状态。
   * 如果车辆正在执行任何操作，如倒车或加速，则在停止车辆之前应该先取消这些操作。
   *
   * @param[in] relative_time 相对时间
   * @param[in] refLateralVelocity 后方速度，即沿x轴方向的速度
   * @return true 检测到行人在步行，false 未检测到行人在步行
   */
  virtual void StopVehicle() TX_NOEXCEPT TX_OVERRIDE;

 public:
  /**
   * @brief 获取二进制归档文件内容
   *
   * 获取此元素的二进制归档文件内容。
   * 二进制归档文件通常包含元素的设置信息和可能的数据。
   *
   * @return 包含此元素二进制归档文件内容的字符串
   */
  virtual txString binary_archive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 获取元素的 XML 归档内容
   *
   * 获取此元素的 XML 归档内容。
   * XML 归档通常包含元素的设置信息和可能的数据。
   *
   * @return 包含此元素 XML 归档内容的字符串
   */
  virtual txString xml_archive() const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将当前元素对象转换为 JSON 格式的字符串
   *
   * 该函数将当前元素对象的所有属性转换为 JSON 格式的字符串，以便于进行数据交换或存储。
   *
   * @return 包含当前元素所有属性的 JSON 格式字符串
   */
  virtual txString json_archive() const TX_NOEXCEPT TX_OVERRIDE;
  template <class Archive>
  void serialize(Archive& archive) {
    SerializeInfo("TAD_Cloud_AI_VehicleElement");
    archive(cereal::base_class<TrafficFlow::TAD_AI_VehicleElement>(this));
  }
  /*public:
          enum class MultiRegionStatus : txInt {eKernel_Archive = 0, eKernel_Simple = 1, eKernel_None=2, eEdge_Archive =
  3, eEdge_Simple = 4, eEdge_None = 5}; virtual void SetMultiRegionStatus(const MultiRegionStatus _status) TX_NOEXCEPT {
  mMultiRegionStatus = _status; } virtual Base::txBool NeedSimulationOnCloud() const TX_NOEXCEPT; protected:
          MultiRegionStatus mMultiRegionStatus = MultiRegionStatus::eKernel_Archive;*/

 public:
  /**
   * @brief 从内核中移动到边界
   *
   * 从内核中移动该元素到边界处，根据所处的位置执行不同的操作，例如让该元素从快速路线弹出时，弹出到较短路线上。
   */
  virtual void MoveFromKernel2Edge() TX_NOEXCEPT { Kill(); }

  /**
   * @brief 更新当前元素对应的车辆信息
   *
   * 更新当前元素对应的车辆信息，包括对象类型，位置和速度等。在更新完成后，对象应当返回 true，否则返回 false。
   *
   * @param _veh 要更新的车辆对象
   * @return 更新成功返回 true，否则返回 false
   */
  virtual Base::txBool UpdateByTxVehicle(const its::txVehicle& _veh) TX_NOEXCEPT;

  /**
   * @brief 使用传入的汽车数据更新当前元素对应的车辆信息
   *
   * 使用传入的汽车数据 _car 更新当前元素对应的车辆信息。在更新完成后，返回 true，否则返回 false。
   *
   * @param _car 要更新的汽车数据
   * @return 更新成功返回 true，否则返回 false
   */
  virtual Base::txBool UpdateByTxVehicle(const sim_msg::Car& _car) TX_NOEXCEPT;

  /**
   * @brief 将传入的归档字符串反序列化为对象
   *
   * 使用传入的归档字符串，将其反序列化为对象。在反序列化完成后，返回 true，反之返回 false。
   *
   * @param strArchive 待反序列化的归档字符串
   * @return 反序列化成功返回 true，反之返回 false
   */
  virtual Base::txBool Deserialization(const std::string& strArchive) TX_NOEXCEPT;

  /**
   * @brief 检查模拟后是否在有效范围内
   *
   * 在模拟后的情况下，检查当前位置是否在有效范围内。如果在，返回 true，否则返回 false。
   *
   * @return 位置在有效范围内返回 true，否则返回 false
   */
  virtual Base::txBool InRangeAfterSimulation() const TX_NOEXCEPT { return mInRangeAfterSimulation; }

 public:
  /**
   * @brief 更新后的后置处理
   *
   * @return Base::txBool 处理成功返回true
   */
  virtual Base::txBool Post_Update(TimeParamManager const&) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 向元素中填充数据
   * @param[in] timeStamp 当前时间戳
   * @param[in,out] pSimVehicle 待填充的模拟车辆
   * @return 填充后的模拟车辆指针
   */
  virtual sim_msg::Car* FillingElement(txFloat const timeStamp, sim_msg::Car* pSimVehicle) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 将数据填充到目标对象中
   * @param tm 时间参数管理器，用于获取当前时间
   * @param t 目标对象，即需要填充数据的traffic
   * @return 填充成功返回true，否则返回false
   */
  virtual txBool FillingElement(Base::TimeParamManager const& tm, sim_msg::Traffic& t) TX_NOEXCEPT TX_OVERRIDE;

 protected:
  Base::txBool mInRangeAfterSimulation = true;
};

using TAD_Cloud_AI_VehicleElementPtr = std::shared_ptr<TAD_Cloud_AI_VehicleElement>;

TX_NAMESPACE_CLOSE(TrafficFlow)

#undef SerializeInfo

CEREAL_REGISTER_TYPE(TrafficFlow::TAD_Cloud_AI_VehicleElement);

CEREAL_REGISTER_POLYMORPHIC_RELATION(TrafficFlow::TAD_AI_VehicleElement, TrafficFlow::TAD_Cloud_AI_VehicleElement);
