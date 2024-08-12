// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_math.h"
#include <stdlib.h>
#include <cmath>

Vertex3 orthogonal(Vertex3 v);
bool IntersectedPlane(Vertex3 vPoly[], Vertex3 vLine[], Vertex3 &vNormal, float &originDistance);
Vertex3 Normal(Vertex3 vTriangle[]);
float PlaneDistance(Vertex3 vNormal, Vertex3 vPoint);
Vertex3 IntersectionPoint(Vertex3 vNormal, Vertex3 vLine[], double distance);
bool InsidePolygon(Vertex3 vIntersection, Vertex3 Poly[], int64_t verticeCount);
inline void SinCos(float *ScalarSin, float *ScalarCos, float Value) {
  // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
  float quotient = (INV_PI * 0.5f) * Value;
  if (Value >= 0.0f) {
    quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
  } else {
    quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
  }
  float y = Value - (2.0f * PI) * quotient;

  // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
  float sign;
  if (y > HALF_PI) {
    y = PI - y;
    sign = -1.0f;
  } else if (y < -HALF_PI) {
    y = -PI - y;
    sign = -1.0f;
  } else {
    sign = +1.0f;
  }

  float y2 = y * y;

  // 11-degree minimax approximation
  *ScalarSin =
      (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) *
           y2 +
       1.0f) *
      y;

  // 10-degree minimax approximation
  float p =
      ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
  *ScalarCos = sign * p;
}

Vertex3 Vertex3::normalized() {
  double fMagnitude = VectorMagnitude(x, y, z);
  if (!ZeroValueCheck(fMagnitude)) {
    x /= fMagnitude;
    y /= fMagnitude;
    z /= fMagnitude;
  } else {
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
  }

  return Vertex3(x, y, z);
}

float Vertex3::Magnitude() { return VectorMagnitude(x, y, z); }

Vertex3 Vertex3::RotateAngleAxis(const float AngleDeg, const Vertex3 &Axis) {
  float S, C;
  SinCos(&S, &C, AngleDeg * (PI / 180.f));

  const float XX = Axis.x * Axis.x;
  const float YY = Axis.y * Axis.y;
  const float ZZ = Axis.z * Axis.z;

  const float XY = Axis.x * Axis.y;
  const float YZ = Axis.y * Axis.z;
  const float ZX = Axis.z * Axis.x;

  const float XS = Axis.x * S;
  const float YS = Axis.y * S;
  const float ZS = Axis.z * S;

  const float OMC = 1.f - C;

  return Vertex3((OMC * XX + C) * x + (OMC * XY - ZS) * y + (OMC * ZX + YS) * z,
                 (OMC * XY + ZS) * x + (OMC * YY + C) * y + (OMC * YZ - XS) * z,
                 (OMC * ZX - YS) * x + (OMC * YZ + XS) * y + (OMC * ZZ + C) * z);
}

void Quaterniond::FindBetweenVectors(Vertex3 vVector1, Vertex3 vVector2) {
  Vertex3 start = vVector1.normalized();
  Vertex3 dest = vVector2.normalized();
  float cosTheta = VectorDot(start, dest);
  if (cosTheta < -1 + 0.001f) {
    Vertex3 rotationAxis = VectorCross(Vertex3(0.0f, 0.0f, 1.0f), start);
    if (rotationAxis.Magnitude() < 0.01) rotationAxis = VectorCross(Vertex3(1.0f, 0.0f, 0.0f), start);

    rotationAxis.normalized();
    w = PI * 0.5;
    x = rotationAxis.x;
    y = rotationAxis.y;
    z = rotationAxis.z;
  } else {
    Vertex3 rotationAxis = VectorCross(start, dest);
    float s = sqrt((1 + cosTheta) * 2);
    float invs = 1 / s;
    w = s * 0.5;
    x = rotationAxis.x * invs;
    y = rotationAxis.y * invs;
    z = rotationAxis.z * invs;
  }
}

Vertex3 orthogonal(Vertex3 v) {
  float x = std::abs(v.x);
  float y = std::abs(v.y);
  float z = std::abs(v.z);

  Vertex3 other = x < y ? (x < z ? Vertex3(1, 0, 0) : Vertex3(0, 0, 1)) : (y < z ? Vertex3(0, 1, 0) : Vertex3(0, 0, 1));
  return VectorCross(v, other);
}

Vertex3 VectorCross(Vertex3 vVector1, Vertex3 vVector2) {
  Vertex3 vCross;
  vCross.x = ((vVector1.y * vVector2.z) - (vVector1.z * vVector2.y));
  vCross.y = ((vVector1.z * vVector2.x) - (vVector1.x * vVector2.z));
  vCross.z = ((vVector1.x * vVector2.y) - (vVector1.y * vVector2.x));
  return vCross;
}

float VectorDot(Vertex3 vVector1, Vertex3 vVector2) {
  return ((vVector1.x * vVector2.x) + (vVector1.y * vVector2.y) + (vVector1.z * vVector2.z));
}

