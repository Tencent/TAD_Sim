// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "http_session.h"

#include <chrono>
#include <sstream>

#include "boost/algorithm/string/predicate.hpp"
#include "boost/beast/websocket.hpp"
#include "uriparser/Uri.h"

#include "http_json_body.h"
#include "ws_session.h"


namespace basio = boost::asio;
namespace beast = boost::beast;
namespace bhttp = beast::http;
namespace bws = beast::websocket;
using btcp = boost::asio::ip::tcp;
using namespace tx_sim::coordinator;
using namespace tx_sim::impl;
using namespace tx_sim::utils;


namespace tx_sim {
namespace service {

static const std::string kContentTypeTextPlain = "text/plain";
static const std::string kContentTypeApplicationJson = "application/json";


const size_t HttpSession::kMaxHttpRequestBodySizeInBytes = 10240;
const size_t HttpSession::kGenericRequestProcessTimeoutInSeconds = 10;


static bool ParseUriQueryParams(const std::string& target,
                                std::map<std::string, std::shared_ptr<std::string>>& queries) {
  UriUriA uri;
  const char* err_pos = nullptr;
  if (uriParseSingleUriA(&uri, target.c_str(), &err_pos) != URI_SUCCESS) {
    LOG(ERROR) << "url parsing error at pos starts from " << err_pos;
    return false;
  }
  UriQueryListA* query_list;
  int item_count = 0;
  if (uriDissectQueryMallocA(&query_list, &item_count, uri.query.first, uri.query.afterLast) != URI_SUCCESS) {
    LOG(ERROR) << "url parsing queries error.";
    return false;
  }
  while (query_list && item_count > 0) {
    queries[query_list->key] = std::make_shared<std::string>(query_list->value);
    query_list = query_list->next;
    --item_count;
  }
  uriFreeQueryListA(query_list);
  uriFreeUriMembersA(&uri);
  return true;
}


HttpSession::HttpSession(size_t session_id, boost::asio::ip::tcp::socket&& socket,
                         const std::shared_ptr<LocalServiceContext>& ctx, std::shared_ptr<SimLocalStatusWriter>& writer)
    : sid_(session_id),
      session_id_(std::string("[http-") + std::to_string(session_id) + "]"),
      stream_(std::move(socket)),
      context_(ctx),
      writer_(writer) {
  VLOG(3) << session_id_ << " new HTTP session created.";
}


HttpSession::~HttpSession() {
  VLOG(3) << session_id_ << " HTTP session destroyed.";
}


void HttpSession::Start() {
  basio::dispatch(stream_.get_executor(), beast::bind_front_handler(&HttpSession::DoRead, shared_from_this()));
}


void HttpSession::DoRead() {
  parser_.emplace();
  parser_->body_limit(kMaxHttpRequestBodySizeInBytes);
  stream_.expires_at((std::chrono::steady_clock::time_point::max)());
  bhttp::async_read(stream_, buffer_, *parser_, beast::bind_front_handler(&HttpSession::OnRead, shared_from_this()));
}


void HttpSession::OnRead(boost::beast::error_code ec, std::size_t bytes_read) {
  if (ec == bhttp::error::end_of_stream) {
    VLOG(3) << session_id_ << " read eof. client closed the connection.";
    return Close();
  }
  if (ec) {
    LOG(ERROR) << session_id_ << " http request read error: " << ec.message();
    return Close();
  }
  VLOG(2) << session_id_ << " read request with " << bytes_read << " bytes.";

  const std::string target(parser_->get().target());
  if (boost::starts_with(target, "/api/config/")) HandleConfigRequest(parser_->release(), target);
  else if (boost::starts_with(target, "/api/play/"))
    HandlePlayRequest(parser_->release(), target);
  else if (boost::starts_with(target, "/api/data/"))
    HandleDataRequest(parser_->release(), target);
  else
    SendBadReqResponse(target, parser_->get().method(), "unknown request");
}


void HttpSession::HandleConfigRequest(const bhttp::request<request_body_t>&& req, const std::string& target) {
  stream_.expires_after(std::chrono::seconds(kGenericRequestProcessTimeoutInSeconds));
  VLOG(2) << session_id_ << " processing config " << req.method() << " request " << target << " ...";
  bool keep_alive = req.keep_alive();
  bhttp::verb req_method = req.method();
  const std::string target_path(target.substr(0, target.find_first_of('?')));
  zmq::multipart_t input;
  zmq::multipart_t output;
  if (target_path == "/api/config/current") {
    if (req_method != bhttp::verb::get) return SendBadReqResponse(target, req_method, "invalid request method");
    CoordinatorConfig cfg;
    context_->core().cfgr->LoadCoordinatorConfig(cfg);
    SendOKResponse(cfg.EncodeToStr(), keep_alive);
  } else if (target == "/api/config/get_pb_info") {
    Command cmd(kCmdGetPbInfo);
    std::string sBody = req.body();
    input.addstr(sBody);
    context_->core().cfgr->Execute(cmd, input, output);
    SimGeneralRespBody rst(kCmdAccepted, output.popstr());
    SendOKResponse(rst.Encode(), keep_alive);
  } else if (target == "/api/config/del_pb_info") {
    Command cmd(kCmdDelPbInfo);
    std::string sBody = req.body();
    if (sBody.empty()) return SendBadReqResponse(target, req_method, "body is empty");
    input.addstr(sBody);
    CmdErrorCode errCode = context_->core().cfgr->Execute(cmd, input, output);
    SimGeneralRespBody rst(errCode, "");
    SendOKResponse(rst.Encode(), keep_alive);
  } else if (target == "/api/config/update_pb_info") {
    std::string sBody = req.body();
    if (sBody.empty()) return SendBadReqResponse(target, req_method, "body is empty");
    input.addstr(sBody);
    Command cmd(kCmdUpdatePbInfo);
    CmdErrorCode errCode = context_->core().cfgr->Execute(cmd, input, output);
    SimGeneralRespBody rst(errCode, output.popstr());
    SendOKResponse(rst.Encode(), keep_alive);
  } else if (target_path == "/api/config/set_grading_kpi_label") {
    Command cmd(kCmdSetGradingKpiLabels);
    std::string sBody = req.body();
    if (sBody.empty()) return SendBadReqResponse(target, req_method, "body is empty");
    input.addstr(sBody);
    CmdErrorCode errCode = context_->core().cfgr->Execute(cmd, input, output);
    SimGeneralRespBody rst(errCode, "");
    SendOKResponse(rst.Encode(), keep_alive);
  } else if (target_path == "/api/config/add_grading_kpis") {
    Command cmd(kCmdAddGradingKpi);
    std::string sBody = req.body();
    if (sBody.empty()) return SendBadReqResponse(target, req_method, "body is empty");
    input.addstr(sBody);
    CmdErrorCode errCode = context_->core().cfgr->Execute(cmd, input, output);
    SimGeneralRespBody rst(errCode, "");
    SendOKResponse(rst.Encode(), keep_alive);
  } else if (target_path == "/api/config/l2w_params") {
    switch (req_method) {
      case bhttp::verb::get: {
        std::map<std::string, std::shared_ptr<std::string>> queries;
        ParseUriQueryParams(target, queries);
        auto it = queries.find("scene_path");
        if (it == queries.end() || it->second == nullptr || it->second->empty())
          return SendBadReqResponse(target, req_method, "no scene_path query parameter provided");
        std::string scene_path = context_->core().snpr->CheckIsOpenScenario(*it->second);
        std::replace(scene_path.begin(), scene_path.end(), ' ', '+');
        SimL2wParamsGetRespBody rst;
        try {
          rst.config = context_->GetScenarioLog2WorldConfig(scene_path);
          context_->core().snpr->ParseLogsimEvents(scene_path, rst.events);
        } catch (const std::exception& e) {
          rst.code = kCmdFailed;
          rst.msg = e.what();
        }
        context_->UpdateScenarioLog2WorldConfig(scene_path, rst.config);
        SendOKResponse(rst.Encode(), keep_alive);
        break;
      }

      case bhttp::verb::post: {
        SimL2wParamsPostReqBody req_body;
        std::string err;
        if (!req_body.Decode(req.body(), err))
          return SendBadReqResponse(target, req_method, std::string("Invalid request body: ") + err);
        req_body.scene_path = context_->core().snpr->CheckIsOpenScenario(req_body.scene_path);
        SimGeneralRespBody rst(kCmdSucceed, "");
        context_->UpdateScenarioLog2WorldConfig(req_body.scene_path, req_body.config);
        SendOKResponse(rst.Encode(), keep_alive);
        break;
      }

      default: SendBadReqResponse(target, req_method, "invalid l2w_params request method");
    }

  } else {
    SendBadReqResponse(target, req_method, "unknown config request");
  }
}


void HttpSession::HandlePlayRequest(const bhttp::request<request_body_t>&& req, const std::string& target) {
  stream_.expires_after(std::chrono::seconds(kGenericRequestProcessTimeoutInSeconds));
  VLOG(2) << session_id_ << " processing play " << req.method() << " request " << target << " ...";
  bool keep_alive = req.keep_alive();
  bhttp::verb req_method = req.method();

  if (target == "/api/play/poll") {
    if (req_method != bhttp::verb::get) return SendBadReqResponse(target, req_method, "invalid request method");
    SimPlayPollRespBody resp;
    resp.scenario_running = context_->core().codr->is_cmd_executing();
    SendOKResponse(resp.Encode(), keep_alive);

  } else if (target == "/api/play/run") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    SimRunReqBody req_body;
    std::string err_msg;
    if (!req_body.Decode(req.body(), err_msg))
      return SendBadReqResponse(target, req_method, std::string("Invalid request body: ") + err_msg);
    CommandInfo c(kCmdSetup);
    if (writer_) c.status_cb = [this](const CommandStatus& st) { writer_->Write(st); };
    std::string scenario_path = context_->core().snpr->CheckIsOpenScenario(req_body.scenario_path);
    c.reset_params.scenario_path = scenario_path;
    try {
      if (!scenario_path.empty()) {  // log2world supported by .sim only currently.
        context_->core().snpr->ParseLog2WorldConfig(scenario_path, c.l2w_config);
        context_->core().snpr->ParseLogsimEvents(scenario_path, c.logsim_events);
        context_->UpdateScenarioLog2WorldConfig(scenario_path, c.l2w_config);
      }
    } catch (const std::exception& e) {
      LOG(ERROR) << "handling " << target << " error: " << e.what();
      return SendOKResponse(SimGeneralRespBody{kCmdFailed, e.what()}.Encode(), keep_alive);
    }
    c.pause_on_start = req_body.pause_on_start;
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ec == kCmdRejected ? "current scenario is running." : ""}.Encode(),
                   keep_alive);

  } else if (target == "/api/play/stop") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    CommandInfo c(kCmdUnSetup);
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ""}.Encode(), keep_alive);

  } else if (target == "/api/play/pause") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    CommandInfo c(kCmdPause);
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ""}.Encode(), keep_alive);

  } else if (target == "/api/play/resume") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    CommandInfo c(kCmdRun);
    if (writer_) c.status_cb = [this](const CommandStatus& st) { writer_->Write(st); };
    c.l2w_config = context_->GetScenarioLog2WorldConfig(context_->core().codr->current_scenario());
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ""}.Encode(), keep_alive);

  } else if (target == "/api/play/step") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    CommandInfo c(kCmdStep);
    if (writer_) c.status_cb = [this](const CommandStatus& st) { writer_->Write(st); };
    c.l2w_config = context_->GetScenarioLog2WorldConfig(context_->core().codr->current_scenario());
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ""}.Encode(), keep_alive);
  } else if (target == "/api/play/terminate_setup") {
    if (req_method != bhttp::verb::post) return SendBadReqResponse(target, req_method, "invalid request method");
    CommandInfo c(kCmdTerminateSteup);
    CmdErrorCode ec = context_->core().codr->Execute(c);
    SendOKResponse(SimGeneralRespBody{ec, ""}.Encode(), keep_alive);
  } else {
    SendBadReqResponse(target, req_method, "unknown play request");
  }
}


