// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "its_sim.pb.h"
#include "tx_map_info.h"
#include "tx_tc_marco.h"
#include "tx_tc_tad_element_generator.h"
TX_NAMESPACE_OPEN(Scene)
#if __pingsn__
class ParallelSimulation_ElementGenerator : public TAD_Cloud_ElementGenerator {
 public:
#  if __pingsn__
  struct VehicleSeparateInputAgent {
   public:
    struct VehicleInfo_t {
      Base::txSysId vehId64 = 1;
      Base::txInt vehType = 1;
      int64_t startTime = 0.0;
      Base::txLaneID vehicleStartLaneId = -1;
      std::vector<int64_t> roadIds = {};

      VehicleInputeAgent::VehicleInitParam_t vehicleInitParam;
    };

   public:
    VehicleSeparateInputAgent() TX_DEFAULT;
    ~VehicleSeparateInputAgent() TX_DEFAULT;

    /**
     * @brief 并行仿真元素生成器对象初始化
     *
     * @param info vehicle的pb对象，用于初始化
     * @param _loader 场景加载器
     * @return Base::txBool 初始化成功返回true
     */
    Base::txBool Init(its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT;

    /**
     * @brief 判断当前对象是否有效
     *
     * @return 如果当前对象有效，则返回 true，否则返回 false
     */
    Base::txBool IsValid() const TX_NOEXCEPT { return mValid; }

    /**
     * @brief 判断当前车辆是否已经到达下一辆车的预定时间
     *
     * @param timeStamp 当前时间戳（以秒为单位）
     * @return 如果当前车辆已经到达下一辆车的预定时间，则返回 true，否则返回 false
     */
    Base::txBool HasArriveNextVehicleTime(const int64_t timeStamp) TX_NOEXCEPT;

    /**
     * @brief 获取当前车辆信息
     * @param timeStamp 当前时间戳（以秒为单位）
     * @return 返回当前车辆信息，包括车牌号、车型、车速、车辆类型等
     */
    VehicleInfo_t GetCurVehicleInfo(const int64_t timeStamp) TX_NOEXCEPT;

    /**
     * @brief 判断车辆列表是否为空
     *
     * @return true 当前车辆列表为空
     * @return false 当前车辆列表不为空
     */
    Base::txBool Empty() const TX_NOEXCEPT { return mVehicleList.empty(); }

    /**
     * @brief 获取字符串对象
     *
     * @return 字符串对象
     */
    Base::txString Str() const TX_NOEXCEPT;

    /**
     * @brief 根据给定的初始参数生成一个车辆信息对象
     *
     * @param vehId 车辆ID
     * @param vehType 车辆类型
     * @param startTime 开始时间
     * @param vehicleStartLaneId 车辆所在的车道ID
     * @param roadIds 车辆所在的道路ID列表
     * @param _loader 地景素材加载器
     * @param startV 车辆初始速度
     * @param roadStartPointX 道路起点X坐标
     * @param roadStartPointY 道路起点Y坐标
     * @param roadStartPointZ 道路起点Z坐标
     * @return 返回生成的车辆信息对象
     */
    VehicleInfo_t generateVehicleInfo(Base::txSysId vehId, Base::txInt vehType, int64_t startTime,
                                      Base::txLaneID vehicleStartLaneId, const std::vector<int64_t>& roadIds,
                                      Base::ISceneLoaderPtr _loader, Base::txFloat startV,
                                      Base::txFloat roadStartPointX, Base::txFloat roadStartPointY,
                                      Base::txFloat roadStartPointZ) TX_NOEXCEPT;

    /**
     * @brief 根据给定的场景信息和地景素材加载器，从Pb中获取车辆信息列表
     *
     * @param info 一个包含场景车辆信息的对象引用
     * @param _loader 地景素材加载器，用于加载车辆的模型等相关信息
     * @return 一个包含车辆信息列表的vector
     */
    std::vector<VehicleInfo_t> getVehicleListFromPb(its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT;

   protected:
    Base::txInt mCurIndex = 0;
    int64_t mVehicleBornTime = 0.0;
    Base::txBool mValid = true;
    std::vector<VehicleInfo_t> mVehicleList = {};
  };

#  endif /*__pingsn__*/

 public:
  /**
   * @brief 初始化模块
   * @param _loader 用于加载场景数据的地景素材加载器
   * @param valid_map_range 场景的地图范围
   * @return 返回初始化成功或失败
   */
  virtual Base::txBool Initialize(Base::ISceneLoaderPtr _loader,
                                  const Base::map_range_t& valid_map_range) TX_NOEXCEPT TX_OVERRIDE;
  /*virtual Base::txBool Init(const hadmap::txPoint & globalPos) TX_NOEXCEPT;*/

  /**
   * @brief 使用地景素材加载器生成游戏元素
   * @param info 存储汽车相关信息的容器
   * @param _loader 地景素材加载器
   * @return 生成成功或失败
   */
  virtual Base::txBool Generate(its::txVehicles& info, Base::ISceneLoaderPtr _loader) TX_NOEXCEPT;

 protected:
  VehicleSeparateInputAgent mVehicleSeparateInput;
};

using ParallelSimulation_ElementGeneratorPtr = std::shared_ptr<ParallelSimulation_ElementGenerator>;
#endif /*__pingsn__*/

TX_NAMESPACE_CLOSE(Scene)
