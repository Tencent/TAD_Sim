#include "proc_service.h"

#include "glog/logging.h"

#include "os.h"
#include "proc.h"
#include "utils/zmq_routine.h"


namespace tx_sim {
namespace utils {

const std::string ModuleProcessServiceImpl::kCalleeBindAddress = "inproc://txsim-module-proc-serv";


ModuleProcessServiceImpl::ModuleProcessServiceImpl(std::shared_ptr<zmq::context_t>& ctx) {
  caller_.reset(new zmq::socket_t(*ctx, zmq::socket_type::req));

  serv_.reset(new std::thread([ctx, this]() -> void {
    SetThreadName("txsim-module-proc-service");

    callee_.reset(new zmq::socket_t(*ctx, zmq::socket_type::rep));
    callee_->bind(kCalleeBindAddress);
    LOG(INFO) << "module process service activated ...";

    zmq::multipart_t msg;
    while (true) {
      msg.recv(*callee_);
      int16_t op = msg.poptyp<int16_t>();
      if (op == 0) break;
      assert(op == 1);
      std::string path, stdout_path;
      std::vector<std::string> args;
      std::vector<tx_sim::impl::StringPair> envs;
      path = msg.popstr();
      size_t s = msg.poptyp<size_t>();
      args.reserve(s);
      for (size_t i = 0; i < s; ++i) args.push_back(msg.popstr());
      s = msg.poptyp<size_t>();
      envs.reserve(s);
      for (size_t i = 0; i < s; ++i) {
        auto k = msg.popstr(), v = msg.popstr();
        envs.emplace_back(k, v);
      }
      stdout_path = msg.popstr();
      txsim_pid_t pid = txsim_invalid_pid_t;
      std::string err;
      try {
        pid = CreateModuleProcess(path, args, envs, stdout_path);
        if (pid == txsim_invalid_pid_t) err = "creating module process error";
      } catch (const std::exception& e) { err = e.what(); }
      msg.clear();
      msg.addtyp<txsim_pid_t>(pid);
      msg.addstr(err);
      msg.send(*callee_);
    }
    msg.clear();
    msg.addtyp<int16_t>(0);
    msg.send(*callee_);

    LOG(INFO) << "module process service exit.";
  }));

  caller_->connect(kCalleeBindAddress);
}


ModuleProcessServiceImpl::~ModuleProcessServiceImpl() {
  zmq::multipart_t msg;
  msg.addtyp<int16_t>(0);
  msg.send(*caller_);
  msg.recv(*caller_);
  serv_->join();
}


txsim_pid_t ModuleProcessServiceImpl::CreateModule(const std::string& path, const std::vector<std::string>& args,
                                                   const std::vector<tx_sim::impl::StringPair>& envs,
                                                   const std::string& stdout_path) {
  zmq::multipart_t msg;
  msg.addtyp<int16_t>(1);
  msg.addstr(path);
  msg.addtyp<size_t>(args.size());
  for (const auto& arg : args) msg.addstr(arg);
  msg.addtyp<size_t>(envs.size());
  for (const auto& env : envs) {
    msg.addstr(env.first);
    msg.addstr(env.second);
  }
  msg.addstr(stdout_path);
  msg.send(*caller_);
  msg.recv(*caller_);
  txsim_pid_t pid = msg.poptyp<txsim_pid_t>();
  std::string err = msg.popstr();
  if (pid == txsim_invalid_pid_t || !err.empty()) throw std::runtime_error(err);
  return pid;
}


void ModuleProcessServiceImpl::TerminateModule(txsim_pid_t pid) {
  tx_sim::utils::TerminateModuleProcess(pid);
}

}  // namespace utils
}  // namespace tx_sim
