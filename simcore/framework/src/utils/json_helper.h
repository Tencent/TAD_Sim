// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "boost/property_tree/json_parser.hpp"
#include "boost/property_tree/ptree.hpp"
#include "json/json.h"

namespace tx_sim {
namespace utils {

inline void ReadJsonDocFromFile(Json::Value& doc, const std::string& file_path) {
  std::ifstream in_file(file_path, std::ifstream::binary);
  if (!in_file.is_open()) throw std::runtime_error("cannot open file " + file_path + " for json input.");
  bool failed = !(in_file >> doc);
  in_file.close();
  if (failed) throw std::runtime_error("failed reading json contennt from file " + file_path);
}

inline void WriteJsonDocToFile(const Json::Value& doc, const std::string& file_path) {
  std::ofstream out_file(file_path, std::ofstream::binary);
  if (!out_file.is_open()) throw std::runtime_error("cannot open file " + file_path + " for json output.");
  bool failed = !(out_file << doc);
  out_file.close();
  if (failed) throw std::runtime_error("failed writing json contennt to file " + file_path);
}

inline void ReadJsonDocFromString(Json::Value& doc, const std::string& json_str) {
  std::string errs;
  if (!std::unique_ptr<Json::CharReader>(Json::CharReaderBuilder().newCharReader())
           ->parse(json_str.data(), json_str.data() + json_str.size(), &doc, &errs))
    throw std::runtime_error("failed reading json from string: " + errs + "\n input string: " + json_str);
}

inline void WriteJsonDocToString(const Json::Value& doc, std::string& json_str) {
  std::stringstream ss;
  ss << doc;
  json_str.assign(std::istreambuf_iterator<char>(ss), {});
  // json_str.assign(Json::FastWriter().write(doc));  // string writer is deprecated by the jsoncpp.
}

inline std::string Map2Json(const std::unordered_map<std::string, std::string>& m) {
  boost::property_tree::ptree pt;
  for (auto& kv : m) pt.put(kv.first, kv.second);
  std::ostringstream buf;
  boost::property_tree::write_json(buf, pt, false);
  return buf.str();
}

inline void Json2Map(const std::string& j, std::unordered_map<std::string, std::string>& m) {
  boost::property_tree::ptree pt;
  std::istringstream ss(j);
  boost::property_tree::read_json(ss, pt);
  m.clear();
  for (auto& item : pt) m[item.first] = item.second.get_value<std::string>();
}

}  // namespace utils
}  // namespace tx_sim
