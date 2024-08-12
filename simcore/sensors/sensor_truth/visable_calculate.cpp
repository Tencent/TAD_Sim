/**
 * @file VisableCalculate.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "visable_calculate.h"

/**
 * @brief initialize the points to a polygon
 *
 * @param pts input point set
 * @return true when the polygon is valid
 * @return false when the polygon is invalid
 */
bool VisableBuffer::init(const points &pts) {
  polygon ply;
  if (pts.size() > 2) boost::geometry::convex_hull(pts, ply);
  if (ply.outer().empty()) {
    return false;
  }
  bounds.push_back(ply);
  return true;
}

/**
 * @brief update the range of the buffer
 *
 */
void VisableBuffer::update_range() {
  const double CEIL_SIZE = 100.0;
  int bboxX = 1000000;
  int bboxY = 1000000;
  int bboxW = 0;
  int bboxH = 0;
  range.clear();
  for (const auto &ply : bounds) {
    for (const auto &p : ply.outer()) {
      bboxX = std::min<int>(bboxX, static_cast<int>(p.get<0>()));
      bboxY = std::min<int>(bboxY, static_cast<int>(p.get<1>()));
      bboxW = std::max<int>(bboxW, static_cast<int>(p.get<0>()));
      bboxH = std::max<int>(bboxH, static_cast<int>(p.get<1>()));
    }
  }
  for (int y = std::floor(bboxY / CEIL_SIZE), ys = std::ceil(bboxH / CEIL_SIZE); y <= ys; y++) {
    for (int x = std::floor(bboxX / CEIL_SIZE), xs = std::ceil(bboxW / CEIL_SIZE); x <= xs; x++) {
      range.insert(std::make_pair(x, y));
    }
  }
}

/**
 * @brief check
 *
 * @param rhs another buffer
 * @return true if the buffer is covered by another buffer
 * @return false if the buffer is not covered by another buffer
 */
bool VisableBuffer::isCover(const VisableBuffer &rhs) const {
  for (const auto &r : range) {
    if (rhs.range.find(r) != rhs.range.end()) {
      return true;
    }
  }
  return false;
}

/**
 * @brief check if the buffer is visible
 *
 * @param vbs vector of buffers
 * @return true when the buffer is visible
 * @return false when the buffer is not visible
 */
bool ReVisable(std::vector<VisableBuffer> &vbs) {
  if (vbs.empty()) {
    return false;
  }

  for (auto &vb : vbs) {
    vb.update_range();
    vb.area0 = boost::geometry::area(vb.bounds);
  }
  std::sort(vbs.begin(), vbs.end(), [](const auto &rh1, const auto &rh2) { return rh1.distance > rh2.distance; });

  for (size_t i = 1; i < vbs.size(); i++) {
    auto &A = vbs[i];
    for (size_t j = 0; j < i; j++) {
      auto &B = vbs[j];
      if (!A.isCover(B)) {
        continue;
      }
      mpolygon bounds;
      boost::geometry::difference(B.bounds, A.bounds, bounds);
      B.bounds = bounds;
      B.update_range();
    }
  }
  for (auto &vb : vbs) {
    vb.area = boost::geometry::area(vb.bounds);
  }

  // test
  // for (const auto& vb : vbs)
  // {
  // std::cout << vb.object->id << ": " <<
  // boost::geometry::wkt(vb.object->bounds) << std::endl;
  // }

  return true;
}
