// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "cloud_topic_writer.h"

#include <limits>
#include <map>

#include "boost/algorithm/string.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "glog/logging.h"
#include "librdkafka/rdkafka.h"
#include "server_impl/http_client.h"
#include "traffic_frame.pb.h"
#include "txsim_messenger.h"
#include "utils/time.h"


#define TXSIM_CLOUD_CONFIG_KEY_MSG_QUEUE_INFO "mqInfo."
#define TXSIM_CLOUD_CONFIG_KEY_MQ_BROKER      TXSIM_CLOUD_CONFIG_KEY_MSG_QUEUE_INFO "broker"
#define TXSIM_CLOUD_CONFIG_KEY_MQ_TOPIC       TXSIM_CLOUD_CONFIG_KEY_MSG_QUEUE_INFO "topic"
#define TXSIM_CLOUD_CONFIG_KEY_MQ_USER        TXSIM_CLOUD_CONFIG_KEY_MSG_QUEUE_INFO "username"
#define TXSIM_CLOUD_CONFIG_KEY_MQ_PWD         TXSIM_CLOUD_CONFIG_KEY_MSG_QUEUE_INFO "passwd"

#define TXSIM_CLOUD_CONFIG_TOPIC_TOTAL_SERVER "topicTotalServer"


#define TXSIM_KAFKA_PRODUCER_POLL_TIMEOUT  0
#define TXSIM_KAFKA_PRODUCER_POLL_INTERVAL 10

#define TXSIM_CLOUD_MSG_VERSION 1

