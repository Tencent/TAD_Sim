// Copyright (c) 2019 Tencent. All rights reserved.
// Written by ChitianSun

#include "basic.pb.h"
#include "localLocation.pb.h"
#include "location.pb.h"
#include "trajectory.pb.h"

#include "glog/logging.h"
#include "perfect_control_with_option.h"

#include <cmath>
#include <complex>
#include <iomanip>

#ifdef CAL_EGO_LANE_INFO
#  include "manager/MapManager.h"
#endif

#define INVALID_MAP_ID 0xFFFFFFFFFFFFFFFF
#define LANE_INVALID_ID 0x0

#ifdef _WIN32
#  define MODULE_API __declspec(dllexport)
#else
#  define MODULE_API
#endif  // _WIN32

#ifndef M_PI
#  define M_PI (3.14159265358979323846)  // pi
#endif                                   // !M_PI

#define PI M_PI

double L3_STEPTIME = 0.01;
int jobid = -1;
int egoid = -1;
// 用于double类型的边界eps常量
const double const_limit_eps = 0.0001;

static const char kTopicLocalLocation[] = "LOCAL_LOCATION";

/**
 * @brief Perfect Control module, inherits from tx_sim::SimModule.
 */
class PerfectControlWithOptionModule : public tx_sim::SimModule {
  // time related variable
  double timeStamp = 0;           // time stamp for current step
  double lastTimeStamp = 0;       // time stamp for last step
  double stepTime = 20;           // steptime between steps
  double thisLocalFrameTime = 0;  // time stamp for current local frame
  double lastLocalFrameTime = 0;  // time stamp for last local frame
  int frameID = -1;               // current step frame ID
  int thisFrameID = -1;           // frame ID for current local frame

  // map related variable
  double originLon, originLat, originAlt = 0;

  std::string payload;
  const Eigen::Vector3d trailerOffset = {-1.457, 0.0, 0.0};

  // trajectory related variable
  const std::string trajectoryTopic = tx_sim::topic::kTrajectory;
  std::string trajectoryStr;
  sim_msg::Trajectory trajectory;
  sim_msg::Trajectory lastTrajectory;

  // location related variable
  const std::string locationTopic = tx_sim::topic::kLocation;
  const std::string LOCATION_TRAILER = "LOCATION_TRAILER";
  std::string locationStr;
  sim_msg::Location egoLocation;
  double initX, initY, initZ, initV, initTheta = 0;
  sim_msg::Location outputTrailerLocation;
  bool firstLocationMessage = false;

  // local location related variable
  const std::string localLocationTopic = "LOCAL_LOCATION";
  std::string localLocationStr;
  sim_msg::LocalLocation inputLocalLocation;

  // local frame related variable
  double localFramePositionX, localFramePositionY, localFrameTheta = 0;
  double lastLocalFramePositionX, lastLocalFramePositionY, lastLocalFrameTheta = 0;
  Eigen::MatrixXd transMatrix;

  // option related variable
  Option option = Option::Global;

  bool enableFakeTrailer = false;

  bool useMapZValue = false;
  double preAlt = 0.0;
#ifdef CAL_EGO_LANE_INFO
  eval::MapManagerPtr g_map_ptr = eval::CMapManager::GetInstance();
  eval::MapQueryAgent g_map_query;
#endif

  void updateEgoLane(sim_msg::Location &location) {
    // reset
    location.mutable_ego_lane()->set_lanepkid(0);
    location.mutable_ego_lane()->set_sectionpkid(INVALID_MAP_ID);
    location.mutable_ego_lane()->set_roadpkid(INVALID_MAP_ID);
    location.mutable_ego_lane()->set_dist_2_ref_line(0.0);
    // location.mutable_position()->set_z(preAlt);

#ifdef CAL_EGO_LANE_INFO
    if (g_map_ptr) {
      // ego position
      eval::CLocation ego_loc;
      auto ego_pos = ego_loc.MutablePosition();
      auto ego_rpy = ego_loc.MutableEuler();

      ego_pos->SetValues(location.position().x(), location.position().y(), location.position().z(), eval::Coord_WGS84);
      ego_rpy->SetValues(location.rpy().x(), location.rpy().y(), location.rpy().z());
      g_map_ptr->WGS84ToENU(*ego_pos);

      // update and get map info
      g_map_query.UpdateMapInfo({ego_loc.GetPosition()});
      auto map_info = g_map_query.GetMapInfo();

      if (map_info) {
        // get ego lane
        hadmap::txLanePtr lane_ptr = map_info->m_active_lane.m_lane;

        // if ego lane ptr exist and ego on road
        if (lane_ptr && map_info->m_on_road) {
          auto lane_id = lane_ptr->getTxLaneId();
          location.mutable_ego_lane()->set_lanepkid(lane_id.laneId);
          location.mutable_ego_lane()->set_sectionpkid(lane_id.sectionId);
          location.mutable_ego_lane()->set_roadpkid(lane_id.roadId);

          // offset from reference line
          double m_ref_line_lateral_offset = 0.0;
          const eval::EvalPoints &sample_points = map_info->m_active_lane.m_ref_line.sample_points;
          int index = eval::CEvalMath::GetNearestPointIndex(sample_points, ego_loc);
          if (0 <= index && index < sample_points.size()) {
            m_ref_line_lateral_offset = eval::CEvalMath::GetLateralDist(sample_points.at(index), ego_loc);

            // override Z value if use map Z value
            if (useMapZValue) {
              eval::CPosition ref_point_wgs84 = sample_points.at(index).GetPosition();
              g_map_ptr->ENUToWGS84(ref_point_wgs84);
              location.mutable_position()->set_z(ref_point_wgs84.GetZ());
              preAlt = ref_point_wgs84.GetZ();
              LOG_INFO << "current alt:" << ref_point_wgs84.GetZ() << "\n";
            }
          }
          location.mutable_ego_lane()->set_dist_2_ref_line(m_ref_line_lateral_offset);
        } else {
          LOG_ERROR << "Current Ego Lane is Empty" << "\n";
        }
      }
    }
    LOG_INFO << "ego lane id:" << location.ego_lane().lanepkid() << ", section id:" << location.ego_lane().sectionpkid()
             << ", road id:" << location.ego_lane().roadpkid()
             << ", lateral dist from ref line:" << location.ego_lane().dist_2_ref_line() << "\n";
#endif
  }

