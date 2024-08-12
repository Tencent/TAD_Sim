// Copyright 2024 Tencent Inc. All rights reserved.
//
// \brief "单机版调度引擎服务"
//
#include <memory>

#include "boost/program_options.hpp"
#include "glog/logging.h"

#include "server_impl/http_session.h"
#include "server_impl/legacy_zmq_service.h"
#include "server_impl/listener.h"
#include "server_impl/service_context.h"
#include "service/local_status_writer.h"


namespace prog_ops = boost::program_options;
namespace basio = boost::asio;         // from <boost/asio.hpp>
using bst_tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
using namespace tx_sim::service;


/**
 * @brief "单机版调度引擎服务端",
 * there are two service methods, one is the traditional zmq communication and the other is the new http communication.
 */
class SimLocalService final {
 public:
  /**
   * @brief Construct a new Sim Local Service object including LocalServiceContext, SimLocalStatusWriter and
   * LegacyZmqService.
   * @param app_root_path the root directory of the local service in where contains configurations and data.
   * @param instance_id the instance id of the local service.
   */
  SimLocalService(const std::string& app_root_path, uint16_t instance_id) {
    context_ = std::make_shared<LocalServiceContext>(app_root_path, instance_id);
    writer_.reset(new SimLocalStatusWriter);
    legacy_service_.reset(new LegacyZmqService(context_->zmq_ctx, context_));
  }

  /**
   * @brief start the simulation service with legacy zmq service and http service.
   * @param endpoint if specified, the local service will listening on the specified http address. format:
   * ipv4_address:port.
   */
  void Serve(const std::string& endpoint) {
    static const int kIOThreadNum = 3;

    // start legacy zmq service.
    legacy_service_->Serve();  // in other thread.

    // start http service.
    basio::io_context ioc(kIOThreadNum);
    std::make_shared<tx_sim::service::Listener>(ioc, MakeTcpEndpointFromStr(endpoint))
        ->Start([this](bst_tcp::socket&& socket) {
          std::make_shared<tx_sim::service::HttpSession>(++session_id_, std::move(socket), context_, writer_)->Start();
        });

    basio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](const boost::beast::error_code& ec, int s) {
      LOG(INFO) << " *** sim http service received signals: " << s << " ***";
      ioc.stop();
    });

    std::vector<std::thread> io_ctx_threads;
    io_ctx_threads.reserve(kIOThreadNum);
    for (int i = 0; i < kIOThreadNum; ++i) io_ctx_threads.emplace_back([&ioc] { ioc.run(); });
    LOG(INFO) << "sim http service serving on " << endpoint << " ...";
    for (auto& t : io_ctx_threads) t.join();
    LOG(INFO) << "sim http service exit.";
  }

 private:
  /**
   * @brief parsing the addr and port from the input string.
   * @param[in] ep_str the input string.
   * @return the parsed address and port in the boost format.
   */
  bst_tcp::endpoint MakeTcpEndpointFromStr(const std::string& ep_str) {
    std::string ip_str, port_str;
    // 分割字符串，获取ip和port。
    auto pos = ep_str.find_first_of(':');
    if (pos == std::string::npos)
      throw std::invalid_argument(std::string("invalid service endpoint provided: ") + ep_str);
    // 获取IP地址字符串
    ip_str = ep_str.substr(0, pos);
    // 获取端口号字符串
    port_str = ep_str.substr(pos + 1);
    basio::ip::address addr;
    try {
      addr = basio::ip::make_address(ip_str);
    } catch (const std::exception& e) {
      throw std::invalid_argument(std::string("invalid service endpoint ip provided: ") + ip_str);
    }
    uint16_t port = 0;
    try {
      port = static_cast<uint16_t>(std::stoi(port_str));
    } catch (const std::exception& e) {
      throw std::invalid_argument(std::string("invalid service endpoint port provided: ") + port_str);
    }
    return {addr, port};
  }

 private:
  // http session id
  size_t session_id_ = 0;
  // a context be used to store local service's core and components.
  std::shared_ptr<LocalServiceContext> context_;
  // a writer be used to write status in tx_sim::service::HttpSession.
  std::shared_ptr<SimLocalStatusWriter> writer_;
  // legacy zmq service
  std::unique_ptr<LegacyZmqService> legacy_service_;
};


/**
 * @brief parse command line arguments.
 * @param ac number of arguments.
 * @param av array of argument values.
 * @param root root directory of the local service.
 * @param endpoint the specified ip address or host name + port which the local service should listen on.
 * @param id instance ID of the local service.
 * @return true if succeed, false otherwise.
 */
static bool InstallProgramOptions(int ac, char** av, std::string& root, std::string& endpoint, uint16_t& id) {
  prog_ops::options_description op_desc("Options");
  // clang-format off
  op_desc.add_options()
    ("help,h", "display help info")
    ("root,r", prog_ops::value<std::string>(&root)->default_value("."),
        "the root directory of the local service in where contains configurations and data.")
    ("endpoint,p", prog_ops::value<std::string>(&endpoint)->default_value("0.0.0.0:8302"),
        "if specified, the local service will listening on the specified http address. format: ipv4_address:port")
    ("logdir,l", prog_ops::value<std::string>(&FLAGS_log_dir)->default_value(""),
        "the directory in where to write the log files of the local service. default will write to the stderr")
    ("id,i", prog_ops::value<uint16_t>(&id)->default_value(0),
        "the instance ID of the local service for localhost parallel simulation. must be a integer in range [0, 100].");
  // clang-format on
  prog_ops::parse_command_line(ac, av, op_desc);
  prog_ops::variables_map vm;
  prog_ops::store(prog_ops::parse_command_line(ac, av, op_desc), vm);
  prog_ops::notify(vm);
  if (vm.count("help")) {
    std::cout << op_desc << std::endl;
    return false;
  }
  if (id > 100) {
    std::cerr << "the specified instance ID(--id,-i) must be less than or equal to 100." << std::endl;
    return false;
  }
  return true;
}


/**
 * @brief main function of the cloud service.
 * @param argc number of arguments.
 * @param argv array of argument values.
 * @return 0 if succeed, 1 or 101 otherwise.
 */
int main(int argc, char* argv[]) {
  std::string app_root, endpoint;
  uint16_t instance_id;
  // parse command line arguments.
  if (!InstallProgramOptions(argc, argv, app_root, endpoint, instance_id)) return 0;

  FLAGS_logbuflevel = -1;  // no buffer
  // FLAGS_v = 4;
  FLAGS_max_log_size = 100;  // maximum 100 mb log file size
  // 如果没用输入日志目录地址,这里初始化日志目录
  if (!FLAGS_log_dir.empty()) google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  std::unique_ptr<SimLocalService> service;
  try {
    // sim service construct
    // 创建单机版服务实例
    service.reset(new SimLocalService(app_root, instance_id));
  } catch (const std::exception& e) {
    LOG(ERROR) << "local service initiation failed: " << e.what();
    std::string sError = "Module registry service initiation failed";
    if (strncmp(e.what(), sError.c_str(), sError.length()) == 0) return 100;
    return 1;
  }

  try {
    // sim service start
    service->Serve(endpoint);
  } catch (const std::exception& e) {
    LOG(ERROR) << "sim local http service error: " << e.what();
    std::string sError = "=====> Software Expired <=====";
    if (std::string(e.what()) == sError) {
      std::cout << e.what() << std::endl;
      return 101;
    }
    return 1;
  }
  return 0;
}
