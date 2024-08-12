// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <array>
#include <iostream>
#include <sstream>
#include "tx_header.h"

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Traffic)
#if __TX_Mark__("traffic.data")
/*<data aggress="0.240000" randomseed="49"/>*/
struct Data {
  /*
  <xs:element name="data" maxOccurs="unbounded" minOccurs="0">
      <xs:complexType>
      <xs:simpleContent>
          <xs:extension base="xs:string">
          <xs:attribute type="xs:decimal" name="aggress" use="optional"/>
          <xs:attribute type="xs:int" name="randomseed" use="optional"/>
          </xs:extension>
      </xs:simpleContent>
      </xs:complexType>
  </xs:element>
  */
  Base::txFloat aggress = 0.5;
  Base::txInt randomseed = -1;
  Base::txFloat ruleCompliance = 1.0;
  friend std::ostream& operator<<(std::ostream& os, const Data& v) TX_NOEXCEPT {
    os << "data : " << TX_VARS_NAME(aggress, v.aggress) << TX_VARS_NAME(randomseed, v.randomseed)
       << TX_VARS_NAME(ruleCompliance, v.ruleCompliance);
    return os;
  }
};

#endif /*__TX_Mark__("traffic.data")*/
#if __TX_Mark__("traffic.route")
struct Route {
  Base::txSysId id;
  Base::txString type;
  Base::txString start;
  Base::txString mid;
  Base::txString end;
  Base::txString info;
  Base::txString controlPath;
  Base::txString relativePath;
  Base::txString absolutePath;
  friend std::ostream& operator<<(std::ostream& os, const Route& v) TX_NOEXCEPT {
    os << "route : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(type, v.type) << TX_VARS_NAME(start, v.start)
       << TX_VARS_NAME(mid, v.mid) << TX_VARS_NAME(end, v.end) << TX_VARS_NAME(controlPath, v.controlPath)
       << TX_VARS_NAME(relativePath, v.relativePath) << TX_VARS_NAME(absolutePath, v.absolutePath)
       << TX_VARS_NAME(info, v.info);
    return os;
  }
};

#endif /*__TX_Mark__("traffic.route")*/

