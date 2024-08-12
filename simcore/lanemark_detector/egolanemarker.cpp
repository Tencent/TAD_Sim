// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "egolanemarker.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <thread>

#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "glog/logging.h"

#include "common/coord_trans.h"
#include "common/map_util.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_predef.h"
#include "txsim_messenger.h"
#include "types/map_defs.h"

#define PTINTINFORMATION 0

TXSIM_MODULE(EgoLaneMarker)

// slightly less than M_PI constant(3.14159265358979323846)
static const double kNearPi = 3.1415926535;
static const double kPIx2 = M_PI * 2;
static const char kEgoTrafficTopicName[] = "EGO_TRAFFIC";

static hadmap::MAP_DATA_TYPE GetMapType(const std::string& map_path) {
  boost::filesystem::path p(map_path);
  std::string ext = p.extension().string();
  boost::to_lower(ext);
  if (ext == ".sqlite") return hadmap::SQLITE;
  if (ext == ".xodr") return hadmap::OPENDRIVE;
  LOG(ERROR) << "unknown map file type(" << ext << "): " << map_path;
  throw std::runtime_error(std::string("unknown map file type: ") + map_path);
}

static int32_t GetLaneMarkType(hadmap::LANE_MARK lmt, bool left = true, int id = 0) {
  // LOG(INFO)<<"lane type "<<lmt<<"\n";

  if (PTINTINFORMATION) std::cout << "lane " << (left ? "left " : "right ") << " id : " << id << " ";
  /*
    LANE_MARK_None         = 0,
    LANE_MARK_Solid        = 0x1,
    LANE_MARK_Broken       = 0x2,
    LANE_MARK_Solid2       = 0x10,
    LANE_MARK_Broken2      = 0x20,
    LANE_MARK_SolidSolid   = LANE_MARK_Solid | LANE_MARK_Solid2,   // (for double solid line) 0x3
    LANE_MARK_SolidBroken  = LANE_MARK_Solid | LANE_MARK_Broken2,  // (from inside to outside, exception:
                                                                   // center lane - from left to right)
    LANE_MARK_BrokenSolid  = LANE_MARK_Broken | LANE_MARK_Solid2,  // (from inside to outside, exception:
                                                                   // center lane - from left to right)
    LANE_MARK_BrokenBroken = LANE_MARK_Broken | LANE_MARK_Broken2,

    LANE_MARK_Curb         = 0x100,
    LANE_MARK_Fence        = 0x200,
    LANE_MARK_RoadEdge     = 0x400,
    LANE_MARK_Bold         = 0x1000,   // bold lines
    LANE_MARK_Yellow       = 0x10000,  // yellow lines, default white,blue green red
    LANE_MARK_Yellow2      = 0x20000,
    LANE_MARK_YellowYellow = 0x30000,
    LANE_MARK_OtherColors  = 0x40000,
    LANE_MARK_Red          = 0x50000,
    LANE_MARK_Green        = 0x60000,
    LANE_MARK_Blue         = 0x70000,
    LANE_MARK_LDM          = 0x100000,
    LANE_MARK_VGL          = 0x200000,

    LANE_MARK_Shielded     = 0x1000000,  // virture mark
    LANE_MARK_Absent       = 0x2000000   // virture mark
  */
  int color = lmt & 0x000f0000;
  int type = lmt & 0xff;
  switch (type) {
    case hadmap::LANE_MARK_None:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_None"
                  << "\n";
      return -1;
      break;
    case hadmap::LANE_MARK_Solid:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Solid"
                  << "\n";
      return hadmap::LANE_MARK_Solid;
      break;
    case hadmap::LANE_MARK_Broken:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Broken"
                  << "\n";
      return hadmap::LANE_MARK_Broken;
      break;
    case hadmap::LANE_MARK_Solid2:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Solid2"
                  << "\n";
      return hadmap::LANE_MARK_Solid2;
      break;
    case hadmap::LANE_MARK_Broken2:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Broken2"
                  << "\n";
      return hadmap::LANE_MARK_Broken2;
      break;
    case hadmap::LANE_MARK_SolidSolid:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_SolidSolid"
                  << "\n";
      return hadmap::LANE_MARK_SolidSolid;
      break;
    case hadmap::LANE_MARK_SolidBroken:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_SolidBroken"
                  << "\n";
      return hadmap::LANE_MARK_SolidBroken;
      break;
    case hadmap::LANE_MARK_BrokenSolid:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_BrokenSolid"
                  << "\n";
      return hadmap::LANE_MARK_BrokenSolid;
    case hadmap::LANE_MARK_BrokenBroken:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_BrokenBroken"
                  << "\n";
      return hadmap::LANE_MARK_BrokenBroken;
      break;
    case hadmap::LANE_MARK_Curb: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Curb"
                  << "\n";
      return hadmap::LANE_MARK_Curb;
      break;
    }  // = 0x100,
    case hadmap::LANE_MARK_Fence: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Fence"
                  << "\n";
      return hadmap::LANE_MARK_Fence;
      break;
    }  // = 0x200,
    case hadmap::LANE_MARK_RoadEdge: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_RoadEdge"
                  << "\n";
      return hadmap::LANE_MARK_RoadEdge;
      break;
    }  // = 0x400,
    case hadmap::LANE_MARK_Bold: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Bold"
                  << "\n";
      return hadmap::LANE_MARK_Bold;
      break;
    }  // = 0x1000,  // bold lines
    case hadmap::LANE_MARK_Yellow: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Yellow"
                  << "\n";
      return hadmap::LANE_MARK_Yellow;
      break;
    }  // = 0x10000,  // yellow lines, default white,blue green red
    case hadmap::LANE_MARK_Yellow2: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Yellow2"
                  << "\n";
      return hadmap::LANE_MARK_Yellow2;
      break;
    }  // = 0x20000,
    case hadmap::LANE_MARK_YellowYellow: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_YellowYellow"
                  << "\n";
      return hadmap::LANE_MARK_YellowYellow;
      break;
    }  // = 0x30000,
    case hadmap::LANE_MARK_OtherColors: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_OtherColors"
                  << "\n";
      return hadmap::LANE_MARK_OtherColors;
      break;
    }  // = 0x40000,
    case hadmap::LANE_MARK_Red: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Red"
                  << "\n";
      return hadmap::LANE_MARK_Red;
      break;
    }  // = 0x50000,
    case hadmap::LANE_MARK_Green: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Green"
                  << "\n";
      return hadmap::LANE_MARK_Green;
      break;
    }  // = 0x60000,
    case hadmap::LANE_MARK_Blue: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Blue"
                  << "\n";
      return hadmap::LANE_MARK_Blue;
      break;
    }  // = 0x70000,
    case hadmap::LANE_MARK_LDM: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_LDM"
                  << "\n";
      return hadmap::LANE_MARK_LDM;
      break;
    }  // = 0x100000,
    case hadmap::LANE_MARK_VGL: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_VGL"
                  << "\n";
      return hadmap::LANE_MARK_VGL;
      break;
    }  // = 0x200000,
    case hadmap::LANE_MARK_Shielded: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Shielded"
                  << "\n";
      return hadmap::LANE_MARK_Shielded;
      break;
    }  // = 0x200000,
    case hadmap::LANE_MARK_Absent: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "LANE_MARK_Absent"
                  << "\n";
      return hadmap::LANE_MARK_Absent;
      break;
    }  // = 0x200000,
    case 0x10002: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "Other unknown as broken " << lmt << "\n";
      return 65538;
      break;
    }  // 65538
    case 0x10001: {
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "Other unknown as solid " << lmt << "\n";
      return 65537;
      break;
    }  // 65537
    default:
      if (PTINTINFORMATION)
        std::cout << "lane type: "
                  << "Other unknown " << lmt << "\n";
      return -1;
      break;
  }
}

