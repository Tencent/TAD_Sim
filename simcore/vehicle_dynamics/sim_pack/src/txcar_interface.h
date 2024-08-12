// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <algorithm>
#include <memory>
#include <set>
#include <string>

#include "txcar_base.h"
#include "txsim_module.h"

namespace tx_car {
/**
 * @brief tx car interface
 */
class TxCarInterface final : public tx_sim::SimModule {
 public:
  TXSIM_API TxCarInterface();
  TXSIM_API virtual ~TxCarInterface();

  void Init(tx_sim::InitHelper &helper) override;
  void Reset(tx_sim::ResetHelper &helper) override;
  void Step(tx_sim::StepHelper &helper) override;
  void Stop(tx_sim::StopHelper &helper) override;

 protected:
  TxCarBasePtr getVehicleModel(const TxCarInit &carInit, int flags_v);
  void calVehicleDllFullPath();

 private:
  // vehicle model
  TxCarBasePtr m_car;
  TxCarInit m_carParam;

  // interface with tadsim
  std::string m_start_loc_payload;
  tx_sim::Vector3d mMapOrigin;
  std::set<std::string, std::less<std::string>> m_subtopics, m_pubtopics;

  // if run on cloud
  bool bCloudEnv;
  bool bUseCatalog;
  std::string mCloudDataDir;

  // working directory of this dll
  std::string m_dllPath;
  std::string m_ICE_DLL_FullPath;
  std::string m_EV_DLL_FullPath;
  std::string m_Hybrid_DLL_FullPath;
};
}  // namespace tx_car