#define safe_printf(DST, LEN, fmt, _arg...) \
  do {                                      \
    snprintf(DST, (LEN), fmt, ##_arg);      \
    (DST)[(LEN) - 1] = 0;                   \
  } while (0)

namespace tx_sim {
namespace coordinator {

/**
 * @brief a map from tx_sim::topic to string abbreviated type name for special fommat
 */
static const std::map<std::string, std::string> kTopic2Types = {
    {tx_sim::topic::kTraffic, "TFC"}, {tx_sim::topic::kLocation, "LOC"},   {tx_sim::topic::kTrajectory, "TRJ"},
    {tx_sim::topic::kGrading, "KPI"}, {tx_sim::topic::kPlanStatus, "STS"}, {"LOCATION_TRAILER", "LOR"},
    {"TRAFFIC_REPLAY", "TFR"}};

/**
 * @brief transform a string to lower case by calling std::transform
 */
void ToLowerCase(std::string& strString) {
  std::transform(strString.begin(), strString.end(), strString.begin(), ::tolower);
}

/**
 * @brief check if a string starts with another string. If bLowerCase is true, the source string will be converted to
 * lower case firstly
 * @param strString the source string
 * @param strPattern the pattern string
 * @param bLowerCase if true, the source string will be converted to lower case firstly
 * @return true if the source string starts with the pattern string
 */
bool StartsWith(const std::string& strString, const std::string& strPattern, bool bLowerCase) {
  size_t nThisLen = strString.length();
  size_t nPatternLen = strPattern.length();

  if (nThisLen < nPatternLen || nPatternLen == 0) return false;

  std::string startOfThis = strString.substr(0, nPatternLen);

  if (bLowerCase) ToLowerCase(startOfThis);

  return (startOfThis == strPattern);
}

/**
 * @brief Trims whitespace characters from the beginning and/or end of a string.
 *
 * This function removes all occurrences of whitespace characters defined in strDelims
 * from the beginning (left side) and/or the end (right side) of the input string.
 * By default, it trims both sides. The function modifies the input string in place.
 *
 * @param strString The string to be trimmed, modified in place.
 * @param bLeft Flag to indicate whether to trim the left side of the string. Default is true.
 * @param bRight Flag to indicate whether to trim the right side of the string. Default is true.
 */
void Trim(std::string& strString, bool bLeft = true, bool bRight = true) {
  static const std::string strDelims = " \t\r\n ";
  if (bRight) strString.erase(strString.find_last_not_of(strDelims) + 1);  // trim bRight

  if (bLeft) strString.erase(0, strString.find_first_not_of(strDelims));  // trim bLeft
}

/**
 * @brief get the line with strPrefix in the file of strFilePath
 * @param strFilePath the file path
 * @param strPrefix the prefix of the line
 * @return the line with strPrefix in the file of strFilePath if found, otherwise empty string
 */
std::string GetFileLine(const std::string& strFilePath, const std::string& strPrefix) {
  char szBuff[4096];
  std::fstream input(strFilePath, std::ios::in | std::ios::binary);
  if (input.is_open()) {
    while (!input.eof()) {
      input.getline(szBuff, sizeof(szBuff));
      if (StartsWith(szBuff, strPrefix, false)) {
        input.close();
        std::string strRes = szBuff + strPrefix.length();
        Trim(strRes);
        return strRes;
      }
    }
  }
  input.close();
  return "";
}

int64_t GetMemUse(int64_t& uVMem) {
  uVMem = 0;
  int64_t uMem = 0;

  int nPid = getpid();
  char szFile[128] = {0};
  safe_printf(szFile, sizeof(szFile), "/proc/%d/status", nPid);
  std::string strMem = GetFileLine(szFile, "VmRSS:");
  std::string strVMem = GetFileLine(szFile, "VmSize:");
  uMem = std::atol(strMem.c_str()) / 1024;
  uVMem = std::atol(strVMem.c_str()) / 1024;
  return uMem;
}


/**
 * @brief the delivery report callback for the cloud topic writer, inherited from RdKafka::DeliveryReportCb
 */
class SimMsgDeliveryReportCb : public RdKafka::DeliveryReportCb {
 public:
  void dr_cb(RdKafka::Message& msg) override {
    auto st = msg.status();
    if (st == RdKafka::Message::MSG_STATUS_PERSISTED) return;

    int32_t key = 0;
    std::string status_name;
    switch (msg.status()) {
      case RdKafka::Message::MSG_STATUS_NOT_PERSISTED: status_name = "not persisted"; break;
      case RdKafka::Message::MSG_STATUS_POSSIBLY_PERSISTED: status_name = "possibly persisted"; break;
      default: status_name = "unknown"; break;
    }
    std::vector<uint8_t> key_bytes(msg.key()->begin(), msg.key()->end());
    std::memcpy(&key, key_bytes.data(), 4);
    LOG(ERROR) << "kafka message(" << msg.topic_name() << " " << key << " <" << msg.partition() << "," << msg.offset()
               << ">) delivery error: [" << status_name << "]" << msg.errstr();
  }
};


/**
 * @brief the error callback for kafka publisher in the cloud topic writer
 * @param rk the kafka producer
 * @param err the error code
 * @param reason the error reason
 * @param opaque the opaque pointer
 */
static void err_cb(rd_kafka_t* rk, int err, const char* reason, void* opaque) {
  if (err == RD_KAFKA_RESP_ERR__FATAL) {
    char errstr[512];
    err = rd_kafka_fatal_error(rk, errstr, sizeof(errstr));
    LOG(ERROR) << "FATAL kafka error(" << rd_kafka_name(rk) << " " << rd_kafka_err2str(rd_kafka_resp_err_t(err))
               << "): " << errstr;
  } else {
    LOG(ERROR) << "kafka error(" << rd_kafka_name(rk) << " " << rd_kafka_err2str(rd_kafka_resp_err_t(err))
               << "): " << reason;
  }
}


#define SET_RDKAFKA_CONF(conf, key, value, err)                                                      \
  if (conf->set(key, value, err) != RdKafka::Conf::CONF_OK) {                                        \
    LOG(FATAL) << "rdkafka set configuration <" << key << "> with <" << value << "> error: " << err; \
  }

/**
 * @brief configure the kafka producer
 * @param broker_list the kafka broker list
 * @param user_name the kafka user name
 * @param password the kafka password
 * @return the rdkafka configuration
 */
static RdKafka::Conf* ConfigureKafka(const std::string& broker_list, const std::string& user_name,
                                     const std::string& password) {
  RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
  // RdKafka::Conf* t_conf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);
  std::string err_str;

  SET_RDKAFKA_CONF(conf, "bootstrap.servers", broker_list, err_str)

  // for a safer message delivery.
  SET_RDKAFKA_CONF(conf, "enable.idempotence", "true", err_str)
  // below 3 configuration is auto set when idempotence enabled.
  // SET_RDKAFKA_CONF(t_conf, "acks", "all", err_str)
  // SET_RDKAFKA_CONF(conf, "retries", std::to_string(std::numeric_limits<int32_t>::max()), err_str)
  // max in flight = 1 (0.11 >= Kafka < 1.1) OR 5 (Kafka >= 1.1)
  // SET_RDKAFKA_CONF(conf, "max.in.flight.requests.per.connection", "1", err_str)

  // for a higher throughput.
  SET_RDKAFKA_CONF(conf, "linger.ms", "1000", err_str)
  // TODO(nemo): batch.size needs librdkafka version v1.5.0+
  // SET_RDKAFKA_CONF(conf, "batch.size", "1000000", err_str)
  SET_RDKAFKA_CONF(conf, "batch.num.messages", "10000", err_str)
  SET_RDKAFKA_CONF(conf, "message.max.bytes", "2000000", err_str);  // in case of a very large traffic msg.

  if (!user_name.empty()) {  // kafka has auth.
    SET_RDKAFKA_CONF(conf, "security.protocol", "sasl_plaintext", err_str);
    SET_RDKAFKA_CONF(conf, "sasl.mechanism", "PLAIN", err_str);
    SET_RDKAFKA_CONF(conf, "sasl.username", user_name, err_str);
    // throw if the pwd field cannot be retrived.
    SET_RDKAFKA_CONF(conf, "sasl.password", password, err_str);
    LOG(INFO) << "kafka set auth info with user: " << user_name;
  }

  return conf;
}


SimCloudTopicWriter::SimCloudTopicWriter(const std::string& config_path) {
  // parse the config file.
  LOG(INFO) << "sim-cloud topic writer reading output config at " << config_path << " ...";
  boost::property_tree::ptree pt;
  boost::property_tree::json_parser::read_json(config_path, pt);

  // kafka config setting
  std::string broker_list = pt.get<std::string>(TXSIM_CLOUD_CONFIG_KEY_MQ_BROKER);
  if (broker_list.empty()) {
    LOG(WARNING) << "kafka broker address is empty. no kafka output!";
    return;
  }

  std::string user_name = pt.get<std::string>(TXSIM_CLOUD_CONFIG_KEY_MQ_USER, "");
  std::string pwd = pt.get<std::string>(TXSIM_CLOUD_CONFIG_KEY_MQ_PWD, "");
  mq_topic_ = pt.get<std::string>(TXSIM_CLOUD_CONFIG_KEY_MQ_TOPIC);
  conf_.reset(ConfigureKafka(broker_list, user_name, pwd));

  m_totalTopicServer = pt.get<std::string>(TXSIM_CLOUD_CONFIG_TOPIC_TOTAL_SERVER, "");
  LOG(INFO) << "topic total server:" << m_totalTopicServer;

  std::string err_str;
  dr_cb_.reset(new SimMsgDeliveryReportCb);
  conf_->set("dr_cb", dr_cb_.get(), err_str);
  rd_kafka_conf_set_error_cb(conf_->c_ptr_global(), err_cb);

  // kafka producer create
  producer_.reset(RdKafka::Producer::create(conf_.get(), err_str));
  LOG(INFO) << "kafka producer writing to topic " << mq_topic_ << "@" << broker_list;
  if (producer_) {
    LOG(INFO) << "kafka producer writing success";
  } else {
    LOG(INFO) << "kafka producer writing failed";
  }

  key_id_byte_array_.resize(4);  // kafka key is 4 bytes long.
  m_bRecLoc = false;
}


SimCloudTopicWriter::~SimCloudTopicWriter() {
  if (producer_) {
    // flush the kafka producer requests before destroy it.
    if (producer_->flush(10000) != RdKafka::ERR_NO_ERROR) LOG(ERROR) << "producer flush error: timed out!";
  }
}

void SimCloudTopicWriter::TotalTopic(const std::string& body) {
  if (m_totalTopicServer.empty()) return;
  std::vector<std::string> vecStr;
  boost::algorithm::split(vecStr, m_totalTopicServer, boost::is_any_of("/:"));
  std::string totalTopicServerIP = vecStr[3];
  std::string totalTopicServerPort = vecStr[4];

  net::io_context ioc;
  std::make_shared<tx_sim::service::CHttpSessionClient>(ioc)->run(
      totalTopicServerIP.c_str(), totalTopicServerPort.c_str(), "/statistics/signal", 1, body);
  ioc.run();
}


void SimCloudTopicWriter::Write(const StepMessage& msgs, uint64_t frame_id, uint32_t sim_time) {
  bool is_logsim = (from_type_ == "LOG");
  size_t total_length = header_length_;
  std::map<std::string, std::string> topic_type_map;

  for (const auto& m : msgs.messages) {
    std::string topic = m.first.sTopic;
    if (is_logsim && boost::ends_with(topic, tx_sim::impl::kLogTopicPostfix)) {
      topic = topic.substr(0, topic.rfind(tx_sim::impl::kLogTopicPostfix));
      auto it = kTopic2Types.find(topic);
      if (it != kTopic2Types.cend()) {
        total_length += 7 + 4 + m.second.size();  // "REP_" + message type + message length + message content
        topic_type_map[m.first.sTopic] = std::string("REP_") + it->second;
      }
    } else {
      auto it = kTopic2Types.find(topic);
      if (it != kTopic2Types.cend()) {
        total_length += 3 + 4 + m.second.size();  // message type + message length + message content
        topic_type_map[m.first.sTopic] = it->second;
      }
    }
  }

  if (total_length > header_length_) {  // has body content.
    std::vector<std::uint8_t> payload;
    payload.resize(total_length);
    AssembleHeader(payload, frame_id, sim_time);
    size_t cur_copy_offset = header_length_;
    for (const auto& m : msgs.messages) {
      auto it = topic_type_map.find(m.first.sTopic);
      if (it != topic_type_map.cend()) cur_copy_offset = AssembleBody(payload, cur_copy_offset, it->second, m.second);
    }
    WriteMsgQueue(payload, "step message");
  }
}


void SimCloudTopicWriter::Write(const std::string& grading_result, uint64_t frame_id, uint32_t sim_time) {
  if (grading_result.size() > 0) {
    size_t total_length = header_length_ + 3 + 4 + grading_result.size();
    std::vector<uint8_t> payload;
    payload.resize(total_length);
    AssembleHeader(payload, frame_id, sim_time);
    zmq::message_t grading_msg(grading_result.data(), grading_result.size());
    AssembleBody(payload, header_length_, "RST", grading_msg);
    WriteMsgQueue(payload, "grading result");
    LOG(INFO) << log_key_ << " sim-cloud topic writer written grading result to kafka topic " << mq_topic_ << " with "
              << payload.size() << " bytes. frame_id=" << frame_id;
    LOG(INFO) << "from_chars job_id_ frame_id ver scenario_id_ is_logsim sim_time cur_time log_time";
  }
}
//    {tx_sim::topic::kTraffic, "TFC"}, {tx_sim::topic::kLocation, "LOC"},   {tx_sim::topic::kTrajectory, "TRJ"},
//    {tx_sim::topic::kGrading, "KPI"}, {tx_sim::topic::kPlanStatus, "STS"}, {"LOCATION_TRAILER", "LOR"},

void SampleTrajectory(sim_msg::Trajectory* pEgoPlanningLine, double angleLimit, double angleLimitLen) {
  angleLimit = angleLimit * 3.1415926 / 180;
  if (pEgoPlanningLine->point_size() <= 2) return;
  sim_msg::Trajectory tra;
  tra.CopyFrom(*pEgoPlanningLine);
  pEgoPlanningLine->mutable_point()->Clear();
  pEgoPlanningLine->add_point()->CopyFrom(tra.point(0));
  int nSize = tra.point_size();
  for (int k = 0; k < nSize - 1; k++) {
    const sim_msg::TrajectoryPoint& pk0 = tra.point(k);
    const sim_msg::TrajectoryPoint& pk1 = tra.point(k + 1);
    double dx = (pk1.x() - pk0.x()) * 100000;
    double dy = (pk1.y() - pk0.y()) * 100000;
    double dLen = std::sqrt(std::pow(dx, 2) + std::pow(dy, 2));
    if (dLen == 0) continue;
    int n = k + 2;
    for (; n < nSize; n++) {
      const sim_msg::TrajectoryPoint& pn = tra.point(n);
      double dnx = (pn.x() - pk0.x()) * 100000;
      double dny = (pn.y() - pk0.y()) * 100000;
      double dNextLen = std::sqrt(std::pow(dnx, 2) + std::pow(dny, 2));
      if (dNextLen == 0) continue;
      double dotMul = dx * dnx + dy * dny;
      double angle = fabs(std::acos(dotMul / (dLen * dNextLen)));
      if (angle >= angleLimit || dNextLen * angle >= angleLimit * angleLimitLen) {
        if (angle >= 2 * angleLimit || dNextLen > angleLimitLen * 3) {
          pEgoPlanningLine->add_point()->CopyFrom(tra.point(n - 1));
        }

        if (n != nSize - 1) {
          pEgoPlanningLine->add_point()->CopyFrom(tra.point(n));
          break;
        }
      }
    }
    k = n;
  }
  pEgoPlanningLine->add_point()->CopyFrom(tra.point(nSize - 1));
}

void SimCloudTopicWriter::FillEgoType(sim_msg::TrafficEgo* pEgo) {
  if (m_sEgoType == "truck") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_Truck);
  } else if (m_sEgoType == "AIV_FullLoad_001") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_AIV_FullLoad_001);
  } else if (m_sEgoType == "AIV_Empty_001") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_AIV_Empty_001);
  } else if (m_sEgoType == "MPV_001") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_Mpv);
  } else if (m_sEgoType == "AIV_FullLoad_002") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_AIV_FullLoad_002);
  } else if (m_sEgoType == "AIV_Empty_002") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_AIV_Empty_002);
  } else if (m_sEgoType == "MIFA_001") {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_MIFA_01);
  } else {
    pEgo->set_type(sim_msg::EgoType::Ego_Type_Car);
  }
  pEgo->set_id(ego_id_);
  pEgo->set_egotype(m_sEgoType);
}

