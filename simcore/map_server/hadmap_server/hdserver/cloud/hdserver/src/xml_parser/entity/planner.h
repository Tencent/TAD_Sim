/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>
#include "common/xml_parser/entity/route.h"

class CPlannerParam;

struct sControlPathPoint {
  double m_dLon;
  double m_dLat;
  double m_dAlt;
  double m_dVelocity;
  int m_nGear;

  std::string m_strLon;
  std::string m_strLat;
  std::string m_strAlt;
  std::string m_strVelocity;
  std::string m_strGear;
  sControlPathPoint();
  void Reset();
  void ConvertToStr();
  void ConvertToValue();
};

struct sControlPath {
  double m_dSampleInterval;
  std::string m_strSampleInterval;
  std::vector<sControlPathPoint> m_vPoints;
  std::string m_strPoints;
  sControlPath();
  void Reset();
  void pushPoint(sControlPathPoint& pp);
  void ConvertToStr();
  void ConvertToValue();
};

class CPlanner {
 public:
  CPlanner();
  explicit CPlanner(CPlannerParam& pp);
  CPlanner& operator=(CPlannerParam& pp);
  CPlanner& operator=(const CPlanner& other);

  bool IsEmpty();
  void Reset();
  void ConvertToValue();
  void ConvertToStr();

  uint64_t m_ID;
  float m_fStartVelocity;
  float m_fStartAngle;
  float m_fMaxVelocity;
  double m_dStartAlt;
  double m_dEndAlt;
  int m_nType;
  std::string m_strStartVelocity;
  std::string m_strTheta;
  std::string m_strVelocityMax;
  std::string m_strStartAlt;
  std::string m_strEndAlt;
  CRoute m_route;
  std::string m_strType;

  sPath m_inputPath;
  sControlPath m_controlPath;

  int m_nUnrealLevel;
  std::string m_strUnrealLevel;
};

class CPlannerParam : public CPlanner {
 public:
  CPlannerParam& operator=(const CPlanner& p);

  int ComputePlannerParamNumber() {
    return std::ceil((m_fStartVelocityEnd - m_fStartVelocityStart) / m_fStartVelocitySep);
  }

  /*
  uint64_t	m_ID;
  float		m_fStartAngle;
  float		m_fMaxVelocity;
  double		m_dStartAlt;
  double		m_dEndAlt;

  std::string m_strTheta;
  std::string m_strVelocityMax;
  std::string	m_strStartAlt;
  std::string	m_strEndAlt;
  CRoute		m_route;

  sPath		m_inputPath;

  int			m_nUnrealLevel;
  std::string m_strUnrealLevel;

  float		m_fStartVelocity;
  std::string m_strStartVelocity;
  */
  float m_fStartVelocityStart;
  float m_fStartVelocityEnd;
  float m_fStartVelocitySep;
};