  /*bool GetValueFromSimMessage(const sim_msg::SimMessage& msg, std::string key, std::string& value)
  {
          google::protobuf::Map<std::string, std::string>::const_iterator  iter = msg.messages().find(key);
          if (iter == msg.messages().end())
                  return false;

          value = iter->second;
          if (value.empty())
          {
                  return false;
          }
          return true;
  }*/

  /**
   * @brief check if the trajectory is updated
   * @param newTrajectory
   * @param oldTrajectory
   * @return true if trajectory is updated, false otherwise
   */
  bool isTrajectoryUpdated(const sim_msg::Trajectory &newTrajectory, const sim_msg::Trajectory &oldTrajectory) {
    if (newTrajectory.point_size() != 0) {
      // VLOG(2) << "Compare point" << std::endl;
      if (oldTrajectory.point(0).x() == newTrajectory.point(0).x() &&
          oldTrajectory.point(0).y() == newTrajectory.point(0).y()) {
        return false;
      } else {
        return true;
      }
    } else {
      VLOG(0) << "Trajectory is empty" << std::endl;
      return false;
    }
  }

  /**
   * @brief interplot theta by ratio, adapted for any range of theta, add by DongYuan.Hu
   * @param theta0
   * @param theta1
   * @param ratio
   * @param thetaOut, return value, limitted to [0,2*pi]
   */
  void thetaInterpolation(const double &theta0, const double &theta1, const double &ratio, double &thetaOut) {
    std::complex<double> v0(std::cos(theta0), std::sin(theta0));
    std::complex<double> v1(std::cos(theta1), std::sin(theta1));

    auto v0Conj = std::conj(v0);
    auto deltaComplex = v0Conj * v1;
    double deltaTheta = std::arg(deltaComplex) * ratio;

    std::complex<double> interplotV(std::cos(deltaTheta), std::sin(deltaTheta));
    interplotV = interplotV * v0;
    thetaOut = std::arg(interplotV);
    thetaOut = thetaOut < 0.0 ? 2 * M_PI + thetaOut : thetaOut;
  }

