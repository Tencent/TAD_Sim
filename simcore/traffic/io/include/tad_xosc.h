// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <iomanip>
#include <sstream>
#include <vector>
#include "tx_header.h"

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(OSC_TADSim)

using txString = Base::txString;

struct xmlFile {
  /*
  <File filepath=""/>
  */
  txString filepath;
  friend std::ostream& operator<<(std::ostream& os, const xmlFile& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<File " << TX_VARS_NAME(filepath, quoted(v.filepath)) << "/>";
    return os;
  }
};

struct xmlProperty {
  /*
  <Property name="control" value="external"/>
  */
  txString name;
  txString value;
  friend std::ostream& operator<<(std::ostream& os, const xmlProperty& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Property " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(value, quoted(v.value)) << "/>";
    return os;
  }
};
using xmlProperties = std::vector<xmlProperty>;
inline std::ostream& operator<<(std::ostream& os, const xmlProperties& v) TX_NOEXCEPT {
  os << "<Properties>" << std::endl;
  for (const auto& refProp : v) {
    os << refProp << std::endl;
  }
  os << "</Properties>";
  return os;
}

struct xmlRearAxle {
  /*
  <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0" positionZ="0.31997"/>
  */
  txString maxSteering;
  txString wheelDiameter;
  txString trackWidth;
  txString positionX;
  txString positionZ;
  friend std::ostream& operator<<(std::ostream& os, const xmlRearAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<RearAxle " << TX_VARS_NAME(maxSteering, quoted(v.maxSteering))
       << TX_VARS_NAME(positionX, quoted(v.positionX)) << TX_VARS_NAME(positionZ, quoted(v.positionZ))
       << TX_VARS_NAME(trackWidth, quoted(v.trackWidth)) << TX_VARS_NAME(wheelDiameter, quoted(v.wheelDiameter))
       << "/>";
    return os;
  }
};

struct xmlFrontAxle {
  /*
  <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536" positionZ="0.31997"/>
  */
  txString maxSteering;
  txString wheelDiameter;
  txString trackWidth;
  txString positionX;
  txString positionZ;
  friend std::ostream& operator<<(std::ostream& os, const xmlFrontAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<FrontAxle " << TX_VARS_NAME(maxSteering, quoted(v.maxSteering))
       << TX_VARS_NAME(positionX, quoted(v.positionX)) << TX_VARS_NAME(positionZ, quoted(v.positionZ))
       << TX_VARS_NAME(trackWidth, quoted(v.trackWidth)) << TX_VARS_NAME(wheelDiameter, quoted(v.wheelDiameter))
       << "/>";
    return os;
  }
};

struct xmlAxles {
  /*
  <Axles>
      <FrontAxle maxSteering="0.46" wheelDiameter="0.63994" trackWidth="1.608" positionX="2.536" positionZ="0.31997"/>
      <RearAxle maxSteering="0" wheelDiameter="0.63994" trackWidth="1.608" positionX="0" positionZ="0.31997"/>
  </Axles>
  */
  xmlFrontAxle FrontAxle;
  xmlRearAxle RearAxle;
  friend std::ostream& operator<<(std::ostream& os, const xmlAxles& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Axles> " << std::endl
       << v.FrontAxle << std::endl
       << v.RearAxle << std::endl
       << "</Axles>";
    return os;
  }
};

struct xmlPerformance {
  /*
  <Performance maxSpeed="69.444" maxDeceleration="10" maxAcceleration="100"/>
  */
  txString maxSpeed;
  txString maxDeceleration;
  txString maxAcceleration;
  friend std::ostream& operator<<(std::ostream& os, const xmlPerformance& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Performance " << TX_VARS_NAME(maxAcceleration, quoted(v.maxAcceleration))
       << TX_VARS_NAME(maxDeceleration, quoted(v.maxDeceleration)) << TX_VARS_NAME(maxSpeed, quoted(v.maxSpeed))
       << "/>";
    return os;
  }
};

struct xmlCenter {
  /*
  <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  */
  txString x;
  txString y;
  txString z;
  friend std::ostream& operator<<(std::ostream& os, const xmlCenter& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Center " << TX_VARS_NAME(x, quoted(v.x)) << TX_VARS_NAME(y, quoted(v.y))
       << TX_VARS_NAME(z, quoted(v.z)) << "/>";
    return os;
  }
};

struct xmlDimensions {
  /*
  <Dimensions width="1.0" length="2.0" height="1.0"/>
  */
  txString width;
  txString length;
  txString height;
  friend std::ostream& operator<<(std::ostream& os, const xmlDimensions& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Dimensions " << TX_VARS_NAME(height, quoted(v.height))
       << TX_VARS_NAME(length, quoted(v.length)) << TX_VARS_NAME(width, quoted(v.width)) << "/>";
    return os;
  }
};

struct xmlBoundingBox {
  /*
  <BoundingBox>
      <Center x="121.17487722345179" y="31.270958542979315" z="0.0"/>
      <Dimensions width="1.0" length="2.0" height="1.0"/>
  </BoundingBox>
  */
  xmlCenter Center;
  xmlDimensions Dimensions;
  friend std::ostream& operator<<(std::ostream& os, const xmlBoundingBox& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<BoundingBox>" << std::endl
       << v.Center << std::endl
       << v.Dimensions << std::endl
       << "</BoundingBox>";
    return os;
  }
};

