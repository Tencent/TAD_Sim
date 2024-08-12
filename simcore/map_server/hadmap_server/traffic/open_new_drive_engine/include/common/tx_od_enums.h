// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <type_traits>
#include "tx_type_defs.h"
TX_NAMESPACE_OPEN(Base)
TX_NAMESPACE_OPEN(Enums)
enum class MapElementType : Base::txInt {
  eXodr_Meta = 1,
  eXodr_Road = 2,
  eXodr_Controller,
  eXodr_Junction,
  eXodr_Road_Link = 5,
  eXodr_Road_Type = 6,
  eXodr_Road_PlanView = 7,
  eXodr_Road_ElevationProfile = 8,
  eXodr_Road_Lanes,
  eXodr_Road_Lane,
  eXodr_Junction_Connect,
};
enum class MapStandardVersion : Base::txInt { eXodr_1_4 };

enum class MapType : Base::txInt { eUnKnown = 0, eOpenDrive = 1 };
enum class MapSourceType : Base::txInt { eUnKnown = 0, eManual = 1, e3rd = 2 };

enum class eCurveType : Base::txInt { eRefenceLine = 0, eLaneType = 1 };
enum class TitleLevel : Base::txInt { eDefault = 0 /*start 0 for container cache*/, e1, e2, e3, e4 };
enum class LaneLineType : Base::txInt {
  eLeftBoundary = 0 /*start 0 for container cache*/,
  eRightBoundary = 1,
  eCenterLine = 2
};
enum class LaneLineMarkType : Base::txInt {
  none = 0,
  solid,
  broken,
  solid_solid,
  solid_broken,
  broken_solid,
  botts_dots,
  grass,
  curb
};
enum class CoordinateType : Base::txInt { eWGS84 = 0 /*start 0 for container cache*/, eEnu = 1, eGCJ02 = 2 };
enum class DimensionType : Base::txInt { e2D = 0 /*start 0 for container cache*/, e3D = 1 };
enum class XodrElementType : Base::txInt { eRoad = 0, eSection = 1, eLane = 2 };
enum class RoadType : Base::txInt { eRoad };
TX_NAMESPACE_CLOSE(Enums)
TX_NAMESPACE_CLOSE(Base)
