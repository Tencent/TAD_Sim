// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "coordinator/status.h"
#include "librdkafka/rdkafkacpp.h"
#include "traffic_frame.pb.h"

namespace tx_sim {
namespace coordinator {

/**
 * @brief filter trajectory points according to distance and theta diff of each point, but always keep first and last
 * points.
 * @param pEgoPlanningLine the trajectory points to filter
 * @param angleLimit the angle limit argument
 * @param angleLimitLen the length of angleLimit argument
 */
void SampleTrajectory(sim_msg::Trajectory* pEgoPlanningLine, double angleLimit, double angleLimitLen);

/**
 * @brief get the used memory of current process
 * @param uVMem the used physical memory of current process
 * @return the used virtual memory of current process
 */
int64_t GetMemUse(int64_t& uVMem);

/**
 * @brief use kafka publisher to send topic messages data to cloud
 */
class SimCloudTopicWriter final {
 public:
  // constructor
  SimCloudTopicWriter(const std::string& config_path);
  // destructor
  virtual ~SimCloudTopicWriter();

  /**
   * @brief write the StepMessage in a special format to the kafka queue.
   * @param msgs step message.
   * @param frame_id frame id.
   * @param sim_time simulation time.
   */
  void Write(const StepMessage& msgs, uint64_t frame_id, uint32_t sim_time);

  /**
   * @brief write the grading result in a special format to the kafka queue.
   * @param grading_result grading result string.
   * @param frame_id frame id.
   * @param sim_time simulation time.
   */
  void Write(const std::string& grading_result, uint64_t frame_id, uint32_t sim_time);

  /**
   * @brief write the StepMessage to in protobuf format the kafka queue.
   * @param msgs step message.
   * @param frame_id frame id.
   * @param sim_time simulation time.
   * @param city whether the simulation is city mode or not
   * @param curmiles the current miles ego has driven
   * @param begin_frame_id the begin frame id of the simulation
   */
  bool WritePb(const StepMessage& msgs, uint64_t frame_id, uint32_t sim_time, bool city = false, double curmiles = 0.0,
               uint64_t begin_frame_id = 0);

  /**
   * @brief write the grading result in protobuf format to the kafka queue.
   * @param grading_result grading result string.
   * @param frame_id frame id.
   * @param sim_time simulation time.
   */
  void WritePb(const std::string& grading_result, uint64_t frame_id, uint32_t sim_time);

  /**
   * @brief set some IDs, including the ID used as key in Kafka publish.
   * @param job_id
   * @param scenario_id
   * @param ego_id
   * @param from car type
   * @param l2w_input_switch_time the time to switch in log2world
   * @param logsim_start_timestamp the logsim start timestamp
   * @param l2w_real_switch_time the real switch time in log2world
   */
  void SetIds(int32_t job_id, int32_t scenario_id, int32_t ego_id, const std::string& from = "EGO",
              int32_t l2w_input_switch_time = -1, int64_t logsim_start_timestamp = -1,
              int32_t l2w_real_switch_time = -1);

  /**
   * @brief record the ego type to m_sEgoType
   * @param sType current ego type
   */
  void SetEgoType(std::string sType);

  /**
   * @brief collect the topic information and send it to the http server.
   * @param body topic information.
   */
  void TotalTopic(const std::string& body);

 private:
  /**
   * @brief assemble setting the header of special format
   * @param[out] payload the target payload of the message
   * @param[in] frame_id the frame id
   * @param[in] sim_time the simulation time
   */
  void AssembleHeader(std::vector<std::uint8_t>& payload, uint64_t frame_id, uint32_t sim_time);

  /**
   * @brief assemble setting the body of special format
   * @param[out] payload the target payload of the message
   * @param[in] offset the offset of the payload
   * @param[in] msg_type the type of the message
   * @param[in] msg the zmq message to write
   */
  size_t AssembleBody(std::vector<std::uint8_t>& payload, size_t offset, const std::string& msg_type,
                      const zmq::message_t& msg);

  /**
   * @brief send the message to the kafka
   * @param[in] payload the payload of the message to be send to kafka
   * @param[in] type the type of the message, if it is "grading feedback", it means the last message and does some
   * special process.
   */
  void WriteMsgQueue(std::vector<std::uint8_t>& payload, const std::string& type);

  /**
   * @brief fill the ego type in to the sim_msg::TrafficEgo message
   * @param pEgo the target sim_msg::TrafficEgo message
   */
  void FillEgoType(sim_msg::TrafficEgo* pEgo);

  /**
   * @brief fill the target messages with record info and last messages.
   * @param trafficFrame the target sim_msg::TrafficFrame message
   * @param pEgo the target sim_msg::TrafficEgo message
   * @param pLogsimEgo the target sim_msg::TrafficEgo message
   * @param bSwitch whether to switch the record mode
   */
  void FileCacheData(sim_msg::TrafficFrame& trafficFrame, sim_msg::TrafficEgo* pEgo, sim_msg::TrafficEgo* pLogsimEgo,
                     bool bSwitch);

