/**
 * @file OsiUtil.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "osi_util.h"
#include <Eigen/Eigen>
#include <fstream>
#include <set>
#include "object_size.h"
#include "user_topic.h"
#include "common/coord_trans.h"
#include "google/protobuf/util/json_util.h"

/**
 * @brief handle dynamic object to osi3::DetectedMovingObject
 *
 * @param car ego message
 * @param mov_object moving object to be filled
 * @param ts_pt coordinate transform of position
 * @param ts_rpy coordinate transform of rotation
 * @return true on success
 * @return false on failure
 */
bool Txsim2OsiDynamicObject(const sim_msg::Location &car, std::int64_t id, osi3::DetectedMovingObject *mov_object,
                            ValueBlock age, std::function<void(double &, double &, double &)> ts_pt,
                            std::function<void(double &, double &, double &)> ts_rpy) {
  // id
  mov_object->mutable_header()->add_ground_truth_id()->set_value(getUUID(id));
  // base moving
  auto basemov = mov_object->mutable_base();
  // set size
  auto &objbb = ego_Bboxes[id];
  if (objbb.category < 0) {
    std::cout << "Cannot find ego type: " << id << std::endl;
  } else {
    auto size = basemov->mutable_dimension();
    size->set_height(objbb.hei);
    size->set_length(objbb.len);
    size->set_width(objbb.wid);
  }
  // set position
  {
    auto bpos = basemov->mutable_position();
    double x = car.position().x(), y = car.position().y(), z = car.position().z();
    ts_pt(x, y, z);
    bpos->set_x(x);
    bpos->set_y(y);
    bpos->set_z(z);
  }
  Eigen::Quaterniond qat = Eigen::AngleAxisd(car.rpy().z(), Eigen::Vector3d::UnitZ()) *
                           Eigen::AngleAxisd(car.rpy().y(), Eigen::Vector3d::UnitY()) *
                           Eigen::AngleAxisd(car.rpy().x(), Eigen::Vector3d::UnitX());

  // set rotation
  {
    auto rpy = basemov->mutable_orientation();
    double r = car.rpy().x(), p = car.rpy().y(), y = car.rpy().z();
    ts_rpy(r, p, y);
    rpy->set_roll(r);
    rpy->set_pitch(p);
    rpy->set_yaw(y);
    qat = Eigen::AngleAxisd(y, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(p, Eigen::Vector3d::UnitY()) *
          Eigen::AngleAxisd(r, Eigen::Vector3d::UnitX());
  }
  // set velocity and acceleration
  {
    auto bv = basemov->mutable_velocity();
    Eigen::Vector3d vv = qat * Eigen::Vector3d(car.velocity().x(), car.velocity().y(), car.velocity().z());
    bv->set_x(vv.x());
    bv->set_y(vv.y());
    bv->set_z(vv.z());
    age.current->values["vx"] = vv.x();
    age.current->values["vy"] = vv.y();
    age.current->values["vz"] = vv.z();
  }
  // set acceleration
  {
    auto a = basemov->mutable_acceleration();
    if (age.last_has("vx") && age.last_has("vy") && age.last_has("vz")) {
      double dt = age.current->timestamp - age.last->timestamp;
      if (dt > 1) {
        dt = 1000. / dt;
        a->set_x(dt * (age.current->values["vx"] - age.last->values.at("vx")));
        a->set_y(dt * (age.current->values["vy"] - age.last->values.at("vy")));
        a->set_z(dt * (age.current->values["vz"] - age.last->values.at("vz")));
      }
    }
  }
  // {//------
  // auto rpya = basemov->mutable_orientation_acceleration();
  // rpya->set_roll(0);
  // rpya->set_pitch(0);
  // rpya->set_yaw(0);
  // }
  // movement state
  mov_object->set_movement_state((abs(car.velocity().x()) > 0.1 || abs(car.velocity().y()) > 0.1)
                                     ? osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_MOVING
                                     : osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_STOPPED);

  // set category
  auto btype = mov_object->add_candidate();
  btype->set_probability(1);
  btype->set_type(osi3::MovingObject_Type_TYPE_VEHICLE);
  auto type = objbb.category;
  //  CATEGORY_VEHICLE_CAR = 0;
  //  CATEGORY_VEHICLE_BUS = 1;
  //  CATEGORY_VEHICLE_TRUCK = 2;
  //  CATEGORY_VEHICLE_TRALLER = 3;
  //  CATEGORY_VEHICLE_BIKE = 4;
  //  CATEGORY_VEHICLE_ELECTRIC_BIKE = 5;
  //  CATEGORY_VEHICLE_MOTORBIKE = 6;
  //  CATEGORY_VEHICLE_ELECTRIC_MOTORBIKE = 7;
  //  CATEGORY_VEHICLE_ELECTRIC_SEMITRAILER = 8;
  //  CATEGORY_VEHICLE_ELECTRIC_CRANE = 9;
  if (type == 0) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR);
  } else if (type == 2) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_HEAVY_TRUCK);
  } else if (type == 8) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_SEMITRAILER);
  } else if (type == 6) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MOTORBIKE);
  } else if (type == 4 || type == 5) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_BICYCLE);
  } else if (type == 9) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_OTHER);
  } else {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR);
  }
  return true;
}