void SimCloudTopicWriter::FileCacheData(sim_msg::TrafficFrame& trafficFrame, sim_msg::TrafficEgo* pEgo,
                                        sim_msg::TrafficEgo* pLogsimEgo, bool bSwitch) {
  if (m_bRecLoc)  // only record location when receive LOCATION
  {
    pEgo->mutable_loc()->CopyFrom(m_LastLocation);  // copy from last location to avoid unexpected behavior from mutable
                                                    // loc() in the case of a change of location.
  }
  if (m_bLogsimRecLoc) {
    pLogsimEgo->set_type(pEgo->type());
    pLogsimEgo->set_id(ego_id_);
    pLogsimEgo->set_egotype(m_sEgoType);
    pLogsimEgo->mutable_loc()->CopyFrom(m_LogsimLastLocation);
  }
  if (m_bRecTrajectory) { pEgo->mutable_planning_line()->CopyFrom(m_LastTrajectory); }

  if (m_bLogsimRecTrajectory) { pLogsimEgo->mutable_planning_line()->CopyFrom(m_LogsimLastTrajectory); }

  if (m_bRecTraffic) { trafficFrame.mutable_traffic()->CopyFrom(m_LastTraffic); }

  if (m_bLogsimRecTraffic) { trafficFrame.mutable_logsim_traffic()->CopyFrom(m_LogsimLastTraffic); }
}

