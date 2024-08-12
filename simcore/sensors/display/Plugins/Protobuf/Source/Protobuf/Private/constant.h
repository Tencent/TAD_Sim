#pragma once

#include <stdint.h>
#include <sstream>
#include <string>


namespace tx_sim {
namespace impl {

enum Command : int16_t {
  kCmdUnSetup = 0,
  kCmdSetup = 1,
  kCmdReset = 2,
  kCmdStep = 3,
  kCmdRun = 4,
  kCmdPause = 5,
  kCmdStop = 6,
  kCmdMaxPlayerType = 100,
  kCmdLaunchModule = 101,
  kCmdTerminateModule = 102,
  kCmdMaxModuleMgrType = 200,
  kCmdAddModuleConfig = 201,
  kCmdRemoveModuleConfig = 202,
  kCmdUpdateModuleConfig = 203,
  kCmdGetModuleConfig = 204,
  kCmdGetAllModuleConfigs = 205,
  kCmdAddModuleScheme = 206,
  kCmdRemoveModuleScheme = 207,
  kCmdUpdateModuleScheme = 208,
  kCmdGetAllModuleScheme = 209,
  kCmdSetActiveModuleScheme = 210,
  kCmdExportModuleScheme = 211,
  kCmdImportModuleScheme = 212,
  kCmdSetSysConfigs = 213,
  kCmdGetSysConfigs = 214,
  kCmdRestoreDefaultConfigs = 215,
  kCmdUpdatePlayList = 216,
  kCmdGetPlayList = 217,
  kCmdRemoveScenariosFromPlayList = 218,
  kCmdAddGradingKpi = 219,
  kCmdRemoveGradingKpi = 220,
  kCmdUpdateGradingKpi = 221,
  kCmdGetGradingKpisInGroup = 222,
  kCmdAddGradingKpiGroup = 223,
  kCmdRemoveGradingKpiGroup = 224,
  kCmdUpdateGradingKpiGroup = 225,
  kCmdGetAllGradingKpiGroups = 226,
  kCmdSetDefaultGradingKpiGroup = 227,
  kCmdMaxConfigMgrType = 300,
};

inline std::string Enum2String(Command c) {
  switch (c) {
    case kCmdSetup: return "CMD_SETUP";
    case kCmdUnSetup: return "CMD_UNSETUP";
    case kCmdReset: return "CMD_RESET";
    case kCmdStep: return "CMD_STEP";
    case kCmdRun: return "CMD_RUN";
    case kCmdPause: return "CMD_PAUSE";
    case kCmdStop: return "CMD_STOP";
    case kCmdLaunchModule: return "CMD_LAUNCH_MODULE";
    case kCmdTerminateModule: return "CMD_TERM_MODULE";
    case kCmdAddModuleConfig: return "CMD_ADD_MODULE_CONFIG";
    case kCmdRemoveModuleConfig: return "CMD_REMOVE_MODULE_CONFIG";
    case kCmdUpdateModuleConfig: return "CMD_UPDATE_MODULE_CONFIG";
    case kCmdGetModuleConfig: return "CMD_GET_MODULE_CONFIG";
    case kCmdGetAllModuleConfigs: return "CMD_GET_ALL_MODULE_CONFIGS";
    case kCmdAddModuleScheme: return "CMD_ADD_MODULE_SCHEME";
    case kCmdRemoveModuleScheme: return "CMD_REMOVE_MODULE_SCHEME";
    case kCmdUpdateModuleScheme: return "CMD_UPDATE_MODULE_SCHEME";
    case kCmdGetAllModuleScheme: return "CMD_GET_ALL_MODULE_SCHEME";
    case kCmdSetActiveModuleScheme: return "CMD_SET_ACTIVE_MODULE_SCHEME";
    case kCmdExportModuleScheme: return "CMD_EXPORT_MODULE_SCHEME";
    case kCmdImportModuleScheme: return "CMD_IMPORT_MODULE_SCHEME";
    case kCmdSetSysConfigs: return "CMD_SET_SYS_CONFIGS";
    case kCmdGetSysConfigs: return "CMD_GET_SYS_CONFIGS";
    case kCmdRestoreDefaultConfigs: return "CMD_RESTORE_DEFAULT_CONFIGS";
    case kCmdUpdatePlayList: return "CMD_UPDATE_PLAY_LIST";
    case kCmdGetPlayList: return "CMD_GET_PLAY_LIST";
    case kCmdRemoveScenariosFromPlayList: return "CMD_REMOVE_SCENARIO_FROM_PLAY_LIST";
    case kCmdAddGradingKpi: return "CMD_ADD_GRADING_KPI";
    case kCmdRemoveGradingKpi: return "CMD_REMOVE_GRADING_KPI";
    case kCmdUpdateGradingKpi: return "CMD_UPDATE_GRADING_KPI";
    case kCmdGetGradingKpisInGroup: return "CMD_GET_GRADING_KPIS_IN_GROUP";
    case kCmdAddGradingKpiGroup: return "CMD_ADD_GRADING_KPI_GROUP";
    case kCmdRemoveGradingKpiGroup: return "CMD_REMOVE_GRADING_KPI_GROUP";
    case kCmdUpdateGradingKpiGroup: return "CMD_UPDATE_GRADING_KPI_GROUP";
    case kCmdGetAllGradingKpiGroups: return "CMD_GET_ALL_GRADING_KPI_GROUPS";
    case kCmdSetDefaultGradingKpiGroup: return "CMD_SET_DEFAULT_GRADING_KPI_GROUP";
    default: return std::to_string(static_cast<int16_t>(c));
  }
}

enum CmdErrorCode : int16_t {
  kCmdAccepted = 0,
  kCmdIgnored = 2,
  kCmdCancelled = 3,
  kCmdRejected = 10,
  kCmdServerBusy = 20,
  kCmdSucceed = 100,
  kCmdSchemeAlreadyExist = 200,
  kCmdScenarioTimeout = 500,
  kCmdScenarioStopped = 900,
  kCmdInvalidTopicPubSub = 1500,
  kCmdFailed = 3000,
  kCmdScenarioParsingError = 20000,
  kCmdLogPlayModuleNotFound = 20100,
  kCmdConfictLogPlayShadowModule = 20200,
  kCmdInvalidModuleConfig = 20300,
  kCmdShmemCreationError = 29000,
  kCmdSystemError = 30000,
};

inline std::string Enum2String(CmdErrorCode c) {
  switch (c) {
    case kCmdSucceed: return "CMD_SUCCEED";
    case kCmdAccepted: return "CMD_ACCEPTED";
    case kCmdCancelled: return "CMD_CANCELLED";
    case kCmdIgnored: return "CMD_IGNORED";
    case kCmdScenarioStopped: return "CMD_SCENARIO_STOPPED";
    case kCmdRejected: return "CMD_REJECTED";
    case kCmdFailed: return "CMD_FAILED";
    case kCmdSystemError: return "CMD_SYS_ERROR";
    case kCmdInvalidTopicPubSub: return "CMD_INVALID_TOPIC_PUBSUB";
    case kCmdScenarioParsingError: return "CMD_SCENARIO_PARSING_ERROR";
    case kCmdScenarioTimeout: return "CMD_SCENARIO_TIMEOUT";
    case kCmdShmemCreationError: return "CMD_SHMEM_CREATION_ERROR";
    case kCmdServerBusy: return "CMD_SERVER_BUSY";
    case kCmdLogPlayModuleNotFound: return "CMD_LOG_PLAY_MODULE_NOT_FOUND";
    case kCmdConfictLogPlayShadowModule: return "CMD_CONFICT_LOGPLAY_SHADOW_MODULE";
    case kCmdInvalidModuleConfig: return "CMD_INVALID_MODULE_CONFIG";
    case kCmdSchemeAlreadyExist: return "CMD_SCHEME_ALREADY_EXIST";
    default: return "UNKNOWN_CMD_ERR_CODE";
  }
}

enum ModuleInitState : int16_t {
  kModuleInitStart = 0,
  kModuleInitConnecting = 2,
  kModuleInitConnected = 3,
  kModuleInitReqSent = 4,
  kModuleInitSucceed = 5,
  kModuleInitFailed = 6,
};

inline std::string Enum2String(ModuleInitState s) {
  switch (s) {
    case kModuleInitStart: return "MODULE_INIT_START";
    case kModuleInitConnecting: return "MODULE_INIT_CONNECTING";
    case kModuleInitConnected: return "MODULE_INIT_CONNECTED";
    case kModuleInitReqSent: return "MODULE_INIT_REQ_SENT";
    case kModuleInitSucceed: return "MODULE_INIT_SUCCEED";
    case kModuleInitFailed: return "MODULE_INIT_FAILED";
    default: return "UNKNOWN_MODULE_INIT_TYPE";
  }
}

enum ModuleMessageType : int16_t {
  kModuleRegisterSucceed = 0,
  kModuleRegisterFailed = 1,
  kModuleRegister = 2,
  kModuleUnRegister = 3,
  kModuleHeartbeat = 5,
  kModuleHeartbeatDisconnecting = 6,
  kModuleRequest = 7,
  kModuleResponse = 8,
  kModuleConnVersionIncompatible = 10000,
  kModuleMessageErrorType = 30000,
};

inline std::string Enum2String(ModuleMessageType r) {
  switch (r) {
    case kModuleRegister: return "MODULE_REGISTER";
    case kModuleUnRegister: return "MODULE_UNREGISTER";
    case kModuleHeartbeat: return "MODULE_HEARTBEAT";
    case kModuleResponse: return "MODULE_RESPONSE";
    default: return std::string("UNKNOWN_MODULE_MESSAGE_TYPE:") + std::to_string(r);
  }
}

enum ModuleRequestType : int16_t {
  kModuleInit = 1,
  kModuleReset = 2,
  kModuleStep = 3,
  kModuleStop = 4,
};

enum ModuleResponseType : int16_t {
  kModuleOK = 0,
  kModuleRequireStop = 1,
  kModuleTimeout = 2,
  kModuleError = 3,
  kModuleVersionIncompatible = 4,
  kModuleSystemError = 5,
  kModuleReInit = 6,
  kModuleProcessExit = 7,
};

inline std::string Enum2String(ModuleResponseType t) {
  switch (t) {
    case kModuleOK: return "MODULE_OK";
    case kModuleRequireStop: return "MODULE_REQUIRE_STOP";
    case kModuleTimeout: return "MODULE_TIMEOUT";
    case kModuleError: return "MODULE_ERROR";
    case kModuleVersionIncompatible: return "MODULE_VERSION_INCOMPATIBLE";
    case kModuleSystemError: return "MODULE_SYSTEM_ERROR";
    case kModuleReInit: return "MODULE_REINIT";
    case kModuleProcessExit: return "MODULE_PROCESS_EXIT";
    default: return "UNKNOWN_MODULE_RESP_TYPE";
  }
}

inline bool ModuleResponseSucceed(ModuleResponseType t) {
  return t == kModuleOK || t == kModuleRequireStop;
}

enum CoordinationMode : int16_t {
  kCoordinateUnknown = 0,
  kCoordinateSync = 1,
  kCoordinateAsync = 2,
  kCoordinatePriority = 3,
};

inline CoordinationMode Int2CoordMode(int16_t v) {
  switch (v) {
    case 1: return kCoordinateSync;
    case 2: return kCoordinateAsync;
    case 3: return kCoordinatePriority;
    default: return kCoordinateUnknown;
  }
}

enum Log2WorldTopicMode : int16_t {
  kLog2WorldTopicLogging = 1,
  kLog2WorldTopicSim = 2,
  kLog2WorldTopicBoth = 3,
  kLog2WorldTopicLog2Sim = 4,
};

inline Log2WorldTopicMode Int2TopicMode(int16_t v) {
  switch (v) {
    case 1: return kLog2WorldTopicLogging;
    case 2: return kLog2WorldTopicSim;
    case 3: return kLog2WorldTopicBoth;
    case 4: return kLog2WorldTopicLog2Sim;
    //default: throw std::invalid_argument("unknown log2world topic mode: " + v);
  }
  return (Log2WorldTopicMode)0;
}

enum Log2WorldSwitchType : int16_t {
  kLog2WorldSwitchScene = 1,
  kLog2WorldSwitchEvent = 2,
};

inline Log2WorldSwitchType Int2Log2WorldSwitchType(int16_t v) {
  switch (v) {
    case 1: return kLog2WorldSwitchScene;
    case 2: return kLog2WorldSwitchEvent;
    //default: throw std::invalid_argument("unknown log2world switch type: " + v);
  }
  return (Log2WorldSwitchType)0;
}

enum ModuleCategory : int16_t {
  kModuleCategoryUnknown = 0,
  kModuleCategoryTruthValue = 1,
  kModuleCategoryGrading = 51,
  kModuleCategoryPerception = 101,
  kModuleCategoryPrediction = 102,
  kModuleCategoryPlanning = 103,
  kModuleCategoryControlling = 104,
  kModuleCategoryVehicleDynamics = 105,
  kModuleCategoryTool = 1001,
};

inline ModuleCategory Int2ModuleCategory(int16_t v) {
  switch (v) {
    case 1: return kModuleCategoryTruthValue;
    case 51: return kModuleCategoryGrading;
    case 101: return kModuleCategoryPerception;
    case 102: return kModuleCategoryPrediction;
    case 103: return kModuleCategoryPlanning;
    case 104: return kModuleCategoryControlling;
    case 105: return kModuleCategoryVehicleDynamics;
    case 1001: return kModuleCategoryTool;
    default: return kModuleCategoryUnknown;
  }
  return (ModuleCategory)0;
}

static const uint32_t kDefaultCmdTimeoutMillis = 30000;
static const uint32_t kDefaultStepTimeoutMillis = 3000;
static const uint32_t kDefaultHeartbeatTimeoutMillis = 3500;
static const uint32_t kDefaultHeartbeatIntervalMillis = 1000;
static const uint16_t kDefaultControlRate = 100;
static const uint32_t kDefaultScenarioTimeLimit = 0;
static const uint32_t kModuleConnVersion = 2000000;
static const CoordinationMode kDefaultCoordMode = kCoordinateSync;
static const bool kDefaultPlayerAutoReset = true;
static const bool kDefaultPlayerAutoStop = true;
static const bool kDefaultAddingInitialLocationMsg = true;
static const bool kDefaultOverrideUserLog = true;
static const std::string kLogTopicPostfix = "._LOGPLAY_";
static const std::string kShadowTopicPostfix = "._SHADOW_";
static const std::string kPathUserDirSys = "sys";
static const std::string kPathUserDirData = "data";
static const std::string kServiceConfigFileName = "local_service.config";
static const std::string kServiceDatabaseName = "service.sqlite";
static const std::string kFileNameDefaultGradingKpi = "default_grading_kpi.json";
static const std::string kServiceConfigJsonKeyGradingPath = "gradingKpiDirectory";
static const std::string kGradingKpiFilePrefix = "grading_";
static const std::string kGlobalGradingKpiFileName = "grading.xml";
static const std::string kDefaultModuleRegistryPort = "21302";
static const std::string kChildModuleEnvUuid = "TXSIM_MODULE_UUID";
static const std::string kTopicScenarioStartTriggering = ".scenario_start";
static const std::string kTopicLog2WorldTriggering = ".log2world_trigger";
static const std::string kTopicPlayConfig = ".play_config";
static const bool kDefaultPerfLoggingEnabled = false;
static const std::string kInnerModuleForwardingEndpoint = "inproc://txsim-coordinator-cmd-forwarding";
static const std::string kInnerModuleImplCmdForwardingEndpoint = "inproc://txsim-module-impl-cmd-forwarding";
static const std::string kModuleInitArgKeySimMode = "_sim_mode";
static const std::string kDefaultModulePriorityLevelString = "50";

inline std::string GetSMCPVersion(uint32_t ver, uint16_t& major, uint16_t& minor, uint16_t& patch) {
  patch = ver % 1000;
  ver /= 1000;
  minor = ver % 1000;
  ver /= 1000;
  major = ver;
  std::ostringstream ss;
  ss << major << "." << minor << "." << patch;
  return ss.str();
}

inline std::string GetSMCPVersionStr(uint32_t ver) {
  uint16_t major, minor, patch;
  return GetSMCPVersion(ver, major, minor, patch);
}

}  // namespace impl
}  // namespace tx_sim
