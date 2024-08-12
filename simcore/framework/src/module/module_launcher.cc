// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <iostream>
#include <thread>

#include "module/module_impl.h"
#include "utils/dylib.h"
#include "utils/os.h"


typedef void* (*LibFuncGetModule)();
#define GET_MODULE_FUNC_NAME   "txsim_new_module"
#define DEL_MODULE_FUNC_NAME   "txsim_delete_module"
#define INIT_MODULE_FUNC_NAME  "txsim_init"
#define RESET_MODULE_FUNC_NAME "txsim_reset"
#define STEP_MODULE_FUNC_NAME  "txsim_step"
#define STOP_MODULE_FUNC_NAME  "txsim_stop"
#define API_VERSION_FUNC_NAME  "txsim_messenger_api_version"
#define MAIN_EXECUTABLE_NAME   "txsim-module-launcher"


using namespace tx_sim::impl;
using namespace tx_sim::utils;


static ModuleServiceImpl* g_impl_ptr = nullptr;


static void HandleSignal(int sig_num) {
#ifdef __linux__
  std::cout << " *** signal " << strsignal(sig_num) << " received. ***" << std::endl;
  if (g_impl_ptr) g_impl_ptr->Shutdown();
#endif  // __linux__
}


int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr << MAIN_EXECUTABLE_NAME << ": wrong arguments: ";
    for (int i = 0; i < argc; ++i) std::cerr << "[" << argv[i] << "] ";
    std::cerr << std::endl;
    return 1;
  }

  // TODO(nemo): for now removing dynamic glog dependency. may we implement it using libunwind directly?
  // google::InstallFailureSignalHandler();

  ConfigureSignalHandling(HandleSignal);

  // loading user's module dynamic library.
  void *lib = nullptr, *new_func = nullptr, *del_func = nullptr, *init_func = nullptr, *reset_func = nullptr,
       *step_func = nullptr, *stop_func = nullptr;
  try {
    lib = LoadSharedLibrary(argv[2]);
    new_func = GetLibraryFunc(GET_MODULE_FUNC_NAME, lib);
    del_func = GetLibraryFunc(DEL_MODULE_FUNC_NAME, lib);
    init_func = GetLibraryFunc(INIT_MODULE_FUNC_NAME, lib);
    reset_func = GetLibraryFunc(RESET_MODULE_FUNC_NAME, lib);
    step_func = GetLibraryFunc(STEP_MODULE_FUNC_NAME, lib);
    stop_func = GetLibraryFunc(STOP_MODULE_FUNC_NAME, lib);
  } catch (const std::exception& e) {
    std::cerr << MAIN_EXECUTABLE_NAME << " loading user library error: " << e.what() << std::endl;
    UnLoadLibrary(lib);
    return 2;
  }
  uint32_t messenger_api_version = 0;
  try {
    void* api_ver_func = GetLibraryFunc(API_VERSION_FUNC_NAME, lib);
    messenger_api_version = reinterpret_cast<uint32_t (*)()>(api_ver_func)();
  } catch (const std::exception& e) {
    // ignored. compitable with the very first version(which has no related function defined).
  }
  std::cout << MAIN_EXECUTABLE_NAME << ": user library API version: " << messenger_api_version << std::endl;

  // try to get user's module object.
  void* module = nullptr;
  try {
    module = reinterpret_cast<void* (*)()>(new_func)();
    if (module == nullptr) {
      std::cerr << MAIN_EXECUTABLE_NAME << ": could not get SimModule with " << GET_MODULE_FUNC_NAME
                << " function: nullptr returned." << std::endl;
      UnLoadLibrary(lib);
      return 2;
    }
  } catch (const std::exception& e) {
    std::cerr << MAIN_EXECUTABLE_NAME << "calling " << GET_MODULE_FUNC_NAME << " error: " << e.what() << std::endl;
    UnLoadLibrary(lib);
    return 2;
  } catch (...) {
    std::cerr << MAIN_EXECUTABLE_NAME << "calling " << GET_MODULE_FUNC_NAME << " unknown type error." << std::endl;
    UnLoadLibrary(lib);
    return 2;
  }

  try {
    g_impl_ptr = new ModuleServiceImpl(init_func, reset_func, step_func, stop_func, messenger_api_version,
                                       GetFileDirectory(argv[2]));
    std::string addr = (argc >= 4 ? argv[3] : "");
    g_impl_ptr->Serve(module, argv[1], addr);
    // after signal handler called g_impl_ptr->Shutdown()
    std::cout << "Module Service finished cleanly." << std::endl;
  } catch (const std::exception& e) { std::cerr << "Module Service error: " << e.what() << std::endl; }
  if (g_impl_ptr) delete g_impl_ptr;

  reinterpret_cast<void (*)(void*)>(del_func)(module);
  UnLoadLibrary(lib);

  return 0;
}
