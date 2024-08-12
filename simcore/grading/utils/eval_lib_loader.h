// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

namespace eval {
/**
 * @brief library loader class, for reading shared library
 */
class LibraryLoader {
 public:
  static void *LoadSharedLibrary(const std::string &lib_path, bool deep_bind = false);
  static void UnLoadLibrary(void *lib);
  static void *GetLibraryFunc(const std::string &func_name, void *lib);
};
}  // namespace eval
