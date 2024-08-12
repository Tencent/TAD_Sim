// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "basic.pb.h"
#include "its_sim.pb.h"
#include "mapRange.pb.h"
#include "regionBasic.pb.h"
#include "rmInternal.pb.h"
#include "road_realtimeinfo.pb.h"
namespace TrafficCloud {
class TrafficManager {
 public:
  enum ErrCode : int { trafficSucc = 0, trafficErr = 1 };
  TrafficManager() = default;
  virtual ~TrafficManager() = default;

  /**
   * @brief 获取模块名称
   *
   * @return const char* 名称字符串的指针
   */
  virtual const char* module_name(void) const noexcept = 0;

  /**
   * @brief 模块的版本
   *
   * @return const char* 版本字符串的指针
   */
  virtual const char* module_version(void) const noexcept = 0;

  struct InitInfo {
    std::string configfile_path;
  };

  /**
   * @brief 初始化地图内的交通流信息
   *
   * @param init_info 初始化信息
   * @return ErrCode 错误码
   */
  virtual ErrCode init(const InitInfo& init_info) noexcept = 0;

  virtual ErrCode release() noexcept = 0;
  /*lon = Location.position.x;
  lat = Location.position.y;
  start_v = Location.velocity.x;
  theta = Location.rpy.z;*/

  /**
   * @brief 增加主车
   *
   * @param ego_id 主车id
   * @param info 住车位置
   * @return ErrCode
   */
  virtual ErrCode addEgo(const int ego_id, const sim_msg::Location& info) noexcept = 0;

  /**
   * @brief 删除住车位置
   *
   * @param ego_id 主车id
   * @param frame_id 帧id
   * @return ErrCode
   */
  virtual ErrCode deleteEgo(const int ego_id, const int64_t frame_id) noexcept = 0;

  /**
   * @brief 根据周围地图相对于本地图的方位 获取 在周围地图视野内的本地图的交通流信息
   *
   * @param pos 地图方位
   * @param traffic 输出的交通流信息
   * @return ErrCode
   */
  virtual ErrCode getTrafficInfoBySurroundingMapPosition(sim_msg::MapPosition pos,
                                                         sim_msg::MergeTrafficInfo& traffic) noexcept = 0;

  /**
   * @brief 根据主车获取 主车视野内的交通流信息
   *
   * @param ego_id 主车id
   * @param dis 距离范围
   * @param traffic 输出的交通流信息
   * @return ErrCode
   */
  virtual ErrCode getTrafficInfoByEgo(const int ego_id, const double dis, sim_msg::Traffic& traffic) noexcept = 0;

  /**
   * @brief 更新周围地图的交通流信息
   *
   * @param merge_traffic_info 需要记录的交通流信息
   * @return ErrCode
   */
  virtual ErrCode mergeSurroundingMapInfo(const sim_msg::MergeTrafficInfo& merge_traffic_info) noexcept = 0;

  /**
   * @brief step逻辑， 更新本地图的交通留信息
   *
   * @param frame_id 当前帧数
   * @param log_level 日志级别
   * @param cur_sim_begin_time 仿真开始时间
   * @param cur_sim_step_count 当前仿真步数
   * @return ErrCode
   */
  virtual ErrCode step(const int64_t frame_id, const int log_level, const int64_t cur_sim_begin_time,
                       const int cur_sim_step_count) noexcept = 0;

  /**
   * @brief 更新主车信息， 如果出地图的管控范围，需要标记
   *
   * @param ego_id 主车id
   * @param info 主车信息
   * @return ErrCode
   */
  virtual ErrCode updateEgoInfo(const int ego_id, const sim_msg::Location& info) noexcept = 0;

#if 1 /*("__pingsn__")*/

  /**
   * @brief 更新vehicles
   *
   * @param frame_id 帧id
   * @param info vehicle信息
   * @return ErrCode
   */
  virtual ErrCode updateVehicles(const int64_t frame_id, its::txVehicles& info) noexcept = 0;

  /**
   * @brief 更新交通灯
   *
   * @param frame_id 帧id
   * @param lights_info light信息
   * @return ErrCode
   */
  virtual ErrCode updateTrafficLight(const int64_t frame_id, const its::txControllers& lights_info) noexcept = 0;

  /**
   * @brief 捕捉一帧场景的快照
   *
   * 此函数用于捕捉指定帧ID的场景快照。通常在模拟器执行阶段调用此函数。
   *
   * @param frame_id 要捕捉的帧ID
   * @param sceneSnapshot 存储快照信息的结构体
   * @return ErrCode 返回错误码，0 表示成功，非0 表示失败
   */
  virtual ErrCode snapshot(const int64_t frame_id, its::txSimStatus& sceneSnapshot) const noexcept = 0;

