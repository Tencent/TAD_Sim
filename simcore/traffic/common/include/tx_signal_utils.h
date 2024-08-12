// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <set>
#include "tx_header.h"
TX_NAMESPACE_OPEN(Utils)

/**
 * @brief 根据给定的参数获取车道前置连接
 *
 * 此函数通过指定的车道集合，存在的干扰长度和待连接车道，返回包含待连接车道的排序集合。
 *
 * @param laneUidSet 车道的编号集合
 * @param existingInflunceLength 存在的干扰长度
 * @param fromLaneUidSet 待连接车道的编号集合
 * @return 返回一个包含待连接车道的排序集合
 */
std::set<Base::txLaneID> GetPreLinkFromLanes(const std::set<Base::txLaneUId>& laneUidSet,
                                             Base::txFloat& existingInflunceLength,
                                             std::set<Base::txLaneUId>& fromLaneUidSet) TX_NOEXCEPT;

/**
 * @brief 根据给定的车道，获取该车道之前车道集合
 *
 * 此函数接收一个车道指针作为参数，通过分析该车道的laneID, neighborLanes和predecessorLane，获取该车道之前的车道集合。
 *
 * @param srcLanePtr 给定的车道指针
 * @return 返回一个包含该车道前方车道ID的set集合
 */
std::set<Base::txLaneUId> GetPreLanesFromLane(const hadmap::txLanePtr srcLanePtr) TX_NOEXCEPT;

/**
 * @brief 从给定的车道集合中获取第一层车道前方车道集合（不包括自身车道）
 *
 * 本函数通过分析给定的车道集合，获取第一层车道前方车道集合。即沿着第一层车道往前走，直到不能往前走为止，返回这些车道的ID集合。
 *
 * @param srcLaneUidSet 给定的车道集合
 * @return 返回一个包含第一层车道前方车道ID的集合
 */
std::set<Base::txLaneUId> Get_1stRing_PreLanes_WithoutSelfLane_FromLaneUidSet(
    const std::set<Base::txLaneUId>& srcLaneUidSet) TX_NOEXCEPT;

/**
 * @brief 从给定的车道中获取所有之前车道及自身车道的ID集合
 *
 * 通过分析给定车道的信息，获取包括之前车道及自身车道在内的所有车道ID集合，并返回结果。
 *
 * @param srcLanePtr 给定的车道指针
 * @param length 返回的所有之前车道及自身车道的长度之和
 * @return 返回包含所有前方车道及自身车道的ID集合
 */
std::set<Base::txLaneUId> GetAllPreLanesAndSelfLaneFromLane(const hadmap::txLanePtr srcLanePtr,
                                                            Base::txFloat& length) TX_NOEXCEPT;

/**
 * @brief 根据给定的之前车道指针，获取其前方车道、连接车道以及自身车道的ID集合和连接ID集合
 *
 * 该函数根据给定的车道指针，获取其之前车道、连接车道以及自身车道的ID集合和连接ID集合。
 *
 * @param curLanePtr   当前车道指针
 * @param refLaneUidSet 之前车道的ID集合（输出参数）
 * @param refLinkIdSet  连接车道的ID集合（输出参数）
 */
void GetPreLaneAndLinkAndSelfLaneFromLane(const hadmap::txLanePtr curLanePtr, std::set<Base::txLaneUId>& refLaneUidSet,
                                          std::set<Base::txLaneID>& refLinkIdSet) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(Utils)
