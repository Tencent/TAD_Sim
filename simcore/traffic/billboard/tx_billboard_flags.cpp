// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_billboard_flags.h"
#include "tx_header.h"
DEFINE_string(billboard_app_name, "Billboard", "");
DEFINE_string(BillboardUpStreamTopics, "TRAFFIC", "");
DEFINE_string(BillboardDownStreamTopics, "TRAFFIC_BILLBOARD", "");

DEFINE_double(BillboardPeriodInMillisecond, 1000.0, "");
DEFINE_bool(LogLevel_Billboard, false, "");
DEFINE_bool(LogLevel_Billboard_PBOutput, false, "");
DEFINE_bool(LogLevel_Billboard_Signal, false, "");

DEFINE_bool(ComputeSectionData, true, "");
DEFINE_double(NavigationUpdateThreshold, 1.0, "");
DEFINE_double(HwyEntryInfluenceDistance, 1000.0, "");
DEFINE_double(ChargingPileInfluenceDistance, 10000.0, "10 km");
DEFINE_bool(ForceUpdatePerStep, false, "");
DEFINE_double(AltitudeStep, 20.0, "");

DEFINE_bool(compute_slope, true, "");
DEFINE_bool(compute_altitude, true, "");
DEFINE_bool(compute_curvature, true, "");
DEFINE_bool(compute_speed_limit, true, "");
DEFINE_bool(compute_vehicle_cnt, true, "");

DEFINE_int64(pile_ignore_roadid, -1, "");
DEFINE_int64(detector_print_roadid, -1, "");
DEFINE_int64(detector_print_sectionid, 0, "");

DEFINE_int32(AverSpeedDuration_ms, 0, "");
DEFINE_int32(compute_start_time_ms, 0, "");
DEFINE_int32(compute_end_time_ms, 60000000, "1000 min");

void PrintFlagsValues_Billboard() TX_NOEXCEPT {
  LOG(INFO) << "#######  Billboard Complier Version ######";
  LOG(INFO) << "#######";
  LOG(INFO) << "####### branch: " << _ST(STR_GIT_BRANCH);
  LOG(INFO) << "####### commit id: " << _ST(STR_COMMIT_ID);
  LOG(INFO) << "#######";
  LOG(INFO) << "#######  " << _ST(PRO_PATH);
  LOG(INFO) << "#######  Billboard Complier Version ######";

  LOG(INFO) << TX_FLAGS(billboard_app_name);
  LOG(INFO) << TX_FLAGS(BillboardUpStreamTopics);
  LOG(INFO) << TX_FLAGS(BillboardDownStreamTopics);
  LOG(INFO) << TX_FLAGS(BillboardPeriodInMillisecond);
  LOG(INFO) << TX_FLAGS(LogLevel_Billboard);
  LOG(INFO) << TX_FLAGS(LogLevel_Billboard_PBOutput);
  LOG(INFO) << TX_FLAGS(LogLevel_Billboard_Signal);
  LOG(INFO) << TX_FLAGS(ComputeSectionData);
  LOG(INFO) << TX_FLAGS(NavigationUpdateThreshold);
  LOG(INFO) << TX_FLAGS(HwyEntryInfluenceDistance);
  LOG(INFO) << TX_FLAGS(ChargingPileInfluenceDistance);
  LOG(INFO) << TX_FLAGS(ForceUpdatePerStep);
  LOG(INFO) << TX_FLAGS(AltitudeStep);

  LOG(INFO) << TX_FLAGS(compute_slope);
  LOG(INFO) << TX_FLAGS(compute_altitude);
  LOG(INFO) << TX_FLAGS(compute_curvature);
  LOG(INFO) << TX_FLAGS(compute_speed_limit);
  LOG(INFO) << TX_FLAGS(compute_vehicle_cnt);
  LOG(INFO) << TX_FLAGS(pile_ignore_roadid);
  LOG(INFO) << TX_FLAGS(detector_print_roadid);
  LOG(INFO) << TX_FLAGS(detector_print_sectionid);
  LOG(INFO) << TX_FLAGS(AverSpeedDuration_ms);
  LOG(INFO) << TX_FLAGS(compute_start_time_ms);
  LOG(INFO) << TX_FLAGS(compute_end_time_ms);
}
