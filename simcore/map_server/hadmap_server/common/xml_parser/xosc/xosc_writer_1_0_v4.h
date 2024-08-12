/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "common/xml_parser/entity/planner.h"
#include "common/xml_parser/entity/scene_event.h"
#include "common/xml_parser/entity/traffic.h"
#include "common/xml_parser/entity/velocity.h"
#include "common/xml_parser/xosc/xosc_base_1_0.h"
#include "openscenario.hpp"
class CTraffic;
class CSimulation;
class TiXmlElement;
class CVehicle;
class CObstacle;
class CPedestrianV2;
class CEnvironment;
class CRoute;
class CAcceleration;
class CMerge;
struct sPath;
struct sTagSimuTraffic;

class XOSCWriter_1_0_v4 : public XOSCBase_1_0 {
 public:
  XOSCWriter_1_0_v4();
  ~XOSCWriter_1_0_v4();

  int Save2XOSC(const char* strFileName, sTagSimuTraffic& scene);

 private:
  void SaveParameterDeclaration(osc::OpenScenario& osc, CSimulation& simulation, CTraffic& traffic);
  void SaveCatalogs(osc::OpenScenario& osc);
  void SaveRoadNetwork(osc::OpenScenario& osc, CSimulation& simulation, CTraffic& traffic);
  void SaveEntities(osc::OpenScenario& osc, CSimulation& simulation, CTraffic& traffic);
  void SaveStoryboard(osc::OpenScenario& osc, CSimulation& simulation, CTraffic& traffic, CEnvironment& environment);
  void FlushToDisk(osc::OpenScenario& osc, const char* strFileName);
  // ParameterDeclaration
  void SaveOneParameterDeclaration(osc::OpenScenario& osc, const char* strName, const char* strType,
                                   const char* strValue);
  // RoadNetwork
  void SaveLogicFile(osc::OpenScenario& osc, const char* str);
  void SceneGraphFile(osc::OpenScenario& osc, const char* str);
  void SceneTrafficSignals(osc::OpenScenario& osc, CTraffic& traffic);

  void SetIniInfos(osc::ScenarioObject& obj, std::string strLength, std::string strWidth, std::string strHeight,
                   std::string strX, std::string strY, std::string strZ, std::string strStartV, std::string strMaxV,
                   std::string strBehavior, std::string strAggress, std::string strFollow, std::string strEventId,
                   std::string strTrajectoryAngle, std::string strTrajectoryTracking, std::string& strName,
                   std::string& strType, bool isSetMaxV = false, std::string obuStatu = "",
                   std::string SesonrGroup = "");

  bool SaveOneEgoEntity(osc::OpenScenario& osc, const char* str, CEgoInfo& simulation);

  // init
  bool SaveStoryboardInitTrafficSignal(osc::OpenScenario& osc, CTraffic& traffic);
  bool SaveStoryboardInitEgo(osc::OpenScenario& osc, CSimulation& simulation);
  bool SaveStoryboardInitOneEgo(osc::OpenScenario& osc, CEgoInfo& egoinfo);
  bool SaveStoryboardInitVehicle(osc::OpenScenario& osc, CTraffic& traffic);
  bool SaveStoryboardInitObstacle(osc::OpenScenario& osc, CTraffic& traffic);
  bool SaveStoryboardInitPedestrian(osc::OpenScenario& osc, CTraffic& traffic);
  bool SaveEnvironmentInit(osc::OpenScenario& osc, CEnvironment& enviroment);
  osc::TeleportAction SaveTeleportAction(double dX, double dY, double dZ, double dH);
  osc::LongitudinalAction SaveLongitudinalAction(std::string strShape, std::string strDimension,
                                                 std::string strDynamicValue,
                                            std::string strSpeed);

  osc::ActivateControllerAction SaveActivateAction(std::string strSubType);
  osc::ControllerAction SaveControlAction(Scene_Action action);
  osc::CustomCommandAction SaveCustomAction(Scene_Action action);
  osc::RoutingAction SaveRoutingAction(std::string strName, std::string strClosed, sPath& path);

  int SaveEgoStory(osc::Act& act, CSimulation& simulation, CTraffic& traffic);

  int SaveVehiclesStory(osc::Act& act, CTraffic& traffic);
  int SavePedestriansStory(osc::Act& act, CTraffic& traffic);

  bool SaveStory(osc::OpenScenario& osc, CSimulation& siminfo, CTraffic& traffic);

  bool SaveStopTrigger(osc::OpenScenario& osc);