void HttpSession::HandleDataRequest(const bhttp::request<request_body_t>&& req, const std::string& target) {
  if (!bws::is_upgrade(req)) return SendBadReqResponse(target, req.method(), "websocket upgrade expected");
  VLOG(3) << session_id_ << " received websocket upgrade request: " << target;
  if (target == "/api/data/status") {
    boost::make_shared<WsSession>(stream_.release_socket(), context_->core().codr, sid_, writer_)->Start(req);
  } else if (target == "/api/data/raw_msg") {
    boost::make_shared<WsSession>(stream_.release_socket(), context_->core().codr, sid_, writer_, true)->Start(req);
  } else {
    SendBadReqResponse(target, req.method(), "unknown websocket request");
  }
}


void HttpSession::SendBadReqResponse(const std::string& target, boost::beast::http::verb method,
                                     const std::string& err) {
  std::ostringstream ss;
  ss << "Invalid " << method << " request " << target << ": " << err;
  LOG(ERROR) << ss.str();
  SendResponse(bhttp::status::bad_request, kContentTypeTextPlain, ss.str(), false);
}


void HttpSession::SendOKResponse(const std::string& body, bool keep_alive) {
  SendResponse(bhttp::status::ok, kContentTypeApplicationJson, body, keep_alive);
}