/**
 * @brief handle dynamic object to osi3::DetectedMovingObject
 *
 * @param car car message
 * @param mov_object moving object to be filled
 * @param ts_pt coordinate transform of position
 * @param ts_rpy coordinate transform of rotation
 * @return true on success
 * @return false on failure
 */
bool Txsim2OsiDynamicObject(const sim_msg::Car &car, osi3::DetectedMovingObject *mov_object, ValueBlock age,
                            std::function<void(double &, double &, double &)> ts_pt,
                            std::function<void(double &, double &, double &)> ts_rpy) {
  // id
  mov_object->mutable_header()->add_ground_truth_id()->set_value(getUUID(car));
  // base moving
  auto basemov = mov_object->mutable_base();
  // set size
  {
    auto size = basemov->mutable_dimension();
    size->set_height(car.height());
    size->set_length(car.length());
    size->set_width(car.width());
  }
  // set position
  {
    auto bpos = basemov->mutable_position();
    double x = car.x(), y = car.y(), z = car.z();
    ts_pt(x, y, z);
    bpos->set_x(x);
    bpos->set_y(y);
    bpos->set_z(z);
  }
  Eigen::Quaterniond qat = Eigen::AngleAxisd(car.heading(), Eigen::Vector3d::UnitZ()) *
                           Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY()) *
                           Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX());

  // set rotation
  {
    auto rpy = basemov->mutable_orientation();
    double r = 0, p = 0, y = car.heading();
    ts_rpy(r, p, y);
    rpy->set_roll(r);
    rpy->set_pitch(p);
    rpy->set_yaw(y);
    qat = Eigen::AngleAxisd(y, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(p, Eigen::Vector3d::UnitY()) *
          Eigen::AngleAxisd(r, Eigen::Vector3d::UnitX());
  }
  // set velocity and acceleration
  {
    auto bv = basemov->mutable_velocity();
    auto v = car.v();
    Eigen::Vector3d vv = qat * Eigen::Vector3d(v, 0, 0);
    bv->set_x(vv.x());
    bv->set_y(vv.y());
    bv->set_z(vv.z());
    age.current->values["vx"] = vv.x();
    age.current->values["vy"] = vv.y();
    age.current->values["vz"] = vv.z();
  }
  // set acceleration
  {
    auto a = basemov->mutable_acceleration();
    if (age.last_has("vx") && age.last_has("vy") && age.last_has("vz")) {
      double dt = age.current->timestamp - age.last->timestamp;
      if (dt > 1) {
        dt = 1000. / dt;
        a->set_x(dt * (age.current->values["vx"] - age.last->values.at("vx")));
        a->set_y(dt * (age.current->values["vy"] - age.last->values.at("vy")));
        a->set_z(dt * (age.current->values["vz"] - age.last->values.at("vz")));
      }
    }
  }
  // {//------
  // auto rpya = basemov->mutable_orientation_acceleration();
  // rpya->set_roll(0);
  // rpya->set_pitch(0);
  // rpya->set_yaw(0);
  // }
  // movement state
  mov_object->set_movement_state(abs(car.v()) > 0.1 ? osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_MOVING
                                                    : osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_STOPPED);

  // (undefined = 0, Sedan = 1, SUV = 2, Bus = 3, Truck = 4, Truck_1 = 5,
  // Sedan_001 = 6, Sedan_002 = 7, Sedan_003 = 8, Sedan_004 = 9, SUV_001 = 10,
  // SUV_002 = 11, SUV_003 = 12, SUV_004 = 13, Truck_002 = 14, Truck_003 = 15,
  // Bus_001 = 16, Semi_Trailer_Truck_001 = 17, SUV_005 = 18, SUV_006 = 19,
  // Bus_004 = 20, Ambulance_001 = 21, HongqiHS5=22, Bus_003=23, Sedan_005 = 24,
  // Bus_005 = 25, Ambulance = 501, AdminVehicle_01 = 601, AdminVehicle_02 =
  // 602)
  //
  //  UNKNOWN = 0,
  // OTHER = 1,
  // SMALL_CAR = 2,
  // COMPACT_CAR = 3,紧凑型轿车
  // MEDIUM_CAR = 4,
  // LUXURY_CAR = 5,
  // DELIVERY_VAN = 6,送货车
  // HEAVY_TRUCK = 7,
  // SEMITRAILER = 8, 半挂车
  // TRAILER = 9,
  // MOTORBIKE = 10,
  // BICYCLE = 11,
  // BUS = 12,
  // TRAM = 13, 电车
  // TRAIN = 14,
  // WHEELCHAIR = 15,

  // set category
  auto btype = mov_object->add_candidate();
  btype->set_probability(1);
  btype->set_type(osi3::MovingObject_Type_TYPE_VEHICLE);
  auto type = car_Bboxes[car.type()].category;
  //  CATEGORY_VEHICLE_CAR = 0;
  //  CATEGORY_VEHICLE_BUS = 1;
  //  CATEGORY_VEHICLE_TRUCK = 2;
  //  CATEGORY_VEHICLE_TRALLER = 3;
  //  CATEGORY_VEHICLE_BIKE = 4;
  //  CATEGORY_VEHICLE_ELECTRIC_BIKE = 5;
  //  CATEGORY_VEHICLE_MOTORBIKE = 6;
  //  CATEGORY_VEHICLE_ELECTRIC_MOTORBIKE = 7;
  //  CATEGORY_VEHICLE_ELECTRIC_SEMITRAILER = 8;
  //  CATEGORY_VEHICLE_ELECTRIC_CRANE = 9;
  if (type == 0) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MEDIUM_CAR);
  } else if (type == 2) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_HEAVY_TRUCK);
  } else if (type == 8) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_SEMITRAILER);
  } else if (type == 6) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MOTORBIKE);
  } else if (type == 4 || type == 5) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_BICYCLE);
  } else if (type == 9) {
    btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_OTHER);
  } else {
    std::map<int, int> car_type_map = {{0, 0},   {1, 3},   {2, 4},   {3, 12},  {4, 6},   {5, 8},  {6, 3},  {7, 3},
                                       {8, 3},   {9, 4},   {10, 4},  {11, 4},  {12, 4},  {13, 4}, {14, 6}, {15, 6},
                                       {16, 12}, {17, 9},  {18, 4},  {19, 4},  {20, 12}, {21, 6}, {22, 5}, {23, 12},
                                       {24, 3},  {25, 12}, {501, 6}, {505, 4}, {601, 6}, {602, 6}};
    btype->mutable_vehicle_classification()->set_type(
        (osi3::MovingObject_VehicleClassification_Type)car_type_map[car.type()]);
  }
  return true;
}