// return shortest distance between point(x, y) and line(1->2), positive when point is on the left side of the line.
// https://math.stackexchange.com/questions/274712/calculate-on-which-side-of-a-straight-line-is-a-given-point-located
static float SignedDistance2Line(double x1, double y1, double x2, double y2, double x, double y) {
  return ((y - y1) * (x2 - x1) - (x - x1) * (y2 - y1)) / std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
}

static void CalculateDistance2LaneLine(sim_msg::LaneMarks& lane_marks, const hadmap::txLanePtr& cur_lane,
                                       const hadmap::txPoint& loc, double passed_dis, const Eigen::Matrix3d& rotMtx) {
  if (lane_marks.left_size() > 0 && lane_marks.left(0).points_size() > 1) {
    const sim_msg::Vec3 &p1 = lane_marks.left(0).points(0), &p2 = lane_marks.left(0).points(1);
    // LOG(INFO) << "==> left 0 -> (" << p1.x() << ", " << p1.y() << "), 1 -> (" << p2.x() << ", " << p2.y() << ").";
    lane_marks.set_dis_to_left_boundary(-SignedDistance2Line(p1.x(), p1.y(), p2.x(), p2.y(), 0, 0));
  }
  if (lane_marks.right_size() > 0 && lane_marks.right(0).points_size() > 1) {
    const sim_msg::Vec3 &p1 = lane_marks.right(0).points(0), &p2 = lane_marks.right(0).points(1);
    // LOG(INFO) << "==> right 0 -> (" << p1.x() << ", " << p1.y() << "), 1 -> (" << p2.x() << ", " << p2.y() << ").";
    lane_marks.set_dis_to_right_boundary(SignedDistance2Line(p1.x(), p1.y(), p2.x(), p2.y(), 0, 0));
  }
  const auto& geo = cur_lane->getGeometry();
  if (!geo->empty()) {
    hadmap::txPoint p1 = geo->getPoint(passed_dis), p2 = geo->getPoint(passed_dis + 1);
    if (p1 == p2) {
      p2 = p1;
      p1 = geo->getPoint(passed_dis - 1);
    }
    if (p1 == p2) {
      // LOG(WARNING) << "current ego lane point is too close. distance to the center line not computed.";
      return;
    }
    coord_trans_api::lonlat2enu(p1.x, p1.y, p1.z, loc.x, loc.y, loc.z);
    Eigen::Vector3d r1 = rotMtx.inverse() * Eigen::Vector3d(p1.x, p1.y, p1.z);
    coord_trans_api::lonlat2enu(p2.x, p2.y, p2.z, loc.x, loc.y, loc.z);
    Eigen::Vector3d r2 = rotMtx.inverse() * Eigen::Vector3d(p2.x, p2.y, p2.z);
    // LOG(INFO) << "==> center 0 -> (" << r1.x() << ", " << r2.y() << "), 1 -> (" << r2.x() << ", " << r2.y() << ").";
    lane_marks.set_dis_to_center_line(SignedDistance2Line(r1.x(), r1.y(), r2.x(), r2.y(), 0, 0));
  }
}

static size_t CountTrafficInEgoPerceptionRange(const sim_msg::Traffic& traffic, double range) {
  double range_pow_2 = range * range;
  size_t count = 0;
  for (const auto& obj : traffic.cars())
    if (std::pow(obj.x(), 2) + std::pow(obj.y(), 2) < range_pow_2) ++count;
  return count;
}

EgoLaneMarker::EgoLaneMarker() { LOG(INFO) << "using hadmap lib version: " << hadmap::version() << std::endl; }

EgoLaneMarker::~EgoLaneMarker() {
  resetMapHandle();
  LOG(INFO) << "ego lane detector destructed.";
}

void EgoLaneMarker::Init(tx_sim::InitHelper& helper) {
  try {
    this->getInitialParameters(helper);
  } catch (const std::exception& e) {
    LOG(ERROR) << "init parameter parsing error: " << e.what();
    throw std::invalid_argument(std::string("init parameter parsing error: ") + e.what());
  }
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Publish(tx_sim::topic::kLaneMark);
  helper.Publish(kEgoTrafficTopicName);

  helper.Publish("OSI_LANEMARKS");
  for (size_t i = 0; i < detectingLaneMarkCount * 2; ++i) {
    std::thread t(&EgoLaneMarker::doTransformingWork, this, i);
    t.detach();
  }
}

void EgoLaneMarker::Reset(tx_sim::ResetHelper& helper) {
  LOG(INFO) << "resetting to " << helper.scenario_file_path() << " ...";

  this->frameCount = 0;
  this->cachedBoundaries.clear();
  this->cachedLeftLanes.clear();
  this->cachedRightLanes.clear();
  this->cachedNextLanes.clear();
  this->cachedPrevLanes.clear();
  this->roadEndLaneUniqIdVector.clear();
  this->connectToJunctionLaneIdVect.clear();

  this->lastEgoLaneId.roadId = this->lastEgoLaneId.sectionId = this->lastEgoLaneId.laneId = 0;
  this->lastEgoLaneNo = 0;
  this->lastEgoSection.reset();  // release resource and convert to empty shared_ptr object
  this->laneKeepMs = 0;

  resetMapHandle();
  LOG(INFO) << "opening map: " << helper.map_file_path();
  int ret = hadmap::hadmapConnect(helper.map_file_path().c_str(), GetMapType(helper.map_file_path()), &this->mapHandle);
  std::cout << "hadmapConnect ret: " << ret << std::endl;
  this->checkHandleCode(ret);  // fileInfo is dbPath
  LOG(INFO) << "map opened successfully.";

  leftValidatIndex = 0;
  rightValidatIndex = 0;
}

