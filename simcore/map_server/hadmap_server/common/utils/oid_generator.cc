// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "common/utils/oid_generator.h"

#include <boost/uuid/uuid.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <string>

#include "common/utils/misc.h"

#if defined(_MSC_VER)
#pragma comment(lib, "bcrypt.lib")
#endif

namespace tad {
namespace sim {

namespace {

std::string ConvertUuidToString(const boost::uuids::uuid& to_convert) {
  const uint8_t* uuid = to_convert.data;

  return StringPrintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", uuid[0], uuid[1], uuid[2],
                      uuid[3], uuid[4], uuid[5], uuid[6], uuid[7], uuid[8], uuid[9], uuid[10], uuid[11], uuid[12],
                      uuid[13], uuid[14], uuid[15]);

  /*return StringPrintf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
               uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
               uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);*/

  // std::cout << "str is: " << str.length() << std::endl;
}

}  // anonymous namespace

std::string ObjectIdGenerator::Next() {
#ifdef RANDOM_WITHOUT_BOOST
  return NextWithoutBoost();
#endif
  std::lock_guard<LockType> l(oid_lock_);
  boost::uuids::uuid uuid = oid_generator_();
  std::cout << "gen next oid done." << std::endl;
  std::cout << "size of data is: " << sizeof(uuid.data) << std::endl;
  std::string res = ConvertUuidToString(uuid);

  return res;
}

std::string ObjectIdGenerator::NextWithoutBoost() {
  thread_local std::random_device rd;
  thread_local std::mt19937 gen(rd());
  thread_local std::uniform_int_distribution<> dis(0, 15);
  thread_local std::uniform_int_distribution<> dis2(8, 11);
  std::stringstream ss;
  ss << std::hex;
  for (int i = 0; i < 15; i++) {
    ss << dis(gen);
  }
  ss << dis2(gen);
  for (int i = 0; i < 15; i++) {
    ss << dis(gen);
  }
  return ss.str();
}

/*
 * implement this later when needed.
 *
bool; ObjectIdGenerator::Canonicalize(const string& input,
                                       string* output) const {
  try {
    boost::uuids::uuid uuid = oid_validator_(input);
    *output = ConvertUuidToString(uuid);
    return true;
  } catch (std::exception& e) {
      return false;
    //return Status::InvalidArgument(Substitute("invalid uuid $0: $1",
                                              input, e.what()));
  }
}*/

}  // namespace sim
}  // namespace tad
