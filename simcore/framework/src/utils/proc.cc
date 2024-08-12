#include "proc.h"

#ifdef __linux__
#  include <paths.h>
#  include <sys/prctl.h>
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#  include <unistd.h>
#  include <cstdio>
#  include <cstring>
#endif  // __linux__
#ifdef _WIN32
#  include <Windows.h>
#endif  // _WIN32
#include <iostream>
#include <sstream>

#ifdef _WIN32
#  include "boost/algorithm/string/predicate.hpp"
#  include "glog/logging.h"
#endif  // _WIN32

#include "utils/os.h"


#ifdef _WIN32
static void *InitProcessJobHandle() {
  void *h_job = CreateJobObject(NULL, "txSim");
  if (!h_job) {
    LOG(ERROR) << "creating win job object failed: " << tx_sim::utils::GetLastWinErrorString();
    throw std::runtime_error("failed creating job object on Windows.");
  }
  JOBOBJECT_EXTENDED_LIMIT_INFORMATION jeli = {};
  jeli.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
  if (!SetInformationJobObject(h_job, JobObjectExtendedLimitInformation, &jeli, sizeof(jeli))) {
    LOG(ERROR) << "setting win job object limit failed: " << tx_sim::utils::GetLastWinErrorString();
    throw std::runtime_error("failed setting job object limit on Windows.");
  }
  return h_job;
}

static void *g_win_handle_job = InitProcessJobHandle();  // for process job control on Windows.
#endif                                                   // _WIN32


#ifdef __linux__
// refer to https://www.oreilly.com/library/view/secure-programming-cookbook/0596003943/ch01s01.html

extern char **environ;

static const char *restricted_environ[] = {"IFS= \t\n", "PATH=" _PATH_STDPATH, 0};
static const char *preserve_environ[] = {
    "TZ",               // time zone
    "HOME",             // dlopen would crash(tested on ubuntu 16.04) without "HOME" env!!! great :)
    "XDG_RUNTIME_DIR",  // the base path of unix domain sockets.
    0};

// return a null pointer terminated array of null terminated strings.
static char **GetSanitizedEnvironment(char **added) {
  char **new_environ, *ptr, *value;
  const char *var;
  size_t arr_size = 1, arr_ptr = 0, len, new_size = 0, i;

  for (i = 0; (var = restricted_environ[i]) != 0; ++i) {
    new_size += strlen(var) + 1;
    arr_size++;
  }
  for (i = 0; (var = preserve_environ[i]) != 0; ++i) {
    if (!(value = getenv(var))) continue;
    new_size += strlen(var) + strlen(value) + 2; /* include the '=' */
    arr_size++;
  }
  if (added) {
    for (i = 0; (var = added[i]) != 0; i++) {
      new_size += strlen(var) + 1;
      arr_size++;
    }
  }

  new_size += (arr_size * sizeof(char *));
  if (!(new_environ = (char **)malloc(new_size))) abort();
  new_environ[arr_size - 1] = 0;

  ptr = (char *)new_environ + (arr_size * sizeof(char *));
  for (i = 0; (var = restricted_environ[i]) != 0; i++) {
    new_environ[arr_ptr++] = ptr;
    len = strlen(var);
    memcpy(ptr, var, len + 1);
    ptr += len + 1;
  }
  for (i = 0; (var = preserve_environ[i]) != 0; i++) {
    if (!(value = getenv(var))) continue;
    new_environ[arr_ptr++] = ptr;
    len = strlen(var);
    memcpy(ptr, var, len);
    *(ptr + len) = '=';
    memcpy(ptr + len + 1, value, strlen(value) + 1);
    ptr += len + strlen(value) + 2; /* include the '=' */
  }
  if (added) {
    for (i = 0; (var = added[i]) != 0; i++) {
      new_environ[arr_ptr++] = ptr;
      len = strlen(var);
      memcpy(ptr, var, len + 1);
      ptr += len + 1;
    }
  }

  return new_environ;
}

// refer to https://www.oreilly.com/library/view/secure-programming-cookbook/0596003943/ch01s05.html
// On Windows, there is no way to determine what file handles are open, but the same issue with open descriptors does
// not exist on Windows as it does on Unix.

#  ifndef OPEN_MAX
#    define OPEN_MAX 256
#  endif

static int RedirectStdInOut(int fd, const char *redirect_path) {
  FILE *f = 0;
  if (!fd) {
    f = freopen(redirect_path, "rb", stdin);
  } else if (fd == 1) {
    fclose(fopen(redirect_path, "wb"));  // touch a new file.
    f = freopen(redirect_path, "ab", stdout);
  } else if (fd == 2) {
    f = freopen(redirect_path, "ab", stderr);
  }
  return (f && fileno(f) == fd);
}