  /**
   * @brief 获取指定帧ID的交通信息
   *
   * 该函数用于获取指定帧ID的交通信息，通常在模拟器运行过程中调用。
   *
   * @param frame_id 需要获取的帧ID
   * @param traffic 存储交通信息的结构体
   * @return ErrCode 返回错误码，0 表示成功，非0 表示失败
   */
  virtual ErrCode getMapTrafficInfo(const int64_t frame_id, sim_msg::Traffic& traffic) noexcept = 0;
#  if 1 /*("__pingsn_event_sim__")*/

  /**
   * @brief 设置指定帧ID的交通事件
   *
   * 该函数用于设置指定帧ID的交通事件，通常在模拟器运行过程中调用。
   *
   * @param frame_id 需要设置的帧ID
   * @param evts 存储交通事件的结构体
   * @return ErrCode 返回错误码，0 表示成功，非0 表示失败
   */
  virtual ErrCode setTrafficEvent(const int64_t frame_id, const its::txEvents& evts) noexcept = 0;
#  endif /*__TX_Mark__("__pingsn_event_sim__")*/
#  if 1  /*("__pingsn_od_info__")*/
  virtual ErrCode getOdInfo(const std::string& input, std::string& output, const double dbFreeflow = 80.0) noexcept = 0;
#  endif /*__TX_Mark__("__pingsn_od_info__")*/
#endif   /*__TX_Mark__("__pingsn__")*/

#if 1 /*__engine__*/
  // Reset dynamic objects such as vehicles
  virtual void reset() noexcept = 0;

  /**
   * @brief 初始化指定的车辆模型在特定场景下的运行环境
   *
   * 这个函数用于初始化一个 tx_tc_plugin_micro 类的实例，它将需要传入一些关于场景、交通信号灯信息和车辆模型等信息。
   *
   * @param scenario_file_path 场景文件的路径，用于指定车辆模型在特定场景下的运行环境
   * @param hadmap_path 地图文件的路径，用于指定车辆模型在特定场景下的运行环境
   * @param traffic_light_controllers 交通信号灯控制器的列表，用于指定车辆模型在特定场景下的运行环境
   * @param simStatus 当前模拟器的运行状态，用于指定车辆模型在特定场景下的运行环境
   * @return 初始化成功返回 true，失败返回 false
   */
  virtual bool init(const std::string& scenario_file_path, const std::string& hadmap_path,
                    const its::txControllers& traffic_light_controllers, const its::txSimStatus& simStatus) = 0;

  /**
   * @brief 向车辆列表中添加一辆新车辆
   *
   * 该函数用于向车辆列表中添加一辆新车辆，并在列表中进行查找操作。
   *
   * @param vehicleMsg 包含新车辆信息的结构体
   * @return 添加成功返回 true，失败返回 false
   */
  virtual bool addVehicle(const its::txVehicle& vehicleMsg) noexcept = 0;

  /**
   * @brief 移除车辆
   *
   * 该函数用于从车辆列表中移除一辆指定的车辆。
   *
   * @param vehicleMsg 包含要移除的车辆信息的结构体
   * @return 移除成功返回 true，失败返回 false
   */
  virtual bool removeVehicle(const its::txVehicle& vehicleMsg) noexcept = 0;

  /**
   * @brief 根据指定的绝对时间执行一次步骤
   *
   * 该函数用于根据指定的绝对时间，执行插件的一次步骤。
   * 该函数通常会在插件周期中被调用，以保持插件的工作状态。
   *
   * @param absTime 指定的绝对时间，以毫秒为单位
   * @return 若成功，返回 ERR_OK；若失败，返回相应的错误码
   */
  virtual ErrCode step(const uint64_t absTime) noexcept = 0;

  /**
   * @brief 获取模拟器状态信息
   *
   * 返回模拟器的状态信息，包括插件模块的启动状态，模拟器的运行状态等。
   * 通常在模拟器运行过程中，调用此方法获取模拟器状态信息。
   *
   * @return 返回模拟器状态信息结构体
   */
  virtual its::txSimStat getSimStatMsg() const noexcept = 0;
#endif /*__engine__*/

#if 1 /*virtual city expand capacity*/
  /*---------------------------- FUNCTION_NAME --------------------------
   |  Function reset_simulation_range
   |
   |  Purpose:  Set simulation range (rectangle), activate / deactivate
   |            element input region.
   |
   |  Parameters:
   |      vec_valid_sim_range (IN) -- vector map range (bottom_left, top_right).
   |      bottom_left.x (IN) -- range left bottom longitude.
   |      bottom_left.y (IN) -- range left bottom latitude.
   |      top_right.x   (IN) -- range right top longitude.
   |      top_right.y   (IN) -- range right top latitude.
   |
   |  Returns:  true / false.
   *-------------------------------------------------------------------*/
  virtual bool reset_simulation_range(const std::vector<sim_msg::cloud::MapRange>& vec_valid_sim_range) noexcept = 0;

