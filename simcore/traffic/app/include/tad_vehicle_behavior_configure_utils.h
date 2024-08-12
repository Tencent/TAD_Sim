// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "nlohmann/json.hpp"
#include "tx_enum_def.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Utils)

enum class ConfigType : Base::txInt { file_path = 0, json_content };

// @brief vehicle行为配置
class VehicleBehaviorCfg {
 public:
  using txFloat = Base::txFloat;
  using txBool = Base::txBool;
  using txSize = Base::txSize;
  static txFloat LaneKeep() TX_NOEXCEPT;

  /**
   * @brief 获取最大速度参数
   *
   * @return txFloat
   */
  static txFloat MAX_SPEED() TX_NOEXCEPT;

  /**
   * @brief 获取changetopology阈值
   *
   * @return txFloat
   */
  static txFloat ChangeTopologyThreshold() TX_NOEXCEPT;

  /**
   * @brief NoCheckLCAbortThreshold 获取车辆行驶中车道障碍遇到判断禁止行驶紧急制动状态的阈值
   *
   * 车道障碍在判断禁止行驶状态时，无需验证LC是否存在，直接使用此阈值进行判断禁止行驶紧急制动
   *
   * @param   无
   * @return NoCheckLCAbortThreshold 返回车道障碍判断禁止行驶紧急制动阈值
   */
  static txFloat NoCheckLCAbortThreshold() TX_NOEXCEPT;

  /**
   * @brief GetTargetLaneGapThreshold 获取目标车道拥堵阈值
   *
   * 对于一般道路，该函数可用于获取目标车道拥堵阈值。在推荐道路升级准备阶段，推荐拥堵阈值为 5。
   *
   * @param Void 无需输入参数
   * @return txFloat 目标车道拥堵阈值
   */
  static txFloat EmptyTargetLaneGapThreshold() TX_NOEXCEPT;

  /**
   * @brief TxMotionTracking算法中的目标车道速度分母偏移量
   *
   * 此函数用于获取TxMotionTracking算法中的目标车道速度分母偏移量。返回值决定了目标车道速度分母如何计算，并影响到速度估计的准确性。
   *
   * @param Void 无需输入参数
   * @return txFloat 目标车道速度分母偏移量
   */
  static txFloat SpeedDenominatorOffset() TX_NOEXCEPT;

  /**
   * @brief 根据索引获取driving参数
   *
   * @param idx 索引值
   * @return txFloat
   */
  static txFloat g_Driving_Parameters(const Base::txInt idx) TX_NOEXCEPT;

  /**
   * @brief 获取后方安全间距
   *
   * @return txFloat
   */
  static txFloat SafetyGap_Rear() TX_NOEXCEPT;

  /**
   * @brief 获取前方安全间距
   *
   * @return txFloat
   */
  static txFloat SafetyGap_Front() TX_NOEXCEPT;

  /**
   * @brief 获取侧方安全间距
   *
   * @return txFloat
   */
  static txFloat SafetyGap_Side() TX_NOEXCEPT;

  /**
   * @brief TAD_VehicleBehaviorConfigureUtils::ComfortGap
   *       获取舒适间隔值
   *
   * 该函数用于获取车辆在当前驾驶模式下舒适的动作间隔。在一些高速或不同驾驶模式下，车辆的行驶速度和操作频率会有差异，需要采用不同的间隔值来保证车辆安全和舒适。
   *
   * @param void 无需输入参数
   * @return txFloat 舒适间隔值
   */
  static txFloat ComfortGap() TX_NOEXCEPT;

  /**
   * @brief 获取最大准备反应距离
   *
   * 该函数用于获取汽车在最大准备反应状态下，在其各轴上的距离。
   * 最大准备反应状态是指在汽车行驶时，当汽车行驶到达某个速度后，汽车停留的时间。
   * 该函数可以帮助开发者确定汽车在最大准备反应状态下，在每个轴上的最大距离。
   *
   * @param void 无需输入参数
   * @return txFloat 最大准备反应距离，单位：米
   */
  static txFloat Max_Ped_Reaction_Distance() TX_NOEXCEPT;

