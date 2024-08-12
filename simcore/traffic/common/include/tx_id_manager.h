// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <set>
#include "tbb/concurrent_hash_map.h"
#include "tbb/concurrent_unordered_set.h"
#include "tx_header.h"
TX_NAMESPACE_OPEN(Base)

class txIdManager {
 public:
  /**
   * @brief RegisterInputRegion 向框架注册新的输入ID并更新映射关系
   *
   * @param inputIdSet 一个包含新的输入ID的集合
   * @return void
   */
  static void RegisterInputRegion(std::set<Base::txSysId> inputIdSet) TX_NOEXCEPT {
    sConcurrentMapInputId2InputVehIdBase.clear();
    for (const auto inputId : inputIdSet) {
      TypeConcurrentMapInputId2InputVehIdBase::accessor a;
      sConcurrentMapInputId2InputVehIdBase.insert(a, inputId);
      a->second = inputId * FLAGS_VehicleInputMaxSize;
    }
  }

  /**
   * @brief 检查输入ID是否有效
   *
   * @param inputId 待检查的输入ID
   * @return txBool 如果输入ID有效则返回 txTrue，否则返回 txFalse
   */
  static txBool CheckInputId(const Base::txSysId inputId) TX_NOEXCEPT {
    TypeConcurrentMapInputId2InputVehIdBase::const_accessor ca;
    return sConcurrentMapInputId2InputVehIdBase.find(ca, inputId);
  }

  /**
   * @brief 检查输入ID是否包含指定元素
   *
   * @param inputId 待检查的输入ID
   * @return txBool 如果输入ID包含指定元素则返回 txTrue，否则返回 txFalse
   */
  static txBool Check_GC_Has_InputIdElement(const Base::txSysId inputId) TX_NOEXCEPT {
    TypeConcurrentMapVehicleId_gcSet::const_accessor ca;
    return (sConcurrentMapVehicleId_gcSet.find(ca, inputId) && (CallFail(ca->second.empty())));
  }

  /**
   * @brief 为指定输入ID生成一个唯一标识符
   *
   * @param inputId 待生成唯一标识符的输入ID
   * @return Base::txSysId 生成的唯一标识符
   */
  static Base::txSysId GenIdPerInput(const Base::txSysId inputId) TX_NOEXCEPT {
    if (CallSucc(CheckInputId(inputId))) {
      // 检查是否有指定ID的输入
      if (CallSucc(Check_GC_Has_InputIdElement(inputId))) {
        /*gc is not empty, using gc id.*/
        TypeConcurrentMapVehicleId_gcSet::accessor a;
        sConcurrentMapVehicleId_gcSet.insert(a, inputId);
        const auto retId = (*(a->second.begin()));
        a->second.erase(a->second.begin());
        a.release();
        return retId;
      } else {
        /*gc is empty, using seq id.*/
        TypeConcurrentMapInputId2InputVehIdBase::accessor a;
        sConcurrentMapInputId2InputVehIdBase.insert(a, inputId);
        return (a->second++);
      }
    } else {
      LOG(WARNING) << TX_VARS_NAME(unknow_vehicle_input_id, inputId);
      return -1;
    }
  }

  /**
   * @brief 从唯一ID池中注销指定ID
   *
   * @param inputId 要注销的ID
   * @param _killVehicleId 被注销的车辆ID
   * @return void
   */
  void UnRegisterVehicleId(const Base::txSysId inputId, const Base::txSysId _killVehicleId) TX_NOEXCEPT {
    TypeConcurrentMapVehicleId_gcSet::accessor a;
    sConcurrentMapVehicleId_gcSet.insert(a, inputId);
    a->second.insert(_killVehicleId);
  }

 protected:
  using TypeConcurrentMapInputId2InputVehIdBase = tbb::concurrent_hash_map<txSysId, txSysId>;
  using TypeConcurrentMapVehicleId_gcSet = tbb::concurrent_hash_map<txSysId, std::set<txSysId> >;
  static TypeConcurrentMapInputId2InputVehIdBase sConcurrentMapInputId2InputVehIdBase;
  static TypeConcurrentMapVehicleId_gcSet sConcurrentMapVehicleId_gcSet;
};

TX_NAMESPACE_CLOSE(Base)