static void SanitizeFiles(const char *out_path) {
  int fd, fds;
  struct stat st;

  /* Make sure all open descriptors other than the standard ones are closed */
  if ((fds = getdtablesize()) == -1) fds = OPEN_MAX;
  for (fd = 3; fd < fds; ++fd) close(fd);

  /* Verify that the standard input descriptor is open.  If it's not, attempt to open it using /dev/null.
   * redirect the standard output/error to the specified file.  If any are unsuccessful, exit.
   */
  if (fstat(0, &st) == -1 && (errno != EBADF || !RedirectStdInOut(0, _PATH_DEVNULL))) exit(1);
  if (!RedirectStdInOut(1, out_path)) exit(1);
  if (!RedirectStdInOut(2, out_path)) exit(1);
}

static const char **GetArgvArray(const std::string &path, const std::vector<std::string> &args) {
  const char **argv = new const char *[args.size() + 2];  // +2 for program name and sentinel nullptr.
  argv[0] = path.c_str();                                 // by convention, argv[0] is program name.
  for (size_t i = 0; i < args.size(); ++i) argv[i + 1] = args[i].c_str();
  argv[args.size() + 1] = nullptr;
  return argv;
}
#endif  // __linux__
#ifdef _WIN32
HANDLE RedirectStdOut(STARTUPINFO &si, const std::string &path) {
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;
  HANDLE out_file =
      CreateFile(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &saAttr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
  si.hStdOutput = out_file;
  si.hStdError = out_file;
  si.dwFlags |= STARTF_USESTDHANDLES;
  return out_file;
}
#endif  // _WIN32


namespace tx_sim {
namespace utils {

txsim_pid_t CreateModuleProcess(const std::string &path, const std::vector<std::string> &args,
                                const std::vector<tx_sim::impl::StringPair> &envs, const std::string &stdout_path) {
#ifdef __linux__
  pid_t child_pid = fork();
  if (child_pid > 0) {  // current process, return child pid directly.
    return child_pid;
  } else if (child_pid == 0) {  // child process
    // if parent process dies, sent a strong KILL signal to ensure all sub-process created by it also dies.
    prctl(PR_SET_PDEATHSIG, SIGKILL);
    // also reset all custom signal handling.
    UnBlockSignals();
    // all dynamically allocated memory below need not be freed since execve replaces current heap with a new one.
    SanitizeFiles(stdout_path.c_str());
    const char **argv = GetArgvArray(path, args);
    SetEnvVars(envs);
    // execve info prints ...
    std::cout << "path: " << path << std::endl;
    for (int i = 0; argv[i] != nullptr; ++i) std::cout << "arg" << i << ": \"" << argv[i] << "\"" << std::endl;
    // for (int i = 0; environ[i] != nullptr; ++i) std::cout << "env" << i << ": [" << environ[i] << "]" << std::endl;
    std::cout << std::endl;
    // executing specified program ...
    execve(path.c_str(), (char **)argv, environ);
    exit(4);  // execve only returned on error.
  } else {    // error
    return txsim_invalid_pid_t;
  }
#endif  // __linux__
#ifdef _WIN32
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);
  HANDLE out_hdl = RedirectStdOut(si, stdout_path);

  LPCSTR path_str = NULL;
  std::stringstream cmdline;
  if (boost::algorithm::ends_with(path, ".bat")) {  // launching batch script.
    cmdline << "cmd.exe /c \"" << path << "\"";
    for (const std::string &arg : args) cmdline << " " << arg;  // cmd /c "foo.bat" arg ...(only one " pair allowed!)
  } else {                                                      // launching executable.
    LPCSTR path_str = path.c_str();
    cmdline << "\"" << path << "\"";
    for (const std::string &arg : args) cmdline << " \"" << arg << "\"";
  }
  std::string cmdline_str = std::move(cmdline.str());
  DWORD offset = 0;
  WriteFile(out_hdl, cmdline_str.c_str(), cmdline_str.size(), &offset, NULL);
  WriteFile(out_hdl, "\n\r", 2, &offset, NULL);

  // TODO(nemo): sanitize lpEnvironment and lpCurrentDirectory?
  SetEnvVars(envs);
  bool succeed = CreateProcess(path_str, const_cast<char *>(cmdline.str().c_str()), NULL, NULL, TRUE,
                               CREATE_NO_WINDOW | CREATE_SUSPENDED, NULL, NULL, &si, &pi) &&
                 AssignProcessToJobObject(g_win_handle_job, pi.hProcess) && (ResumeThread(pi.hThread) != (DWORD)-1);
  RemoveEnvVars(envs);

  CloseHandle(pi.hThread);
  if (succeed) {
    CloseHandle(out_hdl);
    return pi.hProcess;
  }
  std::string err_str = GetLastWinErrorString();
  WriteFile(out_hdl, err_str.c_str(), err_str.size(), &offset, NULL);
  CloseHandle(out_hdl);
  CloseHandle(pi.hProcess);  // close child process handle if failed.
  return txsim_invalid_pid_t;
#endif  // _WIN32
}


void TerminateModuleProcess(txsim_pid_t pid) {
#ifdef __linux__
  pid_t ret = waitpid(pid, nullptr, WNOHANG);
  if (ret == -1 && errno == ECHILD) {
    throw std::runtime_error("no child process found");
  } else if (ret == pid) {
    return;
  }

  if (kill(pid, SIGTERM) != 0) {  // try killing gently.
    throw std::runtime_error("failed to terminate process: " + std::string(std::strerror(errno)));
  }

  for (int i = 0; i < 3; ++i) {  // waits SIGTERM for at most 3 senconds.
    sleep(1);
    ret = waitpid(pid, nullptr, WNOHANG);
    if (ret == pid) return;
    if (ret == -1 && errno == ECHILD) return;  // the child may have been reaped.
    // ret == 0 or errno == EINTR: continue to poll the child's status.
  }

  if (kill(pid, SIGKILL) != 0) {  // killing forceably.
    throw std::runtime_error("failed to kill process: " + std::string(std::strerror(errno)));
  }
  waitpid(pid, nullptr, 0);
#endif  // __linux__
#ifdef _WIN32
  std::string pid_str = std::to_string(GetProcessId(pid));

  // ======================================================================
  // using Andrew Tucker's code, see http://www.drdobbs.com/a-safer-alternative-to-terminateprocess/184416547
  DWORD dw_tid, dw_code;
  HANDLE rt_hdl = NULL;
  HINSTANCE kernel_hdl = GetModuleHandle("Kernel32");
  bool succeed = false;

  if (GetExitCodeProcess(pid, &dw_code) && (dw_code == STILL_ACTIVE)) {
    // process exists, kill it
    FARPROC fn_exit_proc;
    fn_exit_proc = GetProcAddress(kernel_hdl, "ExitProcess");
    rt_hdl = CreateRemoteThread(pid, NULL, 0, (LPTHREAD_START_ROUTINE)fn_exit_proc, (PVOID)15, 0, &dw_tid);
    if (rt_hdl) {
      succeed = (WaitForSingleObject(pid, 30000) == WAIT_OBJECT_0);
      CloseHandle(rt_hdl);
    }
  }
  CloseHandle(kernel_hdl);
  // ======================================================================

  std::string err_str;
  if (!succeed) {  // Andrew Tucker's code not work, kill it forceably(and ugly) ...
    if (TerminateProcess(pid, 0) == 0) {
      err_str = tx_sim::utils::GetLastWinErrorString();
    } else {
      if (WaitForSingleObject(pid, 5000) == WAIT_OBJECT_0) {
        succeed = true;
      } else {
        LPDWORD ec;
        err_str = (GetExitCodeProcess(pid, ec) == 0) ? tx_sim::utils::GetLastWinErrorString() : std::to_string(*ec);
      }
    }
  }

  CloseHandle(pid);
  if (!succeed) throw std::runtime_error("failed to terminate process " + pid_str + ", " + err_str);
#endif  // _WIN32
}


void PollChildrenExitStatus(ChildProcessStatus &status) {
  status.pid = txsim_invalid_pid_t;
#ifdef __linux__
  int stat_val = 0;
  pid_t ret = waitpid(-1, &stat_val, WNOHANG);
  if (ret <= 0) return;
  status.pid = ret;
  status.status_desc = GetProcessExitStatusDescription(stat_val);
#endif  // __linux__
#ifdef _WIN32
  static HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  if (status.queries.empty()) return;
  DWORD count = 0;
  for (size_t i = 0; i < status.queries.size() && i < MAXIMUM_WAIT_OBJECTS; ++i) handles[count++] = status.queries[i];
  DWORD ret = WaitForMultipleObjects(count, handles, FALSE, 0);
  if (ret >= WAIT_OBJECT_0 && ret < (WAIT_OBJECT_0 + count)) {
    HANDLE hdl = handles[ret];
    DWORD exit_code;
    if (GetExitCodeProcess(hdl, &exit_code)) {
      // TODO(nemo): how to determine whether it is a exit value or exception value?
      // see Remarks on
      // https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getexitcodeprocess
      status.pid = hdl;
      status.status_desc = GetProcessExitStatusDescription(exit_code);
    }
  }
#endif  // _WIN32
}


std::string GetProcessExitStatusDescription(int stat) {
  std::ostringstream ss;
#ifdef __linux__
  if (WIFEXITED(stat) != 0) ss << "exit code: " << std::to_string(WEXITSTATUS(stat));
  else if (WIFSIGNALED(stat) != 0)
    ss << "un-caught signal: " << std::string(strsignal(WTERMSIG(stat)));
  else if (WIFSTOPPED(stat) != 0)
    ss << "stop signal: " << std::string(strsignal(WSTOPSIG(stat)));
  else if (WIFCONTINUED(stat) != 0)
    ss << "process continued.";
#endif  // __linux__
#ifdef _WIN32
  ss << "exit code: " << std::to_string(stat);
#endif  // _WIN32
  return ss.str();
}

}  // namespace utils
}  // namespace tx_sim
