// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <map>
#include "its_sim.pb.h"
#include "traffic.pb.h"

namespace hadmap {
class txMapHandle;
}

namespace TrafficFlow {
class TrafficRecord {
 public:
  TrafficRecord();
  ~TrafficRecord();

  struct VehicleStartEndlTime {
    int64_t arriveTime;  // ms
    int64_t startTime;   // ms
    std::vector<int> actualPath;
  };

  struct RoadTravelTime {
    double roadLength;
    int passVehicle;
    int64_t averageTime;  // ms
  };

  struct VehicleArriveRoadTime {
    int curRoadId;
    int64_t arriveTime;  // ms
  };

  /**
   * @brief 记录行程时间
   *
   * 该函数根据指定的起始交通信息，计算并记录当前时间的行程时间。
   *
   * @param originTraffic 指定的起始交通信息
   * @param currentTime 当前时间
   */
  void RecordTravelTime(const sim_msg::Traffic& originTraffic, int64_t currentTime);

  /**
   * @brief 删除未达到终点的车辆的起始和终点时间
   *
   * 该函数从给定的所有车辆的起始和终点时间的映射中，移除那些未达到终点的车辆的起始和终点时间。
   *
   * @param originTraffic 原始交通信息
   * @param allVehicleStartEndTime 所有车辆的起始和终点时间的映射
   * @return 移除未达到终点的车辆的起始和终点时间的映射
   */
  std::map<int, VehicleStartEndlTime> RemoveVehicleNotReachEnd(
      const sim_msg::Traffic& originTraffic, std::map<int, VehicleStartEndlTime> allVehicleStartEndTime) const;

  /**
   * @brief 清除所有记录
   *
   * 清除tx_tc_ps_traffic_record对象中所有记录的数据，以便重新开始记录。
   */
  void Clear();

  /**
   * @brief 清除部分记录
   *
   * 清除tx_tc_ps_traffic_record对象中的部分记录，可以通过设置partial_clear_enable参数控制是否清除。
   *
   * @param partial_clear_enable 是否开启部分清除功能，1表示开启，0表示关闭。
   */
  void PartialClear();

  /**
   * @brief 输出流量记录到PB文件
   *
   * 该函数将 tx_tc_ps_traffic_record 对象中的流量记录输出到指定的 Protocol Buffer 文件。
   *
   * @param pbFilePath 包含 Protocol Buffer 文件路径的字符串，该文件将存储流量记录数据。
   */
  void OutputTrafficRecordsPBFile(std::string pbFilePath);

  /**
   * @brief 获取模拟统计信息消息
   *
   * 该函数根据传入的原始流量数据生成一个模拟统计信息消息，并返回其统计数据。
   *
   * @param originTraffic 传入的原始流量数据
   * @param currentTime 当前时间，以毫秒为单位
   * @return its::txSimStat 返回模拟统计信息
   */
  its::txSimStat getSimStatMsg(const sim_msg::Traffic& originTraffic, int64_t currentTime) const;

  /**
   * @brief 获取当前traffic状态
   *
   * 根据传入的原始流量数据，获取当前流量状态。
   *
   * @param originTraffic 传入的原始流量数据
   * @return its::txSimStatus 当前流量状态
   */
  its::txSimStatus getCurrentTrafficState(const sim_msg::Traffic& originTraffic) const;
  // sim_msg::Traffic RecordMesoInfo(const sim_msg::Traffic& originTraffic) const;

  /**
   * @brief 记录输入的交通车辆信息
   *
   * 此函数接收一个包含交通车辆信息的消息，并将其添加到对应的交通数据记录中。
   *
   * @param vehiclesMsg 包含交通车辆信息的消息
   */
  void RecordInputVehicles(const its::txVehicles& vehiclesMsg);

  /**
   * @brief 记录输入的车辆信息
   *
   * 此函数接收一个包含车辆信息的消息，并将其添加到对应的交通数据记录中。
   *
   * @param vehicleMsg 包含车辆信息的消息
   */
  void RecordInputVehicle(const its::txVehicle& vehicleMsg);

  /**
   * @brief 记录交通信息
   *
   * 该函数根据提供的交通信息记录交通信息。
   *
   * @param originTraffic 提供的交通信息
   */
  void RecordTrafficMsg(const sim_msg::Traffic& originTraffic);

 protected:
  /**
   * @brief 根据车辆ID获取车辆信息
   *
   * 该函数根据提供的车辆ID获取车辆信息。
   *
   * @param vehicleId 车辆ID，64位无符号整数
   * @param vehicle 获取到的车辆信息
   * @return 成功返回true，失败返回false
   */
  bool getInputVehicle(uint64_t vehicleId, its::txVehicle& vehicle) const;

  /**
   * @brief 更新指定道路的行驶时间
   *
   * 根据指定道路的ID和单车行驶时间，更新该道路的行驶时间信息。
   *
   * @param roadId 道路ID
   * @param singleVehicleTravelTime 单车行驶时间
   * @param allRoadTravelTime 所有道路的行驶时间信息
   * @return 更新后的道路行驶时间信息
   */
  std::map<int, TrafficRecord::RoadTravelTime> UpdateRoadTravelTime(
      int roadId, int64_t singleVehicleTravelTime, const std::map<int, RoadTravelTime>& allRoadTravelTime) const;

  /**
   * @brief 计算最终道路行驶时间
   *
   * 根据给定的交通信息和当前时间，计算所有道路的最终行驶时间。
   *
   * @param originTraffic 原始交通信息
   * @param currentTime 当前时间，单位为秒
   * @return std::map<int, RoadTravelTime> 所有道路的最终行驶时间，以道路ID为键
   */
  std::map<int, RoadTravelTime> CalculateFinalRoadTravelTime(const sim_msg::Traffic& originTraffic,
                                                             int64_t currentTime) const;

  /**
   * @brief 使用当前车辆更新道路的行驶时间
   *
   * 根据给定的原始交通信息和当前时间，更新道路的行驶时间。
   *
   * @param roadsTravelTime 原始的道路行驶时间信息
   * @param originTraffic 原始交通信息
   * @param currentTime 当前时间，单位为秒
   */
  void UpdateRoadTravelTimeWithCurrentVehicle(std::map<int, RoadTravelTime> roadsTravelTime,
                                              const sim_msg::Traffic& originTraffic, int64_t currentTime) const;

  /**
   * @brief 根据MESO网络的要求修改车辆信息
   *
   * 根据给定的车辆信息，修改车辆信息以符合MESO网络的要求。此函数应该将车辆信息更新到`its::txVehicle`引用。
   *
   * @param car 原始车辆信息
   * @param vehicle 需要修改的车辆引用
   * @return 修改成功返回true，否则返回false
   */
  bool ModifyVehicleForMeso(const sim_msg::Car& car, its::txVehicle& vehicle) const;

  sim_msg::TrafficRecords traffic_records;
  std::map<int, VehicleStartEndlTime> allVehicleStartEndTime;
  std::map<int, RoadTravelTime> allRoadTravelTime;
  std::map<int, VehicleArriveRoadTime> allVehicleArriveRoadTime;
  std::map<int, its::txVehicle> inputVehicles_;
};

}  // namespace TrafficFlow
