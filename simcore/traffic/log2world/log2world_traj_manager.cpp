// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "log2world_traj_manager.h"
#include "tx_tadsim_flags.h"
#include "tx_time_utils.h"
TX_NAMESPACE_OPEN(TrafficFlow)

Base::txFloat interp(const Base::txFloat t1, const Base::txFloat t2, const Base::txFloat v1, const Base::txFloat v2,
                     const Base::txFloat t) TX_NOEXCEPT {
  return v1 + (v2 - v1) * (t - t1) / (t2 - t1);
}

Base::txFloat interp_angle(const Base::txFloat t1, const Base::txFloat t2, Base::txFloat a1, Base::txFloat a2,
                           const Base::txFloat t) TX_NOEXCEPT {
  if (FLAGS_Logsim_HeadingInterpolation) {
    if ((a2 - a1) > Math::PI) {
      a1 += Math::TWO_PI;
    } else if ((a1 - a2) > Math::PI) {
      a2 += Math::TWO_PI;
    }
    Base::txFloat res = interp(t1, t2, a1, a2, t);
    if (res > Math::PI) {
      res -= Math::TWO_PI;
    }
    return res;
  } else {
    return a1;
  }
}

sim_msg::Vec3 interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Vec3& v1, const sim_msg::Vec3& v2,
                     const Base::txFloat t) TX_NOEXCEPT {
  sim_msg::Vec3 res;
  res.set_x(interp(t1, t2, v1.x(), v2.x(), t));
  res.set_y(interp(t1, t2, v1.y(), v2.y(), t));
  res.set_z(interp(t1, t2, v1.z(), v2.z(), t));
  return std::move(res);
}

sim_msg::Vec3 interp_wgs84(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Vec3& v1,
                           const sim_msg::Vec3& v2, const Base::txFloat t) TX_NOEXCEPT {
  sim_msg::Vec3 res;
  Coord::txENU car1_enu;
  car1_enu.FromWGS84(v1.x(), v1.y());
  Coord::txENU car2_enu;
  car2_enu.FromWGS84(v2.x(), v2.y());
  Coord::txWGS84 res_wgs84;
  res_wgs84.FromENU(interp(t1, t2, car1_enu.X(), car2_enu.X(), t), interp(t1, t2, car1_enu.Y(), car2_enu.Y(), t));
  res.set_x(res_wgs84.Lon() /*interp(t1, t2, v1.x(), v2.x(), t)*/);
  res.set_y(res_wgs84.Lat() /*interp(t1, t2, v1.y(), v2.y(), t)*/);
  res.set_z(interp(t1, t2, v1.z(), v2.z(), t));
  return std::move(res);
}

sim_msg::Location interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Location& loc1,
                         const sim_msg::Location& loc2, const Base::txFloat t) TX_NOEXCEPT {
  sim_msg::Location res;
  res.set_t(t);
  *res.mutable_position() = interp_wgs84(t1, t2, loc1.position(), loc2.position(), t);
  *res.mutable_velocity() = interp(t1, t2, loc1.velocity(), loc2.velocity(), t);
  *res.mutable_angular() = interp(t1, t2, loc1.angular(), loc2.angular(), t);
  *res.mutable_rpy() = interp(t1, t2, loc1.rpy(), loc2.rpy(), t);
  *res.mutable_acceleration() = interp(t1, t2, loc1.acceleration(), loc2.acceleration(), t);
  return std::move(res);
}

sim_msg::Car interp(const Base::txFloat t1, const Base::txFloat t2, const sim_msg::Car& car1, const sim_msg::Car& car2,
                    const Base::txFloat t) TX_NOEXCEPT {
  sim_msg::Car res = car1;
  res.set_t(t);
  Coord::txENU car1_enu;
  car1_enu.FromWGS84(car1.x(), car1.y());
  Coord::txENU car2_enu;
  car2_enu.FromWGS84(car2.x(), car2.y());
  Coord::txWGS84 res_wgs84;
  res_wgs84.FromENU(interp(t1, t2, car1_enu.X(), car2_enu.X(), t), interp(t1, t2, car1_enu.Y(), car2_enu.Y(), t));

  res.set_x(res_wgs84.Lon() /*interp(t1, t2, car1.x(), car2.x(), t)*/);
  res.set_y(res_wgs84.Lat() /*interp(t1, t2, car1.y(), car2.y(), t)*/);
  res.set_z(interp(t1, t2, car1.z(), car2.z(), t));
  res.set_heading(interp_angle(t1, t2, car1.heading(), car2.heading(), t));
  res.set_v(interp(t1, t2, car1.v(), car2.v(), t));
  res.set_vl(interp(t1, t2, car1.vl(), car2.vl(), t));
  res.set_theta(interp_angle(t1, t2, car1.theta(), car2.theta(), t));
  res.set_length(interp(t1, t2, car1.length(), car2.length(), t));
  res.set_width(interp(t1, t2, car1.width(), car2.width(), t));
  res.set_height(interp(t1, t2, car1.height(), car2.height(), t));
  return std::move(res);
}