Vertex3 VectorAdd(Vertex3 vVector1, Vertex3 vVector2) {
  return Vertex3(vVector1.x + vVector2.x, vVector1.y + vVector2.y, vVector1.z + vVector2.z);
}

Vertex3 VectorSub(Vertex3 vVector1, Vertex3 vVector2) {
  return Vertex3(vVector1.x - vVector2.x, vVector1.y - vVector2.y, vVector1.z - vVector2.z);
}

Vertex3 VectorMulti(Vertex3 vVector, float value) {
  return Vertex3(vVector.x * value, vVector.y * value, vVector.z * value);
}

float VectorMagnitude(float x, float y, float z) { return sqrtf(x * x + y * y + z * z); }

float VectorLength(Vertex3 vVector1, Vertex3 vVector2) {
  Vertex3 vDir = VectorSub(vVector1, vVector2);
  return vDir.Magnitude();
}

Vertex3 VectorNormal(Vertex3 vPos1, Vertex3 vPos2, Vertex3 vPos3) {
  Vertex3 v1 = Vertex3(vPos2.x - vPos1.x, vPos2.y - vPos1.y, vPos2.z - vPos1.z);
  Vertex3 v2 = Vertex3(vPos3.x - vPos1.x, vPos3.y - vPos1.y, vPos3.z - vPos1.z);
  Vertex3 vNormal = VectorCross(v2, v1);
  vNormal.normalized();
  return vNormal;
}

double AngleBetweenVectors(Vertex3 Vector1, Vertex3 Vector2) {
  float dotProduct = VectorDot(Vector1, Vector2);
  float vectorsMagnitude = Vector1.Magnitude() * Vector2.Magnitude();
  double angle = acos(dotProduct / vectorsMagnitude);

  if (std::isnan(angle)) return 0;

  return (angle);
}

bool IsPointOnLine(Vertex3 linePointA, Vertex3 linePointB, Vertex3 point) {
  float A = point.x - linePointA.x;
  float B = point.z - linePointA.z;
  float C = linePointB.x - linePointA.x;
  float D = linePointB.z - linePointA.z;
  float dot = A * C + B * D;
  float len = C * C + D * D;
  float para = dot / len;
  float x, z;
  if (para < 0) {
    x = linePointA.x;
    z = linePointA.z;
  } else if (para > 1) {
    x = linePointB.x;
    z = linePointB.z;
  } else {
    x = linePointA.x + para * C;
    z = linePointA.z + para * D;
  }

  float xdif = point.x - x;
  float zdif = point.z - z;
  float hypo = sqrt((xdif * xdif) + (zdif * zdif));
  if (hypo < 2) return true;

  return false;
}

float RandomRange(float startValue, float endValue) {
  float randValue = (rand() % 10000) * 0.0001;
  return (startValue + (endValue - startValue) * randValue);
  // return startValue;
}

double Power(double _value, int count) {
  double value = _value;
  for (unsigned int i = 0; i < count - 1; i++) value *= _value;

  return value;
}

void toEulerAngle(const Quaterniond &q, double &roll, double &pitch, double &yaw) {
  // roll (x-axis rotation)
  double sinr = 2.0 * (q.w * q.x + q.y * q.z);
  double cosr = 1.0 - 2.0 * (q.x * q.x + q.y * q.y);
  roll = atan2(sinr, cosr);

  // pitch (y-axis rotation)
  double sinp = 2.0 * (q.w * q.y - q.z * q.x);
  if (fabs(sinp) >= 1)
    pitch = copysign(PI / 2, sinp);  // use 90 degrees if out of range
  else
    pitch = asin(sinp);

  // yaw (z-axis rotation)
  double siny = +2.0 * (q.w * q.z + q.x * q.y);
  double cosy = +1.0 - 2.0 * (q.y * q.y + q.z * q.z);
  yaw = atan2(siny, cosy);
}

bool CheckPointInBox(Vertex3 vPos, Vertex3 vBox, float size) {
  if (vPos.x >= (vBox.x - size * 0.5) && vPos.x <= (vBox.x + size * 0.5) && vPos.z >= (vBox.z - size * 0.5) &&
      vPos.z <= (vBox.z + size * 0.5))
    return true;

  return false;
}

bool CheckRectInRect(Vertex3 vPos1, float r1Size, Vertex3 vPos2, float r2Size) {
  Vertex3 vPos[4] = {Vertex3(vPos1.x - r1Size * 0.5, 0, vPos1.z - r1Size * 0.5),
                     Vertex3(vPos1.x + r1Size * 0.5, 0, vPos1.z - r1Size * 0.5),
                     Vertex3(vPos1.x + r1Size * 0.5, 0, vPos1.z + r1Size * 0.5),
                     Vertex3(vPos1.x - r1Size * 0.5, 0, vPos1.z + r1Size * 0.5)};

  for (unsigned int i = 0; i < 4; i++) {
    if (vPos[i].x >= vPos2.x - r2Size * 0.5 && vPos[i].x <= vPos2.x + r2Size * 0.5 &&
        vPos[i].z >= vPos2.z - r2Size * 0.5 && vPos[i].z <= vPos2.z + r2Size * 0.5) {
      return true;
    }
  }

  return false;
}

