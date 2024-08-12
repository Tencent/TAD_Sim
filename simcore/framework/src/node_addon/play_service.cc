// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: fangccheng@tencent.com
//
// 文件注释
// 单机版编译.node文件与前端通信

#define NAPI_CPP_EXCEPTIONS
#include <condition_variable>
#include <ctime>
#include <memory>
#include <set>
#include <sstream>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "boost/filesystem.hpp"
#include "boost/range/iterator_range.hpp"
#include "google/protobuf/util/json_util.h"
#include "napi.h"

#include <chrono>
#include <codecvt>
#include <fstream>
#include <string>
#include "control.pb.h"
#include "control_v2.pb.h"
#include "coordinator/local_client.h"
#include "debugmessage.pb.h"
#include "grading.pb.h"
#include "json/json.h"
#include "location.pb.h"
#include "traffic.pb.h"
#include "trajectory.pb.h"
#include "trajectoryFollowing.pb.h"
#include "txsim_messenger.h"
#include "utils/json_helper.h"
#include "utils/os.h"
#include "vehState.pb.h"


#define TXSIM_GRADING_KPI_REPORT_DIR "data/service_data/sim_data/pblog"


using namespace tx_sim::cli;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
namespace fs = boost::filesystem;

// map ret code to nodejs error code
inline int RetCodeMappings(CmdErrorCode ec) {
  switch (ec) {
    // cmd error code mappings
    // 请求接收返回0
    case kCmdAccepted: return 0;
    // 成功返回1
    case kCmdSucceed: return 1;
    // 场景停止返回2
    case kCmdScenarioStopped: return 2;
    // 请求被拒绝返回3
    case kCmdRejected: return 3;
    // 请求失败返回4
    case kCmdFailed: return 4;
    // 系统错误返回5
    case kCmdSystemError: return 5;
    // 无效的topic返回6
    case kCmdInvalidTopicPubSub: return 6;
    // 场景解析错误返回7
    case kCmdScenarioParsingError: return 7;
    // 场景解析超时返回8
    case kCmdScenarioTimeout: return 8;
    // 共享内存创建失败返回9
    case kCmdShmemCreationError: return 9;
    // 服务端忙碌返回10
    case kCmdServerBusy: return 10;
    // 模组配置已经准备好返回11
    case kCmdSchemeAlreadyExist: return 11;
    case kCmdConfictLogPlayShadowModule: return 12;
    case kCmdCancelled: return 13;
    case kCmdLogPlayModuleNotFound: 14;
    default: return -1000 - static_cast<int>(ec);
  }
};

// 评测报告目录最大大小为5GB
const size_t kMaxGradingKpiReportDirectorySize = (size_t)5 * 1024 * 1024 * 1024;  // 5Gb

// 连接调度服务IP和端口
static std::string g_endpoint{tx_sim::utils::GetLocalServiceBindAddress()};
// tadsim数据配置目录和安装目录
static std::string g_app_data_path, g_app_install_path;
// 评测报告目录
static std::string g_grading_kpi_dir;
// 评测标签目录
static std::string g_grading_label_path;
static std::vector<std::string> g_vecImageExt = {".png", ".jpeg", ".jpg", ".bmp"};

std::thread run_thread;

enum ClientErrCode : int16_t {
  kClientConnErr = -1,
  kClientInvalidArgs = -2,
  kClientCmdReject = 3,
};

// 设置调度服务通信IP和端口
void SetClientEndpoint(const Napi::CallbackInfo& info) {
  if (info.Length() < 1 || !info[0].IsString())
    Napi::TypeError::New(info.Env(), "expected string argument").ThrowAsJavaScriptException();
  g_endpoint = info[0].As<Napi::String>().ToString();
}

// 设置数据目录
void SetAppPath(const Napi::CallbackInfo& info) {
  if (info.Length() < 2 || !info[0].IsString() || !info[1].IsString())
    Napi::TypeError::New(info.Env(), "expected 2 path string argument").ThrowAsJavaScriptException();
  g_app_data_path = info[0].As<Napi::String>().ToString();
  g_app_install_path = info[1].As<Napi::String>().ToString();
  g_grading_kpi_dir = (fs::path(g_app_data_path) /= TXSIM_GRADING_KPI_REPORT_DIR).lexically_normal().string();
  g_grading_label_path = (fs::path(g_app_data_path) /= "data/scenario/grading_labels.json").lexically_normal().string();
}

// 处理返回结果
static void HandleReturnCode(const Napi::Env& env, const Napi::Function& cb, int16_t ec, const std::string& err = "") {
  Napi::Object result = Napi::Object::New(env);
  result.Set("err", err);
  result.Set("retCode", ec < 0 ? ec : RetCodeMappings(static_cast<CmdErrorCode>(ec)));
  cb.Call(env.Global(), {result});
}

// 检测callback参数
static bool CheckCbArg(const Napi::CallbackInfo& info) {
  return info.Length() == 1 && info[0].IsFunction();
}

// 检测reset参数
static bool CheckResetArgs(const Napi::CallbackInfo& info) {
  if (info.Length() == 2 && info[0].IsFunction() && info[1].IsObject()) {
    Napi::Value v = info[1].As<Napi::Object>().Get("scenarioPath");
    if (v.IsString() && !std::string(v.ToString()).empty()) return true;
  }
  if (CheckCbArg(info)) HandleReturnCode(info.Env(), info[0].As<Napi::Function>(), kClientInvalidArgs);
  return false;
}

// 检测string参数
static bool CheckStringArgs(const Napi::CallbackInfo& info) {
  bool ret = info.Length() == 2 && info[0].IsFunction() && info[1].IsString();
  if (!ret && CheckCbArg(info)) HandleReturnCode(info.Env(), info[0].As<Napi::Function>(), kClientInvalidArgs);
  return ret;
}

// 检查obj结构参数
static bool CheckObjArgs(const Napi::CallbackInfo& info) {
  bool ret = info.Length() == 2 && info[0].IsFunction() && info[1].IsObject();
  if (!ret && CheckCbArg(info)) HandleReturnCode(info.Env(), info[0].As<Napi::Function>(), kClientInvalidArgs);
  return ret;
}

// 检查整形参数
static bool CheckNumArgs(const Napi::CallbackInfo& info) {
  bool ret = info.Length() == 2 && info[0].IsFunction() && info[1].IsNumber();
  if (!ret && CheckCbArg(info)) HandleReturnCode(info.Env(), info[0].As<Napi::Function>(), kClientInvalidArgs);
  return ret;
}

// 检查数组参数是否合规
static bool CheckArrayArgs(const Napi::CallbackInfo& info) {
  bool ret = info.Length() == 2 && info[0].IsFunction() && info[1].IsArray();
  if (!ret && CheckCbArg(info)) HandleReturnCode(info.Env(), info[0].As<Napi::Function>(), kClientInvalidArgs);
  return ret;
}

// add error code.
static void AddError(Napi::Env& env, Napi::Array& err_list, size_t& err_idx, const std::string& name,
                     const std::string& err, tx_sim::impl::ModuleResponseType ec) {
  Napi::Object e = Napi::Object::New(env);
  e.Set("name", name);
  e.Set("code", static_cast<int16_t>(ec));
  e.Set("err", err);
  err_list[err_idx++] = e;
}

