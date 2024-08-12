// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <sstream>
#include <string>

#include "boost/uuid/nil_generator.hpp"
#include "boost/uuid/random_generator.hpp"
#include "boost/uuid/string_generator.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"

namespace tx_sim {
namespace impl {

inline const std::string UUID2String(const boost::uuids::uuid& id) { return boost::uuids::to_string(id); }

const boost::uuids::uuid String2UUID(const std::string& id_str);

const std::string GenerateRandomUUID();

inline const boost::uuids::uuid GenerateUUIDFromString(const std::string& id_str) {
  static boost::uuids::string_generator gen;
  return gen(id_str);
}

static const boost::uuids::uuid kEmptyUuid = boost::uuids::nil_uuid();

}  // namespace impl
}  // namespace tx_sim