void HttpSession::SendResponse(bhttp::status status, const std::string& content_type, const std::string& body,
                               bool keep_alive) {
  response_.emplace();
  response_->result(status);
  response_->keep_alive(keep_alive);
  response_->set(bhttp::field::server, "txSim");
  response_->set(bhttp::field::content_type, content_type);
  response_->content_length(body.size());
  response_->body() = body;

  DoWrite();
}


void HttpSession::DoWrite() {
  bhttp::async_write(stream_, *response_,
                     beast::bind_front_handler(&HttpSession::OnWrite, shared_from_this(), response_->need_eof()));
}


void HttpSession::OnWrite(bool close, boost::beast::error_code ec, std::size_t bytes_written) {
  if (ec) {
    LOG(ERROR) << session_id_ << " writting error: " << ec.message();
    return Close();
  }
  if (close) {
    VLOG(3) << session_id_ << " closing the connection since the response indicates close semantic.";
    return Close();
  }
  VLOG(2) << session_id_ << " written response with " << bytes_written << " bytes.";

  DoRead();
}


void HttpSession::Close() {
  auto& socket = stream_.socket();
  if (!socket.is_open()) return;
  beast::error_code ec;
  socket.shutdown(btcp::socket::shutdown_send, ec);
  if (ec) LOG(ERROR) << session_id_ << " clean up(shutdown) socket error: " << ec.message();
  else
    VLOG(3) << session_id_ << " connection closed gracefully.";
  // socket.close(ec);
  // if (ec) LOG(ERROR) << session_id_ << " clean up(close) socket error: " << ec.message();
}

}  // namespace service
}  // namespace tx_sim