  /**
   * @brief get location by time, most commonly used
   * @param deltaTime in ms
   * @param trajectory input trajectory data
   * @param trailerLocation output trailer's current location, used when enableFakeTrailer is true
   * @return sim_msg::Location, as the main output
   */
  sim_msg::Location getLocationByTime(double deltaTime, const sim_msg::Trajectory &trajectory,
                                      sim_msg::Location &trailerLocation) {
    sim_msg::Location resultLocation;

    // if trajectory points is empty, quit and set time to -1
    if (trajectory.point().size() <= 0) {
      resultLocation.set_t(-1);
      VLOG(0) << "trajectory point is empty.\n";
      return resultLocation;
    }

    // convert deltaTime from ms to s
    deltaTime += lastTimeStamp;
    deltaTime = deltaTime / 1000;
    double tmpX = 0, tmpY = 0, tmpZ = 0;
    // get result location using linear interpolation
    // todo: is it necessary to loop all points or just do it between trajectory[0] and trajectory[1]?
    for (int i = 0; i < trajectory.point_size() - 1; i++) {
      if (trajectory.point(i).t() <= deltaTime && trajectory.point(i + 1).t() >= deltaTime) {
        // VLOG(2) << "Point " << i << std::endl;
        double x0, x1, y0, y1, z0, z1;
        x0 = trajectory.point(i).x();
        x1 = trajectory.point(i + 1).x();
        y0 = trajectory.point(i).y();
        y1 = trajectory.point(i + 1).y();
        z0 = trajectory.point(i).z();
        z1 = trajectory.point(i + 1).z();
        // add level 0 log to verify what points are selected by lohasye
        VLOG(0) << std::fixed << std::setprecision(9) << "Point " << i << " x:" << x0 << " y:" << y0 << " z:" << z0
                << " t:" << trajectory.point(i).t() << " v:" << trajectory.point(i).v()
                << " a:" << trajectory.point(i).a() << " theta: " << trajectory.point(i).theta() << std::endl;
        VLOG(0) << std::fixed << std::setprecision(9) << "Point " << i + 1 << " x:" << x1 << " y:" << y1 << " z:" << z1
                << " t:" << trajectory.point(i + 1).t() << " v:" << trajectory.point(i + 1).v()
                << " a:" << trajectory.point(i + 1).a() << " theta: " << trajectory.point(i + 1).theta() << std::endl;

        // wgs84 to ENU
        coord_trans_api::lonlat2enu(x0, y0, z0, originLon, originLat, originAlt);
        coord_trans_api::lonlat2enu(x1, y1, z1, originLon, originLat, originAlt);

        // when time is equal to the t of point i, or the t of point i and i+1 are the same, ratio is 0
        double ratio = 0.0;
        if (deltaTime != trajectory.point(i).t()) {
          // linear interpolation ratio
          ratio = (deltaTime - trajectory.point(i).t()) / (trajectory.point(i + 1).t() - trajectory.point(i).t());
        }

        // set ego position
        sim_msg::Vec3 *egoPosition = resultLocation.mutable_position();
        if (trajectory.point(i).v() <= 0 && trajectory.point(i + 1).v() <= 0) {
          egoPosition->set_x(trajectory.point(i).x());
          egoPosition->set_y(trajectory.point(i).y());
          egoPosition->set_z(trajectory.point(i).z());
        } else {
          tmpX = (x1 - x0) * ratio + x0;
          tmpY = (y1 - y0) * ratio + y0;
          tmpZ = (z1 - z0) * ratio + z0;
          // ENU to wgs84
          coord_trans_api::enu2lonlat(tmpX, tmpY, tmpZ, originLon, originLat, originAlt);

          // add position guard
          if (isInRange(tmpX, trajectory.point(i).x(), trajectory.point(i + 1).x()) &&
              isInRange(tmpY, trajectory.point(i).y(), trajectory.point(i + 1).y()) &&
              isInRange(tmpZ, trajectory.point(i).z(), trajectory.point(i + 1).z())) {
            // do nothing
          } else {
            // this situation is more likely (by lohasye)
            tmpX = (trajectory.point(i + 1).x() - trajectory.point(i).x()) * ratio + trajectory.point(i).x();
            tmpY = (trajectory.point(i + 1).y() - trajectory.point(i).y()) * ratio + trajectory.point(i).y();
            tmpZ = (trajectory.point(i + 1).z() - trajectory.point(i).z()) * ratio + trajectory.point(i).z();
            VLOG(0) << "Fix position range! t: " << deltaTime << std::endl;
          }

          egoPosition->set_x(tmpX);
          egoPosition->set_y(tmpY);
          egoPosition->set_z(tmpZ);
        }

        // set ego Rotation
        sim_msg::Vec3 *egoRotation = resultLocation.mutable_rpy();
        double theta0 = trajectory.point(i).theta();
        double theta1 = trajectory.point(i + 1).theta();

        double theta = 0;
        // new interplot algorithm for theta, add by DongYuan.Hu
        thetaInterpolation(theta0, theta1, ratio, theta);

        egoRotation->set_x(0);
        egoRotation->set_y(0);
        egoRotation->set_z(theta);

        // set ego velocity
        sim_msg::Vec3 *egoVelocity = resultLocation.mutable_velocity();
        double v0 = trajectory.point(i).v();
        double v1 = trajectory.point(i + 1).v();
        double v = (v1 - v0) * ratio + v0;
        if (v < 0.01 && trajectory.point(i).a() <= -1) {
          VLOG(0) << "Perfect Control Stops Vehicle" << std::endl;
          egoVelocity->set_x(0);
          egoVelocity->set_y(0);
          egoVelocity->set_z(0);
        } else {
          egoVelocity->set_x(v * cos(theta));
          egoVelocity->set_y(v * sin(theta));
          egoVelocity->set_z(0);
        }

        // set ego angular
        sim_msg::Vec3 *egoAngular = resultLocation.mutable_angular();
        double yaw_rate = 0.0;
        egoAngular->set_x(0);
        egoAngular->set_y(0);
        if (0 != trajectory.point(i).kappa()) {
          yaw_rate = trajectory.point(i).kappa() * v;
        } else if (fabs(theta1 - theta0) > const_limit_eps) {
          VLOG(0) << "different theta but kappa = 0! t: " << deltaTime << std::endl;
          yaw_rate = (theta1 - theta0) / (trajectory.point(i + 1).t() - trajectory.point(i).t());
        }
        egoAngular->set_z(yaw_rate);

        // set ego acceleration
        sim_msg::Vec3 *egoAcceleration = resultLocation.mutable_acceleration();
        double a0 = trajectory.point(i).a();
        double a1 = trajectory.point(i + 1).a();
        double a = (a1 - a0) * ratio + a0;
        egoAcceleration->set_x(a);
        egoAcceleration->set_y(v * yaw_rate);
        egoAcceleration->set_z(0);

        // set ego t
        resultLocation.set_t(deltaTime);
        // add level 0 log to verify the output message
        VLOG(0) << std::fixed << std::setprecision(9) << "Result point x:" << egoPosition->x()
                << " y:" << egoPosition->y() << " z:" << egoPosition->z() << " t:" << deltaTime
                << " vx:" << egoVelocity->x() << " vy:" << egoVelocity->y() << " theta:" << egoRotation->z()
                << " angular_v:" << egoAngular->z() << std::endl;

        // set trailer location if is enabled
        if (enableFakeTrailer) {
          trailerLocation.Clear();
          setTrailerLocation(resultLocation, trailerLocation);
        }
        return resultLocation;
      }
    }

    // if only one point in trajectory and it's at target time, copy it to resultLocation
    if (1 == trajectory.point_size() && deltaTime == trajectory.point(0).t()) {
      const auto &point = trajectory.point(0);
      // add level 0 log to verify the output message
      VLOG(0) << "Trajectory has only one point. t: " << deltaTime << std::endl;
      VLOG(0) << std::fixed << std::setprecision(9) << "Point 0 x:" << point.x() << " y:" << point.y()
              << " z:" << point.z() << " t:" << point.t() << " v:" << point.v() << " a:" << point.a()
              << " theta: " << point.theta() << std::endl;

      // set ego position
      sim_msg::Vec3 *egoPosition = resultLocation.mutable_position();
      egoPosition->set_x(point.x());
      egoPosition->set_y(point.y());
      egoPosition->set_z(point.z());

      // set ego Rotation
      sim_msg::Vec3 *egoRotation = resultLocation.mutable_rpy();
      egoRotation->set_x(0);
      egoRotation->set_y(0);
      egoRotation->set_z(point.theta());

      // set ego angular
      sim_msg::Vec3 *egoAngular = resultLocation.mutable_angular();
      egoAngular->set_x(0);
      egoAngular->set_y(0);
      // can't calculate yaw rate from theta difference
      egoAngular->set_z(point.kappa() * point.v());

      // set ego velocity
      sim_msg::Vec3 *egoVelocity = resultLocation.mutable_velocity();
      double v = point.v();
      if (v < 0.01 && point.a() <= -1) {
        VLOG(0) << "Perfect Control Stops Vehicle" << std::endl;
        egoVelocity->set_x(0);
        egoVelocity->set_y(0);
        egoVelocity->set_z(0);
      } else {
        egoVelocity->set_x(v * cos(point.theta()));
        egoVelocity->set_y(v * sin(point.theta()));
        egoVelocity->set_z(0);
      }

      // set ego acceleration
      sim_msg::Vec3 *egoAcceleration = resultLocation.mutable_acceleration();
      egoAcceleration->set_x(point.a());
      // Lateral acceleration can only calculated from kappa and v^2
      egoAcceleration->set_y(point.kappa() * point.v() * point.v());
      egoAcceleration->set_z(0);

      // set ego t
      resultLocation.set_t(deltaTime);
      // add level 0 log to verify the output message
      VLOG(0) << std::fixed << std::setprecision(9) << "Result point x:" << egoPosition->x()
              << " y:" << egoPosition->y() << " z:" << egoPosition->z() << " t:" << deltaTime
              << " vx:" << egoVelocity->x() << " vy:" << egoVelocity->y() << " theta:" << egoRotation->z()
              << " angular_v:" << egoAngular->z() << std::endl;
    }

    // log problem when trajectory's points are not useful at all.
    VLOG(0) << "trajectory time range is:[" << trajectory.point(0).t() << ", "
            << trajectory.point(trajectory.point_size() - 1).t() << "].\n";
    VLOG(0) << "current interplot sim time:" << deltaTime
            << ". interplot failed since current simtime is not within range of trajectory time.\n";

    resultLocation.set_t(-1);

    return resultLocation;
  }