  /**
   * @brief 获取最大反应间隔
   *
   * 该函数用于获取汽车在最大反应状态下，在其各轴上的距离。
   * 最大反应状态是指在汽车行驶时，当汽车行驶到达某个速度后，汽车停留的时间。
   * 该函数可以帮助开发者确定汽车在最大反应状态下，在每个轴上的最大距离。
   *
   * @param void 无需输入参数
   * @return txFloat 最大反应间隔，单位：米
   */
  static txFloat MAX_REACTION_GAP() TX_NOEXCEPT;

  /**
   * @brief 获取激进改变车道的阈值
   *
   * 该函数用于获取汽车在激进改变车道状态下，在其各轴上的距离。
   * 激进改变车道状态是指在汽车行驶时，当汽车在靠近另一车辆的车道时，开始尝试改变车道。
   * 该函数可以帮助开发者确定汽车在激进改变车道状态下，在每个轴上的最大距离。
   *
   * @param void 无需输入参数
   * @return txFloat 激进改变车道的阈值，单位：米
   */
  static txFloat LaneChanging_Aggressive_Threshold() TX_NOEXCEPT;

  /**
   * @brief 获取容差阈值
   *
   * 该函数用于获取汽车在容差状态下，在其各轴上的距离。
   * 容差状态是指在汽车行驶时，当汽车的位置和速度变化很小时，仍然能保持在道路上。
   * 该函数可以帮助开发者确定汽车在容差状态下，在每个轴上的最大距离。
   *
   * @param void 无需输入参数
   * @return txFloat 容差阈值，单位：米
   */
  static txFloat Tolerance_Threshold() TX_NOEXCEPT;

  /**
   * @brief 获取最大减速度
   *
   * 该函数用于获取汽车在最大减速状态下，在其各轴上的最大减速度。
   * 最大减速状态是指在汽车行驶时，当汽车需要进行最大减速时，汽车能够达到的最大减速度。
   * 该函数可以帮助开发者确定汽车在最大减速状态下，在每个轴上的最大减速度。
   *
   * @param void 无需输入参数
   * @return txFloat 最大减速度，单位：米/秒²
   */
  static txFloat Max_Deceleration() TX_NOEXCEPT;

  /**
   * @brief 获取信号反应距离
   *
   * 该函数用于获取汽车在信号反应状态下，在其各轴上的信号反应距离。
   * 信号反应状态是指在汽车行驶时，当汽车行驶时遇到信号时，车辆与信号之间的间距。
   * 该函数可以帮助开发者确定汽车在信号反应状态下，在每个轴上的最大信号反应距离。
   *
   * @param void 无需输入参数
   * @return txFloat 信号反应距离，单位：米
   */
  static txFloat Signal_Reaction_Gap() TX_NOEXCEPT;

  /**
   * @brief 获取最大的舒适性减速度
   *
   * @return txFloat
   */
  static txFloat Max_Comfort_Deceleration() TX_NOEXCEPT;

  /**
   * @brief 获取激进响应阈值
   *
   * 该函数用于获取汽车在激进响应状态下，在其各轴上的激进响应阈值。
   * 激进响应状态是指在汽车行驶时，当汽车需要执行激进操作时，发生在汽车轴上的最小弹性距离。
   * 该函数可以帮助开发者确定汽车在激进响应状态下，在每个轴上的最小弹性距离。
   *
   * @param void 无需输入参数
   * @return txFloat 激进响应阈值，单位：米
   */
  static txFloat Yielding_Aggressive_Threshold() TX_NOEXCEPT;

  /**
   * @brief 获取滑动因子
   *
   * 该函数用于获取汽车在滑动状态下，在其各轴上的滑动因子。
   * 滑动状态是指在汽车行驶时，当汽车行驶时，汽车在相应轴上的速度在减小。
   * 该函数可以帮助开发者确定汽车在滑动状态下，在每个轴上的滑动因子。
   *
   * @param void 无需输入参数
   * @return txFloat 滑动因子，单位：米/秒²
   */
  static txFloat Friction_Factor() TX_NOEXCEPT;

  /**
   * @brief 侧前方的视野距离
   *
   * @return txFloat
   */
  static txFloat SideFrontVisionDistance() TX_NOEXCEPT;

  /**
   * @brief
   *
   * @return txFloat
   */
  static txFloat SideRearVisionDistance() TX_NOEXCEPT;

