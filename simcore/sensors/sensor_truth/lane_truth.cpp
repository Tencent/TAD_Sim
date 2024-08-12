/**
 * @file LaneTruth.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "lane_truth.h"
#include <Eigen/Dense>
#include <fstream>
#include <functional>
#include <iostream>
#include "user_topic.h"
#include "common/coord_trans.h"
#include "detectedlane.pb.h"
#include "location.pb.h"
#include "mapengine/hadmap_codes.h"
#include "structs/hadmap_curve.h"
#include "traffic.pb.h"

#ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623158e+308
#endif  // !

void LaneTruth::Init(tx_sim::InitHelper &helper) {
  std::cout << "LaneTruth init.\n";
  helper.Subscribe(tx_sim::topic::kLocation);
  helper.Publish(tx_sim::user_topic::kLaneTruth);
  std::string value = helper.GetParameter("maxDetectLenght");
  if (!value.empty()) {
    maxDetectLen = std::atoi(value.c_str()) > 0;
  }
  value = helper.GetParameter("nearestLimit");
  if (!value.empty()) {
    nearestLimit = std::atoi(value.c_str()) > 0;
  }
  std::cout << "maxDetectLenght = " << maxDetectLen << std::endl;
  std::cout << "nearestLimit = " << nearestLimit << std::endl;
}

void LaneTruth::Reset(tx_sim::ResetHelper &helper) { InitMap(); }

void LaneTruth::Step(tx_sim::StepHelper &helper) {
  sim_msg::Location loc = getLocation(helper);

  FovFilterSensor fov(0);
  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);

  fov.setCarPosition(locPosition);
  fov.setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
  std::cout << "[LaneTruth(" << helper.timestamp() << ")]: ";

  hadmap::txLaneId egoLaneid;
  hadmap::txLanePtr lanePtr;
  if (GetEgoLane(hadmap::txPoint(loc.position().x(), loc.position().y(), loc.position().z()), lanePtr)) {
    egoLaneid = lanePtr->getTxLaneId();
    lastLid = egoLaneid;
  } else {
    const auto &lastLane = mapLanes[lastLid];
    if (lastLane.next.empty()) {
      std::cout << "cannot locate lane\n";

      sim_msg::DetectedLaneBoundarySeries lanebdys;
      lanebdys.add_left()->set_valid(false);
      lanebdys.add_left()->set_valid(false);
      lanebdys.add_right()->set_valid(false);
      lanebdys.add_right()->set_valid(false);
      std::string buf;
      lanebdys.SerializeToString(&buf);
      helper.PublishMessage(tx_sim::user_topic::kLaneTruth, buf);

      lbdyAge.YearEnd();
      return;
    } else {
      egoLaneid = lastLane.next.front();
    }
  }

  sim_msg::DetectedLaneBoundarySeries lanebdys;

#ifdef DEBUG_OUT
  auto lineoutput = [&](const EPointList &pts, int id) {
    std::string fname = std::string("h:/tmp/lines") + std::to_string(id) + ".0.bmp";
    int w = 60, h = 60;
    std::vector<uint8_t> buf(w * h, 0);
    for (const auto &i : pts) {
      int x = i.x();
      int y = i.y() + 30;
      if (x >= 0 && x < w && y >= 0 && y < h) {
        buf.at(y * w + x) = 255;
      }
    }
    SaveBMP(fname, buf, w, h);
  };
  auto coeoutput = [&](double coe[4], double bg, double end, int id) {
    std::string fname = std::string("h:/tmp/lines") + std::to_string(id) + ".1.bmp";
    int w = 60, h = 60;
    std::vector<uint8_t> buf(w * h, 0);
    int b = std::max(0, static_cast<int>(bg));
    int e = std::min(w - 1, static_cast<int>(end));
    for (int i = b; i < e; i++) {
      double iy = coe[0] * i * i * i;
      iy += coe[1] * i * i;
      iy += coe[2] * i;
      iy += coe[3];
      iy = -iy;
      iy += 30;
      int y = static_cast<int>(iy);
      if (y >= 0 && y < h) {
        buf.at(y * w + i) = 255;
      }
    }
    SaveBMP(fname, buf, w, h);
  };
#endif  // DEBUG_OUT

  detect(fov, lanebdys, egoLaneid, true, 1);
  detect(fov, lanebdys, egoLaneid, false, 2);
  detect(fov, lanebdys, mapLanes[egoLaneid].leftLane, true, 3);
  detect(fov, lanebdys, mapLanes[egoLaneid].rightLane, false, 4);
  // cal lane width
  if (lanebdys.left_size() > 0 && lanebdys.right_size() > 0) {
    if (lanebdys.left(0).valid() && lanebdys.right(0).valid()) {
      double d = lanebdys.left(0).line_coefficient(3);
      double c = lanebdys.left(0).line_coefficient(2);
      double l = std::abs(d * std::cos(std::atan(c)));
      d = lanebdys.right(0).line_coefficient(3);
      c = lanebdys.right(0).line_coefficient(2);
      l += std::abs(d * std::cos(std::atan(c)));
      lanebdys.set_host_lane_width(l);
    }
  }

  std::string buf;
  lanebdys.SerializeToString(&buf);
  helper.PublishMessage(tx_sim::user_topic::kLaneTruth, buf);

  lbdyAge.YearEnd();
  std::cout << "\n";
}

void LaneTruth::Stop(tx_sim::StopHelper &helper) {}

bool LaneTruth::selectpts(FovFilterSensor &fov, const EPointList &orip, EPointList &dstp, double &slen, double &len) {
  for (size_t i = 0, is = orip.size(); i < is; i++) {
    bool isbg = false;
    Eigen::Vector3d np = fov.FovVectorWithoutSelfRot(orip[i]);
    np.z() = 0;  // ego always has zero z
    // only select front points
    if (np.x() < 0) {
      if (isbg)
        return true;
      else
        continue;
    }
    if (len > maxDetectLen) {
      return true;
    }
    isbg = true;
    if (dstp.empty()) {
      len = sqrt(np.x() * np.x() + std::pow(std::abs(np.y() - 1.7), 2));
      slen = len;
      if (len > nearestLimit) {
        return true;
      }
    } else {
      len += (np - dstp.back()).norm();
    }
    dstp.push_back(np);
  }
  return len > maxDetectLen;
}

void LaneTruth::collectpts(FovFilterSensor &fov, const hadmap::txLaneId &laneid, EPointList &dstp, bool isleft,
                           double &dstart, double &dend, hadmap::laneboundarypkid &fstBid) {
  dend = 0;
  fstBid = BOUN_PKID_INVALID;
  bool isStop = false;
  const Lane &lane = mapLanes[laneid];
  if (isleft) {
    if (lane.left < mapBounds.size() && !mapBounds.at(lane.left).pts.empty()) {
      fstBid = lane.left;
      isStop = selectpts(fov, mapBounds.at(lane.left).pts, dstp, dstart, dend);
    }
  } else {
    if (lane.right < mapBounds.size() && !mapBounds.at(lane.right).pts.empty()) {
      fstBid = lane.right;
      isStop = selectpts(fov, mapBounds.at(lane.right).pts, dstp, dstart, dend);
    }
  }
  if (lane.next.empty()) return;
  auto nxLaneid = lane.next.front();
  while (!isStop) {
    const Lane &nxlane = mapLanes[nxLaneid];
    if (isleft) {
      if (nxlane.left >= mapBounds.size() || mapBounds.at(nxlane.left).pts.empty()) {
        return;
      }
      if (fstBid == BOUN_PKID_INVALID) fstBid = nxlane.left;
      isStop = selectpts(fov, mapBounds.at(nxlane.left).pts, dstp, dstart, dend);
    } else {
      if (nxlane.right >= mapBounds.size() || mapBounds.at(nxlane.right).pts.empty()) {
        return;
      }
      if (fstBid == BOUN_PKID_INVALID) fstBid = nxlane.right;
      isStop = selectpts(fov, mapBounds.at(nxlane.right).pts, dstp, dstart, dend);
    }
    if (nxlane.next.empty()) return;
    nxLaneid = nxlane.next.front();
  }
}

bool LaneTruth::cal3c(const EPointList &orip, double coe[4]) {
  if (orip.size() < 4) {
    return false;
  }
  Eigen::MatrixXd A = Eigen::MatrixXd::Zero(orip.size(), 4);
  Eigen::VectorXd b = Eigen::VectorXd::Zero(orip.size());
  for (size_t i = 0; i < orip.size(); i++) {
    // left hand
    b(i) = -orip[i].y();
    double px = orip[i].x();
    A(i, 3) = 1;
    for (int j = 2; j >= 0; j--) {
      A(i, j) = A(i, j + 1) * px;
    }
  }
  Eigen::Vector4d x = A.colPivHouseholderQr().solve(b);
  // Eigen::Vector4d x = A.jacobiSvd(Eigen::ComputeThinU |
  // Eigen::ComputeThinV).solve(b);
  coe[0] = x(0);
  coe[1] = x(1);
  coe[2] = x(2);
  coe[3] = x(3);
  return true;
}

bool LaneTruth::detect(FovFilterSensor &fov, sim_msg::DetectedLaneBoundarySeries &lanebdys,
                       const hadmap::txLaneId &laneid, bool isleft, int id) {
  EPointList lines;
  double maxdis = 0, dstart = 0;
  hadmap::laneboundarypkid bid;
  collectpts(fov, laneid, lines, isleft, dstart, maxdis, bid);
#ifdef DEBUG_OUT
  lineoutput(lines, id);
#endif  // DEBUG_OUT
  auto *newbdy = isleft ? lanebdys.add_left() : lanebdys.add_right();
  newbdy->set_valid(false);
  double coe[4] = {0};
  if (cal3c(lines, coe) && bid < mapBounds.size()) {
#ifdef DEBUG_OUT
    coeoutput(coe, dstart, maxdis, id);
#endif
    std::cout << id << ", ";
    const auto &bd = mapBounds[bid];
    newbdy->set_valid(true);
    newbdy->set_track_id(id);
    newbdy->set_age(lbdyAge(id));
    newbdy->set_start(dstart);
    newbdy->set_end(maxdis);
    newbdy->set_is_crossing(0);
    for (const auto &p : lines) {
      const double carlen2 = 3;
      const double carwid2 = 1;
      if (p.x() > carlen2) {
        break;
      }
      if (abs(p.y()) < carwid2) {
        newbdy->set_is_crossing(1);
        break;
      }
    }
    newbdy->set_width(0.14);
    newbdy->set_width_std(0);
    newbdy->set_dash_length(0);
    newbdy->set_dash_gap(0);
    newbdy->add_line_coefficient(coe[0]);
    newbdy->add_line_coefficient(coe[1]);
    newbdy->add_line_coefficient(coe[2]);
    newbdy->add_line_coefficient(coe[3]);
    newbdy->add_line_coefficient_std(0);
    newbdy->add_line_coefficient_std(0);
    newbdy->add_line_coefficient_std(0);
    newbdy->add_line_coefficient_std(0);
    auto *candidt = newbdy->add_candidate();
    candidt->set_probability(1);
    candidt->set_classification(1);
    candidt->set_double_line_type(0);
    candidt->set_decel_type(0);
    if ((bd.mark & hadmap::LANE_MARK_SolidSolid) || (bd.mark & hadmap::LANE_MARK_SolidBroken) ||
        (bd.mark & hadmap::LANE_MARK_BrokenSolid) || (bd.mark & hadmap::LANE_MARK_BrokenBroken)) {
      newbdy->set_width(0.32);
      candidt->set_classification(3);
      if (bd.mark & hadmap::LANE_MARK_SolidSolid) {
        candidt->set_double_line_type(3);
      } else if (bd.mark & hadmap::LANE_MARK_SolidBroken) {
        candidt->set_double_line_type(isleft ? 2 : 1);
      } else if (bd.mark & hadmap::LANE_MARK_BrokenSolid) {
        candidt->set_double_line_type(isleft ? 1 : 2);
      } else if (bd.mark & hadmap::LANE_MARK_BrokenBroken) {
        candidt->set_double_line_type(4);
      }
    }
    if ((bd.mark & hadmap::LANE_MARK_Broken) || (bd.mark & hadmap::LANE_MARK_Broken2) ||
        (bd.mark & hadmap::LANE_MARK_BrokenBroken)) {
      newbdy->set_dash_length(6);
      newbdy->set_dash_gap(9);
      candidt->set_classification(2);
    }
    candidt->set_color(((bd.mark & hadmap::LANE_MARK_Yellow) || (bd.mark & hadmap::LANE_MARK_Yellow2) ||
                        (bd.mark & hadmap::LANE_MARK_YellowYellow))
                           ? 2
                           : 1);
    candidt->set_classification(((bd.mark & hadmap::LANE_MARK_Yellow) || (bd.mark & hadmap::LANE_MARK_Yellow2) ||
                                 (bd.mark & hadmap::LANE_MARK_YellowYellow))
                                    ? 2
                                    : 1);

    return true;
  }
  return false;
}

using namespace hadmap;

void LaneTruth::calLen(Boundary &bds) {
  bds.lenght = 0;
  for (size_t i = 1; i < bds.pts.size(); i++) {
    bds.lenght += (bds.pts[i] - bds.pts[i - 1]).norm();
  }
}
void LaneTruth::soline(EPointList &pts) {
  double len = 1;
  if (pts.size() < 2) return;
  EPointList npts;
  for (size_t i = 0, is = pts.size(); i < is; i++) {
    const auto &p = pts[i];
    if (i == 0) {
      npts.push_back(p);
    } else if (i == is - 1) {
      npts.push_back(p);
    } else {
      double d = (p - npts.back()).norm();
      if (d > len) {
        npts.push_back(p);
      }
    }
  }
  pts.swap(npts);
  len *= 2;
  Eigen::Vector3d pp = pts.front();
  for (auto it = pts.begin() + 1; it != pts.end(); ++it) {
    double l = (*it - pp).norm();
    if (l > len) {
      int c = static_cast<int>(std::floor(l / len));
      Eigen::Vector3d nor = (*it - pp) / (c + 1);

      for (int i = 1; i <= c; i++) {
        it = pts.insert(it, pp + nor * i);
        it++;
      }
    }
    pp = *it;
  }
}

void LaneTruth::calnormal(const PointVec &pts, Eigen::Vector3d &begin, Eigen::Vector3d &end) {
  if (pts.size() > 1) {
    Eigen::Vector3d p0(pts[0].x, pts[0].y, pts[0].z);
    Eigen::Vector3d p1(pts[1].x, pts[1].y, pts[1].z);
    coord_trans_api::lonlat2enu(p0.x(), p0.y(), p0.z(), map_ori.x, map_ori.y, map_ori.z);
    coord_trans_api::lonlat2enu(p1.x(), p1.y(), p1.z(), map_ori.x, map_ori.y, map_ori.z);
    begin = (p1 - p0).normalized();
    p0 = Eigen::Vector3d(pts[pts.size() - 2].x, pts[pts.size() - 2].y, pts[pts.size() - 2].z);
    p1 = Eigen::Vector3d(pts[pts.size() - 1].x, pts[pts.size() - 1].y, pts[pts.size() - 1].z);
    coord_trans_api::lonlat2enu(p0.x(), p0.y(), p0.z(), map_ori.x, map_ori.y, map_ori.z);
    coord_trans_api::lonlat2enu(p1.x(), p1.y(), p1.z(), map_ori.x, map_ori.y, map_ori.z);
    end = (p1 - p0).normalized();
  }
}

size_t LaneTruth::addbound(txLaneBoundaryPtr bd, std::map<laneboundarypkid, size_t> &boudidxmap) {
  if (boudidxmap.find(bd->getId()) == boudidxmap.end()) {
    boudidxmap[bd->getId()] = mapBounds.size();
    mapBounds.push_back(Boundary());
    Boundary &bds = mapBounds.back();
    bds.mark = bd->getLaneMark();
    PointVec pts;
    if (bd->getGeometry()) {
      dynamic_cast<const txLineCurve *>(bd->getGeometry())->getPoints(pts);
      for (auto &p : pts) {
        coord_trans_api::lonlat2enu(p.x, p.y, p.z, map_ori.x, map_ori.y, map_ori.z);
        bds.pts.push_back(Eigen::Vector3d(p.x, p.y, p.z));
      }
      soline(bds.pts);
      calLen(bds);
    }
  }
  return boudidxmap[bd->getId()];
}

void LaneTruth::calLinkBdy(Boundary &bdy, const EPointList &cenPts, const EPointList &prl, const EPointList &nxl,
                           double arot) {
  assert(cenPts.size() > 1);
  bdy.mark = LANE_MARK_None;
  assert(!nxl.empty());
  assert(!prl.empty());
  double wid0 = 1.75, wid1 = 1.75;
  EPointList linkPts = cenPts;
  soline(linkPts);
  if (!prl.empty()) {
    wid0 = (prl.back() - linkPts.front()).norm();
    bdy.pts.push_back(prl.back());
  }
  if (!nxl.empty()) {
    wid1 = (nxl.front() - linkPts.back()).norm();
  }
  if (linkPts.size() > 2) {
    for (size_t i = 1, is = linkPts.size() - 1; i < is; i++) {
      Eigen::Vector3d nor =
          ((linkPts[i] - linkPts[i - 1]).normalized() + (linkPts[i + 1] - linkPts[i]).normalized()) * 0.5;
      nor *= (wid0 * (is - i) + wid1 * (i)) / is;
      bdy.pts.push_back(nor);
      rotate(bdy.pts.back().x(), bdy.pts.back().y(), arot);
      bdy.pts.back() += linkPts[i];
    }
  }
  if (!nxl.empty()) {
    bdy.pts.push_back(nxl.front());
  }
  calLen(bdy);
}
void LaneTruth::sortLanes() {
  // sort
  for (auto &lane : mapLanes) {
    if (lane.second.prev.size() > 1) {
      std::sort(lane.second.prev.begin(), lane.second.prev.end(),
                [&](const txLaneId &rhs1, const txLaneId &rhs2) -> bool {
                  return lane.second.normBegin.dot(mapLanes[rhs1].normEnd) >
                         lane.second.normBegin.dot(mapLanes[rhs1].normEnd);
                });
    }
    if (lane.second.next.size() > 1) {
      std::sort(lane.second.next.begin(), lane.second.next.end(),
                [&](const txLaneId &rhs1, const txLaneId &rhs2) -> bool {
                  return lane.second.normEnd.dot(mapLanes[rhs1].normBegin) >
                         lane.second.normEnd.dot(mapLanes[rhs1].normBegin);
                });
    }
  }
#ifdef DEBUG_OUT

  {
    std::ofstream obj("H:/tmp/laneboundary.obj");
    size_t iii = 0;
    for (const auto &by : mapBounds) {
      for (const auto &i : by.pts) {
        char ch[1000];
        sprintf_s(ch, "%f  %f", i.x(), i.y());
        obj << "v " << ch << " 0" << std::endl;
      }

      for (size_t i = 1; i < by.pts.size(); i++) {
        char ch[1000];
        sprintf_s(ch, "%d  %d", iii + i, iii + i + 1);
        obj << "l " << ch << std::endl;
      }
      iii += by.pts.size();
    }

    obj.close();
  }

#endif  // 1
}

void LaneTruth::InitMap_s1(hadmap::roadpkid &roadlink0, std::map<hadmap::laneboundarypkid, size_t> &boudidxmap) {
  for (auto &road_ptr : mapRoads) {
    roadlink0 = std::max(roadlink0, road_ptr->getId());
    const txSections &sections = road_ptr->getSections();

    for (auto &section_ptr : sections) {
      const txLanes &lanes = section_ptr->getLanes();
      for (size_t i = 0, is = lanes.size(); i < is; i++) {
        auto &lane = lanes[i];
        auto &mln = mapLanes[lane->getTxLaneId()];
        // set left right lane
        if (i > 0) {
          mln.leftLane = lanes[i - 1]->getTxLaneId();
          assert(lane->getId() == lanes[i - 1]->getId() - 1);
        }
        if (i < is - 1) {
          mln.rightLane = lanes[i + 1]->getTxLaneId();
          assert(lane->getId() == lanes[i + 1]->getId() + 1);
        }
        mln.left = addbound(lane->getLeftBoundary(), boudidxmap);
        mln.right = addbound(lane->getRightBoundary(), boudidxmap);

        const txLineCurve *line = dynamic_cast<const txLineCurve *>(lane->getGeometry());
        if (line) {
          PointVec pts;
          line->getPoints(pts);
          calnormal(pts, mln.normBegin, mln.normEnd);
        }
        txLanes oLanes;
        if (getNextLanes(hMapHandle, lane, oLanes) == TX_HADMAP_DATA_OK) {
          for (const auto &ol : oLanes) {
            mln.next.push_back(ol->getTxLaneId());
          }
        }
        oLanes.clear();
        if (getPrevLanes(hMapHandle, lane, oLanes) == TX_HADMAP_DATA_OK) {
          for (const auto &ol : oLanes) {
            mln.prev.push_back(ol->getTxLaneId());
          }
        }
      }
    }
  }
}

std::map<hadmap::roadpkid, LaneTruth::Roadlk> LaneTruth::InitMap_s2_rlink(const hadmap::txRoadPtr &road_ptr) {
  std::map<roadpkid, Roadlk> rdlks;
  if (!road_ptr->getSections().empty()) {
    for (auto &lane_ptr : road_ptr->getSections().back()->getLanes()) {
      txLaneLinks olinks;
      if (getNextLaneLinks(hMapHandle, lane_ptr->getTxLaneId(), olinks) == TX_HADMAP_DATA_OK) {
        for (const auto &ol : olinks) {
          if (rdlks.find(ol->toRoadId()) == rdlks.end()) {
            auto na = mapLanes[ol->fromTxLaneId()].normEnd;
            auto nb = mapLanes[ol->toTxLaneId()].normBegin;
            if (na.dot(nb) > -0.7) {
              // 45du ,not uturn
              double sta = std::atan2(na.y(), na.x());
              rotate(nb.x(), nb.y(), -sta);
              Roadlk rlk;
              rlk.sita = std::atan2(nb.y(), nb.x());
              rlk.rlinks.push_back(ol);
              rdlks[ol->toRoadId()] = rlk;
            }
          } else {
            rdlks[ol->toRoadId()].rlinks.push_back(ol);
          }
        }
      }
    }
  }
  return rdlks;
}

void LaneTruth::InitMap_s2(std::map<hadmap::txLaneLinkPtr, int> &linelinks) {
  const int LEFT = 0x01;
  const int RIGHT = 0x10;
  const int BOTH = LEFT | RIGHT;
  for (const auto &road_ptr : mapRoads) {
    std::map<roadpkid, Roadlk> rdlks = InitMap_s2_rlink(road_ptr);
    for (auto &rlk : rdlks) {
      std::set<roadpkid> rds;
      for (const auto &lk : rlk.second.rlinks) {
        txLaneLinks olinks;
        if (getPrevLaneLinks(hMapHandle, lk->toTxLaneId(), olinks) == TX_HADMAP_DATA_OK) {
          for (const auto &ol : olinks) {
            if (rds.find(ol->toRoadId()) == rds.end()) {
              auto na = mapLanes[ol->fromTxLaneId()].normEnd;
              auto nb = mapLanes[ol->toTxLaneId()].normBegin;
              if (na.dot(nb) > -0.7) {
                // 45du ,not uturn
                if (lk->fromRoadId() != ol->fromRoadId()) {
                  double sta = std::atan2(na.y(), na.x());
                  rotate(nb.x(), nb.y(), -sta);
                  sta = std::atan2(nb.y(), nb.x());
                  rds.insert(ol->toRoadId());
                  rlk.second.others.push_back(sta);
                }
              }
            }
          }
        }
      }
    }

    if (rdlks.empty()) continue;
    if (rdlks.size() == 1 && rdlks.begin()->second.others.empty()) {
      for (const auto &lk : rdlks.begin()->second.rlinks) {
        linelinks.insert(std::make_pair(lk, BOTH));
      }
    } else {
      bool hasleft = false, hasright = false;
      bool rightunique = false;
      const double lrth = EIGEN_PI * 0.25;

      for (const auto &rlk : rdlks) {
        if (rlk.second.sita > lrth) {
          hasleft = true;
        }
        if (rlk.second.sita < -lrth) {
          hasright = true;
          if (rlk.second.others.empty()) {
            rightunique = true;
          }
        }
      }

      for (const auto &rlk : rdlks) {
        for (const auto &lk : rlk.second.rlinks) {
          linelinks.insert(std::make_pair(lk, 0));
        }

        // only straight ~~~~~~~may be
        if (abs(rlk.second.sita) > lrth) {
          continue;
        }
        if (hasleft && hasright) {
          continue;
        }
        if (!hasleft && !hasright) {
          bool hasl = false, hasr = false;
          for (double sita : rlk.second.others) {
            if (sita > rlk.second.sita) {
              hasl = true;
            }
            if (sita < rlk.second.sita) {
              hasr = true;
            }
          }
          if (!hasl && !hasr) {
            for (const auto &lk : rlk.second.rlinks) {
              linelinks[lk] = BOTH;
            }
          } else {
            if (hasl)
              linelinks[rlk.second.rlinks.back()] = RIGHT;
            else if (hasr)
              linelinks[rlk.second.rlinks.front()] = LEFT;
          }
        } else if (hasleft && !hasright) {
          linelinks[rlk.second.rlinks.back()] = RIGHT;
        } else {  // if(!hasleft && hasright)
          if (rightunique) linelinks[rlk.second.rlinks.front()] = LEFT;
        }
      }
    }
  }
}
bool LaneTruth::InitMap() {
  static hadmap::txMapHandle *maphandle = nullptr;
  if (maphandle == hMapHandle) {
    return true;
  }
  if (!hMapHandle) {
    return false;
  }
  maphandle = hMapHandle;

  std::map<hadmap::laneboundarypkid, size_t> boudidxmap;
  roadpkid roadlink0 = 0;
  InitMap_s1(roadlink0, boudidxmap);
  std::map<txLaneLinkPtr, int> linelinks;
  InitMap_s2(linelinks);

  const int LEFT = 0x01;
  const int RIGHT = 0x10;
  const int BOTH = LEFT | RIGHT;

  for (const auto &lk : linelinks) {
    if (lk.first->getGeometry() == nullptr) continue;
    PointVec pts;
    dynamic_cast<const txLineCurve *>(lk.first->getGeometry())->getPoints(pts);
    if (pts.empty()) continue;
    txLaneId lid(roadlink0 + lk.first->getId(), 0, -1);
    Lane &lane = mapLanes[lid];
    calnormal(pts, lane.normBegin, lane.normEnd);
    lane.next.push_back(lk.first->toTxLaneId());
    lane.prev.push_back(lk.first->fromTxLaneId());
    mapLanes[lk.first->toTxLaneId()].prev.push_back(lid);
    mapLanes[lk.first->fromTxLaneId()].next.push_back(lid);

    // has boundary
    std::vector<Eigen::Vector3d> epts;
    for (auto &p : pts) {
      coord_trans_api::lonlat2enu(p.x, p.y, p.z, map_ori.x, map_ori.y, map_ori.z);
      epts.push_back(Eigen::Vector3d(p.x, p.y, p.z));
    }

    // has left
    if (lk.second & LEFT) {
      lane.left = mapBounds.size();
      mapBounds.emplace_back(Boundary());
      calLinkBdy(mapBounds.back(), epts, mapBounds.at(mapLanes[lk.first->fromTxLaneId()].left).pts,
                 mapBounds.at(mapLanes[lk.first->toTxLaneId()].left).pts, EIGEN_PI * 0.5);
    }

    // has right
    if (lk.second & RIGHT) {
      lane.right = mapBounds.size();
      mapBounds.emplace_back(Boundary());
      calLinkBdy(mapBounds.back(), epts, mapBounds.at(mapLanes[lk.first->fromTxLaneId()].right).pts,
                 mapBounds.at(mapLanes[lk.first->toTxLaneId()].right).pts, -EIGEN_PI * 0.5);
    }
  }

  sortLanes();

  std::cout << "Init map success in LaneTruth." << std::endl;

  return true;
}
