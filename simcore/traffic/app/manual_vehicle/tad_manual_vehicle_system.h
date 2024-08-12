// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tad_stand_alone_traffic_element_system.h"

TX_NAMESPACE_OPEN(TrafficFlow)
#if Use_ManualVehicle
class TAD_ManualVehicleSystem : public TAD_StandAlone_TrafficElementSystem {
 public:
  virtual void CreateAssemblerCtx() TX_NOEXCEPT TX_OVERRIDE;
  virtual Base::txBool UpdateManualVehicleData(const Base::TimeParamManager& timeMgr,
                                               const Base::txString& egoInfoStr) TX_NOEXCEPT;
};

using TAD_ManualVehicleSystemPtr = std::shared_ptr<TAD_ManualVehicleSystem>;
#endif /*Use_ManualVehicle*/
TX_NAMESPACE_CLOSE(TrafficFlow)
