/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/utils/stringhelper.h"

namespace utils {

void TrimInplace(std::string& s) {
  if (s.empty()) {
    return;
  }
  s.erase(0, s.find_first_not_of(' '));
  s.erase(s.find_last_not_of(' ') + 1);
}

std::string Trim(const std::string& s) {
  std::string res = s;
  TrimInplace(res);
  return res;
}

std::string TrimPrefix(const std::string& s, const std::string& prefix) {
  if (s.length() < prefix.length() || s.substr(0, prefix.length()) != prefix) {
    return s;
  }
  return s.substr(prefix.length());
}

std::string TrimSuffix(const std::string& s, const std::string& suffix) {
  if (s.length() < suffix.length() || s.substr(s.length() - suffix.length()) != suffix) {
    return s;
  }
  return s.substr(0, s.length() - suffix.length());
}

std::string FromatNumberToString(int num, std::string suffix) {
  std::ostringstream oss;
  oss << suffix;
  oss << std::setw(3) << std::setfill('0') << num;
  return oss.str();
}

std::string FloatPrecisionToString(float num, int precision) {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(precision) << num;
  return oss.str();
}

bool IsStartWith(const std::string& s, const std::string& prefix) {
  if (s.length() >= prefix.length() && s.substr(0, prefix.length()).compare(prefix) == 0) {
    return true;
  }
  return false;
}
std::string MergeStringAddLink(std::vector<std::string> strings, std::string linkstr) {
  std::string result = "";
  if (strings.size() < 1) return result;
  result = strings.at(0);
  for (auto it = strings.begin() + 1; it != strings.end(); it++) {
    result += (";" + (*it));
  }
  return result;
}
std::unordered_map<std::string, std::string> SplitToMap(const std::string& in, char separator1, char separator2,
                                                        bool trim) {
  std::unordered_map<std::string, std::string> res;
  // 最外层分割
  auto sub_str_vec = Split(in, separator1);
  // 分割每个子串
  for (auto const& sub_str : sub_str_vec) {
    auto kv = Split(sub_str, separator2);
    if (kv.empty()) {
      continue;
    }
    // key固定取第一个
    auto key = trim ? Trim(kv[0]) : kv[0];
    if (key.empty()) {
      continue;
    }
    // value可以设置为空
    if (kv.size() == 1) {
      res[key] = "";
      continue;
    }
    // value固定取第二个
    res[key] = trim ? Trim(kv[1]) : kv[1];
  }

  return res;
}

}  // namespace utils
