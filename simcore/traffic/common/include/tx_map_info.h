// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "nlohmann/json.hpp"
#include "structs/base_struct.h"
#include "tx_header.h"
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Base)
struct map_range_t {
  hadmap::txPoint bottom_left;
  hadmap::txPoint top_right;
  hadmap::txPoint center;
  TX_MARK("Can be vacant");

  /**
   * @brief 返回对象的格式化字符串
   *
   * @return Base::txString
   */
  Base::txString Str() const TX_NOEXCEPT {
    std::ostringstream oss;
    oss << "{" << TX_VARS_NAME(bottom_left, Utils::ToString(bottom_left))
        << TX_VARS_NAME(top_right, Utils::ToString(top_right)) << TX_VARS_NAME(center, Utils::ToString(center)) << "}";
    return oss.str();
  }
  friend std::ostream& operator<<(std::ostream& os, const map_range_t& v) {
    os << v.Str();
    return os;
  }
};
using json = nlohmann::json;

/**
 * @brief json格式的区域信息解析
 *
 * 该函数用于将json格式的区域信息解析为map_range_t对象。
 *
 * @param j json对象，包含区域的坐标信息
 * @param _map_range map_range_t对象，存储解析后的区域信息
 */
inline void from_json(const json& j, map_range_t& _map_range) {
  const auto& j_bottom_left = j.at("bottom_left");
  _map_range.bottom_left = hadmap::txPoint(j_bottom_left.at("lon"), j_bottom_left.at("lat"), 0.0);
  const auto& j_top_right = j.at("top_right");
  _map_range.top_right = hadmap::txPoint(j_top_right.at("lon"), j_top_right.at("lat"), 0.0);
}
TX_NAMESPACE_CLOSE(Base)