bool IntersectedPolygon(Vertex3 vPoly[], Vertex3 vLine[], Vertex3 *vIntersection, int verticeCount) {
  Vertex3 vNormal = Vertex3(0, 0, 0);
  float originDistance = 0;

  if (!IntersectedPlane(vPoly, vLine, vNormal, originDistance)) return false;

  Vertex3 vTestPos = IntersectionPoint(vNormal, vLine, originDistance);
  if (InsidePolygon(vTestPos, vPoly, verticeCount)) {
    *vIntersection = vTestPos;
    return true;
  }

  return false;
}

bool IntersectedPlane(Vertex3 vPoly[], Vertex3 vLine[], Vertex3 &vNormal, float &originDistance) {
  vNormal = Normal(vPoly);
  originDistance = PlaneDistance(vNormal, vPoly[0]);
  float distance1 = ((vNormal.x * vLine[0].x) + (vNormal.y * vLine[0].y) + (vNormal.z * vLine[0].z)) + originDistance;
  float distance2 = ((vNormal.x * vLine[1].x) + (vNormal.y * vLine[1].y) + (vNormal.z * vLine[1].z)) + originDistance;
  return (distance1 * distance2 >= 0) ? false : true;
}

Vertex3 Normal(Vertex3 vTriangle[]) {
  Vertex3 v1 = VectorSub(vTriangle[2], vTriangle[0]);
  Vertex3 v2 = VectorSub(vTriangle[1], vTriangle[0]);
  Vertex3 vNormal = VectorCross(v1, v2);
  vNormal = vNormal.normalized();
  return vNormal;
}

float PlaneDistance(Vertex3 vNormal, Vertex3 vPoint) {
  float distance = 0;
  distance = -((vNormal.x * vPoint.x) + (vNormal.y * vPoint.y) + (vNormal.z * vPoint.z));
  return distance;
}

Vertex3 IntersectionPoint(Vertex3 vNormal, Vertex3 vLine[], double distance) {
  Vertex3 vPoint = Vertex3(0, 0, 0), vLineDir = Vertex3(0, 0, 0);
  double Numerator = 0.0, Denominator = 0.0, dist = 0.0;
  vLineDir = VectorSub(vLine[1], vLine[0]);
  vLineDir = vLineDir.normalized();
  Numerator = -(vNormal.x * vLine[0].x + vNormal.y * vLine[0].y + vNormal.z * vLine[0].z + distance);
  Denominator = VectorDot(vNormal, vLineDir);
  if (Denominator == 0.0) return vLine[0];

  dist = Numerator / Denominator;
  vPoint.x = static_cast<float>(vLine[0].x + (vLineDir.x * dist));
  vPoint.y = static_cast<float>(vLine[0].y + (vLineDir.y * dist));
  vPoint.z = static_cast<float>(vLine[0].z + (vLineDir.z * dist));
  return vPoint;
}

bool InsidePolygon(Vertex3 vIntersection, Vertex3 Poly[], int64_t verticeCount) {
  const double MATCH_FACTOR = 0.9999;
  double Angle = 0.0;
  Vertex3 vA, vB;

  for (int i = 0; i < verticeCount; i++) {
    vA = VectorSub(Poly[i], vIntersection);
    vB = VectorSub(Poly[(i + 1) % verticeCount], vIntersection);
    Angle += AngleBetweenVectors(vA, vB);
  }

  return (Angle >= (MATCH_FACTOR * (2.0 * PI))) ? true : false;
}

bool lineIntersection(Vertex3 v1, Vertex3 v2, Vertex3 v3, Vertex3 v4, Vertex3 &result) {
  float A1 = v2.z - v1.z;
  float B1 = v1.x - v2.x;
  float C1 = A1 * v1.x + B1 * v1.z;
  float A2 = v4.z - v3.z;
  float B2 = v3.x - v4.x;
  float C2 = A2 * v3.x + B2 * v3.z;
  float det = A1 * B2 - A2 * B1;
  if (det != 0) {
    float x = (B2 * C1 - B1 * C2) / det;
    float z = (A1 * C2 - A2 * C1) / det;
    if (x >= fmin(v1.x, v2.x) && x <= fmax(v1.x, v2.x) && x >= fmin(v3.x, v4.x) && x <= fmax(v3.x, v4.x) &&
        z >= fmin(v1.z, v2.z) && z <= fmax(v1.z, v2.z) && z >= fmin(v3.z, v4.z) && z <= fmax(v3.z, v4.z)) {
      result.x = x;
      result.z = z;
      return true;
    }
  }

  return false;
}

void FixAngle(float &angle) {
  if (angle > 360)
    angle -= 360;
  else if (angle < 0)
    angle += 360;
}
