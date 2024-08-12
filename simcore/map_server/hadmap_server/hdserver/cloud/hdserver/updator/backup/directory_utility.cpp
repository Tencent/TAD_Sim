/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "directory_utility.h"
#include <iostream>
#include "common/log/system_logger.h"

bool CDirectoryUtility::CopyDir(boost::filesystem::path const& source, boost::filesystem::path const& destination) {
  namespace fs = boost::filesystem;
  try {
    // Check whether the function call is valid
    if (!fs::exists(source) || !fs::is_directory(source)) {
      std::cerr << "Source directory " << source.string() << " does not exist or is not a directory." << '\n';
      return false;
    }
    if (fs::exists(destination)) {
      std::cerr << "Destination directory " << destination.string() << " already exists." << '\n';
      return false;
    }
    // Create the destination directory
    if (!fs::create_directory(destination)) {
      std::cerr << "Unable to create destination directory" << destination.string() << '\n';
      return false;
    }
  } catch (fs::filesystem_error const& e) {
    std::cerr << e.what() << '\n';
    return false;
  }
  // Iterate through the source directory
  for (fs::directory_iterator file(source); file != fs::directory_iterator(); ++file) {
    try {
      fs::path current(file->path());
      if (fs::is_directory(current)) {
        // Found directory: Recursion
        if (!CopyDir(current, destination / current.filename())) {
          return false;
        }
      } else {
        // Found file: Copy
        fs::copy_file(current, destination / current.filename());
      }
    } catch (fs::filesystem_error const& e) {
      std::cerr << e.what() << '\n';
    }
  }
  return true;
}

bool CDirectoryUtility::RemoveDir(boost::filesystem::path const& destination) {
  namespace fs = boost::filesystem;
  try {
    // Check whether the function call is valid
    if (!fs::exists(destination) || !fs::is_directory(destination)) {
      std::cerr << "directory " << destination.string() << " does not exist or is not a directory." << '\n';
      return false;
    }
  } catch (fs::filesystem_error const& e) {
    std::cerr << e.what() << '\n';
    return false;
  }
  // Iterate through the source directory
  for (fs::directory_iterator file(destination); file != fs::directory_iterator(); ++file) {
    try {
      fs::path current(file->path());
      if (fs::is_directory(current)) {
        // Found directory: Recursion
        if (!RemoveDir(current)) {
          return false;
        }
        fs::remove(current);
      } else {
        // Found file: Remove
        fs::remove(current);
      }
    } catch (fs::filesystem_error const& e) {
      std::cerr << e.what() << '\n';
    }
  }

  fs::remove(destination);
  return true;
}
