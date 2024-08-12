// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <algorithm>
#include <boost/algorithm/string.hpp> /*string splite*/
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>
#include <cctype>
#include <locale>
#include <regex>
#include "basic.pb.h"
#include "structs/base_struct.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Utils)

/**
 * @brief 将 Vec3 转换为字符串
 *
 * 此函数将传入的 Vec3 对象转换为一个格式为 "Vec3 (x, y, z)" 的字符串。其中 x, y, z 分别是 Vec3 对象的 x, y, z 分量。
 *
 * @param _v 需要转换为字符串的 Vec3 对象
 * @return std::string 转换后的字符串表示形式
 */
static inline Base::txString ToString(const ::sim_msg::Vec3& _v) TX_NOEXCEPT {
  return (boost::format("Vec3 (%.14f, %.14f, %.14f)") % _v.x() % _v.y() % _v.z()).str();
}

/**
 * @brief 将 Vec2 转换为字符串
 *
 * 此函数将传入的 Vec2 对象转换为一个格式为 "Vec2 (x, y)" 的字符串。其中 x, y 分别是 Vec2 对象的 x, y 分量。
 *
 * @param _v 需要转换为字符串的 Vec2 对象
 * @return std::string 转换后的字符串表示形式
 */
static inline Base::txString ToString(const Base::txVec2& _v) TX_NOEXCEPT {
  return (boost::format("Vec2 (%.14f, %.14f)") % _v.x() % _v.y()).str();
}

/**
 * @brief 将 txVec3 对象转换为字符串
 *
 * 此函数将传入的 txVec3 对象转换为一个格式为 "Vec3 (x, y, z)" 的字符串。其中 x, y, z 分别是 txVec3 对象的 x, y, z
 * 分量。
 *
 * @param _v 需要转换为字符串的 txVec3 对象
 * @return txString 转换后的字符串表示形式
 */
static inline Base::txString ToString(const Base::txVec3& _v) TX_NOEXCEPT {
  return (boost::format("Vec3 (%.14f, %.14f, %.14f)") % _v.x() % _v.y() % _v.z()).str();
}

/**
 * @brief txPoint 转为字符串
 *
 * 将传入的 hadmap::txPoint 对象转换为格式为 "WGS84 (x, y, z)" 的字符串。
 * 其中 x, y, z 分别为 txPoint 对象的 x, y, z 分量。
 *
 * @param _p 需要转换为字符串的 txPoint 对象
 * @return Base::txString 转换后的字符串表示形式
 */
static inline Base::txString ToString(const hadmap::txPoint& _p) TX_NOEXCEPT {
  return (boost::format("WGS84 (%.14f, %.14f, %.14f)") % _p.x % _p.y % _p.z).str();
}

/**
 * @brief txLaneId 转换为字符串
 *
 * 将给定的 txLaneId 对象转换为格式为 "txLaneId (road.id = x, section.id = x, lane.id = x)" 的字符串。其中 x 分别为
 * roadId, sectionId 和 laneId 的值。
 *
 * @param _id 需要转换为字符串的 txLaneId 对象
 * @return Base::txString 转换后的字符串表示形式
 */
static inline Base::txString ToString(const hadmap::txLaneId& _id) TX_NOEXCEPT {
  return (boost::format("txLaneId (road.id = %d, section.id = %d, lane.id = %d)") % _id.roadId % _id.sectionId %
          _id.laneId)
      .str();
}

/**
 * @brief txFloat 转换为字符串
 *
 * 将给定的 txFloat 对象转换为格式为 "x.xxxxxx" 的字符串。其中 x 表示小数位数。
 *
 * @param _f 需要转换为字符串的 txFloat 对象
 * @return Base::txString 转换后的字符串表示形式
 */
static inline Base::txString FloatToString(const Base::txFloat& _f) TX_NOEXCEPT {
  return (boost::format("%.14f") % _f).str();
}

/**
 * @brief IntToString 将整数转换为字符串
 *
 * 将给定的整数转换为字符串格式，例如 123 转换为 "123"。
 *
 * @param _n 需要转换为字符串的整数
 * @return Base::txString 转换后的字符串表示形式
 */
static inline Base::txString IntToString(const Base::txInt& _n) TX_NOEXCEPT { return (boost::format("%d") % _n).str(); }

