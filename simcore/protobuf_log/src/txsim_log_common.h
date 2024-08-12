// Copyright 2024 Tencent Inc. All rights reserved.
/**
 * @file txSimLogCommon.h
 * @author DongYuanHu (dongyuanhu@tencent.com)
 * @brief common utils for protobuf logger and player
 * @version 0.1
 * @date 2023-07-05
 *
 *
 */

#pragma once
#include <cstdint>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <unordered_set>

#ifdef _WIN32
#  define GLOG_NO_ABBREVIATED_SEVERITIES
#endif
#include "glog/logging.h"
#define Log_Switch 1

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

#define BYTE_SIZE_4 4
#define BYTE_SIZE_8 8

namespace pblog {

// 定义一个名为 PBLogEvent 的结构体, 作为每个消息的基本单元
struct PBLogEvent {
  // 声明一个 32 位无符号整数 m_channelStringSize，用于存储topic名称的大小
  uint32_t m_channelStringSize = 0;

  // 声明一个 32 位无符号整数 m_payloadSize，用于存储这条消息内容的大小
  uint32_t m_payloadSize = 0;

  // 声明一个 64 位有符号整数 m_timestamp，用于存储事件发生的时间戳
  int64_t m_timestamp = 0;  // microsecond, not millisecond

  // 声明一个字符串 m_channel，用于存储topic名称
  std::string m_channel;

  // 声明一个字符串 m_payload，用于存储这条消息内容
  std::string m_payload;
};

// typedef std::unique_ptr<ifstream> PBLogInStream;
// typedef std::unique_ptr<ofstream> PBLogOutStream;

typedef std::ifstream PBLogInStream;
typedef std::ofstream PBLogOutStream;

// internal ego topic list, we handle it by adding "EgoUnion" or "Ego_xxx" prefix
extern const std::unordered_set<std::string> kEgoTopicList;
// internal global topic list, we handle it without adding prefix
extern const std::unordered_set<std::string> kGlobalTopicList;

/**
 * @brief Logger Common Utilities, including some common operations about std::fstream and how to read and write
 * log files.
 */
class CPBLogCommon {
 public:
  // return initialized struct
  static PBLogEvent GetInitLogEvent();

  // create input stream
  static bool CreateInStream(PBLogInStream &in_stream, const std::string &pblog_file);
  // create output stream
  static bool createOutStream(PBLogOutStream &out_stream, const std::string &pblog_file);

  // close input stream
  static bool CloseInStream(PBLogInStream &in_stream);
  // close output stream
  static bool CloseOutStream(PBLogOutStream &out_stream);

  // split string
  static void StringSplit(const std::string str_in, const std::string &spliter, std::vector<std::string> &splited_strs);

  // read event from stream
  static bool ReadEventFromStream(PBLogEvent &event, PBLogInStream &ptr_instream);

  // convert from pb to pb log event
  static void PB2Event(PBLogEvent &event, const std::string &serialize_msg, const std::string &channel,
                       const int64_t &t_in);

  // write an pb log event to stream
  static void WriteEvent2Stream(PBLogEvent &event, PBLogOutStream &ptr_outstream);

  // check input stream
  static bool IsInStreamValid(const PBLogInStream &ptr_instream);
  // check output stream
  static bool IsOutStreamValid(const PBLogOutStream &ptr_outstream);

  // generate pblog file name
  static void GenPBLogFileName(std::string &pblog_file);
  // get scene name
  static std::string GetSceneName(const std::string &scene_full_path);

  // time convertion, second to millisecond
  static int64_t secondToMillisecond(const int64_t second);
  // time convertion, millisecond to second
  static int64_t millisecondToSecond(const int64_t milli);

  // time convertion, millisecond to microsecond
  static int64_t millisecondToMicrosecond(const int64_t milli);
  // time convertion, microsecond to millisecond
  static int64_t microsecondToMillisecond(const int64_t micro);

  // execute system commands
  static std::string ExecCommand(const std::string cmd, int &out_exit_status);
};
}  // namespace pblog
