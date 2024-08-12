// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <vector>

#ifdef _WIN32
#  define MODULE_API __declspec(dllexport)
#else
#  define MODULE_API
#endif

namespace tx_car {
MODULE_API const int kMap1dSize = 500;
MODULE_API const int kMap2dSize = kMap1dSize * kMap1dSize;
MODULE_API const size_t Const_1_Million = 1024 * 1024;

// 0: no error, 1~63: reserved, >=64: error state
enum TxCarStateCode {
  no_error = 0,        // no error
  license_error = 64,  // license error
  parameter_error,     // parameter error
  model_error          // model runtime error
};

/* tx car model state */
struct TxCarInfo {
  TxCarStateCode m_state;
  std::string m_info;
};

namespace car_config {

/* get parameter file full path */
MODULE_API const std::string &getParPath();

/* set parameter file full path */
MODULE_API void setParPath(const std::string &par_path, bool force_update = false);

/* load string from file path*/
MODULE_API bool loadFromFile(std::string &content, const std::string &file_path);

/* load string from file path*/
MODULE_API bool loadFromFile(std::list<std::string> &content, const std::string &file_path);

/* dump string to file*/
MODULE_API bool dumpToFile(const std::string &content, const std::string &file_path);

/* set error infomation */
MODULE_API void setErrorInfo(const tx_car::TxCarInfo &err_info = {tx_car::TxCarStateCode::no_error, ""});

/* get error infomation, return true if there is error */
MODULE_API tx_car::TxCarInfo getErrorInfo();
}  // namespace car_config

/* set log flags */
MODULE_API void setLogFlag(int level = 0);

/* if file exist */
MODULE_API bool isFileExist(const std::string &file);

/* split string, empty substr has been excluded from return value */
MODULE_API void splitString(const std::string &content, std::vector<std::string> &splitted,
                            const std::string &spliter = "=");

/* remove character from string, default is whitespace */
MODULE_API void rmCharFromString(std::string &content, const char &ch = ' ');

/* trim string, remove string begin from 'trim_char' */
MODULE_API void trim(std::string &content, const char &trim_char = '\n');

/* check if string begin with specific char */
MODULE_API bool beginWithChar(const std::string &content, const char &ch = '[');
}  // namespace tx_car
