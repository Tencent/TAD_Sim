/**
 * @file PakMapsConfig.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief 
 * @version 0.1
 * @date 2024-07-12
 * 
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 * 
 */
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

void split_string(const std::string& s, const std::string& delim, std::vector<std::string>& ret) {
  ret.clear();
  size_t last = 0;
  size_t index = s.find_first_of(delim, last);
  while (index != std::string::npos) {
    ret.push_back(s.substr(last, index - last));
    last = index + 1;
    index = s.find_first_of(delim, last);
  }
  if (index - last > 0) {
    ret.push_back(s.substr(last, index - last));
  }
}

bool readini(const std::string& ini, std::map<std::string, std::map<std::string, std::string>>& data) {
  using namespace std;
  ifstream in_conf_file(ini);
  if (!in_conf_file) {
    return false;
  }
  data.clear();
  std::map<std::string, std::string> node;
  string str_line = "";
  string str_root = "";
  while (getline(in_conf_file, str_line)) {
    if (str_line.empty() || str_line.front() == ';') {
      continue;
    }
    string::size_type left_pos = 0;
    string::size_type right_pos = 0;
    string::size_type equal_div_pos = 0;
    string str_key = "";
    string str_value = "";
    if ((str_line.npos != (left_pos = str_line.find("["))) && (str_line.npos != (right_pos = str_line.find("]")))) {
      if (!node.empty() && !str_root.empty()) {
        data.insert(make_pair(str_root, node));
      }
      // cout << str_line.substr(left_pos+1, right_pos-1) << endl;
      str_root = str_line.substr(left_pos + 1, right_pos - 1);
      node.clear();
    } else if (str_line.npos != (equal_div_pos = str_line.find("="))) {
      str_key = str_line.substr(0, equal_div_pos);
      str_value = str_line.substr(equal_div_pos + 1, str_line.size() - 1);
      if (str_key.find_first_not_of(" ") != std::string::npos) str_key.erase(0, str_key.find_first_not_of(" "));
      if (str_key.find_last_not_of(" ") != std::string::npos) str_key.erase(str_key.find_last_not_of(" ") + 1);
      if (str_value.find_first_not_of(" ") != std::string::npos) str_value.erase(0, str_value.find_first_not_of(" "));
      if (str_value.find_last_not_of(" ") != std::string::npos) str_value.erase(str_value.find_last_not_of(" ") + 1);
      if (str_value.rfind('\n') != std::string::npos) str_value.erase(str_value.rfind('\n'));
      if (str_value.rfind('\r') != std::string::npos) str_value.erase(str_value.rfind('\r'));

      // cout << str_key << "=" << str_value << endl;
      node.insert(make_pair(str_key, str_value));
    }
  }
  if (!node.empty() && !str_root.empty()) {
    data.insert(make_pair(str_root, node));
  }
  in_conf_file.close();
  return true;
}

int main(int argn, char** argv) {
  if (argn < 2) {
    std::cout << "need map idx";
    return 1;
  }
  std::map<std::string, std::map<std::string, std::string>> data;
  if (!readini("Config/DefaultGame.ini", data)) {
    std::cout << "cannot open game.ini";
    return 1;
  }
  std::vector<std::string> mapidxs;
  split_string(argv[1], "+", mapidxs);

  std::ofstream oini("Config/Linux/LinuxGame.ini");
  oini << "[/Script/UnrealEd.ProjectPackagingSettings]\n";
  for (const auto& mapidx : mapidxs) {
    const auto& mapnames = data["MapName"];
    if (mapnames.find(mapidx) == mapnames.end()) {
      std::cout << "cannot find map idx: " << mapidx << std::endl;
      continue;
    }
    const auto& mapName = mapnames.at(mapidx);

    const auto& mappaths = data["MapPath"];
    if (mappaths.find(mapName) == mappaths.end()) {
      std::cout << "cannot find map name: " << mapName << std::endl;
      continue;
    }
    const auto& mapPath = mappaths.at(mapName);
    std::cout << "find map name: " << mapName << std::endl;
    oini << "+MapsToCook=(FilePath=\"" << mapPath << "\")\n";
  }

  return 0;
}
