// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_math.h"
#include "tx_traj_manager_base.h"
TX_NAMESPACE_OPEN(TrafficFlow)

/**
 * @brief 使用线性插值计算值
 * @param t1 第一个时间点
 * @param t2 第二个时间点
 * @param v1 第一个时间点对应的值
 * @param v2 第二个时间点对应的值
 * @param t 需要计算值的时间点
 * @return Base::txFloat 返回插值后的值
 */
static Base::txFloat interp(const Base::txFloat t1, const Base::txFloat t2, const Base::txFloat v1,
                            const Base::txFloat v2, const Base::txFloat t) TX_NOEXCEPT;

/**
 * @brief 在给定时间和角度范围内进行线性插值计算角度
 * @param t1 时间点1
 * @param t2 时间点2
 * @param a1 角度点1
 * @param a2 角度点2
 * @param t 需要计算的时间点
 * @return Base::txFloat 返回插值后的角度值
 */
static Base::txFloat interp_angle(const Base::txFloat t1, const Base::txFloat t2, Base::txFloat a1, Base::txFloat a2,
                                  const Base::txFloat t) TX_NOEXCEPT;

/**
 * @brief 根据两个时间点和两个向量，进行线性插值计算
 * @param t1 第一个时间点
 * @param t2 第二个时间点
 * @param v1 第一个时间点对应的向量
 * @param v2 第二个时间点对应的向量
 * @param t 需要计算的时间点
 * @return sim_msg::Vec3 返回插值后的向量
 */
static sim_msg::Vec3 interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Vec3& v1,
                            const sim_msg::Vec3& v2, const Base::txFloat t) TX_NOEXCEPT;

/**
 * @brief 在给定时间段内，根据两个位置，进行线性插值计算
 * @param t1 起始时间
 * @param t2 结束时间
 * @param loc1 起始时间对应的位置
 * @param loc2 结束时间对应的位置
 * @param t 需要计算的时间
 * @return sim_msg::Location 返回插值后的位置
 */
static sim_msg::Location interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Location& loc1,
                                const sim_msg::Location& loc2, const Base::txFloat t) TX_NOEXCEPT;

/**
 * @brief 对两个car对象在给定时间段内进行线性插值计算
 * @param t1 起始时间
 * @param t2 结束时间
 * @param car1 起始时间对应的car对象
 * @param car2 结束时间对应的car对象
 * @param t 需要计算的时间
 * @return sim_msg::Car 返回插值后的car对象
 */
static sim_msg::Car interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Car& car1,
                           const sim_msg::Car& car2, const Base::txFloat t) TX_NOEXCEPT;

struct txL2W_Car_FrameAware_TrajMgr : public txTrajManagerBase {
 public:
  using ParentClass = txTrajManagerBase;
  struct tx_L2W_SpaceTimeCar : public txSpaceTimeCar {
    tx_L2W_SpaceTimeCar() TX_DEFAULT;
    tx_L2W_SpaceTimeCar(const Base::txFloat _absTime, const sim_msg::Car& refCar)
        : txSpaceTimeCar(_absTime, refCar), bSend(false) {}

    /**
     * @brief tx_L2W_SpaceTimeCar 构造函数
     * @param refParent 参考的 SpaceTimeCar 对象
     * @param bSend 是否发送此车辆信息的标志
     */
    explicit tx_L2W_SpaceTimeCar(const txSpaceTimeCar& refParent) : txSpaceTimeCar(refParent), bSend(false) {}

    Base::txBool bSend = false;

    /**
     * @brief IsSend 获取是否发送此车辆信息的标志
     * @return 返回是否发送此车辆信息的标志
     */
    Base::txBool IsSend() const TX_NOEXCEPT { return bSend; }

    /**
     * @brief 检查当前顶点是否未发送
     * @return 如果当前顶点已发送，则返回false；否则返回true
     */
    Base::txBool IsNotSend() const TX_NOEXCEPT { return !bSend; }

    /**
     * @brief 设置当前顶点为已发送
     * @details 将当前顶点的 bSend 标记设置为 true
     * @note 不抛出任何异常
     */
    void SetBeSend() TX_NOEXCEPT { bSend = true; }

    /**
     * @brief 获取原始车辆数据
     *
     * 获取最近一次接收到的原始车辆数据。
     *
     * @return const sim_msg::Car& 返回原始车辆数据的 const 引用
     */
    const sim_msg::Car& RawCar() const TX_NOEXCEPT { return raw_car_pb; }