struct xmlPosition {
  struct xmlWorldPosition {
    /*
    <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
    */
    txString h;
    txString p = "0";
    txString r = "0";
    txString x;
    txString y;
    txString z;
    friend std::ostream& operator<<(std::ostream& os, const xmlWorldPosition& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<WorldPosition " << TX_VARS_NAME(h, quoted(v.h)) << TX_VARS_NAME(p, quoted(v.p))
         << TX_VARS_NAME(r, quoted(v.r)) << TX_VARS_NAME(x, quoted(v.x)) << TX_VARS_NAME(y, quoted(v.y))
         << TX_VARS_NAME(z, quoted(v.z)) << "/>";
      return os;
    }
    Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(x.empty()); }
  };

  xmlWorldPosition WorldPosition;
  /*
  <Position>
  <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
  <RelativeObjectPosition dx="0" dy="0" entityRef="M_1">
      <Orientation h="80.000000" type="relative"/>
  </RelativeObjectPosition>
  </Position>
  */
  struct xmlRelativeObjectPosition {
    txString dx;
    txString dy;
    txString entityRef;
    struct xmlOrientation {
      txString h;
      txString type;
      friend std::ostream& operator<<(std::ostream& os, const xmlOrientation& v) TX_NOEXCEPT {
        os << "<Orientation " << TX_VARS_NAME(h, quoted(v.h)) << TX_VARS_NAME(type, quoted(v.type)) << "/>";
        return os;
      }
    };
    xmlOrientation Orientation;
    Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(entityRef.empty()); }
    friend std::ostream& operator<<(std::ostream& os, const xmlRelativeObjectPosition& v) TX_NOEXCEPT {
      os << "<RelativeObjectPosition " << TX_VARS_NAME(dx, quoted(v.dx)) << TX_VARS_NAME(dy, quoted(v.dy))
         << TX_VARS_NAME(entityRef, quoted(v.entityRef)) << ">" << std::endl;
      os << v.Orientation << std::endl;
      os << "</RelativeObjectPosition>";
      return os;
    }
  };
  xmlRelativeObjectPosition RelativeObjectPosition;
  friend std::ostream& operator<<(std::ostream& os, const xmlPosition& v) TX_NOEXCEPT {
    os << "<Position> " << std::endl;
    if (v.WorldPosition.IsValid()) {
      os << v.WorldPosition << std::endl;
    }
    if (v.RelativeObjectPosition.IsValid()) {
      os << v.RelativeObjectPosition << std::endl;
    }
    os << "</Position>";
    return os;
  }
};

struct xmlPrivateAction {
  struct xmlRoutingAction {
    struct xmlAssignRouteAction {
      struct xmlRoute {
        txString name;
        txString closed;
        Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(name.empty()); }
        struct xmlWaypoint {
          txString routeStrategy;

          xmlPosition Position;
          friend std::ostream& operator<<(std::ostream& os, const xmlWaypoint& v) TX_NOEXCEPT {
            os << "<Waypoint " << TX_VARS_NAME(routeStrategy, quoted(v.routeStrategy)) << ">" << std::endl
               << v.Position << std::endl
               << "</Waypoint>";

            return os;
          }
        };
        std::vector<xmlWaypoint> Waypoint;
        friend std::ostream& operator<<(std::ostream& os, const xmlRoute& v) TX_NOEXCEPT {
          os << "<Route " << TX_VARS_NAME(closed, quoted(v.closed)) << TX_VARS_NAME(name, quoted(v.name)) << ">"
             << std::endl;
          for (const auto& wp : v.Waypoint) {
            os << wp << std::endl;
          }
          os << "</Route>";
          return os;
        }
      };

      xmlRoute Route;
      friend std::ostream& operator<<(std::ostream& os, const xmlAssignRouteAction& v) TX_NOEXCEPT {
        os << "<AssignRouteAction> " << std::endl << v.Route << std::endl << "</AssignRouteAction>";
        return os;
      }

      Base::txBool IsValid() const TX_NOEXCEPT { return Route.IsValid(); }
    };
    xmlAssignRouteAction AssignRouteAction;

    struct xmlFollowTrajectoryAction {
      struct xmlTrajectory {
        /*
        <Trajectory name="Trajectory1" closed="false">
        */
        txString name;
        txString closed;

        struct xmlShape {
          struct xmlPolyline {
            /*
            <Vertex time="0.0">
            */
            struct xmlVertex {
              xmlPosition Position;
              txString time;
              /*
              <Vertex time="0.0">
                  <Position>
                  <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
                  </Position>
              </Vertex>
              */
              friend std::ostream& operator<<(std::ostream& os, const xmlVertex& v) TX_NOEXCEPT {
                os << _StreamPrecision_ << "<Vertex " << TX_VARS_NAME(time, quoted(v.time)) << ">" << std::endl
                   << v.Position << std::endl
                   << "</Vertex>";
                return os;
              }
            };

            std::vector<xmlVertex> Vertexs;
            friend std::ostream& operator<<(std::ostream& os, const xmlPolyline& v) TX_NOEXCEPT {
              os << "<Polyline>" << std::endl;
              for (const auto& refVertex : v.Vertexs) {
                os << refVertex << std::endl;
              }
              os << "</Polyline>";
              return os;
            }
          };
          xmlPolyline Polyline;
          friend std::ostream& operator<<(std::ostream& os, const xmlShape& v) TX_NOEXCEPT {
            os << "<Shape>" << std::endl << v.Polyline << std::endl << "</Shape>";
            return os;
          }
        };
        xmlShape Shape;
        friend std::ostream& operator<<(std::ostream& os, const xmlTrajectory& v) TX_NOEXCEPT {
          os << _StreamPrecision_ << "<Trajectory " << TX_VARS_NAME(closed, quoted(v.closed))
             << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
             << v.Shape << std::endl
             << "</Trajectory>";
          return os;
        }
        Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(name.empty()); }
      };
      xmlTrajectory Trajectory;