#if __TX_Mark__("txL2W_Car_FrameAware_TrajMgr")

Base::txBool txL2W_Car_FrameAware_TrajMgr::EnableFilling(Base::TimeParamManager const&) const TX_NOEXCEPT {
  if (IsValid() && _NonEmpty_(mSpaceTimeCarList)) {
    /*have element which not be sended, but it does not mean send at current frame*/
    return mSpaceTimeCarList.back().IsNotSend();
  } else {
    return false;
  }
}

Base::txBool txL2W_Car_FrameAware_TrajMgr::NeedFinish(Base::TimeParamManager const& timeMgr) TX_NOEXCEPT {
  if (IsValid() && _NonEmpty_(mSpaceTimeCarList)) {
    if (Utils::SecondToMillisecond(timeMgr.AbsTime()) > mSpaceTimeCarList.back().rsv_abs_time) {
      return true;
    } else {
      return false;
    }
  } else {
    return true;
  }
}

Base::txBool txL2W_Car_FrameAware_TrajMgr::add_new_pos(const txSpaceTimeCarList& l2w_car_list) TX_NOEXCEPT {
  TX_MARK("avoid clear mSpaceTimeCarList");
  mSrcL2wCarList = l2w_car_list;
  for (const auto& refNode : l2w_car_list) {
    mSpaceTimeCarList.emplace_back(tx_L2W_SpaceTimeCar(refNode));
  }
  return true;
}

TrafficFlow::txTrajManagerBasePtr txL2W_Car_FrameAware_TrajMgr::clone_traj() TX_NOEXCEPT {
  TrafficFlow::txTrajManagerBasePtr ret_ptr = std::make_shared<txL2W_Car_FrameAware_TrajMgr>();
  if (NonNull_Pointer(ret_ptr)) {
    ret_ptr->add_new_pos(mSrcL2wCarList);
    return ret_ptr;
  } else {
    return nullptr;
  }
}

Base::txBool txL2W_Car_FrameAware_TrajMgr::update_kinect(const Base::txInt leftIdx, const Base::txInt rightIdx,
                                                         const sim_msg::Car& refCarInfo,
                                                         const Base::txFloat _carTimeStamp,
                                                         txKinectInfo& refKinectInfo) const TX_NOEXCEPT {
  if ((leftIdx < rightIdx) && (leftIdx >= 0) && (rightIdx <= mSpaceTimeCarList.size())) {
    if (rightIdx == mSpaceTimeCarList.size()) {
      TX_MARK("last sample, and have more than one sample point.");
      TX_MARK("acc, velocity keep.");
      refKinectInfo.mAverTimeStamp = _carTimeStamp;
      return true;
    } else {
      Base::txFloat sum_dist = 0.0;
      Base::txFloat sum_duration_ms = 0.0;
      for (Base::txInt idx = leftIdx; idx >= 0; --idx) {
        /*LOG(WARNING) << TX_VARS(idx) << TX_VARS(leftIdx) << TX_VARS(rightIdx);*/
        const auto& left_node = mSpaceTimeCarList[idx];
        const auto& right_node = mSpaceTimeCarList[idx + 1];
        /*LOG(WARNING) << TX_VARS(idx)
            << TX_VARS(left_node.rsv_abs_time)
            << TX_VARS(left_node.raw_car_pb.x())
            << TX_VARS(left_node.raw_car_pb.y())
            << TX_VARS(right_node.rsv_abs_time)
            << TX_VARS(right_node.raw_car_pb.x())
            << TX_VARS(right_node.raw_car_pb.y());*/
        const hadmap::txPoint left_wgs84(left_node.raw_car_pb.x(), left_node.raw_car_pb.x(), 0.0);
        const hadmap::txPoint right_wgs84(right_node.raw_car_pb.x(), right_node.raw_car_pb.x(), 0.0);

        const Base::txFloat dist = Coord::txWGS84::EuclideanDistanceByWGS84WithoutAlt(left_wgs84, right_wgs84);
        sum_dist += dist;
        sum_duration_ms += (right_node.rsv_abs_time - left_node.rsv_abs_time);
        if (sum_dist > L2W_Motion_Dist()) {
          break;
        }
      }
      Base::txFloat objSpeed = mKinectInfo.mAverSpeed;
      if (sum_duration_ms > 0.0) {
        objSpeed = sum_dist / Utils::MillisecondToSecond(sum_duration_ms);
      }
      refKinectInfo.mAverAcc = (objSpeed - mKinectInfo.mAverSpeed) / (1.0 /*second*/);
      refKinectInfo.mAverSpeed = objSpeed;
      refKinectInfo.mAverTimeStamp = _carTimeStamp;
      return true;
    }
  } else {
    LOG(FATAL) << "Impossible situation leftIdx > rightIdx " << TX_VARS(leftIdx) << TX_VARS(rightIdx)
               << TX_VARS(mSpaceTimeCarList.size());
    refKinectInfo.Clear();
    return false;
  }
}

