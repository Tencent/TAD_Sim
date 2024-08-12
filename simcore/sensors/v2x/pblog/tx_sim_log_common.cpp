/**
 * @file txSimLogCommon.cpp
 * @author kekesong
 * @brief
 * @version 0.1
 * @date 2024-03-26
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "tx_sim_log_common.h"
#include <stdio.h>
#include <array>

#include "boost/filesystem.hpp"

namespace pblog {
/**
 * @brief create in stream
 *
 * @param in_stream in stream
 * @param pblog_file path of pblog file
 * @return true on success
 * @return false
 */
bool CPBLogCommon::CreateInStream(PBLogInStream &in_stream, const std::string &pblog_file) {
  in_stream = std::ifstream(pblog_file, std::ios::in | std::ios::binary);
  if (in_stream.is_open()) {
    in_stream.seekg(0, std::ios::beg);
  }
  return IsInStreamValid(in_stream);
}

/**
 * @brief create out stream
 *
 * @param out_stream out stream
 * @param pblog_file path of pblog file
 * @return true on success
 * @return false
 */
bool CPBLogCommon::createOutStream(PBLogOutStream &out_stream, const std::string &pblog_file) {
  out_stream = std::ofstream(pblog_file, std::ios::out | std::ios::binary);
  return IsOutStreamValid(out_stream);
}

/**
 * @brief close in stream
 *
 * @param in_stream  in stream
 * @return true on success
 * @return false
 */
bool CPBLogCommon::CloseInStream(PBLogInStream &in_stream) {
  if (IsInStreamValid(in_stream)) {
    in_stream.close();
  }
  return true;
}

/**
 * @brief close out stream
 *
 * @param out_stream  out stream
 * @return true on success
 * @return false
 */
bool CPBLogCommon::CloseOutStream(PBLogOutStream &out_stream) {
  if (IsOutStreamValid(out_stream)) {
    out_stream.flush();
    out_stream.close();
  }
  return true;
}

/**
 * @brief split string
 *
 * @param strIn input string
 * @param spliter spliter
 * @param splited_strs out of splite lists
 */
void CPBLogCommon::StringSplit(const std::string strIn, const std::string &spliter,
                               std::vector<std::string> &splited_strs) {
  splited_strs.clear();
  std::string::size_type pos1 = 0, pos2 = strIn.find(spliter);
  while (std::string::npos != pos2) {
    splited_strs.push_back(strIn.substr(pos1, pos2 - pos1));
    pos1 = pos2 + spliter.size();
    pos2 = strIn.find(spliter, pos1);
  }
  if (pos1 != strIn.length()) splited_strs.push_back(strIn.substr(pos1));
}

/**
 * @brief get init event
 *
 * @return PBLogEvent initialized struct
 */
PBLogEvent CPBLogCommon::GetInitLogEvent() {
  pblog::PBLogEvent event;
  event.channel = "";
  event.chn_str_size = 0;
  event.pb_msg_str = "";
  event.pb_str_size = 0;
  event.timestamp = 0;
  return event;
}

/**
 * @brief read event from stream
 *
 * @param event output event
 * @param ptr_instream in stream
 * @return true on success
 * @return false
 */
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
    memcpy(&event.chn_str_size, chnLen, BYTE_SIZE_4);
    memcpy(&event.pb_str_size, pbMsgLen, BYTE_SIZE_4);
    memcpy(&event.timestamp, time, BYTE_SIZE_8);

    // read channel name and protobuf msg
    ptrChn = new char[event.chn_str_size];
    ptrPbMsg = new char[event.pb_str_size];

    ptr_instream.read(ptrChn, event.chn_str_size);
    ptr_instream.read(ptrPbMsg, event.pb_str_size);

    // copy channel and protobuf string byte by byte
    for (auto i = 0; i < event.chn_str_size; ++i) {
      event.channel.push_back(ptrChn[i]);
    }
    for (auto i = 0; i < event.pb_str_size; ++i) {
      event.pb_msg_str.push_back(ptrPbMsg[i]);
    }

    delete[] ptrChn;
    delete[] ptrPbMsg;

    if (CPBLogCommon::IsInStreamValid(ptr_instream) && event.chn_str_size > 0) {
      eventReadState = true;
    }
  }
  return eventReadState;
}

/**
 * @brief convert from pb to pb log event
 *
 * @param event out envent
 * @param serializeMSG serialize message
 * @param channel channel
 * @param tIn timestamp
 */
void CPBLogCommon::PB2Event(PBLogEvent &event, const std::string &serializeMSG, const std::string &channel,
                            const int64_t &tIn) {
  event.channel = channel;
  event.chn_str_size = (uint32_t)channel.length();
  event.timestamp = tIn;

  event.pb_str_size = (uint32_t)serializeMSG.length();
  event.pb_msg_str = serializeMSG;
}

/**
 * @brief write an pb log event to stream
 *
 * @param event out envent
 * @param ptr_outsream out stream
 * @param bJson whether to write as json
 */
