// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "common/macros.h"
#include "types/map_defs.h"

#include <map>
#include <memory>
#include <ostream>
#include <vector>

namespace hadmap {
// Forward declaration of txLaneBoundary class
class txLaneBoundary;
// Define shared pointer and vector types for txLaneBoundary
typedef std::shared_ptr<txLaneBoundary> txLaneBoundaryPtr;
typedef std::vector<txLaneBoundaryPtr> txLaneBoundaries;
typedef std::shared_ptr<const txLaneBoundary> txLaneBoundaryConstPtr;
typedef std::vector<txLaneBoundaryConstPtr> txLaneBoundariesConst;
// Define map type for txLaneBoundary using lane boundary pkid as key
typedef std::map<laneboundarypkid, txLaneBoundaryPtr> txLaneBoundaryMap;

// Define txLaneId structure
struct TXSIMMAP_API txLaneId {
  roadpkid roadId;
  sectionpkid sectionId;
  lanepkid laneId;

  // Default constructor
  txLaneId() : roadId(0), sectionId(0), laneId(0) {}
  // Constructor with parameters
  txLaneId(roadpkid rid, sectionpkid sid, lanepkid lid) : roadId(rid), sectionId(sid), laneId(lid) {}

  // Copy constructor
  txLaneId(const txLaneId& id) : roadId(id.roadId), sectionId(id.sectionId), laneId(id.laneId) {}

  // Equality operator
  bool operator==(const txLaneId& id) const {
    return (roadId == id.roadId) && (sectionId == id.sectionId) && (laneId == id.laneId);
  }

  // Less than operator for sorting
  bool operator<(const hadmap::txLaneId& other) const {
    if ((roadId < other.roadId) || ((roadId == other.roadId) && (sectionId < other.sectionId)) ||
        ((roadId == other.roadId) && (sectionId == other.sectionId) && (laneId < other.laneId))) {
      return true;
    }
    return false;
  }

  // Output stream operator for printing
  friend std::ostream& operator<<(std::ostream& out, const txLaneId& id) {
    out << id.roadId << "." << id.sectionId << "." << id.laneId;
    return out;
  }
};

// Forward declaration of txLane class
class txLane;
// Define shared pointer and vector types for txLane
typedef std::shared_ptr<txLane> txLanePtr;
typedef std::vector<txLanePtr> txLanes;
typedef std::shared_ptr<const txLane> txLaneConstPtr;
typedef std::vector<txLaneConstPtr> txLanesConst;
// Define map type for txLane using txLaneId as key
typedef std::map<txLaneId, txLanePtr> txLaneMap;

// Define txSectionId structure
struct TXSIMMAP_API txSectionId {
  roadpkid roadId;
  sectionpkid sectionId;

  // Default constructor
  txSectionId() : roadId(0), sectionId(0) {}
  // Constructor with parameters
  txSectionId(roadpkid rid, sectionpkid sid) : roadId(rid), sectionId(sid) {}
};

// Forward declaration of txSection class
class txSection;
// Define shared pointer and vector types for txSection
typedef std::shared_ptr<txSection> txSectionPtr;
typedef std::vector<txSectionPtr> txSections;
typedef std::shared_ptr<const txSection> txSectionConstPtr;
typedef std::vector<txSectionConstPtr> txSectionsConst;
// Define map type for txSection using txSectionId as key and txLanes as value
typedef std::map<txSectionId, txLanes> txSectionMap;

// Forward declaration of txRoad class
class txRoad;
// Define shared pointer and vector types for txRoad
typedef std::shared_ptr<txRoad> txRoadPtr;
typedef std::vector<txRoadPtr> txRoads;
typedef std::shared_ptr<const txRoad> txRoadConstPtr;
typedef std::vector<txRoadConstPtr> txRoadsConst;
// Define map type for txRoad using roadpkid as key
typedef std::map<roadpkid, txRoadPtr> txRoadMap;

// Forward declaration of txObjectGeom class
class txObjectGeom;
// Define shared pointer and vector types for txObjectGeom
typedef std::shared_ptr<txObjectGeom> txObjGeomPtr;
typedef std::vector<txObjGeomPtr> txObjGeoms;
typedef std::shared_ptr<const txObjectGeom> txObjGeomConstPtr;
typedef std::vector<txObjGeomConstPtr> txObjGeomsConst;

// Forward declaration of txCurve and txLineCurve classes
class txCurve;
class txLineCurve;
// Define shared pointer and vector types for txLineCurve
typedef std::shared_ptr<txLineCurve> txLineCurvePtr;
typedef std::vector<txLineCurvePtr> txLineCurves;
typedef std::shared_ptr<const txLineCurve> txLineCurveConstPtr;
typedef std::vector<txLineCurveConstPtr> txLineCurvesConst;

// Forward declaration of txObject class
class txObject;
// Define shared pointer and vector types for txObject
typedef std::shared_ptr<txObject> txObjectPtr;
typedef std::vector<txObjectPtr> txObjects;
typedef std::shared_ptr<const txObject> txObjectConstPtr;
typedef std::vector<txObjectConstPtr> txObjectsConst;

// Forward declaration of txLaneLink class
class txLaneLink;
// Define shared pointer and vector types for txLaneLink
typedef std::shared_ptr<txLaneLink> txLaneLinkPtr;
typedef std::vector<txLaneLinkPtr> txLaneLinks;
typedef std::map<uint64_t, txLaneLinkPtr> txLaneLinkMap;
typedef std::shared_ptr<const txLaneLink> txLaneLinkConstPtr;
typedef std::vector<txLaneLinkConstPtr> txLaneLinksConst;

// Forward declaration of txJunction class
class txJunction;
// Define shared pointer and vector types for txJunction
typedef std::shared_ptr<txJunction> txJunctionPtr;
typedef std::vector<txJunctionPtr> txJunctions;
typedef std::shared_ptr<const txJunction> txJunctionConstPtr;
typedef std::vector<txJunctionConstPtr> txJunctionsConst;

// Forward declaration of txOdHeader class
class txOdHeader;
// Define shared pointer type for txOdHeader
typedef std::shared_ptr<txOdHeader> txOdHeaderPtr;

}  // namespace hadmap
