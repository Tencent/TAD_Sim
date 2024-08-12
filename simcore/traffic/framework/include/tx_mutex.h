// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"

#if USE_TBB
#  include <tbb/mutex.h>
#  ifdef max
#    undef max
#    undef min
#  endif  // max
#endif

TX_NAMESPACE_OPEN(Base)

class IMutexTBB {
#if USE_TBB

 protected:
  static tbb::mutex s_tbbMutex_outputTraffic;
#endif

 public:
  template <class Archive>
  void serialize(Archive& archive) {}
};
TX_NAMESPACE_CLOSE(Base)
