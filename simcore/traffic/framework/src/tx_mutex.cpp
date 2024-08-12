// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_mutex.h"

TX_NAMESPACE_OPEN(Base)
tbb::mutex IMutexTBB::s_tbbMutex_outputTraffic;
TX_NAMESPACE_CLOSE(Base)