/**
 * @brief handle dynamic object to osi3::DetectedMovingObject
 *
 * @param dob dynamic object
 * @param mov_object detected moving object to be filled
 * @param ts_pt coordinate transformation of position
 * @param ts_rpy coordinate transformation of rotation
 * @return true on success
 * @return false on failure
 */
bool Txsim2OsiDynamicObject(const sim_msg::DynamicObstacle &dob, osi3::DetectedMovingObject *mov_object, ValueBlock age,
                            std::function<void(double &, double &, double &)> ts_pt,
                            std::function<void(double &, double &, double &)> ts_rpy) {
  // id
  mov_object->mutable_header()->add_ground_truth_id()->set_value(getUUID(dob));
  // base moving
  auto basemov = mov_object->mutable_base();
  // set size
  {
    auto size = basemov->mutable_dimension();
    size->set_height(dob.height());
    size->set_length(dob.length());
    size->set_width(dob.width());
  }
  // set position
  {
    auto bpos = basemov->mutable_position();
    double x = dob.x(), y = dob.y(), z = dob.z();
    ts_pt(x, y, z);
    bpos->set_x(x);
    bpos->set_y(y);
    bpos->set_z(z);
  }
  Eigen::Quaterniond qat = Eigen::AngleAxisd(dob.heading(), Eigen::Vector3d::UnitZ()) *
                           Eigen::AngleAxisd(0, Eigen::Vector3d::UnitY()) *
                           Eigen::AngleAxisd(0, Eigen::Vector3d::UnitX());

  // set rotation
  {
    auto rpy = basemov->mutable_orientation();
    double r = 0, p = 0, y = dob.heading();
    ts_rpy(r, p, y);
    rpy->set_roll(r);
    rpy->set_pitch(p);
    rpy->set_yaw(y);
    qat = Eigen::AngleAxisd(y, Eigen::Vector3d::UnitZ()) * Eigen::AngleAxisd(p, Eigen::Vector3d::UnitY()) *
          Eigen::AngleAxisd(r, Eigen::Vector3d::UnitX());
  }
  // set velocity
  {
    auto bv = basemov->mutable_velocity();
    auto v = dob.v();
    Eigen::Vector3d vv = qat * Eigen::Vector3d(v, 0, 0);
    bv->set_x(vv.x());
    bv->set_y(vv.y());
    bv->set_z(vv.z());
    age.current->values["vx"] = vv.x();
    age.current->values["vy"] = vv.y();
    age.current->values["vz"] = vv.z();
  }
  // set acceleration
  {
    auto a = basemov->mutable_acceleration();
    if (age.last_has("vx") && age.last_has("vy") && age.last_has("vz")) {
      double dt = age.current->timestamp - age.last->timestamp;
      if (dt > 1) {
        dt = 1000. / dt;
        a->set_x(dt * (age.current->values["vx"] - age.last->values.at("vx")));
        a->set_y(dt * (age.current->values["vy"] - age.last->values.at("vy")));
        a->set_z(dt * (age.current->values["vz"] - age.last->values.at("vz")));
      }
    }
  }
  {  //------
    auto rpya = basemov->mutable_orientation_acceleration();
    rpya->set_roll(0);
    rpya->set_pitch(0);
    rpya->set_yaw(0);
  }
  // movement state
  mov_object->set_movement_state(abs(dob.v()) > 0.1 ? osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_MOVING
                                                    : osi3::DetectedMovingObject_MovementState_MOVEMENT_STATE_STOPPED);

  auto btype = mov_object->add_candidate();
  btype->set_probability(1);

  // set category
  auto type = dyn_Bboxes[dob.type()].category;
  //  CATEGORY_VRU_ANIMAL = 0;
  //  CATEGORY_VRU_PEDESTRIAN = 1;
  //  CATEGORY_VRU_WHEELCHAIR = 2;
  if (type == 1) {
    btype->set_type(osi3::MovingObject_Type_TYPE_PEDESTRIAN);
  } else if (type == 0) {
    btype->set_type(osi3::MovingObject_Type_TYPE_ANIMAL);
  } else {
    // 行人类型，人，动物等(human = 0, child = 1, oldman = 2, woman = 3, girl =
    // 4, cat = 100, dog = 101, bike_001 = 201, elecBike_001 = 202, tricycle_001
    // = 203, moto_001 = 301, moto_002 = 302)

    switch (dob.type()) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        btype->set_type(osi3::MovingObject_Type_TYPE_PEDESTRIAN);
        break;
      case 100:
      case 101:
        btype->set_type(osi3::MovingObject_Type_TYPE_ANIMAL);
        break;
      case 202:
      case 203:
        btype->set_type(osi3::MovingObject_Type_TYPE_VEHICLE);
        btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_BICYCLE);
      case 301:
      case 302:
        btype->set_type(osi3::MovingObject_Type_TYPE_VEHICLE);
        btype->mutable_vehicle_classification()->set_type(osi3::MovingObject_VehicleClassification_Type_TYPE_MOTORBIKE);
        break;
      default:
        btype->set_type(osi3::MovingObject_Type_TYPE_UNKNOWN);
        break;
    }
  }
  return true;
}