// transform json string into protobuf message object.
static bool NeedDecodingTopicMessage(const std::string& topic, std::unique_ptr<google::protobuf::Message>& pb_msg) {
  std::string real_topic(topic);
  std::string::size_type pos = 0;
  if ((pos = topic.rfind(kLogTopicPostfix)) != std::string::npos ||
      (pos = topic.rfind(kShadowTopicPostfix)) != std::string::npos)
    real_topic = topic.substr(0, pos);
  // clang-format off
  if (real_topic == tx_sim::topic::kLocation || real_topic == "LOCATION_REPLAY") pb_msg.reset(new sim_msg::Location());
  else if (real_topic == tx_sim::topic::kTrajectory
      || real_topic == "TRAJECTORY_1"
      || real_topic == "TRAJECTORY_2"
      || real_topic == "TRAJECTORY_3"
      || real_topic == "TRAJECTORY_4"
      || real_topic == "TRAJECTORY_5"
      || real_topic == "TRAJECTORY_REPLAY") pb_msg.reset(new sim_msg::Trajectory());
  else if (real_topic == tx_sim::topic::kTraffic || real_topic == "TRAFFIC_REPLAY") pb_msg.reset(new sim_msg::Traffic());
  else if (real_topic == tx_sim::topic::kGrading) pb_msg.reset(new sim_msg::Grading());
  else if (real_topic == tx_sim::topic::kControl) pb_msg.reset(new sim_msg::Control());
  else if (real_topic == tx_sim::topic::kVehicleState) pb_msg.reset(new sim_msg::VehicleState());
  else if (real_topic == "TRAJECTORY_FOLLOW") pb_msg.reset(new sim_msg::TrajectoryFollow());
  else if (real_topic == "LOCATION_TRAILER") pb_msg.reset(new sim_msg::Location());
  else if (real_topic == tx_sim::topic::kControl_V2) pb_msg.reset(new sim_msg::Control_V2());
  else if (real_topic == "DEBUG_MESSAGE") pb_msg.reset(new sim_msg::DebugMessage());
  else return false;
  // clang-format on
  return true;
}

// parse init status message from json string.
static void ParseInitStatus(Napi::Env& env, const std::vector<ModuleInitStatus>& status, Napi::Array& init_list,
                            Napi::Array& updated_list) {
  size_t updated_idx = 0;
  for (size_t m_idx = 0; m_idx < status.size(); ++m_idx) {
    const ModuleInitStatus& s = status[m_idx];
    Napi::Object m = Napi::Object::New(env);
    m.Set("name", s.name);
    m.Set("connected", s.state >= tx_sim::impl::kModuleInitConnected && s.state <= tx_sim::impl::kModuleInitSucceed);
    m.Set("initiated", s.state == tx_sim::impl::kModuleInitSucceed);
    const TopicPubSubInfo& pubsubs = s.topic_info;
    Napi::Array st = Napi::Array::New(env, pubsubs.sub_topics.size() + pubsubs.sub_shmems.size());
    size_t t_idx = 0;
    for (size_t i = 0; i < pubsubs.sub_topics.size(); ++i) st[t_idx++] = pubsubs.sub_topics[i];
    for (size_t i = 0; i < pubsubs.sub_shmems.size(); ++i) st[t_idx++] = pubsubs.sub_shmems[i];
    m.Set("sub_topics", st);
    Napi::Array pt = Napi::Array::New(env, pubsubs.pub_topics.size());
    for (size_t i = 0; i < pubsubs.pub_topics.size(); ++i) pt[i] = pubsubs.pub_topics[i];
    m.Set("pub_topics", pt);
    if (!pubsubs.pub_shmems.empty()) {
      Napi::Array ps = Napi::Array::New(env, pubsubs.pub_shmems.size());
      for (size_t i = 0; i < pubsubs.pub_shmems.size(); ++i) {
        Napi::Object sm = Napi::Object::New(env);
        sm.Set("topic", pubsubs.pub_shmems[i].first);
        sm.Set("size", pubsubs.pub_shmems[i].second);
        ps[i] = sm;
      }
      m.Set("pub_shmems", ps);
    }
    m.Set("logFileName", s.log_file);
    init_list[m_idx] = m;
    if (s.binary_updated) updated_list[updated_idx++] = s.name;
  }
}

// parse module status message from json string.
static void ParseModuleStatus(Napi::Env& env, const std::vector<ModuleCmdStatus>& status, Napi::Array& module_list,
                              Napi::Array& err_list) {
  size_t st_idx = 0, err_idx = 0;
  for (size_t m_idx = 0; m_idx < status.size(); ++m_idx) {
    const ModuleCmdStatus& s = status[m_idx];
    if (s.ec != tx_sim::impl::kModuleOK && s.ec != tx_sim::impl::kModuleRequireStop) {
      AddError(env, err_list, err_idx, s.name, s.msg, s.ec);
    } else {
      Napi::Object m = Napi::Object::New(env);
      m.Set("name", s.name);
      m.Set("timeCost", s.elapsed_time);
      m.Set("msg", s.msg);
      m.Set("feedback", s.feedback);
      module_list[st_idx++] = m;
    }
  }
}

// parse module status message from json string.
static void ParseStepMessage(Napi::Env& env, const StepMessage& status, Napi::Object& msg_list) {
  Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
  Napi::Function parse = json.Get("parse").As<Napi::Function>();
  Napi::Array egoMsgs = Napi::Array::New(env);
  google::protobuf::util::JsonPrintOptions opts;
  opts.always_print_primitive_fields = true;
  // std::string filename{"D://playlog.log"};
  // std::fstream s{filename, s.app | s.in | s.out};

  // std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
  //         .count();
  std::unordered_map<std::string, std::vector<std::pair<std::string, zmq::message_t>>> messageEgoMap;
  for (const std::pair<EgoTopic, zmq::message_t>& kv : status.messages) {
    std::unique_ptr<google::protobuf::Message> pb_msg;
    std::string sEgoGroup = kv.first.sEgoGroup;
    if (sEgoGroup.empty()) sEgoGroup = "common";
    std::string sEgoTopic = kv.first.sTopic;
    if (NeedDecodingTopicMessage(sEgoTopic, pb_msg)) {
      // pb_msg->ParseFromArray(kv.second.data(), kv.second.size());
      // std::string json_str;
      // google::protobuf::util::MessageToJsonString(*pb_msg, &json_str, opts);
      // Napi::Object msg = parse.Call(json, {Napi::String::New(env, json_str)}).As<Napi::Object>();
      auto& vecMsgs = messageEgoMap[sEgoGroup];
      zmq::message_t t;
      t.copy(*const_cast<zmq::message_t*>(&kv.second));
      vecMsgs.emplace_back(std::make_pair(sEgoTopic, std::move(t)));
      // s << "egogroup:" << sEgoGroup << std::endl;
      // s << "egotopic:" << sEgoTopic << std::endl;
    }
  }

  size_t ego_idx = 0;
  for (auto const& keyEgo : messageEgoMap) {
    size_t msg_idx = 0;
    std::string sEgoGroup = keyEgo.first;
    Napi::Object egoObj = Napi::Object::New(env);
    Napi::Array topicMsgs = Napi::Array::New(env);
    egoObj.Set("name", sEgoGroup);
    for (auto const& keyMsg : keyEgo.second) {
      Napi::Object msgObj = Napi::Object::New(env);
      Napi::ArrayBuffer arrayBuf =
          Napi::ArrayBuffer::New(env, const_cast<void*>(keyMsg.second.data()), keyMsg.second.size());
      msgObj.Set("topic", keyMsg.first);
      msgObj.Set("content", arrayBuf);
      topicMsgs[msg_idx++] = msgObj;
    }
    egoObj.Set("messages", topicMsgs);
    egoMsgs[ego_idx++] = egoObj;
  }
  // int64_t time2 =
  //     std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch())
  //         .count();
  // s << "ParseStepMessage: "<< time2 - time1 << std::endl;
  msg_list.Set("timestamp", status.timestamp);
  msg_list["groups"] = egoMsgs;
}

