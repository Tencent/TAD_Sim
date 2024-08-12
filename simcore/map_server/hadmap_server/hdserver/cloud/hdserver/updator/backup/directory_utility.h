/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/


#pragma once

#include <boost/filesystem.hpp>

class CDirectoryUtility {
 public:
  static bool CopyDir(boost::filesystem::path const& srcDir, boost::filesystem::path const& dstDir);

  static bool RemoveDir(boost::filesystem::path const& dstDir);
};
