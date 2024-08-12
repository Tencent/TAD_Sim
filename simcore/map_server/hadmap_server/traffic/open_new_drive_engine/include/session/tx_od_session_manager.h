// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <atomic>
#include <unordered_map>
#include "./common/tx_od_header.h"
#include "tx_od_error_code.h"
#include "tx_od_map_system_base.h"

TX_NAMESPACE_OPEN(Server)
class txSessionManager {
 public:
  static txSessionManager& getInstance() {
    static txSessionManager instance;
    return instance;
  }

 private:
  txSessionManager();
  ~txSessionManager();
  txSessionManager(const txSessionManager&) TX_DELETE;
  txSessionManager& operator=(const txSessionManager&) TX_DELETE;

  void clear() TX_NOEXCEPT;

 public:
  using txSessionId = Base::txSessionId;
  enum { SessionIdBase = 100 };
  bool initlog(const char* dirpath) TX_NOEXCEPT;
  txSessionId create_session() TX_NOEXCEPT;
  void release_session(const txSessionId _sHdr, tx_od_error_info& errCode) TX_NOEXCEPT;
  Server::txMapSystemBasePtr query_session(const txSessionId _sHdr, tx_od_error_info& errCode) TX_NOEXCEPT;
  std::unordered_map<txSessionId, Server::txMapSystemBasePtr>& get_allsession() TX_NOEXCEPT;

 protected:
  std::unordered_map<txSessionId, Server::txMapSystemBasePtr> mSessionMap;
  std::atomic<int32_t> mAtomicSessionIdBase{SessionIdBase};
};

TX_NAMESPACE_CLOSE(Server)