// parse module status message from json string.
//
// input
// env: node env
// status: protobuf message
// cb: callback function for node
//
// output
// return value of this method is thed execution
static tx_sim::impl::CmdErrorCode HandleCmdStatus(Napi::Env& env, const CommandStatus& status, Napi::Function& cb) {
  Napi::Object result = Napi::Object::New(env);
  Napi::Array err_list = Napi::Array::New(env);
  result.Set("retCode", RetCodeMappings(status.ec));
  Napi::Array init_list = Napi::Array::New(env, status.init_status.size());
  Napi::Array updated_list = Napi::Array::New(env);
  ParseInitStatus(env, status.init_status, init_list, updated_list);
  result.Set("initStatus", init_list);
  result.Set("updatedModules", updated_list);
  Napi::Array module_list = Napi::Array::New(env);
  ParseModuleStatus(env, status.module_status, module_list, err_list);
  result.Set("cmdStatus", module_list);
  Napi::Object msg_list = Napi::Object::New(env);
  ParseStepMessage(env, status.step_message, msg_list);
  result.Set("msgStatus", msg_list);
  cb.Call(env.Global(), {result, err_list});
  return status.ec;
}

// parse command response message from json string.
// input
// env: node env
// json_str: json string
//
// output
// return node object value
static Napi::Object JsonParse(const Napi::Env& env, const std::string& json_str) {
  Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
  Napi::Function parse = json.Get("parse").As<Napi::Function>();
  return parse.Call(json, {Napi::String::New(env, json_str)}).As<Napi::Object>();
}

// convert node object value to std::string
// input
// env: node env
// json_obj: node object value
//
// output
// return std::string
static std::string JsonStringify(const Napi::Env& env, const Napi::Object& json_obj) {
  Napi::Object json = env.Global().Get("JSON").As<Napi::Object>();
  Napi::Function stringify = json.Get("stringify").As<Napi::Function>();
  return stringify.Call(json, {json_obj}).As<Napi::String>();
}


#ifdef _WIN32
extern "C" void strptime(const char* s, const char* f, struct tm* tm) {
  std::istringstream input(s);
  input.imbue(std::locale(setlocale(LC_ALL, nullptr)));
  input >> std::get_time(tm, f);
}
#endif  // _WIN32

// convert std::string time to time_t
// input
// s: std::string time
//
// output
// return time_t
time_t TimeString2Time(const std::string& s) {
  struct tm tmp;
  memset(&tmp, 0, sizeof(struct tm));
  strptime(s.c_str(), "%Y_%m_%d_%H_%M_%S", &tmp);
  return mktime(&tmp);
}

// convert time_t to std::string time
// input
// s: std::string time
//
// output
// return time_t
std::string Time2TimeString(const time_t& t) {
  struct tm* ptm = std::localtime(&t);
  char buf[32];
  strftime(buf, 32, "%Y-%m-%d %H:%M:%S", ptm);
  return buf;
}


/***************************************************************************************************
 ************************************ Player Calls *************************************************
 **************************************************************************************************/

// coordinatoor setup step,
// this function will be called by player to setup the coordinator,
// call CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) cmd_into type is kCmdSetup
//
// example
// addon.setup((status, err) => {
//      console.log("err: " + JSON.stringify(err));
//      console.log("status: " + util.inspect(status, false, 3, true));
//    }, { scenarioPath: "/home/nemo/.config/tadsim/scenario/scene/AEB_straight_001.sim" });
// status and error are callback functions that you need to implement yourself
// scenarioPath is the path of the scenario file
void Setup(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckResetArgs(info)) return;
  Napi::Function status_cb = info[0].As<Napi::Function>();
  std::string scenario_path(info[1].As<Napi::Object>().Get("scenarioPath").ToString());

  std::thread(
      [env, scenario_path](Napi::ThreadSafeFunction tsfn) {
        Client cli(g_endpoint);
        try {
          cli.Setup(scenario_path, [&tsfn](const CommandStatus& status) {
            tsfn.BlockingCall(new CommandStatus(status), [](Napi::Env env, Napi::Function jscb, CommandStatus* status) {
              HandleCmdStatus(env, *status, jscb);
              delete status;
            });
          });
        } catch (const std::exception& e) {
          tsfn.BlockingCall([](Napi::Env env, Napi::Function jscb) { HandleReturnCode(env, jscb, kClientConnErr); });
        }
        tsfn.Release();
      },
      Napi::ThreadSafeFunction::New(env, status_cb, "Setup", 0, 1))
      .detach();
}

// coordinator reset step,
// this function will be called by player to unreset the coordinator,
// call CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) cmd_into type is kCmdUnSetup
//
// example
// addon.unSetup((status, err) => {
//      console.log("err: " + JSON.stringify(err));
//      console.log("status: " + util.inspect(status, false, 3, true));
//    });
void UnSetup(const Napi::CallbackInfo& info) {
  if (!CheckCbArg(info)) return;
  Napi::Env env = info.Env();
  Napi::Function status_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    cli.UnSetup([&env, &status_cb](const CommandStatus& status) { HandleCmdStatus(env, status, status_cb); });
  } catch (const std::exception& e) { HandleReturnCode(env, status_cb, kClientConnErr, e.what()); }
}

// coordinator step,
// this function will be called by player to kCmdStep the coordinator,
// call CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) cmd_into type is kCmdStep
//
// example
// addon.step((status, err) => {
//      console.log("err: " + JSON.stringify(err));
//      console.log("status: " + util.inspect(status, false, 3, true));
//    });
void Step(const Napi::CallbackInfo& info) {
  if (!CheckCbArg(info)) return;
  Napi::Env env = info.Env();
  Napi::Function status_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    cli.Step([&env, &status_cb](const CommandStatus& status) { HandleCmdStatus(env, status, status_cb); });
  } catch (const std::exception& e) { HandleReturnCode(env, status_cb, kClientConnErr); }
}

// coordinator stop,
// this function will be called by player to kCmdStop the coordinator,
// call CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) cmd_into type is kCmdStop
//
// example
// addon.stop((status, err) => {
//      console.log("err: " + JSON.stringify(err));
//      console.log("status: " + util.inspect(status, false, 3, true));
//    });
void Stop(const Napi::CallbackInfo& info) {
  if (!CheckCbArg(info)) return;
  Napi::Env env = info.Env();
  Napi::Function status_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    cli.Stop([&env, &status_cb](const CommandStatus& status) { HandleCmdStatus(env, status, status_cb); });
  } catch (const std::exception& e) { HandleReturnCode(env, status_cb, kClientConnErr); }
}

