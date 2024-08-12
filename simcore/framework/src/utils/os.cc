#include "os.h"

#ifdef _WIN32
#  include <Windows.h>
#  include <processthreadsapi.h>
#  include <stringapiset.h>
#endif  // _WIN32
#ifdef __linux__
#  include <pthread.h>
#  include <pwd.h>
#  include <string.h>
#  include <sys/types.h>
#  include <unistd.h>
#  include <cassert>
#  include <csignal>
#endif  // __linux__
#include <cstdlib>
#include <regex>

#include "constant.h"


namespace tx_sim {
namespace utils {

const std::string GetHomeDir() {
  const char* home_dir = nullptr;
#ifdef __linux__
  // struct passwd *pwd = getpwuid(getuid());  // getpwuid core dumps when called in dlmopen-ed library.
  // if (pwd) {  // getpwuid is the most reliable way to determine where the user’s home directory is.
  //   home_dir = pwd->pw_dir;
  // } else {  // otherwise we can only using env variable. it's much less reliable.
  if ((home_dir = getenv("XDG_RUNTIME_DIR")) == nullptr) {  // freedesktop.org XDG Base Directory Specification
    home_dir = getenv("HOME");
  }
  // }
#endif  // __linux__
  return home_dir;
}


const std::string GetFileDirectory(const std::string& file_abs_path) {
  size_t pos = file_abs_path.find_last_of("/\\");
  return file_abs_path.substr(0, pos);
}


const std::string GetModuleRegistryBindAddress(uint16_t port_offset) {
  int port = std::stoi(tx_sim::impl::kDefaultModuleRegistryPort) + port_offset;
  return "tcp://*:" + std::to_string(port);
}


const std::string GetLocalServiceBindAddress() {
#ifdef __linux__
  return "ipc://" + GetHomeDir() + "/tadsim-service.sock";
#endif  // __linux__
#ifdef _WIN32
  return "tcp://127.0.0.1:8401";
#endif  // _WIN32
}


#ifdef _WIN32
const std::string GetLastWinErrorString() {
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


void ConfigureSignalHandling(void (*handler)(int)) {
#ifdef __linux__
  struct sigaction action;
  action.sa_handler = handler;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
#endif  // __linux__
}


void BlockSignals() {
#ifdef __linux__
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGINT);
  sigaddset(&set, SIGTERM);
  int r = pthread_sigmask(SIG_BLOCK, &set, NULL);
  assert(r == 0);
#endif  // __linux__
}


void UnBlockSignals() {
#ifdef __linux__
  sigset_t unblock_set;
  sigemptyset(&unblock_set);
  sigaddset(&unblock_set, SIGINT);
  sigaddset(&unblock_set, SIGTERM);
  int r = pthread_sigmask(SIG_UNBLOCK, &unblock_set, NULL);
  assert(r == 0);
#endif  // __linux__
}


const std::string GetEnvVar(const std::string& env_key) {
  char* env_value = std::getenv(env_key.c_str());
  return env_value ? env_value : "";
}


static void SetEnvVarsImpl(const std::vector<std::pair<std::string, std::string>>& envs, bool deleting) {
  // replace all existing environment variables.
#ifdef __linux__
  for (const auto& kv : envs) setenv(kv.first.c_str(), kv.second.c_str(), 1);
#elif defined _WIN32
  for (const auto& kv : envs) SetEnvironmentVariable(kv.first.c_str(), deleting ? NULL : kv.second.c_str());
#endif
}


void SetEnvVars(const std::vector<std::pair<std::string, std::string>>& envs) {
  SetEnvVarsImpl(envs, false);
}


void RemoveEnvVars(const std::vector<std::pair<std::string, std::string>>& envs) {
  SetEnvVarsImpl(envs, true);
}


void SetThreadName(const std::string& name) {
#ifdef __linux__
  pthread_setname_np(pthread_self(), name.c_str());
#endif
#ifdef _WIN32
  std::wstring wname;
  int sz = MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, nullptr, 0);
  if (sz > 0) {
    wname.resize(sz + 10);
    sz = MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, &wname[0], wname.size());
    if (sz > 0) SetThreadDescription(GetCurrentThread(), wname.c_str());
  }
#endif  // _WIN32
}

}  // namespace utils
}  // namespace tx_sim
