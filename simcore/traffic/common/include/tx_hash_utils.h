// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/functional/hash.hpp>
#include "tx_enum_def.h"
#include "tx_header.h"
#include "tx_locate_info.h"
TX_NAMESPACE_OPEN(Utils)

struct LaneUIdHashCompare {
  /**
   * @brief txHashUtils 用于生成 txLaneUId 类型的变量的哈希值
   *
   * 此函数可以帮助我们快速准确地生成一个 txLaneUId 类型变量的哈希值，
   * 用于快速定位车道，避免线性搜索，提高程序效率。
   *
   * @param _uid txLaneUId 类型的变量，包含车道的信息
   * @return size_t 返回计算出的哈希值
   */
  static size_t hash(const Base::txLaneUId& _uid) {
    size_t seed = 0;
    boost::hash_combine(seed, _uid.roadId);
    boost::hash_combine(seed, _uid.sectionId);
    boost::hash_combine(seed, _uid.laneId);
    return seed;
  }

  /**
   * @brief txHashUtils 中的 equal 函数，用于比较两个 txLaneUId 类型的变量是否相等
   *
   * @param lv 第一个待比较的 txLaneUId 变量
   * @param rv 第二个待比较的 txLaneUId 变量
   * @return true 如果两个变量相等，即它们的 roadId、sectionId 和 laneId 都相等
   * @return false 如果两个变量不相等，即它们的 roadId、sectionId 或 laneId 有任何不同
   */
  static bool equal(const Base::txLaneUId& lv, const Base::txLaneUId& rv) {
    return lv.roadId == rv.roadId && lv.sectionId == rv.sectionId && lv.laneId == rv.laneId;
  }

  size_t operator()(const Base::txLaneUId& interval) const { return LaneUIdHashCompare::hash(interval); }
};

struct RoadIdHashCompare {
  /**
   * @brief txHashUtils 静态函数，用于计算 roadID 的哈希值
   *
   * @param _rid 指向 txRoadID 类型的指针，表示要计算哈希值的 roadID
   * @return size_t 返回计算出的哈希值
   */
  static size_t hash(const Base::txRoadID& _rid) { return std::hash<Base::txRoadID>{}(_rid); }

  /**
   * @brief txHashUtils 类中的静态函数，用于比较两个 txRoadID 类型的变量是否相等
   *
   * @param lv 一个指向 const txRoadID 类型的指针，表示第一个要比较的 roadID
   * @param rv 一个指向 const txRoadID 类型的指针，表示第二个要比较的 roadID
   * @return 如果两个 roadID 相等，则返回 true；否则返回 false
   */
  static bool equal(const Base::txRoadID& lv, const Base::txRoadID& rv) { return lv == rv; }
};

struct LandLinkIDHashCompare {
  /**
   * @brief txHashUtils 静态函数，用于计算指定类型的变量的哈希值
   *
   * @param _link_id 指向 Base::txLaneLinkID 类型的指针，表示要计算哈希值的 roadID
   * @return size_t 返回计算出的哈希值
   */
  static size_t hash(const Base::txLaneLinkID& _link_id) { return std::hash<Base::txLaneLinkID>{}(_link_id); }

  /**
   * @brief txHashUtils 类中的静态函数，用于比较两个 txLaneLinkID 类型的变量是否相等
   *
   * @param lv 指向 const txLaneLinkID 类型的指针，表示第一个要比较的 roadID
   * @param rv 指向 const txLaneLinkID 类型的指针，表示第二个要比较的 roadID
   * @return bool 如果两个 roadID 相等，则返回 true；否则返回 false
   */
  static bool equal(const Base::txLaneLinkID& lv, const Base::txLaneLinkID& rv) { return lv == rv; }
};

struct FromToUidHashCompare {
  /**
   * @brief hash 对给定的std::pair<Base::txLaneUId, Base::txLaneUId>类型的_link_from_to进行哈希计算
   * 哈希计算基于Boost库，使用boost::hash_combine()实现链式哈希计算
   *
   * @param _link_from_to 需要进行哈希计算的txLaneUId对
   * @return size_t 返回计算得到的哈希值
   */
  static size_t hash(const std::pair<Base::txLaneUId, Base::txLaneUId>& _link_from_to) {
    size_t seed = 0;
    boost::hash_combine(seed, _link_from_to.first.roadId);
    boost::hash_combine(seed, _link_from_to.first.sectionId);
    boost::hash_combine(seed, _link_from_to.first.laneId);

    boost::hash_combine(seed, _link_from_to.second.roadId);
    boost::hash_combine(seed, _link_from_to.second.sectionId);
    boost::hash_combine(seed, _link_from_to.second.laneId);
    return seed;
  }