// coordinator run,
// this function will be called by player to kCmdRun the coordinator,
// call CmdErrorCode Coordinator::Execute(const CommandInfo& cmd_info, bool async) cmd_into type is kCmdRun
//
// example
// addon.run((status, err) => {
//      console.log("err: " + JSON.stringify(err));
//      console.log("status: " + util.inspect(status, false, 3, true));
//    });
//
// status param eter is a callback function that will be called after coordinator run finished.
// HandleCmdStatus will be called when client recv a step command.
void Run(const Napi::CallbackInfo& info) {
  if (!CheckCbArg(info)) return;
  Napi::Env env = info.Env();
  Napi::Function status_cb = info[0].As<Napi::Function>();

  if (run_thread.joinable()) {
    HandleReturnCode(env, status_cb, kClientCmdReject);
    return;  // since last run thead not finished. should call pause first.
  }

  run_thread = std::thread(
      [](Napi::ThreadSafeFunction tsfn) {
        Client cli(g_endpoint);
        try {
          cli.Run([&tsfn](const CommandStatus& status) {
            CommandStatus* call_data = new CommandStatus(status);
            if (call_data->ec == kCmdAccepted) {
              napi_status st =
                  tsfn.NonBlockingCall(call_data, [](Napi::Env env, Napi::Function jscb, CommandStatus* status) {
                    HandleCmdStatus(env, *status, jscb);
                    delete status;
                  });
              if (st == napi_queue_full) delete call_data;
            } else {  // non-accepted-ec status may NOT be dropped.
              tsfn.BlockingCall(call_data, [](Napi::Env env, Napi::Function jscb, CommandStatus* status) {
                HandleCmdStatus(env, *status, jscb);
                delete status;
              });
            }
          });
        } catch (const std::exception& e) {
          tsfn.BlockingCall([](Napi::Env env, Napi::Function jscb) { HandleReturnCode(env, jscb, kClientConnErr); });
        }
        tsfn.Release();
      },
      Napi::ThreadSafeFunction::New(env, status_cb, "Run", 1, 1, [](Napi::Env) { run_thread.join(); }));
}

// pause the simulator
void Pause(const Napi::CallbackInfo& info) {
  if (!CheckCbArg(info)) return;
  Napi::Env env = info.Env();
  Napi::Function status_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    cli.Pause([&env, &status_cb](const CommandStatus& status) { HandleCmdStatus(env, status, status_cb); });
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

void SetHighlightGroup(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckStringArgs(info)) return;
  Client cli(g_endpoint);
  try {
    cli.SetHighlightGroup([&env](const CommandStatus& status) {}, info[1].As<Napi::String>());
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/***************************************************************************************************
 **************************** Config Manager Calls *************************************************
 **************************************************************************************************/

// add a new config
// example
// addon.addModuleConfig((status) => { console.log(status); }, {
//      name: "Planning",
//      stepTime: 100,
//      endpoint: "",
//      initArgs: {},
//      cmdTimeout: 3000,
//      stepTimeout: 10000,
//      autoLaunch: true,
//      depPaths: ["/opt/TADSim/resources/app/buildin/planning", "/opt/TADSim/resources/app/buildin/simdeps"],
//      soPath: "/opt/TADSim/resources/app/buildin/planning/libsim_planning.so",
//      binPath: "",
//      binArgs: []
//    });
void AddModuleConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.AddModuleConfig(JsonStringify(env, info[1].As<Napi::Object>())));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// remove module config
// example
// addon.removeModuleConfig((status) => { console.log(status); }, "Traffic");
void RemoveModuleConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckStringArgs(info)) return;
  std::string name = info[1].As<Napi::String>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.RemoveModuleConfig(name));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// update module config
// example
// addon.updateModuleConfig((status) => { console.log(status); }, {
//      name: "Traffic",
//      stepTime: 20,
//      endpoint: "tcp://127.0.0.1:8888",
//      initArgs: {},
//      cmdTimeout: 3000,
//      stepTimeout: 500,
//      autoLaunch: true,
//      depPaths: ["/opt/TADSim/resources/app/buildin/simdeps"],
//      soPath: "",
//      binPath: "/opt/TADSim/resources/app/buildin/txSimTraffic",
//      binArgs: ["tcp://127.0.0.1:8888"]
//    });
void UpdateModuleConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.UpdateModuleConfig(JsonStringify(env, info[1].As<Napi::Object>())));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// get module detail config
// example
// addon.getModuleConfig((status) => { console.log(status); }, "Traffic");
void GetModuleConfig(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckStringArgs(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();
  std::string name = info[1].As<Napi::String>();

  Client cli(g_endpoint);
  try {
    std::string config_str;
    CmdErrorCode ec = cli.GetModuleConfig(name, config_str);
    if (ec == kCmdSucceed) {
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, config_str));
      result.Set("retCode", RetCodeMappings(ec));
      config_cb.Call(env.Global(), {result});
    } else {
      HandleReturnCode(env, config_cb, ec);
    }
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// get all module detail configs
// example
// addon.getAllModuleConfigs((status) => { console.log(util.inspect(status, false, null, true)); });
// all module configs will in status.data field as an object with key being the module names and value is its json
// string
void GetAllModuleConfigs(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    std::vector<std::string> configs;
    CmdErrorCode ec = cli.GetAllModuleConfigs(configs);
    if (ec != kCmdSucceed) {
      HandleReturnCode(env, config_cb, ec);
      return;
    }

    Napi::Array cfg_list = Napi::Array::New(env, configs.size());
    for (size_t i = 0; i < configs.size(); ++i) cfg_list[i] = JsonParse(env, Napi::String::New(env, configs[i]));
    Napi::Object result = Napi::Object::New(env);
    result.Set("data", cfg_list);
    result.Set("retCode", RetCodeMappings(ec));
    config_cb.Call(env.Global(), {result});
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// add moudle schemem cofig
// example
// addon.addModuleScheme((status) => { console.log(status); }, {
//      name: "L4-Planning-test",
//      modules: ["Traffic", "Planning", "Perfect_Control"]
//    });
void AddModuleScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.AddModuleScheme(JsonStringify(env, info[1].As<Napi::Object>())));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// remove module scheme
// example
// addon.removeModuleScheme((status) => { console.log(status); }, scheme_id);
// the id is scheme_id of a specific scheme which you want to delete
void RemoveModuleScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckNumArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.RemoveModuleScheme(info[1].As<Napi::Number>().Int64Value()));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}


void UpdateModuleScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.UpdateModuleScheme(JsonStringify(env, info[1].As<Napi::Object>())));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}


void GetAllModuleSchemes(const Napi::CallbackInfo& info) {
  // 获取当前运行环境
  Napi::Env env = info.Env();
  // 检查第一个参数是否为回调函数
  if (!CheckCbArg(info)) return;
  // 获取传入的回调函数
  Napi::Function config_cb = info[0].As<Napi::Function>();

  // 创建一个客户端实例
  Client cli(g_endpoint);
  try {
    // 获取所有模块的方案列表
    std::string scheme_list;
    CmdErrorCode ec = cli.GetAllModuleSchemes(scheme_list);
    // 如果获取方案列表失败，则调用回调函数并返回错误信息
    if (ec != kCmdSucceed) {
      HandleReturnCode(env, config_cb, ec);
      return;
    }

    // 将方案列表转换为 JSON 对象
    Napi::Object result = Napi::Object::New(env);
    result.Set("data", JsonParse(env, scheme_list));
    // 设置返回码
    result.Set("retCode", RetCodeMappings(ec));
    // 调用回调函数并传递结果对象
    config_cb.Call(env.Global(), {result});
  } catch (const std::exception& e) {
    // 如果发生异常，调用回调函数并返回错误信息
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}

void UpdateMultiEgoScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.UpdateMultiEgoScheme(JsonStringify(env, info[1].As<Napi::Object>())));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}


void GetMultiEgoScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    std::string scheme_list;
    CmdErrorCode ec = cli.GetMultiEgoScheme(scheme_list);
    if (ec != kCmdSucceed) {
      HandleReturnCode(env, config_cb, ec);
      return;
    }

    Napi::Object result = Napi::Object::New(env);
    result.Set("data", JsonParse(env, Napi::String::New(env, scheme_list)));
    result.Set("retCode", RetCodeMappings(ec));
    config_cb.Call(env.Global(), {result});
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}


void SetActiveModuleScheme(const Napi::CallbackInfo& info) {
  // 获取当前运行环境
  Napi::Env env = info.Env();
  // 定义模块方案编号，初始化为99999
  int64_t moduleSchemeNum = 99999;

  // 检查输入参数的数量是否正确
  if (info.Length() == 2 && info[0].IsFunction() && info[1].IsNumber()) {
    // 如果第二个参数是数字，则获取其值并赋给 moduleSchemeNum
    moduleSchemeNum = info[1].As<Napi::Number>().Int64Value();
  }

  // 创建一个客户端实例
  Client cli(g_endpoint);

  try {
    // 调用客户端类的 SetActiveModuleScheme 方法，将方案编号作为参数传递
    // 如果调用成功，则调用 HandleReturnCode 函数处理返回结果
    // 如果调用失败，则调用 HandleReturnCode 函数处理错误信息
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.SetActiveModuleScheme(moduleSchemeNum));
  } catch (const std::exception& e) {
    // 如果发生异常，则调用 HandleReturnCode 函数处理错误信息
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}

/*
 * 函数名:
 *
 * 输入: 输入文件路径
 *
 * 输出: 无
 *
 * 描述: 这个函数的功能是将一个模块的配置导出.
 *
 * 参数 info 是一个 Napi::CallbackInfo 类型的变量，它包含了函数被调用时的上下文信息，如环境变量、调用参数等。
 *
 * 首先，我们从 info 中获取环境变量 env，并检查第一个参数是否为对象。如果不是，则直接返回。
 *
 * 接下来，我们从第二个参数（info[1]）中获取 "name" 和 "path" 属性，并检查它们是否为字符串类型。如果不是，则直接返回。
 *
 * 然后，我们创建一个 Client 类型的对象 cli，并使用给定的 g_endpoint 初始化。
 *
 * 接着，我们尝试调用 cli 的 ExportModuleScheme 方法，并将 result 传递给 HandleReturnCode
 * 函数，该函数用于处理返回值和错误代码。如果在此过程中捕获到任何异常，我们将返回 kClientConnErr 错误代码。
 */
void ExportModuleScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Value v_name = info[1].As<Napi::Object>().Get("name");
  Napi::Value v_path = info[1].As<Napi::Object>().Get("path");
  if (!v_name.IsString() || !v_path.IsString()) {
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientInvalidArgs);
    return;
  }

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.ExportModuleScheme(v_name.As<Napi::String>(), v_path.As<Napi::String>()));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*!\~Chinese
 * @brief 导入模块方案
 *
 * @param[in] Napi::CallbackInfo& info 包含函数调用所需的参数和上下文信息的结构
 *
 * @return 无
 */
void ImportModuleScheme(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckStringArgs(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    std::string imported_name;
    CmdErrorCode ec = cli.ImportModuleScheme(info[1].As<Napi::String>(), imported_name);
    if (ec == kCmdSucceed || ec == kCmdSchemeAlreadyExist) {
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, imported_name));
      result.Set("retCode", RetCodeMappings(ec));
      config_cb.Call(env.Global(), {result});
    } else {
      HandleReturnCode(env, config_cb, ec);
    }
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 设置仿真系统的配置.
 */
void SetSystemConfigs(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object cfg = info[1].As<Napi::Object>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.SetSysConfigs(JsonStringify(env, cfg)));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 获取仿真系统的配置
 */
void GetSystemConfigs(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    std::string config_str;
    CmdErrorCode ec = cli.GetSysConfigs(config_str);
    if (ec == kCmdSucceed) {
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, config_str));
      result.Set("retCode", RetCodeMappings(ec));
      config_cb.Call(env.Global(), {result});
    } else {
      HandleReturnCode(env, config_cb, ec);
    }
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*!
 * @brief 函数名：UpdatePlayList
 * @details 函数功能：更新播放列表
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void UpdatePlayList(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object id_list = info[1].As<Napi::Object>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.UpdatePlayList(JsonStringify(env, id_list)));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*!
 * @brief 函数名：GetPlayList
 * @details 函数功能：获取播放列表
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void GetPlayList(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();

  Client cli(g_endpoint);
  try {
    std::string play_list_str;
    CmdErrorCode ec = cli.GetPlayList(play_list_str);
    if (ec == kCmdSucceed) {
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, play_list_str));
      result.Set("retCode", RetCodeMappings(ec));
      config_cb.Call(env.Global(), {result});
    } else {
      HandleReturnCode(env, config_cb, ec);
    }
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*!
 * @brief 函数名：RemoveScenarios
 * @details 函数功能：从播放列表中移除场景
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void RemoveScenarios(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object id_list = info[1].As<Napi::Object>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.RemoveScenariosFromPlayList(JsonStringify(env, id_list)));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 添加评测指标
 */
void AddGradingKpis(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object kpis = info[1].As<Napi::Object>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.AddGradingKpis(JsonStringify(env, kpis)));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 删除评测指标
 */
void RemoveGradingKpi(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckNumArgs(info)) return;

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.RemoveGradingKpi(info[1].As<Napi::Number>().Int64Value()));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 更新评测指标
 */
void UpdateGradingKpi(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object kpi = info[1].As<Napi::Object>();

  Client cli(g_endpoint);
  try {
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.UpdateGradingKpi(JsonStringify(env, kpi)));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

/*
 * 函数名:
 *
 * 输入:
 *
 * 输出:
 *
 * 描述: 获取评测报告
 */
void GetGradingKpisInGroup(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckNumArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  int64_t group_id = info[1].As<Napi::Number>().Int64Value();

  Client cli(g_endpoint);
  try {
    std::string kpis_str;
    CmdErrorCode ec = cli.GetGradingKpisInGroup(group_id, kpis_str);
    if (ec == kCmdSucceed) {
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, kpis_str));
      result.Set("retCode", RetCodeMappings(ec));
      cb.Call(env.Global(), {result});
    } else {
      HandleReturnCode(env, cb, ec);
    }
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}


