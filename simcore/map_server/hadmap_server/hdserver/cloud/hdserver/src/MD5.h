/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once

#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

#include "openssl/md5.h"

namespace TX_MD5 {

inline std::string GenMD5Impl(const std::vector<std::string>& inputs) {
  unsigned char output[MD5_DIGEST_LENGTH] = {0};

  MD5_CTX ctx;
  MD5_Init(&ctx);
  for (auto&& input : inputs) {
    MD5_Update(&ctx, input.c_str(), input.length());
  }
  MD5_Final(output, &ctx);

  std::stringstream ss;
  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(output[i]);
  }

  return ss.str();
}

}  // namespace TX_MD5

inline std::string GenMD5(const std::string& input) { return TX_MD5::GenMD5Impl({input}); }

inline std::string GenMD5Sum(const std::string& file) {
  std::ifstream ifs(file.c_str(), std::ios::in | std::ios::binary);
  if (!ifs.is_open()) {
    return "";
  }

  std::vector<std::string> inputs;
  for (char line[1024] = {0}; !ifs.eof();) {
    ifs.read(line, 1024);
    auto size = ifs.gcount();
    if (size > 0) {
      inputs.emplace_back(std::string(line, size));
    }
  }
  ifs.close();
  return TX_MD5::GenMD5Impl(inputs);
}