void EgoLaneMarker::Stop(tx_sim::StopHelper& helper) { std::cout << "stop callback" << std::endl; }

void EgoLaneMarker::Step(tx_sim::StepHelper& helper) {
  ++this->frameCount;
  VLOG(2) << "step @ timestamp " << helper.timestamp() << ", frame id " << this->frameCount;
  sim_msg::LaneMarks laneMarks;
  for (size_t i = 0; i < this->detectingLaneMarkCount; i++) {
    laneMarks.add_left();
    laneMarks.add_right();
  }

  // 1. set header.
  sim_msg::Header& hdr = *laneMarks.mutable_header();
  hdr.set_time_stamp(helper.timestamp());
  hdr.set_frame_id(this->frameCount);
  hdr.set_sequence(this->frameCount);

  // 2. get ego car location in lon-lat, theta.
  sim_msg::Location loc;
  helper.GetSubscribedMessage(tx_sim::topic::kLocation, this->payload);
  loc.ParseFromString(this->payload);
  hadmap::txPoint locPoint(loc.position().x(), loc.position().y(), loc.position().z()), pedal;  // pedal是垂点
  sim_msg::Vec3 rpy = loc.rpy();
  VLOG(2) << std::setprecision(12) << "the location of ego car received: (" << locPoint.x << ", " << locPoint.y << ", "
          << locPoint.z << ") with theta " << rpy.z();

  hadmap::txLanePtr lane;
  this->checkHandleCode(hadmap::getLane(this->mapHandle, locPoint, lane));  // 根据主车位置获取车道线存入 lane
  if (!lane) {
    LOG(INFO) << "lane is empty";
    return;
  }

  hadmap::txLanePtr leftLane = fetchLeftLane(lane);
  if (leftLane && isLaneDrivable(leftLane)) {
    laneMarks.set_left_lane_id(leftLane->getId());
  }

  hadmap::txLanePtr rightLane = fetchRightLane(lane);
  if (rightLane && isLaneDrivable(rightLane)) {
    laneMarks.set_right_lane_id(rightLane->getId());
  }
  double distanceAlongCurve = 0;
  if (lane && isLaneDrivable(lane)) {
    LOG(INFO) << " host lane id " << lane->getUniqueId() << std::endl;

    getEgoLaneArrow(laneMarks, lane->getLaneArrow());
    getTwoSideLaneMarksNumber(lane, leftLaneNum, rightLaneNum);

    laneMarks.set_left_lanemark_num(leftLaneNum + 1);
    laneMarks.set_right_lanemark_num(rightLaneNum + 1);
    LOG(INFO) << "left_lanemark_num:" << laneMarks.left_lanemark_num();
    LOG(INFO) << "right_lanemark_num:" << laneMarks.right_lanemark_num();

    countEgoLaneKeepTime(lane);  // 获取主车在车道上滞留的时间
    laneMarks.set_lane_keep_duration(laneKeepMs / 1000.0);
    laneMarks.set_lane_no(this->lastEgoLaneNo);
    // 3. get the passed distance of the lane on where the ego car is.
    distanceAlongCurve = getDistanceAlongLane(lane, locPoint, pedal);  // 计算车辆距离车道线起点的距离
    laneMarks.set_begin_lanemark_to_ego(distanceAlongCurve);
    laneMarks.set_end_lanemark_to_ego(lane->getLength() - distanceAlongCurve);

    double s = 0.0;
    double l = 0.0;
    double laneYaw = 0.0;
    VLOG(2) << "xy3sl"
            << " loc.position().x():" << loc.position().x() << ",  loc.position().y():" << loc.position().y()
            << ", s:" << s << ", l:" << l << ", laneYaw:" << laneYaw;
    lane->xy2sl(loc.position().x(), loc.position().y(), s, l, laneYaw);
    double egoYaw = loc.rpy().z();
    double yaw = egoYaw - laneYaw - kNearPi;
    laneMarks.set_ego_direction_with_lane(yaw);

    laneMarks.set_dis_to_center_line(map_util::distanceBetweenPoints(
        hadmap::Point3d(locPoint.x, locPoint.y, locPoint.z), hadmap::Point3d(pedal.x, pedal.y, pedal.z),
        true));  // 计算车辆中心到车道线中点的距离？ 直线距离？

    // 4. get lane marks in lon-lat first, according to the "forwardLength" and "laneDetectingCount" parameters.
    if (PTINTINFORMATION) std::cout << "\n\nbegin retriveLineData forward\n";
    retriveLineData(laneMarks, lane, distanceAlongCurve, true, this->forwardLength);  // true 向前 false - 向后
    if (PTINTINFORMATION) std::cout << "end retriveLineData forward\n";
    retriveLineData(laneMarks, lane, distanceAlongCurve, false, 100);
    if (PTINTINFORMATION) std::cout << "end retriveLineData backward\n\n";
    laneMarks.set_remain_length(getRemainLengthToNextJunction(
        lane, distanceAlongCurve));  // 车道线总长度减去车辆距离起点的长度？ 也就是剩余的长度？

    for (auto& middleLineItem : m_listMiddlePoint) {
      sim_msg::Vec3* pPoint = laneMarks.add_middle_points();
      pPoint->set_x(std::get<0>(middleLineItem));
      pPoint->set_y(std::get<1>(middleLineItem));
      pPoint->set_z(std::get<2>(middleLineItem));
    }
    m_listMiddlePoint.clear();
    laneMarks.set_lane_id(lane->getId());
    laneMarks.set_road_id(lane->getRoadId());
    laneMarks.set_section_id(lane->getSectionId());
  } else {
    LOG(WARNING) << "no lane found near the ego car.";
  }

  sim_msg::Traffic traffic;
  helper.GetSubscribedMessage(tx_sim::topic::kTraffic, this->payload);
  traffic.ParseFromString(this->payload);

  transformCoordinates(laneMarks, locPoint, lane, rpy, traffic, distanceAlongCurve);
  laneMarks.set_car_count_in_perception_range(CountTrafficInEgoPerceptionRange(
      traffic, this->forwardLength));  // 计算this->forwardLength半径范围内的交通车数目？

  VLOG(1) << std::fixed << std::setprecision(2) << "lane No.: " << laneMarks.lane_no()
          << ", keep duration: " << laneMarks.lane_keep_duration()
          << "s, distance to (center: " << laneMarks.dis_to_center_line()
          << "m, left: " << laneMarks.dis_to_left_boundary() << "m, right: " << laneMarks.dis_to_right_boundary()
          << "m), percepted car count: " << laneMarks.car_count_in_perception_range() << ".";

  if (rightValidatIndex < laneMarks.right_size() - 1 || (rightLaneNum + 1) < laneMarks.right_size()) {
    for (int index = std::min(rightValidatIndex + 1, rightLaneNum + 1); index < laneMarks.right_size(); index++) {
      laneMarks.mutable_right(index)->set_type(-1);
      laneMarks.mutable_right(index)->set_confidence(0);
      laneMarks.mutable_right(index)->clear_points();
      laneMarks.mutable_right(index)->clear_backward_points();
    }
  }
  if (leftValidatIndex < laneMarks.left_size() - 1 || (leftLaneNum + 1) < laneMarks.left_size()) {
    for (int index = std::min(leftValidatIndex + 1, leftLaneNum + 1); index < laneMarks.left_size(); index++) {
      laneMarks.mutable_left(index)->set_type(-1);
      laneMarks.mutable_left(index)->set_confidence(0);
      laneMarks.mutable_left(index)->clear_points();
      laneMarks.mutable_left(index)->clear_backward_points();
    }
  }
  if (PTINTINFORMATION) std::cout << "leftValidatIndex:" << leftValidatIndex << "\n";

  laneMarks.SerializeToString(&this->payload);
  helper.PublishMessage(tx_sim::topic::kLaneMark, this->payload);
  traffic.SerializeToString(&this->payload);
  helper.PublishMessage(kEgoTrafficTopicName, this->payload);

  leftValidatIndex = 0;
  rightValidatIndex = 0;
  resetLaneState();
}