  sim_msg::LocalLocation getLocalLocationByTime(double deltaTime, const sim_msg::Trajectory &trajectory,
                                                sim_msg::Location &outputLocation) {
    sim_msg::LocalLocation resultLocalLocation;
    // convert deltaTime from ms to s
    deltaTime += lastTimeStamp;
    deltaTime = deltaTime / 1000;
    double tmpX, tmpY, tmpZ = 0;
    // get result location using linear interpolation
    // todo: is it necessary to loop all points or just do it between trajectory[0] and trajectory[1]?
    for (int i = 0; i < trajectory.point_size() - 1; i++) {
      // VLOG(2) << "DeltaTime is: "<< deltaTime <<"  " << "Point 0's t is: " << trajectory.point(i).t() << "Point 1's t
      // is: " << trajectory.point(i + 1).t() << std::endl;
      if (trajectory.point(i).t() <= deltaTime && trajectory.point(i + 1).t() >= deltaTime) {
        // set local location msg
        // set header
        sim_msg::Header *header = resultLocalLocation.mutable_header();
        header->set_time_stamp(deltaTime * 1000);
        header->set_frame_id(frameID);
        header->set_sequence(frameID);

        // set x, y
        double x0, x1, y0, y1;
        x0 = trajectory.point(i).x();
        x1 = trajectory.point(i + 1).x();
        y0 = trajectory.point(i).y();
        y1 = trajectory.point(i + 1).y();

        double ratio = 0.0;
        if (deltaTime != trajectory.point(i).t()) {
          ratio = (deltaTime - trajectory.point(i).t()) / (trajectory.point(i + 1).t() - trajectory.point(i).t());
        }

        // sim_msg::Vec3* egoPosition = resultLocalLocation.mutable_position();
        if (trajectory.point(i).v() <= 0 && trajectory.point(i + 1).v() <= 0) {
          // should not move
          resultLocalLocation.set_x(trajectory.point(i).x());
          resultLocalLocation.set_y(trajectory.point(i).y());
        } else {
          tmpX = (x1 - x0) * ratio + x0;
          tmpY = (y1 - y0) * ratio + y0;

          resultLocalLocation.set_x(tmpX);
          resultLocalLocation.set_y(tmpY);
        }
        // VLOG(2) << "x0: " << x0 << " y0: " << y0 << " x1: " << x1 << " y1 " << " tmpX: " << tmpX << " tmpY: " << tmpY
        // << std::endl;

        // set yaw, yaw_v
        // VLOG(2) << "Set yaw" << std::endl;
        double yaw =
            (trajectory.point(i + 1).theta() - trajectory.point(i).theta()) * ratio + trajectory.point(i).theta();
        resultLocalLocation.set_yaw(yaw);
        resultLocalLocation.set_yaw_v(yaw);

        // set vx, vy
        double v0_l, v1_l, v_l = 0;
        v0_l = trajectory.point(i).v();
        v1_l = trajectory.point(i + 1).v();
        v_l = (v1_l - v0_l) * ratio + v0_l;
        // VLOG(2) << "local v: " << v_l << std::endl;
        resultLocalLocation.set_vx(v_l * cos(yaw));
        resultLocalLocation.set_vy(v_l * sin(yaw));

        // set time_thisLocalFrame, time_lastLocalFrame
        // VLOG(2) << "Set local frame time" << std::endl;
        resultLocalLocation.set_time_thislocalframe(thisLocalFrameTime);
        resultLocalLocation.set_time_lastlocalframe(lastLocalFrameTime);

        // set this frame ID
        // VLOG(2) << "Set frame id" << std::endl;

        // resultLocalLocation.set_thisframeid((int)(timeStamp / 1000));
        // Debug code
        resultLocalLocation.set_thisframeid(0);

        // set transform matrix
        // VLOG(2) << "Get mat array" << std::endl;
        double mat[16];
        // VLOG(2) << "TransMatrix size is: "<< transMatrix.size() << std::endl;

        // Debug code
        transMatrix = setLocalToLocal(0, 0, 0, 0, 0, 0);

        getMatArray(transMatrix, mat);
        // VLOG(2) << "Set location transform" << std::endl;
        for (int i = 0; i < 16; ++i) {
          // resultLocalLocation.set_transform(i, mat[i]);
          // VLOG(2) << "Add value: " << mat[i] << std::endl;
          resultLocalLocation.add_transform(mat[i]);
        }

        // set location msg
        // set t
        // VLOG(2) << "Set t" << std::endl;
        outputLocation.set_t(timeStamp);

        // set position
        // VLOG(2) << "Set position" << std::endl;
        /*double x, y = 0;
        double z = originAlt;
        Eigen::Vector2d locationENU;
        locationENU = resultLocalLocation.x() * localFrameAxisX + resultLocalLocation.y() * localFrameAxisY;
        double offsetX, offsetY = 0;
        if (trajectory.flag() == (int)(timeStamp / 1000))
        {
                offsetX = localFramePositionX;
                offsetY = localFramePositionY;
        }
        else if(trajectory.flag() == ((int)(timeStamp / 1000) - 1))
        {
                offsetX = lastLocalFramePositionX;
                offsetY = lastLocalFramePositionY;
        }
        x = locationENU.x() + offsetX;
        y = locationENU.y() + offsetY;
        coord_trans_api::enu2lonlat(x, y, z, originLon, originLat, originAlt);
        sim_msg::Vec3* position = outputLocation.mutable_position();
        position->set_x(x);
        position->set_y(y);
        position->set_z(z);*/

        // qiqi's solution
        getLocalToEnu(tmpX, tmpY, localFramePositionX, localFramePositionY, localFrameTheta);
        // VLOG(2) << "qiqi's soluation: " << localFramePositionX << " " << localFramePositionY << " " <<
        // localFrameTheta << " " << tmpX << " " << tmpY << std::endl;
        double tmpZ = 0;
        coord_trans_api::enu2lonlat(tmpX, tmpY, tmpZ, originLon, originLat, originAlt);
        sim_msg::Vec3 *position = outputLocation.mutable_position();
        position->set_x(tmpX);
        position->set_y(tmpY);
        position->set_z(tmpZ);

        // get theta
        // VLOG(2) << "Set theta" << std::endl;
        double theta0 = trajectory.point(i).theta();
        double theta1 = trajectory.point(i + 1).theta();

        // Fix the bug when degree from PI to -PI. Add by JiangyuLin
        double theta = 0;
        double angle = 2.0 * PI - (std::abs(theta1) + std::abs(theta0));
        if (theta1 * theta0 < 0 && angle < PI) {
          if (theta0 > 0) {
            theta = (angle)*ratio + theta0;
          } else {
            theta = -(angle)*ratio + theta0;
          }
        } else {
          theta = (theta1 - theta0) * ratio + theta0;
        }

        // todo: Should convert to -pi to pi
        theta += localFrameTheta;

        // set velocity
        // VLOG(2) << "Set velocity" << std::endl;
        sim_msg::Vec3 *velocity = outputLocation.mutable_velocity();
        double v0 = trajectory.point(i).v();
        double v1 = trajectory.point(i + 1).v();
        double v = (v1 - v0) * ratio + v0;

        if (v < 0.01 && trajectory.point(i).a() <= -1) {
          VLOG(2) << "Perfect Control Stops Vehicle" << std::endl;
          velocity->set_x(0);
          velocity->set_y(0);
          velocity->set_z(0);
        } else {
          velocity->set_x(v * cos(theta));
          velocity->set_y(v * sin(theta));
          velocity->set_z(0);
        }

        // set angular
        // VLOG(2) << "Set angular" << std::endl;
        sim_msg::Vec3 *angular = outputLocation.mutable_angular();
        angular->set_x(0);
        angular->set_y(0);
        angular->set_z(trajectory.point(i + 1).kappa() * trajectory.point(i + 1).v());

        // set rpy
        // VLOG(2) << "Set rpy" << std::endl;
        sim_msg::Vec3 *rpy = outputLocation.mutable_rpy();
        rpy->set_x(0);
        rpy->set_y(0);
        rpy->set_z(theta);

        // set acceleration
        sim_msg::Vec3 *egoAcceleration = outputLocation.mutable_acceleration();
        double a0 = trajectory.point(i).a();
        double a1 = trajectory.point(i + 1).a();
        double a = (a1 - a0) * ratio + a0;
        egoAcceleration->set_x(a * cos(theta));
        egoAcceleration->set_y(a * sin(theta));
        egoAcceleration->set_z(0);

        return resultLocalLocation;
      } else {
        // print all trajectory point for debugging
        double x, y, z;
        x = trajectory.point(i).x();
        y = trajectory.point(i).y();
        z = originAlt;

        // convert point from lonlat to local
        coord_trans_api::lonlat2enu(x, y, z, originLon, originLat, originAlt);
        // VLOG(2) << "Point " << i << ": x:" << x << " y:" << y << " t:" << trajectory.point(i).t() << std::endl;
      }
    }

    VLOG(0) << "Trajectory not long enough, biggest t is: " << trajectory.point(trajectory.point_size() - 1).t()
            << std::endl;
    // set header
    sim_msg::Header *header = resultLocalLocation.mutable_header();
    header->set_time_stamp(timeStamp);
    header->set_frame_id(frameID);
    header->set_sequence(frameID);

    // set x, y
    double x, y;
    x = trajectory.point(0).x();
    y = trajectory.point(0).y();
    resultLocalLocation.set_x(x);
    resultLocalLocation.set_y(y);

    double yaw = trajectory.point(0).theta();
    resultLocalLocation.set_yaw(yaw);
    resultLocalLocation.set_yaw_v(yaw);

    resultLocalLocation.set_vx(trajectory.point(0).v() * cos(yaw));
    resultLocalLocation.set_vy(trajectory.point(0).v() * sin(yaw));

    resultLocalLocation.set_time_thislocalframe(thisLocalFrameTime);
    resultLocalLocation.set_time_lastlocalframe(lastLocalFrameTime);

    resultLocalLocation.set_thisframeid(0);
    double mat[16];
    transMatrix = setLocalToLocal(0, 0, 0, 0, 0, 0);
    getMatArray(transMatrix, mat);
    for (int i = 0; i < 16; ++i) {
      resultLocalLocation.add_transform(mat[i]);
    }
    return resultLocalLocation;
  }