  /*---------------------------- FUNCTION_NAME --------------------------
   |  Function reset_ego_info
   |
   |  Purpose:  ReSet current range ego info.
   |            Update if it already exist, create if it doesn't exist, and delete if it doesn't exist in params.
   |
   |  Parameters:
   |      abs_time_ms  (IN) -- current abstract time in ms.
   |      cur_ego_info_vec   (IN) -- need reset ego location info.
   |
   |  Returns:  ego count after reset.
   *-------------------------------------------------------------------*/
  virtual int32_t reset_ego_info(const uint64_t abs_time_ms,
                                 const std::map<int64_t, sim_msg::Location>& cur_ego_info_vec) noexcept = 0;

  /**
   * @brief 重置流量信息
   *
   * 此函数重置并清除流量信息，以便在新的模拟周期开始时重新开始。它确保
   * 模拟器正确处理任何模拟不一致的情况。
   *
   * @param abs_time_ms 当前模拟器时间（以毫秒为单位）
   * @param status      当前模拟器状态
   * @param maxErrDist 允许的最大错误距离（默认为2.0，单位：千米）
   * @return int32_t    返回重置流量信息的状态
   */
  virtual int32_t reset_traffic_info(const uint64_t abs_time_ms, const its::txSimStatus&,
                                     const double maxErrDist = 2.0) noexcept = 0;

  /**
   * @brief 捕捉指定帧的微场快照
   *
   * 此函数用于获取特定帧的微场快照，以在后续模拟过程中使用。获取到的微场快照信息将在后续模拟过程中提供给其他相关模块。
   *
   * @param frame_id 指定的帧ID
   * @param microSceneSnapshot 用于存储微场快照的引用
   * @return ErrCode 返回操作的错误码
   */
  virtual ErrCode snapshot_micro(const int64_t frame_id, its::txSimStatus& microSceneSnapshot) noexcept = 0;

  /**
   * @brief 重置流量信息
   *
   * 此函数重置传入的流量信息，并确保模拟器可以在新的模拟周期中使用。
   * 通常，此函数将在模拟器过程中用于解决模拟不一致问题。
   *
   * @param abs_time_ms 当前模拟器时间（以毫秒为单位）
   * @param traffic 要重置的流量信息
   * @param maxErrDist 允许的最大错误距离（默认值为2.0，单位：千米）
   * @return int32_t 返回操作的错误码
   */
  virtual int32_t reset_traffic_info(const uint64_t abs_time_ms, const sim_msg::Traffic&,
                                     const double maxErrDist = 2.0) noexcept = 0;

  /**
   * @brief 获取指定帧的微场景快照
   *
   * 该函数用于获取指定帧的微场景快照信息，以便在后续模拟过程中使用。
   * 通常，该函数会在模拟器的模拟周期中使用，以解决模拟不一致问题。
   *
   * @param frame_id 指定的帧ID
   * @param microSceneSnapshot 用于存储微场景快照的引用
   * @return ErrCode 返回操作的错误码
   */
  virtual ErrCode snapshot_micro(const int64_t frame_id, sim_msg::Traffic& microSceneSnapshot) noexcept = 0;

  virtual ErrCode inject_perception_raw_data(const PerceptionMessageData& _input_frame) noexcept = 0;

  /**
   * @brief 初始化log2world仿真
   *
   * @param status_pb_record 状态记录pb
   * @param 事件json
   */
  virtual ErrCode initialize_l2w_sim(const its::txStatusRecord& status_pb_record /* size = 1 */,
                                     const std::string _event_json) noexcept = 0;
  virtual ErrCode initialize_l2w_sim(const sim_msg::TrafficRecords& traffic_pb_record /* size = 1 */,
                                     const std::string _event_json) noexcept = 0;

  virtual ErrCode set_vehicle_cfg(const std::string _cfg_json_content) noexcept = 0;

  virtual ErrCode inject_traffic_event(const std::string _event_json) noexcept = 0;
#endif /*virtual city expand capacity*/
};
}  // namespace TrafficCloud

std::shared_ptr<TrafficCloud::TrafficManager> createTrafficManagerPtr(void);
// - begin
// update_od_info()
// step() // 只能运行一帧
// get_traffic_info() //获取一帧数据, 每帧的时间t()

// - run
// step()
// get_traffic_info()

// - end
// step()
// get_traffic_info()
// get_traffic_snapshot()