bool EgoLaneMarker::checkGeometry(const hadmap::txLanePtr& lane) {
  if (lane.get() && lane->getGeometry() && !lane->getGeometry()->empty()) return true;
  LOG(WARNING) << "the geometry of the lane " << lane->getTxLaneId() << " is empty.";
  return false;
}

double EgoLaneMarker::getDistanceAlongLane(const hadmap::txLanePtr& lane, const hadmap::txPoint& loc,
                                           hadmap::txPoint& pedal) {
  double dis = 0;
  if (this->checkGeometry(lane)) {
    dis = dynamic_cast<const hadmap::txLineCurve*>(lane->getGeometry())
              ->getPassedDistance(loc, pedal, hadmap::CoordType::COORD_WGS84);

    // std::cout<<"lane total length "<<lane->getLength()<<" passed length: "<<dis<<"\n";
  }
  if (dis < 0)
    dis = 0;
  else if (dis > lane->getLength())
    dis = lane->getLength();
  VLOG(1) << "ego car is currently on lane " << lane->getTxLaneId() << " with passed distance " << dis << "m.";
  return dis;
}

double EgoLaneMarker::getRemainLengthToNextJunction(const hadmap::txLanePtr& curLane, double passedDis) {
  double len = curLane->getLength() - passedDis;
  if (isConnectToJunction(curLane->getUniqueId())) {
    // std::cout<<"\n";
    return len;
  }
  // std::cout<<"lane length "<<len ;
  hadmap::txLanePtr lane = curLane;
  lane = this->fetchNextLane(lane);
  while (lane.get() && len < 1000 && isConnectToJunction(lane->getUniqueId()) == false) {
    len += lane->getLength();
    lane = this->fetchNextLane(lane);
  }
  if (len < 0) len = 0;
  VLOG(1) << "remaining " << len << "m to the next road juntion.";
  // std::cout<<" ramain length "<<len<<"\n";
  // std::cout<<"\n";
  return len;
}

size_t EgoLaneMarker::getEgoLaneNo(const hadmap::txLanePtr& curLane) {
  if (!this->lastEgoSection) return 0;
  size_t count = 0;
  const hadmap::txLanes& lanes = this->lastEgoSection->getLanes();
  for (auto it = lanes.crbegin(); it != lanes.crend(); ++it, ++count) {
    if ((*it)->getId() == curLane->getId()) return count;
  }
  LOG(ERROR) << "current ego lane (" << curLane->getUniqueId() << ") not found in the current section ("
             << this->lastEgoSection->getRoadId() << "," << this->lastEgoSection->getId() << ").";
  throw std::runtime_error("Current lane not found in the current section.");
}

void EgoLaneMarker::countEgoLaneKeepTime(const hadmap::txLanePtr& lane) {
  const auto& cur_lane_id = lane->getTxLaneId();
  size_t cur_lane_no = this->lastEgoLaneNo;  // 车道编号 protoc

  if (!this->lastEgoSection || cur_lane_id.roadId != this->lastEgoLaneId.roadId ||
      cur_lane_id.sectionId != this->lastEgoLaneId.sectionId) {
    hadmap::txSections secs;
    this->checkHandleCode(hadmap::getSections(this->mapHandle,
                                              hadmap::txSectionId(lane->getRoadId(), lane->getSectionId()),
                                              secs));  // 获取主车所在SECTION 非空返回一个
    this->lastEgoSection = secs.empty() ? nullptr : secs[0];
  }
  cur_lane_no = getEgoLaneNo(lane);

  if (cur_lane_id.roadId == this->lastEgoLaneId.roadId && cur_lane_no == this->lastEgoLaneNo) {
    laneKeepMs += stepIntervalMs;
  } else {
    laneKeepMs = 0;
  }

  this->lastEgoLaneId = cur_lane_id;
  this->lastEgoLaneNo = cur_lane_no;  // 0 1 2 从左往右
}

void EgoLaneMarker::retriveLineData(sim_msg::LaneMarks& laneMarks, const hadmap::txLanePtr& curLane, double passedDis,
                                    bool forward, double totalLength) {
  hadmap::txLanePtr lane = curLane;
  double remainingLength = totalLength, startPos = passedDis;
  while (remainingLength > 0) {
    VLOG(2) << "remaining length of " << (forward ? "forward" : "backward") << " detection: " << remainingLength << "m";
    if (this->checkGeometry(lane)) {
      double len = std::min(forward ? (lane->getGeometry()->getLength() - startPos) : startPos, remainingLength);
      if (len > 0) {
        VLOG(2) << "get " << (forward ? "forward" : "backward") << " " << len << "m line data ...";
        this->getLaneMarks(laneMarks, lane, startPos, len, true, forward);
        this->getLaneMarks(laneMarks, lane, startPos, len, false, forward);
        remainingLength -= len;
      }
      FillMiddlePoints(lane, forward, startPos, remainingLength);
    }

    if (remainingLength > 0) {  // get next/prev lane
      hadmap::txLanePtr nextLane;
      if (forward) {
        nextLane = this->fetchNextLane(lane);
        if (!nextLane)  // reach the last section of road, get nextLanes from next roads.
          nextLane = this->getNextRoadLanes(lane);
      } else {
        nextLane = this->fetchPrevLane(lane);
      }
      if (!nextLane) {
        LOG(WARNING) << "no more " << (forward ? "forward" : "backward") << " lanes from " << lane->getTxLaneId();
        break;  // breaking while (remainingLength > 0) out
      }
      lane = nextLane;
      startPos = forward ? 0 : lane->getLength();
      // std::cout<<" next lane is "<<lane.get();
      VLOG(2) << "chosen " << (forward ? "forward" : "backward") << " lane " << lane->getTxLaneId();
    }

    // std::cout<<"\n";
  }
}

