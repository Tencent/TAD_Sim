// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_traffic.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "tx_xml_utils.h"
#include "xml/intros_ptree.hpp"
#include "xml/util_traits.hpp"
using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Traffic)

BEGIN_INTROS_TYPE_USER_NAME(Data, "data")
ADD_INTROS_ITEM_USER_NAME(aggress, MAKE_USER_NAME("aggress", "", true))
ADD_INTROS_ITEM_USER_NAME(randomseed, MAKE_USER_NAME("randomseed", "", true))
ADD_INTROS_ITEM_USER_NAME(ruleCompliance, MAKE_USER_NAME("ruleCompliance", "", true))
END_INTROS_TYPE(Data)

BEGIN_INTROS_TYPE_USER_NAME(Route, "route")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(start, MAKE_USER_NAME("start", "", true))
ADD_INTROS_ITEM_USER_NAME(mid, MAKE_USER_NAME("mid", "", true))
ADD_INTROS_ITEM_USER_NAME(end, MAKE_USER_NAME("end", "", true))
ADD_INTROS_ITEM_USER_NAME(controlPath, MAKE_USER_NAME("controlPath", "", true))
ADD_INTROS_ITEM_USER_NAME(relativePath, MAKE_USER_NAME("relativePath", "", true))
ADD_INTROS_ITEM_USER_NAME(absolutePath, MAKE_USER_NAME("absolutePath", "", true))
ADD_INTROS_ITEM_USER_NAME(info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(Route)

BEGIN_INTROS_TYPE_USER_NAME(Acc, "acc")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(profile, MAKE_USER_NAME("profile", "", true))
ADD_INTROS_ITEM_USER_NAME(endCondition, MAKE_USER_NAME("endCondition", "", true))
END_INTROS_TYPE(Acc)

BEGIN_INTROS_TYPE_USER_NAME(Merge, "merge")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(profile, MAKE_USER_NAME("profile", "", true))
END_INTROS_TYPE(Merge)

BEGIN_INTROS_TYPE_USER_NAME(Velocity, "velocity")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(profile, MAKE_USER_NAME("profile", "", true))
END_INTROS_TYPE(Velocity)

BEGIN_INTROS_TYPE_USER_NAME(Time_velocity, "time_velocity")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(profile, MAKE_USER_NAME("profile", "", true))
END_INTROS_TYPE(Time_velocity)

BEGIN_INTROS_TYPE_USER_NAME(Event_velocity, "event_velocity")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(profile, MAKE_USER_NAME("profile", "", true))
END_INTROS_TYPE(Event_velocity)

BEGIN_INTROS_TYPE_USER_NAME(event, "event")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))