/**
 * @brief handle stationary object to osi3::DetectedStationaryObject
 *
 * @param sob stationary object
 * @param sta_object detected stationary object to be filled
 * @param ts_pt coordination transformation of position
 * @param ts_rpy coordination transformation of rotation
 * @return true on success
 * @return false on failure
 */
bool Txsim2OsiStationaryObject(const sim_msg::StaticObstacle &sob, osi3::DetectedStationaryObject *sta_object,
                               std::function<void(double &, double &, double &)> ts_pt,
                               std::function<void(double &, double &, double &)> ts_rpy) {
  // id
  sta_object->mutable_header()->add_ground_truth_id()->set_value(getUUID(sob));
  // base moving
  auto basemov = sta_object->mutable_base();
  // set size
  {
    auto size = basemov->mutable_dimension();
    size->set_height(sob.height());
    size->set_length(sob.length());
    size->set_width(sob.width());
  }
  // set position
  {
    auto bpos = basemov->mutable_position();
    double x = sob.x(), y = sob.y(), z = sob.z();
    ts_pt(x, y, z);
    bpos->set_x(x);
    bpos->set_y(y);
    bpos->set_z(z);
  }
  // set rotation
  {
    auto rpy = basemov->mutable_orientation();
    double r = 0, p = 0, y = sob.heading();
    ts_rpy(r, p, y);
    rpy->set_roll(r);
    rpy->set_pitch(p);
    rpy->set_yaw(y);
  }

  // 障碍物类型，沙堆，路障等(Sedan = 0, Box = 1, Person = 2, Cone = 3, Sand =
  // 4, Box_001 = 5, Stob_001 = 101, Stob_002 = 102, Stob_003 = 103, Stob_004 =
  // 104, Stob_005 = 105, Stob_006 = 106, Stob_tran_001 = 201, Trash_001 = 301,
  // Trash_002 = 302, Wheel_001 = 401, Wheel_002 = 402)

  auto btype = sta_object->add_candidate();
  btype->set_probability(1);

  // set category
  auto type = sta_Bboxes[sob.type()].category;
  // CATEGORY_MISC_BARRIER = 0;
  // CATEGORY_MISC_BUILDING = 1;
  // CATEGORY_MISC_CROSSING = 2;
  // CATEGORY_MISC_GANTRY = 3;
  // CATEGORY_MISC_OBSTACLE = 4;
  // CATEGORY_MISC_PARKING_SPACE = 5;
  // CATEGORY_MISC_PATCH = 6;
  // CATEGORY_MISC_POLE = 7;
  // CATEGORY_MISC_RAILING = 8;
  // CATEGORY_MISC_ROADMARK = 9;
  // CATEGORY_MISC_SOUND_BARRIER = 10;
  // CATEGORY_MISC_STREET_LAMP = 11;
  // CATEGORY_MISC_TRAFFIC_IS_LAND = 12;
  // CATEGORY_MISC_TREE = 13;
  // CATEGORY_MISC_VEGETATION = 14;
  // CATEGORY_MISC_NONE = 15;
  if (type == 4) {
    btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_RECTANGULAR_STRUCTURE);
  } else if (type == 0) {
    btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_BARRIER);
  } else {
    switch (sob.type()) {
      case 0:
      case 1:
      case 5:
        btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_VERTICAL_STRUCTURE);
        break;
      case 2:
        btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_VERTICAL_STRUCTURE);
        break;
      case 3:
      case 4:
        btype->mutable_classification()->set_type(
            osi3::StationaryObject_Classification_Type_TYPE_CONSTRUCTION_SITE_ELEMENT);
        break;
      case 101:
      case 102:
      case 103:
      case 104:
      case 105:
      case 106:
        btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_DELINEATOR);
        break;
      case 201:
      case 301:
      case 302:
        btype->mutable_classification()->set_type(
            osi3::StationaryObject_Classification_Type_TYPE_CONSTRUCTION_SITE_ELEMENT);
        break;
      case 401:
      case 402:
        btype->mutable_classification()->set_type(
            osi3::StationaryObject_Classification_Type_TYPE_CONSTRUCTION_SITE_ELEMENT);
        break;
      default:
        btype->mutable_classification()->set_type(osi3::StationaryObject_Classification_Type_TYPE_OTHER);
        break;
    }
  }
  return true;
}