  //! @brief 成员变量：log_key_
  //! @details 成员变量用途：存储日志密钥
  std::string log_key_;
  //! @brief 成员变量：from_type_
  //! @details 成员变量用途：存储来源类型
  std::string from_type_;
  //! @brief 成员变量：job_id_
  //! @details 成员变量用途：存储任务ID
  int32_t job_id_ = 0;
  //! @brief 成员变量：ego_id_
  //! @details 成员变量用途：存储自身ID
  int32_t ego_id_ = -1;
  //! @brief 成员变量：scenario_id_
  //! @details 成员变量用途：存储场景ID
  int32_t scenario_id_ = 0;
  //! @brief 成员变量：l2w_input_switch_time_
  //! @details 成员变量用途：存储Log2World输入切换时间
  int32_t l2w_input_switch_time_ = -1;
  //! @brief 成员变量：m_l2w_real_switch_time
  //! @details 成员变量用途：存储Log2World实际切换时间
  int32_t m_l2w_real_switch_time = -1;
  //! @brief 成员变量：logsim_start_timestamp_
  //! @details 成员变量用途：存储Logsim开始时间戳
  int64_t logsim_start_timestamp_ = -1;
  //! @brief 成员变量：write_times_
  //! @details 成员变量用途：存储写入次数
  uint64_t write_times_ = 0;
  //! @brief 成员变量：header_length_
  //! @details 成员变量用途：存储头部长度
  size_t header_length_ = 0;

  //! @brief 成员变量：mq_topic_
  //! @details 成员变量用途：存储消息队列主题
  std::string mq_topic_;
  //! @brief 成员变量：m_sEgoType
  //! @details 成员变量用途：存储自身类型
  std::string m_sEgoType;
  //! @brief 成员变量：key_id_byte_array_
  //! @details 成员变量用途：存储密钥ID字节数组
  std::vector<uint8_t> key_id_byte_array_;
  // kafka related
  //! @brief 成员变量：producer_
  //! @details 成员变量用途：存储Kafka生产者的指针
  std::unique_ptr<RdKafka::Producer> producer_;
  //! @brief 成员变量：dr_cb_
  //! @details 成员变量用途：存储Kafka投递报告回调的指针
  std::unique_ptr<RdKafka::DeliveryReportCb> dr_cb_;
  //! @brief 成员变量：conf_
  //! @details 成员变量用途：存储Kafka配置的指针
  std::unique_ptr<RdKafka::Conf> conf_;
  //! @brief 成员变量：m_totalTopicServer
  //! @details 成员变量用途：存储总主题服务器
  std::string m_totalTopicServer;
  // store the last message
  //! @brief 成员变量：m_bRecLoc
  //! @details 成员变量用途：标记是否记录位置信息
  bool m_bRecLoc = false;
  //! @brief 成员变量：m_LastLocation
  //! @details 成员变量用途：存储最后一个位置信息
  sim_msg::Location m_LastLocation;
  //! @brief 成员变量：m_bLogsimRecLoc
  //! @details 成员变量用途：标记是否记录Logsim位置信息
  bool m_bLogsimRecLoc = false;
  //! @brief 成员变量：m_LogsimLastLocation
  //! @details 成员变量用途：存储最后一个Logsim位置信息
  sim_msg::Location m_LogsimLastLocation;
  //! @brief 成员变量：m_bRecTraffic
  //! @details 成员变量用途：标记是否记录交通信息
  bool m_bRecTraffic = false;
  //! @brief 成员变量：m_LastTraffic
  //! @details 成员变量用途：存储最后一个交通信息
  sim_msg::Traffic m_LastTraffic;
  //! @brief 成员变量：m_bLogsimRecTraffic
  //! @details 成员变量用途：标记是否记录Logsim交通信息
  bool m_bLogsimRecTraffic = false;
  //! @brief 成员变量：m_LogsimLastTraffic
  //! @details 成员变量用途：存储最后一个Logsim交通信息
  sim_msg::Traffic m_LogsimLastTraffic;
  //! @brief 成员变量：m_bRecTrajectory
  //! @details 成员变量用途：标记是否记录轨迹信息
  bool m_bRecTrajectory = false;
  //! @brief 成员变量：m_LastTrajectory
  //! @details 成员变量用途：存储最后一个轨迹信息
  sim_msg::Trajectory m_LastTrajectory;
  //! @brief 成员变量：m_bLogsimRecTrajectory
  //! @details 成员变量用途：标记是否记录Logsim轨迹信息
  bool m_bLogsimRecTrajectory = false;
  //! @brief 成员变量：m_LogsimLastTrajectory
  //! @details 成员变量用途：存储最后一个Logsim轨迹信息
  sim_msg::Trajectory m_LogsimLastTrajectory;
};

}  // namespace coordinator
}  // namespace tx_sim
