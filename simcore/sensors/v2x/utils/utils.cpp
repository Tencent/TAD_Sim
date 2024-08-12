
/**
 * @file utils.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "utils.h"
#include <math.h>
#include <algorithm>
#include <iostream>

double rad(double d) { return d * MY_PI / 180.0; }

/**
 * @brief distance of two points in meters
 *
 * @param lat1 latitude
 * @param lng1 longitude
 * @param lat2 latitude
 * @param lng2 longitude
 * @return double of distance in meters
 */
double RealDistance(double lat1, double lng1, double lat2, double lng2) {
  double a;
  double b;
  double radLat1 = rad(lat1);
  double radLat2 = rad(lat2);
  a = radLat1 - radLat2;
  b = rad(lng1) - rad(lng2);
  double s = 2 * asin(sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2)));
  s = s * EARTH_RADIUS;
  s = s * 1000;
  return s;
}

/**
 * @brief distance of two points in meters
 *
 * @param lat1 latitude
 * @param lng1 longitude
 * @param lat2 latitude
 * @param lng2 longitude
 * @return double
 */
double Distance(double lat1, double lon1, double lat2, double lon2) {
  const double R = 6371000.0;  //
  double dLat = (lat2 - lat1) * MY_PI / 180.0;
  double dLon = (lon2 - lon1) * MY_PI / 180.0;
  double a = sin(dLat / 2.0) * sin(dLat / 2.0) +
             cos(lat1 * MY_PI / 180.0) * cos(lat2 * MY_PI / 180.0) * sin(dLon / 2.0) * sin(dLon / 2.0);
  double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
  return R * c;  //
}

/**
 * @brief Get the Max Channel Capacity object
 *
 * @param distance
 * @return int
 */
int GetMaxChannelCapacity(std::vector<int>& distance) {
  if (distance.size() < 1) return 0;
  std::sort(distance.begin(), distance.end());
  int equal_distance = distance.size() % 2 == 0 ? distance[distance.size() / 2 - 1] : distance[distance.size() / 2];
  std::cout << "equal_distance = " << equal_distance;
  double snr = 0.07178 - 0.00000028684 * equal_distance * equal_distance;
  double delta = distance.size() > 199 ? 1 : 0.1 * log2(distance.size()) / log2(200);
  double adjust = 1 - delta * 0.1;
  int max_channel_capacity = 10000000 * log2(1 + snr * adjust);
  std::cout << " max_channel_capacity = " << max_channel_capacity << std::endl;
  return max_channel_capacity;
}

/**
 * @brief Get the Each Channel Capacity object
 *
 * @param my_distance
 * @param total_distance
 * @param max_capacity
 * @return int
 */
int GetEachChannelCapacity(int my_distance, int total_distance, int max_capacity) {
  return max_capacity * my_distance / total_distance;
}

#define M_PI_ 3.14159265358979323846
constexpr double kEarthRadius = 6371.0;  // 地球半径，单位为千米
double DegreeToRadian(double degree) { return degree * M_PI_ / 180.0; }

double Haversine(double radian) { return std::pow(std::sin(radian / 2.0), 2.0); }

/**
 * @brief Distance of two points in the Earth
 *
 * @param lat1 latitude
 * @param lng1 longitude
 * @param lat2 latitude
 * @param lng2 longitude
 * @return double
 */
double CppDistance(double lat1, double lon1, double lat2, double lon2) {
  double lat1_rad = DegreeToRadian(lat1);
  double lon1_rad = DegreeToRadian(lon1);
  double lat2_rad = DegreeToRadian(lat2);
  double lon2_rad = DegreeToRadian(lon2);
  double d_lat = lat2_rad - lat1_rad;
  double d_lon = lon2_rad - lon1_rad;
  double a = Haversine(d_lat) + std::cos(lat1_rad) * std::cos(lat2_rad) * Haversine(d_lon);
  double c = 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
  return kEarthRadius * c * 1000;
}

// std::vector<int> distance = {50, 100, 60, 90, 150};
// GetMaxChannelCapacity(distance);
