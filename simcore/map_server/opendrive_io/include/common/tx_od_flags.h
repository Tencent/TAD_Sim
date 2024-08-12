// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <gflags/gflags.h>
#include "tx_od_marco.h"

DECLARE_bool(LogLevel_Console);
DECLARE_bool(LogLevel_XmlLoader);
DECLARE_double(Sampling_Interval_Min);
extern void PrintFlagsValues() TX_NOEXCEPT;
