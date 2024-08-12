// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <sys/stat.h>

#include <codecvt>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <locale>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"
#include "common/log/system_logger.h"
#include "common/utils/scoped_cleanup.h"
#include "engine/error_code.h"

#ifdef _MSC_VER
enum { IS__MSC_VER = 1 };
#else
enum { IS__MSC_VER = 0 };
#endif

//////////////////////////////////
// global, for json
namespace misc {

inline int ParseJson(const char* str, Json::Value* root, const std::string& tip = {}) {
  if (!str || !root) {
    return 0;
  }
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  std::unique_ptr<Json::CharReader> const reader(builder.newCharReader());
  JSONCPP_STRING err;
  try {
    if (!reader->parse(str, str + std::strlen(str), root, &err)) {
      if (!tip.empty()) {
        SYSTEM_LOGGER_ERROR("%s: parse json failed, err: %s, raw: %s", tip.c_str(), err.c_str(), str);
      }
      return -1;
    }
  } catch (...) {
    return -2;
  }
  return 0;
}

inline int WriteJson(const Json::Value* root, std::string* str, const std::string& tip = {}) {
  if (!root || !str) {
    return 0;
  }
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  std::unique_ptr<Json::StreamWriter> const writer(builder.newStreamWriter());
  std::ostringstream oss;
  try {
    writer->write(*root, &oss);
  } catch (const std::exception& ex) {
    SYSTEM_LOGGER_ERROR("%s: write json failed, err: %s", tip.c_str(), ex.what());
    return -1;
  }
  *str = oss.str();
  return 0;
}

inline int ParseJson(const std::string& str, Json::Value& root, const std::string& tip = {}) {
  return ParseJson(str.c_str(), &root, tip);
}
inline int WriteJson(const Json::Value& root, std::string& str, const std::string& tip = {}) {
  return WriteJson(&root, &str, tip);
}

template <typename Input, typename Output>
inline Output XsdGetOptional(const Input& input, Output default_value) {
  if (input.present()) {
    return input.get();
  }
  return default_value;
}

inline std::vector<std::string> TravelDir(const std::string& dir, bool recursive = true,
                                          const std::string& filter = ".*") {
  boost::filesystem::path _dir(dir);
  if (!boost::filesystem::exists(_dir) || !boost::filesystem::is_directory(_dir)) {
    return {};
  }

  std::vector<std::string> files;

  auto loop_and_select_file = [&](auto iter) {
    const std::regex re(filter);
    for (auto& one : iter) {
      if (!boost::filesystem::is_regular_file(one)) {
        continue;
      }
      if (!std::regex_search(one.path().string(), re)) {
        continue;
      }
      files.emplace_back(one.path().string());
    }
  };

  try {
    if (recursive) {
      loop_and_select_file(boost::filesystem::recursive_directory_iterator(_dir));
    } else {
      loop_and_select_file(boost::filesystem::directory_iterator(_dir));
    }
  } catch (const std::exception& ex) {
  }

  return files;
}

}  // namespace misc

inline std::string& operator<<(std::string& str, const Json::Value& root) {
  misc::WriteJson(&root, &str);
  return str;
}

inline Json::Value& operator<<(Json::Value& root, const char* str) {
  root.clear();
  misc::ParseJson(str, &root);
  return root;
}

inline Json::Value& operator<<(Json::Value& root, const std::string& str) {
  root << str.c_str();
  return root;
}

inline std::string JsonToString(const Json::Value& json) {
  std::string data;
  data << json;
  return std::move(data);
}

inline Json::Value StringToJson(const char* data) {
  Json::Value json;
  json << data;
  return std::move(json);
}
inline Json::Value StringToJson(const std::string& data) { return StringToJson(data.c_str()); }
//////////////////////////////////
template <typename T>
T concatenate(T v) {
  return v;
}

template <typename T, typename... Args>
T concatenate(T first, Args... args) {
  return first + concatenate(args...);
}

#define VERSION_NAME_DELIM "#"

inline std::string computeMapCompleteName(const std::string& mapN, const std::string& version) {
  if (version == "") return mapN;
  return version + VERSION_NAME_DELIM + mapN;
}

inline std::string ComputeMapCompleteName(const std::string& mapN, const std::string& version) {
  if (version == "") return mapN;
  return version + VERSION_NAME_DELIM + mapN;
}

inline std::string ComputeMapCompleteName(const std::string& map_name, const int version) {
  return ComputeMapCompleteName(map_name, std::to_string(version));
}

