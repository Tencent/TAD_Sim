/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include "gflags/gflags.h"

DECLARE_string(cos_config);

DECLARE_string(sim_host);
DECLARE_string(sim_url);
DECLARE_string(sim_port);
DECLARE_string(sim_salt);
DECLARE_string(sim_all_map_url);
DECLARE_int32(map_syncer_interval_seconds);
DECLARE_bool(enable_map_syncer);