/**
 * @brief 将 64 位整数转换为字符串表示
 *
 * 将给定的 64 位整数转换为字符串格式。例如，调用 `Int64ToString(123)` 会返回 `"123"`。
 *
 * @param _n 需要转换为字符串的 64 位整数
 * @return Base::txString 转换后的字符串表示形式
 */
static inline Base::txString Int64ToString(const Base::txSysId& _n) TX_NOEXCEPT {
  return (boost::format("%lld") % _n).str();
}

// trim from start (in place)
/**
 * @brief 移除字符串开头的空白字符
 *
 * 此函数用于移除字符串开头的所有空白字符（空格、制表符、换行符等）。
 *
 * @param s 需要移除开头空白字符的字符串
 */
static inline void ltrim(Base::txString& s) TX_NOEXCEPT {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
}

// trim from end (in place)
/**
 * @brief 去除字符串尾部的空白字符
 *
 * 此函数用于去除字符串尾部的所有空白字符（空格、制表符、换行符等）。
 *
 * @param s 需要去除尾部空白字符的字符串
 */
static inline void rtrim(Base::txString& s) TX_NOEXCEPT {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
}

// trim from both ends (in place)
/**
 * @brief 删除字符串中的空白字符
 *
 * 删除字符串中前后的空白字符（包括空格、制表符、换行符等）。
 *
 * @param s 需要删除空白字符的字符串
 */
static inline void trim(Base::txString& s) TX_NOEXCEPT {
  ltrim(s);
  rtrim(s);
}

/**
 * @brief 检查字符串是否包含子串
 *
 * 该函数检查源字符串是否包含指定的子串。
 * 它会将源字符串和子串转换为小写形式进行比较，以便不区分大小写。
 *
 * @param srcStr 源字符串
 * @param subStr 子串
 * @return true 如果源字符串包含子串，否则返回 false
 */