#if __TX_Mark__("traffic.acc")
/*<acc id="0" profile="0.000000,0.000000" endCondition="None,0.0;Time,2.3;Velocity,8.0;None,0.0"/>*/
struct Acc {
  /*
  <xs:element name="acc" maxOccurs="unbounded" minOccurs="0">
  <xs:complexType>
      <xs:simpleContent>
      <xs:extension base="xs:string">
          <xs:attribute type="xs:int" name="id" use="required"/>
          <xs:attribute type="xs:string" name="profile" use="required"/>
          <xs:attribute type="xs:string" name="endCondition" use="optional"/>
      </xs:extension>
      </xs:simpleContent>
  </xs:complexType>
  </xs:element>
  */
  Base::txSysId id;
  Base::txString profile;
  Base::txString endCondition;
  friend std::ostream& operator<<(std::ostream& os, const Acc& v) TX_NOEXCEPT {
    os << "acc : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(profile, v.profile)
       << TX_VARS_NAME(endCondition, v.endCondition);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.acc")*/

#if __TX_Mark__("traffic.merge")

struct Merge {
  /*
  <xs:element name="merge" maxOccurs="unbounded" minOccurs="0">
  <xs:complexType>
      <xs:simpleContent>
      <xs:extension base="xs:string">
          <xs:attribute type="xs:int" name="id" use="required"/>
          <xs:attribute type="xs:string" name="profile" use="required"/>
      </xs:extension>
      </xs:simpleContent>
  </xs:complexType>
  </xs:element>
  */
  Base::txSysId id;
  Base::txString profile;
  friend std::ostream& operator<<(std::ostream& os, const Merge& v) TX_NOEXCEPT {
    os << "merge : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(profile, v.profile);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.merge")*/

#if __TX_Mark__("traffic.velocity")
struct Velocity {
  /*
  <xs:element name="velocity" maxOccurs="unbounded" minOccurs="0">
  <xs:complexType>
      <xs:simpleContent>
      <xs:extension base="xs:string">
          <xs:attribute type="xs:int" name="id" use="required"/>
          <xs:attribute type="xs:string" name="profile" use="required"/>
      </xs:extension>
      </xs:simpleContent>
  </xs:complexType>
  </xs:element>
  */
  Base::txSysId id;
  Base::txString profile;
  friend std::ostream& operator<<(std::ostream& os, const Velocity& v) TX_NOEXCEPT {
    os << "velocity : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(profile, v.profile);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.velocity")*/

#if __TX_Mark__("traffic.time_velocity")
struct Time_velocity {
  Base::txSysId id;
  Base::txString profile;
  friend std::ostream& operator<<(std::ostream& os, const Time_velocity& v) TX_NOEXCEPT {
    os << "time_velocity : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(profile, v.profile);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.time_velocity")*/

#if __TX_Mark__("traffic.event_velocity")
struct Event_velocity {
  Base::txSysId id;
  Base::txString profile;
  friend std::ostream& operator<<(std::ostream& os, const Event_velocity& v) TX_NOEXCEPT {
    os << "event_velocity : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(profile, v.profile);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.event_velocity")*/

#if __TX_Mark__("traffic.scene_event.event")

#  define _type_index_ (0)
#  define _cond_index_ (1)

struct event {
  Base::txString id;
  Base::txString type;
  Base::txString condition;
  Base::txString endCondition;
  Base::txString action;
  Base::txString info;
#  if __SecenEventVersion_1_2_0_0__
  Base::txBool IsConditionGroup() const TX_NOEXCEPT { return _NonEmpty_(conditionNumber); }
  Base::txString conditionNumber;
  std::array<std::array<Base::txString, 2>, 9> type_condition_array;
  Base::txString groupType(const Base::txInt idx) const TX_NOEXCEPT {
    return type_condition_array.at(idx).at(_type_index_);
  }
  Base::txString groupCondition(const Base::txInt idx) const TX_NOEXCEPT {
    return type_condition_array.at(idx).at(_cond_index_);
  }
#  endif /*__SecenEventVersion_1_2_0_0__*/
  friend std::ostream& operator<<(std::ostream& os, const event& v) TX_NOEXCEPT {
#  if __SecenEventVersion_1_2_0_0__
    std::ostringstream oss;
    oss << TX_VARS_NAME(conditionNumber, v.conditionNumber);
    Base::txInt idx = 1;
    for (const auto& ref_type_cond : v.type_condition_array) {
      oss << "{" << TX_VARS_NAME(idx, idx) << TX_VARS_NAME(type, ref_type_cond[_type_index_])
          << TX_VARS_NAME(cond, ref_type_cond[_cond_index_]) << "},";
    }
#  endif /*__SecenEventVersion_1_2_0_0__*/
    os << "< event : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(type, v.type) << TX_VARS_NAME(condition, v.condition)
#  if __SecenEventVersion_1_2_0_0__
       << TX_VARS_NAME(conditiongroup, oss.str())
#  endif /*__SecenEventVersion_1_2_0_0__*/
       << TX_VARS_NAME(endCondition, v.endCondition) << TX_VARS_NAME(action, v.action) << TX_VARS_NAME(info, v.info)
       << " />";
    return os;
  }
};

#endif /*__TX_Mark__("traffic.scene_event.event")*/
#if __TX_Mark__("traffic.scene_event")

struct scene_event {
  Base::txString version;
  std::vector<event> event_array;
  friend std::ostream& operator<<(std::ostream& os, const scene_event& v) TX_NOEXCEPT {
    os << "< scene_event : " << TX_VARS_NAME(version, v.version) << " />" << std::endl;
    for (const auto& ev : v.event_array) {
      os << ev << std::endl;
    }
    os << "< scene_event/>";
    return os;
  }
};

#endif /*__TX_Mark__("traffic.scene_event")*/

#if __TX_Mark__("traffic.vehicle")
struct Vehicle {
  /*
  <xs:element name="vehicle" maxOccurs="unbounded" minOccurs="0">
  <xs:complexType>
      <xs:simpleContent>
      <xs:extension base="xs:string">
          <xs:attribute type="xs:int" name="id" use="required"/>
          <xs:attribute type="xs:int" name="routeID" use="required"/>
          <xs:attribute type="xs:long" name="laneID" use="optional"/>
          <xs:attribute type="xs:decimal" name="start_s" use="optional"/>
          <xs:attribute type="xs:decimal" name="start_t" use="optional"/>
          <xs:attribute type="xs:decimal" name="start_v" use="optional"/>
          <xs:attribute type="xs:decimal" name="max_v" use="optional"/>
          <xs:attribute type="xs:decimal" name="l_offset" use="optional"/>
          <xs:attribute type="xs:decimal" name="length" use="optional"/>
          <xs:attribute type="xs:decimal" name="width" use="optional"/>
          <xs:attribute type="xs:decimal" name="height" use="optional"/>
          <xs:attribute type="xs:int" name="accID" use="optional"/>
          <xs:attribute type="xs:int" name="mergeID" use="optional"/>
          <xs:attribute type="xs:string" name="vehicleType" use="optional"/>
          <xs:attribute type="xs:decimal" name="aggress" use="optional"/>
          <xs:attribute type="xs:string" name="behavior" use="optional"/>
          <xs:attribute type="xs:string" name="follow" use="optional"/>
          <xs:attribute type="xs:decimal" name="mergeTime" use="optional"/>
          <xs:attribute type="xs:decimal" name="offsetTime" use="optional"/>
      </xs:extension>
      </xs:simpleContent>
  </xs:complexType>
  </xs:element>
  */
  Base::txSysId id;
  Base::txSysId routeID;
  Base::txSysId accID;
  Base::txSysId mergeID;
  Base::txInt laneID;
  Base::txFloat start_s;
  Base::txFloat start_t;
  Base::txFloat start_v;
  Base::txFloat max_v;
  Base::txFloat l_offset;
  Base::txFloat length;
  Base::txFloat width;
  Base::txFloat height;
  Base::txString vehicleType;
  Base::txFloat aggress;
  Base::txString behavior;
  Base::txString follow;
  Base::txFloat mergeTime;
  Base::txFloat offsetTime;
  Base::txString eventId;
  Base::txFloat angle;
  Base::txString catalog;
  Base::txString start_angle;
  friend std::ostream& operator<<(std::ostream& os, const Vehicle& v) TX_NOEXCEPT {
    os << "vehicle : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(routeID, v.routeID) << TX_VARS_NAME(accID, v.accID)
       << TX_VARS_NAME(mergeID, v.mergeID) << TX_VARS_NAME(laneID, v.laneID) << TX_VARS_NAME(start_s, v.start_s)
       << TX_VARS_NAME(start_t, v.start_t) << TX_VARS_NAME(start_v, v.start_v) << TX_VARS_NAME(max_v, v.max_v)
       << TX_VARS_NAME(l_offset, v.l_offset) << TX_VARS_NAME(length, v.length) << TX_VARS_NAME(width, v.width)
       << TX_VARS_NAME(height, v.height) << TX_VARS_NAME(vehicleType, v.vehicleType) << TX_VARS_NAME(aggress, v.aggress)
       << TX_VARS_NAME(behavior, v.behavior) << TX_VARS_NAME(follow, v.follow) << TX_VARS_NAME(mergeTime, v.mergeTime)
       << TX_VARS_NAME(offsetTime, v.offsetTime) << TX_VARS_NAME(eventId, v.eventId) << TX_VARS_NAME(angle, v.angle)
       << TX_VARS_NAME(catalog, v.catalog) << TX_VARS_NAME(start_angle, v.start_angle);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.vehicle")*/
/*<vehicle id="200011" routeID="2" laneID="-1" start_s="70" start_t="0" start_v="12.0" max_v="12.2" l_offset="0"
 * length="4.5" width="1.8" height="1.5" accID="0" mergeID="0" vehicleType="Sedan" aggress="0.7"
 * behavior="TrafficVehicle" />*/

#if __TX_Mark__("traffic.obstacle")

/*< obstacle id = "1" routeID = "4" laneID = "-2" start_s = "159.41384532640345" l_offset = "1.3064446206059406"
              length = "0.75" width = "0.75" height = "0.75"
              vehicleType = "Box" direction = "0" / >*/
struct Obstacle {
  Base::txSysId id;
  Base::txSysId routeID;
  Base::txInt laneID;
  Base::txFloat start_s;
  Base::txFloat l_offset;
  Base::txFloat length;
  Base::txFloat width;
  Base::txFloat height;
  Base::txString vehicleType;
  Base::txFloat direction;
  Base::txString behavior;
  Base::txString eventId;
  Base::txFloat angle;
  Base::txString catalog;
  Base::txString start_angle;
  friend std::ostream& operator<<(std::ostream& os, const Obstacle& v) TX_NOEXCEPT {
    os << "obstacle : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(routeID, v.routeID) << TX_VARS_NAME(laneID, v.laneID)
       << TX_VARS_NAME(start_s, v.start_s) << TX_VARS_NAME(l_offset, v.l_offset) << TX_VARS_NAME(length, v.length)
       << TX_VARS_NAME(width, v.width) << TX_VARS_NAME(height, v.height) << TX_VARS_NAME(vehicleType, v.vehicleType)
       << TX_VARS_NAME(direction, v.direction) << TX_VARS_NAME(behavior, v.behavior) << TX_VARS_NAME(eventId, v.eventId)
       << TX_VARS_NAME(angle, v.angle) << TX_VARS_NAME(catalog, v.catalog) << TX_VARS_NAME(start_angle, v.start_angle);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.obstacle")*/

#if __TX_Mark__("traffic.pedestrian")
/*
<pedestrian id="1" routeID="1" laneID="-3" start_s="71.51625790096891" start_t="0"
type="human" l_offset="2.9023818012396578" start_v="5" max_v="12" behavior="UserDefine" />
*/
struct Pedestrian {
  Base::txSysId id;
  Base::txSysId routeID;
  Base::txInt laneID;
  Base::txFloat start_s;
  Base::txFloat start_t;
  Base::txFloat end_t;
  Base::txString type;
  Base::txFloat l_offset;
  Base::txFloat start_v;
  Base::txFloat max_v;
  Base::txString behavior;
  Base::txString eventId;
  Base::txFloat angle;
  Base::txString catalog;
  Base::txString start_angle;
  friend std::ostream& operator<<(std::ostream& os, const Pedestrian& v) TX_NOEXCEPT {
    os << "pedestrian : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(routeID, v.routeID)
       << TX_VARS_NAME(laneID, v.laneID) << TX_VARS_NAME(start_s, v.start_s) << TX_VARS_NAME(start_t, v.start_t)
       << TX_VARS_NAME(end_t, v.end_t) << TX_VARS_NAME(type, v.type) << TX_VARS_NAME(l_offset, v.l_offset)
       << TX_VARS_NAME(start_v, v.start_v) << TX_VARS_NAME(max_v, v.max_v) << TX_VARS_NAME(behavior, v.behavior)
       << TX_VARS_NAME(eventId, v.eventId) << TX_VARS_NAME(angle, v.angle) << TX_VARS_NAME(catalog, v.catalog)
       << TX_VARS_NAME(start_angle, v.start_angle);
    return os;
  }
};
#endif /*__TX_Mark__("traffic.pedestrian")*/

#if __TX_Mark__("traffic.signlight")
struct Signlight {
  Base::txSysId id;
  Base::txSysId routeID;
  Base::txFloat start_s;
  Base::txFloat start_t;
  Base::txFloat l_offset;
  Base::txInt time_green;
  Base::txInt time_yellow;
  Base::txInt time_red;
  Base::txInt direction;
  Base::txString lane;
  Base::txString phase;
  Base::txString status;
  Base::txFloat compliance;

  Base::txString plan;
  Base::txString junction;
  Base::txString phaseNumber;
  Base::txString signalHead;
  Base::txString eventId;

  friend std::ostream& operator<<(std::ostream& os, const Signlight& v) TX_NOEXCEPT {
    os << "Signlight : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(routeID, v.routeID)
       << TX_VARS_NAME(start_s, v.start_s) << TX_VARS_NAME(start_t, v.start_t) << TX_VARS_NAME(l_offset, v.l_offset)
       << TX_VARS_NAME(time_green, v.time_green) << TX_VARS_NAME(time_yellow, v.time_yellow)
       << TX_VARS_NAME(time_red, v.time_red) << TX_VARS_NAME(direction, v.direction) << TX_VARS_NAME(lane, v.lane)
       << TX_VARS_NAME(phase, v.phase) << TX_VARS_NAME(status, v.status) << TX_VARS_NAME(compliance, v.compliance)
       << TX_VARS_NAME(plan, v.plan) << TX_VARS_NAME(junction, v.junction) << TX_VARS_NAME(phaseNumber, v.phaseNumber)
       << TX_VARS_NAME(signalHead, v.signalHead) << TX_VARS_NAME(eventId, v.eventId);
    return os;
  }
};

#endif

#if __TX_Mark__("traffic.trafficflow")
struct Location {
  Base::txString id;
  Base::txString Pos;
  Base::txString info;
  friend std::ostream& operator<<(std::ostream& os, const Location& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "Location : " << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Pos, v.Pos)
       << TX_VARS_NAME(info, v.info);
    return os;
  }
};

struct VehType {
  Base::txString id;
  Base::txString Type;
  Base::txString length;
  Base::txString width;
  Base::txString height;
  Base::txString behavior;
  friend std::ostream& operator<<(std::ostream& os, const VehType& v) TX_NOEXCEPT {
    os << "VehType : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Type, v.Type)
       << TX_VARS_NAME(length, v.length) << TX_VARS_NAME(width, v.width) << TX_VARS_NAME(height, v.height)
       << TX_VARS_NAME(behavior, v.behavior);
    return os;
  }
};

struct CompNode {
  Base::txString Type;
  Base::txString Percentage;
  Base::txString Behavior;
  Base::txString Aggress;

  friend std::ostream& operator<<(std::ostream& os, const CompNode& v) TX_NOEXCEPT {
    os << "CompNode : " << _StreamPrecision_ << TX_VARS_NAME(Type, v.Type) << TX_VARS_NAME(Percentage, v.Percentage)
       << TX_VARS_NAME(Behavior, v.Behavior) << TX_VARS_NAME(Aggress, v.Aggress);
    return os;
  }
};

struct VehComp {
  enum { nCompCnt = 12 };
  Base::txString id;
  std::array<CompNode, nCompCnt> CompNodeArray;
  Base::txString Info;

  Base::txString Type(const Base::txInt idx) const TX_NOEXCEPT { return CompNodeArray[idx].Type; }
  Base::txString Percentage(const Base::txInt idx) const TX_NOEXCEPT { return CompNodeArray[idx].Percentage; }
  Base::txString Behavior(const Base::txInt idx) const TX_NOEXCEPT { return CompNodeArray[idx].Behavior; }
  Base::txString Aggress(const Base::txInt idx) const TX_NOEXCEPT { return CompNodeArray[idx].Aggress; }

  friend std::ostream& operator<<(std::ostream& os, const VehComp& v) TX_NOEXCEPT {
    os << "VehComp : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id);
    for (Base::txInt idx = 0; idx < v.CompNodeArray.size(); ++idx) {
      os << idx << ": " << v.CompNodeArray[idx] << ", ";
    }
    os << TX_VARS_NAME(Info, v.Info);
    return os;
  }
};

struct VehInput {
  Base::txString id;
  Base::txString Location;
  Base::txString Composition;
  Base::txString start_v;
  Base::txString max_v;
  Base::txString halfRange_v;
  Base::txString Distribution;
  Base::txString TimeHeadway;
  Base::txString Duration;
  Base::txString Cover;
  Base::txString Info;
  Base::txString Profile;

  friend std::ostream& operator<<(std::ostream& os, const VehInput& v) TX_NOEXCEPT {
    os << "VehInput : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Location, v.Location)
       << TX_VARS_NAME(Composition, v.Composition) << TX_VARS_NAME(start_v, v.start_v) << TX_VARS_NAME(max_v, v.max_v)
       << TX_VARS_NAME(halfRange_v, v.halfRange_v) << TX_VARS_NAME(Distribution, v.Distribution)
       << TX_VARS_NAME(TimeHeadway, v.TimeHeadway) << TX_VARS_NAME(Duration, v.Duration) << TX_VARS_NAME(Cover, v.Cover)
       << TX_VARS_NAME(Info, v.Info) << TX_VARS_NAME(Profile, v.Profile);
    return os;
  }
};

struct Beh {
  Base::txString id;
  Base::txString Type;
  Base::txString cc0;
  Base::txString cc1;
  Base::txString cc2;
  Base::txString AX;
  Base::txString BX_Add;
  Base::txString BX_Mult;
  Base::txString LCduration;

  friend std::ostream& operator<<(std::ostream& os, const Beh& v) TX_NOEXCEPT {
    os << "Beh : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Type, v.Type)
       << TX_VARS_NAME(cc0, v.cc0) << TX_VARS_NAME(cc1, v.cc1) << TX_VARS_NAME(cc2, v.cc2) << TX_VARS_NAME(AX, v.AX)
       << TX_VARS_NAME(BX_Add, v.BX_Add) << TX_VARS_NAME(BX_Mult, v.BX_Mult) << TX_VARS_NAME(LCduration, v.LCduration);
    return os;
  }
};

struct VehExit {
  Base::txString id;
  Base::txString Location;
  Base::txString Cover;
  Base::txString Info;
  friend std::ostream& operator<<(std::ostream& os, const VehExit& v) TX_NOEXCEPT {
    os << "VehExit : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Location, v.Location)
       << TX_VARS_NAME(Cover, v.Cover) << TX_VARS_NAME(Info, v.Info);
    return os;
  }
};

struct RouteGroup {
  Base::txString id;
  Base::txString Start;

  Base::txString Mid1;
  Base::txString End1;
  Base::txString Percentage1;

  Base::txString Mid2;
  Base::txString End2;
  Base::txString Percentage2;

  Base::txString Mid3;
  Base::txString End3;
  Base::txString Percentage3;

  friend std::ostream& operator<<(std::ostream& os, const RouteGroup& v) TX_NOEXCEPT {
    os << "RouteGroup : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Start, v.Start)

       << TX_VARS_NAME(Mid1, v.Mid1) << TX_VARS_NAME(End1, v.End1) << TX_VARS_NAME(Percentage1, v.Percentage1)

       << TX_VARS_NAME(Mid2, v.Mid2) << TX_VARS_NAME(End2, v.End2) << TX_VARS_NAME(Percentage2, v.Percentage2)

       << TX_VARS_NAME(Mid3, v.Mid3) << TX_VARS_NAME(End3, v.End3) << TX_VARS_NAME(Percentage3, v.Percentage3);
    return os;
  }
};

struct Compliance {
  Base::txString proportion;
  friend std::ostream& operator<<(std::ostream& os, const Compliance& v) TX_NOEXCEPT {
    os << "Compliance : " << TX_VARS_NAME(proportion, v.proportion);
    return os;
  }
};

struct TrafficFlow {
  std::vector<VehType> VehicleType;
  std::vector<VehComp> VehicleComposition;
  std::vector<VehInput> VehicleInput;
  std::vector<Beh> Behavior;
  std::vector<VehExit> VehicleExit;
  std::vector<RouteGroup> RouteGroups;
  std::vector<Compliance> RuleCompliance;
  friend std::ostream& operator<<(std::ostream& os, const TrafficFlow& v) TX_NOEXCEPT {
    os << _StreamPrecision_;
    os << "TrafficFlow : " << std::endl;
    os << "VehicleType : " << std::endl;
    for (const auto& refVehType : v.VehicleType) {
      os << refVehType << std::endl;
    }
    os << "VehicleComposition : " << std::endl;
    for (const auto& refVehComp : v.VehicleComposition) {
      os << refVehComp << std::endl;
    }
    os << "VehicleInput : " << std::endl;
    for (const auto& refVehInput : v.VehicleInput) {
      os << refVehInput << std::endl;
    }

    os << "Behavior : " << std::endl;
    for (const auto& refBeh : v.Behavior) {
      os << refBeh << std::endl;
    }
    os << "VehicleExit : " << std::endl;
    for (const auto& refVehExit : v.VehicleExit) {
      os << refVehExit << std::endl;
    }

    os << "RouteGroups : " << std::endl;
    for (const auto& refRouteGroup : v.RouteGroups) {
      os << refRouteGroup << std::endl;
    }

    os << "RuleCompliance : " << std::endl;
    for (const auto& refCompliance : v.RuleCompliance) {
      os << refCompliance << std::endl;
    }
    return os;
  }
};

#endif /*__TX_Mark__("traffic.trafficflow")*/

#if __TX_Mark__("ego input")

struct Start {
  Base::txString id;
  Base::txString Location;
  Base::txString start_v;
  Base::txString max_v;
  Base::txString theta;
  friend std::ostream& operator<<(std::ostream& os, const Start& v) TX_NOEXCEPT {
    os << "Start : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Location, v.Location)

       << TX_VARS_NAME(start_v, v.start_v) << TX_VARS_NAME(max_v, v.max_v) << TX_VARS_NAME(theta, v.theta);
    return os;
  }
};

struct End {
  Base::txString id;
  Base::txString Location;
  friend std::ostream& operator<<(std::ostream& os, const End& v) TX_NOEXCEPT {
    os << "End : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(Location, v.Location);
    return os;
  }
};

struct Input {
  Base::txString id;
  Base::txString mode;
  Base::txString Route;
  Base::txString number;
  Base::txString TimeHeadway;
  friend std::ostream& operator<<(std::ostream& os, const Input& v) TX_NOEXCEPT {
    os << "Input : " << _StreamPrecision_ << TX_VARS_NAME(id, v.id) << TX_VARS_NAME(mode, v.mode)
       << TX_VARS_NAME(Route, v.Route) << TX_VARS_NAME(number, v.number) << TX_VARS_NAME(TimeHeadway, v.TimeHeadway);
    return os;
  }
};

struct Ego {
  std::vector<Start> EgoStart;
  std::vector<End> EgoEnd;
  std::vector<Input> EgoInput;

  friend std::ostream& operator<<(std::ostream& os, const Ego& v) TX_NOEXCEPT {
    os << "Ego : " << std::endl;
    for (const auto& refStart : v.EgoStart) {
      os << refStart << std::endl;
    }
    for (const auto& refEnd : v.EgoEnd) {
      os << refEnd << std::endl;
    }
    for (const auto& refInput : v.EgoInput) {
      os << refInput << std::endl;
    }
    return os;
  }
};

struct LowerLeft {
  Base::txString Location;
  friend std::ostream& operator<<(std::ostream& os, const LowerLeft& v) TX_NOEXCEPT {
    os << "LowerLeft : " << TX_VARS_NAME(Location, v.Location) << std::endl;
    return os;
  }
};

struct UpperRight {
  Base::txString Location;
  friend std::ostream& operator<<(std::ostream& os, const UpperRight& v) TX_NOEXCEPT {
    os << "UpperRight : " << TX_VARS_NAME(Location, v.Location) << std::endl;
    return os;
  }
};

struct Center {
  Base::txString Location;
  friend std::ostream& operator<<(std::ostream& os, const Center& v) TX_NOEXCEPT {
    os << "Center : " << TX_VARS_NAME(Location, v.Location) << std::endl;
    return os;
  }
};

struct Map {
  LowerLeft ll;
  UpperRight ur;
  Center c;
  friend std::ostream& operator<<(std::ostream& os, const Map& v) TX_NOEXCEPT {
    os << "Map : " << v.ll << v.ur << v.c;
    return os;
  }
};

struct Rand_pedestrian {
  Base::txInt valid = 0;
  Base::txInt obs_row = 50;
  Base::txInt obs_col = 50;
  Base::txFloat obs_valid_radius = 500.0;
  Base::txInt obs_rnd_seed = 55;
  Base::txFloat obs_valid_section_length = 100.0;
  Base::txFloat pedestrian_valid_section_length = 10.0;
  Base::txFloat pedestrian_velocity = 1.2;
  Base::txFloat pedestrian_occurrence_dist = 30.0;
  Base::txInt pedestrian_show = 1;

  friend std::ostream& operator<<(std::ostream& os, const Rand_pedestrian& v) TX_NOEXCEPT {
    os << "Rand_pedestrian : " << TX_VARS_NAME(valid, v.valid) << TX_VARS_NAME(obs_row, v.obs_row)
       << TX_VARS_NAME(obs_col, v.obs_col) << TX_VARS_NAME(obs_valid_radius, v.obs_valid_radius)
       << TX_VARS_NAME(obs_rnd_seed, v.obs_rnd_seed)
       << TX_VARS_NAME(obs_valid_section_length, v.obs_valid_section_length)
       << TX_VARS_NAME(pedestrian_valid_section_length, v.pedestrian_valid_section_length)
       << TX_VARS_NAME(pedestrian_velocity, v.pedestrian_velocity)
       << TX_VARS_NAME(pedestrian_occurrence_dist, v.pedestrian_occurrence_dist)
       << TX_VARS_NAME(pedestrian_show, v.pedestrian_show);
    return os;
  }
};
#endif

struct traffic {
  Data data;
  std::vector<Location> Locations;
  std::vector<Route> routes;
  std::vector<Acc> accs;
  std::vector<Acc> accs_event;
  std::vector<Merge> merges;
  std::vector<Merge> merges_event;
  std::vector<Velocity> velocities;
  std::vector<Velocity> velocities_event;
  std::vector<Time_velocity> pedestrians_event_time_velocity;
  std::vector<Event_velocity> pedestrians_event_event_velocity;
  scene_event scene_event_array;
  std::vector<Vehicle> vehicles;
  std::vector<Obstacle> obstacles;
  std::vector<Pedestrian> pedestrians;
  std::vector<Signlight> signlights;
  std::string signlights_activePlan;
  TrafficFlow trafficFlow;
  Ego ego;
  Map map;
  Rand_pedestrian rand_pedestrian;
  friend std::ostream& operator<<(std::ostream& os, const traffic& v) TX_NOEXCEPT {
    os << v.data << std::endl;
    os << "Locations : " << std::endl;
    for (const auto& refLocation : v.Locations) {
      os << refLocation << std::endl;
    }

    os << "routes : " << std::endl;
    for (const auto& refRoute : v.routes) {
      os << refRoute << std::endl;
    }

    os << "accs : " << std::endl;
    for (const auto& refAcc : v.accs) {
      os << refAcc << std::endl;
    }

    os << "accs_event : " << std::endl;
    for (const auto& refAcc : v.accs_event) {
      os << refAcc << std::endl;
    }

    os << "merges : " << std::endl;
    for (const auto& refMerge : v.merges) {
      os << refMerge << std::endl;
    }

    os << "merges_event : " << std::endl;
    for (const auto& refMerge : v.merges_event) {
      os << refMerge << std::endl;
    }

    os << "velocities : " << std::endl;
    for (const auto& refVelocity : v.velocities) {
      os << refVelocity << std::endl;
    }

    os << "velocities_event : " << std::endl;
    for (const auto& refVelocity : v.velocities_event) {
      os << refVelocity << std::endl;
    }

    os << "pedestrians_event : " << std::endl;
    for (const auto& refTimeVelocity : v.pedestrians_event_time_velocity) {
      os << refTimeVelocity << std::endl;
    }
    for (const auto& refEventVelocity : v.pedestrians_event_event_velocity) {
      os << refEventVelocity << std::endl;
    }

    os << "scene_event : " << std::endl << v.scene_event_array << std::endl;

    os << "vehicles : " << std::endl;
    for (const auto& refVehicle : v.vehicles) {
      os << refVehicle << std::endl;
    }

    os << "obstacles : " << std::endl;
    for (const auto& refObstacle : v.obstacles) {
      os << refObstacle << std::endl;
    }

    os << "pedestrians : " << std::endl;
    for (const auto& refPedestrian : v.pedestrians) {
      os << refPedestrian << std::endl;
    }

    os << "signlights : "
       << "activePlan = " << v.signlights_activePlan << std::endl;
    for (const auto& refSignal : v.signlights) {
      os << refSignal << std::endl;
    }

    os << (v.trafficFlow) << std::endl;
    os << v.ego << std::endl;
    os << v.map << std::endl;
    os << v.rand_pedestrian << std::endl;
    return os;
  }
};

using traffic_ptr = std::shared_ptr<traffic>;

void debug_tip();

extern traffic_ptr load_scene_traffic(const Base::txString& _traffic_path) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(Traffic)
TX_NAMESPACE_CLOSE(SceneLoader)