void EgoLaneMarker::getLaneMarks(sim_msg::LaneMarks& laneMarks, const hadmap::txLanePtr& baseLane, double startPos,
                                 double length, bool left, bool forward) {
  // int64_t t_start = GetCpuTimeInMillis(), t1, t2, t3, t4;
  hadmap::txLanePtr lane = baseLane;
  for (size_t idx = 0; idx < this->detectingLaneMarkCount; idx++) {
    VLOG(2) << "found " << (left ? "left" : "right") << " lane " << lane->getTxLaneId();
    sim_msg::LaneMark* laneMark = left ? laneMarks.mutable_left(idx) : laneMarks.mutable_right(idx);
    hadmap::laneboundarypkid boundaryId = left ? lane->getLeftBoundaryId() : lane->getRightBoundaryId();
    hadmap::txLaneBoundaryPtr laneBoundary = this->fetchLaneBoundary(boundaryId);
    // if(!left ){std::cout<<"right idx "<<idx <<( forward? " forward ":" back ")<<" right forward laneBoundaryPtr
    // "<<laneBoundary.get()<<"\n";}
    if (laneBoundary) {
      auto lm = GetLaneMarkType(laneBoundary->getLaneMark(), left, idx);
      laneBoundary->getGeometry();
      if (left) {  // 对左侧车道线进行有效性判断
        auto it = laneStateCheck.find("left");
        if (it != laneStateCheck.end()) {
          auto state = it->second.at(idx);
          if (state == false) {
            it->second.at(idx) = true;
            laneMark->set_type(lm);
            setLaneColor(lm, laneMark);
            if (lm != -1) leftValidatIndex = idx;
          } else {
            if (lm != -1 && laneMark->type() == -1) {
              laneMark->set_type(lm);
              leftValidatIndex = idx;
              setLaneColor(lm, laneMark);
            }
          }
        }
      } else {  // 对右侧车道线进行有效性判断
        auto it = laneStateCheck.find("right");
        if (it != laneStateCheck.end()) {
          auto state = it->second.at(idx);
          if (state == false) {
            it->second.at(idx) = true;
            laneMark->set_type(lm);
            setLaneColor(lm, laneMark);
            if (lm != -1) rightValidatIndex = idx;
          } else {
            if (lm != -1 && laneMark->type() == -1) {
              laneMark->set_type(lm);
              leftValidatIndex = idx;
              setLaneColor(lm, laneMark);
            }
          }
        }
      }

      if (PTINTINFORMATION) {
        if (left) {
          std::cout << "lane " << (left ? "left " : "right ") << " id : " << idx << " type " << lm << "\n";
        }
      }
      if (lm == -1) {
        return;
      }
      this->fillSampleBoundaryPoint(laneMark, laneBoundary, startPos, length, forward);  // 获取到laneMark
    } else {
      laneMark->set_type(-1);
      laneMark->clear_points();
      laneMark->clear_backward_points();
      LOG(WARNING) << "lane boundary " << boundaryId << " is nullptr.";
    }
    hadmap::txLanePtr sideLane = left ? this->fetchLeftLane(lane) : this->fetchRightLane(lane);  // ？？
    if (PTINTINFORMATION) {
      if (left) {
        std::cout << "get left lane idx: " << idx << " " << sideLane.get() << (forward ? " forward " : " back ")
                  << " lane length: " << (sideLane.get() ? sideLane->getLength() : 0) << "\n";
      }
    }
    // std::cout<<"fetch "<<(left ? "left " : "right ") << sideLane.get()<<" number "<<idx<<"\n";
    if (sideLane.get() && this->checkGeometry(sideLane)) {
      if (left) {
        // std::cout<<"check left lane Ok "<<( forward? " forward ":" back ")<<"\n";
      }
      lane = sideLane;
    } else {
      // if(!left){std::cout<<"check right lane failed "<<( forward? " forward ":" back ")<<"\n";}
      // if(!left && idx+1<laneMarks.left_size()) {std::cout<<"clear right 1 lane data\n";
      // laneMarks.mutable_left(idx+1)->clear_points();} if(left) leftValidatIndex = idx; if(!left) rightValidatIndex =
      // idx;
      return;
    }
  }
}

void EgoLaneMarker::fillSampleBoundaryPoint(sim_msg::LaneMark* laneMark, const hadmap::txLaneBoundaryPtr& boundary,
                                            double startPos, double length, bool forward) {
  const hadmap::txCurve* curve = boundary->getGeometry();
  if (!curve || curve->empty()) {
    LOG(WARNING) << "lane boundary " << boundary->getId() << " is empty.";
    // <<(forward? "forward" : "back ")<< " boundary is empty\n ";
    return;
  }
  size_t sampleCount = length / this->sampleInterval;
  VLOG(2) << "fill the line in with " << sampleCount << " sample points of boundary " << boundary->getId();
  double pos = startPos;

  for (size_t i = 0; i < sampleCount; i++) {
    sim_msg::Vec3* pp = nullptr;
    if (forward)
      pp = laneMark->add_points();
    else
      pp = laneMark->add_backward_points();
    hadmap::txPoint p = curve->getPoint(pos);
    pp->set_x(p.x);
    pp->set_y(p.y);
    pp->set_z(p.z);
    if (forward)
      pos += this->sampleInterval;
    else
      pos -= this->sampleInterval;
  }
}

hadmap::txLanePtr EgoLaneMarker::getNextRoadLanes(const hadmap::txLanePtr& lane) {
  hadmap::txLaneLinks laneLinks;
  this->checkHandleCode(hadmap::getLaneLinks(this->mapHandle, lane->getRoadId(), ROAD_PKID_INVALID, laneLinks));
  // std::cout<<"road id "<<lane->getUniqueId()<<" get next road links size "<<laneLinks.size()<<"\n\n\n";
  if (laneLinks.size() > 1) {
    this->connectToJunctionLaneIdVect.insert(lane->getUniqueId());
    // std::cout<<"lane link type: "<<lane->getLinkType()<<"\n";
  }
  hadmap::txLanes nextLanes;
  for (const hadmap::txLaneLinkPtr& l : laneLinks) {
    hadmap::txLanePtr nextLaneCandidate;
    hadmap::txLaneId lid(l->toRoadId(), l->toSectionId(), l->toLaneId());
    this->checkHandleCode(hadmap::getLane(this->mapHandle, lid, nextLaneCandidate));
    if (this->checkGeometry(nextLaneCandidate)) nextLanes.push_back(nextLaneCandidate);
  }
  auto& p = this->cachedNextLanes[lane->getUniqueId()];
  if (nextLanes.empty()) return nullptr;
  p = this->chooseNextLane(*lane, nextLanes);
  return p;
}

