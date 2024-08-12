// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "coord_trans.h"
#include <stdio.h>
#include <algorithm>
#include <cmath>

using namespace std;

namespace coord_trans_api {
#ifndef M_PI
const double M_PI = 3.141592653589;
#endif
const double roudu = 57.2957795130823210;
const double roufen = 3437.74677078493917;
const double roumiao = 206264.8062470963551565;

const double WGS84_a = 6378137.0;
const double WGS84_e = 0.003352810664;
const double WGS84_b = WGS84_a - WGS84_a * WGS84_e;
const double WGS84_c = 6399593.6258;
const double WGS84_e2 = 0.0066943799013;
const double WGS84_epie2 = 0.00673949674227;

const double MERCATOR_LON_MAX = 180.0;
const double MERCATOR_LAT_MAX = 85.05112878;

const double METER_PER_DEGREE = WGS84_a * M_PI / 180.0;

struct MatrixOperation {
  void mult_matrix_333(const double* A, const double* B, double* Result) {
    Result[0] = A[0] * B[0] + A[1] * B[3] + A[2] * B[6];
    Result[1] = A[0] * B[1] + A[1] * B[4] + A[2] * B[7];
    Result[2] = A[0] * B[2] + A[1] * B[5] + A[2] * B[8];
    Result[3] = A[3] * B[0] + A[4] * B[3] + A[5] * B[6];
    Result[4] = A[3] * B[1] + A[4] * B[4] + A[5] * B[7];
    Result[5] = A[3] * B[2] + A[4] * B[5] + A[5] * B[8];
    Result[6] = A[6] * B[0] + A[7] * B[3] + A[8] * B[6];
    Result[7] = A[6] * B[1] + A[7] * B[4] + A[8] * B[7];
    Result[8] = A[6] * B[2] + A[7] * B[5] + A[8] * B[8];
  }
  void mult_matrix_331(const double* A, const double* B, double* Result) {
    Result[0] = A[0] * B[0] + A[1] * B[1] + A[2] * B[2];
    Result[1] = A[3] * B[0] + A[4] * B[1] + A[5] * B[2];
    Result[2] = A[6] * B[0] + A[7] * B[1] + A[8] * B[2];
  }

  void zx_to_dx_matrix(double L, double B, double H, double* m) {
    double newL, newB;
    newL = L / roudu;
    newB = B / roudu;

    double R1[9], R2[9];

    R1[0] = cos(M_PI / 2 - newB);
    R1[1] = 0;
    R1[2] = sin(M_PI / 2 - newB);
    R1[3] = 0;
    R1[4] = 1;
    R1[5] = 0;
    R1[6] = -R1[2];
    R1[7] = 0;
    R1[8] = R1[0];

    R2[0] = cos(newL);
    R2[1] = -sin(newL);
    R2[2] = 0;
    R2[3] = -R2[1];
    R2[4] = R2[0];
    R2[5] = 0;
    R2[6] = 0;
    R2[7] = 0;
    R2[8] = 1;

    mult_matrix_333(R2, R1, m);
  }

