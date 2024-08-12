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
  // Get the singleton instance of txSessionManager
  static txSessionManager& getInstance() {
    static txSessionManager instance;
    return instance;
  }

 private:
  // Constructor
  txSessionManager();

  // Destructor
  ~txSessionManager();

  // Delete copy constructor
  txSessionManager(const txSessionManager&) TX_DELETE;

  // Delete assignment operator
  txSessionManager& operator=(const txSessionManager&) TX_DELETE;

  // Clear the session manager data
  void clear() TX_NOEXCEPT;

 public:
  // Type alias for session ID
  using txSessionId = Base::txSessionId;

  // Base value for session IDs
  enum { SessionIdBase = 100 };

  // Initialize the log
  bool initlog(const char* dirpath) TX_NOEXCEPT;

  // Create a new session
  txSessionId create_session() TX_NOEXCEPT;

  // Release a session with the specified session ID
  void release_session(const txSessionId _sHdr, tx_od_error_info& errCode) TX_NOEXCEPT;

  // Query a session with the specified session ID
  Server::txMapSystemBasePtr query_session(const txSessionId _sHdr, tx_od_error_info& errCode) TX_NOEXCEPT;

  // Get all sessions
  std::unordered_map<txSessionId, Server::txMapSystemBasePtr>& get_allsession() TX_NOEXCEPT;

 protected:
  // Member variable for the map of sessions
  std::unordered_map<txSessionId, Server::txMapSystemBasePtr> mSessionMap;

  // Member variable for the atomic session ID base
  std::atomic<int32_t> mAtomicSessionIdBase{SessionIdBase};
};

TX_NAMESPACE_CLOSE(Server)