Base::txBool txL2W_Car_FrameAware_TrajMgr::compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Car& refCarInfo,
                                                       Base::txFloat& refVelocity, Base::txFloat& refAcc) TX_NOEXCEPT {
  if (EnableFilling(timeMgr)) {
    const Base::txFloat halfStep = timeMgr.RelativeTime() * 0.5;
    const Base::txDuration curDuration(Utils::SecondToMillisecond(timeMgr.PassTime() - halfStep),
                                       Utils::SecondToMillisecond(timeMgr.PassTime() + timeMgr.RelativeTime()));

    Base::txInt idx = 0;
    for (; idx < mSpaceTimeCarList.size(); ++idx) {
      const tx_L2W_SpaceTimeCar& refNode = mSpaceTimeCarList[idx];

      if (CallSucc(refNode.IsNotSend()) &&
          Math::isInScope(curDuration.left(), curDuration.right(), (refNode).RsvTime())) {
        LOG(WARNING) << TX_VARS(curDuration.left()) << TX_VARS(curDuration.right()) << TX_COND(refNode.IsNotSend())
                     << TX_VARS((refNode).RsvTime());
        /*find non send data in duration*/
        break;
      }
    }

    if (idx < mSpaceTimeCarList.size()) {
      tx_L2W_SpaceTimeCar& refNode = mSpaceTimeCarList[idx];
      refNode.SetBeSend();
      refCarInfo.CopyFrom(refNode.RawCar());

      if (update_kinect(idx, idx + 1, refCarInfo, timeMgr.AbsTime(), mKinectInfo)) {
        refVelocity = mKinectInfo.mAverSpeed;
        refAcc = mKinectInfo.mAverAcc;
        return true;
      } else {
        mValid = false;
        return false;
      }
    }
    return false;
  } else {
    return false;
  }
}

void txL2W_Car_FrameAware_TrajMgr::Clear() TX_NOEXCEPT {
  ParentClass::Clear();
  mSpaceTimeCarList.clear();
}

#endif /*__TX_Mark__("txL2W_Car_FrameAware_TrajMgr")*/

#if __TX_Mark__("txL2W_Car_SmoothAware_TrajMgr")
Base::txBool txL2W_Car_SmoothAware_TrajMgr::EnableFilling(Base::TimeParamManager const& timeMgr) const TX_NOEXCEPT {
  if (IsValid() && _NonEmpty_(mSpaceTimeCarList)) {
    /*have element which not be sended, but it does not mean send at current frame*/
    return (mSpaceTimeCarList.front().RsvTime() <= Utils::SecondToMillisecond(timeMgr.AbsTime())) &&
           (Utils::SecondToMillisecond(timeMgr.AbsTime()) <= mSpaceTimeCarList.back().RsvTime());
  } else {
    return false;
  }
}