  /**
   * @brief 获取最小速度阈值
   *
   * 该函数用于获取汽车在行驶时的最小速度阈值。当汽车的行驶速度在减小到此阈值时，汽车将进入刹车状态。
   * 该函数可以帮助开发者确定汽车在行驶状态下的最小速度阈值。
   *
   * @param void 无需输入参数
   * @return txFloat 最小速度阈值，单位：米/秒
   */
  static txFloat SideGapMinVelocity() TX_NOEXCEPT;

  /**
   * @brief IDM模型长度限制
   *
   * 根据道路行驶环境和车辆类型，计算IDM模型下的行驶最大长度，即控制速度的较大参数。
   *
   * @return 返回IDM模型行驶长度限制
   */
  static txFloat IDM_Vision_Length() TX_NOEXCEPT;

  /**
   * @brief 车辆视野宽度
   *
   * 根据车辆类型和道路情况，计算IDM模型下的车辆视野宽度，即控制行驶方向的小参数。
   *
   * @return 返回IDM模型下的车辆视野宽度
   */
  static txFloat IDM_Vision_Width() TX_NOEXCEPT;

  /**
   * @brief 获取L2W链接状态的切换时间
   *
   * 该函数用于获取L2W链接状态的切换时间，即在L2W链接状态转换为非L2W状态或者非L2W状态转换为L2W状态的时间。
   * 该函数可以帮助开发者确定L2W链接状态的切换时间。
   *
   * @param void 无需输入参数
   * @return txFloat 切换时间，单位：秒
   */
  static txFloat L2W_Switch_Time() TX_NOEXCEPT;

  /**
   * @brief 获取L2W切换结束时间
   *
   * 该函数用于获取L2W切换结束时间，即在L2W状态下，这辆车把W链接置换为L链接的时间。
   * 该函数可以帮助开发者确定L2W切换结束时间。
   *
   * @param void 无需输入参数
   * @return txFloat 切换结束时间，单位：秒
   */
  static txFloat L2W_Switch_EndTime() TX_NOEXCEPT;

  /**
   * @brief 设置L2W切换时间
   *
   * 此函数用于设置L2W切换时间，即从L链接切换到W链接的时间。
   * 在实际应用中，开发者可以根据需要调整此参数。
   *
   * @param _time 要设置的L2W切换时间，单位：秒
   */
  static void Set_L2W_Switch_Time(Base::txFloat _time) TX_NOEXCEPT;

  /**
   * @brief 设置L2W切换结束时间
   *
   * 此函数用于设置L2W切换结束时间，即从L链接切换到W链接的结束时间。
   * 在实际应用中，开发者可以根据需要调整此参数。
   *
   * @param _time 要设置的L2W切换结束时间，单位：秒
   */
  static void Set_L2W_Switch_EndTime(Base::txFloat _time) TX_NOEXCEPT;

  /**
   * @brief L2W速度比率
   *
   * 计算车辆在坐标系中的旋转速度与世界坐标系中的线性速度之间的比率，用于调整输出值的大小。
   *
   * @return 返回L2W速度比率
   */
  static txFloat L2W_Speed_Ratio() TX_NOEXCEPT;

  /**
   * @brief 速度比率上限
   *
   * 计算车辆在世界坐标系中的旋转速度与局部坐标系中的线性速度之间的比率上限，防止超出可接受范围。
   *
   * @return 返回速度比率上限
   */
  static txFloat L2W_Speed_Ratio_Upper() TX_NOEXCEPT;

  /**
   * @brief L2W的时间步长
   *
   * @return txFloat
   */
  static txFloat L2W_Log2worldsim_Timestep() TX_NOEXCEPT;

  /**
   * @brief L2W的持续时长
   *
   * @return txFloat
   */
  static txFloat L2W_Log2worldsim_Duration() TX_NOEXCEPT;

  /**
   * @brief 场景最大vehicle数量
   *
   * @return Base::txInt
   */
  static Base::txInt SceneMaxVehicleSize() TX_NOEXCEPT;

  /**
   * @brief 检查是否可以发送交通流量统计数据
   *
   * 该函数用于检查交通流量统计数据是否可以被发送。
   * 若为真，表示可以发送； 若为假，表示不可以发送。
   *
   * @return txBool 返回交通流量统计数据发送状态，为真表示可以发送，为假表示不可发送。
   */
  static txBool L2W_SendTraficTopic() TX_NOEXCEPT;

