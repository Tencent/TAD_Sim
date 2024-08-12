// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <sstream>
#include "tx_header.h"
#include "tx_string_utils.h"

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Type_Catalog)

struct FileHeader {
  Base::txString revMajor;
  Base::txString revMinor;
  Base::txString date;
  Base::txString description;
  Base::txString author;
  friend std::ostream& operator<<(std::ostream& os, const FileHeader& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(revMajor, v.revMajor) << TX_VARS_NAME(revMinor, v.revMinor)
       << TX_VARS_NAME(date, v.date) << TX_VARS_NAME(description, v.description) << TX_VARS_NAME(author, v.author);
    return os;
  }
};

struct Center {
  Base::txString x, y, z;
  Base::txVec3 rear2center;
  void Init() TX_NOEXCEPT {
    rear2center[0] = std::stod(x);
    rear2center[1] = std::stod(y);
    rear2center[2] = std::stod(z);
  }
  friend std::ostream& operator<<(std::ostream& os, const Center& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(x, v.x) << TX_VARS_NAME(y, v.y) << TX_VARS_NAME(z, v.z)
       << TX_VARS_NAME(rear2center, Utils::ToString(v.rear2center));
    return os;
  }
};

struct Dimensions {
  Base::txString height, length, width;
  Base::txFloat dbLength, dbWidth, dbHeight;
  void Init() TX_NOEXCEPT {
    dbLength = std::stod(length);
    dbWidth = std::stod(width);
    dbHeight = std::stod(height);
  }
  friend std::ostream& operator<<(std::ostream& os, const Dimensions& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(height, v.height) << TX_VARS_NAME(length, v.length)
       << TX_VARS_NAME(width, v.width) << TX_VARS_NAME(dbLength, v.dbLength) << TX_VARS_NAME(dbWidth, v.dbWidth)
       << TX_VARS_NAME(dbHeight, v.dbHeight);
    return os;
  }
};

struct BoundingBox {
  Center center;
  Dimensions dim;
  void Init() TX_NOEXCEPT {
    center.Init();
    dim.Init();
  }
  friend std::ostream& operator<<(std::ostream& os, const BoundingBox& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(Center, v.center) << std::endl
       << TX_VARS_NAME(Dimensions, v.dim) << std::endl;
    return os;
  }
};

struct Performance {
  Base::txString maxSpeed, maxDeceleration, maxAcceleration;
  friend std::ostream& operator<<(std::ostream& os, const Performance& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(maxSpeed, v.maxSpeed) << TX_VARS_NAME(maxDeceleration, v.maxDeceleration)
       << TX_VARS_NAME(maxAcceleration, v.maxAcceleration);
    return os;
  }
};

struct FrontAxle {
  Base::txString maxSteering, wheelDiameter, trackWidth, positionX, positionZ;
  friend std::ostream& operator<<(std::ostream& os, const FrontAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(maxSteering, v.maxSteering) << TX_VARS_NAME(wheelDiameter, v.wheelDiameter)
       << TX_VARS_NAME(trackWidth, v.trackWidth) << TX_VARS_NAME(positionX, v.positionX)
       << TX_VARS_NAME(positionZ, v.positionZ);
    return os;
  }
};

struct RearAxle {
  Base::txString maxSteering, wheelDiameter, trackWidth, positionX, positionZ;
  friend std::ostream& operator<<(std::ostream& os, const RearAxle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(maxSteering, v.maxSteering) << TX_VARS_NAME(wheelDiameter, v.wheelDiameter)
       << TX_VARS_NAME(trackWidth, v.trackWidth) << TX_VARS_NAME(positionX, v.positionX)
       << TX_VARS_NAME(positionZ, v.positionZ);
    return os;
  }
};

struct Axles {
  FrontAxle frontAxle;
  RearAxle rearAxle;
  friend std::ostream& operator<<(std::ostream& os, const Axles& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(FrontAxle, v.frontAxle) << std::endl
       << TX_VARS_NAME(RearAxle, v.rearAxle) << std::endl;
    return os;
  }
};

struct Property {
  Base::txString name, value;
  friend std::ostream& operator<<(std::ostream& os, const Property& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(PropertyName, v.name) << std::endl
       << TX_VARS_NAME(PropertyValue, v.value) << std::endl;
    return os;
  }
};