  sim_msg::Location getGlobalLocation(const sim_msg::Trajectory &trajectory, double deltaTime, bool intervene) {
    if (intervene) {
      // todo Add intervene calculation here
      return getLocationByTime(deltaTime, trajectory, outputTrailerLocation);
    } else {
      return getLocationByTime(deltaTime, trajectory, outputTrailerLocation);
    }
  }

  sim_msg::LocalLocation getLocalLocation(const sim_msg::Trajectory &trajectory, double deltaTime,
                                          sim_msg::Location &outputLocation) {
    return getLocalLocationByTime(deltaTime, trajectory, outputLocation);
  }

  void updateLocalFrameAxis(const sim_msg::Location &location) {
    // localFrameAxisX = egoVehicle->GetForwardVector();
    // localFrameAxisY = egoVehicle->GetRightVector();
    double x, y, z, theta = 0;
    x = location.position().x();
    y = location.position().y();
    z = originAlt;
    theta = location.rpy().z();
    coord_trans_api::lonlat2enu(x, y, z, originLon, originLat, originAlt);
    // todo:localframe position X init value?
    transMatrix = setLocalToLocal(localFramePositionX, localFramePositionY, localFrameTheta, x, y, theta);
    lastLocalFramePositionX = localFramePositionX;
    lastLocalFramePositionY = localFramePositionY;
    lastLocalFrameTheta = localFrameTheta;

    localFramePositionX = x;
    localFramePositionY = y;
    localFrameTheta = theta;

    // VLOG(2) << "Input location theta: " << theta << std::endl;
  }

