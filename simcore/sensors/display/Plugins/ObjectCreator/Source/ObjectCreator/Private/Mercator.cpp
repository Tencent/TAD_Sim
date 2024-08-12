#pragma once
#include "Mercator.h"
#include "CoreMinimal.h"
#include <cmath>
#include <algorithm>

double tilesize()
{
    const double WGS84_a = 6378137.0;
    const double MERCATOR_LON_MAX = 180.0;
    const double MERCATOR_LAT_MAX = 85.05112878;
    const double PAI = 3.1415926535897932384626433832795;
    const double METER_ar = WGS84_a * PAI;
    const double METER_PER_DEGREE = METER_ar / 180.0;
    const int k = 13;
    const double tile_num = std::pow(2, k);
    return METER_ar * 2 / tile_num;
}

std::pair<int, int> xy2tile(double x, double y)
{
    const double WGS84_a = 6378137.0;
    const double PAI = 3.1415926535897932384626433832795;
    const double METER_ar = WGS84_a * PAI;
    const double tile_size = tilesize();
    const double x0 = 12127193.159612923238980142972475;    // xi`an north station
    const double y0 = 4079902.82174956756002801098634;      // xi`an north station
    int xx = int(std::floor((x + x0 + METER_ar) / tile_size));
    int yy = int(std::floor((y + y0 + METER_ar) / tile_size));
    return std::make_pair(xx, 8191 - yy);
}
std::pair<double, double> tilec(int x, int y)
{
    y = 8191 - y;
    const double WGS84_a = 6378137.0;
    const double PAI = 3.1415926535897932384626433832795;
    const double METER_ar = WGS84_a * PAI;
    const double tile_size = tilesize();
    const double x0 = 12127193.159612923238980142972475;    // xi`an north station
    const double y0 = 4079902.82174956756002801098634;      // xi`an north station
    double xx = tile_size * (x + 0.5) - METER_ar - x0;
    double yy = tile_size * (y + 0.5) - METER_ar - y0;
    return std::make_pair(std::round(xx), std::round(yy));
}

std::pair<double, double> tileo(int x, int y)
{
    y = 8191 - y;
    const double WGS84_a = 6378137.0;
    const double PAI = 3.1415926535897932384626433832795;
    const double METER_ar = WGS84_a * PAI;
    const double tile_size = tilesize();
    const double x0 = 12127193.159612923238980142972475;    // xi`an north station
    const double y0 = 4079902.82174956756002801098634;      // xi`an north station
    double xx = tile_size * x - METER_ar - x0;
    double yy = tile_size * y - METER_ar - y0;
    return std::make_pair(xx, yy);
}
