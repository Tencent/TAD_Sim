// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_flags.h"
#include "tx_od_log.h"

DEFINE_bool(LogLevel_Console, true, "");
DEFINE_bool(LogLevel_XmlLoader, true, "");
DEFINE_double(Sampling_Interval_Min, 0.5, "");
void PrintFlagsValues() TX_NOEXCEPT {
  LOG(INFO) << "#######  tx_od_server Git Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  git commit id : " << STR_COMMIT_ID << ", git branch name : " << STR_GIT_BRANCH;
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  tx_od_server Git Version ######";

  LOG(INFO) << TX_FLAGS(LogLevel_Console);
  LOG(INFO) << TX_FLAGS(LogLevel_XmlLoader);
  LOG(INFO) << TX_FLAGS(Sampling_Interval_Min);
}
