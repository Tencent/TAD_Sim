/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
namespace utils {

void TrimInplace(std::string& s);

std::string Trim(const std::string& s);

std::string TrimPrefix(const std::string& s, const std::string& prefix);

std::string TrimSuffix(const std::string& s, const std::string& suffix);

// 数字补齐
std::string FromatNumberToString(int num, std::string suffix = "");
// 浮点数转字符串
std::string FloatPrecisionToString(float num, int precision);

bool IsStartWith(const std::string& s, const std::string& prefix);
// 字符串拼接
std::string MergeStringAddLink(std::vector<std::string> strings, std::string linkstr);
// 字符串切分
template <typename T = std::string>
std::vector<T> Split(const std::string& in, char separator) {
  std::vector<T> output;
  auto begin = in.begin(), end = in.end();
  auto first = begin;

  while (first != end) {
    auto second = std::find(first, end, separator);
    if (first != second) output.emplace_back(in.substr(std::distance(begin, first), std::distance(first, second)));
    if (second == end) break;
    first = ++second;
  }

  return output;
}

std::unordered_map<std::string, std::string> SplitToMap(const std::string& in, char separator1, char separator2,
                                                        bool trim = true);

// 字符串、数字相互转化
template <class OutType, class InType>
OutType Convert(const InType& t) {
  std::stringstream stream;

  stream << t;
  OutType result;
  stream >> result;

  return result;
}

template <class T>
std::string Join(const std::vector<T>& in, const std::string& sep) {
  std::stringstream stream;

  bool first = true;
  for (const auto& s : in) {
    if (first)
      first = false;
    else
      stream << sep;

    stream << s;
  }

  return stream.str();
}

}  // namespace utils
