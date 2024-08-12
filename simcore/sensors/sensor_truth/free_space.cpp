/**
 * @file FreeSpace.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "free_space.h"
#include <cmath>
#include <fstream>
#include <functional>
#include <iostream>
#include "object_size.h"
#include "user_topic.h"
#include "common/coord_trans.h"
#include "freespace.pb.h"
#include "location.pb.h"
#include "structs/hadmap_curve.h"
#include "traffic.pb.h"
#include "union.pb.h"

#ifndef DBL_MAX
#  define DBL_MAX 1.7976931348623158e+308
#endif  // !

void FreeSpace::Init(tx_sim::InitHelper &helper) {
  std::cout << "FreeSpace init.\n";
  helper.Subscribe(tx_sim::topic::kTraffic);
  helper.Subscribe(tx_sim::topic::kUnionFlag + tx_sim::topic::kLocation);
  helper.Subscribe(tx_sim::topic::kLocation);

  auto value = helper.GetParameter("freespace_distance");
  if (!value.empty()) {
    maxDis = atof(value.c_str());
  }
  std::cout << "freespace_distance = " << maxDis << std::endl;
  value = helper.GetParameter("freespace_limitZ");
  if (!value.empty()) {
    limitZ = atof(value.c_str());
  }
  std::cout << "freespace_limitZ = " << limitZ << std::endl;

  value = helper.GetParameter("freespace_grid_resolution");
  if (!value.empty()) {
    grid_resolution = std::max(0.01, atof(value.c_str()));
  }
  std::cout << "freespace_grid_resolution = " << grid_resolution << std::endl;

  value = helper.GetParameter("freespace_debug_bmp");
  if (!value.empty()) {
    debugbmp = value;
    std::cout << "freespace_debug_bmp = " << debugbmp << std::endl;
  }

  helper.Publish(tx_sim::user_topic::kFreeSpace);
  helper.Publish(tx_sim::user_topic::kFreeSpaceGrid);
}

void FreeSpace::Reset(tx_sim::ResetHelper &helper) {
  // InitMap();
}

/// @brief
/// @param helper
void FreeSpace::Step(tx_sim::StepHelper &helper) {
  sim_msg::Traffic traffic = getTraffic(helper);
  sim_msg::Location loc = getLocation(helper);
  sim_msg::Union union_location = getUnion(helper);

  Eigen::Vector3d locPosition(loc.position().x(), loc.position().y(), loc.position().z());
  coord_trans_api::lonlat2enu(locPosition.x(), locPosition.y(), locPosition.z(), map_ori.x, map_ori.y, map_ori.z);

  FovFilterSensor fov(0);
  fov.setCarRoatation(loc.rpy().x(), loc.rpy().y(), loc.rpy().z());
  fov.setCarPosition(locPosition);

  const int N2 = N / 2;
  std::vector<double> fs(N, DBL_MAX);

  // grid: not finish
  int grid_width = std::ceil(maxDis / grid_resolution);
  int grid_width2 = grid_width / 2;
  std::vector<std::uint8_t> grid(grid_width * grid_width, 0);
  std::vector<std::pair<int, std::vector<Eigen::Vector3d>>> grid_pts;

  /*{
          hadmap::PointVec pts;
          pts.push_back(hadmap::txPoint(2, 1, 0));
          pts.push_back(hadmap::txPoint(-2, -3, 0));
          bound_free(pts);
  }*/
  double maxDissqurt = maxDis * maxDis * 1.5;

  std::vector<std::pair<size_t, size_t>> indexs;
  std::vector<double> diss;

  double p[2] = {locPosition.x(), locPosition.y()};
  if (bpFlann.SearchRadius(&p[0], indexs, diss, maxDissqurt)) {
    std::map<size_t, std::vector<bool>> searchedline;
    for (const auto &i : indexs) {
      if (searchedline[i.first].empty()) searchedline[i.first].resize(boundPoints.at(i.first).lines.size());
      searchedline[i.first].at(i.second) = true;
    }
    for (const auto &line : searchedline) {
      const auto &pts = boundPoints.at(line.first).lines;
      if (limitZ > 1e-6 && pts.size() > 1 && std::abs(pts.at(pts.size() / 2).z() - locPosition.z()) > limitZ) {
        continue;
      }
      bound_free_ring(fov, fs, pts, line.second);
      // bound_line_grid(pts, boundPoints.at(line.first).left);
      auto gpts = sampling_line(pts);
      if (!gpts.empty()) {
        grid_pts.push_back(std::make_pair(0, gpts));
      }
    }
  }

  for (const auto &msg : union_location.messages()) {
    std::int64_t eid = std::atoi(msg.groupname().substr(msg.groupname().length() - 3).c_str());
    if (eid == ego_id) continue;
    sim_msg::Location uloc;
    if (uloc.ParseFromString(msg.content())) {
      auto &objbb = ego_Bboxes[eid];
      if (objbb.category < 0) {
        std::cout << "Cannot find ego type: " << eid << std::endl;
        continue;
      }
      Eigen::Vector3d pos(uloc.position().x(), uloc.position().y(), uloc.position().z());
      coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
      if ((pos - locPosition).squaredNorm() > maxDissqurt) {
        continue;
      }
      if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
        continue;
      }
      traffic_free(fov, fs, grid_pts, pos, objbb.len, objbb.wid, uloc.rpy().z(), 2);
    }
  }
  for (const auto &car : traffic.cars()) {
    Eigen::Vector3d pos(car.x(), car.y(), car.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if ((pos - locPosition).squaredNorm() > maxDissqurt) {
      continue;
    }
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    traffic_free(fov, fs, grid_pts, pos, car.length(), car.width(), car.heading(), 2);
  }
  for (const auto &obs : traffic.dynamicobstacles()) {
    Eigen::Vector3d pos(obs.x(), obs.y(), obs.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if ((pos - locPosition).squaredNorm() > maxDissqurt) {
      continue;
    }
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    traffic_free(fov, fs, grid_pts, pos, obs.length(), obs.width(), obs.heading(), 2);
  }
  for (const auto &sta : traffic.staticobstacles()) {
    Eigen::Vector3d pos(sta.x(), sta.y(), sta.z());
    coord_trans_api::lonlat2enu(pos.x(), pos.y(), pos.z(), map_ori.x, map_ori.y, map_ori.z);
    if ((pos - locPosition).squaredNorm() > maxDissqurt) {
      continue;
    }
    if (limitZ > 1e-6 && std::abs(pos.z() - locPosition.z()) > limitZ) {
      continue;
    }
    traffic_free(fov, fs, grid_pts, pos, sta.length(), sta.width(), sta.heading(), 0);
  }

  sim_msg::FreeSpaceRing tFreespace;
  tFreespace.mutable_elements()->Reserve(N);
  int cc = 0;
  for (int i = 0; i < N; i++) {
    auto *ele = tFreespace.add_elements();
    ele->set_valid(fs[i] != DBL_MAX);
    if (ele->valid()) cc++;
    double sita = (i - N2) * EIGEN_PI / N2;
    double len = fs[i] != DBL_MAX ? fs[i] : 0;
    double x = len * std::cos(sita) * 100;  // cm
    double y = len * std::sin(sita) * 100;  // cm
    ele->set_x(std::round(x));
    ele->set_y(std::round(y));
  }
  std::cout << "[freespace(" << helper.timestamp() << ")]: " << cc << " valid\n";
  std::string buf;
  tFreespace.SerializeToString(&buf);
  helper.PublishMessage(tx_sim::user_topic::kFreeSpace, buf);

  sim_msg::GridUTM gridout;
  cc = 0;
  gridout.set_timestamp(helper.timestamp());
  gridout.set_fresolution(grid_resolution);
  const double a = 6378137.0;
  const double e2 = 0.006694379990;
  const double d2r = 0.01745329251994329576923690768489;
  for (const auto &obj : grid_pts) {
    for (const auto &p : obj.second) {
      auto *ele = gridout.add_pts();
      ele->set_x(p.x());
      ele->set_y(p.y());
      ele->set_label(obj.first);
      cc++;
    }
  }
  gridout.set_npointsnum(cc);
  gridout.SerializeToString(&buf);
  helper.PublishMessage(tx_sim::user_topic::kFreeSpaceGrid, buf);

  // test
  /*
  if (!debugbmp.empty()) {
    for (const auto &obj : grid_pts) {
      for (const auto &p : obj.second) {
        Eigen::Vector3d lp = p - locPosition;
        int xx = std::round(lp.x() / grid_resolution);
        int yy = std::round(lp.y() / grid_resolution);
        if (xx >= -grid_width2 && xx < grid_width2 && yy >= -grid_width2 && yy < grid_width2) {
          grid[(yy + grid_width2) * grid_width + xx + grid_width2] = obj.first ? 255 : 128;
        }
      }
    }
    SaveBMP(debugbmp, grid, grid_width, grid_width);
  }
  /**/
}

/// @brief
/// @param helper
void FreeSpace::Stop(tx_sim::StopHelper &helper) {}

/// @brief
/// @param fov
/// @param fs
/// @param pts
/// @param tag
void FreeSpace::bound_free_ring(FovFilterSensor &fov, std::vector<double> &fs, const std::vector<Eigen::Vector3d> &pts,
                                const std::vector<bool> &tag) {
  if (pts.size() < 2) {
    return;
  }
  assert(pts.size() == tag.size());
  Eigen::Vector3d pp;
  double pa = -100000;
  for (size_t i = 1; i < pts.size(); i++) {
    if (!tag.at(i) && !tag.at(i - 1)) {
      pa = -100000;
      continue;
    }

    if (pa < 0) {
      pp = fov.FovVectorWithoutSelfRot(pts[i - 1]);
      pp.z() = 0;
      pa = atan2(pp.y(), pp.x()) * 180.0 / EIGEN_PI;
      pa += 180.;
      pa *= N / 360.;
    }
    auto pi = fov.FovVectorWithoutSelfRot(Eigen::Vector3d(pts[i]));
    pi.z() = 0;
    double dis = pi.norm();
    double a = atan2(pi.y(), pi.x()) * 180.0 / EIGEN_PI;
    a += 180.;
    a *= N / 360.;

    if (abs(pa - a) > N / 2) {
      if (pa < a) {
        pa += N;
      } else {
        a += N;
      }
    }
    double ap = pa;
    double aq = a;
    Eigen::Vector3d p = pp;
    Eigen::Vector3d q = pi;
    if (ap > aq) {
      std::swap(ap, aq);
      std::swap(p, q);
    }
    double ar = std::ceil(ap);
    while (ar <= aq) {
      double opq = std::acos(p.normalized().dot((p - q).normalized()));
      double orp = EIGEN_PI - opq - (ar - ap) * EIGEN_PI * 2 / N;
      double orr = p.norm() * std::sin(opq) / std::sin(orp);
      int idx = static_cast<int>(ar) % N;
      fs.at(idx) = std::min(orr, fs.at(idx));

      ar += 1.;
    }

    pa = std::fmod(a, N);
    pp = pi;
  }
}

/// @brief
/// @param pts
/// @return
std::vector<Eigen::Vector3d> FreeSpace::sampling_line(const std::vector<Eigen::Vector3d> &pts) {
  std::vector<Eigen::Vector3d> samplePts;
  if (pts.size() < 2) {
    return samplePts;
  }
  for (int i = 1; i < pts.size(); ++i) {
    auto &p = pts.at(i);
    auto &pp = pts.at(i - 1);
    double len = (p - pp).norm();
    int num = std::ceil(len / grid_resolution);
    if (num == 0) {
      continue;
    }
    Eigen::Vector3d n = (p - pp) / num;
    for (int i = 0; i < num; i++) {
      samplePts.push_back(pp + n * i);
    }
  }
  return samplePts;
}

/// @brief
/// @param pts
/// @return
std::vector<Eigen::Vector3d> FreeSpace::sampling_ply(const std::vector<Eigen::Vector3d> &pts) {
  std::vector<Eigen::Vector3d> samplePts;
  if (pts.size() < 2) {
    return samplePts;
  }
  Eigen::Vector3d pp = pts.back();
  for (const auto &p : pts) {
    double len = (p - pp).norm();
    int num = std::ceil(len / grid_resolution);
    if (num == 0) {
      pp = p;
      continue;
    }
    Eigen::Vector3d n = (p - pp) / num;
    for (int i = 0; i < num; i++) {
      samplePts.push_back(pp + n * i);
    }
    pp = p;
  }
  return samplePts;
}

void FreeSpace::traffic_free(FovFilterSensor &fov, std::vector<double> &fs,
                             std::vector<std::pair<int, std::vector<Eigen::Vector3d>>> &grid_pts,
                             const Eigen::Vector3d pos, double len, double wid, double heading, int label) {
  std::vector<Eigen::Vector3d> pts;
  pts.push_back(Eigen::Vector3d(-len * 0.5, -wid * 0.5, 0));
  pts.push_back(Eigen::Vector3d(-len * 0.5, wid * 0.5, 0));
  pts.push_back(Eigen::Vector3d(len * 0.5, wid * 0.5, 0));
  pts.push_back(Eigen::Vector3d(len * 0.5, -wid * 0.5, 0));
  pts.push_back(Eigen::Vector3d(-len * 0.5, -wid * 0.5, 0));
  for (auto &p : pts) {
    rotate(p.x(), p.y(), heading);
    p += pos;
  }
  bound_free_ring(fov, fs, pts, std::vector<bool>(pts.size(), true));
  auto gpts = sampling_ply(pts);
  if (!gpts.empty()) {
    grid_pts.push_back(std::make_pair(label, gpts));
  }
}

/// @brief
/// @param pts
void FreeSpace::soline(std::vector<Eigen::Vector3d> &pts) {
  const double len = 3;
  if (pts.size() < 2) return;
  auto pp = pts.front();
  for (auto it = pts.begin() + 1; it != pts.end(); ++it) {
    int c = static_cast<int>(std::floor((*it - pp).norm() / len));
    if (c > 1) {
      Eigen::Vector3d nor = (*it - pp) / (c + 1);
      for (int i = 1; i <= c; i++) {
        it = pts.insert(it, pp + nor * i);
      }
    }

    pp = *it;
  }
}

/// @brief
/// @param pts
/// @param type
/// @return
auto FreeSpace::addline(std::vector<Eigen::Vector3d> &pts, int type) {
  soline(pts);
  if (pts.size() < 2) return;
  size_t a = boundPoints.size();
  BoundLine bl;
  bl.lines = pts;
  bl.type = type;
  boundPoints.push_back(bl);
  for (size_t i = 0; i < pts.size(); i++) {
    double p[2] = {pts[i].x(), pts[i].y()};
    bpFlann.Add(p, std::make_pair(a, i));
  }
}

bool FreeSpace::InitMap() {
  using namespace hadmap;
  static hadmap::txMapHandle *maphandle = nullptr;
  if (maphandle == hMapHandle) {
    return true;
  }
  if (!hMapHandle) {
    return false;
  }
  maphandle = hMapHandle;
  bpFlann.Clear();

  for (auto &road_ptr : mapRoads) {
    const txSections &sections = road_ptr->getSections();

    for (auto &section_ptr : sections) {
      // auto& rbound = mapBounds[std::make_pair(road_ptr->getId(),
      // section_ptr->getId())];

      const txLanes &lanes = section_ptr->getLanes();
      const auto &bounds = section_ptr->getBoundaries();

      if (bounds.front()) {
        hadmap::PointVec pts;
        dynamic_cast<const txLineCurve *>(bounds.front()->getGeometry())->getPoints(pts);
        std::vector<Eigen::Vector3d> epts;
        for (auto &p : pts) {
          coord_trans_api::lonlat2enu(p.x, p.y, p.z, map_ori.x, map_ori.y, map_ori.z);
          epts.push_back(Eigen::Vector3d(p.x, p.y, p.z));
        }
        addline(epts, 0);
      }
      if (bounds.back()) {
        hadmap::PointVec pts;
        dynamic_cast<const txLineCurve *>(bounds.back()->getGeometry())->getPoints(pts);
        std::vector<Eigen::Vector3d> epts;
        for (auto &p : pts) {
          coord_trans_api::lonlat2enu(p.x, p.y, p.z, map_ori.x, map_ori.y, map_ori.z);
          epts.push_back(Eigen::Vector3d(p.x, p.y, p.z));
        }
        addline(epts, 0);
      }
      if (!lanes.empty()) {
        txLaneLinks llinks;
        getNextLaneLinks(hMapHandle, lanes.back()->getTxLaneId(), llinks);
        if (!llinks.empty()) {
          hadmap::txLanePtr lane0, lane1;
          getLane(hMapHandle, llinks.back()->fromTxLaneId(), lane0);
          getLane(hMapHandle, llinks.back()->toTxLaneId(), lane1);
          if (lane0 && lane1 && lane0->getRightBoundary() && lane1->getRightBoundary()) {
            assert(lane0->getRightBoundary() == bounds.back());
            auto p0 = dynamic_cast<const txLineCurve *>(lane0->getRightBoundary()->getGeometry())->getEnd();
            auto p1 = dynamic_cast<const txLineCurve *>(lane1->getRightBoundary()->getGeometry())->getStart();
            std::vector<Eigen::Vector3d> epts;
            coord_trans_api::lonlat2enu(p0.x, p0.y, p0.z, map_ori.x, map_ori.y, map_ori.z);
            epts.push_back(Eigen::Vector3d(p0.x, p0.y, p0.z));
            coord_trans_api::lonlat2enu(p1.x, p1.y, p1.z, map_ori.x, map_ori.y, map_ori.z);
            epts.push_back(Eigen::Vector3d(p1.x, p1.y, p1.z));

            addline(epts, 0);
          }
        }
      }
    }
  }
  if (bpFlann.Build()) {
    std::cout << "Init map success in freespace." << std::endl;
  } else {
    std::cout << "Init map faild in freespace." << std::endl;
  }

  return true;
}