/*!
 * @brief 函数名：AddGradingKpiGroup
 * @details 函数功能：添加评分 KPI 组
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void AddGradingKpiGroup(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckObjArgs(info)) return;

  // 获取参数中的 kpi_group 对象
  Napi::Object kpi_group = info[1].As<Napi::Object>();

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 AddGradingKpiGroup 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.AddGradingKpiGroup(JsonStringify(env, kpi_group)));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：RemoveGradingKpiGroup
 * @details 函数功能：移除评分 KPI 组
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void RemoveGradingKpiGroup(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckNumArgs(info)) return;

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 RemoveGradingKpiGroup 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.RemoveGradingKpiGroup(info[1].As<Napi::Number>().Int64Value()));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：UpdateGradingKpiGroup
 * @details 函数功能：更新评分 KPI 组
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void UpdateGradingKpiGroup(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckObjArgs(info)) return;

  // 获取参数中的 kpi_group 对象
  Napi::Object kpi_group = info[1].As<Napi::Object>();

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 UpdateGradingKpiGroup 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.UpdateGradingKpiGroup(JsonStringify(env, kpi_group)));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：GetAllGradingKpiGroups
 * @details 函数功能：获取所有评分 KPI 组
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void GetAllGradingKpiGroups(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckCbArg(info)) return;

  // 获取参数中的回调函数
  Napi::Function cb = info[0].As<Napi::Function>();

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    std::string kpi_groups_str;
    // 调用 Client 对象的 GetAllGradingKpiGroups 方法
    CmdErrorCode ec = cli.GetAllGradingKpiGroups(kpi_groups_str);
    if (ec == kCmdSucceed) {
      // 如果成功，将结果转换为 JSON 对象并返回
      Napi::Object result = Napi::Object::New(env);
      result.Set("data", JsonParse(env, kpi_groups_str));
      result.Set("retCode", RetCodeMappings(ec));
      cb.Call(env.Global(), {result});
    } else {
      // 如果失败，将错误信息传递给 HandleReturnCode 函数处理
      HandleReturnCode(env, cb, ec);
    }
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：SetDefaultGradingKpiGroup
 * @details 函数功能：设置默认评分 KPI 组
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void SetDefaultGradingKpiGroup(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckNumArgs(info)) return;

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 SetDefaultGradingKpiGroup 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(),
                     cli.SetDefaultGradingKpiGroup(info[1].As<Napi::Number>().Int64Value()));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：RestoreDefaultConfigs
 * @details 函数功能：恢复默认配置
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void RestoreDefaultConfigs(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckCbArg(info)) return;

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 RestoreDefaultConfigs 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.RestoreDefaultConfigs());
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}

/***************************************************************************************************
 **************************** Module Manager Calls *************************************************
 **************************************************************************************************/

/*!
 * @brief 函数名：LaunchModule
 * @details 函数功能：启动模块
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void LaunchModule(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckStringArgs(info)) return;

  // 获取模块名称
  Napi::String name = info[1].As<Napi::String>();

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 LaunchModule 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.LaunchModule(name));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}


/*!
 * @brief 函数名：TerminateModule
 * @details 函数功能：终止模块
 *
 * @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
 *
 * @note 该函数使用了 Napi::CallbackInfo 类型的参数，用于处理 JavaScript 调用传入的参数和执行环境
 */
void TerminateModule(const Napi::CallbackInfo& info) {
  // 获取执行环境
  Napi::Env env = info.Env();

  // 检查参数是否符合要求
  if (!CheckStringArgs(info)) return;

  // 获取模块名称
  Napi::String name = info[1].As<Napi::String>();

  // 创建 Client 对象
  Client cli(g_endpoint);

  try {
    // 调用 Client 对象的 TerminateModule 方法，并将结果传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.TerminateModule(name));
  } catch (const std::exception& e) {
    // 如果发生异常，将异常信息传递给 HandleReturnCode 函数处理
    HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr);
  }
}

/***************************************************************************************************
 **************************** Local Function Calls *************************************************
 **************************************************************************************************/

//! @brief 结构体名：GradingKpiReportFileMeta
//! @details 用于存储评分 KPI 报告文件的元数据
struct GradingKpiReportFileMeta {
  std::vector<fs::path> array_json_path;  //!< 存储 JSON 文件路径的向量
  fs::path pblog_path;                    //!< pblog 文件路径
  std::string file_name;                  //!< 文件名
  size_t size_in_bytes = 0;               //!< 文件大小（字节）
};

