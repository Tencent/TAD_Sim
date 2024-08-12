// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_id_manager.h"

TX_NAMESPACE_OPEN(Base)

txIdManager::TypeConcurrentMapInputId2InputVehIdBase txIdManager::sConcurrentMapInputId2InputVehIdBase;
txIdManager::TypeConcurrentMapVehicleId_gcSet txIdManager::sConcurrentMapVehicleId_gcSet;

TX_NAMESPACE_CLOSE(Base)