  /**
   * @brief equal 比较两个std::pair<Base::txLaneUId, Base::txLaneUId>类型的txLaneUId对是否相等
   *
   * @param lv 第一个txLaneUId对
   * @param rv 第二个txLaneUId对
   * @return true 当两个txLaneUId对相等时返回true
   * @return false 当两个txLaneUId对不相等时返回false
   */
  static bool equal(const std::pair<Base::txLaneUId, Base::txLaneUId>& lv,
                    const std::pair<Base::txLaneUId, Base::txLaneUId>& rv) {
    return (lv.first == rv.first) && (lv.second == rv.second);
  }
};

struct txSysIdHashCompare {
  /**
   * @brief txHashUtils 用于处理txSysId类型的数据结构的哈希函数
   *
   * @param _id txSysId类型的数据结构，其中包含一个唯一标识的系统ID和一个定义时间的时间戳
   * @return size_t 返回txSysId数据结构的哈希值
   */
  static size_t hash(const Base::txSysId& _id) { return std::hash<Base::txSysId>{}(_id); }

  /**
   * @brief txHashUtils 比较两个 txSysId 结构是否相等
   *
   * @param lv 第一个待比较的 txSysId 结构
   * @param rv 第二个待比较的 txSysId 结构
   * @return true 如果两个 txSysId 结构相等
   * @return false 如果两个 txSysId 结构不相等
   */
  static bool equal(const Base::txSysId& lv, const Base::txSysId& rv) { return lv == rv; }
};

struct LaneBoundaryIDHashCompare {
  /**
   * @brief txHashUtils 生成 Base::txBoundaryID 类型变量哈希值
   *
   * @param _laneBoundary_id 需要生成哈希值的 Base::txBoundaryID 类型变量
   * @return size_t 返回哈希值
   */
  static size_t hash(const Base::txBoundaryID& _laneBoundary_id) {
    return std::hash<Base::txBoundaryID>{}(_laneBoundary_id);
  }

  /**
   * @brief txHashUtils 用于比较两个 Base::txBoundaryID 类型变量是否相等
   *
   * @param lv 第一个需要比较的 Base::txBoundaryID 类型变量
   * @param rv 第二个需要比较的 Base::txBoundaryID 类型变量
   * @return true 如果两个 Base::txBoundaryID 类型变量相等
   * @return false 如果两个 Base::txBoundaryID 类型变量不相等
   */
  static bool equal(const Base::txBoundaryID& lv, const Base::txBoundaryID& rv) { return lv == rv; }
};

struct SectionUIdHashCompare {
  /**
   * @brief hash 对 Base::txSectionUId 类型变量进行哈希计算
   *
   * @param _section_id 待哈希计算的 Base::txSectionUId 类型变量
   * @return size_t 计算结果
   */
  static size_t hash(const Base::txSectionUId& _section_id) {
    size_t seed = 0;
    boost::hash_combine(seed, _section_id.first);
    boost::hash_combine(seed, _section_id.second);
    return seed;
  }

  /**
   * @brief equal 比较两个 Base::txSectionUId 是否相等
   *
   * @param lv 第一个要比较的 Base::txSectionUId
   * @param rv 第二个要比较的 Base::txSectionUId
   * @return bool 如果两个 Base::txSectionUId 相等，则返回 true，否则返回 false
   */
  static bool equal(const Base::txSectionUId& lv, const Base::txSectionUId& rv) {
    return lv.first == rv.first && lv.second == rv.second;
  }
};

struct txSizeHashCompare {
  /**
   * @brief txHashUtils 对 Base::txSize 类型变量进行哈希计算
   *
   * @param _txSize_value 待哈希计算的 Base::txSize 类型变量
   * @return size_t 计算结果
   */
  static size_t hash(const Base::txSize& _txSize_value) { return std::hash<Base::txSize>{}(_txSize_value); }

  /**
   * @brief equal 比较两个 Base::txSize 类型变量是否相等
   *
   * @param lv 第一个 Base::txSize 类型变量
   * @param rv 第二个 Base::txSize 类型变量
   * @return bool 如果两个 Base::txSize 类型变量相等则返回 true，否则返回 false
   */
  static bool equal(const Base::txSize& lv, const Base::txSize& rv) { return lv == rv; }
};

struct txElementTypeHashCompare {
  /**
   * @brief txHashUtils 用于对一些基本类型进行哈希计算
   *
   * @param _elemType_value 表示一个基本类型的枚举值，例如 ElementType::INT
   * @return size_t 计算出的哈希值
   */
  static size_t hash(const Base::Enums::ElementType& _elemType_value) {
    return std::hash<Base::txInt>{}(__enum2idx__(_elemType_value));
  }

  /**
   * @brief txHashUtils 工具类，提供基于不同元素类型的比较操作
   *
   * @param lv 表示左侧元素的类型枚举值
   * @param rv 表示右侧元素的类型枚举值
   * @return bool 如果两个元素类型相等，则返回 true，否则返回 false
   */
  static bool equal(const Base::Enums::ElementType& lv, const Base::Enums::ElementType& rv) { return lv == rv; }
};

TX_NAMESPACE_CLOSE(Utils)