  /**
   * @brief 获取车道保持A参数
   *
   * 获取车道保持A的参数，根据车辆特点和使用场景可自定义此参数。
   *
   * @return txFloat 返回车道保持A的参数
   */
  static txFloat LaneKeepA() TX_NOEXCEPT;

  /**
   * @brief 获取车道保持B参数
   *
   * 根据车辆特点和使用场景，此函数用于获取车道保持B的参数。
   *
   * @return txFloat 返回车道保持B的参数
   */
  static txFloat LaneKeepB() TX_NOEXCEPT;

  /**
   * @brief 获取车辆的默认a氧系数
   *
   * 根据车辆的特性和使用场景，此函数用于获取车辆的默认a氧系数。
   *
   * @return txFloat 返回车辆的默认a氧系数
   */
  static txFloat W74_DEFAULT_ax() TX_NOEXCEPT;

  /**
   * @brief 获取车辆的默认b氧系数
   *
   * 根据车辆的特性和使用场景，此函数用于获取车辆的默认b氧系数。
   *
   * @return txFloat 返回车辆的默认b氧系数
   */
  static txFloat W74_DEFAULT_bx_add() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_bx_mult() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_cx() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_ex_add() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_ex_mult() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_opdv_mult() TX_NOEXCEPT;
  static txFloat W74_DEFAULT_bnull_mult() TX_NOEXCEPT;
  static txBool W74_DEFAULT_v2cav() TX_NOEXCEPT;

  static Base::txString Str() TX_NOEXCEPT;

 protected:
  /**
   * @brief 初始化json路径
   *
   * @param cfg_path
   * @return Base::txBool
   */
  static Base::txBool Initialize_json_path(const Base::txString cfg_path) TX_NOEXCEPT;

  /**
   * @brief 初始化json内容
   *
   * @param cfg_content
   * @return Base::txBool
   */
  static Base::txBool Initialize_json_content(const Base::txString cfg_content) TX_NOEXCEPT;

  /**
   * @brief 初始化配置
   * @param cfg_value 配置内容
   * @param cfg_type 配置类型
   * @return 初始化成功返回true，失败返回false
   */
  static Base::txBool Initialize(const Base::txString cfg_value, ConfigType cfg_type) TX_NOEXCEPT;

  /**
   * @brief 根据给定的 JSON 对象初始化车辆行为配置
   * @param jonObj 车辆行为配置的 JSON 对象
   * @return 初始化成功返回 true，失败返回 false
   */
  static Base::txBool Initialize_json_object(const nlohmann::json &jonObj) TX_NOEXCEPT;

  /**
   * @brief 设置可视化器参数
   * @param CC0 可视化器参数1
   * @param CC1 可视化器参数2
   * @param CC2 可视化器参数3
   * @return 设置成功返回true，否则返回false
   */
  static Base::txBool SetVisualizerParams(const Base::txFloat CC0, const Base::txFloat CC1,
                                          const Base::txFloat CC2) TX_NOEXCEPT;
};

class BehaviorCfgDB {
 public:
  /**
   * @brief 检查配置文件版本是否支持
   * @param _strVer 配置文件版本字符串
   * @return 如果支持返回true，否则返回false
   */
  static Base::txBool IsSupportCfgVersion(const Base::txString &_strVer) TX_NOEXCEPT {
    return Base::txString("20220718") == _strVer;
  }

  /**
   * @brief 初始化车辆行为配置信息
   *
   * 该函数用于初始化车辆的行为配置信息，接收一个包含车辆行为配置信息的字符串作为输入，并返回初始化是否成功的布尔值。
   *
   * @param cfg_value 包含车辆行为配置信息的字符串
   * @param cfg_type 车辆行为配置类型
   * @return 返回初始化是否成功的布尔值
   */
  static Base::txBool Initialize(const Base::txString cfg_value, ConfigType cfg_type) TX_NOEXCEPT;

