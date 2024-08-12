// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "utils/eval_lib_loader.h"

#include <stdexcept>

#ifdef _WIN32
#  include <Windows.h>
#  include <shlwapi.h>  // for PathIsRelative API
#elif defined __linux__
#  include <dlfcn.h>
#endif  // _WIN32

#ifdef _WIN32
static const std::string GetLastWinErrorString() {
  // Get the error message, if any.
  DWORD err = GetLastError();
  if (err == 0) return std::string();  // No error message has been recorded
  LPSTR buffer = nullptr;
  size_t size =
      FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
                     err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
  std::string message(buffer, size);
  // Free the buffer.
  LocalFree(buffer);
  return message;
}
#endif  // _WIN32

static const std::string GetLibError() {
  std::string dl_error;
#ifdef _WIN32
  dl_error = GetLastWinErrorString();
#elif defined __linux__
  dl_error = dlerror();
#else
  dl_error = "Un-supported platform for loading dynamic libraries.";
#endif  // _WIN32
  return dl_error;
}

namespace eval {
void *LibraryLoader::LoadSharedLibrary(const std::string &lib_path, bool deep_bind) {
  void *lib = nullptr;
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
  if (deep_bind)
    lib = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
  else
    lib = dlopen(lib_path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif  // _WIN32

  if (lib == nullptr) {
    throw std::runtime_error(GetLibError());
  }
  return lib;
}

void LibraryLoader::UnLoadLibrary(void *lib) {
  if (!lib) return;
  bool res = false;
#ifdef _WIN32
  res = FreeLibrary((HMODULE)lib);
#elif defined __linux__
  res = dlclose(lib) == 0;
#else
  res = false;
#endif  // _WIN32
  if (!res) throw std::runtime_error("failed to close library: " + GetLibError());
}

void *LibraryLoader::GetLibraryFunc(const std::string &func_name, void *lib) {
  void *func = nullptr;
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
}  // namespace eval