/**
 * @brief handle traffic light object to osi3::DetectedTrafficLight
 *
 * @param lig traffic light object
 * @param lig_object detected traffic light object
 * @param ts_pt coordinate transformation of position
 * @param ts_rpy coordinate transformation of rotation
 * @return true on success
 * @return false on failure
 */
bool Txsim2OsiTrafficLightObject(const sim_msg::TrafficLight &lig, osi3::DetectedTrafficLight *lig_object,
                                 std::function<void(double &, double &, double &)> ts_pt,
                                 std::function<void(double &, double &, double &)> ts_rpy) {
  // id
  lig_object->mutable_header()->add_ground_truth_id()->set_value(lig.id());
  // base moving
  auto basemov = lig_object->mutable_base();
  // set position
  {
    auto bpos = basemov->mutable_position();
    double x = lig.x(), y = lig.y(), z = lig.z();
    ts_pt(x, y, z);
    bpos->set_x(x);
    bpos->set_y(y);
    bpos->set_z(z);
  }
  // set rotation
  {
    auto rpy = basemov->mutable_orientation();
    double r = 0, p = 0, y = lig.heading();
    ts_rpy(r, p, y);
    rpy->set_roll(r);
    rpy->set_pitch(p);
    rpy->set_yaw(y);
  }

  // set classification
  auto btype = lig_object->add_candidate();
  btype->set_probability(1);
  auto color = lig.color();
  switch (color) {
    case 0:
      btype->mutable_classification()->set_color(osi3::TrafficLight::Classification::COLOR_GREEN);
      break;
    case 1:
      btype->mutable_classification()->set_color(osi3::TrafficLight::Classification::COLOR_YELLOW);
      break;
    case 2:
      btype->mutable_classification()->set_color(osi3::TrafficLight::Classification::COLOR_RED);
      break;
    case 3:
      btype->mutable_classification()->set_color(osi3::TrafficLight::Classification::COLOR_OTHER);
      break;
    case 4:
      btype->mutable_classification()->set_color(osi3::TrafficLight::Classification::COLOR_UNKNOWN);
      break;
    default:
      break;
  }
  btype->mutable_classification()->set_counter(lig.age());
  for (const auto &l : lig.control_lanes()) {
    btype->mutable_classification()->add_assigned_lane_id()->set_value(l.tx_road_id());
    btype->mutable_classification()->add_assigned_lane_id()->set_value(l.tx_section_id());
    btype->mutable_classification()->add_assigned_lane_id()->set_value(l.tx_lane_id());
  }
  return true;
}

/**
 * @brief add lidar data
 *
 * @return lidar detection data
 */
osi3::LidarDetectionData *OsiSensorData::AddLidar() { return sensordata.mutable_feature_data()->add_lidar_sensor(); }

/**
 * @brief add camera data
 *
 * @return camera detection data
 */
osi3::CameraDetectionData *OsiSensorData::AddCamera() { return sensordata.mutable_feature_data()->add_camera_sensor(); }

