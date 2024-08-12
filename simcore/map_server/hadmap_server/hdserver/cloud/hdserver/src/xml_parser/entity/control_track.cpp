/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "control_track.h"
#include <fstream>
#include <iostream>

sTagControlTrackPoint::sTagControlTrackPoint() { Reset(); }

void sTagControlTrackPoint::Reset() {
  m_dLon = 0;
  m_dLat = 0;
  m_dAlt = 0;
  m_dVelocity = 0;
  m_nGear = 0;
  m_dTime = 0;
  m_dTotalLength = 0;
  m_dTheta = 0;
  m_dAcceleration = 0;
}

CControlTrack::CControlTrack() { Clear(); }

void CControlTrack::Clear() { m_track.m_points.clear(); }

void CControlTrack::AddPoint(sTagControlTrackPoint& p) { m_track.m_points.push_back(p); }

int CControlTrack::Save(const char* fileName) {
  std::ofstream out;
  out.open(fileName);
  out.setf(std::ios::fixed);
  out.precision(8);

  sTagControlTrack::ControlTrackPoints::iterator itr = m_track.m_points.begin();
  for (; itr != m_track.m_points.end(); ++itr) {
    out << itr->m_dLat << " " << itr->m_dLon << " "  // position
        << itr->m_dTime << " "                       // time
        << itr->m_dVelocity << " " << 0 << " "       // velocity
        << itr->m_dTotalLength << " "                // total length
        << "0 " << itr->m_dTheta << " "              // theta
        << itr->m_dAcceleration << " "               // yaw
        << "0 0 0 0 0 0 0 0 "                        // unused
        << itr->m_nGear << std::endl;                // gear
  }
  out.close();
  return 0;
}
