/**
 * @file hmap.h
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-03-27
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#pragma once

#include <string>
#include "mapengine/hadmap_engine.h"

extern hadmap::txMapHandle* hMapHandle;
extern hadmap::txRoads mapRoads;
extern hadmap::txObjects mapObjects;
extern hadmap::txJunctions mapJunctions;

bool LoadMap(const std::string& fname);

bool GetEgoLane(const hadmap::txPoint& loc, hadmap::txLanePtr& lanePtr);
