// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_flags.h"

DECLARE_string(billboard_app_name);
DECLARE_string(BillboardUpStreamTopics);
DECLARE_string(BillboardDownStreamTopics);

DECLARE_double(BillboardPeriodInMillisecond);
DECLARE_bool(LogLevel_Billboard);
DECLARE_bool(LogLevel_Billboard_PBOutput);
DECLARE_bool(LogLevel_Billboard_Signal);

DECLARE_bool(ComputeSectionData);
DECLARE_double(NavigationUpdateThreshold);
DECLARE_double(HwyEntryInfluenceDistance);
DECLARE_double(ChargingPileInfluenceDistance);
DECLARE_bool(ForceUpdatePerStep);
DECLARE_double(AltitudeStep);

DECLARE_bool(compute_slope);
DECLARE_bool(compute_altitude);
DECLARE_bool(compute_curvature);
DECLARE_bool(compute_speed_limit);
DECLARE_bool(compute_vehicle_cnt);
DECLARE_int64(pile_ignore_roadid);
DECLARE_int32(AverSpeedDuration_ms);
DECLARE_int64(detector_print_roadid);
DECLARE_int64(detector_print_sectionid);
DECLARE_int32(compute_start_time_ms);
DECLARE_int32(compute_end_time_ms);

extern void PrintFlagsValues_Billboard() TX_NOEXCEPT;
