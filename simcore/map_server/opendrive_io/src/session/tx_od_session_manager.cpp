// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_od_session_manager.h"
#include "tx_od_api_assist.h"
#include "tx_od_flags.h"
#include "tx_od_log.h"
#include "tx_od_map_system_base.h"
#include "tx_od_map_system_sqlite.h"

TX_NAMESPACE_OPEN(Server)

txSessionManager::txSessionManager() {}

txSessionManager::~txSessionManager() { Base::Logger::Release(); }

bool txSessionManager::initlog(const char* dirpath) TX_NOEXCEPT {
  // Initialize the logger
  Base::Logger::Initialize(dirpath);

  // Print the flag values
  PrintFlagsValues();

  return true;
}

void txSessionManager::release_session(const txSessionId _sHdr, tx_od_error_info& errCode) TX_NOEXCEPT {
  // Check if the session ID is in the session map
  if (_Contain_(mSessionMap, _sHdr)) {
    auto itr = mSessionMap.find(_sHdr);

    // Check if the session pointer is not null
    if (NonNull_Pointer((*itr).second)) {
      (*itr).second->clear();
    }

    // Erase the session from the map
    mSessionMap.erase(itr);

    // Set the success error code
    Utils::make_error_code(txErrorCode::EC_SUCCESS, "", errCode);
  } else {
    // Log a warning for unknown session ID
    LOG(WARNING) << "Unknown Session Id : " << _sHdr;

    // Set the unknown session error code
    Utils::make_error_code(txErrorCode::EC_UNKNOW_SESSION, "session id not find.", errCode);
  }
}

void txSessionManager::clear() TX_NOEXCEPT {
  // Clear the session map
  mSessionMap.clear();

  // Reset the atomic session ID base
  mAtomicSessionIdBase.store(SessionIdBase, std::memory_order_relaxed);
}

txSessionManager::txSessionId txSessionManager::create_session() TX_NOEXCEPT {
  // Create a new session ID
  const int32_t newId = mAtomicSessionIdBase++;

  // Create a new map system and add it to the session map
  mSessionMap[newId] = std::make_shared<Server::txMapSystemSqlite>();

  // Check if the map system pointer is not null
  if (NonNull_Pointer(mSessionMap[newId])) {
    return newId;
  } else {
    // Log a warning for failed map system creation
    LOG(WARNING) << "create map system failure.";

    return -1;
  }
}

Server::txMapSystemBasePtr txSessionManager::query_session(const txSessionId _sHdr,
                                                           tx_od_error_info& errCode) TX_NOEXCEPT {
  // Check if the session ID is in the session map
  if (_Contain_(mSessionMap, _sHdr)) {
    // Set the success error code
    Utils::make_error_code(txErrorCode::EC_SUCCESS, "", errCode);

    // Return the session with the specified ID
    return mSessionMap.at(_sHdr);
  } else {
    // Set the unknown session error code
    Utils::make_error_code(txErrorCode::EC_UNKNOW_SESSION, "session id not find.", errCode);

    return nullptr;
  }
}

std::unordered_map<txSessionManager::txSessionId, Server::txMapSystemBasePtr>& txSessionManager::get_allsession()
    TX_NOEXCEPT {
  // Return the session map
  return mSessionMap;
};
TX_NAMESPACE_CLOSE(Server)
