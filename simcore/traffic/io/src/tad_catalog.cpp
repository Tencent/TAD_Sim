// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_catalog.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include "tx_xml_utils.h"
#include "xml/intros_ptree.hpp"
#include "xml/util_traits.hpp"
using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SceneLoader)
TX_NAMESPACE_OPEN(Type_Catalog)

BEGIN_INTROS_TYPE_USER_NAME(FileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(revMajor, MAKE_USER_NAME("revMajor", "", true))
ADD_INTROS_ITEM_USER_NAME(revMinor, MAKE_USER_NAME("revMinor", "", true))
ADD_INTROS_ITEM_USER_NAME(date, MAKE_USER_NAME("date", "", true))
ADD_INTROS_ITEM_USER_NAME(description, MAKE_USER_NAME("description", "", true))
ADD_INTROS_ITEM_USER_NAME(author, MAKE_USER_NAME("author", "", true))
END_INTROS_TYPE(FileHeader)

BEGIN_INTROS_TYPE_USER_NAME(Center, "Center")
ADD_INTROS_ITEM_USER_NAME(x, MAKE_USER_NAME("x", "", true))
ADD_INTROS_ITEM_USER_NAME(y, MAKE_USER_NAME("y", "", true))
ADD_INTROS_ITEM_USER_NAME(z, MAKE_USER_NAME("z", "", true))
END_INTROS_TYPE(Center)

BEGIN_INTROS_TYPE_USER_NAME(Dimensions, "Dimensions")
ADD_INTROS_ITEM_USER_NAME(height, MAKE_USER_NAME("height", "", true))
ADD_INTROS_ITEM_USER_NAME(length, MAKE_USER_NAME("length", "", true))
ADD_INTROS_ITEM_USER_NAME(width, MAKE_USER_NAME("width", "", true))
END_INTROS_TYPE(Dimensions)

BEGIN_INTROS_TYPE_USER_NAME(BoundingBox, "BoundingBox")
ADD_INTROS_ITEM_USER_NAME(center, "Center")
ADD_INTROS_ITEM_USER_NAME(dim, "Dimensions")
END_INTROS_TYPE(BoundingBox)

BEGIN_INTROS_TYPE_USER_NAME(Performance, "Performance")
ADD_INTROS_ITEM_USER_NAME(maxSpeed, MAKE_USER_NAME("maxSpeed", "", true))
ADD_INTROS_ITEM_USER_NAME(maxDeceleration, MAKE_USER_NAME("maxDeceleration", "", true))
ADD_INTROS_ITEM_USER_NAME(maxAcceleration, MAKE_USER_NAME("maxAcceleration", "", true))
END_INTROS_TYPE(Performance)

BEGIN_INTROS_TYPE_USER_NAME(FrontAxle, "FrontAxle")
ADD_INTROS_ITEM_USER_NAME(maxSteering, MAKE_USER_NAME("maxSteering", "", true))
ADD_INTROS_ITEM_USER_NAME(wheelDiameter, MAKE_USER_NAME("wheelDiameter", "", true))
ADD_INTROS_ITEM_USER_NAME(trackWidth, MAKE_USER_NAME("trackWidth", "", true))
ADD_INTROS_ITEM_USER_NAME(positionX, MAKE_USER_NAME("positionX", "", true))
ADD_INTROS_ITEM_USER_NAME(positionZ, MAKE_USER_NAME("positionZ", "", true))
END_INTROS_TYPE(FrontAxle)

BEGIN_INTROS_TYPE_USER_NAME(RearAxle, "RearAxle")
ADD_INTROS_ITEM_USER_NAME(maxSteering, MAKE_USER_NAME("maxSteering", "", true))
ADD_INTROS_ITEM_USER_NAME(wheelDiameter, MAKE_USER_NAME("wheelDiameter", "", true))
ADD_INTROS_ITEM_USER_NAME(trackWidth, MAKE_USER_NAME("trackWidth", "", true))
ADD_INTROS_ITEM_USER_NAME(positionX, MAKE_USER_NAME("positionX", "", true))
ADD_INTROS_ITEM_USER_NAME(positionZ, MAKE_USER_NAME("positionZ", "", true))
END_INTROS_TYPE(RearAxle)

BEGIN_INTROS_TYPE_USER_NAME(Axles, "Axles")
ADD_INTROS_ITEM_USER_NAME(frontAxle, "FrontAxle")
ADD_INTROS_ITEM_USER_NAME(rearAxle, "RearAxle")
END_INTROS_TYPE(Axles)

BEGIN_INTROS_TYPE_USER_NAME(Property, "Property")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(value, MAKE_USER_NAME("value", "", true))
END_INTROS_TYPE(Property)

BEGIN_INTROS_TYPE_USER_NAME(Vehicle, "Vehicle")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicleCategory, MAKE_USER_NAME("vehicleCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(bbx, "BoundingBox")
ADD_INTROS_ITEM_USER_NAME(perf, "Performance")
ADD_INTROS_ITEM_USER_NAME(axles, "Axles")
ADD_INTROS_ITEM_USER_NAME(properties, MAKE_USER_NAME("Property", "Properties", false))
END_INTROS_TYPE(Vehicle)

BEGIN_INTROS_TYPE_USER_NAME(Pedestrian, "Pedestrian")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(pedestrianCategory, MAKE_USER_NAME("pedestrianCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(bbx, "BoundingBox")
ADD_INTROS_ITEM_USER_NAME(properties, MAKE_USER_NAME("Property", "Properties", false))
END_INTROS_TYPE(Pedestrian)

BEGIN_INTROS_TYPE_USER_NAME(MiscObject, "MiscObject")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(miscObjectCategory, MAKE_USER_NAME("miscObjectCategory", "", true))
ADD_INTROS_ITEM_USER_NAME(bbx, "BoundingBox")
ADD_INTROS_ITEM_USER_NAME(properties, MAKE_USER_NAME("Property", "Properties", false))
END_INTROS_TYPE(MiscObject)

BEGIN_INTROS_TYPE_USER_NAME(Catalog, "Catalog")
ADD_INTROS_ITEM_USER_NAME(name, MAKE_USER_NAME("name", "", true))
ADD_INTROS_ITEM_USER_NAME(vehicles, MAKE_USER_NAME("Vehicle", "", false))
ADD_INTROS_ITEM_USER_NAME(pedestrians, MAKE_USER_NAME("Pedestrian", "", false))
ADD_INTROS_ITEM_USER_NAME(obstacles, MAKE_USER_NAME("MiscObject", "", false))
END_INTROS_TYPE(Catalog)

BEGIN_INTROS_TYPE_USER_NAME(VehicleCatalog, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(fileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(catalog, "Catalog")
END_INTROS_TYPE(VehicleCatalog)

BEGIN_INTROS_TYPE_USER_NAME(PedestrianCatalog, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(fileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(catalog, "Catalog")
END_INTROS_TYPE(PedestrianCatalog)

BEGIN_INTROS_TYPE_USER_NAME(MiscObjectCatalog, "OpenSCENARIO")
ADD_INTROS_ITEM_USER_NAME(fileHeader, "FileHeader")
ADD_INTROS_ITEM_USER_NAME(catalog, "Catalog")
END_INTROS_TYPE(MiscObjectCatalog)

VehicleCatalogPtr load_vehicle_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT {
  std::ifstream infile(_catalog_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    return std::make_shared<VehicleCatalog>(make_intros_object<VehicleCatalog>(tree));
  } else {
    LOG(WARNING) << "Load Catalog File Failure. " << _catalog_path;
    return nullptr;
  }
}

PedestrianCatalogPtr load_pedestrian_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT {
  std::ifstream infile(_catalog_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    return std::make_shared<PedestrianCatalog>(make_intros_object<PedestrianCatalog>(tree));
  } else {
    LOG(WARNING) << "Load Catalog File Failure. " << _catalog_path;
    return nullptr;
  }
}

MiscObjectCatalogPtr load_obstacle_catalog(const Base::txString& _catalog_path) TX_NOEXCEPT {
  std::ifstream infile(_catalog_path);
  if (infile) {
    std::stringstream ss;
    ss << infile.rdbuf();
    infile.close();
    ptree tree;
    Utils::FileToStreamWithStandaloneYes(ss);
    read_xml(ss, tree);
    return std::make_shared<MiscObjectCatalog>(make_intros_object<MiscObjectCatalog>(tree));
  } else {
    LOG(WARNING) << "Load Catalog File Failure. " << _catalog_path;
    return nullptr;
  }
}

TX_NAMESPACE_CLOSE(Type_Catalog)
TX_NAMESPACE_CLOSE(SceneLoader)