void SimCloudTopicWriter::WritePb(const std::string& grading_result, uint64_t frame_id, uint32_t sim_time) {
  if (frame_id > 1 && sim_time == 0) return;
  sim_msg::TrafficFrame trafficFrame;
  trafficFrame.set_frame_id(frame_id);
  trafficFrame.set_job_id(job_id_);
  trafficFrame.set_world_id(scenario_id_);
  trafficFrame.set_switch_time(l2w_input_switch_time_);
  trafficFrame.set_real_switch_time(m_l2w_real_switch_time);
  trafficFrame.set_log_time(logsim_start_timestamp_ + sim_time);
  trafficFrame.set_sim_time(sim_time);
  int64_t cur_time = tx_sim::utils::GetWallTimeInMillis();
  trafficFrame.set_sys_time(cur_time);

  std::string sSourceType = from_type_;
  if ("LOG" == from_type_) { sSourceType = "LOG2WORLD"; }
  trafficFrame.set_source(sSourceType);

  sim_msg::TrafficEgo* pEgo = trafficFrame.add_egos();
  pEgo->set_feedback(grading_result);
  pEgo->set_id(ego_id_);
  pEgo->set_egotype(m_sEgoType);

  std::vector<std::uint8_t> payload;
  payload.resize(trafficFrame.ByteSizeLong());
  trafficFrame.SerializeToArray(payload.data(), payload.size());
  std::string sDebug = trafficFrame.DebugString();
  LOG(INFO) << sDebug;
  WriteMsgQueue(payload, "grading feedback");
}