hadmap::txLanePtr EgoLaneMarker::chooseNextLane(const hadmap::txLane& currentLane, const hadmap::txLanes& nextLanes) {
  // TODO(nemo): we should choose the next lane of which curvature is the most similar to the current lane.
  return nextLanes[0];
}

void EgoLaneMarker::transformCoordinates(sim_msg::LaneMarks& laneMarks, const hadmap::txPoint& locPoint,
                                         const hadmap::txLanePtr& cur_lane, const sim_msg::Vec3& rpy,
                                         sim_msg::Traffic& traffic, double passed_dis) {
  Eigen::AngleAxisd rollAngle(rpy.x(), Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitchAngle(rpy.y(), Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yawAngle(rpy.z(), Eigen::Vector3d::UnitZ());
  Eigen::Matrix3d rotationMatrix = (yawAngle * pitchAngle * rollAngle).matrix();

  // 5. transform all points in lane marks from lon-lat to vihicle coordinate.
  size_t tid = 0;
  std::unique_lock<std::mutex> lk(this->mtx);
  for (size_t i = 0; i < laneMarks.left_size(); ++i) {
    sim_msg::LaneMark* lm = laneMarks.mutable_left(i);
    TransformData& d = this->transformed[tid++];
    d.laneMark = lm, d.locPoint = &locPoint, d.rotationMatrix = &rotationMatrix;
  }
  for (size_t i = 0; i < laneMarks.right_size(); ++i) {
    sim_msg::LaneMark* lm = laneMarks.mutable_right(i);
    TransformData& d = this->transformed[tid++];
    d.laneMark = lm, d.locPoint = &locPoint, d.rotationMatrix = &rotationMatrix;
  }
  lk.unlock();
  cv.notify_all();

  // also transforms the points in traffic message.
  this->transformCoordinates(traffic, locPoint, rotationMatrix);
  this->adjustTrafficTheta(traffic, rpy.z());

  lk.lock();
  cv.wait(lk, [this] { return this->transformed.empty(); });

  if (cur_lane) CalculateDistance2LaneLine(laneMarks, cur_lane, locPoint, passed_dis, rotationMatrix);
}

void EgoLaneMarker::transformCoordinates(sim_msg::LaneMark& laneMark, const hadmap::txPoint& loc,
                                         const Eigen::Matrix3d& m) {
  // lon-lat -> enu -> vehichle
  double refLon = loc.x, refLat = loc.y, refAlt = loc.z, x, y, z;
  for (sim_msg::Vec3& p : *laneMark.mutable_points()) {
    x = p.x(), y = p.y(), z = p.z();
    coord_trans_api::lonlat2enu(x, y, z, refLon, refLat, refAlt);
    Eigen::Vector3d r = m.inverse() * Eigen::Vector3d(x, y, z);
    p.set_x(r.x()), p.set_y(r.y()), p.set_z(r.z());
  }
  for (sim_msg::Vec3& p : *laneMark.mutable_backward_points()) {
    x = p.x(), y = p.y(), z = p.z();
    coord_trans_api::lonlat2enu(x, y, z, refLon, refLat, refAlt);
    Eigen::Vector3d r = m.inverse() * Eigen::Vector3d(x, y, z);
    p.set_x(r.x()), p.set_y(r.y()), p.set_z(r.z());
  }
}

void EgoLaneMarker::transformCoordinates(sim_msg::Traffic& traffic, const hadmap::txPoint& loc,
                                         const Eigen::Matrix3d& m) {
  // lon-lat -> enu -> vehichle
  double refLon = loc.x, refLat = loc.y, refAlt = loc.z, x, y, z = 0;
  // TODO(nemo): waiting for the re-definition of the traffic proto to remove the ugly code below.
#define TX_SIM_L3_TRANSFORM                                       \
  do {                                                            \
    x = item.x(), y = item.y();                                   \
    coord_trans_api::lonlat2enu(x, y, z, refLon, refLat, refAlt); \
    Eigen::Vector3d r = m.inverse() * Eigen::Vector3d(x, y, z);   \
    item.set_x(r.x()), item.set_y(r.y());                         \
  } while (0)
  for (sim_msg::Car& item : *traffic.mutable_cars()) TX_SIM_L3_TRANSFORM;
  for (sim_msg::StaticObstacle& item : *traffic.mutable_staticobstacles()) TX_SIM_L3_TRANSFORM;
  for (sim_msg::DynamicObstacle& item : *traffic.mutable_dynamicobstacles()) TX_SIM_L3_TRANSFORM;
  for (sim_msg::TrafficLight& item : *traffic.mutable_trafficlights()) TX_SIM_L3_TRANSFORM;
}

void EgoLaneMarker::adjustTrafficTheta(sim_msg::Traffic& traffic, double egoTheta) {
  for (sim_msg::Car& car : *traffic.mutable_cars()) {
    double adjust = car.theta() - egoTheta;
    if (adjust > M_PI)
      adjust -= kPIx2;
    else if (adjust < -M_PI)
      adjust += kPIx2;

    if (std::fabs(adjust) >= M_PI) adjust = kNearPi;
    car.set_theta(adjust);
    car.set_heading(adjust);
  }
}

void EgoLaneMarker::getInitialParameters(tx_sim::InitHelper& params) {
  this->stepIntervalMs = std::stoi(params.GetParameter(tx_sim::constant::kInitKeyModuleExecutePeriod));
  std::string arg = params.GetParameter("forwardLength");
  if (!arg.empty()) {
    this->forwardLength = std::stod(arg);
    if (this->forwardLength > 0)
      LOG(INFO) << "the length for forward lane mark detection: " << this->forwardLength << "(m)";
    else
      throw std::invalid_argument("forwardLength should be grater than 0.");
  } else {
    LOG(WARNING) << "the length for forward lane mark detection is not specified, using default value: "
                 << this->forwardLength << "(m)";
  }

  arg = params.GetParameter("sampleInterval");
  if (!arg.empty()) {
    this->sampleInterval = std::stod(arg);
    if (this->sampleInterval > 0)
      LOG(INFO) << "the interval for lane mark points sampling: " << this->sampleInterval << "(m)";
    else
      throw std::invalid_argument("sampleInterval should be grater than 0.");
  } else {
    LOG(WARNING) << "the interval for lane mark points sampling is not specified, using default value: "
                 << this->sampleInterval << "(m)";
  }

  arg = params.GetParameter("laneDetectingCount");
  if (!arg.empty()) {
    this->detectingLaneMarkCount = std::stoi(arg);
    if (this->detectingLaneMarkCount > 0)
      LOG(INFO) << "the count for lane mark detection: " << this->detectingLaneMarkCount << " per side";
    else
      throw std::invalid_argument("laneDetectingCount should be grater than 0.");
  } else {
    LOG(WARNING) << "the count for lane mark detection is not specified, using default value: " << this->sampleInterval
                 << " per side" << std::endl;
  }

  std::vector<bool> state;
  for (int i = 0; i < detectingLaneMarkCount; i++) {
    state.push_back(false);
  }
  laneStateCheck.insert(std::pair<std::string, std::vector<bool>>("left", state));
  laneStateCheck.insert(std::pair<std::string, std::vector<bool>>("right", state));

  arg = params.GetParameter("vlog");
  if (!arg.empty()) FLAGS_v = std::stoi(arg);

  // arg = params.GetParameter("usingLonLat");
  // if (!arg.empty()) {
  //   this->isOuputInLonLat = true;
  //   LOG(INFO) << "the coordinates of lane marks output is in lon-lat-alt form.";
  // }
}

void EgoLaneMarker::checkHandleCode(int optCode) {
  if (optCode == TX_HADMAP_HANDLE_ERROR || optCode == TX_HADMAP_DATA_ERROR || optCode == TX_HADMAP_PARAM_ERROR) {
    std::stringstream ss;
    ss << "hadmap engine operation failed(" << optCode << "): ";
    if (this->mapHandle) ss << hadmap::getLastOptInfo(this->mapHandle);
    throw std::runtime_error(ss.str());
  }
}

inline hadmap::txLaneBoundaryPtr EgoLaneMarker::fetchLaneBoundary(hadmap::laneboundarypkid id) {
  auto it = this->cachedBoundaries.find(id);
  if (it != this->cachedBoundaries.end()) return it->second;
  auto& p = this->cachedBoundaries[id];  //?
  this->checkHandleCode(hadmap::getBoundary(this->mapHandle, id, p));
  return p;
}

inline hadmap::txLanePtr EgoLaneMarker::fetchNextLane(const hadmap::txLanePtr& lane) {
  const auto& id = lane->getUniqueId();
  // std::cout<<"lane id string "<<id<<" road id "<<lane->getTxLaneId().roadId<<" sectionId
  // "<<lane->getTxLaneId().sectionId<<" laneId "<<lane->getTxLaneId().laneId;
  auto it = this->cachedNextLanes.find(id);
  if (it != this->cachedNextLanes.end()) {
    return it->second;
  }
  auto& p = this->cachedNextLanes[id];
  hadmap::txLanes nextLanes;  // 返回下一段所有车道数据
  this->checkHandleCode(hadmap::getNextLanes(this->mapHandle, lane, nextLanes));
  if (nextLanes.empty()) {
    this->roadEndLaneUniqIdVector.insert(id);
    std::cout << "add one endLane \n";
    p.reset();
    return nullptr;
  }
  p = this->chooseNextLane(*lane, nextLanes);
  return p;
}

inline hadmap::txLanePtr EgoLaneMarker::fetchPrevLane(const hadmap::txLanePtr& lane) {
  const auto& id = lane->getUniqueId();
  auto it = this->cachedPrevLanes.find(id);
  if (it != this->cachedPrevLanes.end()) return it->second;
  auto& p = this->cachedPrevLanes[id];
  hadmap::txLanes nextLanes;
  this->checkHandleCode(hadmap::getPrevLanes(this->mapHandle, lane, nextLanes));
  if (nextLanes.empty()) {
    p.reset();
    return nullptr;
  }
  p = this->chooseNextLane(*lane, nextLanes);
  return p;
}

inline hadmap::txLanePtr EgoLaneMarker::fetchLeftLane(const hadmap::txLanePtr& lane) {
  const auto& id = lane->getUniqueId();
  auto it = this->cachedLeftLanes.find(id);
  if (it != this->cachedLeftLanes.end()) {
    // std::cout<<" left lane is "<<it->second.get()<<"\n";
    return it->second;
  }
  auto& p = this->cachedLeftLanes[id];
  int retCode = hadmap::getLeftLane(this->mapHandle, lane, p);
  LOG(INFO) << "left lane " << (p ? p->getUniqueId() : "empty") << " retCode " << retCode << "\n";
  this->checkHandleCode(retCode);
  if (retCode == TX_HADMAP_DATA_EMPTY) {
    LOG(INFO) << "left lane " << lane->getUniqueId() << " retCode empty \n";
    p.reset();
    return nullptr;
  } else {
    return p;
  }
}

inline hadmap::txLanePtr EgoLaneMarker::fetchRightLane(const hadmap::txLanePtr& lane) {
  const auto& id = lane->getUniqueId();
  auto it = this->cachedRightLanes.find(id);
  if (it != this->cachedRightLanes.end()) {
    // std::cout<<" right lane is "<<it->second.get()<<"\n";
    return it->second;
  }
  auto& p = this->cachedRightLanes[id];
  int retCode = hadmap::getRightLane(this->mapHandle, lane, p);
  LOG(INFO) << "right lane " << (p ? p->getUniqueId() : "empty") << " retCode " << retCode << "\n";
  this->checkHandleCode(retCode);
  if (retCode == TX_HADMAP_DATA_EMPTY) {
    LOG(INFO) << "right lane " << lane->getUniqueId() << " retCode empty \n";
    p.reset();
    return nullptr;
  }
  return p;
}

void EgoLaneMarker::doTransformingWork(size_t tid) {
  while (true) {
    std::unique_lock<std::mutex> lk(this->mtx);
    this->cv.wait(lk, [this, tid] { return transformed.find(tid) != transformed.end(); });
    TransformData d = transformed.find(tid)->second;
    lk.unlock();

    this->transformCoordinates(*d.laneMark, *d.locPoint, *d.rotationMatrix);

    lk.lock();
    transformed.erase(tid);
    lk.unlock();
    cv.notify_all();
  }
}

void EgoLaneMarker::resetMapHandle() {
  LOG(INFO) << "closing map ...";
  if (this->mapHandle) this->checkHandleCode(hadmap::hadmapClose(&this->mapHandle));
}

bool EgoLaneMarker::isRoadEndLane(std::string laneUniqueId) {
  auto it = roadEndLaneUniqIdVector.find(laneUniqueId);
  if (it != roadEndLaneUniqIdVector.end())
    return true;
  else
    return false;
}

bool EgoLaneMarker::isConnectToJunction(std::string laneUniqueId) {
  auto it = connectToJunctionLaneIdVect.find(laneUniqueId);
  if (it != connectToJunctionLaneIdVect.end())
    return true;
  else
    return false;
}

void EgoLaneMarker::resetLaneState() {
  auto leftVector = laneStateCheck.find("left");
  if (leftVector != laneStateCheck.end()) {
    for (auto it = leftVector->second.begin(); it != leftVector->second.end(); it++) {
      if (PTINTINFORMATION)
        std::cout << "left state :" << *it << " "
                  << "\n";
      (*it) = false;
    }
  }

  auto rightVector = laneStateCheck.find("right");
  if (rightVector != laneStateCheck.end()) {
    for (auto it = rightVector->second.begin(); it != rightVector->second.end(); it++) {
      if (PTINTINFORMATION)
        std::cout << "right state :" << *it << " "
                  << "\n";
      (*it) = false;
    }
  }
}

void EgoLaneMarker::setLaneColor(int32_t& LM, sim_msg::LaneMark* lane) {
  switch (LM) {
    case hadmap::LANE_MARK_None:
      lane->set_color(5);
      lane->set_confidence(0);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_Solid:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_Broken:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_Solid2:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_Broken2:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_SolidSolid:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_SolidBroken:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_BrokenSolid:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
    case hadmap::LANE_MARK_BrokenBroken:
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    case hadmap::LANE_MARK_Curb: {
      lane->set_color(0);
      lane->set_confidence(0);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x100,
    case hadmap::LANE_MARK_Fence: {
      lane->set_color(0);
      lane->set_confidence(0);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x200,
    case hadmap::LANE_MARK_RoadEdge: {
      lane->set_color(0);
      lane->set_confidence(0);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x400,
    case hadmap::LANE_MARK_Bold: {
      lane->set_color(5);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x1000,  // bold lines
    case hadmap::LANE_MARK_Yellow: {
      lane->set_color(1);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x10000,  // yellow lines, default white,blue green red
    case hadmap::LANE_MARK_Yellow2: {
      lane->set_color(1);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x20000,
    case hadmap::LANE_MARK_YellowYellow: {
      lane->set_color(1);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x30000,
    case hadmap::LANE_MARK_OtherColors: {
      lane->set_color(5);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x40000,
    case hadmap::LANE_MARK_Red: {
      lane->set_color(2);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x50000,
    case hadmap::LANE_MARK_Green: {
      lane->set_color(3);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x60000,
    case hadmap::LANE_MARK_Blue: {
      lane->set_color(4);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x70000,
    case hadmap::LANE_MARK_LDM: {
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x100000,
    case hadmap::LANE_MARK_VGL: {
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x200000,
    case hadmap::LANE_MARK_Shielded: {
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x200000,
    case hadmap::LANE_MARK_Absent: {
      lane->set_color(0);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
    }  // = 0x200000,
    case 0x10002: {
      lane->set_color(1);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      lane->set_type(2);
      break;
    }  // 65538
    case 0x10001: {
      lane->set_color(1);
      lane->set_confidence(1);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      lane->set_type(1);
      break;
    }  // 65537
    default:
      lane->set_color(5);
      lane->set_confidence(0);  // color 0-white 1-yellow 2-red 3-green 4-blue 5-other
      break;
  }
}

void EgoLaneMarker::getEgoLaneArrow(sim_msg::LaneMarks& laneMarks, hadmap::LANE_ARROW arrow) {
  switch (arrow) {
    case hadmap::LANE_ARROW_None: {
      laneMarks.set_lane_arrow(0);
      return;
    }
    case hadmap::LANE_ARROW_Straight: {
      laneMarks.set_lane_arrow(1);
      return;
    }
    case hadmap::LANE_ARROW_Left: {
      laneMarks.set_lane_arrow(2);
      return;
    }
    case hadmap::LANE_ARROW_Right: {
      laneMarks.set_lane_arrow(3);
      return;
    }
    case hadmap::LANE_ARROW_Uturn: {
      laneMarks.set_lane_arrow(4);
      return;
    }
    case hadmap::LANE_ARROW_SkewLeft: {
      laneMarks.set_lane_arrow(5);
      return;
    }
    case hadmap::LANE_ARROW_SkewRight: {
      laneMarks.set_lane_arrow(6);
      return;
    }
    case hadmap::LANE_ARROW_StraightLeft: {
      laneMarks.set_lane_arrow(7);
      return;
    }
    case hadmap::LANE_ARROW_StraightRight: {
      laneMarks.set_lane_arrow(8);
      return;
    }
    case hadmap::LANE_ARROW_LeftRight: {
      laneMarks.set_lane_arrow(9);
      return;
    }
    case hadmap::LANE_ARROW_LeftUturn: {
      laneMarks.set_lane_arrow(10);
      return;
    }
    case hadmap::LANE_ARROW_StraightUturn: {
      laneMarks.set_lane_arrow(11);
      return;
    }
    default: {
      laneMarks.set_lane_arrow(0);
      return;
    }
  }
}

void EgoLaneMarker::getTwoSideLaneMarksNumber(const hadmap::txLanePtr& lane, int& leftNum, int& rightNum) {
  leftNum = 0;
  rightNum = 0;
  if (lane.get() == nullptr) {
    return;
  }

  auto leftLane = fetchLeftLane(lane);
  while (leftLane.get()) {
    if (isLaneDrivable(leftLane)) {
      leftNum++;
    }
    leftLane = fetchLeftLane(leftLane);
  }

  auto rightLane = fetchRightLane(lane);
  while (rightLane.get()) {
    if (isLaneDrivable(rightLane)) {
      rightNum++;
    }
    rightLane = fetchRightLane(rightLane);
  }
  // LOG(INFO)<<"Ego lane: "<<lane->getUniqueId()<<" left lane number: "<<leftNum<<" right lane number:
  // "<<rightNum<<"\n";
}

bool EgoLaneMarker::isLaneDrivable(const hadmap::txLanePtr& lane) {
  if (lane.get() == nullptr) return false;
  if (lane->getLaneType() == hadmap::LANE_TYPE_Driving || lane->getLaneType() == hadmap::LANE_TYPE_Restricted)
    return true;
  else
    return false;
}

void EgoLaneMarker::FillMiddlePoints(const hadmap::txLanePtr currentLane, bool forward, double startPos,
                                     double remainingLength) {
  const hadmap::txCurve* pCurve = currentLane->getGeometry();
  double laneLength = pCurve->getLength();
  size_t nSize = remainingLength / sampleInterval;
  if (forward) {
    for (int i = 0; i < nSize; i++) {
      hadmap::txPoint point = pCurve->getPoint(startPos);
      m_listMiddlePoint.emplace_back(std::make_tuple(point.x, point.y, point.z));
      startPos += remainingLength;
    }
  } else {
    for (int i = 0; i < nSize; i++) {
      hadmap::txPoint point = pCurve->getPoint(startPos);
      m_listMiddlePoint.emplace_front(std::make_tuple(point.x, point.y, point.z));
      startPos -= remainingLength;
    }
  }
}
