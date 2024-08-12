/**
 * @file Hmap.h
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include "mapengine/hadmap_engine.h"

/// @brief map handle
extern hadmap::txMapHandle *hMapHandle;
/// @brief road information
extern hadmap::txRoads mapRoads;
/// @brief object information
extern hadmap::txObjects mapObjects;

bool LoadMap(const std::string &fname);

bool GetEgoLane(const hadmap::txPoint &loc, hadmap::txLanePtr &lanePtr);