      struct xmlTrajectoryFollowingMode {
        txString followingMode;
        /*
        <TrajectoryFollowingMode followingMode="follow"/>
        */
        friend std::ostream& operator<<(std::ostream& os, const xmlTrajectoryFollowingMode& v) TX_NOEXCEPT {
          os << _StreamPrecision_ << "<TrajectoryFollowingMode " << TX_VARS_NAME(followingMode, quoted(v.followingMode))
             << "/>";
          return os;
        }
      };
      xmlTrajectoryFollowingMode TrajectoryFollowingMode;
      Base::txBool IsValid() const TX_NOEXCEPT { return Trajectory.IsValid(); }
      friend std::ostream& operator<<(std::ostream& os, const xmlFollowTrajectoryAction& v) TX_NOEXCEPT {
        os << "<FollowTrajectoryAction>" << std::endl
           << v.Trajectory << std::endl
           << "<TimeReference/>" << std::endl
           << v.TrajectoryFollowingMode << std::endl
           << "</FollowTrajectoryAction>";
        return os;
      }
    };
    xmlFollowTrajectoryAction FollowTrajectoryAction;

    Base::txBool IsInited() const TX_NOEXCEPT {
      return AssignRouteAction.IsValid() || FollowTrajectoryAction.IsValid();
    }

    friend std::ostream& operator<<(std::ostream& os, const xmlRoutingAction& v) TX_NOEXCEPT {
      os << "<RoutingAction>" << std::endl;
      if (v.AssignRouteAction.IsValid()) {
        os << v.AssignRouteAction << std::endl;
      }
      if (v.FollowTrajectoryAction.IsValid()) {
        os << v.FollowTrajectoryAction << std::endl;
      }

      os << "</RoutingAction>";
      return os;
    }
  };

  xmlRoutingAction RoutingAction;

  struct xmlLongitudinalAction {
    struct xmlSpeedAction {
      struct xmlSpeedActionDynamics {
        txString dynamicsShape;
        txString value;
        txString dynamicsDimension;
        friend std::ostream& operator<<(std::ostream& os, const xmlSpeedActionDynamics& v) TX_NOEXCEPT {
          os << "<SpeedActionDynamics " << TX_VARS_NAME(dynamicsDimension, quoted(v.dynamicsDimension))
             << TX_VARS_NAME(dynamicsShape, quoted(v.dynamicsShape)) << TX_VARS_NAME(value, quoted(v.value)) << "/>";
          return os;
        }
      };
      xmlSpeedActionDynamics SpeedActionDynamics;

      struct xmlSpeedActionTarget {
        struct xmlAbsoluteTargetSpeed {
          txString value;
          friend std::ostream& operator<<(std::ostream& os, const xmlAbsoluteTargetSpeed& v) TX_NOEXCEPT {
            os << "<AbsoluteTargetSpeed " << TX_VARS_NAME(value, quoted(v.value)) << "/>";
            return os;
          }
          Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(value.empty()); }
        };
        xmlAbsoluteTargetSpeed AbsoluteTargetSpeed;
        friend std::ostream& operator<<(std::ostream& os, const xmlSpeedActionTarget& v) TX_NOEXCEPT {
          os << "<SpeedActionTarget> " << std::endl << (v.AbsoluteTargetSpeed) << std::endl << "</SpeedActionTarget>";
          return os;
        }
      };
      xmlSpeedActionTarget SpeedActionTarget;
      friend std::ostream& operator<<(std::ostream& os, const xmlSpeedAction& v) TX_NOEXCEPT {
        os << "<SpeedAction> " << std::endl
           << v.SpeedActionDynamics << std::endl
           << v.SpeedActionTarget << std::endl
           << "</SpeedAction>";
        return os;
      }
    };
    xmlSpeedAction SpeedAction;
    Base::txBool IsInited() const TX_NOEXCEPT {
      return CallFail(SpeedAction.SpeedActionDynamics.dynamicsShape.empty());
    }
    friend std::ostream& operator<<(std::ostream& os, const xmlLongitudinalAction& v) TX_NOEXCEPT {
      os << "<LongitudinalAction>" << std::endl << v.SpeedAction << std::endl << "</LongitudinalAction>";
      return os;
    }
  };
  xmlLongitudinalAction LongitudinalAction;

  struct xmlTeleportAction {
    struct xmlPosition {
      struct xmlWorldPosition {
        /*
        <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
        */
        txString h;
        txString p = "0";
        txString r = "0";
        txString x;
        txString y;
        txString z;
        friend std::ostream& operator<<(std::ostream& os, const xmlWorldPosition& v) TX_NOEXCEPT {
          os << _StreamPrecision_ << "<WorldPosition " << TX_VARS_NAME(h, quoted(v.h)) << TX_VARS_NAME(p, quoted(v.p))
             << TX_VARS_NAME(r, quoted(v.r)) << TX_VARS_NAME(x, quoted(v.x)) << TX_VARS_NAME(y, quoted(v.y))
             << TX_VARS_NAME(z, quoted(v.z)) << "/>";
          return os;
        }
        Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(x.empty()); }
      };

      xmlWorldPosition WorldPosition;
      /*
      <Position>
      <WorldPosition h="-1.330236836202213" x="121.17487722345179" y="31.270958542979315" z="0.0"/>
      </Position>
      */
      friend std::ostream& operator<<(std::ostream& os, const xmlPosition& v) TX_NOEXCEPT {
        os << _StreamPrecision_ << "<Position> " << std::endl << v.WorldPosition << std::endl << "</Position>";
        return os;
      }
    };
    xmlPosition Position;
    Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(Position.WorldPosition.x.empty()); }

    friend std::ostream& operator<<(std::ostream& os, const xmlTeleportAction& v) TX_NOEXCEPT {
      os << "<TeleportAction>" << std::endl << v.Position << std::endl << "</TeleportAction>";
      return os;
    }
  };
  xmlTeleportAction TeleportAction;

  struct xmlLateralAction {
    /*
    <LateralAction>
        <LaneChangeAction>
            <LaneChangeActionDynamics dynamicsDimension="time" dynamicsShape="sinusoidal" value="4.500000"/>
            <LaneChangeTarget>
            <RelativeTargetLane entityRef="V_3" value="1"/>
            </LaneChangeTarget>
        </LaneChangeAction>
    </LateralAction>
    */

    struct xmlLaneChangeAction {
      txString targetLaneOffset;
      struct xmlLaneChangeActionDynamics {
        txString dynamicsDimension;
        txString dynamicsShape;
        txString value;
        friend std::ostream& operator<<(std::ostream& os, const xmlLaneChangeActionDynamics& v) TX_NOEXCEPT {
          os << "<LaneChangeActionDynamics " << TX_VARS_NAME(dynamicsDimension, quoted(v.dynamicsDimension))
             << TX_VARS_NAME(dynamicsShape, quoted(v.dynamicsShape)) << TX_VARS_NAME(value, quoted(v.value)) << "/>";
          return os;
        }
      };
      xmlLaneChangeActionDynamics LaneChangeActionDynamics;
      struct xmlLaneChangeTarget {
        struct xmlRelativeTargetLane {
          txString entityRef;
          txString value;
          friend std::ostream& operator<<(std::ostream& os, const xmlRelativeTargetLane& v) TX_NOEXCEPT {
            os << "<RelativeTargetLane " << TX_VARS_NAME(entityRef, quoted(v.entityRef))
               << TX_VARS_NAME(value, quoted(v.value)) << "/>";
            return os;
          }
        };
        xmlRelativeTargetLane RelativeTargetLane;
        friend std::ostream& operator<<(std::ostream& os, const xmlLaneChangeTarget& v) TX_NOEXCEPT {
          os << "<LaneChangeTarget>" << std::endl << v.RelativeTargetLane << std::endl << "</LaneChangeTarget>";
          return os;
        }
        Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(RelativeTargetLane.entityRef.empty()); }
      };
      xmlLaneChangeTarget LaneChangeTarget;
      friend std::ostream& operator<<(std::ostream& os, const xmlLaneChangeAction& v) TX_NOEXCEPT {
        os << "<LaneChangeAction ";
        if (CallFail(v.targetLaneOffset.empty())) {
          os << TX_VARS_NAME(targetLaneOffset, quoted(v.targetLaneOffset));
        }
        os << ">" << std::endl
           << v.LaneChangeActionDynamics << std::endl
           << v.LaneChangeTarget << std::endl
           << "</LaneChangeAction>";
        return os;
      }
      Base::txBool IsInited() const TX_NOEXCEPT { return LaneChangeTarget.IsInited(); }
    };
    xmlLaneChangeAction LaneChangeAction;
    friend std::ostream& operator<<(std::ostream& os, const xmlLateralAction& v) TX_NOEXCEPT {
      os << "<LateralAction>" << std::endl << v.LaneChangeAction << std::endl << "</LateralAction>";
      return os;
    }
    Base::txBool IsInited() const TX_NOEXCEPT { return LaneChangeAction.IsInited(); }
  };
  xmlLateralAction LateralAction;
  friend std::ostream& operator<<(std::ostream& os, const xmlPrivateAction& v) TX_NOEXCEPT {
    os << "<PrivateAction>" << std::endl;
    if (v.LongitudinalAction.IsInited()) {
      os << v.LongitudinalAction << std::endl;
    }
    if (v.RoutingAction.IsInited()) {
      os << v.RoutingAction << std::endl;
    }
    if (v.TeleportAction.IsInited()) {
      os << v.TeleportAction << std::endl;
    }
    if (v.LateralAction.IsInited()) {
      os << v.LateralAction << std::endl;
    }
    os << "</PrivateAction>";
    return os;
  }
};

