/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <string>

class CRosbag {
 public:
  CRosbag();
  CRosbag(const CRosbag& other);
  CRosbag& operator=(const CRosbag& other);

  bool IsEmpty();
  const char* Path() { return m_strRosbagPath.c_str(); }
  void SetPath(const char* strPath) { m_strRosbagPath = strPath; }

 protected:
  std::string m_strRosbagPath;
};
