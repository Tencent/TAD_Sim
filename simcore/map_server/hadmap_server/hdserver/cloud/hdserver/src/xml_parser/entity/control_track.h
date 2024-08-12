/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
#include <string>
#include <vector>

struct sTagControlTrackPoint {
  double m_dLon;
  double m_dLat;
  double m_dAlt;
  double m_dVelocity;
  int m_nGear;
  double m_dTime;
  double m_dTotalLength;
  double m_dTheta;
  double m_dAcceleration;

  sTagControlTrackPoint();

  void Reset();
};

struct sTagControlTrack {
  typedef std::vector<sTagControlTrackPoint> ControlTrackPoints;

  ControlTrackPoints m_points;
};

class CControlTrack {
 public:
  CControlTrack();
  void Clear();

  int Save(const char* strFileName);

  void AddPoint(sTagControlTrackPoint& p);

  sTagControlTrack& Path() { return m_track; }

 private:
  sTagControlTrack m_track;
};