bool SimCloudTopicWriter::WritePb(const StepMessage& msgs, uint64_t frame_id, uint32_t sim_time, bool city,
                                  double lastmiles, uint64_t begin_frame_id) {
  LOG(INFO) << "sim_time:" << sim_time << ", frame_id:" << frame_id;
  if (frame_id > 1 && sim_time == 0 && !city) return false;
  sim_msg::TrafficFrame trafficFrame;
  trafficFrame.set_frame_id(frame_id);
  trafficFrame.set_job_id(job_id_);
  trafficFrame.set_world_id(scenario_id_);
  trafficFrame.set_switch_time(l2w_input_switch_time_);
  trafficFrame.set_real_switch_time(m_l2w_real_switch_time);

  trafficFrame.set_log_time(logsim_start_timestamp_ + sim_time);
  trafficFrame.set_sim_time(sim_time);
  int64_t cur_time = tx_sim::utils::GetWallTimeInMillis();
  trafficFrame.set_sys_time(cur_time);
  trafficFrame.set_begin_frame_id(begin_frame_id);

  std::string sSourceType = from_type_;
  if ("LOG" == from_type_) { sSourceType = "LOG2WORLD"; }
  trafficFrame.set_source(sSourceType);

  bool bSwitch = true;
  if (m_l2w_real_switch_time != -1 && sim_time < m_l2w_real_switch_time) { bSwitch = false; }
  LOG(INFO) << "switch:" << bSwitch << ", m_l2w_real_switch_time:" << m_l2w_real_switch_time;

  sim_msg::TrafficEgo* pEgo = trafficFrame.add_egos();
  sim_msg::TrafficEgo* pLogsimEgo = trafficFrame.add_logsim_egos();
  FillEgoType(pEgo);
  FileCacheData(trafficFrame, pEgo, pLogsimEgo, bSwitch);

  for (const auto& m : msgs.messages) {
    std::string topic = m.first.sTopic;
    if (tx_sim::topic::kTraffic == topic) {
      m_bRecTraffic = true;
      sim_msg::Traffic* pTraffic = trafficFrame.mutable_traffic();
      pTraffic->ParseFromArray(m.second.data(), m.second.size());
      m_LastTraffic.CopyFrom(*pTraffic);
      if (pTraffic->cars_size() > 0) {
        LOG(INFO) << "topic:" << topic << " " << "x:" << pTraffic->cars(0).x() << " " << "y:" << pTraffic->cars(0).y()
                  << "id:" << pTraffic->cars(0).id();
      }
    }
    // LOCATION_REPLAY
    else if ("LOCATION_REPLAY" == topic) {
      m_bLogsimRecLoc = true;
      sim_msg::Location* pLogLocation = pLogsimEgo->mutable_loc();
      pLogLocation->ParseFromArray(m.second.data(), m.second.size());
      m_LogsimLastLocation.CopyFrom(*pLogLocation);
      LOG(INFO) << "topic:" << topic << " " << pLogLocation->DebugString();
    } else if (tx_sim::topic::kLocation == topic)  // 切换后worldsim图层换读LOCATION
    {
      sim_msg::Location* pLocation = pEgo->mutable_loc();
      pLocation->ParseFromArray(m.second.data(), m.second.size());
      m_LastLocation.CopyFrom(*pLocation);
      m_bRecLoc = true;
      LOG(INFO) << "topic:" << topic << " " << pLocation->DebugString();
      LOG(INFO) << "-----------------------------------------------------------------------------------------";
    } else if (tx_sim::topic::kTrajectory == topic) {
      m_bRecTrajectory = true;
      sim_msg::Trajectory* pPlanning_line = pEgo->mutable_planning_line();
      pPlanning_line->ParseFromArray(m.second.data(), m.second.size());
      // note: sample trajectory point for reduce front-end pressure
      SampleTrajectory(pPlanning_line, 3, 5);
      m_LastTrajectory.CopyFrom(*pPlanning_line);
    }
    // TRAJECTORY_REPLAY
    else if ("TRAJECTORY_REPLAY" == topic) {
      m_bLogsimRecTrajectory = true;
      sim_msg::Trajectory* pLog_Planning_line = pLogsimEgo->mutable_planning_line();
      pLog_Planning_line->ParseFromArray(m.second.data(), m.second.size());
      // note: sample trajectory point for reduce front-end pressure
      SampleTrajectory(pLog_Planning_line, 3, 5);
      m_LogsimLastTrajectory.CopyFrom(*pLog_Planning_line);
    } else if (tx_sim::topic::kGrading == topic) {
      sim_msg::Grading* pGrading = trafficFrame.mutable_grading();
      pGrading->ParseFromArray(m.second.data(), m.second.size());
      double miles = lastmiles + pGrading->mileage();
      pGrading->set_mileage(miles);

      pEgo->mutable_grading()->ParseFromArray(m.second.data(), m.second.size());
      pEgo->mutable_grading()->set_mileage(miles);

      LOG(INFO) << "grading miles: " << miles;
    } else if (tx_sim::topic::kPlanStatus == topic) {
    } else if ("LOCATION_TRAILER" == topic) {
      sim_msg::Location* pTrailerLoc = pEgo->add_trailer();
      pTrailerLoc->ParseFromArray(m.second.data(), m.second.size());
    }
    // LOCATION_TRAILER._LOGPLAY_
    else if ("LOCATION_TRAILER" + tx_sim::impl::kLogTopicPostfix == topic) {
      sim_msg::Location* pTrailerLoc = pLogsimEgo->add_trailer();
      pTrailerLoc->ParseFromArray(m.second.data(), m.second.size());
    } else if ("TRAFFIC_REPLAY" == topic) {
      m_bLogsimRecTraffic = true;
      sim_msg::Traffic* pTraffic = trafficFrame.mutable_logsim_traffic();
      pTraffic->ParseFromArray(m.second.data(), m.second.size());
      m_LogsimLastTraffic.CopyFrom(*pTraffic);
      if (pTraffic->cars_size() > 0) {
        LOG(INFO) << "topic:" << topic << " " << "x:" << pTraffic->cars(0).x() << " " << "y:" << pTraffic->cars(0).y()
                  << "id:" << pTraffic->cars(0).id();
      }
    } else if (tx_sim::topic::kControl == topic) {
      sim_msg::Control* pControl = trafficFrame.mutable_control();
      pControl->ParseFromArray(m.second.data(), m.second.size());
    } else if (tx_sim::topic::kVehicleState == topic) {
      sim_msg::VehicleState* pVehState = trafficFrame.mutable_vehicle_state();
      pVehState->ParseFromArray(m.second.data(), m.second.size());
    } else if (tx_sim::topic::kTraffic_billborad == topic) {
      sim_msg::Billboard* pBillboard = trafficFrame.mutable_traffic_billboard();
      pBillboard->ParseFromArray(m.second.data(), m.second.size());
    } else if (tx_sim::topic::kEnviroment == topic) {
      sim_msg::EnvironmentalConditions* pEnvironmentalConditions = trafficFrame.mutable_environmental_conditions();
      pEnvironmentalConditions->ParseFromArray(m.second.data(), m.second.size());
    } else if ("kDebug" == topic) {
      sim_msg::DebugMessage* pDebugMessage = trafficFrame.mutable_debugmessage();
      pDebugMessage->ParseFromArray(m.second.data(), m.second.size());
    } else if (tx_sim::topic::kControl_V2 == topic) {
      sim_msg::Control_V2* pControl_V2 = trafficFrame.mutable_control_v2();
      pControl_V2->ParseFromArray(m.second.data(), m.second.size());
    }
  }
  // std::string sDebug = trafficFrame.DebugString();
  // LOG(INFO)<<"write_times_:"<<write_times_;
  // LOG(INFO)<<sDebug;

  std::vector<std::uint8_t> payload;
  payload.resize(trafficFrame.ByteSizeLong());
  trafficFrame.SerializeToArray(payload.data(), payload.size());
  WriteMsgQueue(payload, "trafficFrame protobuf");
  return true;
}


