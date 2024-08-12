// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/car_common.h"

#include <cstring>
#include <fstream>
#include <mutex>

#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
class PathManager {
 public:
    static PathManager& getInstance() {
        static PathManager instance;
        return instance;
    }

    std::string& getParPath() {
        return par_path;
    }
    void setParPath(std::string path) {
        par_path = path;
    }

 private:
    std::string par_path;
    PathManager() {}
    PathManager(const PathManager&) = delete;
    PathManager& operator=(const PathManager&) = delete;
};

tx_car::TxCarInfo global_err_info = {tx_car::TxCarStateCode::no_error, "no error"};  // error info
std::mutex global_state_lock;                                                        // error info lock

namespace car_config {

const std::string &getParPath() {  // get parameter file full path
  return  PathManager::getInstance().getParPath();
}

void setParPath(const std::string &par_path, bool force_update) {  // set parameter file full path
  if (force_update) {
    PathManager::getInstance().setParPath(par_path);
  } else if (!tx_car::isFileExist(PathManager::getInstance().getParPath())) {
    PathManager::getInstance().setParPath(par_path);
  }
}

/* dump string to file*/
bool dumpToFile(const std::string &content, const std::string &file_path) {
  std::ofstream out(file_path, std::ios::out);

  if (out.is_open()) {
    out << content;
    out.flush();
    out.close();
    LOG_INFO << "dump to file done " << file_path << ".\n";
  } else {
    LOG_ERROR << "fail to dump " << file_path << "\n";
    return false;
  }

  return true;
}

/* load string from file path*/
bool loadFromFile(std::string &content, const std::string &file_path) {
  std::ifstream in(file_path, std::ios::in);
  content.clear();

  if (in.is_open()) {
    content = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    return true;
  }

  return false;
}

/* load string from file path*/
MODULE_API bool loadFromFile(std::list<std::string> &content, const std::string &file_path) {
  std::ifstream in(file_path, std::ios::in);
  content.clear();

  if (in.is_open()) {
    char buffer[1024];
    do {
      in.getline(buffer, 1023);
      if (!in.eof() && in.good()) content.push_back(std::string(buffer));
    } while (!in.eof() && in.good());
    in.close();
    return true;
  }

  return false;
}

/* set error infomation */
void setErrorInfo(const tx_car::TxCarInfo &err_info) {
  std::lock_guard<std::mutex> guard(global_state_lock);
  global_err_info = err_info;
}

/* get error infomation, return true if there is error */
tx_car::TxCarInfo getErrorInfo() {
  std::lock_guard<std::mutex> guard(global_state_lock);
  return global_err_info;
}
}  // namespace car_config

void setLogFlag(int level) { FLAGS_v = level; }

/* if file exist */
bool isFileExist(const std::string &file) {
  std::ifstream in(file, std::ios::in);
  bool file_exist = in.is_open();
  if (file_exist) in.close();
  return file_exist;
}

/* split string */
void splitString(const std::string &content, std::vector<std::string> &splitted, const std::string &spliter) {
  splitted.reserve(tx_car::kMap1dSize);
  size_t p1 = 0, p2 = 0;

  do {
    p2 = content.find(spliter, p1);
    if (p2 != content.npos) {
      if (p2 > p1) {
        std::string subString = content.substr(p1, p2 - p1);
        splitted.push_back(subString);
      }
      p1 = p2 + spliter.size();
    }
  } while (p2 != content.npos);

  if (p1 < content.size()) {
    std::string subString = content.substr(p1);
    splitted.push_back(subString);
  }
}

/* remove white space from string*/
void rmCharFromString(std::string &content, const char &ch) {
  if (content.size() == 0) return;

  std::string buffer;
  buffer.reserve(2048);

  for (auto i = 0; i < content.size(); ++i) {
    if (content.at(i) != ch) {
      buffer.push_back(content.at(i));
    }
  }
  content = buffer;
}

/* trim string, remove '/n' at end */
void trim(std::string &content, const char &trim_char) {
  if (content.size() == 0) return;

  std::string buffer;
  buffer.reserve(1024);

  for (auto i = 0; i < content.size(); ++i) {
    if (content.at(i) != trim_char) {
      buffer.push_back(content.at(i));
    } else {
      break;
    }
  }
  content = buffer;
}

/* check if string begin with specific char */
bool beginWithChar(const std::string &content, const char &ch) {
  if (content.size() == 0) return false;

  return content.at(0) == ch;
}
}  // namespace tx_car
