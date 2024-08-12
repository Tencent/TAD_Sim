// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <fstream>
#include <iostream>
#include <string>

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/util/json_util.h>

#include "model_hooks.h"

#ifdef _WIN32
#  include <windows.h>
#else
#  include <dlfcn.h>
#endif  // _WIN32

namespace tx_car {
/* load string from file path*/
bool loadDataFromFile(std::string& content, const std::string& file_path) {
  std::ifstream in(file_path, std::ios::in);
  content.clear();

  if (in.is_open()) {
    content = std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    return true;
  }

  return false;
}

// json string to protobuf message
bool jsonToProto(const std::string& json_str, google::protobuf::Message& message) {
  return google::protobuf::util::JsonStringToMessage(json_str, &message).ok();
}

// dll handler to work with vehicle model dll
struct VehicleModel_DLL_Handle {
  VehicleModel_DLL_Handle() : m_dllHandle(nullptr) {}
  ~VehicleModel_DLL_Handle() { freeVehicleLibrary(); }

  // load library
  bool loadVehicleLibrary(const std::string& dllFullPath) {
    m_dllFullPath = dllFullPath;
#ifdef _WIN32
    m_dllHandle = LoadLibraryEx(dllFullPath.c_str(), nullptr,
                                LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
    if (m_dllHandle == nullptr) {
      std::string additionalInfo;
      additionalInfo = GetLastWinErrorString();
      LOG(INFO) << "error info on loading dll on windows -> " << additionalInfo << "\n";
    }
#else
    m_dllHandle = dlopen(dllFullPath.c_str(), RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if (m_dllHandle == nullptr) {
      std::string additionalInfo;
      additionalInfo = dlerror();
      LOG(INFO) << "error info on loading dll on windows -> " << additionalInfo << "\n";
    }
#endif
    return m_dllHandle != nullptr;
  }

  // free loaded library if there is
  bool freeVehicleLibrary() {
    if (m_dllHandle != nullptr) {
#ifdef _WIN32
      FreeLibrary(m_dllHandle);
#else
      dlclose(m_dllHandle);
#endif
    }
    LOG(INFO) << "vehicle model library unloaded.\n";
    m_dllHandle = nullptr;
    return true;
  }

  // throw an exception if can not find function by name
  void* getFunctionPtr(const std::string& funcName) {
    void* getModelFunc = nullptr;
#ifdef _WIN32
    getModelFunc = GetProcAddress(m_dllHandle, funcName.c_str());
#else
    getModelFunc = dlsym(m_dllHandle, "getVehicleModel");
#endif
    // check if get function is nullptr
    if (getModelFunc == nullptr) {
      std::string additionalInfo;
#ifdef _WIN32
      additionalInfo = GetLastWinErrorString();
#else
      additionalInfo = dlerror();
#endif  // _WIN32

      std::string errorInfo = "fail to get " + funcName + " from " + m_dllFullPath + ", " + additionalInfo;
      LOG(ERROR) << errorInfo;
      throw std::runtime_error(errorInfo.c_str());

      return getModelFunc;
    }

    return getModelFunc;
  }
#ifdef _WIN32
  std::string GetLastWinErrorString() {
    // Get the error message, if any.
    DWORD err = GetLastError();
    if (err == 0) return std::string();  // No error message has been recorded
    LPSTR buffer = nullptr;
    size_t size =
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                       NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
    std::string message(buffer, size);
    // Free the buffer.
    LocalFree(buffer);
    return message;
  }
#endif  // _WIN32

#ifdef _WIN32
  HMODULE m_dllHandle;
#else
  void* m_dllHandle;
#endif
  std::string m_dllFullPath;
};

}  // namespace tx_car