void SimCloudTopicWriter::SetIds(int32_t job_id, int32_t scenario_id, int32_t ego_id, const std::string& from,
                                 int32_t l2w_input_switch_time, int64_t logsim_start_timestamp,
                                 int32_t l2w_real_switch_time) {
  write_times_ = 0;
  job_id_ = job_id;
  scenario_id_ = scenario_id;
  ego_id_ = ego_id;
  int32_t id_key = job_id + scenario_id + ego_id;
  from_type_ = from;
  if (from_type_ == "EGO")  // from + job id + frame id + version + scene id + ego id + sim_time + server time
    header_length_ = 3 + 4 + 4 + 1 + 4 + 4 + 4 + 8;
  else  // from + job id + frame id + version + scene id + L2W switch time + sim_time + server time + log timestamp
    header_length_ = 3 + 4 + 4 + 1 + 4 + 4 + 4 + 8 + 8;
  l2w_input_switch_time_ = l2w_input_switch_time;
  m_l2w_real_switch_time = l2w_real_switch_time;
  logsim_start_timestamp_ = logsim_start_timestamp;
  std::memcpy(key_id_byte_array_.data(), reinterpret_cast<char*>(&id_key), 4);
  std::stringstream ss;
  ss << "(job_id=" << std::to_string(job_id) << " ego_id=" << std::to_string(ego_id)
     << " scene_id=" << std::to_string(scenario_id) << ")";
  log_key_ = ss.str();
  LOG(INFO) << log_key_ << " reset with FROM type -> " << from_type_ << ", header size -> " << header_length_
            << ", log2world input switch time -> " << l2w_input_switch_time << ", log2world real switch time -> "
            << l2w_real_switch_time << ", logsim start timestamp -> " << logsim_start_timestamp_;
}