    /**
     * @brief 获取预留时间
     *
     * @return double 返回预留时间
     */
    Base::txFloat RsvTime() const TX_NOEXCEPT { return rsv_abs_time; }
  };

 public:
  /*override function*/

  /**
   * @brief 获取类名
   * @return const char* 返回类名的字符串指针
   */
  virtual Base::txLpsz className() const TX_NOEXCEPT TX_OVERRIDE { return "txL2W_Car_FrameAware_TrajMgr"; }

  /**
   * @brief 检查是否可以启用填充
   *
   * 当需要启用填充时，返回 true，否则返回 false。可以在这个函数中自定义填充条件。
   *
   * @param timeMgr 时间参数管理器，包含了当前时间、当前速度等信息
   * @return true 可以启用填充，false 不能启用填充
   */
  virtual Base::txBool EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 判断是否需要结束
   *
   * @param timeMgr 时间参数管理器，包含了当前时间、当前速度等信息
   * @return true 需要完成当前路线，false 不需要完成当前路线
   */
  virtual Base::txBool NeedFinish(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 向车辆管理器添加新的路径
   *
   * @param l2w_car_list 需要添加的车辆列表，包含车辆信息、车辆编号、所在路段、车辆当前时间等
   * @return true 添加成功，false 添加失败
   */
  virtual Base::txBool add_new_pos(const txSpaceTimeCarList& l2w_car_list) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间参数管理器，计算车辆的位置、速度和加速度
   * @param timeMgr 时间参数管理器，包含车辆当前时间等信息
   * @param refCarInfo 车辆信息，用于返回车辆当前的位置、速度和角度等信息
   * @param refVelocity 速度，用于返回车辆当前的速度
   * @param refAcc 加速度，用于返回车辆当前的加速度
   * @return true 计算成功，false 计算失败
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Car& refCarInfo,
                                   Base::txFloat& refVelocity, Base::txFloat& refAcc) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 更新Kinect检测数据
   * @param leftIdx 左侧的索引
   * @param rightIdx 右侧的索引
   * @param refCarInfo 车辆信息，用于获取车辆当前的位置、速度和方向等信息
   * @param _carTimeStamp 车辆时间戳
   * @param refKinectInfo 参考的Kinect检测数据
   * @return true 更新成功，false 更新失败
   */
  virtual Base::txBool update_kinect(const Base::txInt leftIdx, const Base::txInt rightIdx,
                                     const sim_msg::Car& refCarInfo, const Base::txFloat _carTimeStamp,
                                     txKinectInfo& refKinectInfo) const TX_NOEXCEPT;

  /**
   * @brief 清除所有轨迹数据
   *
   */
  virtual void Clear() TX_NOEXCEPT TX_OVERRIDE;

  /*disable function*/

  /**
   * @brief 计算车辆的WGS84坐标
   *
   * @param timeParamManager 时间参数管理器，包含时间、速度和精度等信息
   * @param wgs84Position 输出的车辆WGS84坐标
   * @param car 车辆信息，包括车辆的ID、位置、速度等信息
   * @return 返回计算结果是否成功
   * @retval true 计算成功
   * @retval false 计算失败
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const&, Coord::txWGS84&,
                                   sim_msg::Car&) TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }

  /**
   * @brief 将新的位置添加到指定记录数组中
   *
   * @param history_pos_info 待添加的新历史位置记录
   * @param position_offset 新历史位置记录所需偏移量
   * @return 返回添加新位置是否成功
   * @retval true 添加成功
   * @retval false 添加失败
   */
  virtual Base::txBool add_new_pos(history_pos_info, const Base::txVec3) TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 复制当前的滚动位置管理器，并创建一个新的对象
   *
   * @return 返回一个新的滚动位置管理器指针，可用于管理新的滚动位置信息
   */
  virtual txTrajManagerBasePtr clone_traj() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  std::vector<tx_L2W_SpaceTimeCar> mSpaceTimeCarList;
  txSpaceTimeCarList mSrcL2wCarList;
};

using txL2W_Car_FrameAware_TrajMgrPtr = std::shared_ptr<txL2W_Car_FrameAware_TrajMgr>;

