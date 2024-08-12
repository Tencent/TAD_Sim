// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "id_impl.h"

#include <iomanip>
#include <limits>
#include <random>
#include <sstream>

namespace tx_sim {
namespace impl {

static std::random_device g_rd;
static std::mt19937 g_gen(g_rd());
static std::uniform_int_distribution<uint64_t> g_uuid_low_dis(0, std::numeric_limits<uint64_t>::max());

const boost::uuids::uuid String2UUID(const std::string& id_str) {
  boost::uuids::uuid id;
  std::stringstream ss(id_str);
  ss >> id;
  return id;
}

const std::string GenerateRandomUUID() {
  static boost::uuids::random_generator gen;
  try {
    return UUID2String(gen());
  } catch (const std::exception& e) {  // tencent cloud deployment environment does not support getRandom() sys call ...
    uint64_t uuid_low_part = g_uuid_low_dis(g_gen);
    std::stringstream ss;
    ss << "ffffffff-ffff-ffff-" << std::setfill('0') << std::setw(16) << std::hex << uuid_low_part;
    return ss.str();
  }
}

}  // namespace impl
}  // namespace tx_sim
