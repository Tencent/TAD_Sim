// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once

#include <vector>

#define MY_PI 3.1415926535897932384626433832795
#define EARTH_RADIUS 6378.137

extern double rad(double d);

extern double RealDistance(double lat1, double lng1, double lat2, double lng2);
extern double Distance(double lat1, double lon1, double lat2, double lon2);
extern int GetMaxChannelCapacity(std::vector<int>& distance);
extern int GetEachChannelCapacity(int my_distance, int total_distance, int max_capacity);
extern double DegreeToRadian(double degree);
extern double Haversine(double radian);
extern double CppDistance(double lat1, double lon1, double lat2, double lon2);
