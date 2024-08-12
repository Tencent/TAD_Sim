#include "txsim_module_service.h"

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <thread>

#include "utils/dylib.h"
#include "CoreMinimal.h"

#define TXSIM_TADSIM_ENV_KEY "TADSIM"
#ifdef _WIN32
#define TXSIM_MODULE_IMPL_INSTALL_SUBDIR "/service/"
#define TXSIM_MODULE_IMPL_LIBRARY_NAME "txsim-module-impl.dll"
#elif defined __linux__
#define TXSIM_MODULE_IMPL_INSTALL_SUBDIR "/service/simdeps/"
#define TXSIM_MODULE_IMPL_LIBRARY_NAME "libtxsim-module-impl.so"
#endif  // _WIN32
#define TXSIM_MODULE_IMPL_FUNC_NAME_ERR_MSG    "txsim_impl_error_message"
#define TXSIM_MODULE_IMPL_FUNC_NAME_ERR_DEL    "txsim_impl_error_destruct"
#define TXSIM_MODULE_IMPL_FUNC_NAME_NEW_MODULE "txsim_new_module_impl"
#define TXSIM_MODULE_IMPL_FUNC_NAME_DEL_MODULE "txsim_delete_module_impl"
#define TXSIM_MODULE_IMPL_FUNC_NAME_SERVE      "txsim_serve"
#define TXSIM_MODULE_IMPL_FUNC_NAME_SHUTDOWN   "txsim_shutdown"


namespace tx_sim {
namespace impl {

typedef const char* (*FuncErrMsg)(void*);
typedef void (*FuncErrDel)(void*);
typedef void* (*FuncNewPtr)(void*, void*, void*, void*, uint32_t, void** err);
typedef void (*FuncDelPtr)(void*);
typedef void (*FuncServePtr)(void*, void*, const char*, const char*, void** err);
typedef void (*FuncShutPtr)(void*);

}  // namespace impl
}  // namespace tx_sim


extern "C" {

struct txsim_service_error {
  std::string msg;
};

const char* txsim_service_error_message(txsim_service_error_t err) {
  return err->msg.c_str();
}

void txsim_service_error_destruct(txsim_service_error_t err) {
  delete err;
}


struct txsim_service {
  void* err = nullptr;
  void* lib_;
  void* impl_;
  void* t_;
  tx_sim::impl::FuncErrMsg fem_;
  tx_sim::impl::FuncErrDel fed_;
  tx_sim::impl::FuncNewPtr fn_;
  tx_sim::impl::FuncDelPtr fd_;
  tx_sim::impl::FuncServePtr fsv_;
  tx_sim::impl::FuncShutPtr fst_;
};


txsim_service_t txsim_service_construct(void* init_cb, void* reset_cb, void* step_cb, void* stop_cb,
                                        uint32_t api_version, txsim_service_error_t* out_error) {
  void* lib = nullptr;
  std::string impl_lib_path = TXSIM_MODULE_IMPL_LIBRARY_NAME;
  char* tadsim_install_dir = std::getenv(TXSIM_TADSIM_ENV_KEY);
  if (tadsim_install_dir != nullptr)
    impl_lib_path = std::string(tadsim_install_dir) + TXSIM_MODULE_IMPL_INSTALL_SUBDIR + TXSIM_MODULE_IMPL_LIBRARY_NAME;
  try {
  lib = tx_sim::utils::LoadSharedLibrary(impl_lib_path,
                                           false);  // cannot asking the client user to -fPIC compiling their codes ...
  } catch (const std::exception& e) {
    std::stringstream ss;
    ss << "** Loading " << TXSIM_MODULE_IMPL_LIBRARY_NAME << " failed. Did you install the TAD Sim correctly?\n"
       << "** details: " << e.what();
    *out_error = new txsim_service_error{ss.str()};
    return nullptr;
  }
  txsim_service_t s = new struct txsim_service;
  s->lib_ = lib;
  s->fem_ = reinterpret_cast<tx_sim::impl::FuncErrMsg>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_ERR_MSG, s->lib_));
  assert(s->fem_ != nullptr);
  s->fed_ = reinterpret_cast<tx_sim::impl::FuncErrDel>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_ERR_DEL, s->lib_));
  assert(s->fed_ != nullptr);
  s->fn_ = reinterpret_cast<tx_sim::impl::FuncNewPtr>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_NEW_MODULE, s->lib_));
  assert(s->fn_ != nullptr);
  s->fd_ = reinterpret_cast<tx_sim::impl::FuncDelPtr>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_DEL_MODULE, s->lib_));
  assert(s->fd_ != nullptr);
  s->fsv_ = reinterpret_cast<tx_sim::impl::FuncServePtr>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_SERVE, s->lib_));
  assert(s->fsv_ != nullptr);
  s->fst_ = reinterpret_cast<tx_sim::impl::FuncShutPtr>(
      tx_sim::utils::GetLibraryFunc(TXSIM_MODULE_IMPL_FUNC_NAME_SHUTDOWN, s->lib_));
  assert(s->fst_ != nullptr);

  void* err = nullptr;
  s->impl_ = s->fn_(init_cb, reset_cb, step_cb, stop_cb, api_version, &err);
  if (err) {
    *out_error = new txsim_service_error{s->fem_(err)};
    s->fed_(err);
    delete s;
    return nullptr;
  }
  assert(s->impl_ != nullptr);
  return s;
}


void txsim_service_serve(txsim_service_t s, const char* name, void* module, const char* address) {
  assert(s != nullptr);
  std::string name_str(name), addr_str(address);
  s->t_ = new std::thread(
      [s, name_str, module, addr_str]() { s->fsv_(s->impl_, module, name_str.c_str(), addr_str.c_str(), &s->err); });
}


void txsim_service_wait(txsim_service_t s, txsim_service_error_t* out_error) {
  assert(s != nullptr);
  std::thread* tp = reinterpret_cast<std::thread*>(s->t_);
  if (tp && tp->joinable()) { tp->join(); }
  if (s->err) {
    *out_error = new txsim_service_error{s->fem_(s->err)};
    s->fed_(s->err);
    s->err = nullptr;
  }
}


void txsim_service_shutdown(txsim_service_t s) {
  assert(s != nullptr);
  if (s->impl_) { s->fst_(s->impl_); }
}


void txsim_service_destruct(txsim_service_t s) {
  assert(s != nullptr);
  if (s->impl_) { s->fd_(s->impl_); }
  if (s->t_) { delete reinterpret_cast<std::thread*>(s->t_); }
  if (s->lib_) { tx_sim::utils::UnLoadLibrary(s->lib_); }
  delete s;
}

}  // extern "C"