  // chomper
  /**
   * @brief 初始化车辆行为配置信息
   *
   * 该函数用于初始化车辆的行为配置信息，接收一个包含车辆行为配置信息的json对象作为输入，并返回初始化是否成功的布尔值。
   *
   * @param cfg_value 包含车辆行为配置信息的json对象
   * @param cfg_type 车辆行为配置类型
   * @param strCfgId 配置标识符
   * @param cc0 指定的参数1
   * @param cc1 指定的参数2
   * @param cc2 指定的参数3
   * @return 返回初始化是否成功的布尔值
   */
  static Base::txBool Initialize(const Base::txString cfg_value, ConfigType cfg_type, const Base::txString strCfgId,
                                 const Base::txFloat cc0, const Base::txFloat cc1, const Base::txFloat cc2) TX_NOEXCEPT;

  /**
   * @brief 初始化车辆行为配置信息
   *
   * 该函数用于初始化车辆的行为配置信息，接收一个包含车辆行为配置信息的 json
   * 对象作为输入，并返回初始化是否成功的布尔值。
   *
   * @param jonObj 包含车辆行为配置信息的 json 对象
   * @return 返回初始化是否成功的布尔值
   */
  static Base::txBool Initialize_json_object(const nlohmann::json &jonObj) TX_NOEXCEPT;

  /**
   * @brief 获取车辆行为配置
   *
   * 该函数用于获取车辆的行为配置信息。返回值为字符串类型，包含了车辆的行为配置信息。
   *
   * @return 返回车辆行为配置信息的字符串
   */
  static Base::txString Str() TX_NOEXCEPT;

  /**
   * @brief 公共数据获取函数
   *
   * 该函数可以获取一个车辆的公共数据。如果车辆存在公共数据，则返回对应的公共数据字符串。否则返回空字符串。
   *
   * @param[in] 车辆ID 需要获取公共数据的车辆ID
   * @return 返回公共数据的字符串
   */
  static Base::txString cfg_version() TX_NOEXCEPT;

  /**
   * @brief 公共数据获取函数
   *
   * 该函数可以获取一个车辆的公共数据。如果车辆存在公共数据，则返回对应的公共数据字符串。否则返回空字符串。
   *
   * @param[in] 车辆ID 需要获取公共数据的车辆ID
   * @return 返回公共数据的字符串
   */
  static Base::txString pulic_data() TX_NOEXCEPT;

  /**
   * @brief 获取车辆行为配置
   *
   * 根据给定的事件ID，获取车辆的行为配置。如果车辆没有配置，则返回一个空的行为配置对象。
   *
   * @param[in] _strEventId 事件ID，用于指定车辆的行为配置
   * @return 返回车辆的行为配置对象
   */
  static Base::txBool HasVehicleCfg(const Base::txString &_strEventId) TX_NOEXCEPT;

  /**
   * @brief 设置车辆行为配置
   *
   * 根据给定的事件ID，设置车辆的行为配置。如果成功设置，则车辆行为会更新。
   *
   * @param[in] _strEventId 事件ID，用于指定车辆的行为配置
   * @return true ： 设置成功
   * @return false : 设置失败
   */
  static Base::txBool SetVehicleCfg(const Base::txString &_strEventId) TX_NOEXCEPT;

  /**
   * @brief 重置默认车辆行为配置内部数据结构
   *
   * 重置为默认车辆行为配置，这通常会影响到后续的车辆行为行为更新
   *
   * @return true ： 重置成功
   * @return false : 重置失败
   */
  static Base::txBool ResetDefaultCfg() TX_NOEXCEPT;

 protected:
  /**
   * @brief 使用给定的路径解析JSON配置文件，初始化车辆行为配置内部数据结构
   *
   * 注意：此功能的调用需要确保给定的路径确实是存在并且包含有效的JSON文件
   *
   * @param[in] cfg_path JSON配置文件的路径
   * @return true ： 解析并初始化成功
   * @return false : 解析并初始化失败，此时数据结构可能未被初始化
   */
  static Base::txBool Initialize_json_path(const Base::txString cfg_path) TX_NOEXCEPT;

  /**
   * @brief 从给定的JSON格式字符串中解析出来车辆行为配置信息，初始化内部数据结构
   *
   * 注意：此功能的调用需要确保传入的JSON字符串符合配置信息的格式要求
   *
   * @param[in] cfg_content JSON格式字符串，包含车辆行为配置信息
   * @return true ： 配置信息解析成功
   * @return false : 配置信息解析失败，此时数据结构可能未被初始化
   */
  static Base::txBool Initialize_json_content(const Base::txString cfg_content) TX_NOEXCEPT;
};
TX_NAMESPACE_CLOSE(Utils)