struct txL2W_Car_SmoothAware_TrajMgr : public txL2W_Car_FrameAware_TrajMgr {
 public:
  /**
   * @brief 返回当前对象的类名称
   *
   * 此函数返回当前对象的类名称，该名称唯一标识了该对象。该函数重写了父类的相应方法。
   * @return const txL2W_Car_SmoothAware_TrajMgr::txLpsz& 当前对象的类名称
   */
  virtual Base::txLpsz className() const TX_NOEXCEPT TX_OVERRIDE { return "txL2W_Car_SmoothAware_TrajMgr"; }

  /**
   * @brief 获取是否启用满足填充的条件
   *
   * 当时间管理器被指定为 "timeMgr" 时，如果启用了满足填充的条件，则返回 txBool 值 "true"，否则返回 "false"。
   *
   * @param timeMgr 当前时间管理器，用于获取时间信息
   * @return txBool 是否启用满足填充的条件
   */
  virtual Base::txBool EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间管理器和车辆信息，计算车辆的位置、速度和加速度
   *
   * 根据传入的时间管理器、车辆信息，计算车辆在给定时间内的位置、速度和加速度。函数返回 true 表示计算成功，false
   * 表示计算失败。
   *
   * @param timeMgr 当前时间管理器，用于获取时间信息
   * @param refCarInfo 当前车辆的信息，用于输出计算结果
   * @param refVelocity 当前车辆的速度，用于输出计算结果
   * @param refAcc 当前车辆的加速度，用于输出计算结果
   * @return txBool 计算成功返回 true，计算失败返回 false
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Car& refCarInfo,
                                   Base::txFloat& refVelocity, Base::txFloat& refAcc) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 复制当前的动态轨迹管理器，返回一个新的轨迹管理器。
   *
   * 此函数将复制当前的动态轨迹管理器，并返回一个新的轨迹管理器。
   * 新的轨迹管理器与原始轨迹管理器具有相同的状态，但是它们是独立的对象。
   *
   * @return txTrajManagerBasePtr 返回一个新的轨迹管理器的指针。
   */
  virtual txTrajManagerBasePtr clone_traj() TX_NOEXCEPT TX_OVERRIDE;
};

using txL2W_Car_SmoothAware_TrajMgrPtr = std::shared_ptr<txL2W_Car_SmoothAware_TrajMgr>;

struct txL2W_Location_FrameAware_TrajMgr : public txTrajManagerBase {
 public:
  using ParentClass = txTrajManagerBase;
  struct tx_L2W_SpaceTimeLocation : public txSpaceTimeLocation {
    tx_L2W_SpaceTimeLocation() TX_DEFAULT;
    tx_L2W_SpaceTimeLocation(const Base::txFloat _absTime, const sim_msg::Location& refLocation)
        : txSpaceTimeLocation(_absTime, refLocation), bSend(false) {}

    /**
     * @brief tx_L2W_SpaceTimeLocation 构造函数
     *
     * 构造函数用于创建一个新的空间时间定位对象，并将其关联到父节点 refParent 上。
     *
     * @param refParent txSpaceTimeLocation 对象的引用，表示父节点的位置信息。
     */
    explicit tx_L2W_SpaceTimeLocation(const txSpaceTimeLocation& refParent)
        : txSpaceTimeLocation(refParent), bSend(false) {}

    Base::txBool bSend = false;

    /**
     * @brief Get the Is Send Flag object
     *
     * 获取发送标志是否为真。
     *
     * @return true 如果已准备好发送，否则返回 false
     */
    Base::txBool IsSend() const TX_NOEXCEPT { return bSend; }

    /**
     * @brief Checks if the message has not been sent
     *
     * 检查消息是否未发送。
     *
     * @return true 如果消息未发送，否则返回 true
     */
    Base::txBool IsNotSend() const TX_NOEXCEPT { return !bSend; }

    /**
     * @brief 设置已经发送的消息标记
     *
     * 该函数用于设置标记，表示此消息已经发送。
     *
     * @param nothing 无参数
     * @return nothing 无返回值
     */
    void SetBeSend() TX_NOEXCEPT { bSend = true; }

    /**
     * @brief 获取原始位置的PB消息
     *
     * 本函数用于获取原始位置的PB消息对象，用于向外部传递和通信。
     *
     * @return const sim_msg::Location& 原始位置的PB消息对象的引用
     */
    const sim_msg::Location& RawLocation() const TX_NOEXCEPT { return raw_location_pb; }

    /**
     * @brief 获取预留时间
     *
     * @return const Base::txFloat& 预留时间
     */
    Base::txFloat RsvTime() const TX_NOEXCEPT { return rsv_abs_time; }
  };