void SimCloudTopicWriter::SetEgoType(std::string sType) {
  m_sEgoType = sType;
  LOG(INFO) << "egotype:" << m_sEgoType;
}


void SimCloudTopicWriter::AssembleHeader(std::vector<std::uint8_t>& payload, uint64_t frame_id, uint32_t sim_time) {
  static char ver = TXSIM_CLOUD_MSG_VERSION;
  int64_t cur_time = tx_sim::utils::GetWallTimeInMillis();
  const char* from_chars = from_type_.c_str();
  std::memcpy(payload.data(), from_chars, 3);
  std::memcpy(payload.data() + 3, reinterpret_cast<char*>(&job_id_), 4);
  std::memcpy(payload.data() + 7, reinterpret_cast<char*>(&frame_id), 4);
  std::memcpy(payload.data() + 11, &ver, 1);
  std::memcpy(payload.data() + 12, reinterpret_cast<char*>(&scenario_id_), 4);
  bool is_logsim = (from_type_ == "LOG");
  std::memcpy(payload.data() + 16, reinterpret_cast<char*>(is_logsim ? &m_l2w_real_switch_time : &ego_id_), 4);
  std::memcpy(payload.data() + 20, reinterpret_cast<char*>(&sim_time), 4);
  std::memcpy(payload.data() + 24, reinterpret_cast<char*>(&cur_time), 8);
  if (is_logsim) {
    int64_t log_time = logsim_start_timestamp_ + sim_time;
    std::memcpy(payload.data() + 32, reinterpret_cast<char*>(&log_time), 8);
  }
}


