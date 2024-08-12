/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "rosbag.h"

CRosbag::CRosbag() {}

CRosbag::CRosbag(const CRosbag& other) { *this = other; }

CRosbag& CRosbag::operator=(const CRosbag& other) {
  m_strRosbagPath = other.m_strRosbagPath;

  return *this;
}

bool CRosbag::IsEmpty() {
  if (m_strRosbagPath.length() > 0) {
    return false;
  }

  return true;
}