struct xmlEntityRef {
  /*<EntityRef entityRef="V_4"/>*/
  txString entityRef;
  friend std::ostream& operator<<(std::ostream& os, const xmlEntityRef& v) TX_NOEXCEPT {
    os << "<EntityRef " << TX_VARS_NAME(entityRef, quoted(v.entityRef)) << "/>";
    return os;
  }
  Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(entityRef.empty()); }
};

struct xmlStartTrigger {
  struct xmlConditionGroup {
    struct xmlCondition {
      struct xmlTriggeringEntities {
        /*
        <TriggeringEntities triggeringEntitiesRule="any">
            <EntityRef entityRef="V_4"/>
        </TriggeringEntities>
        */
        txString triggeringEntitiesRule;
        xmlEntityRef EntityRef;
        friend std::ostream& operator<<(std::ostream& os, const xmlTriggeringEntities& v) TX_NOEXCEPT {
          os << "<TriggeringEntities " << TX_VARS_NAME(triggeringEntitiesRule, quoted(v.triggeringEntitiesRule)) << ">"
             << std::endl
             << v.EntityRef << std::endl
             << "</TriggeringEntities>";
          return os;
        }
      };

      struct xmlEntityCondition {
        struct xmlTimeToCollisionCondition {
          struct xmlTimeToCollisionConditionTarget {
            /*<TimeToCollisionConditionTarget>
                <EntityRef entityRef="$Vehicle_Ego"/>
              </TimeToCollisionConditionTarget>
              */
            xmlEntityRef EntityRef;
            friend std::ostream& operator<<(std::ostream& os, const xmlTimeToCollisionConditionTarget& v) TX_NOEXCEPT {
              os << "<TimeToCollisionConditionTarget>" << std::endl
                 << v.EntityRef << std::endl
                 << "</TimeToCollisionConditionTarget>";
              return os;
            }
            Base::txBool IsValid() const TX_NOEXCEPT { return EntityRef.IsValid(); }
          };
          /*
          <TimeToCollisionCondition alongRoute="true" freespace="true" rule="lessThan" value="1.000000">
            <TimeToCollisionConditionTarget>
              <EntityRef entityRef="$Vehicle_Ego"/>
            </TimeToCollisionConditionTarget>
          </TimeToCollisionCondition>
          */
          txString alongRoute, freespace, rule, value;
          xmlTimeToCollisionConditionTarget TimeToCollisionConditionTarget;

