// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

#include "json/json.h"

#include "coordinator/config.h"
#include "utils/constant.h"

namespace tx_sim {
namespace service {

struct HttpJsonBody {
  virtual const std::string Encode() { return ""; }
  virtual bool Decode(const std::string& payload, std::string& err) { return true; }
};

struct SimGeneralRespBody : public HttpJsonBody {
  tx_sim::impl::CmdErrorCode code;
  std::string msg;

  SimGeneralRespBody(tx_sim::impl::CmdErrorCode c, const std::string& m) : code(c), msg(m) {}
  const std::string Encode() override;

 private:
  virtual void EncodeFields(Json::Value& root) {}
};

struct SimPlayPollRespBody final : public SimGeneralRespBody {
  bool scenario_running;

  SimPlayPollRespBody() : SimGeneralRespBody(tx_sim::impl::kCmdSucceed, "") {}
  void EncodeFields(Json::Value& root) override;
};

struct SimRunReqBody final : public HttpJsonBody {
  std::string scenario_path;
  bool pause_on_start = false;

  bool Decode(const std::string& payload, std::string& err) override;
};

struct SimL2wParamsGetRespBody final : public SimGeneralRespBody {
  tx_sim::coordinator::Log2WorldPlayConfig config;
  tx_sim::coordinator::LogsimEventInfo events;

  SimL2wParamsGetRespBody() : SimGeneralRespBody(tx_sim::impl::kCmdSucceed, "") {}
  void EncodeFields(Json::Value& root) override;
};

struct SimL2wParamsPostReqBody final : public HttpJsonBody {
  std::string scene_path;
  tx_sim::coordinator::Log2WorldPlayConfig config;

  bool Decode(const std::string& payload, std::string& err) override;
};

}  // namespace service
}  // namespace tx_sim