/**
 * @brief add ultrasonic data
 *
 * @return ultrasonic detection data
 */
osi3::UltrasonicDetectionData *OsiSensorData::AddUltrasonic() {
  return sensordata.mutable_feature_data()->add_ultrasonic_sensor();
}

/**
 * @brief osi init
 *
 * @param helper helper of txsim
 */
void OsiSensorData::Init(tx_sim::InitHelper &helper) {
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Publish(std::string(tx_sim::user_topic::kSensorObject) + (device.empty() ? "" : "_") + device);

  noise.initDrop(helper.GetParameter("NoiseDrop"));
  noise.initStay(helper.GetParameter("NoiseStay"));
  noise.initPosition(helper.GetParameter("NoisePosition"));
  noise.initSize(helper.GetParameter("NoiseSize"));
  noise.initVeolity(helper.GetParameter("NoiseVeolity"));
}

/**
 * @brief osi reset
 *
 * @param helper helper of txsim
 */
void OsiSensorData::Reset(tx_sim::ResetHelper &helper) {
  ages[0] = Age();
  ages[1] = Age();
  ages[2] = Age();
}

/**
 * @brief osi step
 *
 * @param helper helper of txsim
 */
void OsiSensorData::Step(tx_sim::StepHelper &helper) {
  // get location message
  std::string payload_;
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload_);
  sim_msg::Location loc;
  loc.ParseFromString(payload_);
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);
  // set car pose of fov sensor
  fov.setCarPosition(locPosition);
  fov.setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());

  // set timestamp
  double time_stamp = loc.t() * 1000;

  // set timestamp
  sensordata.mutable_timestamp()->set_seconds(time_stamp * 0.001);
  sensordata.mutable_timestamp()->set_nanos(std::fmod(time_stamp, 1000) * 1000000);
  sensordata.mutable_sensor_id()->set_value(device.empty() ? 99999 : std::atoi(device.c_str()));

  // set vechicle info: position, rotation, velocity, acceleration
  sensordata.mutable_host_vehicle_location()->mutable_position()->set_x(locPosition.x());
  sensordata.mutable_host_vehicle_location()->mutable_position()->set_y(locPosition.y());
  sensordata.mutable_host_vehicle_location()->mutable_position()->set_z(locPosition.z());
  sensordata.mutable_host_vehicle_location()->mutable_orientation()->set_roll(loc.rpy().x());
  sensordata.mutable_host_vehicle_location()->mutable_orientation()->set_pitch(loc.rpy().y());
  sensordata.mutable_host_vehicle_location()->mutable_orientation()->set_yaw(loc.rpy().z());
  sensordata.mutable_host_vehicle_location()->mutable_velocity()->set_x(loc.velocity().x());
  sensordata.mutable_host_vehicle_location()->mutable_velocity()->set_y(loc.velocity().y());
  sensordata.mutable_host_vehicle_location()->mutable_velocity()->set_z(loc.velocity().z());
  sensordata.mutable_host_vehicle_location()->mutable_acceleration()->set_x(loc.acceleration().x());
  sensordata.mutable_host_vehicle_location()->mutable_acceleration()->set_y(loc.acceleration().y());
  sensordata.mutable_host_vehicle_location()->mutable_acceleration()->set_z(loc.acceleration().z());

  // update ages
  ages[0].NewYear(time_stamp);
  ages[1].NewYear(time_stamp);
  ages[2].NewYear(time_stamp);
  ages[3].NewYear(time_stamp);
  // handle ego
  for (const auto &car : egoBuf) {
    if (car.first == ego_id) continue;
    auto mov_object = sensordata.add_moving_object();
    // car to moving object
    Txsim2OsiDynamicObject(
        car.second, car.first, mov_object, ages[3].get_value(car.first),
        [&](double &x, double &y, double &z) {
          coord_trans_api::lonlat2enu(x, y, z, map_ori.x, map_ori.y, map_ori.z);
          auto &objbb = ego_Bboxes[car.first];
          Eigen::Vector3d p = fov.FovVector(objbb.getBboxCen(Eigen::Vector3d(x, y, z), car.second.rpy().z()));
          x = p.x();
          y = p.y();
          z = p.z();
        },
        [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
    int age = ages[0](car.first);
    mov_object->mutable_header()->set_age(age);
    // add moveing noise
    if (!MovingNoise(mov_object, time_stamp, noise)) {
      if (sensordata.moving_object_size() > 0) {
        sensordata.mutable_moving_object()->RemoveLast();
      }
    }
  }
  // handle car
  for (const auto &car : carBuf) {
    auto mov_object = sensordata.add_moving_object();
    // car to moving object
    Txsim2OsiDynamicObject(
        car.second, mov_object, ages[0].get_value(car.first),
        [&](double &x, double &y, double &z) {
          coord_trans_api::lonlat2enu(x, y, z, map_ori.x, map_ori.y, map_ori.z);
          auto &objbb = car_Bboxes[car.second.type()];
          Eigen::Vector3d p = fov.FovVector(objbb.getBboxCen(Eigen::Vector3d(x, y, z), car.second.heading()));
          x = p.x();
          y = p.y();
          z = p.z();
        },
        [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
    int age = ages[0](car.first);
    mov_object->mutable_header()->set_age(age);
    // add moveing noise
    if (!MovingNoise(mov_object, time_stamp, noise)) {
      if (sensordata.moving_object_size() > 0) {
        sensordata.mutable_moving_object()->RemoveLast();
      }
    }
  }
  // handle dynamic object
  for (const auto &dyn : dynBuf) {
    auto mov_object = sensordata.add_moving_object();
    // dynamic object to moving object
    Txsim2OsiDynamicObject(
        dyn.second, mov_object, ages[1].get_value(dyn.first),
        [&](double &x, double &y, double &z) {
          coord_trans_api::lonlat2enu(x, y, z, map_ori.x, map_ori.y, map_ori.z);
          auto &objbb = car_Bboxes[dyn.second.type()];
          Eigen::Vector3d p = fov.FovVector(objbb.getBboxCen(Eigen::Vector3d(x, y, z), dyn.second.heading()));
          x = p.x();
          y = p.y();
          z = p.z();
        },
        [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
    int age = ages[1](dyn.first);
    mov_object->mutable_header()->set_age(age);
    // add moveing noise
    if (!MovingNoise(mov_object, time_stamp, noise)) {
      if (sensordata.moving_object_size() > 0) {
        sensordata.mutable_moving_object()->RemoveLast();
      }
    }
  }
  // handle stationary objects
  for (const auto &sta : staBuf) {
    auto mov_object = sensordata.add_stationary_object();
    // stationary object to stationary object
    Txsim2OsiStationaryObject(
        sta.second, mov_object,
        [&](double &x, double &y, double &z) {
          coord_trans_api::lonlat2enu(x, y, z, map_ori.x, map_ori.y, map_ori.z);
          auto &objbb = car_Bboxes[sta.second.type()];
          Eigen::Vector3d p = fov.FovVector(objbb.getBboxCen(Eigen::Vector3d(x, y, z), sta.second.heading()));
          x = p.x();
          y = p.y();
          z = p.z();
        },
        [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
    int age = ages[2](sta.first);
    mov_object->mutable_header()->set_age(age);
    // add stationay noise
    if (!StationaryNoise(mov_object, time_stamp, noise)) {
      if (sensordata.moving_object_size() > 0) {
        sensordata.mutable_moving_object()->RemoveLast();
      }
    }
  }

  // handle traffic light
  for (const auto &light : ligBuf) {
    auto lig_object = sensordata.add_traffic_light();
    // traffic light to traffic light
    Txsim2OsiTrafficLightObject(
        light.second, lig_object,
        [&](double &x, double &y, double &z) {
          coord_trans_api::lonlat2enu(x, y, z, map_ori.x, map_ori.y, map_ori.z);
          Eigen::Vector3d p = fov.FovVector(Eigen::Vector3d(light.second.x(), light.second.y(), light.second.z()));
          x = p.x();
          y = p.y();
          z = p.z();
        },
        [&](double &r, double &p, double &y) { fov.FovRotator(r, p, y); });
  }

  // publish message
  std::string buf;
  sensordata.SerializeToString(&buf);
  helper.PublishMessage(std::string(tx_sim::user_topic::kSensorObject) + (device.empty() ? "" : "_") + device, buf);

  // debug print
  if (!debugDir.empty()) {
    std::ofstream debug(debugDir + "/osi_truth_" + std::to_string(helper.timestamp()) + ".json");
    if (debug.good()) {
      std::string json;
      google::protobuf::util::MessageToJsonString(sensordata, &json);
      debug << json;
    }
  }

  sensordata = osi3::SensorData();
  carBuf.clear();
  dynBuf.clear();
  staBuf.clear();
  // update ages
  ages[0].YearEnd();
  ages[1].YearEnd();
  ages[2].YearEnd();
}

/**
 * @brief add car to osi
 *
 * @param car car of message
 */
void OsiSensorData::Add(const sim_msg::Car &car) {
  auto id = getUUID(car);
  if (carBuf.find(id) != carBuf.end()) return;
  carBuf.insert(std::make_pair(id, car));
}

/**
 * @brief add ego car to osi
 *
 * @param car location
 * @param id id
 */
void OsiSensorData::Add(const sim_msg::Location &car, std::int64_t id) {
  if (carBuf.find(id) != carBuf.end()) return;
  egoBuf.insert(std::make_pair(id, car));
}

/**
 * @brief add dynamic object to osi
 *
 * @param dob dynamic obstacle of message
 */
void OsiSensorData::Add(const sim_msg::DynamicObstacle &dob) {
  auto id = getUUID(dob);
  if (dynBuf.find(id) != dynBuf.end()) return;
  dynBuf.insert(std::make_pair(id, dob));
}

/**
 * @brief add static object to osi
 *
 * @param sob static obstacle of message
 */
void OsiSensorData::Add(const sim_msg::StaticObstacle &sob) {
  auto id = getUUID(sob);
  if (staBuf.find(id) != staBuf.end()) return;
  staBuf.insert(std::make_pair(id, sob));
}

/**
 * @brief add traffic light to osi
 *
 * @param lig traffic light of message
 */
void OsiSensorData::Add(const sim_msg::TrafficLight &lig) {
  auto id = lig.id();
  if (staBuf.find(id) != staBuf.end()) return;
  ligBuf.insert(std::make_pair(id, lig));
}

/**
 * @brief
 *
 * @param ego_id
 * @return int64_t
 */
int64_t getUUID(std::int64_t ego_id) {
  assert(ego_id >= 0);
  return STA_ID0 + ego_id;
}
/**
 * @brief get UUID of car
 *
 * @param obj car of message
 * @return uuid of car
 */
int64_t getUUID(const sim_msg::Car &obj) {
  assert(obj.id() >= 0);
  return obj.id();
}

/**
 * @brief get UUID of dynamic obstacle
 *
 * @param obj dynamic obstacle of message
 * @return uuid of dynamic obstacle
 */
int64_t getUUID(const sim_msg::DynamicObstacle &obj) {
  assert(obj.id() <= 0);
  return DYN_ID0 - obj.id();
}

/**
 * @brief get UUID of static obstacle
 *
 * @param obj static obstacle of message
 * @return uuid of static obstacle
 */
int64_t getUUID(const sim_msg::StaticObstacle &obj) {
  assert(obj.id() >= 0);
  return STA_ID0 + obj.id();
}

/**
 * @brief add movement object noise
 *
 * @param mov_object detected moving object
 * @param tstamp current timestamp
 * @param noise noise model
 * @return true if noise is added
 * @return false if the object had to drop
 */
bool MovingNoise(osi3::DetectedMovingObject *mov_object, double tstamp, Noise &noise) {
  if (!noise.good()) return true;
  // read object info
  Noise::Data data;
  data.id = mov_object->header().ground_truth_id()[0].value();
  data.time = tstamp;
  data.x = mov_object->base().position().x();
  data.y = mov_object->base().position().y();
  data.z = mov_object->base().position().z();
  data.hei = mov_object->base().dimension().height();
  data.wid = mov_object->base().dimension().width();
  data.len = mov_object->base().dimension().length();
  data.vx = mov_object->base().velocity().x();
  data.vy = mov_object->base().velocity().y();
  // data.vz = mov_object->base().velocity().z();
  //  this object must drop
  if (!noise.noise(data)) {
    return false;
  }
  // update object info
  mov_object->mutable_base()->mutable_position()->set_x(data.x);
  mov_object->mutable_base()->mutable_position()->set_y(data.y);
  mov_object->mutable_base()->mutable_position()->set_z(data.z);
  mov_object->mutable_base()->mutable_dimension()->set_height(data.hei);
  mov_object->mutable_base()->mutable_dimension()->set_width(data.wid);
  mov_object->mutable_base()->mutable_dimension()->set_length(data.len);
  mov_object->mutable_base()->mutable_velocity()->set_x(data.vx);
  mov_object->mutable_base()->mutable_velocity()->set_y(data.vy);
  // mov_object->mutable_base()->mutable_velocity()->set_z(data.vz);

  return true;
}

/**
 * @brief add stationary object noise
 *
 * @param sta_object detected stationary object
 * @param tstamp current timestamp
 * @param noise noise model
 * @return true if noise is added
 * @return false if the object had to drop
 */
bool StationaryNoise(osi3::DetectedStationaryObject *sta_object, double tstamp, Noise &noise) {
  if (!noise.good()) return true;
  // read object info
  Noise::Data data;
  data.id = sta_object->header().ground_truth_id()[0].value();
  data.time = tstamp;
  data.x = sta_object->base().position().x();
  data.y = sta_object->base().position().y();
  data.z = sta_object->base().position().z();
  data.hei = sta_object->base().dimension().height();
  data.wid = sta_object->base().dimension().width();
  data.len = sta_object->base().dimension().length();
  if (!noise.noise(data)) {
    return false;
  }
  // update object info
  sta_object->mutable_base()->mutable_position()->set_x(data.x);
  sta_object->mutable_base()->mutable_position()->set_y(data.y);
  sta_object->mutable_base()->mutable_position()->set_z(data.z);
  sta_object->mutable_base()->mutable_dimension()->set_height(data.hei);
  sta_object->mutable_base()->mutable_dimension()->set_width(data.wid);
  sta_object->mutable_base()->mutable_dimension()->set_length(data.len);
  return true;
}
