// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <math.h>
#include <vector>

#define PI 3.14159265359
#define INV_PI (0.31830988618f)
#define HALF_PI (1.57079632679f)

inline bool DoubleValueCheck(double value, double equalValue, double deff) {
  return (value > (equalValue - deff) && (value < (equalValue + deff))) ? true : false;
}

inline bool ZeroValueCheck(double value) { return DoubleValueCheck(value, 0.0, 0.0001); }

struct Vertex3 {
  Vertex3() {
    x = 0;
    y = 0;
    z = 0;
  }
  Vertex3(double _x, double _y, double _z) { SetData(_x, _y, _z); }
  void SetData(double _x, double _y, double _z) {
    x = _x;
    y = _y;
    z = _z;
  }
  Vertex3 normalized();
  Vertex3 RotateAngleAxis(const float AngleDeg, const Vertex3 &Axis);
  float Magnitude();
  double x, y, z;
};

struct ColorVertex3 {
  ColorVertex3() { ColorVertex3(1.0, 1.0, 1.0); }

  ColorVertex3(float r, float g, float b) {
    rColor = r;
    gColor = g;
    bColor = b;
  }

  float rColor, gColor, bColor;
  Vertex3 vPos;
};

struct ColorLineList {
  float rColor, gColor, bColor;
  std::vector<Vertex3> vPosList;
};

struct Rotator {
  Rotator() {
    Roll = 0;
    Pitch = 0;
    Yaw = 0;
  }
  float Roll, Pitch, Yaw;
};

struct BBox {
  BBox() {
    vMin = Vertex3(10000, 10000, 10000);
    vMax = Vertex3(-10000, -10000, -10000);
  }

  Vertex3 GetSize() { return Vertex3(vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z); }

  Vertex3 GetCenter() { return Vertex3((vMax.x + vMin.x) * 0.5, (vMax.y + vMin.y) * 0.5, (vMax.z + vMin.z) * 0.5); }

  void UpdateData(Vertex3 vPos) {
    if (vPos.x < vMin.x)
      vMin.x = vPos.x;
    else if (vPos.x > vMax.x)
      vMax.x = vPos.x;

    if (vPos.y < vMin.y)
      vMin.y = vPos.y;
    else if (vPos.y > vMax.y)
      vMax.y = vPos.y;

    if (vPos.z < vMin.z)
      vMin.z = vPos.z;
    else if (vPos.z > vMax.z)
      vMax.z = vPos.z;
  }

  double GetMaxSize() {
    Vertex3 vSize = GetSize();
    return (vSize.x > vSize.z) ? vSize.x : vSize.z;
  }

  Vertex3 vMin;
  Vertex3 vMax;
};

struct Quaterniond {
  explicit Quaterniond(double _x = 0, double _y = 0, double _z = 0, double _w = 0) : x(_x), y(_y), z(_z), w(_w) {}
  void FindBetweenVectors(Vertex3 vVector1, Vertex3 vVector2);
  double x, y, z, w;
};

Vertex3 VectorCross(Vertex3 vVector1, Vertex3 vVector2);
float VectorDot(Vertex3 vVector1, Vertex3 vVector2);
Vertex3 VectorAdd(Vertex3 vVector1, Vertex3 vVector2);
Vertex3 VectorSub(Vertex3 vVector1, Vertex3 vVector2);
Vertex3 VectorMulti(Vertex3 vVector, float value);
float VectorMagnitude(float x, float y, float z);
float VectorLength(Vertex3 vVector1, Vertex3 vVector2);
Vertex3 VectorNormal(Vertex3 vPos1, Vertex3 vPos2, Vertex3 vPos3);
double AngleBetweenVectors(Vertex3 Vector1, Vertex3 Vector2);
bool IsPointOnLine(Vertex3 linePointA, Vertex3 linePointB, Vertex3 point);
float RandomRange(float startValue, float endValue);
double Power(double _value, int count);
void toEulerAngle(const Quaterniond &q, double &roll, double &pitch, double &yaw);
bool CheckPointInBox(Vertex3 vPos, Vertex3 vBox, float size);
bool CheckRectInRect(Vertex3 vPos1, float r1Size, Vertex3 vPos2, float r2Size);
bool IntersectedPolygon(Vertex3 vPoly[], Vertex3 vLine[], Vertex3 *vIntersection, int verticeCount);
bool lineIntersection(Vertex3 v1, Vertex3 v2, Vertex3 v3, Vertex3 v4, Vertex3 &result);
void FixAngle(float &angle);