size_t SimCloudTopicWriter::AssembleBody(std::vector<std::uint8_t>& payload, size_t offset, const std::string& msg_type,
                                         const zmq::message_t& msg) {
  std::int32_t msg_size = msg.size(), msg_type_size = msg_type.size();
  std::memcpy(payload.data() + offset, msg_type.data(), msg_type_size);
  std::memcpy(payload.data() + offset + msg_type_size, reinterpret_cast<char*>(&msg_size), 4);
  std::memcpy(payload.data() + offset + msg_type_size + 4, msg.data(), msg_size);
  return offset + msg_type_size + 4 + msg_size;
}


void SimCloudTopicWriter::WriteMsgQueue(std::vector<std::uint8_t>& payload, const std::string& type) {
  if (!producer_) return;
  RdKafka::ErrorCode err =
      producer_->produce(mq_topic_.c_str(), RdKafka::Topic::PARTITION_UA, RdKafka::Producer::RK_MSG_COPY,
                         payload.data(), payload.size(), key_id_byte_array_.data(), key_id_byte_array_.size(), 0, NULL);
  if (err != RdKafka::ERR_NO_ERROR) {
    LOG(INFO) << "write kafka error times " << write_times_ << ",---------------:" << RdKafka::err2str(err);
  }
  if (write_times_ % TXSIM_KAFKA_PRODUCER_POLL_INTERVAL == 0) { producer_->poll(TXSIM_KAFKA_PRODUCER_POLL_TIMEOUT); }
  if ("grading feedback" == type) {
    LOG(INFO) << "write kafka times:" << write_times_;
    while (producer_->outq_len() > 0) {
      LOG(INFO) << producer_->outq_len() << " message(s) were not delivered before flush";
      producer_->flush(10 * 1000);
    }
    LOG(INFO) << "all message(s) were delivered.";
  }
  ++write_times_;
}

}  // namespace coordinator
}  // namespace tx_sim