  void Init(tx_sim::InitHelper &helper) {
    // VLOG(2) << "Perfect Control Init" << std::endl;
    // set subscription topic and publish topic
    helper.Subscribe(tx_sim::topic::kTrajectory);
    helper.Subscribe(tx_sim::topic::kLocation);
    helper.Subscribe(kTopicLocalLocation);
    helper.Publish(tx_sim::topic::kLocation);
    helper.Publish(kTopicLocalLocation);
    helper.Publish(LOCATION_TRAILER);
    // get step time setting from coordinator
    std::string arg = helper.GetParameter("stepTime");
    if (arg.empty()) {
      stepTime = std::stoi(helper.GetParameter(tx_sim::constant::kInitKeyModuleExecutePeriod));
      VLOG(0) << "stepTime is not at Parameter list. use kInitKeyModuleExecutePeriod: " << stepTime << std::endl;
    } else {
      stepTime = std::stod(arg);
      VLOG(0) << "stepTime is at Parameter list. use it: " << stepTime << std::endl;
    }

    // get option setting from coordinator
    arg = helper.GetParameter("option");
    if (!arg.empty()) {
      // option = argIt->second;
      if (arg == "global") {
        option = Option::Global;
        VLOG(0) << "Perfect Control Option is: Global" << std::endl;
      } else if (arg == "local") {
        option = Option::Local;
        VLOG(0) << "Perfect Control Option is: Local" << std::endl;
      } else {
        VLOG(0) << "Perfect Control received undefined option" << std::endl;
      }
    } else {
      VLOG(0) << "Perfect Control Can`t Get Option!" << std::endl;
    }

    // get trailer setting from coordinator
    arg = helper.GetParameter("enableFakeTrailer");
    if (!arg.empty()) {
      enableFakeTrailer = static_cast<bool>(std::stoi(arg));
    }

    // get use map Z value option
    arg = helper.GetParameter("useMapZValue");
    if (!arg.empty()) {
      useMapZValue = static_cast<bool>(std::stoi(arg));
    }

    // vlog level
    std::string glog_level = helper.GetParameter("_log_level");
    FLAGS_v = 0;
    if (!glog_level.empty()) {
      FLAGS_v = std::atoi(glog_level.c_str());
    }
  }

  void Reset(tx_sim::ResetHelper &helper) {
    VLOG(0) << "Perfect Control Reset" << std::endl;
    // reset timestamp
    timeStamp = 0.0;
    lastTimeStamp = 0;

    // reset origin map location
    originLon = helper.map_local_origin().x;
    originLat = helper.map_local_origin().y;
    originAlt = helper.map_local_origin().z;
    preAlt = originAlt;

    // get first ego location
    sim_msg::Location start_location;
    start_location.ParseFromString(helper.ego_start_location());
    egoLocation.ParseFromString(helper.ego_start_location());
    initX = start_location.position().x();
    initY = start_location.position().y();
    initZ = start_location.position().z();
    initV = sqrt(start_location.velocity().x() * start_location.velocity().x() +
                 start_location.velocity().y() * start_location.velocity().y() +
                 start_location.velocity().z() * start_location.velocity().z());
    initTheta = start_location.rpy().z();
    initTheta = std::atan2(std::sin(initTheta), std::cos(initTheta));
    initTheta = initTheta < 0.0 ? initTheta + 2 * M_PI : initTheta;
    egoLocation.mutable_rpy()->set_z(initTheta);  // [0,2*pi]
    firstLocationMessage = true;

    transMatrix = setLocalToLocal(0, 0, 0, 0, 0, 0);
#ifdef CAL_EGO_LANE_INFO
    // init grading map
    if (g_map_ptr->Connect(helper.map_file_path(), helper.ego_start_location())) {
      eval::CPosition pivot_enu(start_location.position().x(), start_location.position().y(),
                                start_location.position().z(), eval::Coord_WGS84);
      g_map_ptr->WGS84ToENU(pivot_enu);
      g_map_query.UpdateMapInfo({pivot_enu});
      std::this_thread::sleep_for(std::chrono::seconds(3));
    }
#endif
  }

