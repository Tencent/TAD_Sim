/**
 * @file txSimLogCommon.h
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>

#ifdef _WIN32
#  define GLOG_NO_ABBREVIATED_SEVERITIES
#endif
#include "glog/logging.h"
#define Log_Switch 0

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/time.h>
#endif

#ifdef _Win32
typedef int64 int64_t;
#endif  // _WIN32

#define BYTE_SIZE_4 4
#define BYTE_SIZE_8 8

namespace pblog {

struct PBLogEvent {
  uint32_t chn_str_size = 0;
  uint32_t pb_str_size = 0;
  int64_t timestamp = 0;  // microsecond, not millisecond
  std::string channel;
  std::string pb_msg_str;
};

// typedef std::unique_ptr<ifstream> PBLogInStream;
// typedef std::unique_ptr<ofstream> PBLogOutStream;

typedef std::ifstream PBLogInStream;
typedef std::ofstream PBLogOutStream;

class CPBLogCommon {
 public:
  // return initialized struct
  static PBLogEvent GetInitLogEvent();

  // create io stream
  static bool CreateInStream(PBLogInStream &in_stream, const std::string &pblog_file);
  static bool createOutStream(PBLogOutStream &out_stream, const std::string &pblog_file);

  // close io stream
  static bool CloseInStream(PBLogInStream &in_stream);
  static bool CloseOutStream(PBLogOutStream &out_stream);

  // split string
  static void StringSplit(const std::string str_in, const std::string &spliter, std::vector<std::string> &splited_strs);

  // read event from stream
  static bool ReadEventFromStream(PBLogEvent &event, PBLogInStream &ptr_instream);

  // convert from pb to pb log event
  static void PB2Event(PBLogEvent &event, const std::string &serialize_msg, const std::string &channel,
                       const int64_t &t_in);

  // write an pb log event to stream
  static void WriteEvent2Stream(PBLogEvent &event, PBLogOutStream &ptr_outstream, bool bJson = true);

  // check io stream
  static bool IsInStreamValid(const PBLogInStream &ptr_instream);
  static bool IsOutStreamValid(const PBLogOutStream &ptr_outstream);

  // generate pblog file name
  static void GenPBLogFileName(std::string &pblog_file);
  static std::string GetSceneName(const std::string &scene_full_path);

  // time convertion
  static int64_t sec_2_milli(const int64_t second);
  static int64_t mili_2_sec(const int64_t milli);

  static int64_t milli_2_micro(const int64_t milli);
  static int64_t micro_2_milli(const int64_t micro);

  static std::string ExecCommand(const std::string cmd, int &out_exit_status);
};
}  // namespace pblog