  void dx_to_zx_matrix(double L, double B, double H, double* m) {
    double newL, newB;
    newL = L / roudu;
    newB = B / roudu;

    double R1[9], R2[9];

    R2[0] = cos(newL);
    R2[1] = sin(newL);
    R2[2] = 0;
    R2[3] = -R2[1];
    R2[4] = R2[0];
    R2[5] = 0;
    R2[6] = 0;
    R2[7] = 0;
    R2[8] = 1;

    R1[0] = cos(M_PI / 2 - newB);
    R1[1] = 0;
    R1[2] = -sin(M_PI / 2 - newB);
    R1[3] = 0;
    R1[4] = 1;
    R1[5] = 0;
    R1[6] = -R1[2];
    R1[7] = 0;
    R1[8] = R1[0];

    mult_matrix_333(R1, R2, m);
  }
};

void lonlat2global(double& x, double& y, double& z) {
  double lon = x;
  double lat = y;
  double h = z;
  double phi = lat / roudu;
  double lam = lon / roudu;
  double sphi = sin(phi);
  double cphi = cos(phi);
  double slam = sin(lam);
  double clam = cos(lam);
  double n = WGS84_a / sqrt(1 - WGS84_e2 * sphi * sphi);
  z = ((1 - WGS84_e2) * n + h) * sphi;
  x = (n + h) * cphi;
  y = x * slam;
  x *= clam;
}

void global2lonlat(double& x, double& y, double& z) {
  double a = WGS84_a;
  double b = WGS84_b;
  double c = sqrt(((a * a) - (b * b)) / (a * a));
  double d = sqrt(((a * a) - (b * b)) / (b * b));
  double p = sqrt((x * x) + (y * y));
  double q = atan2((z * a), (p * b));
  double Longitude = atan2(y, x);
  double Latitude = atan2((z + (d * d) * b * pow(sin(q), 3)), (p - (c * c) * a * pow(cos(q), 3)));
  double N = a / sqrt(1 - ((c * c) * pow(sin(Latitude), 2)));
  z = (p / cos(Latitude)) - N;
  x = Longitude * roudu;
  y = Latitude * roudu;
}

void global2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  MatrixOperation mo;

  double R[9];
  mo.dx_to_zx_matrix(refLon, refLat, refAlt, R);

  lonlat2global(refLon, refLat, refAlt);

  double X[3], rt[3];
  X[0] = x - refLon;
  X[1] = y - refLat;
  X[2] = z - refAlt;

  mo.mult_matrix_331(R, X, rt);

  x = rt[0];
  y = rt[1];
  z = rt[2];
}

void local2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  MatrixOperation mo;

  double R[9];
  mo.zx_to_dx_matrix(refLon, refLat, refAlt, R);

  lonlat2global(refLon, refLat, refAlt);

  double X[3], rt[3];

  X[0] = x;
  X[1] = y;
  X[2] = z;

  mo.mult_matrix_331(R, X, rt);

  x = rt[0] + refLon;
  y = rt[1] + refLat;
  z = rt[2] + refAlt;
}

void lonlat2local(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  lonlat2global(x, y, z);
  global2local(x, y, z, refLon, refLat, refAlt);
}

void local2lonlat(double& lon, double& lat, double& ele, double refLon, double refLat, double refAlt) {
  local2global(lon, lat, ele, refLon, refLat, refAlt);

  global2lonlat(lon, lat, ele);
}

void global2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  global2local(x, y, z, refLon, refLat, refAlt);

  seu2enu(x, y, z, x, y, z);
}

void enu2global(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  enu2seu(x, y, z, x, y, z);

  local2global(x, y, z, refLon, refLat, refAlt);
}

void lonlat2enu(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  lonlat2local(x, y, z, refLon, refLat, refAlt);

  seu2enu(x, y, z, x, y, z);
}

void enu2lonlat(double& x, double& y, double& z, double refLon, double refLat, double refAlt) {
  enu2seu(x, y, z, x, y, z);

  local2lonlat(x, y, z, refLon, refLat, refAlt);
}

void seu2enu(double& enuX, double& enuY, double& enuZ, double seuX, double seuY, double seuZ) {
  enuX = seuY;
  enuY = -seuX;
  enuZ = seuZ;
}

void enu2seu(double& seuX, double& seuY, double& seuZ, double enuX, double enuY, double enuZ) {
  seuX = -enuY;
  seuY = enuX;
  seuZ = enuZ;
}

void lonlat2mercator(double& x, double& y) {
  y = std::min(std::max(y, -MERCATOR_LAT_MAX), MERCATOR_LAT_MAX);
  x = std::min(std::max(x, -MERCATOR_LON_MAX), MERCATOR_LON_MAX);

  x *= METER_PER_DEGREE;
  y = std::log(std::tan((90.0 + y) * M_PI / 360.0)) * WGS84_a;
}

void mercator2lonlat(double& x, double& y) {
  x /= METER_PER_DEGREE;
  y = std::atan(std::exp(y / WGS84_a)) * 360.0 / M_PI - 90.0;
}
}  // namespace coord_trans_api