          friend std::ostream& operator<<(std::ostream& os, const xmlTimeToCollisionCondition& v) TX_NOEXCEPT {
            os << "<TimeToCollisionCondition " << TX_VARS_NAME(alongRoute, quoted(v.alongRoute))
               << TX_VARS_NAME(freespace, quoted(v.freespace)) << TX_VARS_NAME(rule, quoted(v.rule))
               << TX_VARS_NAME(value, quoted(v.value)) << ">" << std::endl
               << v.TimeToCollisionConditionTarget << std::endl
               << "</TimeToCollisionCondition>";
            return os;
          }
          Base::txBool IsValid() const TX_NOEXCEPT { return TimeToCollisionConditionTarget.IsValid(); }
        };
        /*
        <EntityCondition>
            <TimeToCollisionCondition alongRoute="true" freespace="true" rule="lessThan" value="1.000000">
              <TimeToCollisionConditionTarget>
                <EntityRef entityRef="$Vehicle_Ego"/>
              </TimeToCollisionConditionTarget>
            </TimeToCollisionCondition>
          </EntityCondition>
        */
        xmlTimeToCollisionCondition TimeToCollisionCondition;

        struct xmlRelativeDistanceCondition {
          txString entityRef;
          txString freespace;
          txString relativeDistanceType;
          txString rule;
          txString value;
          friend std::ostream& operator<<(std::ostream& os, const xmlRelativeDistanceCondition& v) TX_NOEXCEPT {
            os << "<RelativeDistanceCondition " << TX_VARS_NAME(entityRef, quoted(v.entityRef))
               << TX_VARS_NAME(freespace, quoted(v.freespace))
               << TX_VARS_NAME(relativeDistanceType, quoted(v.relativeDistanceType))
               << TX_VARS_NAME(rule, quoted(v.rule)) << TX_VARS_NAME(value, quoted(v.value)) << "/>";
            return os;
          }
          Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(entityRef.empty()); }
        };
        xmlRelativeDistanceCondition RelativeDistanceCondition;
        friend std::ostream& operator<<(std::ostream& os, const xmlEntityCondition& v) TX_NOEXCEPT {
          os << "<EntityCondition> " << std::endl;
          if (v.TimeToCollisionCondition.IsValid()) {
            os << v.TimeToCollisionCondition << std::endl;
          }
          if (v.RelativeDistanceCondition.IsInited()) {
            os << v.RelativeDistanceCondition << std::endl;
          }
          os << "</EntityCondition>";
          return os;
        }
        Base::txBool IsValid() const TX_NOEXCEPT {
          return TimeToCollisionCondition.IsValid() || RelativeDistanceCondition.IsInited();
        }
      };

      struct xmlByValueCondition {
        /*  <ByValueCondition>
              <SimulationTimeCondition rule="greaterThan" value="0"/>
            </ByValueCondition>
        */
        struct xmlSimulationTimeCondition {
          txString rule, value;
          friend std::ostream& operator<<(std::ostream& os, const xmlSimulationTimeCondition& v) TX_NOEXCEPT {
            os << "<SimulationTimeCondition " << TX_VARS_NAME(rule, quoted(v.rule))
               << TX_VARS_NAME(value, quoted(v.value)) << "/>";
            return os;
          }
        };
        xmlSimulationTimeCondition SimulationTimeCondition;
        friend std::ostream& operator<<(std::ostream& os, const xmlByValueCondition& v) TX_NOEXCEPT {
          os << "<ByValueCondition>" << std::endl << v.SimulationTimeCondition << std::endl << "</ByValueCondition>";
          return os;
        }

        Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(SimulationTimeCondition.rule.empty()); }
      };
      xmlByValueCondition ByValueCondition;