  int SaveOneVehicleActions(CTraffic& traffic, CVehicle& v, osc::ManeuverGroup& maneuver_group,
                            std::string strObjectName);
  int SaveOneVehicleActionsOldVersion(CTraffic& traffic, CVehicle& v, osc::ManeuverGroup& maneuver_group,
                                      std::string strObjectName, osc::Maneuver& maneuver, int TimeOrder, int Maneuvers);
  int SaveOneVehicleActionsNewVersion(CTraffic& traffic, CVehicle& v, osc::ManeuverGroup& maneuver_group,
                                      std::string strObjectName, osc::Maneuver& maneuver, int TimeOrder, int Maneuvers);

  int ConvertOneVehicleRouting(CVehicle& v, CRoute& r, osc::Maneuver& maneuver, std::string strObjectName, int& nEvents,
                               int& nTimeOrder, int& nOrder);
  // 先把所有的数据给穿起来  ----
  void JointData(CTraffic& traffic, CVehicle& v, std::vector<TimeTraggerData>& vecTT,
                 std::vector<ConditionTraggerData>& CT1, std::vector<ConditionTraggerData>& CT2,
                 std::vector<ConditionTraggerData>& CT3, std::vector<ConditionTraggerData>& CT4);

  void AddVehicle_V_A_M(std::vector<TimeTraggerData>::iterator ptr, std::string& actionBaseName,
                        std::string& strObjectName, osc::Event& event, int& num);
  void AddVehicle_V_A_M(std::vector<ConditionTraggerData>::iterator ptr, std::string& actionBaseName,
                        std::string& strObjectName, osc::Event& event, int& num);
  void AddVehicleAction_V_A_M(SceneEvent& singleSceneEvent, std::string& actionBaseName, osc::Event& event,
                              bool& ishaveaction);
  void AddVehicleTrigger_V_A_M(SceneEvent& singleSceneEvent, osc::Event& event, CTraffic::SceneEventMap& eventMap);

  int ConvertOnePedestrianRoutings(CPedestrianV2& p, CRoute& r, osc::Maneuver& maneuver, std::string strObjectName,
                                   int& nEvents, int& nTimeOrder, int& nOrder);
  int ConvertOnePedestrianVelocities(CPedestrianV2& p, CPedestrianVelocity& v, osc::Maneuver& maneuver,
                                     std::string strObjectName, int& nEvents, int& nTimeOrder, int& nOrder);
  osc::Center CreateCenter(std::string x, std::string y, std::string z);
  osc::Dimensions CreateDimensions(std::string height, std::string length, std::string width);
  osc::Property CreateProperty(std::string name, std::string value);

  osc::Vehicle CreateVehicle(osc::Axles axles, osc::BoundingBox boundingBox,
                             osc::Performance performance,
                             osc::Properties properties, std::string name, std::string vehicleCategory);

  osc::Performance CreatePerformance(std::string maxAcceleration, std::string maxDeceleration, std::string maxSpeed);

  osc::Sun CreateSun(std::string azimuth, std::string elevation, std::string intensity);

  osc::Event CreateEvent(osc::Trigger sub_StartTrigger, std::string maximumExecutionCount, std::string name);

  osc::Condition CreateCondition(std::string conditionEdge, std::string delay, std::string name);

  osc::MiscObject CreateMiscObject(osc::BoundingBox boundingBox, osc::Properties properties, std::string mass,
                                   std::string miscObjectCategory, std::string name);

  osc::Pedestrian CreatePedestrian(osc::BoundingBox boundingBox, osc::Properties properties, std::string mass,
                                   std::string model, std::string name, std::string pedestrianCategory);

  osc::ParameterDeclaration CreateParameterDeclaration(std::string name, std::string parameterType,
                                                       std::string value);

  int SaveOnePedestrianActions(CTraffic& traffic, CPedestrianV2& p, osc::ManeuverGroup& maneuver_group,
                               std::string strObjectName);

  // tool
  std::string formatRule(const std::string str);
  const std::string s_Entities_Vehicle_Name_Prefix = "V_";
  const std::string s_Vehicle_Name_Prefix = "Vehicle_";

  const std::string s_Entities_MiscObject_Name_Prefix = "O_";
  const std::string s_MiscObject_Name_Prefix = "MiscObject_";

  const std::string s_Entities_Pedestrian_Name_Prefix = "P_";
  const std::string s_Pedestrian_Name_Prefix = "Pedestrian_";

  int index;  // traffic文件里 行人集合中也有自行车，摩托车这种在   所以记个index标记
  int m_ManeuverGroupIndex;

  //
  std::string m_strCatalogDir;
};