inline std::wstring MakeRspWithCodeMessageData(int code = 0, const std::string& message = "ok",
                                               const Json::Value& data = {}) {
  Json::Value rsp;
  rsp["code"] = code;
  rsp["message"] = message;
  rsp["data"] = data;

  std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
  return converter.from_bytes(JsonToString(rsp));
}

inline std::string GenMapEditorCosKeyByVersion(const std::string& name, const std::string& version) {
  char key[256] = {0};
  snprintf(key, sizeof(key), "maps/editor/%s/%s", version.c_str(), name.c_str());
  return key;
}

inline std::string GenMapEditorJsonKeyByVersion(const std::string& name, const std::string& version) {
  return GenMapEditorCosKeyByVersion(name, version) + ".json";
}

inline std::string GenSceneEditorCosKeyByVersion(const std::string& version) {
  char key[256] = {0};
  snprintf(key, sizeof(key), "scenes/editor/%s/", version.c_str());
  return key;
}

inline bool GetFileSize(const std::string& fname, uint64_t* size) {
  // TRACE_EVENT1("io", "PosixEnv::GetFileSize", "path", fname);
  // MAYBE_RETURN_EIO(fname, IOError(Env::kInjectedFailureStatusMsg, EIO));
  // ThreadRestrictions::AssertIOAllowed();
  bool s = true;
  struct stat sbuf;
  if (stat(fname.c_str(), &sbuf) != 0) {
    // s = IOError(fname, errno);
    s = false;
  } else {
    *size = sbuf.st_size;
  }
  return s;
}

inline int checkEmptyFile(const std::string& strMapFile) {
  uint64_t sz;
  bool gf = GetFileSize(strMapFile, &sz);
  if (!gf) {
    SYSTEM_LOGGER_ERROR("IO error when get file size for %s", strMapFile);
    return HSEC_Error;
  }

  if (sz <= 0) {
    SYSTEM_LOGGER_ERROR("map file is empty , skip it here.");
    return HSEC_Error;
  }
  return HSEC_OK;
}

// First try with a small fixed size buffer
inline void StringAppendV(std::string* dst, const char* format, va_list ap) {
  static const int kSpaceLength = 1024;
  char space[kSpaceLength];

  // It's possible for methods that use a va_list to invalidate
  // the data in it upon use.  The fix is to make a copy
  // of the structure before using it and use that copy instead.
  va_list backup_ap;
  va_copy(backup_ap, ap);
  int result = vsnprintf(space, kSpaceLength, format, backup_ap);
  va_end(backup_ap);

  if (result < kSpaceLength) {
    if (result >= 0) {
      // Normal case -- everything fit.
      dst->append(space, result);
      return;
    }

    if (IS__MSC_VER) {
      // Error or MSVC running out of space.  MSVC 8.0 and higher
      // can be asked about space needed with the special idiom below:
      va_copy(backup_ap, ap);
      result = vsnprintf(nullptr, 0, format, backup_ap);
      va_end(backup_ap);
    }

    if (result < 0) {
      // Just an error.
      return;
    }
  }

  // Increase the buffer size to the size requested by vsnprintf,
  // plus one for the closing \0.
  int length = result + 1;
  auto buf = new char[length];

  // Restore the va_list before we use it again
  va_copy(backup_ap, ap);
  result = vsnprintf(buf, length, format, backup_ap);
  va_end(backup_ap);

  if (result >= 0 && result < length) {
    // It fit
    dst->append(buf, result);
  }
  delete[] buf;
}

inline std::string StringPrintf(const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  std::string result;
  StringAppendV(&result, format, ap);
  va_end(ap);
  return result;
}

inline std::string formatValue(const Json::Value& value) {
  if (value.isNull()) return "";
  if (value.isInt()) {
    return std::to_string(value.asInt());
  } else if (value.isString()) {
    return value.asString();
  } else if (value.isDouble()) {
    return std::to_string(value.asDouble());
  } else if (value.isBool()) {
    return value.asBool() ? "true" : "false";
  }
  return "";
}

inline std::string formatJsonValue(const Json::Value& value) {
  if (value.isInt()) {
    return std::to_string(value.asInt());
  } else if (value.isInt64()) {
    return std::to_string(value.asInt64());
  } else if (value.isString()) {
    return value.asString();
  } else if (value.isDouble()) {
    return std::to_string(value.asDouble());
  } else if (value.isBool()) {
    return std::to_string(value.asBool());
  }
  return "";
}

inline std::string StringifyJsonValue(const Json::Value& value) {
  return value.isConvertibleTo(Json::stringValue) ? value.asString() : "";
}