      struct xmlByEntityCondition {
        xmlTriggeringEntities TriggeringEntities;
        xmlEntityCondition EntityCondition;
        Base::txBool IsValid() const TX_NOEXCEPT { return EntityCondition.IsValid(); }

        friend std::ostream& operator<<(std::ostream& os, const xmlByEntityCondition& v) TX_NOEXCEPT {
          os << "<ByEntityCondition>" << std::endl
             << v.TriggeringEntities << std::endl
             << v.EntityCondition << std::endl
             << "</ByEntityCondition>";
          return os;
        }
      };
      xmlByEntityCondition ByEntityCondition;

      txString conditionEdge, delay, name;
      friend std::ostream& operator<<(std::ostream& os, const xmlCondition& v) TX_NOEXCEPT {
        os << "<Condition " << TX_VARS_NAME(conditionEdge, quoted(v.conditionEdge))
           << TX_VARS_NAME(delay, quoted(v.delay)) << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl;
        if (v.ByEntityCondition.IsValid()) {
          os << v.ByEntityCondition << std::endl;
        }
        if (v.ByValueCondition.IsValid()) {
          os << v.ByValueCondition << std::endl;
        }
        os << "</Condition>";
        return os;
      }
    };
    std::vector<xmlCondition> Condition;
    friend std::ostream& operator<<(std::ostream& os, const xmlConditionGroup& v) TX_NOEXCEPT {
      os << "<ConditionGroup>" << std::endl;
      for (const auto& refCond : v.Condition) {
        os << refCond << std::endl;
      }
      os << "</ConditionGroup>";
      return os;
    }
  };
  std::vector<xmlConditionGroup> ConditionGroup;

  friend std::ostream& operator<<(std::ostream& os, const xmlStartTrigger& v) TX_NOEXCEPT {
    os << "<StartTrigger>" << std::endl;
    for (const auto& refCG : v.ConditionGroup) {
      os << refCG << std::endl;
    }
    os << "</StartTrigger>";
    return os;
  }
};

struct xmlManeuver {
  txString name;

  struct xmlEvent {
    /*
    <Event name="Event1" priority="overwrite">
    */
    txString name;
    txString priority;

    struct xmlAction {
      txString name;
      /*
      <Action name="Action1">
      */
      xmlPrivateAction PrivateAction;
      friend std::ostream& operator<<(std::ostream& os, const xmlAction& v) TX_NOEXCEPT {
        os << "<Action " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl
           << v.PrivateAction << std::endl
           << "</Action>";
        return os;
      }
    };
    std::vector<xmlAction> Action;

    xmlStartTrigger StartTrigger;
    friend std::ostream& operator<<(std::ostream& os, const xmlEvent& v) TX_NOEXCEPT {
      os << "<Event " << TX_VARS_NAME(name, quoted(v.name)) << TX_VARS_NAME(priority, quoted(v.priority)) << ">"
         << std::endl;
      for (const auto& refAct : v.Action) {
        os << refAct << std::endl;
      }
      os << v.StartTrigger << std::endl << "</Event>";
      return os;
    }
  };
  std::vector<xmlEvent> Event;
  friend std::ostream& operator<<(std::ostream& os, const xmlManeuver& v) TX_NOEXCEPT {
    os << "<Maneuver " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl;
    for (const auto& refEv : v.Event) {
      os << refEv << std::endl;
    }
    os << "</Maneuver>";
    return os;
  }
};

struct xmlStory {
  struct xmlAct {
    txString name;

    struct xmlManeuverGroup {
      /*
      <ManeuverGroup maximumExecutionCount="1" name="egocar">
      */
      txString maximumExecutionCount;
      txString name;

      struct xmlActors {
        /*
        <Actors selectTriggeringEntities="false">
        */
        txString selectTriggeringEntities;

        std::vector<xmlEntityRef> EntityRef;

        /*
        <Actors selectTriggeringEntities="false">
                <EntityRef entityRef="egocar"/>
              </Actors>
        */
        friend std::ostream& operator<<(std::ostream& os, const xmlActors& v) TX_NOEXCEPT {
          os << _StreamPrecision_ << "<Actors "
             << TX_VARS_NAME(selectTriggeringEntities, quoted(v.selectTriggeringEntities)) << ">" << std::endl;
          for (const auto& refEntity : v.EntityRef) {
            os << refEntity << std::endl;
          }
          os << "</Actors>";
          return os;
        }
      };

      xmlActors Actors;
      std::vector<xmlManeuver> Maneuver;
      friend std::ostream& operator<<(std::ostream& os, const xmlManeuverGroup& v) TX_NOEXCEPT {
        os << _StreamPrecision_ << "<ManeuverGroup "
           << TX_VARS_NAME(maximumExecutionCount, quoted(v.maximumExecutionCount)) << TX_VARS_NAME(name, quoted(v.name))
           << ">" << std::endl
           << v.Actors << std::endl;
        for (const auto& refManeuver : v.Maneuver) {
          os << refManeuver << std::endl;
        }

        os << "</ManeuverGroup>";
        return os;
      }
    };
    using xmlManeuverGroups = std::vector<xmlManeuverGroup>;
    xmlManeuverGroups ManeuverGroup;

