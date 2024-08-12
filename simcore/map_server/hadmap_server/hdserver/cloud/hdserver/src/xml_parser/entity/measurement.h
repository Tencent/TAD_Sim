/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <list>
#include <string>

struct sTagMeasurementPoint {
  double dLon;
  double dLat;
  double dAlt;
  sTagMeasurementPoint();
  void Reset();
};

typedef std::list<sTagMeasurementPoint> MeasurementNodes;

class CMeasurement {
 public:
  CMeasurement();

  CMeasurement& operator=(const CMeasurement& other);
  bool operator<(const CMeasurement& other);

  void ConvertToValue();
  void ConvertToStr();

  void Reset();

  void Set(uint64_t id);
  void Set(std::string strPositions);

 public:
  std::string m_strID;
  std::string m_strPositions;

  uint64_t m_ID;
  MeasurementNodes m_nodes;
};