struct Vehicle {
  Base::txString name;
  Base::txString vehicleCategory;
  BoundingBox bbx;
  Performance perf;
  Axles axles;
  std::vector<Property> properties;
  friend std::ostream& operator<<(std::ostream& os, const Vehicle& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(name, v.name) << TX_VARS_NAME(vehicleCategory, v.vehicleCategory)
       << std::endl
       << TX_VARS_NAME(BoundingBox, v.bbx) << std::endl
       << TX_VARS_NAME(Performance, v.perf) << std::endl
       << TX_VARS_NAME(Axles, v.axles) << std::endl;
    for (const auto& refProperty : v.properties) {
      os << refProperty << std::endl;
    }
    return os;
  }
};

struct Pedestrian {
  Base::txString name;
  Base::txString pedestrianCategory;
  BoundingBox bbx;
  std::vector<Property> properties;
  friend std::ostream& operator<<(std::ostream& os, const Pedestrian& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(name, v.name) << TX_VARS_NAME(pedestrianCategory, v.pedestrianCategory)
       << std::endl
       << TX_VARS_NAME(BoundingBox, v.bbx) << std::endl;
    for (const auto& refProperty : v.properties) {
      os << refProperty << std::endl;
    }
    return os;
  }
};

struct MiscObject {
  Base::txString name;
  Base::txString miscObjectCategory;
  BoundingBox bbx;
  std::vector<Property> properties;
  friend std::ostream& operator<<(std::ostream& os, const MiscObject& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(name, v.name) << TX_VARS_NAME(miscObjectCategory, v.miscObjectCategory)
       << std::endl
       << TX_VARS_NAME(BoundingBox, v.bbx) << std::endl;
    for (const auto& refProperty : v.properties) {
      os << refProperty << std::endl;
    }
    return os;
  }
};

struct Catalog {
  Base::txString name;
  std::vector<Vehicle> vehicles;
  std::vector<Pedestrian> pedestrians;
  std::vector<MiscObject> obstacles;
  friend std::ostream& operator<<(std::ostream& os, const Catalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(name, v.name);
    for (const auto& refVeh : v.vehicles) {
      os << refVeh << std::endl;
    }

    for (const auto& refPed : v.pedestrians) {
      os << refPed << std::endl;
    }

    for (const auto& refObs : v.obstacles) {
      os << refObs << std::endl;
    }
    return os;
  }
};

struct VehicleCatalog {
  FileHeader fileHeader;
  Catalog catalog;
  friend std::ostream& operator<<(std::ostream& os, const VehicleCatalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(< FileHeader, v.fileHeader) << "</FileHeader>" << std::endl
       << TX_VARS_NAME(< Catalog, v.catalog) << "</Catalog>" << std::endl;
    return os;
  }
};

using VehicleCatalogPtr = std::shared_ptr<VehicleCatalog>;

struct PedestrianCatalog {
  FileHeader fileHeader;
  Catalog catalog;
  friend std::ostream& operator<<(std::ostream& os, const PedestrianCatalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(FileHeader, v.fileHeader) << std::endl
       << TX_VARS_NAME(Catalog, v.catalog) << std::endl;
    return os;
  }
};

using PedestrianCatalogPtr = std::shared_ptr<PedestrianCatalog>;

struct MiscObjectCatalog {
  FileHeader fileHeader;
  Catalog catalog;
  friend std::ostream& operator<<(std::ostream& os, const MiscObjectCatalog& v) TX_NOEXCEPT {
    os << _StreamPrecision_ << TX_VARS_NAME(FileHeader, v.fileHeader) << std::endl
       << TX_VARS_NAME(Catalog, v.catalog) << std::endl;
    return os;
  }
};
using MiscObjectCatalogPtr = std::shared_ptr<MiscObjectCatalog>;

extern VehicleCatalogPtr load_vehicle_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT;
extern PedestrianCatalogPtr load_pedestrian_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT;
extern MiscObjectCatalogPtr load_obstacle_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT;

TX_NAMESPACE_CLOSE(Type_Catalog)
TX_NAMESPACE_CLOSE(SceneLoader)