  void Step(tx_sim::StepHelper &helper) {
    // should comment out when released as it is called every step
    // VLOG(2) << "Perfect Control Step" << input.timestamp() << std::endl;

    helper.GetSubscribedMessage("LOCAL_LOCATION", payload);
    inputLocalLocation.ParseFromString(payload);

    if (helper.timestamp() == 0) {
      helper.GetSubscribedMessage(tx_sim::topic::kLocation, payload);
      // egoLocation.ParseFromString(payload);
      // VLOG(2) << "Update local frame axis at first step" << std::endl;
      // todo:localframe position X init value?
      double tmpX, tmpY, tmpZ = 0;
      tmpX = egoLocation.position().x();
      tmpY = egoLocation.position().y();
      tmpZ = egoLocation.position().z();

      coord_trans_api::lonlat2enu(tmpX, tmpY, tmpZ, originLon, originLat, originAlt);
      localFramePositionX = tmpX;
      localFramePositionY = tmpY;
      localFrameTheta = egoLocation.rpy().z();
      // VLOG(2) << "Local frame starts from " << localFramePositionX << " " << localFramePositionY << " " <<
      // localFrameTheta << std::endl;
      updateLocalFrameAxis(egoLocation);
    }

    // get trajectory msg
    helper.GetSubscribedMessage(tx_sim::topic::kTrajectory, payload);
    if (!payload.empty()) {
      // get trajectory msg
      VLOG(2) << "Parse trajectory" << std::endl;
      trajectory.Clear();
      trajectory.ParseFromString(payload);

      if (lastTrajectory.point_size() == 0) {
        lastTrajectory = trajectory;
      }

      if (isTrajectoryUpdated(trajectory, lastTrajectory)) {
        lastTimeStamp = helper.timestamp();
        lastTrajectory = trajectory;
      } else {
        // planning did not give updated trajectory
        VLOG(2) << "Trajectory not updated" << std::endl;
      }
      double deltaTime = helper.timestamp() - lastTimeStamp;
      timeStamp = helper.timestamp();
      /*
      frameID = helper.frameid();
      VLOG(2) << "Delta time is: " << deltaTime << std::endl;

      // set local frame every second
      if ((int)helper.timestamp() % 1000 == 0) {
        // set local frame axis
        VLOG(2) << "Update local frame" << std::endl;
        // transMatrix = setLocalToLocal(x, y, theta, localFramePositionX, localFramePositionY, localFrameTheta);
        if (GetValueFromSimMessage(input, locationTopic, locationStr)) {
          inputLocation.Clear();
          inputLocation.ParseFromString(locationStr);
          // updateLocalFrameAxis(inputLocation);
        }
      } else {
        // Do nothing
      }
      */

      sim_msg::Location location;
      sim_msg::LocalLocation localLocation;
      std::string locationOutput;
      std::string trailerOutput;
      // ::google::protobuf::Map<::std::string, ::std::string>* outMessage = output.mutable_messages();

      switch (option) {
        case Option::Global:
          VLOG(2) << "Option:Global" << std::endl;
          location = getGlobalLocation(trajectory, deltaTime, false);

          // get ego lane and override Z value if use map Z value
          updateEgoLane(location);

          if (location.t() > -1e-12) {
            // update ego location
            egoLocation.CopyFrom(location);
          } else {
            // interplot failed, ego stays where it is and ticking
            egoLocation.set_t(helper.timestamp() * 0.001);
          }

          // if trajectory points is empty, no location publish
          if (!trajectory.point().empty()) {
            egoLocation.SerializeToString(&locationOutput);
            helper.PublishMessage(tx_sim::topic::kLocation, locationOutput);
            if (firstLocationMessage) {
              VLOG(0) << "first location message published in Option::Global.\n";
              firstLocationMessage = false;
            }
          }

          if (enableFakeTrailer) {
            outputTrailerLocation.SerializeToString(&trailerOutput);
            helper.PublishMessage(LOCATION_TRAILER, trailerOutput);
          }
          break;
        case Option::Local:
          VLOG(2) << "Option:Local" << std::endl;
          localLocation = getLocalLocation(trajectory, deltaTime, location);
          localLocation.SerializeToString(&locationOutput);
          helper.PublishMessage("LOCAL_LOCATION", locationOutput);

          // send global location according to L3's request
          location.SerializeToString(&locationOutput);
          // (*outMessage)[tx_sim::topic_Name(tx_sim::topic::kLocation)] = locationOutput;
          helper.PublishMessage(tx_sim::topic::kLocation, locationOutput);
          if (firstLocationMessage) {
            VLOG(0) << "first location message published in Option::Local.\n";
            firstLocationMessage = false;
          }
          break;
        default:
          break;
      }
    } else {
      LOG(ERROR) << "trajectory empty.\n";
      // sim_msg::Location location;
      sim_msg::LocalLocation localLocation;
      std::string locationOutput;
      // ::google::protobuf::Map<::std::string, ::std::string>* outMessage = output.mutable_messages();

      // set init value of localLocation
      transMatrix = setLocalToLocal(0, 0, 0, 0, 0, 0);
      double mat[16];
      getMatArray(transMatrix, mat);
      for (int i = 0; i < 16; ++i) {
        localLocation.add_transform(mat[i]);
      }
      sim_msg::Header *header = localLocation.mutable_header();
      header->set_time_stamp(helper.timestamp());
      header->set_frame_id(0);
      header->set_sequence(0);
      // VLOG(2) << "transform size is : " <<localLocation.transform_size() << std::endl;
      localLocation.set_x(0);
      localLocation.set_y(0);
      localLocation.set_yaw(0);
      localLocation.set_vx(initV);
      localLocation.set_vy(0);
      localLocation.SerializeToString(&locationOutput);
      // (*outMessage)[tx_sim::topic_Name(tx_sim::topic::kLocalLocation)] = locationOutput;
      helper.PublishMessage("LOCAL_LOCATION", locationOutput);

      // set init value of location
      /*sim_msg::Vec3* egoPosition = location.mutable_position();
      egoPosition->set_x(initX);
      egoPosition->set_y(initY);
      egoPosition->set_z(initZ);
      sim_msg::Vec3* egoRotation = location.mutable_rpy();
      egoRotation->set_x(0);
      egoRotation->set_y(0);
      egoRotation->set_z(initTheta);
      sim_msg::Vec3* egoVelocity = location.mutable_velocity();
      egoVelocity->set_x(initV * cos(initTheta));
      egoVelocity->set_y(initV * sin(initTheta));
      egoVelocity->set_z(0);
      */

      if (firstLocationMessage) {
        egoLocation.set_t(helper.timestamp() * 0.001);
        egoLocation.SerializeToString(&locationOutput);
        // (*outMessage)[tx_sim::topic_Name(tx_sim::topic::kLocation)] = locationOutput;
        helper.PublishMessage(tx_sim::topic::kLocation, locationOutput);
        VLOG(0) << "first location message published while trajectory empty.\n";
        firstLocationMessage = false;
      }
    }
  }

