/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#include "common/engine/math/utils.h"
#include <time.h>

#include <cmath>

#include <boost/format.hpp>

const double PI = 3.14159265359;

const double roudu = 57.2957795130823210;
const double roufen = 3437.74677078493917;
const double roumiao = 206264.8062470963551565;

// WGS84椭球参数
const double WGS84_a = 6378137.0;
const double WGS84_e = 0.003352810664;
const double WGS84_b = WGS84_a - WGS84_a * WGS84_e;
const double WGS84_c = 6399593.6258;
const double WGS84_e2 = 0.0066943799013;
const double WGS84_epie2 = 0.00673949674227;

// 米/度
const double meter_per_degree = WGS84_a * PI / 180.0;

using namespace std;

void CUtils::ll_to_meter(double& x, double& y, double& z) {
  double lon = x;
  double lat = y;
  double h = z;
  double phi = lat / roudu;
  double lam = lon / roudu;
  double sphi = sin(phi);
  double cphi = abs(lat) == 90 ? 0 : cos(phi);
  double n = WGS84_a / sqrt(1 - WGS84_e2 * sphi * sphi);
  double slam = lon == -180 ? 0 : sin(lam);
  double clam = abs(lon) == 90 ? 0 : cos(lam);

  z = ((1 - WGS84_e2) * n + h) * sphi;
  x = (n + h) * cphi;
  y = x * slam;
  x *= clam;
}

void CUtils::meter_to_ll(double& x, double& y, double& z) {
  double L = atan2(y, x);

  double B0 = atan2(z * (1 + WGS84_e2), sqrt(x * x + y * y));

  double Th = 1E-10;  // condition to finish the iteration
  double dDiff = 1;
  double tmpTan = z / sqrt(x * x + y * y);
  while (dDiff > Th) {
    double tmp = WGS84_a * WGS84_e2 * sin(B0) / (z * sqrt(1 - WGS84_e2 * sin(B0) * sin(B0)));
    double B = atan(tmpTan * (1 + tmp));
    dDiff = fabs(B - B0);
    B0 = B;
  }
  double W = sqrt(1 - WGS84_e2 * sin(B0) * sin(B0));
  double N = WGS84_a / W;

  double dis = sqrt(x * x + y * y) / cos(B0);
  z = dis - N;
  x = L * 180. / PI;
  y = B0 * 180. / PI;
}

bool CUtils::is_zero(float f) {
  static float fDelta = 0.000001f;
  if (f < fDelta && f > -fDelta) {
    return true;
  }

  return false;
}

std::string CUtils::getTime() {
  time_t timep;
  tm tt;
  time(&timep);
#ifdef _WINDOWS
  localtime_s(&tt, &timep);
#else
  localtime_r(&timep, &tt);
#endif
  char tmp[64];
  strftime(tmp, sizeof(tmp), "%Y%m%d%H%M%S", &tt);
  return tmp;
}

std::string CUtils::doubleToStringDot3(double d) {
  static boost::format fmtPrecision3("%.3f");
  fmtPrecision3.clear();
  fmtPrecision3 % d;
  return fmtPrecision3.str();
}