Base::txBool txL2W_Car_SmoothAware_TrajMgr::compute_pos(Base::TimeParamManager const& timeMgr, sim_msg::Car& refCarInfo,
                                                        Base::txFloat& refVelocity, Base::txFloat& refAcc) TX_NOEXCEPT {
  if (EnableFilling(timeMgr)) {
    const Base::txFloat time_ms = Utils::SecondToMillisecond(timeMgr.AbsTime());
    for (Base::txInt idx = 1; idx < mSpaceTimeCarList.size(); ++idx) {
      const tx_L2W_SpaceTimeCar& left_node = mSpaceTimeCarList[idx - 1];
      const tx_L2W_SpaceTimeCar& right_node = mSpaceTimeCarList[idx];

      if (left_node.RsvTime(/*left close*/) <= time_ms && time_ms < right_node.RsvTime(/*right open*/)) {
        /*LOG(WARNING) << TX_VARS(idx) << TX_VARS(left_node.RsvTime())
            << TX_VARS(time_ms) << TX_VARS(right_node.RsvTime());*/
        refCarInfo.CopyFrom(TX_MARK("interp base on enu")
                                interp(left_node.RsvTime(/*left close*/), right_node.RsvTime(/*right open*/),
                                       left_node.RawCar(), right_node.RawCar(), time_ms));
        if (update_kinect(idx - 1, idx, refCarInfo, time_ms, mKinectInfo)) {
          refVelocity = mKinectInfo.mAverSpeed;
          refAcc = mKinectInfo.mAverAcc;
          return true;
        } else {
          mValid = false;
          return false;
        }
      }
    }
    refCarInfo.CopyFrom(mSpaceTimeCarList.back().RawCar());
    refVelocity = mKinectInfo.mAverSpeed;
    refAcc = mKinectInfo.mAverAcc;
    mKinectInfo.mAverTimeStamp = time_ms;
    return true;
  } else {
    mKinectInfo.Clear();
    refVelocity = mKinectInfo.mAverSpeed;
    refAcc = mKinectInfo.mAverAcc;
    return false;
  }
}

TrafficFlow::txTrajManagerBasePtr txL2W_Car_SmoothAware_TrajMgr::clone_traj() TX_NOEXCEPT {
  TrafficFlow::txTrajManagerBasePtr ret_ptr = std::make_shared<txL2W_Car_SmoothAware_TrajMgr>();
  if (NonNull_Pointer(ret_ptr)) {
    ret_ptr->add_new_pos(mSrcL2wCarList);
    return ret_ptr;
  } else {
    return nullptr;
  }
}

#endif /*__TX_Mark__("txL2W_Car_SmoothAware_TrajMgr")*/

#if __TX_Mark__("txL2W_Location_FrameAware_TrajMgr")
Base::txBool txL2W_Location_FrameAware_TrajMgr::EnableFilling(Base::TimeParamManager const&) const TX_NOEXCEPT {
  if (IsValid() && _NonEmpty_(mSpaceTimeLocationList)) {
    /*have element which not be sended, but it does not mean send at current frame*/
    return mSpaceTimeLocationList.back().IsNotSend();
  } else {
    return false;
  }
}

Base::txBool txL2W_Location_FrameAware_TrajMgr::add_new_pos(const txSpaceTimeLocationList& l2w_ego_list) TX_NOEXCEPT {
  TX_MARK("avoid clear mSpaceTimeCarList");
  mSrcL2wEgoList = l2w_ego_list;
  for (const auto& refNode : l2w_ego_list) {
    mSpaceTimeLocationList.emplace_back(tx_L2W_SpaceTimeLocation(refNode));
  }
  return true;
}

TrafficFlow::txTrajManagerBasePtr txL2W_Location_FrameAware_TrajMgr::clone_traj() TX_NOEXCEPT {
  TrafficFlow::txTrajManagerBasePtr ret_ptr = std::make_shared<txL2W_Location_FrameAware_TrajMgr>();
  if (NonNull_Pointer(ret_ptr)) {
    ret_ptr->add_new_pos(mSrcL2wEgoList);
    return ret_ptr;
  } else {
    return nullptr;
  }
}

