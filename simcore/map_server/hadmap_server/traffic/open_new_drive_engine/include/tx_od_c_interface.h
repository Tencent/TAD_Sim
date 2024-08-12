// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define __tx_windows__
#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define __tx_linux__
#endif

#if defined(__tx_windows__)
#  ifdef TX_OD_STATIC_API
#    define TX_OD_API
#  else
#    ifdef TX_OD_DLL_API
#      define TX_OD_API __declspec(dllexport)
#    else
#      define TX_OD_API __declspec(dllimport)
#    endif
#  endif
#elif defined(__tx_linux__)
#  define TX_OD_API
#endif

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct tx_od_error_code {
  enum { ERR_DESC_SIZE = 256 };
  int32_t err_code;
  char err_desc[ERR_DESC_SIZE];
} tx_od_error_code_t;

typedef struct _tx_od_param_string {
  uint64_t length;
  char* data;
} tx_od_param_string_t;

typedef const char* tx_od_lpsz_t;
typedef int32_t tx_session_id_t;
/*
 * Function: init_log
 * ----------------------------
 *   init log path
 *
 *   returns: the init true/false.
 */
TX_OD_API bool init_log(const char* path);

/*
 * Function: create_session
 * ----------------------------
 *   Create new session
 *
 *   returns: the id of new session, -1 session error.
 */
TX_OD_API tx_session_id_t create_session();

/*
 * Function: release_session
 * ----------------------------
 *   Release special session resources
 *
 *   returns: error code.
 */
TX_OD_API tx_od_error_code_t release_session(const tx_session_id_t sHdr);

/*
 * Function: open_hadmap
 * ----------------------------
 *   Load hadmap, create map info.
 *
 *   returns: error code.
 */
TX_OD_API tx_od_error_code_t open_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path);

/*
 * Function: create_hadmap
 * ----------------------------
 *   Create hadmap, create map info.
 *
 *   returns: error code.
 */
TX_OD_API tx_od_error_code_t create_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                           const tx_od_param_string_t& cmd_json);
/*
 * Function: save_hadmap
 * ----------------------------
 *   Save current session map info.
 *
 *   returns: error code.
 */
TX_OD_API tx_od_error_code_t save_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                         tx_od_lpsz_t outhdmap_path);
/*
 * Function: modify_hadmap
 * ----------------------------
 *   Modify map structure according to json parameters.
 *
 *   returns: error code.
 */
TX_OD_API tx_od_error_code_t modify_hadmap(const tx_session_id_t sHdr, tx_od_lpsz_t hdmap_path,
                                           const tx_od_param_string_t cmd_json);

/*
 * Function: create_hadmap_string
 * ----------------------------
 * Release char element allocate in api.
 *
 * returns: error code.
 */
TX_OD_API tx_od_error_code_t create_hadmap_string(const tx_session_id_t sHdr, tx_od_param_string_t& ret);

/*
 * Function: release_param_string
 * ----------------------------
 *   Release char element allocate in api.
 *
 *   returns: error code.
 */
TX_OD_API void release_param_string(tx_od_param_string_t& _resource_allocate_in_api);

/*
 * Function: close hadmap
 * ----------------------------
 * close hadmap in api.
 *
 * returns: error code.
 */
TX_OD_API tx_od_error_code_t close_hadmap(tx_od_lpsz_t hdmap_path);

#ifdef __cplusplus
} /*extern "C" */
#endif