    xmlStartTrigger StartTrigger;
    friend std::ostream& operator<<(std::ostream& os, const xmlAct& v) TX_NOEXCEPT {
      os << "<Act " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl;
      for (const auto& refManeuverGroup : v.ManeuverGroup) {
        os << refManeuverGroup << std::endl;
      }
      os << v.StartTrigger << std::endl;
      os << "</Act>";
      return os;
    }
  };
  std::vector<xmlAct> Act;
  /*
  <Story name="MyStory">
  */
  friend std::ostream& operator<<(std::ostream& os, const xmlStory& v) TX_NOEXCEPT {
    os << "<Story> " << std::endl;
    for (const auto& refAct : v.Act) {
      os << refAct << std::endl;
    }
    os << "</Story>";
    return os;
  }
};

struct xmlVehicle {
  /*
    <Vehicle name="Sedan" vehicleCategory="null">
      <BoundingBox>
        <Center x="0" y="0" z="0"/>
        <Dimensions height="1.360" length="4.700" width="1.850"/>
      </BoundingBox>
      <Performance maxAcceleration="null" maxDeceleration="null" maxSpeed="null"/>
      <Axles>
        <FrontAxle maxSteering="0" positionX="0" positionZ="0" trackWidth="0" wheelDiameter="0"/>
        <RearAxle maxSteering="0" positionX="0" positionZ="0" trackWidth="0" wheelDiameter="0"/>
      </Axles>
      <Properties>
        <Property name="StartV" value="5.000"/>
        <Property name="MaxV" value="12.000"/>
        <Property name="Behavior" value="TrafficVehicle"/>
      </Properties>
    </Vehicle>
  */
  txString name;
  txString vehicleCategory;
  xmlBoundingBox BoundingBox;
  xmlPerformance Performance;
  xmlAxles Axles;
  xmlProperties Properties;
  Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(name.empty()); }

  friend std::ostream& operator<<(std::ostream& os, const xmlVehicle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Vehicle " << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(vehicleCategory, quoted(v.vehicleCategory)) << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Performance << std::endl
       << v.Axles << std::endl
       << v.Properties << std::endl
       << "</Vehicle>";
    return os;
  }
};

struct xmlMiscObject {
  /*
  <MiscObject mass="mass" miscObjectCategory="miscObjectCategory" name="Box">
      <BoundingBox>
        <Center x="0" y="0" z="0"/>
        <Dimensions height="0.750" length="0.750" width="0.750"/>
      </BoundingBox>
      <Properties/>
    </MiscObject>
  */
  txString mass;
  txString miscObjectCategory;
  txString name;
  xmlBoundingBox BoundingBox;
  Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(name.empty()); }
  friend std::ostream& operator<<(std::ostream& os, const xmlMiscObject& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<MiscObject " << TX_VARS_NAME(mass, quoted(v.mass))
       << TX_VARS_NAME(miscObjectCategory, quoted(v.miscObjectCategory)) << TX_VARS_NAME(name, quoted(v.name)) << ">"
       << std::endl
       << v.BoundingBox << std::endl
       << "</MiscObject>";
    return os;
  }
};

struct xmlPedestrian {
  /*
  <Pedestrian mass="mass" model="model" name="moto_001" pedestrianCategory="pedestrianCategory">
      <BoundingBox>
        <Center x="0" y="0" z="0"/>
        <Dimensions height="1" length="1" width="1"/>
      </BoundingBox>
      <Properties>
        <Property name="StartV" value="0.000"/>
        <Property name="MaxV" value="1.000"/>
        <Property name="Behavior" value="UserDefine"/>
      </Properties>
    </Pedestrian>
  */
  txString mass, model, name, pedestrianCategory;
  xmlBoundingBox BoundingBox;
  xmlProperties Properties;
  Base::txBool IsValid() const TX_NOEXCEPT { return CallFail(name.empty()); }
  friend std::ostream& operator<<(std::ostream& os, const xmlPedestrian& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << "<Pedestrian " << TX_VARS_NAME(mass, quoted(v.mass))
       << TX_VARS_NAME(model, quoted(v.model)) << TX_VARS_NAME(name, quoted(v.name))
       << TX_VARS_NAME(pedestrianCategory, quoted(v.pedestrianCategory)) << ">" << std::endl
       << v.BoundingBox << std::endl
       << v.Properties << std::endl
       << "</Pedestrian>";
    return os;
  }
};

struct xmlOpenSCENARIO {
  struct xmlParameterDeclarations {
    struct xmlParameterDeclaration {
      /*
  <ParameterDeclaration parameterType="string" name="$owner" value="A2"/>
      */
      txString parameterType;
      txString name;
      txString value;
      friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclaration& v) TX_NOEXCEPT {
        os << _StreamPrecision_ << "<ParameterDeclaration " << TX_VARS_NAME(name, quoted(v.name))
           << TX_VARS_NAME(parameterType, quoted(v.parameterType)) << TX_VARS_NAME(value, quoted(v.value)) << "/>";
        return os;
      }
    };