#if __SecenEventVersion_1_2_0_0__
ADD_INTROS_ITEM_USER_NAME(conditionNumber, MAKE_USER_NAME("conditionNumber", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[0][_type_index_], MAKE_USER_NAME("type1", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[0][_cond_index_], MAKE_USER_NAME("condition1", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[1][_type_index_], MAKE_USER_NAME("type2", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[1][_cond_index_], MAKE_USER_NAME("condition2", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[2][_type_index_], MAKE_USER_NAME("type3", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[2][_cond_index_], MAKE_USER_NAME("condition3", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[3][_type_index_], MAKE_USER_NAME("type4", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[3][_cond_index_], MAKE_USER_NAME("condition4", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[4][_type_index_], MAKE_USER_NAME("type5", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[4][_cond_index_], MAKE_USER_NAME("condition5", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[5][_type_index_], MAKE_USER_NAME("type6", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[5][_cond_index_], MAKE_USER_NAME("condition6", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[6][_type_index_], MAKE_USER_NAME("type7", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[6][_cond_index_], MAKE_USER_NAME("condition7", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[7][_type_index_], MAKE_USER_NAME("type8", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[7][_cond_index_], MAKE_USER_NAME("condition8", "", true))

ADD_INTROS_ITEM_USER_NAME(type_condition_array[8][_type_index_], MAKE_USER_NAME("type9", "", true))
ADD_INTROS_ITEM_USER_NAME(type_condition_array[8][_cond_index_], MAKE_USER_NAME("condition9", "", true))
#endif /*__SecenEventVersion_1_2_0_0__*/

ADD_INTROS_ITEM_USER_NAME(condition, MAKE_USER_NAME("condition", "", true))
ADD_INTROS_ITEM_USER_NAME(endCondition, MAKE_USER_NAME("endCondition", "", true))
ADD_INTROS_ITEM_USER_NAME(action, MAKE_USER_NAME("action", "", true))
ADD_INTROS_ITEM_USER_NAME(info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(event)

BEGIN_INTROS_TYPE_USER_NAME(scene_event, "scene_event")
ADD_INTROS_ITEM_USER_NAME(version, MAKE_USER_NAME("version", "", true))
ADD_INTROS_ITEM_USER_NAME(event_array, MAKE_USER_NAME("event", "", false))
END_INTROS_TYPE(scene_event)

BEGIN_INTROS_TYPE_USER_NAME(Vehicle, "vehicle")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(routeID, MAKE_USER_NAME("routeID", "", true))
ADD_INTROS_ITEM_USER_NAME(accID, MAKE_USER_NAME("accID", "", true))
ADD_INTROS_ITEM_USER_NAME(mergeID, MAKE_USER_NAME("mergeID", "", true))
ADD_INTROS_ITEM_USER_NAME(laneID, MAKE_USER_NAME("laneID", "", true))
ADD_INTROS_ITEM_USER_NAME(start_s, MAKE_USER_NAME("start_s", "", true))
ADD_INTROS_ITEM_USER_NAME(start_t, MAKE_USER_NAME("start_t", "", true))
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", true))
ADD_INTROS_ITEM_USER_NAME(max_v, MAKE_USER_NAME("max_v", "", true))
ADD_INTROS_ITEM_USER_NAME(l_offset, MAKE_USER_NAME("l_offset", "", true))
ADD_INTROS_ITEM_USER_NAME(length, MAKE_USER_NAME("length", "", true))
ADD_INTROS_ITEM_USER_NAME(width, MAKE_USER_NAME("width", "", true))
ADD_INTROS_ITEM_USER_NAME(height, MAKE_USER_NAME("height", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicleType, MAKE_USER_NAME("vehicleType", "", true))
ADD_INTROS_ITEM_USER_NAME(aggress, MAKE_USER_NAME("aggress", "", true))
ADD_INTROS_ITEM_USER_NAME(behavior, MAKE_USER_NAME("behavior", "", true))
ADD_INTROS_ITEM_USER_NAME(follow, MAKE_USER_NAME("follow", "", true))
ADD_INTROS_ITEM_USER_NAME(mergeTime, MAKE_USER_NAME("mergeTime", "", true))
ADD_INTROS_ITEM_USER_NAME(offsetTime, MAKE_USER_NAME("offsetTime", "", true))
ADD_INTROS_ITEM_USER_NAME(eventId, MAKE_USER_NAME("eventId", "", true))
ADD_INTROS_ITEM_USER_NAME(angle, MAKE_USER_NAME("angle", "", true))
ADD_INTROS_ITEM_USER_NAME(catalog, MAKE_USER_NAME("catalog", "", true))
ADD_INTROS_ITEM_USER_NAME(start_angle, MAKE_USER_NAME("start_angle", "", true))
END_INTROS_TYPE(Vehicle)

BEGIN_INTROS_TYPE_USER_NAME(Obstacle, "obstacle")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(routeID, MAKE_USER_NAME("routeID", "", true))
ADD_INTROS_ITEM_USER_NAME(laneID, MAKE_USER_NAME("laneID", "", true))
ADD_INTROS_ITEM_USER_NAME(start_s, MAKE_USER_NAME("start_s", "", true))
ADD_INTROS_ITEM_USER_NAME(l_offset, MAKE_USER_NAME("l_offset", "", true))
ADD_INTROS_ITEM_USER_NAME(length, MAKE_USER_NAME("length", "", true))
ADD_INTROS_ITEM_USER_NAME(width, MAKE_USER_NAME("width", "", true))
ADD_INTROS_ITEM_USER_NAME(height, MAKE_USER_NAME("height", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicleType, MAKE_USER_NAME("vehicleType", "", true))
ADD_INTROS_ITEM_USER_NAME(direction, MAKE_USER_NAME("direction", "", true))
ADD_INTROS_ITEM_USER_NAME(behavior, MAKE_USER_NAME("behavior", "", true))
ADD_INTROS_ITEM_USER_NAME(eventId, MAKE_USER_NAME("eventId", "", true))
ADD_INTROS_ITEM_USER_NAME(angle, MAKE_USER_NAME("angle", "", true))
ADD_INTROS_ITEM_USER_NAME(catalog, MAKE_USER_NAME("catalog", "", true))
ADD_INTROS_ITEM_USER_NAME(start_angle, MAKE_USER_NAME("start_angle", "", true))
END_INTROS_TYPE(Obstacle)

BEGIN_INTROS_TYPE_USER_NAME(Pedestrian, "pedestrian")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(routeID, MAKE_USER_NAME("routeID", "", true))
ADD_INTROS_ITEM_USER_NAME(laneID, MAKE_USER_NAME("laneID", "", true))
ADD_INTROS_ITEM_USER_NAME(start_s, MAKE_USER_NAME("start_s", "", true))
ADD_INTROS_ITEM_USER_NAME(start_t, MAKE_USER_NAME("start_t", "", true))
ADD_INTROS_ITEM_USER_NAME(end_t, MAKE_USER_NAME("end_t", "", true))
ADD_INTROS_ITEM_USER_NAME(type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(l_offset, MAKE_USER_NAME("l_offset", "", true))
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", true))
ADD_INTROS_ITEM_USER_NAME(max_v, MAKE_USER_NAME("max_v", "", true))
ADD_INTROS_ITEM_USER_NAME(behavior, MAKE_USER_NAME("behavior", "", true))
ADD_INTROS_ITEM_USER_NAME(eventId, MAKE_USER_NAME("eventId", "", true))
ADD_INTROS_ITEM_USER_NAME(angle, MAKE_USER_NAME("angle", "", true))
ADD_INTROS_ITEM_USER_NAME(catalog, MAKE_USER_NAME("catalog", "", true))
ADD_INTROS_ITEM_USER_NAME(start_angle, MAKE_USER_NAME("start_angle", "", true))
END_INTROS_TYPE(Pedestrian)

BEGIN_INTROS_TYPE_USER_NAME(Signlight, "signlight")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(routeID, MAKE_USER_NAME("routeID", "", true))
ADD_INTROS_ITEM_USER_NAME(start_s, MAKE_USER_NAME("start_s", "", true))
ADD_INTROS_ITEM_USER_NAME(start_t, MAKE_USER_NAME("start_t", "", true))
ADD_INTROS_ITEM_USER_NAME(l_offset, MAKE_USER_NAME("l_offset", "", true))
ADD_INTROS_ITEM_USER_NAME(time_green, MAKE_USER_NAME("time_green", "", true))
ADD_INTROS_ITEM_USER_NAME(time_yellow, MAKE_USER_NAME("time_yellow", "", true))
ADD_INTROS_ITEM_USER_NAME(time_red, MAKE_USER_NAME("time_red", "", true))
ADD_INTROS_ITEM_USER_NAME(direction, MAKE_USER_NAME("direction", "", true))
ADD_INTROS_ITEM_USER_NAME(lane, MAKE_USER_NAME("lane", "", true))
ADD_INTROS_ITEM_USER_NAME(phase, MAKE_USER_NAME("phase", "", true))
ADD_INTROS_ITEM_USER_NAME(status, MAKE_USER_NAME("status", "", true))
ADD_INTROS_ITEM_USER_NAME(compliance, MAKE_USER_NAME("compliance", "", true))
ADD_INTROS_ITEM_USER_NAME(plan, MAKE_USER_NAME("plan", "", true))
ADD_INTROS_ITEM_USER_NAME(junction, MAKE_USER_NAME("junction", "", true))
ADD_INTROS_ITEM_USER_NAME(phaseNumber, MAKE_USER_NAME("phaseNumber", "", true))
ADD_INTROS_ITEM_USER_NAME(signalHead, MAKE_USER_NAME("signalHead", "", true))
ADD_INTROS_ITEM_USER_NAME(eventId, MAKE_USER_NAME("eventId", "", true))

END_INTROS_TYPE(Signlight)

#if __TX_Mark__("traffic.trafficflow")

BEGIN_INTROS_TYPE_USER_NAME(Location, "location")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Pos, MAKE_USER_NAME("pos", "", true))
ADD_INTROS_ITEM_USER_NAME(info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(Location)

/*<vehType id="101" type="Sedan" length="4.5" width="1.8" height="1.5" behavior="TrafficVehicle" />*/
BEGIN_INTROS_TYPE_USER_NAME(VehType, "vehType")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(length, MAKE_USER_NAME("length", "", true))
ADD_INTROS_ITEM_USER_NAME(width, MAKE_USER_NAME("width", "", true))
ADD_INTROS_ITEM_USER_NAME(height, MAKE_USER_NAME("height", "", true))
ADD_INTROS_ITEM_USER_NAME(behavior, MAKE_USER_NAME("behavior", "", true))
END_INTROS_TYPE(VehType)

/*<vehComp id="1001" type1="101" percentage1="80" behavior1="1" aggress1="0.5"
                     type2="102" percentage2="15" behavior2="1" aggress2="0.6"
                     type3="103" percentage3="5" behavior3="1" aggress3="0.1" info="Platoon1" />*/
BEGIN_INTROS_TYPE_USER_NAME(VehComp, "vehComp")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[0].Type, MAKE_USER_NAME("type1", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[0].Percentage, MAKE_USER_NAME("percentage1", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[0].Behavior, MAKE_USER_NAME("behavior1", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[0].Aggress, MAKE_USER_NAME("aggress1", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[1].Type, MAKE_USER_NAME("type2", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[1].Percentage, MAKE_USER_NAME("percentage2", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[1].Behavior, MAKE_USER_NAME("behavior2", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[1].Aggress, MAKE_USER_NAME("aggress2", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[2].Type, MAKE_USER_NAME("type3", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[2].Percentage, MAKE_USER_NAME("percentage3", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[2].Behavior, MAKE_USER_NAME("behavior3", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[2].Aggress, MAKE_USER_NAME("aggress3", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[3].Type, MAKE_USER_NAME("type4", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[3].Percentage, MAKE_USER_NAME("percentage4", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[3].Behavior, MAKE_USER_NAME("behavior4", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[3].Aggress, MAKE_USER_NAME("aggress4", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[4].Type, MAKE_USER_NAME("type5", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[4].Percentage, MAKE_USER_NAME("percentage5", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[4].Behavior, MAKE_USER_NAME("behavior5", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[4].Aggress, MAKE_USER_NAME("aggress5", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[5].Type, MAKE_USER_NAME("type6", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[5].Percentage, MAKE_USER_NAME("percentage6", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[5].Behavior, MAKE_USER_NAME("behavior6", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[5].Aggress, MAKE_USER_NAME("aggress6", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[6].Type, MAKE_USER_NAME("type7", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[6].Percentage, MAKE_USER_NAME("percentage7", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[6].Behavior, MAKE_USER_NAME("behavior7", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[6].Aggress, MAKE_USER_NAME("aggress7", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[7].Type, MAKE_USER_NAME("type8", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[7].Percentage, MAKE_USER_NAME("percentage8", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[7].Behavior, MAKE_USER_NAME("behavior8", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[7].Aggress, MAKE_USER_NAME("aggress8", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[8].Type, MAKE_USER_NAME("type9", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[8].Percentage, MAKE_USER_NAME("percentage9", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[8].Behavior, MAKE_USER_NAME("behavior9", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[8].Aggress, MAKE_USER_NAME("aggress9", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[9].Type, MAKE_USER_NAME("type10", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[9].Percentage, MAKE_USER_NAME("percentage10", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[9].Behavior, MAKE_USER_NAME("behavior10", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[9].Aggress, MAKE_USER_NAME("aggress10", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[10].Type, MAKE_USER_NAME("type11", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[10].Percentage, MAKE_USER_NAME("percentage11", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[10].Behavior, MAKE_USER_NAME("behavior11", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[10].Aggress, MAKE_USER_NAME("aggress11", "", true))

ADD_INTROS_ITEM_USER_NAME(CompNodeArray[11].Type, MAKE_USER_NAME("type12", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[11].Percentage, MAKE_USER_NAME("percentage12", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[11].Behavior, MAKE_USER_NAME("behavior12", "", true))
ADD_INTROS_ITEM_USER_NAME(CompNodeArray[11].Aggress, MAKE_USER_NAME("aggress12", "", true))

ADD_INTROS_ITEM_USER_NAME(Info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(VehComp)

/*<vehInput id="10001" location="100001" composition="1001" start_v="14" max_v="21"
     distribution="Fixed" timeHeadway="1" duration="10000" cover="100" info="Platoon1"/>
 */
BEGIN_INTROS_TYPE_USER_NAME(VehInput, "vehInput")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
ADD_INTROS_ITEM_USER_NAME(Composition, MAKE_USER_NAME("composition", "", true))
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", true))
ADD_INTROS_ITEM_USER_NAME(max_v, MAKE_USER_NAME("max_v", "", true))
ADD_INTROS_ITEM_USER_NAME(halfRange_v, MAKE_USER_NAME("halfRange_v", "", true))
ADD_INTROS_ITEM_USER_NAME(Distribution, MAKE_USER_NAME("distribution", "", true))
ADD_INTROS_ITEM_USER_NAME(TimeHeadway, MAKE_USER_NAME("timeHeadway", "", true))
ADD_INTROS_ITEM_USER_NAME(Duration, MAKE_USER_NAME("duration", "", true))
ADD_INTROS_ITEM_USER_NAME(Cover, MAKE_USER_NAME("cover", "", true))
ADD_INTROS_ITEM_USER_NAME(Info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(VehInput)
/*<behavior id="1" type="Freeway" cc0="1.5" cc1="1.3" cc2="4"/>*/
/*<behavior id="2" type="Arterial" AX="2" BX_Add="2" BX_Mult="3"/>*/
BEGIN_INTROS_TYPE_USER_NAME(Beh, "behavior")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Type, MAKE_USER_NAME("type", "", true))
ADD_INTROS_ITEM_USER_NAME(cc0, MAKE_USER_NAME("cc0", "", true))
ADD_INTROS_ITEM_USER_NAME(cc1, MAKE_USER_NAME("cc1", "", true))
ADD_INTROS_ITEM_USER_NAME(cc2, MAKE_USER_NAME("cc2", "", true))
ADD_INTROS_ITEM_USER_NAME(AX, MAKE_USER_NAME("AX", "", true))
ADD_INTROS_ITEM_USER_NAME(BX_Add, MAKE_USER_NAME("BX_Add", "", true))
ADD_INTROS_ITEM_USER_NAME(BX_Mult, MAKE_USER_NAME("BX_Mult", "", true))
ADD_INTROS_ITEM_USER_NAME(LCduration, MAKE_USER_NAME("LCduration", "", true))
END_INTROS_TYPE(Beh)

/*<vehExit id="20001" location="200001" cover="0" info="Exit1"/>*/
BEGIN_INTROS_TYPE_USER_NAME(VehExit, "vehExit")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
ADD_INTROS_ITEM_USER_NAME(Cover, MAKE_USER_NAME("cover", "", true))
ADD_INTROS_ITEM_USER_NAME(Info, MAKE_USER_NAME("info", "", true))
END_INTROS_TYPE(VehExit)
/*<routeGroup id="8001" start="800010" mid1="0" end1="800011" percentage1="20" mid2="0" end2="800012" percentage2="50"
 * mid3="0" end3="800013" percentage3="30" />*/
BEGIN_INTROS_TYPE_USER_NAME(RouteGroup, "routeGroup")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Start, MAKE_USER_NAME("start", "", true))

ADD_INTROS_ITEM_USER_NAME(Mid1, MAKE_USER_NAME("mid1", "", true))
ADD_INTROS_ITEM_USER_NAME(End1, MAKE_USER_NAME("end1", "", true))
ADD_INTROS_ITEM_USER_NAME(Percentage1, MAKE_USER_NAME("percentage1", "", true))

ADD_INTROS_ITEM_USER_NAME(Mid2, MAKE_USER_NAME("mid2", "", true))
ADD_INTROS_ITEM_USER_NAME(End2, MAKE_USER_NAME("end2", "", true))
ADD_INTROS_ITEM_USER_NAME(Percentage2, MAKE_USER_NAME("percentage2", "", true))

ADD_INTROS_ITEM_USER_NAME(Mid3, MAKE_USER_NAME("mid3", "", true))
ADD_INTROS_ITEM_USER_NAME(End3, MAKE_USER_NAME("end3", "", true))
ADD_INTROS_ITEM_USER_NAME(Percentage3, MAKE_USER_NAME("percentage3", "", true))
END_INTROS_TYPE(RouteGroup)

BEGIN_INTROS_TYPE_USER_NAME(Compliance, "compliance")
ADD_INTROS_ITEM_USER_NAME(proportion, MAKE_USER_NAME("proportion", "", true))
END_INTROS_TYPE(Compliance)

BEGIN_INTROS_TYPE_USER_NAME(TrafficFlow, "trafficFlow")
ADD_INTROS_ITEM_USER_NAME(VehicleType, MAKE_USER_NAME("vehType", "vehicleType", false))
ADD_INTROS_ITEM_USER_NAME(VehicleComposition, MAKE_USER_NAME("vehComp", "vehicleComposition", false))
ADD_INTROS_ITEM_USER_NAME(VehicleInput, MAKE_USER_NAME("vehInput", "vehicleInput", false))
ADD_INTROS_ITEM_USER_NAME(Behavior, MAKE_USER_NAME("behavior", "behaviors", false))
ADD_INTROS_ITEM_USER_NAME(VehicleExit, MAKE_USER_NAME("vehExit", "vehicleExit", false))
ADD_INTROS_ITEM_USER_NAME(RouteGroups, MAKE_USER_NAME("routeGroup", "routeGroups", false))
ADD_INTROS_ITEM_USER_NAME(RuleCompliance, MAKE_USER_NAME("compliance", "ruleCompliance", false))
END_INTROS_TYPE(TrafficFlow)
#endif /*__TX_Mark__("traffic.trafficflow")*/

#if __TX_Mark__("ego input")
/*<start id="1" location="300001" start_v="12" max_v="15" theta="1.9854026024876614"/>*/
BEGIN_INTROS_TYPE_USER_NAME(Start, "start")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
ADD_INTROS_ITEM_USER_NAME(start_v, MAKE_USER_NAME("start_v", "", true))
ADD_INTROS_ITEM_USER_NAME(max_v, MAKE_USER_NAME("max_v", "", true))
ADD_INTROS_ITEM_USER_NAME(theta, MAKE_USER_NAME("theta", "", true))
END_INTROS_TYPE(Start)

/*<end id="1" location="400001"/>*/
BEGIN_INTROS_TYPE_USER_NAME(End, "end")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
END_INTROS_TYPE(End)

/*<input id="1" mode="FixedRoute" route="1,1" number="100" timeHeadway="5"/>*/
BEGIN_INTROS_TYPE_USER_NAME(Input, "input")
ADD_INTROS_ITEM_USER_NAME(id, MAKE_USER_NAME("id", "", true))
ADD_INTROS_ITEM_USER_NAME(mode, MAKE_USER_NAME("mode", "", true))
ADD_INTROS_ITEM_USER_NAME(Route, MAKE_USER_NAME("route", "", true))
ADD_INTROS_ITEM_USER_NAME(number, MAKE_USER_NAME("number", "", true))
ADD_INTROS_ITEM_USER_NAME(TimeHeadway, MAKE_USER_NAME("timeHeadway", "", true))
END_INTROS_TYPE(Input)

BEGIN_INTROS_TYPE(Ego)
ADD_INTROS_ITEM_USER_NAME(EgoStart, MAKE_USER_NAME("start", "egoStart", false))
ADD_INTROS_ITEM_USER_NAME(EgoEnd, MAKE_USER_NAME("end", "egoEnd", false))
ADD_INTROS_ITEM_USER_NAME(EgoInput, MAKE_USER_NAME("input", "egoInput", false))
END_INTROS_TYPE(Ego)

BEGIN_INTROS_TYPE(LowerLeft)
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
END_INTROS_TYPE(LowerLeft)

BEGIN_INTROS_TYPE(UpperRight)
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
END_INTROS_TYPE(UpperRight)

BEGIN_INTROS_TYPE(Center)
ADD_INTROS_ITEM_USER_NAME(Location, MAKE_USER_NAME("location", "", true))
END_INTROS_TYPE(Center)

BEGIN_INTROS_TYPE(Map)
ADD_INTROS_ITEM_USER_NAME(ll, "lowerLeft")
ADD_INTROS_ITEM_USER_NAME(ur, "upperRight")
ADD_INTROS_ITEM_USER_NAME(c, "center")
END_INTROS_TYPE(Map)
#endif /*__TX_Mark__("ego input")*/

BEGIN_INTROS_TYPE_USER_NAME(traffic, "traffic")
ADD_INTROS_ITEM_USER_NAME(data, MAKE_USER_NAME("data", "", false))
ADD_INTROS_ITEM_USER_NAME(Locations, MAKE_USER_NAME("location", "locations", false))
ADD_INTROS_ITEM_USER_NAME(routes, MAKE_USER_NAME("route", "routes", false))
ADD_INTROS_ITEM_USER_NAME(accs, MAKE_USER_NAME("acc", "accs", false))
ADD_INTROS_ITEM_USER_NAME(accs_event, MAKE_USER_NAME("acc", "accs_event", false))
ADD_INTROS_ITEM_USER_NAME(merges, MAKE_USER_NAME("merge", "merges", false))
ADD_INTROS_ITEM_USER_NAME(merges_event, MAKE_USER_NAME("merge", "merges_event", false))
ADD_INTROS_ITEM_USER_NAME(velocities, MAKE_USER_NAME("velocity", "velocities", false))
ADD_INTROS_ITEM_USER_NAME(velocities_event, MAKE_USER_NAME("velocity", "velocities_event", false))
ADD_INTROS_ITEM_USER_NAME(pedestrians_event_time_velocity, MAKE_USER_NAME("time_velocity", "pedestrians_event", false))
ADD_INTROS_ITEM_USER_NAME(pedestrians_event_event_velocity,
                          MAKE_USER_NAME("event_velocity", "pedestrians_event", false))
ADD_INTROS_ITEM_USER_NAME(scene_event_array, MAKE_USER_NAME("scene_event", "", false))
ADD_INTROS_ITEM_USER_NAME(vehicles, MAKE_USER_NAME("vehicle", "vehicles", false))
ADD_INTROS_ITEM_USER_NAME(obstacles, MAKE_USER_NAME("obstacle", "obstacles", false))
ADD_INTROS_ITEM_USER_NAME(pedestrians, MAKE_USER_NAME("pedestrian", "pedestrians", false))
ADD_INTROS_ITEM_USER_NAME(signlights_activePlan, MAKE_USER_NAME("activePlan", "signlights", true))
ADD_INTROS_ITEM_USER_NAME(signlights, MAKE_USER_NAME("signlight", "signlights", false))
ADD_INTROS_ITEM_USER_NAME(trafficFlow, "trafficFlow")
ADD_INTROS_ITEM_USER_NAME(ego, "ego")
ADD_INTROS_ITEM_USER_NAME(map, "map")
ADD_INTROS_ITEM_USER_NAME(rand_pedestrian, "rand_pedestrian")
END_INTROS_TYPE(traffic)

void debug_tip() {
  traffic c;
  c.routes.resize(3);
  c.accs.resize(3);
  c.accs_event.resize(3);

  c.merges.resize(3);
  c.merges_event.resize(3);
  c.velocities_event.resize(3);

  c.pedestrians_event_time_velocity.resize(3);
  c.pedestrians_event_event_velocity.resize(3);
  c.vehicles.resize(3);

  c.obstacles.resize(3);
  c.pedestrians.resize(3);
  c.signlights.resize(3);

  c.trafficFlow.VehicleType.resize(3);
  c.trafficFlow.VehicleComposition.resize(3);
  c.trafficFlow.VehicleInput.resize(3);
  c.trafficFlow.Behavior.resize(3);
  c.trafficFlow.VehicleExit.resize(3);
  c.trafficFlow.RouteGroups.resize(3);
  c.ego.EgoStart.resize(3);
  c.ego.EgoEnd.resize(3);
  c.ego.EgoInput.resize(3);

  ptree tree = make_ptree(c);
  xml_writer_settings<Base::txString> settings(' ', 4);  // this is needed for xml printing to have proper whitespace
  write_xml(std::cout, tree, settings);
}

traffic_ptr load_scene_traffic(const Base::txString& _traffic_path) TX_NOEXCEPT {
  std::ifstream infile(_traffic_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    // xml_writer_settings<Base::txString> settings(' ', 4); // this is needed for xml printing to have proper
    // whitespace write_xml(std::cout, tree, settings);
    return std::make_shared<traffic>(make_intros_object<traffic>(tree));
  } else {
    LOG(WARNING) << "Load Xml File Failure. " << _traffic_path;
    return nullptr;
  }
}

TX_NAMESPACE_CLOSE(Traffic)
TX_NAMESPACE_CLOSE(SceneLoader)

#undef _type_index_
#undef _cond_index_
