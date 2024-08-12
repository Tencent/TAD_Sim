// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "txsim_log_common.h"
#include <stdio.h>
#include <array>

#include "boost/filesystem.hpp"

namespace pblog {
const std::unordered_set<std::string> kEgoTopicList{"TRAJECTORY",
                                                    "LOCATION",
                                                    "LOCATION_TRAILER",
                                                    "GRADING",
                                                    "CONTROL",
                                                    "CONTROL_V2",
                                                    "VEHICLE_STATE",
                                                    "PLAN_STATUS",
                                                    "GRADING_STATISTICS",
                                                    "PARKING_STATE",
                                                    "TXSIM_PARKING_SPACE",
                                                    "L4_ALGORITHM_SIGNAL"};

const std::unordered_set<std::string> kGlobalTopicList{"TRAFFIC",       "LANE_MARKS", "TRAFFIC_BILLBOARD",
                                                       "ENVIRONMENTAL", "PREDICTION", "DEBUG_MESSAGE"};

// create io stream
bool CPBLogCommon::CreateInStream(PBLogInStream &in_stream, const std::string &pblog_file) {
  // 使用 ifstream 类型的对象打开文件，std::ios::in 表示以输入模式（读取）打开文件，std::ios::binary
  // 表示以二进制模式打开文件
  in_stream = std::ifstream(pblog_file, std::ios::in | std::ios::binary);
  if (in_stream.is_open()) {
    // 将文件指针移动到文件的开头
    in_stream.seekg(0, std::ios::beg);
  }

  return IsInStreamValid(in_stream);
}

bool CPBLogCommon::createOutStream(PBLogOutStream &out_stream, const std::string &pblog_file) {
  // 使用 ofstream 类型的对象打开文件，std::ios::out
  // 表示以输出模式（写入）打开文件，std::ios::binary表示以二进制模式打开文件
  out_stream = std::ofstream(pblog_file, std::ios::out | std::ios::binary);

  return IsOutStreamValid(out_stream);
}

// close io stream
bool CPBLogCommon::CloseInStream(PBLogInStream &in_stream) {
  // 如果流已经关闭了，则返回true；否则调用close函数关闭流并返回false
  if (IsInStreamValid(in_stream)) {
    in_stream.close();
  }

  return true;
}

bool CPBLogCommon::CloseOutStream(PBLogOutStream &out_stream) {
  // 如果流已经关闭了，则返回true；否则调用close函数关闭流并返回false
  if (IsOutStreamValid(out_stream)) {
    out_stream.flush();
    out_stream.close();
  }

  return true;
}

// split string
void CPBLogCommon::StringSplit(const std::string strIn, const std::string &spliter,
                               std::vector<std::string> &splited_strs) {
  splited_strs.clear();

  // 定义两个指针，初始值pos1为开头，pos2为第一个分隔符的位置
  std::string::size_type pos1 = 0, pos2 = strIn.find(spliter);

  // 当找到分隔符时，执行循环
  while (std::string::npos != pos2) {
    splited_strs.push_back(strIn.substr(pos1, pos2 - pos1));

    // 更新pos1和pos2的值，以便搜索下一个子字符串
    pos1 = pos2 + spliter.size();
    pos2 = strIn.find(spliter, pos1);
  }
  // 如果没有找到分隔符，将剩余的字符串添加到vector中
  if (pos1 != strIn.length()) splited_strs.push_back(strIn.substr(pos1));
}

// return initialized struct
PBLogEvent CPBLogCommon::GetInitLogEvent() {
  pblog::PBLogEvent event;

  // 初始化 event 结构体的成员变量
  event.m_channel = "";
  event.m_channelStringSize = 0;
  event.m_payload = "";
  event.m_payloadSize = 0;
  event.m_timestamp = 0;

  return event;
}

bool CPBLogCommon::ReadEventFromStream(PBLogEvent &event, PBLogInStream &ptr_instream) {
  bool eventReadState = false;

  event = CPBLogCommon::GetInitLogEvent();

  if (CPBLogCommon::IsInStreamValid(ptr_instream)) {
    char chnLen[BYTE_SIZE_4], pbMsgLen[BYTE_SIZE_4], time[BYTE_SIZE_8];
    char *ptrChn = nullptr, *ptrPbMsg = nullptr;

    // read channel length, protobuf msg length, timestamp
    ptr_instream.read(chnLen, BYTE_SIZE_4);
    ptr_instream.read(pbMsgLen, BYTE_SIZE_4);
    ptr_instream.read(time, BYTE_SIZE_8);

    if (!CPBLogCommon::IsInStreamValid(ptr_instream)) {
      LOG(INFO) << "player | input stream invalid, may reach end of replay.\n";
      return false;
    }

    // bytes to uint32/long
    // 将chnLen的值复制到event.m_channelStringSize中
    memcpy(&event.m_channelStringSize, chnLen, BYTE_SIZE_4);
    // 将pbMsgLen的值复制到event.m_payloadSize中
    memcpy(&event.m_payloadSize, pbMsgLen, BYTE_SIZE_4);
    // 将time的值复制到event.m_timestamp中
    memcpy(&event.m_timestamp, time, BYTE_SIZE_8);

    // read channel name and protobuf msg
    ptrChn = new char[event.m_channelStringSize];
    ptrPbMsg = new char[event.m_payloadSize];

    ptr_instream.read(ptrChn, event.m_channelStringSize);
    ptr_instream.read(ptrPbMsg, event.m_payloadSize);

    // copy channel and protobuf string byte by byte
    for (auto i = 0; i < event.m_channelStringSize; ++i) {
      event.m_channel.push_back(ptrChn[i]);
    }
    for (auto i = 0; i < event.m_payloadSize; ++i) {
      event.m_payload.push_back(ptrPbMsg[i]);
    }

    delete[] ptrChn;
    delete[] ptrPbMsg;

    if (CPBLogCommon::IsInStreamValid(ptr_instream) && event.m_channelStringSize > 0) {
      eventReadState = true;
    }
  }

  return eventReadState;
}

// convert from pb to pb log event
void CPBLogCommon::PB2Event(PBLogEvent &event, const std::string &serializeMSG, const std::string &channel,
                            const int64_t &tIn) {
  event.m_channel = channel;
  event.m_channelStringSize = (uint32_t)channel.length();
  event.m_timestamp = tIn;

  event.m_payloadSize = (uint32_t)serializeMSG.length();
  event.m_payload = serializeMSG;
}

// write an pb log event to stream
void CPBLogCommon::WriteEvent2Stream(PBLogEvent &event, PBLogOutStream &ptr_outsream) {
  // 定义相应字节大小的字符数组
  char channelSizeBuf[BYTE_SIZE_4], pbSizeBuf[BYTE_SIZE_4], timeBuf[BYTE_SIZE_8];

  // 将字符数组初始化为0
  memset(channelSizeBuf, 0x00, BYTE_SIZE_4);
  memset(pbSizeBuf, 0x00, BYTE_SIZE_4);
  memset(timeBuf, 0x00, BYTE_SIZE_8);

  // 将PBLogEvent类中的成员变量复制到字符数组中
  memcpy(channelSizeBuf, &event.m_channelStringSize, BYTE_SIZE_4);
  memcpy(pbSizeBuf, &event.m_payloadSize, BYTE_SIZE_4);
  memcpy(timeBuf, &event.m_timestamp, BYTE_SIZE_8);

  // 先写入相应的size值
  ptr_outsream.write(channelSizeBuf, BYTE_SIZE_4);
  ptr_outsream.write(pbSizeBuf, BYTE_SIZE_4);
  ptr_outsream.write(timeBuf, BYTE_SIZE_8);

  // 再写入具体内容
  ptr_outsream.write(event.m_channel.c_str(), event.m_channelStringSize);
  ptr_outsream.write(event.m_payload.c_str(), event.m_payloadSize);
}

bool CPBLogCommon::IsInStreamValid(const PBLogInStream &ptr_instream) {
  try {
    // 检查输入流是否打开且无错误
    return !ptr_instream.eof() && !ptr_instream.fail();
  } catch (std::exception &e) {
    // 如果遇到异常，记录错误日志并返回false
    LOG(ERROR) << "player | error while checking input stream " << e.what() << "\n";
    return false;
  }

  return false;
}

bool CPBLogCommon::IsOutStreamValid(const PBLogOutStream &ptr_outstream) {
  try {
    // 检查输入流是否打开且无错误
    return ptr_outstream.is_open() && ptr_outstream.good();
  } catch (std::exception &e) {
    // 如果遇到异常，记录错误日志并返回false
    LOG(ERROR) << "logger | error while checking output stream " << e.what() << "\n";
    return false;
  }

  return false;
}

// generate pblog file name
void CPBLogCommon::GenPBLogFileName(std::string &pblog_file_name) {
  // get system time and use it as protobuf log file name
  time_t time_now = time(0);

  // 获取当前时间的 tm 结构体
  tm *gm_tm = gmtime(&time_now);

  pblog_file_name = "";

  // 根据当前时间生成日志文件名
  // 日志文件名格式为：年_月_日_时_分_秒.pblog
  pblog_file_name += std::to_string(gm_tm->tm_year + 1900) + "_" + std::to_string(gm_tm->tm_mon + 1) + "_" +
                     std::to_string(gm_tm->tm_mday) + "_" + std::to_string(gm_tm->tm_hour + 8) + "_" +
                     std::to_string(gm_tm->tm_min) + "_" + std::to_string(gm_tm->tm_sec) + ".pblog";
}

std::string CPBLogCommon::GetSceneName(const std::string &scene_full_path) {
  std::string scene_name = "";

  // 将给定的完整路径字符串转换为 Boost 文件系统路径对象
  boost::filesystem::path path(scene_full_path);

  // 从路径对象中提取文件名（不包括目录部分），并将其转换为字符串形式
  scene_name = path.filename().string();

  return scene_name;
}

std::string CPBLogCommon::ExecCommand(const std::string cmd, int &out_exit_status) {
  out_exit_status = 0;
  // 声明一个文件指针
  FILE *pPipe = nullptr;

#ifdef _WIN32
  // 如果是Windows系统，使用_popen函数打开一个管道
  pPipe = _popen(cmd.c_str(), "r");
#else
  // 如果是Linux系统，使用popen函数打开一个管道
  pPipe = popen(cmd.c_str(), "r");
#endif  // _WIN32

  if (pPipe == nullptr) {
    // 抛出一个运行时错误异常，提示无法打开管道
    throw std::runtime_error("Cannot open pipe");
  }

  // 用于缓存从管道中读取的数据
  std::array<char, 256> buffer;

  // 用于存储从管道中读取的数据
  std::string result;

  // 循环来读取管道中的数据，直到到达文件末尾
  while (!std::feof(pPipe)) {
    auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
    result.append(buffer.data(), bytes);
  }
#ifdef _WIN32
  // 如果是Windows系统，使用_pclose函数打开一个管道
  out_exit_status = _pclose(pPipe);
#else
  // 如果是Linux系统，使用pclose函数打开一个管道
  out_exit_status = pclose(pPipe);
#endif

  return result;
}

// time convertion
int64_t CPBLogCommon::secondToMillisecond(const int64_t second) { return second * 1000; }
int64_t CPBLogCommon::millisecondToSecond(const int64_t milli) { return milli / 1000; }
int64_t CPBLogCommon::millisecondToMicrosecond(const int64_t milli) { return milli * 1000; }
int64_t CPBLogCommon::microsecondToMillisecond(const int64_t micro) { return micro / 1000; }

}  // namespace pblog