Base::txBool txL2W_Location_FrameAware_TrajMgr::compute_pos(Base::TimeParamManager const& timeMgr,
                                                            sim_msg::Location& refEgoInfo) TX_NOEXCEPT {
  if (EnableFilling(timeMgr)) {
    const Base::txFloat halfStep = timeMgr.RelativeTime() * 0.5;
    const Base::txDuration curDuration(timeMgr.PassTime() - halfStep, timeMgr.PassTime() + timeMgr.RelativeTime());

    auto itr = mSpaceTimeLocationList.begin();
    for (; itr != mSpaceTimeLocationList.end(); ++itr) {
      if (CallSucc(itr->IsNotSend()) && Math::isInScope(curDuration.left(), curDuration.right(), (*itr).RsvTime())) {
        /*find non send data in duration*/
        itr->SetBeSend();
        refEgoInfo.CopyFrom(itr->RawLocation());
        itr = mSpaceTimeLocationList.erase(mSpaceTimeLocationList.begin(), itr);
        return true;
      }
    }
    return false;
  } else {
    return false;
  }
}

void txL2W_Location_FrameAware_TrajMgr::Clear() TX_NOEXCEPT {
  ParentClass::Clear();
  mSpaceTimeLocationList.clear();
}
#endif /*__TX_Mark__("txL2W_Location_FrameAware_TrajMgr")*/

#if __TX_Mark__("txL2W_Location_SmoothAware_TrajMgr")

Base::txBool txL2W_Location_SmoothAware_TrajMgr::EnableFilling(Base::TimeParamManager const& timeMgr) const
    TX_NOEXCEPT {
  if (IsValid() && _NonEmpty_(mSpaceTimeLocationList)) {
    /*have element which not be sended, but it does not mean send at current frame*/
    return (mSpaceTimeLocationList.front().RsvTime() <= Utils::SecondToMillisecond(timeMgr.AbsTime())) &&
           (Utils::SecondToMillisecond(timeMgr.AbsTime()) <= mSpaceTimeLocationList.back().RsvTime());
  } else {
    return false;
  }
}

Base::txBool txL2W_Location_SmoothAware_TrajMgr::compute_pos(Base::TimeParamManager const& timeMgr,
                                                             sim_msg::Location& refLocationInfo) TX_NOEXCEPT {
  if (EnableFilling(timeMgr)) {
    const Base::txFloat time_ms = Utils::SecondToMillisecond(timeMgr.AbsTime());
    for (Base::txInt idx = 1; idx < mSpaceTimeLocationList.size(); ++idx) {
      if (mSpaceTimeLocationList[idx - 1].RsvTime(/*left close*/) <= time_ms &&
          time_ms < mSpaceTimeLocationList[idx].RsvTime(/*right open*/)) {
        refLocationInfo.CopyFrom(interp(mSpaceTimeLocationList[idx - 1].RsvTime(/*left close*/),
                                        mSpaceTimeLocationList[idx].RsvTime(/*right open*/),
                                        mSpaceTimeLocationList[idx - 1].RawLocation(),
                                        mSpaceTimeLocationList[idx].RawLocation(), time_ms));

        mSpaceTimeLocationList.erase(mSpaceTimeLocationList.begin(), (mSpaceTimeLocationList.begin() + (idx - 1)));
        return true;
      }
    }

    if (mSpaceTimeLocationList.front().RsvTime() <= time_ms && time_ms <= mSpaceTimeLocationList.back().RsvTime()) {
      TX_MARK("1. only single sample; 2. have least one duration, and at right boundary");
      refLocationInfo.CopyFrom(mSpaceTimeLocationList.back().RawLocation());
      return true;
    } else {
      mSpaceTimeLocationList.clear();
      return false;
    }
  } else {
    return false;
  }
}

TrafficFlow::txTrajManagerBasePtr txL2W_Location_SmoothAware_TrajMgr::clone_traj() TX_NOEXCEPT {
  TrafficFlow::txTrajManagerBasePtr ret_ptr = std::make_shared<txL2W_Location_SmoothAware_TrajMgr>();
  if (NonNull_Pointer(ret_ptr)) {
    ret_ptr->add_new_pos(mSrcL2wEgoList);
    return ret_ptr;
  } else {
    return nullptr;
  }
}

#endif /*__TX_Mark__("txL2W_Location_SmoothAware_TrajMgr")*/
TX_NAMESPACE_CLOSE(TrafficFlow)