void CPBLogCommon::WriteEvent2Stream(PBLogEvent &event, PBLogOutStream &ptr_outsream, bool bJson) {
  if (bJson) {
    // save as json
    int64_t time = event.timestamp / 1000;
    std::string sTime = "simtime:" + std::to_string(time);
    ptr_outsream.write(sTime.c_str(), sTime.length());
    ptr_outsream << "\n";
    std::string sChannel = "topic:" + event.channel;
    ptr_outsream.write(sChannel.c_str(), sChannel.length());
    ptr_outsream << "\n";
    ptr_outsream.write(event.pb_msg_str.c_str(), event.pb_str_size);
  } else {
    // save as binary
    char channelSizeBuf[BYTE_SIZE_4], pbSizeBuf[BYTE_SIZE_4], timeBuf[BYTE_SIZE_8];

    memset(channelSizeBuf, 0x00, BYTE_SIZE_4);
    memset(pbSizeBuf, 0x00, BYTE_SIZE_4);
    memset(timeBuf, 0x00, BYTE_SIZE_8);

    memcpy(channelSizeBuf, &event.chn_str_size, BYTE_SIZE_4);
    memcpy(pbSizeBuf, &event.pb_str_size, BYTE_SIZE_4);
    memcpy(timeBuf, &event.timestamp, BYTE_SIZE_8);

    ptr_outsream.write(channelSizeBuf, BYTE_SIZE_4);
    ptr_outsream.write(pbSizeBuf, BYTE_SIZE_4);
    ptr_outsream.write(timeBuf, BYTE_SIZE_8);

    ptr_outsream.write(event.channel.c_str(), event.chn_str_size);
    ptr_outsream.write(event.pb_msg_str.c_str(), event.pb_str_size);
  }
}

/**
 * @brief judge the in stream is valid or not
 *
 * @param ptr_instream in stream
 * @return true on valid
 * @return false on invalid
 */
bool CPBLogCommon::IsInStreamValid(const PBLogInStream &ptr_instream) {
  try {
    return !ptr_instream.eof() && !ptr_instream.fail();
  } catch (std::exception &e) {
    LOG(ERROR) << "player | error while checking input stream " << e.what() << "\n";
    return false;
  }
  return false;
}

/**
 * @brief judge the out stream is valid or not
 *
 * @param ptr_outstream out stream
 * @return true
 * @return false
 */
bool CPBLogCommon::IsOutStreamValid(const PBLogOutStream &ptr_outstream) {
  try {
    return ptr_outstream.is_open() && ptr_outstream.good();
  } catch (std::exception &e) {
    LOG(ERROR) << "logger | error while checking output stream " << e.what() << "\n";
    return false;
  }
  return false;
}

/**
 * @brief generate pblog file name
 *
 * @param pblog_file_name out file path
 */
void CPBLogCommon::GenPBLogFileName(std::string &pblog_file_name) {
  // get system time and use it as protobuf log file name
  time_t time_now = time(0);
  tm *gm_tm = gmtime(&time_now);
  pblog_file_name = "";
  pblog_file_name += std::to_string(gm_tm->tm_year + 1900) + "_" + std::to_string(gm_tm->tm_mon + 1) + "_" +
                     std::to_string(gm_tm->tm_mday) + "_" + std::to_string(gm_tm->tm_hour + 8) + "_" +
                     std::to_string(gm_tm->tm_min) + "_" + std::to_string(gm_tm->tm_sec) + ".asn.pblog";
}

/**
 * @brief get scene name
 *
 * @param scene_full_path full path of scene
 * @return std::string of scene name
 */
std::string CPBLogCommon::GetSceneName(const std::string &scene_full_path) {
  std::string scene_name = "";
  boost::filesystem::path path(scene_full_path);
  scene_name = path.filename().string();
  return scene_name;
}

/**
 * @brief execute command
 *
 * @param cmd
 * @param out_exit_status out exit status
 * @return std::string of result
 */
std::string CPBLogCommon::ExecCommand(const std::string cmd, int &out_exit_status) {
  out_exit_status = 0;
  FILE *pPipe = nullptr;

#ifdef _WIN32
  pPipe = _popen(cmd.c_str(), "r");
#else
  pPipe = popen(cmd.c_str(), "r");
#endif  // _WIN32

  if (pPipe == nullptr) {
    throw std::runtime_error("Cannot open pipe");
  }

  std::array<char, 256> buffer;

  std::string result;

  while (!std::feof(pPipe)) {
    auto bytes = std::fread(buffer.data(), 1, buffer.size(), pPipe);
    result.append(buffer.data(), bytes);
  }
#ifdef _WIN32
  out_exit_status = _pclose(pPipe);
#else
  out_exit_status = pclose(pPipe);
#endif

  return result;
}

/**
 * @brief second to millisecond
 *
 * @param second
 * @return int64_t of milli second
 */
int64_t CPBLogCommon::sec_2_milli(const int64_t second) { return second * 1000; }
int64_t CPBLogCommon::mili_2_sec(const int64_t milli) { return milli / 100; }
int64_t CPBLogCommon::milli_2_micro(const int64_t milli) { return milli * 1000; }
int64_t CPBLogCommon::micro_2_milli(const int64_t micro) { return micro / 1000; }

}  // namespace pblog
