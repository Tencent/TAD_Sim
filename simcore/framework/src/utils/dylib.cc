// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "dylib.h"

#ifdef _WIN32
#  include <Windows.h>
#  include <shlwapi.h>  // for PathIsRelative API
#  include "os.h"       // for GetLastWinErrorString
#elif defined __linux__
#  include <dlfcn.h>
#endif  // _WIN32
#include <stdexcept>

static const std::string GetLibError() {
  std::string dl_error;
#ifdef _WIN32
  dl_error = tx_sim::utils::GetLastWinErrorString();
#elif defined __linux__
  dl_error = dlerror();
#else
  dl_error = "Un-supported platform for loading dynamic libraries.";
#endif  // _WIN32
  return dl_error;
}

namespace tx_sim {
namespace utils {

void* LoadSharedLibrary(const std::string& lib_path, bool deep_bind) {
  void* lib = nullptr;
#ifdef _WIN32
  DWORD load_flags = LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;
  if (!PathIsRelative(lib_path.c_str())) {
    // for absolute path we should also search the dependencies in the direcotry that contains the loaded dll.
    load_flags |= LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR;
  }
  lib = LoadLibraryEx(lib_path.c_str(), NULL, load_flags);
#elif defined __linux__
  // dlmopen causes getpwuid() system call, etc. crash.
  // lib = dlmopen(LM_ID_NEWLM, lib_path.c_str(), RTLD_NOW | RTLD_LOCAL);
  // see https://sourceware.org/glibc/wiki/LinkerNamespaces for details.

  // RTLD_DEEPBIND requires the executable build with -fPIC.
  if (deep_bind) lib = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
  else
    lib = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif  // _WIN32

  if (lib == nullptr) { throw std::runtime_error(GetLibError()); }
  return lib;
}

void UnLoadLibrary(void* lib) {
  if (!lib) return;
#ifdef _WIN32
  if (!FreeLibrary((HMODULE)lib)) {
#elif defined __linux__
  if (dlclose(lib) != 0) {
#else
  if (false) {
#endif  // _WIN32
    throw std::runtime_error("failed to close library: " + GetLibError());
  }
}

void* GetLibraryFunc(const std::string& func_name, void* lib) {
  void* func = nullptr;
#ifdef _WIN32
  func = GetProcAddress((HMODULE)lib, func_name.c_str());
#elif defined __linux__
  func = dlsym(lib, func_name.c_str());
#endif  // _WIN32

  if (func == nullptr) {
    throw std::runtime_error("could not load the function from library: " + func_name + "\nreason: " + GetLibError());
  }
  return func;
}

}  // namespace utils
}  // namespace tx_sim