 public:
  /*override function*/

  /**
   * @brief 获取类名
   * @return 类名
   */
  virtual Base::txLpsz className() const TX_NOEXCEPT TX_OVERRIDE { return "txL2W_Location_FrameAware_TrajMgr"; }

  /**
   * @brief 检查是否可以启用填充功能
   *
   * @param timeMgr 时间管理器
   * @return 如果启用填充功能，则返回true，否则返回false
   */
  virtual Base::txBool EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 向系统添加新的位置
   *
   * @param l2w_ego_list 一个包含欧拉时间空间位置列表的列表，这些位置对应于ego agent的当前位置
   * @return true 成功添加新位置
   * @return false 添加新位置失败
   */
  virtual Base::txBool add_new_pos(const txSpaceTimeLocationList& l2w_ego_list) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 使用时间管理器计算模型的位置信息
   *
   * @param timeMgr 时间管理器
   * @param refEgoInfo 返回当前欧拉时间空间的位置信息
   * @return true 位置信息计算成功
   * @return false 位置信息计算失败
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr,
                                   sim_msg::Location& refEgoInfo) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 清除所有轨迹数据
   *
   * @return void
   */
  virtual void Clear() TX_NOEXCEPT TX_OVERRIDE;

  /*disable function*/

  /**
   * @brief 根据给定的参数计算车辆当前的位置信息
   *
   * @param timeMgr 当前的时间参数管理器
   * @param wgs84Coord 输出的经纬度坐标，存储计算结果
   * @param carInfo 存储计算结果的车辆信息
   * @return true 计算成功
   * @return false 计算失败
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const&, Coord::txWGS84&,
                                   sim_msg::Car&) TX_NOEXCEPT TX_OVERRIDE {
    return false;
  }

  /**
   * @brief 将新的车辆位置信息添加到数据结构中
   *
   * @param history_pos_info 待添加的车辆位置信息
   * @param new_pos 新的车辆位置坐标
   * @return true 添加成功
   * @return false 添加失败
   */
  virtual Base::txBool add_new_pos(history_pos_info, const Base::txVec3) TX_NOEXCEPT TX_OVERRIDE { return false; }

  /**
   * @brief 复制当前的轨迹管理器，并创建一个新的轨迹管理器实例
   *
   * @return 返回一个新的轨迹管理器实例的智能指针
   */
  virtual txTrajManagerBasePtr clone_traj() TX_NOEXCEPT TX_OVERRIDE;

 protected:
  std::vector<tx_L2W_SpaceTimeLocation> mSpaceTimeLocationList;
  txSpaceTimeLocationList mSrcL2wEgoList;
};

using txL2W_Location_FrameAware_TrajMgrPtr = std::shared_ptr<txL2W_Location_FrameAware_TrajMgr>;

struct txL2W_Location_SmoothAware_TrajMgr : public txL2W_Location_FrameAware_TrajMgr {
 public:
  /**
   * @brief 返回当前轨迹管理器的类名字符串
   *
   * @return 返回一个表示类名的字符串
   */
  virtual Base::txLpsz className() const TX_NOEXCEPT TX_OVERRIDE { return "txL2W_Location_SmoothAware_TrajMgr"; }

  /**
   * @brief 检查是否启用轨迹填充功能
   *
   * @param timeMgr 当前的时间参数管理器
   * @return bool 如果启用轨迹填充功能，返回 true，否则返回 false
   */
  virtual Base::txBool EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 根据时间参数管理器计算轨迹的位置信息
   *
   * @param timeMgr 时间参数管理器
   * @param refLocationInfo 用于存储计算得到的轨迹位置信息
   * @return bool 如果计算成功，则返回 true；否则返回 false
   */
  virtual Base::txBool compute_pos(Base::TimeParamManager const& timeMgr,
                                   sim_msg::Location& refLocationInfo) TX_NOEXCEPT TX_OVERRIDE;

  /**
   * @brief 复制当前的轨迹管理器，并创建一个新的轨迹管理器实例
   *
   * @return txTrajManagerBasePtr 返回新创建的轨迹管理器实例
   */
  virtual txTrajManagerBasePtr clone_traj() TX_NOEXCEPT TX_OVERRIDE;
};

using txL2W_Location_SmoothAware_TrajMgrPtr = std::shared_ptr<txL2W_Location_SmoothAware_TrajMgr>;

TX_NAMESPACE_CLOSE(TrafficFlow)