static inline Base::txBool IsStringContain(Base::txString srcStr, Base::txString subStr) TX_NOEXCEPT {
  boost::to_lower(srcStr);
  boost::to_lower(subStr);
  if (std::string::npos != srcStr.find(subStr)) {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 对字符串进行分割
 *
 * 此函数将一个字符串根据指定的分隔符进行分割，并将分割后的子字符串存储在一个字符串向量中。
 *
 * @param srcStr 需要分割的源字符串
 * @param delimiter 分隔符
 * @return 返回一个包含分割后子字符串的向量
 */
static inline std::vector<Base::txString> SpliteStringVector(const Base::txString& srcStr,
                                                             const Base::txString& delimiter) TX_NOEXCEPT {
  std::vector<Base::txString> results_pairs;
  boost::algorithm::split(results_pairs, srcStr, boost::is_any_of(delimiter));
  return results_pairs;
}

/**
 * @brief 将字符串按指定分隔符分割并返回两部分的子字符串
 *
 * 根据指定的分隔符将源字符串分割为两部分子字符串，并以元组的形式返回。
 *
 * @param srcStr 需要分割的源字符串
 * @param delimiter 分隔符
 * @return 返回一个包含两部分子字符串的元组
 */
static inline std::tuple<Base::txString, Base::txString> SpliteStr(const Base::txString& srcStr,
                                                                   const Base::txString& delimiter) TX_NOEXCEPT {
  std::vector<Base::txString> results_pairs;
  results_pairs.reserve(2);
  boost::algorithm::split(results_pairs, srcStr, boost::is_any_of(delimiter));
  txAssert(results_pairs.size() >= 2);
  return std::make_tuple(results_pairs[0], results_pairs[1]);
}

/**
 * @brief 将字符串转换为txLaneUId对象
 *
 * 将字符串srcStr按照分隔符delimiter分割成三部分子字符串，然后使用这三部分子字符串构建一个txLaneUId对象。
 *
 * @param srcStr 需要转换的字符串
 * @param delimiter 分隔符
 * @return txLaneUId 构建好的txLaneUId对象
 */
static inline Base::txLaneUId Str2LaneUid(const Base::txString& srcStr, const Base::txString& delimiter) TX_NOEXCEPT {
  std::vector<Base::txString> results_pairs;
  results_pairs.reserve(3);
  boost::algorithm::split(results_pairs, srcStr, boost::is_any_of(delimiter));
  txAssert(results_pairs.size() >= 3);
  trim(results_pairs[0]);
  trim(results_pairs[1]);
  trim(results_pairs[2]);
  return Base::txLaneUId(atol(results_pairs[0].c_str()), atol(results_pairs[1].c_str()),
                         atol(results_pairs[2].c_str()));
}

/**
 * @brief 字符串替换
 *
 * 在str中找到from子串，并将其替换为to子串。
 * 如果未找到from子串，则返回false。
 * 如果找到from子串，则返回true。
 *
 * @param str       需要进行字符串替换的字符串
 * @param from      需要替换的子串
 * @param to        替换后的子串
 * @return true     替换成功
 * @return false    替换失败
 */
static inline Base::txBool StringReplace(Base::txString& str, const Base::txString& from,
                                         const Base::txString& to) TX_NOEXCEPT {
  size_t start_pos = str.find(from);
  if (std::string::npos == start_pos) {
    return false;
  } else {
    str.replace(start_pos, from.length(), to);
    return true;
  }
}

/**
 * @brief 对字符串进行正则表达式替换
 *
 * 将字符串 s 中所有出现的从字符串 from 到字符串 to 的替换。
 * 注意：从 C++11 开始，请在编译选项中启用正则表达式支持（例如，使用 -std=c++11 编译选项）。
 *
 * @param s 要进行替换的字符串
 * @param from 需要被替换的子串
 * @param to 替换后的子串
 */
static inline void StringReplaceRegex(Base::txString& s, const Base::txString& from,
                                      const Base::txString& to) TX_NOEXCEPT {
#if 1
  std::regex pattern(from);
  s = std::regex_replace(s, pattern, to);
#else
  Base::txString::size_type n = p.length();
  for (Base::txString::size_type i = s.find(p); i != Base::txString::npos; i = s.find(p)) {
    s.erase(i, n);
  }
#endif
}

/**
 * @brief 字符串转换为bool类型
 *
 * 此函数将传入的字符串str转换为bool类型，返回值存储在retV中。如果str为非空且等于"true"，则retV为true，否则为false。
 *
 * @param str 要转换为bool类型的字符串
 * @param retV 转换后的bool类型值
 * @return 返回转换是否成功的bool类型值
 */
static inline Base::txBool Str2Type(const Base::txString str, Base::txBool& retV) TX_NOEXCEPT {
  if (_NonEmpty_(str)) {
    if (std::string("true") == str) {
      retV = true;
    } else {
      retV = false;
    }
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 字符串转换为float类型
 *
 * 将传入的字符串str转换为float类型，返回值存储在retV中。
 *
 * @param str 要转换为float类型的字符串
 * @param retV 转换后的float类型值
 * @return 返回转换是否成功的bool类型值
 */
static inline Base::txBool Str2Type(const Base::txString str, Base::txFloat& retV) TX_NOEXCEPT {
  if (_NonEmpty_(str)) {
    retV = std::stof(str);
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 字符串转换为int类型
 *
 * 将传入的字符串str转换为int类型，返回值存储在retV中。
 *
 * @param str 要转换为int类型的字符串
 * @param retV 转换后的int类型值
 * @return 返回转换是否成功的bool类型值
 */
static inline Base::txBool Str2Type(const Base::txString str, int32_t& retV) TX_NOEXCEPT {
  if (_NonEmpty_(str)) {
    retV = std::stoi(str);
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 字符串转换为GPS坐标
 * @param str 输入字符串，格式为 "WGS84 (%.14f, %.14f, %.14f)"
 * @param gps 转换后的GPS坐标，形式为 hadmap::txPoint
 * @return 转换成功返回 true，否则返回 false
 */
static inline Base::txBool Str2GPS(const Base::txString str, hadmap::txPoint& gps) TX_NOEXCEPT {
  /*WGS84 (%.14f, %.14f, %.14f)*/
  Base::txString str_gps = str.substr(7, str.size() - 2);
  auto lon_lat_alt = SpliteStringVector(str_gps, ",");
  if (3 == lon_lat_alt.size()) {
    Utils::trim(lon_lat_alt[0]);
    __Lon__(gps) = std::stod(lon_lat_alt[0]);
    Utils::trim(lon_lat_alt[1]);
    __Lat__(gps) = std::stod(lon_lat_alt[1]);
    Utils::trim(lon_lat_alt[2]);
    __Alt__(gps) = std::stod(lon_lat_alt[2]);
    return true;
  }
  return false;
}

TX_NAMESPACE_CLOSE(Utils)

inline std::ostream& operator<<(std::ostream& os, const hadmap::txPoint& v) TX_NOEXCEPT {
  os << Utils::ToString(v);
  return os;
}
