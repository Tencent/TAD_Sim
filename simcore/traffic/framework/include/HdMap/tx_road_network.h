// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tbb/concurrent_hash_map.h"
#include "tx_hash_utils.h"
#include "tx_header.h"
#include "tx_locate_info.h"

TX_NAMESPACE_OPEN(HdMap)
class txRoadNetwork {
 public:
  using JointPointTreeNodePtr = Base::JointPointTreeNodePtr;
  using JointPointId_t = Base::JointPointId_t;
  using JointPointIdHashCompare = Base::JointPointIdHashCompare;

 public:
  /**
   * @brief 初始化路网数据
   *
   * 该函数用于初始化txRoadNetwork类的实例，将路网信息转换为有效的数据结构，包括道路的数据和相互连接的道路的信息。
   *
   * @param lanePtrVec 一个数组，包含了所有道路的指针。
   * @param lanelinkPtrVec 一个数组，包含了所有道路之间的连接信息。
   * @return 返回一个布尔值，如果初始化成功则返回true，否则返回false。
   */
  static Base::txBool Initialize(const hadmap::txLanes& lanePtrVec,
                                 const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT;

 protected:
  /**
   * @brief 合并两个节点的路网信息
   *
   * 如果两个节点满足合并条件（例如它们在同一个交叉点，但在不同的道路中），则将这两个节点合并到一起，并返回
   * true。否则，返回 false。
   *
   * @param leftPtr 第一个节点的指针
   * @param rightPtr 第二个节点的指针
   * @return 如果成功合并，则返回 true，否则返回 false
   */
  static Base::txBool MergeNode(Base::JointPointTreeNodePtr leftPtr, Base::JointPointTreeNodePtr rightPtr) TX_NOEXCEPT;

  /**
   * @brief 从道路链接数组中创建道路联接关节点点向量
   *
   * 该函数用于根据输入的hadmap::txLaneLinks类型的链接向量，生成一个新的向量包含所有关联的道路联接关节点。
   *
   * @param lanelinkPtrVec 一个包含hadmap::txLaneLinks类型对象指针的向量，表示道路链接数组
   * @return 返回一个布尔值，如果成功创建新的关节点向量，则返回true，否则返回false。
   */
  static Base::txBool CreateLaneLinkJointPointVec(const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT;

  /**
   * @brief 根据输入的hadmap::txLanes类型的lanePtrVec，创建一个包含所有关联的道路联接关节点的向量
   *
   * @param lanePtrVec 一个包含hadmap::txLanes类型对象指针的向量，表示道路链接数组
   * @return 返回一个布尔值，如果成功创建新的关节点向量，则返回true，否则返回false。
   */
  static Base::txBool CreateLaneJointPointVec(const hadmap::txLanes& lanePtrVec) TX_NOEXCEPT;

  /**
   * @brief 合并节点到泊位链接及道路
   *
   * 此函数将合并道路和道路连接。如果成功合并，则返回true，否则返回false。
   *
   * @param linkId 关节点的ID
   * @param fromLaneUid 从道路的ID
   * @param toLaneUid 到道路的ID
   * @param isStartPt 是否为关节点的起始点
   * @return 返回一个布尔值，如果成功合并，则返回true，否则返回false。
   */
  static Base::txBool MergeJointWithLanelinkAndLane(const Base::txLaneLinkID linkId, const Base::txLaneUId& fromLaneUid,
                                                    const Base::txLaneUId& toLaneUid,
                                                    const Base::txBool isStartPt) TX_NOEXCEPT;

  /**
   * @brief 合并节点到道路及相邻道路
   *
   * 此函数将指定的关节点合并到指定的道路和相邻道路上。如果成功合并，则返回true，否则返回false。
   *
   * @param srcLaneUid 关节点所在的道路ID
   * @param isStartPt 是否为关节点的起始点
   * @return 返回一个布尔值，如果成功合并，则返回true，否则返回false。
   */
  static Base::txBool MergeJointWithLaneAndLane(const Base::txLaneUId srcLaneUid,
                                                const Base::txBool isStartPt) TX_NOEXCEPT;

  /**
   * @brief 创建图顶点列表
   *
   * 创建顶点列表并初始化顶点结构数组，用于存储图的顶点信息。
   *
   * @return 如果创建成功则返回 true，否则返回 false。
   */
  static Base::txBool CreateGraphVertexList() TX_NOEXCEPT;

  /**
   * @brief 根据道路网的相关信息创建图的边列表
   *
   * 使用给定的道路信息创建图的边列表，包括两条相邻道路的交叉口。
   * 注意: 使用此方法前，请确保您已经成功地创建了图的顶点列表。
   *
   * @param lanePtrVec  指向 hadmap 中的道路信息列表
   * @param lanelinkPtrVec 指向 hadmap 中的道路连接信息列表
   * @return 返回 true 表示创建成功，返回 false 表示创建失败。
   */
  static Base::txBool CreateGraphEdgeList(const hadmap::txLanes& lanePtrVec,
                                          const hadmap::txLaneLinks& lanelinkPtrVec) TX_NOEXCEPT;

  /**
   * @brief 检查关联树的有效性
   *
   * 此函数用于检查关联树（joint tree）的有效性。关联树是一个数据结构，用于表示汽车道路网络中的交叉口。
   * 返回 true 表示关联树有效，返回 false 表示关联树无效。
   *
   * @return 返回 true 表示关联树有效，返回 false 表示关联树无效。
   */
  static Base::txBool CheckJointTree() TX_NOEXCEPT;

  /**
   * @brief 计算关联点位置
   *
   * 根据指定的关联点 ID 集合，计算关联点在地图上的位置。
   *
   * @param hashSetId 一个关联点 ID 集合
   * @return 返回关联点在地图上的位置
   */
  static hadmap::txPoint ComputeJointPointLocation(
      const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& hashSetId) TX_NOEXCEPT;

 public:
#if __TX_Mark__("VertexProperty & EdgeProperty")
  class VertexProperty {
   public:
    VertexProperty(const Base::txSize id = 0, const hadmap::txPoint wgs84_point = hadmap::txPoint(),
                   const std::unordered_set<JointPointId_t, JointPointIdHashCompare> HashSetId =
                       std::unordered_set<JointPointId_t, JointPointIdHashCompare>())
        : m_vtx_id(id), m_wgs84_point(wgs84_point), m_HashSetId(HashSetId) {}
    virtual ~VertexProperty() TX_DEFAULT;

    /**
     * @brief 转换为字符串表示
     *
     * 将 roadNetwork 对象转换为字符串表示，以便输出或存储。
     *
     * @return 返回 roadNetwork 对象的字符串表示
     */
    std::string toString() const TX_NOEXCEPT {
      std::ostringstream oss;
      oss << *this;
      return oss.str();
    }

    /**
     * @brief 获取关联关节点 ID 集合
     *
     * 返回当前 roadNetwork 对象关联的关节点 ID 集合。
     *
     * @return 返回当前 roadNetwork 对象关联的关节点 ID 集合
     */
    const std::unordered_set<JointPointId_t, JointPointIdHashCompare>& RelatJointPointSet() const TX_NOEXCEPT {
      return m_HashSetId;
    }

    /**
     * @brief 获取顶点ID
     *
     * 获取存储在roadNetwork对象中的顶点ID。
     *
     * @return 返回当前roadNetwork对象中的顶点ID
     */
    Base::txSize getVertexID() const TX_NOEXCEPT { return m_vtx_id; }

    /**
     * @brief 获取WGS84坐标
     *
     * 获取存储在roadNetwork对象中的WGS84坐标点。
     *
     * @return WGS84坐标点
     */
    hadmap::txPoint getWgs84() const TX_NOEXCEPT { return m_wgs84_point; }
    friend std::ostream& operator<<(std::ostream& os, VertexProperty const& v) TX_NOEXCEPT {
      return os << "id = " << v.m_vtx_id << " wgs84_point " << Utils::ToString(v.m_wgs84_point);
    }

   public:
    template <class Archive>
    void serialize(Archive& ar) {
      ar(_MAKE_NVP_("vertex_id", m_vtx_id), _MAKE_NVP_("joint_point_pos", m_wgs84_point),
         _MAKE_NVP_("joint_point_set", m_HashSetId));
    }

   protected:
    Base::txSize m_vtx_id;
    hadmap::txPoint m_wgs84_point;
    std::unordered_set<JointPointId_t, JointPointIdHashCompare> m_HashSetId;
  };
#endif

 public:
  /**
   * @brief 获取顶点属性
   *
   * 根据给定的关节点ID获取对应的顶点属性。
   *
   * @param jointPtId 关节点ID
   * @return 返回关节点对应的顶点属性
   */
  static VertexProperty GetVtxProperty(const JointPointId_t& jointPtId) TX_NOEXCEPT;

  /**
   * @brief 获取顶点属性
   *
   * 根据给定的顶点ID获取对应的顶点属性。
   *
   * @param vtxId 顶点ID
   * @return 返回顶点对应的属性信息
   */
  static VertexProperty GetVtxProperty(const Base::txSize& vtxId) TX_NOEXCEPT;

  /**
   * @brief 释放 txRoadNetwork 类的实例
   *
   * 释放 txRoadNetwork 类的实例，回收相关资源。
   *
   * @param TX_NOEXCEPT 异常捕获断言，禁止抛出异常
   */
  static void Release() TX_NOEXCEPT;

 protected:
  using JointPointId2TreeNodePtr = std::unordered_map<JointPointId_t, JointPointTreeNodePtr, JointPointIdHashCompare>;
  static JointPointId2TreeNodePtr s_map_jointpoint2treeNode;
  static std::vector<JointPointTreeNodePtr> s_vec_non_leaf_nodes;

  using JointPointId2VertexPropertyType =
      tbb::concurrent_hash_map<JointPointId_t, VertexProperty, JointPointIdHashCompare>;
  static JointPointId2VertexPropertyType s_JointPtId2VtxProperty;

  using VtxPropertyId2VtxPropertyType =
      tbb::concurrent_hash_map<Base::txSize, VertexProperty, Utils::txSizeHashCompare>;
  static VtxPropertyId2VtxPropertyType s_VtxId2VtxProperty;
};
TX_NAMESPACE_CLOSE(HdMap)