  void Stop(tx_sim::StopHelper &helper) { VLOG(0) << "Perfect Control Stop" << std::endl; }

  // Eigen related functions
  void getLocalToLocal(double &x, double &y, double last_x, double last_y, double last_theta, double cur_x,
                       double cur_y, double cur_theta) {
    auto mat = setLocalToLocal(last_x, last_y, last_theta, cur_x, cur_y, cur_theta);
    Eigen::Vector4d vec(4);
    vec(0) = x;
    vec(1) = y;
    vec(2) = 0;
    vec(3) = 1;
    vec = mat * vec;
    x = vec(0);
    y = vec(1);
  }

  void getEnuToLocal(double &x, double &y, double local_x, double local_y, double local_theta) {
    auto mat = setLocalToGlobal(local_x, local_y, local_theta);
    mat = mat.inverse();
    Eigen::Vector4d vec(4);
    vec(0) = x;
    vec(1) = y;
    vec(2) = 0;
    vec(3) = 1;
    vec = mat * vec;
    x = vec(0);
    y = vec(1);
  }

  void getLocalToEnu(double &x, double &y, double local_x, double local_y, double local_theta) {
    auto mat = setLocalToGlobal(local_x, local_y, local_theta);
    Eigen::Vector4d vec(4);
    vec(0) = x;
    vec(1) = y;
    vec(2) = 0;
    vec(3) = 1;
    vec = mat * vec;
    x = vec(0);
    y = vec(1);
  }

  Eigen::MatrixXd setLocalToGlobal(double x, double y, double theta) {
    // rotation then translation
    //  cos -sin 0 x
    //  sin cos 0 y
    //  0    0  1 0
    //  0    0  0 1

    Eigen::MatrixXd mat(4, 4);
    mat(0, 0) = cos(theta);
    mat(0, 1) = -sin(theta);
    mat(0, 2) = 0;
    mat(0, 3) = x;

    mat(1, 0) = sin(theta);
    mat(1, 1) = cos(theta);
    mat(1, 2) = 0;
    mat(1, 3) = y;

    mat(2, 0) = 0;
    mat(2, 1) = 0;
    mat(2, 2) = 1;
    mat(2, 3) = 0;

    mat(3, 0) = 0;
    mat(3, 1) = 0;
    mat(3, 2) = 0;
    mat(3, 3) = 1;
    return mat;
  }

  Eigen::MatrixXd setGlobalToLocal(double x, double y, double theta) {
    Eigen::MatrixXd mat = setLocalToGlobal(x, y, theta);
    mat = mat.inverse();
    return mat;
  }

  Eigen::MatrixXd setLocalToLocal(double x1, double y1, double theta1, double x2, double y2, double theta2) {
    Eigen::MatrixXd mat1 = setLocalToGlobal(x1, y1, theta1);
    Eigen::MatrixXd mat2 = setGlobalToLocal(x2, y2, theta2);
    Eigen::MatrixXd mat = mat2 * mat1;
    return mat;
  }

  // get the transformation matrix of the trailer
  void getMatArray(const Eigen::MatrixXd &trans, double *mat) {
    mat[0] = trans(0, 0);
    mat[1] = trans(0, 1);
    mat[2] = trans(0, 2);
    mat[3] = trans(0, 3);

    mat[4] = trans(1, 0);
    mat[5] = trans(1, 1);
    mat[6] = trans(1, 2);
    mat[7] = trans(1, 3);

    mat[8] = trans(2, 0);
    mat[9] = trans(2, 1);
    mat[10] = trans(2, 2);
    mat[11] = trans(2, 3);

    mat[12] = trans(3, 0);
    mat[13] = trans(3, 1);
    mat[14] = trans(3, 2);
    mat[15] = trans(3, 3);
  }

  void setTrailerLocation(const sim_msg::Location &tractorLocation, sim_msg::Location &trailerLocation) {
    // this function is copied from VDModule written by dongyuanhu
    trailerLocation.CopyFrom(tractorLocation);

    // get enu location of ego vehicle
    double egoX = tractorLocation.position().x();
    double egoY = tractorLocation.position().y();
    double egoZ = tractorLocation.position().z();
    coord_trans_api::lonlat2enu(egoX, egoY, egoZ, originLon, originLat, originAlt);

    // get enu location of trailer
    Eigen::AngleAxisd rollAngle(tractorLocation.rpy().x(), Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd yawAngle(tractorLocation.rpy().z(), Eigen::Vector3d::UnitZ());
    Eigen::AngleAxisd pitchAngle(tractorLocation.rpy().y(), Eigen::Vector3d::UnitY());

    Eigen::Quaterniond globalToVehicleQuaternion = yawAngle * pitchAngle * rollAngle;
    Eigen::Matrix3d globalToVehicleRotation = globalToVehicleQuaternion.toRotationMatrix();
    Eigen::Vector3d trailerPosition = globalToVehicleRotation * trailerOffset;

    double trailerX = egoX + trailerPosition[0];
    double trailerY = egoY + trailerPosition[1];
    double trailerZ = egoZ + trailerPosition[2];

    coord_trans_api::enu2lonlat(trailerX, trailerY, trailerZ, originLon, originLat, originAlt);

    sim_msg::Vec3 *tmpPosition = trailerLocation.mutable_position();
    tmpPosition->set_x(trailerX);
    tmpPosition->set_y(trailerY);
    tmpPosition->set_z(originAlt);
  }

  // receives three floating point numbers as parameters, namely x and boundA and boundB.
  bool isInRange(double x, double boundA, double boundB) {
    // Determine whether x is between boundA and boundB (inclusive of the boundary), or whether x is between boundB and
    // boundA (inclusive of the boundary)
    return (((x >= boundA) && (x <= boundB)) || ((x <= boundA) && (x >= boundB)));
  }
};

// must including this macro definition to access to simulation system
TXSIM_MODULE(PerfectControlWithOptionModule)