    std::vector<xmlParameterDeclaration> ParameterDeclaration;
    friend std::ostream& operator<<(std::ostream& os, const xmlParameterDeclarations& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<ParameterDeclarations>" << std::endl;
      for (const auto& refPD : v.ParameterDeclaration) {
        os << refPD << std::endl;
      }
      os << "</ParameterDeclarations>";
      return os;
    }
    Base::txBool IsInited() const TX_NOEXCEPT { return CallFail(ParameterDeclaration.empty()); }
  };
  xmlParameterDeclarations ParameterDeclarations;

  struct xmlFileHeader {
    /*
<FileHeader
    description="simulation-test-1251316161/modules/144115205301725114/12334-5fd04d0ee0103c2314c7d695/sim_planning.tar.gz"
date="2020-12-10T02:28:13" author="" revMajor="1" revMinor="0"/>
    */
    txString description;
    txString date;
    txString author;
    txString revMajor;
    txString revMinor;
    friend std::ostream& operator<<(std::ostream& os, const xmlFileHeader& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<FileHeader " << TX_VARS_NAME(author, quoted(v.author))
         << TX_VARS_NAME(date, quoted(v.date)) << TX_VARS_NAME(description, quoted(v.description))
         << TX_VARS_NAME(revMajor, quoted(v.revMajor)) << TX_VARS_NAME(revMinor, quoted(v.revMinor)) << "/>";
      return os;
    }
  };
  xmlFileHeader FileHeader;

  struct xmlRoadNetwork {
    struct xmlLogicFile {
      /*
  <LogicFile filepath="test/map/d2d_20190726.xodr"/>
      */
      txString filepath;
      friend std::ostream& operator<<(std::ostream& os, const xmlLogicFile& v) TX_NOEXCEPT {
        os << _StreamPrecision_ << "<LogicFile " << TX_VARS_NAME(filepath, quoted(v.filepath)) << "/>";
        return os;
      }
    };
    xmlLogicFile LogicFile;
    friend std::ostream& operator<<(std::ostream& os, const xmlRoadNetwork& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<RoadNetwork>" << std::endl << v.LogicFile << std::endl << "</RoadNetwork>";
      return os;
    }
  };
  xmlRoadNetwork RoadNetwork;

  struct xmlScenarioObject {
    txString name;
    xmlVehicle Vehicle;
    xmlMiscObject Obstacle;
    xmlPedestrian Pedestrian;
    friend std::ostream& operator<<(std::ostream& os, const xmlScenarioObject& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<ScenarioObject " << TX_VARS_NAME(name, quoted(v.name)) << ">" << std::endl;
      if (v.Vehicle.IsValid()) {
        os << v.Vehicle << std::endl;
      }
      if (v.Pedestrian.IsValid()) {
        os << v.Pedestrian << std::endl;
      }
      if (v.Obstacle.IsValid()) {
        os << v.Obstacle << std::endl;
      }
      os << "</ScenarioObject>";
      return os;
    }
  };
  using xmlEntities = std::vector<xmlScenarioObject>;
  xmlEntities Entities;

  struct xmlStoryboard {
    struct xmlInit {
      struct xmlActions {
        struct xmlPrivate {
          /*
          <Private entityRef="egocar">
          */
          txString entityRef;

          std::vector<xmlPrivateAction> PrivateAction;

          friend std::ostream& operator<<(std::ostream& os, const xmlPrivate& v) TX_NOEXCEPT {
            os << "<Private " << TX_VARS_NAME(entityRef, quoted(v.entityRef)) << ">" << std::endl;
            for (const auto& refPrivate : v.PrivateAction) {
              os << refPrivate << std::endl;
            }
            os << "</Private>";
            return os;
          }
        };

        std::vector<xmlPrivate> Private;
        friend std::ostream& operator<<(std::ostream& os, const xmlActions& v) TX_NOEXCEPT {
          os << "<Actions>" << std::endl;
          for (const auto& refPrivate : v.Private) {
            os << refPrivate << std::endl;
          }
          os << "</Actions>";
          return os;
        }
      };
      // std::vector<xmlGlobalAction>  GlobalActions;
      xmlActions Actions;
      friend std::ostream& operator<<(std::ostream& os, const xmlInit& v) TX_NOEXCEPT {
        os << _StreamPrecision_ << "<Init>" << std::endl << v.Actions << std::endl << "</Init>";
        return os;
      }
    };
    xmlInit Init;

    xmlStory Story;

    friend std::ostream& operator<<(std::ostream& os, const xmlStoryboard& v) TX_NOEXCEPT {
      os << _StreamPrecision_ << "<Storyboard> " << std::endl
         << v.Init << std::endl
         << v.Story << std::endl
         << "</Storyboard>";
      return os;
    }

    Base::txBool IsInited() const TX_NOEXCEPT { return true; }
  };
  xmlStoryboard Storyboard;
  friend std::ostream& operator<<(std::ostream& os, const xmlOpenSCENARIO& v) TX_NOEXCEPT {
    os << R"(<?xml version="1.0" encoding="UTF-8" standalone="no" ?>
<OpenSCENARIO xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:noNamespaceSchemaLocation="OpenSCENARIO.xsd">)"
       << std::endl
       << v.FileHeader << std::endl
       << v.ParameterDeclarations << std::endl
       << R"(<CatalogLocations>
<VehicleCatalog>
<Directory path="../xosc/Catalogs/Vehicles"/>
</VehicleCatalog>
</CatalogLocations>)"
       << std::endl
       << v.RoadNetwork << std::endl
       << "<Entities>" << std::endl;
    for (const auto& refEntity : v.Entities) {
      os << refEntity << std::endl;
    }
    os << "</Entities>" << std::endl << v.Storyboard << std::endl << "</OpenSCENARIO>";
    return os;
  }
};

using OpenSCENARIO_ptr = std::shared_ptr<xmlOpenSCENARIO>;

void debug_tip();

extern OpenSCENARIO_ptr load_scene_tadsim_xosc(const Base::txString& _traffic_path) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(OSC_TADSim)
TX_NAMESPACE_CLOSE(SceneLoader)