//! @brief 函数名：ListKpiReports
//! @details 函数功能：列出可用的评分 KPI 报告
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void ListKpiReports(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();

  if (!fs::exists(g_grading_kpi_dir) || !fs::is_directory(g_grading_kpi_dir))
    HandleReturnCode(env, cb, kClientInvalidArgs, "grading kpi report directory path error: " + g_grading_kpi_dir);

  std::map<time_t, GradingKpiReportFileMeta, std::greater<time_t>> report_files;
  for (const fs::directory_entry& entry : boost::make_iterator_range(fs::directory_iterator(g_grading_kpi_dir), {})) {
    fs::path p(entry.path().filename());  // [P|F].[scenario_name].[date_time].dict
    const std::string ext = p.extension().string();

    if (ext != ".dict" && ext != ".pblog" && ext != ".tmp") continue;
    p = p.stem();  // remove file extension
    std::string time_str = p.extension().string().substr(1);
    auto& m = report_files[TimeString2Time(time_str)];
    fs::path item_path = entry.path();
    if (ext == ".dict" || ext == ".tmp") {
      m.array_json_path.push_back(item_path);
    } else {  // ".pblog"
      m.pblog_path = item_path;
    }
    m.size_in_bytes += fs::file_size(item_path);
    m.file_name = p.stem().string();  // remove time string
  }
  Napi::Array reports = Napi::Array::New(env);
  size_t total_size = 0, count = 0;
  for (const auto& item : report_files) {
    const time_t& t = item.first;
    const GradingKpiReportFileMeta& m = item.second;
    if (total_size > kMaxGradingKpiReportDirectorySize) {
      if (m.array_json_path.empty()) {
        fs::remove(m.pblog_path);
      } else {
        for (auto& path_item : m.array_json_path) { fs::remove(path_item); }
      }
    }
    total_size += m.size_in_bytes;
    if (m.array_json_path.empty()) continue;
    Napi::Object report = Napi::Object::New(env);
    report.Set("name", m.file_name.substr(2));
    report.Set("time", Time2TimeString(t));
    Napi::Array node_path_array = Napi::Array::New(env);
    size_t pathindex = 0;
    bool bPass = true;
    std::string filename;
    for (auto& node_path_array_item : m.array_json_path) {
      node_path_array[pathindex++] = node_path_array_item.string();
      filename = node_path_array_item.filename().string();
      if (filename.substr(0, 1) != "P") { bPass = false; }
    }
    report.Set("pass", bPass);
    report.Set("paths", node_path_array);
    reports[count++] = report;
  }

  Napi::Object result = Napi::Object::New(env);
  result.Set("data", reports);
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：GetKpiReportContent
//! @details 函数功能：获取评分 KPI 报告的内容
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void GetKpiReportContent(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckStringArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  const std::string report_path = info[1].As<Napi::String>();

  if (!fs::is_regular_file(report_path))
    HandleReturnCode(env, cb, kClientCmdReject, "kpi report file not exist: " + report_path);

  std::ifstream ifs(report_path);
  std::stringstream buf;
  buf << ifs.rdbuf();

  Napi::Object result = Napi::Object::New(env);
  result.Set("data", JsonParse(env, buf.str()));
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：RemoveKpiReports
//! @details 函数功能：移除评分 KPI 报告
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void RemoveKpiReports(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckArrayArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  Napi::Array report_paths = info[1].As<Napi::Array>();

  for (size_t i = 0; i < report_paths.Length(); ++i) {
    const std::string p = report_paths.Get(i).As<Napi::String>();
    fs::remove(p);
  }

  Napi::Object result = Napi::Object::New(env);
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：UploadReportImage
//! @details 函数功能：上传报告图像
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void UploadReportImage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  Napi::Value objID = info[1].As<Napi::Object>().Get("id");
  Napi::Value objFilePath = info[1].As<Napi::Object>().Get("filePath");

  std::string sID = objID.As<Napi::String>();
  std::string sFilePath = objFilePath.ToString().Utf8Value();
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wideString = converter.from_bytes(sFilePath);
  fs::path filePath(sFilePath);
  std::string fileExt = filePath.extension().string();

  std::string newFilePath = g_grading_kpi_dir + "/" + sID;
  for (const auto& itemExt : g_vecImageExt) {
    if (fs::is_regular_file(newFilePath + itemExt)) fs::remove(newFilePath + itemExt);
  }
  try {
    fs::copy(wideString, newFilePath + fileExt);
  } catch (const std::exception& e) {
    std::string err = e.what();
    std::cout << err << std::endl;
  }

  Napi::Object result = Napi::Object::New(env);
  Napi::Object resData = Napi::Object::New(env);
  resData.Set("resultPath", newFilePath + fileExt);
  result.Set("data", resData);
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：DeleteReportImage
//! @details 函数功能：删除报告图像
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void DeleteReportImage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  Napi::Value objID = info[1].As<Napi::Object>().Get("id");
  std::string sID = objID.As<Napi::String>();
  std::string newFilePath = g_grading_kpi_dir + "/" + sID;
  for (const auto& itemExt : g_vecImageExt) {
    if (fs::is_regular_file(newFilePath + itemExt)) fs::remove(newFilePath + itemExt);
  }

  Napi::Object result = Napi::Object::New(env);
  Napi::Object resData = Napi::Object::New(env);
  resData.Set("resultPath", "");
  result.Set("data", resData);
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：GetReportImage
//! @details 函数功能：获取报告图像
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void GetReportImage(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Function cb = info[0].As<Napi::Function>();
  Napi::Value objID = info[1].As<Napi::Object>().Get("id");
  std::string sID = objID.As<Napi::String>();
  std::string newFilePath = g_grading_kpi_dir + "/" + sID;
  std::string fileName;
  for (const auto& itemExt : g_vecImageExt) {
    if (fs::is_regular_file(newFilePath + itemExt)) {
      fileName = newFilePath + itemExt;
      break;
    }
  }

  Napi::Object result = Napi::Object::New(env);
  Napi::Object resData = Napi::Object::New(env);
  resData.Set("resultPath", "");
  result.Set("data", fileName);
  result.Set("retCode", 1);
  cb.Call(env.Global(), {result});
}

//! @brief 函数名：SetGradingLabelDescription
//! @details 函数功能：设置评分标签描述
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void SetGradingLabelDescription(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  std::string labelKey = info[1].As<Napi::Object>().Get("labelKey").As<Napi::String>();
  std::string labelDes = info[1].As<Napi::Object>().Get("labelDescription").As<Napi::String>();

  Json::Value jsonDocRoot;
  if (fs::is_regular_file(g_grading_label_path)) {
    tx_sim::utils::ReadJsonDocFromFile(jsonDocRoot, g_grading_label_path);
    Json::Value& node = jsonDocRoot["keyDescirption"];
    node[labelKey] = labelDes;
  } else {
    Json::Value node;
    node[labelKey] = labelDes;
    jsonDocRoot["keyDescirption"] = node;
  }
  tx_sim::utils::WriteJsonDocToFile(jsonDocRoot, g_grading_label_path);
  Napi::Function config_cb = info[0].As<Napi::Function>();
  Napi::Object result = Napi::Object::New(env);
  result.Set("retCode", 1);
  Napi::Object data = Napi::Object::New(env);
  data.Set("labelKey", labelKey);
  data.Set("labelDescription", labelDes);
  result.Set("data", data);
  config_cb.Call(env.Global(), {result});
}

//! @brief 函数名：DelGradingLabelDescription
//! @details 函数功能：删除评分标签描述
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void DelGradingLabelDescription(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  std::string labelKey = info[1].As<Napi::Object>().Get("labelKey").As<Napi::String>();
  Napi::Function config_cb = info[0].As<Napi::Function>();
  Napi::Object result = Napi::Object::New(env);

  if (!fs::is_regular_file(g_grading_label_path)) {
    result.Set("retCode", 1);
    config_cb.Call(env.Global(), {result});
    return;
  }
  Json::Value jsonDocRoot;
  tx_sim::utils::ReadJsonDocFromFile(jsonDocRoot, g_grading_label_path);
  Json::Value& node = jsonDocRoot["keyDescirption"];
  node.removeMember(labelKey);
  tx_sim::utils::WriteJsonDocToFile(jsonDocRoot, g_grading_label_path);


  result.Set("retCode", 1);
  config_cb.Call(env.Global(), {result});
}

//! @brief 函数名：GetGradingLabelDescriptions
//! @details 函数功能：获取评分标签描述列表
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void GetGradingLabelDescriptions(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckCbArg(info)) return;
  Napi::Function config_cb = info[0].As<Napi::Function>();
  Napi::Object result = Napi::Object::New(env);
  Napi::Array resArray = Napi::Array::New(env);
  if (!fs::is_regular_file(g_grading_label_path)) {
    result.Set("data", resArray);
    result.Set("retCode", 1);
    config_cb.Call(env.Global(), {result});
    return;
  }
  Json::Value jsonDocRoot;
  tx_sim::utils::ReadJsonDocFromFile(jsonDocRoot, g_grading_label_path);
  Json::Value& node = jsonDocRoot["keyDescirption"];

  int msg_idx = 0;
  for (const auto& item : node.getMemberNames()) {
    Napi::Object m = Napi::Object::New(env);
    m.Set("labelKey", item);
    std::string des = node.get(item, "").asString();
    m.Set("labelDescription", des);
    resArray[msg_idx++] = m;
  }

  result.Set("data", resArray);
  result.Set("retCode", 1);
  config_cb.Call(env.Global(), {result});
}

//! @brief 函数名：SetGradingLabel
//! @details 函数功能：设置评分标签
//!
//! @param[in] info Napi::CallbackInfo - 包含调用该函数的相关信息，如参数、执行环境等
void SetGradingLabel(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  if (!CheckObjArgs(info)) return;
  Napi::Object labelDesObj = info[1].As<Napi::Object>();
  Client cli(g_endpoint);
  try {
    std::string str = JsonStringify(env, labelDesObj);
    HandleReturnCode(env, info[0].As<Napi::Function>(), cli.SetGradingLabel(str));
  } catch (const std::exception& e) { HandleReturnCode(env, info[0].As<Napi::Function>(), kClientConnErr); }
}

// SetClientEndpoint：设置客户端端点
// SetAppPath：设置应用程序路径
// Setup：设置
// UnSetup：取消设置
// Step：执行一步操作
// Stop：停止操作
// Run：运行操作
// Pause：暂停操作
// AddModuleConfig：添加模块配置
// RemoveModuleConfig：删除模块配置
// UpdateModuleConfig：更新模块配置
// GetModuleConfig：获取模块配置
// GetAllModuleConfigs：获取所有模块配置
// AddModuleScheme：添加模块方案
// RemoveModuleScheme：删除模块方案
// UpdateModuleScheme：更新模块方案
// GetAllModuleSchemes：获取所有模块方案
// SetActiveModuleScheme：设置活动模块方案
// ExportModuleScheme：导出模块方案
// ImportModuleScheme：导入模块方案
// SetSystemConfigs：设置系统配置
// GetSystemConfigs：获取系统配置
// RestoreDefaultConfigs：恢复默认配置
// UpdatePlayList：更新播放列表
// GetPlayList：获取播放列表
// RemoveScenarios：从播放列表中删除场景
// AddGradingKpis：添加评分关键绩效指标
// RemoveGradingKpi：删除评分关键绩效指标
// UpdateGradingKpi：更新评分关键绩效指标
// GetGradingKpisInGroup：获取评分关键绩效指标组中的关键绩效指标
// AddGradingKpiGroup：添加评分关键绩效指标组
// RemoveGradingKpiGroup：删除评分关键绩效指标组
// UpdateGradingKpiGroup：更新评分关键绩效指标组
// GetAllGradingKpiGroups：获取所有评分关键绩效指标组
// SetDefaultGradingKpiGroup：设置默认评分关键绩效指标组
// ListKpiReports：列出关键绩效指标报告
// GetKpiReportContent：获取关键绩效指标报告内容
// RemoveKpiReports：删除关键绩效指标报告
// UploadReportImage：上传报告图像
// DeleteReportImage：删除报告图像
// GetReportImage：获取报告图像
// SetGradingLabelDescription：设置评分标签描述
// DelGradingLabelDescription：删除评分标签描述
// GetGradingLabelDescriptions：获取评分标签描述列表
// SetGradingLabel：设置评分标签
Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "setClientEndpoint"), Napi::Function::New(env, SetClientEndpoint));
  exports.Set(Napi::String::New(env, "setAppPath"), Napi::Function::New(env, SetAppPath));
  exports.Set(Napi::String::New(env, "setup"), Napi::Function::New(env, Setup));
  exports.Set(Napi::String::New(env, "unSetup"), Napi::Function::New(env, UnSetup));
  exports.Set(Napi::String::New(env, "step"), Napi::Function::New(env, Step));
  exports.Set(Napi::String::New(env, "stop"), Napi::Function::New(env, Stop));
  exports.Set(Napi::String::New(env, "run"), Napi::Function::New(env, Run));
  exports.Set(Napi::String::New(env, "pause"), Napi::Function::New(env, Pause));
  exports.Set(Napi::String::New(env, "setHighlightGroup"), Napi::Function::New(env, SetHighlightGroup));
  exports.Set(Napi::String::New(env, "addModuleConfig"), Napi::Function::New(env, AddModuleConfig));
  exports.Set(Napi::String::New(env, "removeModuleConfig"), Napi::Function::New(env, RemoveModuleConfig));
  exports.Set(Napi::String::New(env, "updateModuleConfig"), Napi::Function::New(env, UpdateModuleConfig));
  exports.Set(Napi::String::New(env, "getModuleConfig"), Napi::Function::New(env, GetModuleConfig));
  exports.Set(Napi::String::New(env, "getAllModuleConfigs"), Napi::Function::New(env, GetAllModuleConfigs));
  exports.Set(Napi::String::New(env, "addModuleScheme"), Napi::Function::New(env, AddModuleScheme));
  exports.Set(Napi::String::New(env, "removeModuleScheme"), Napi::Function::New(env, RemoveModuleScheme));
  exports.Set(Napi::String::New(env, "updateModuleScheme"), Napi::Function::New(env, UpdateModuleScheme));
  exports.Set(Napi::String::New(env, "getAllModuleSchemes"), Napi::Function::New(env, GetAllModuleSchemes));
  exports.Set(Napi::String::New(env, "setActiveModuleScheme"), Napi::Function::New(env, SetActiveModuleScheme));
  exports.Set(Napi::String::New(env, "exportModuleScheme"), Napi::Function::New(env, ExportModuleScheme));
  exports.Set(Napi::String::New(env, "importModuleScheme"), Napi::Function::New(env, ImportModuleScheme));
  exports.Set(Napi::String::New(env, "setSystemConfigs"), Napi::Function::New(env, SetSystemConfigs));
  exports.Set(Napi::String::New(env, "getSystemConfigs"), Napi::Function::New(env, GetSystemConfigs));
  exports.Set(Napi::String::New(env, "restoreDefaultConfigs"), Napi::Function::New(env, RestoreDefaultConfigs));
  exports.Set(Napi::String::New(env, "updatePlayList"), Napi::Function::New(env, UpdatePlayList));
  exports.Set(Napi::String::New(env, "getPlayList"), Napi::Function::New(env, GetPlayList));
  exports.Set(Napi::String::New(env, "removeScenariosFromPlayList"), Napi::Function::New(env, RemoveScenarios));
  exports.Set(Napi::String::New(env, "addGradingKpis"), Napi::Function::New(env, AddGradingKpis));
  exports.Set(Napi::String::New(env, "removeGradingKpi"), Napi::Function::New(env, RemoveGradingKpi));
  exports.Set(Napi::String::New(env, "updateGradingKpi"), Napi::Function::New(env, UpdateGradingKpi));
  exports.Set(Napi::String::New(env, "getGradingKpisInGroup"), Napi::Function::New(env, GetGradingKpisInGroup));
  exports.Set(Napi::String::New(env, "addGradingKpiGroup"), Napi::Function::New(env, AddGradingKpiGroup));
  exports.Set(Napi::String::New(env, "removeGradingKpiGroup"), Napi::Function::New(env, RemoveGradingKpiGroup));
  exports.Set(Napi::String::New(env, "updateGradingKpiGroup"), Napi::Function::New(env, UpdateGradingKpiGroup));
  exports.Set(Napi::String::New(env, "getAllGradingKpiGroups"), Napi::Function::New(env, GetAllGradingKpiGroups));
  exports.Set(Napi::String::New(env, "setDefaultGradingKpiGroup"), Napi::Function::New(env, SetDefaultGradingKpiGroup));
  exports.Set(Napi::String::New(env, "listKpiReports"), Napi::Function::New(env, ListKpiReports));
  exports.Set(Napi::String::New(env, "getKpiReport"), Napi::Function::New(env, GetKpiReportContent));
  exports.Set(Napi::String::New(env, "removeKpiReports"), Napi::Function::New(env, RemoveKpiReports));
  exports.Set(Napi::String::New(env, "uploadReportImage"), Napi::Function::New(env, UploadReportImage));
  exports.Set(Napi::String::New(env, "deleteReportImage"), Napi::Function::New(env, DeleteReportImage));
  exports.Set(Napi::String::New(env, "getReportImage"), Napi::Function::New(env, GetReportImage));
  exports.Set(Napi::String::New(env, "setGradingLabelDescription"),
              Napi::Function::New(env, SetGradingLabelDescription));
  exports.Set(Napi::String::New(env, "delGradingLabelDescription"),
              Napi::Function::New(env, DelGradingLabelDescription));
  exports.Set(Napi::String::New(env, "getGradingLabelDescriptions"),
              Napi::Function::New(env, GetGradingLabelDescriptions));
  exports.Set(Napi::String::New(env, "setGradingLabel"), Napi::Function::New(env, SetGradingLabel));
  exports.Set(Napi::String::New(env, "updateMultiEgoScheme"), Napi::Function::New(env, UpdateMultiEgoScheme));
  exports.Set(Napi::String::New(env, "getMultiEgoScheme"), Napi::Function::New(env, GetMultiEgoScheme));
  return exports;
}


NODE_API_MODULE(play_service, Init)
