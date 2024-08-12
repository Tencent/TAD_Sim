// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_collision_detection2d.h"
#include <functional>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
#include "tx_logger.h"
#include "tx_math.h"

TX_NAMESPACE_OPEN(Geometry)
TX_NAMESPACE_OPEN(CD2D)

txFloat minimum_distance(const cdVec2& v, const cdVec2& w, const cdVec2& p, cdVec2& ptInSeg) TX_NOEXCEPT {
  const txFloat l2 = (w - v).squaredNorm();
  if (Math::isZero(l2)) {
    ptInSeg = p;
    return (p - v).norm(); /* CVec3::distance3d(p, v);*/
  }
  const txFloat t = Math::Max(0.0, Math::Min(1.0, ((p - v).dot(w - v) / l2 /*CVec3::dot3(p - v, w - v) / l2*/)));
  const cdVec2 projection = v + (w - v) * t;
  ptInSeg = projection;
  return (p - projection).norm(); /* ::distance3d(p, projection);*/
}

txFloat dist3D_Segment_to_Segment(const cdVec2& S1_P0, const cdVec2& S1_P1, const cdVec2& S2_P0, const cdVec2& S2_P1,
                                  cdVec2& ptInS1, cdVec2& ptInS2) TX_NOEXCEPT {
  const txFloat db_EPSINON = Math::EPSILON;
  const cdVec2 u = S1_P1 - S1_P0;
  const cdVec2 v = S2_P1 - S2_P0;
  const cdVec2 w = S1_P0 - S2_P0;
  txFloat a = u.dot(u); /*CVec3::dot3(u, u);*/ /* always >= 0*/
  txFloat b = u.dot(v);                        /*CVec3::dot3(u, v);*/
  txFloat c = v.dot(v); /*CVec3::dot3(v, v);*/ /* always >= 0*/
  txFloat d = u.dot(w);                        /*CVec3::dot3(u, w);*/
  txFloat e = v.dot(w);                        /*CVec3::dot3(v, w);*/
  txFloat D = a * c - b * b;                   /* always >= 0*/
  txFloat sc = 0.0;
  txFloat sN = 0.0;
  txFloat sD = D; /* sc = sN / sD, default sD = D >= 0*/
  txFloat tc = 0.0;
  txFloat tN = 0.0;
  txFloat tD = D; /* tc = tN / tD, default tD = D >= 0*/

  /* compute the line parameters of the two closest points*/
  if (D < db_EPSINON) { /* the lines are almost parallel*/
    sN = 0.0;           /* force using point P0 on segment S1*/
    sD = 1.0;           /* to prevent possible division by 0.0 later*/
    tN = e;
    tD = c;
  } else { /* get the closest points on the infinite lines*/
    sN = (b * e - c * d);
    tN = (a * e - b * d);
    if (sN < 0.0) { /* sc < 0 => the s=0 edge is visible*/
      sN = 0.0;
      tN = e;
      tD = c;
    } else if (sN > sD) { /* sc > 1  => the s=1 edge is visible*/
      sN = sD;
      tN = e + b;
      tD = c;
    }
  }

  if (tN < 0.0) { /* tc < 0 => the t=0 edge is visible*/
    tN = 0.0;
    /* recompute sc for this edge*/
    if (-d < 0.0) {
      sN = 0.0;
    } else if (-d > a) {
      sN = sD;
    } else {
      sN = -d;
      sD = a;
    }
  } else if (tN > tD) { /* tc > 1  => the t=1 edge is visible*/
    tN = tD;
    /* recompute sc for this edge*/
    if ((-d + b) < 0.0) {
      sN = 0;
    } else if ((-d + b) > a) {
      sN = sD;
    } else {
      sN = (-d + b);
      sD = a;
    }
  }
  /* finally do the division to get sc and tc*/
  sc = (std::fabs(sN) < db_EPSINON ? 0.0 : sN / sD);
  tc = (std::fabs(tN) < db_EPSINON ? 0.0 : tN / tD);

  /* get the difference of the two closest points*/
  cdVec2 dP = w + (u * sc) - (v * tc); /* =  S1(sc) - S2(tc)*/
  ptInS1 = S1_P0 + (u * sc);
  ptInS2 = S2_P0 + (v * tc);
  return dP.norm(); /* CVec3::norm(dP); */ /* return the closest distance*/
}

txFloat smallest_dist_between_ego_and_obsVehicle(const txPolygon2D& current_adc_car_pointset,
                                                 const txPolygon2D& current_frame_obs_pointset, cdVec2& finalPtInAdc,
                                                 cdVec2& finalPtInObs) TX_NOEXCEPT {
  /*std::vector< CVec3 > current_frame_obs_pointset = generate_obstacle_pointset();
  std::vector< CVec3 > current_adc_car_pointset = generate_adc_car_pointset(adc_status);*/

  txFloat smallest_dist = 9999.0;
  cdVec2 curPtInAdc;
  cdVec2 curPtInObs;
  const txSize n_obs_point_size = current_frame_obs_pointset.size();
  const txSize n_adc_car_pointset_size = current_adc_car_pointset.size();
  for (txSize obsIdx = 0; obsIdx < n_obs_point_size; ++obsIdx) {
    const cdVec2& obsPos_0 = current_frame_obs_pointset[obsIdx];
    const cdVec2& obsPos_1 = current_frame_obs_pointset[(obsIdx + 1) % n_obs_point_size];
    for (txSize lineIdx = 0; lineIdx < n_adc_car_pointset_size; ++lineIdx) {
      const txFloat cur_dist_edge2edge = dist3D_Segment_to_Segment(
          current_adc_car_pointset[lineIdx], current_adc_car_pointset[(lineIdx + 1) % n_adc_car_pointset_size],
          obsPos_0, obsPos_1, curPtInAdc, curPtInObs);
      /*smallest_dist = std::min(cur_dist_edge2edge, smallest_dist);*/
      if (cur_dist_edge2edge < smallest_dist) {
        smallest_dist = cur_dist_edge2edge;
        finalPtInAdc = curPtInAdc;
        finalPtInObs = curPtInObs;
      }

      const txFloat cur_dist_point2edge =
          minimum_distance(current_adc_car_pointset[lineIdx],
                           current_adc_car_pointset[(lineIdx + 1) % n_adc_car_pointset_size], obsPos_0, curPtInAdc);
      /*smallest_dist = std::min(cur_dist_point2edge, smallest_dist);*/
      if (cur_dist_point2edge < smallest_dist) {
        smallest_dist = cur_dist_point2edge;
        finalPtInAdc = curPtInAdc;
        finalPtInObs = obsPos_0;
      }

      const txFloat cur_dist_point2point =
          (obsPos_0 - current_adc_car_pointset[lineIdx])
              .norm(); /*CVec3::distance2d(obsPos_0, current_adc_car_pointset[lineIdx]);*/
      /*smallest_dist = std::min(cur_dist_point2point, smallest_dist);*/
      if (cur_dist_point2point < smallest_dist) {
        smallest_dist = cur_dist_point2point;
        finalPtInAdc = current_adc_car_pointset[lineIdx];
        finalPtInObs = obsPos_0;
      }
    }
  }

  return smallest_dist;
}

txFloat half_plane_sign_2d(const cdVec2& p1, const cdVec2& p2, const cdVec2& p3) TX_NOEXCEPT {
  return (p1.x() - p3.x()) * (p2.y() - p3.y()) - (p2.x() - p3.x()) * (p1.y() - p3.y());
}

txBool PointInTriangle2d(const cdVec2& pt, const cdVec2& v1, const cdVec2& v2, const cdVec2& v3) TX_NOEXCEPT {
  txFloat d1, d2, d3;
  txBool has_neg, has_pos;

  d1 = half_plane_sign_2d(pt, v1, v2);
  d2 = half_plane_sign_2d(pt, v2, v3);
  d3 = half_plane_sign_2d(pt, v3, v1);

  has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
  has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

  return !(has_neg && has_pos);
}

TX_NAMESPACE_OPEN(GJK)

cdVec2 perpendicular(const cdVec2& v) TX_NOEXCEPT {
  return cdVec2(v.y(), -1.0 * v.x());
  /*vec2 p = { v.y, -v.x }; return p; */
}

cdVec2 tripleProduct(const cdVec2& a, const cdVec2& b, const cdVec2& c) TX_NOEXCEPT {
  return b * a.dot(c) - a * b.dot(c);
  /*float ac = a.x * c.x + a.y * c.y; // perform a.dot(c)
  float bc = b.x * c.x + b.y * c.y; // perform b.dot(c)

  // perform b * a.dot(c) - a * b.dot(c)
  r.x = b.x * ac - a.x * bc;
  r.y = b.y * ac - a.y * bc;
  return r;*/
}

cdVec2 averagePoint(const txPolygon2D& polygon) TX_NOEXCEPT {
  cdVec2 avg;
  avg.setZero();
  if (!polygon.empty()) {
    avg = std::accumulate(polygon.begin(), polygon.end(), avg);
    const txSize nSize = polygon.size();
    avg.x() /= nSize;
    avg.y() /= nSize;
  }
  return avg;
}

txSize indexOfFurthestPoint(const txPolygon2D& polygon, const cdVec2& d) TX_NOEXCEPT {
  txFloat maxProduct = d.dot(polygon[0]); /*dotProduct(d, vertices[0]);*/
  txSize index = 0;
  const txSize count = polygon.size();
  for (txSize i = 1; i < count; i++) {
    const txFloat product = d.dot(polygon[i]); /*dotProduct(d, vertices[i]);*/
    if (product > maxProduct) {
      maxProduct = product;
      index = i;
    }
  }
  return index;
}

cdVec2 support(const txPolygon2D& polygon1, const txPolygon2D& polygon2, const cdVec2& d) TX_NOEXCEPT {
  // get furthest point of first body along an arbitrary direction
  const txSize i = indexOfFurthestPoint(polygon1, d);

  // get furthest point of second body along the opposite direction
  const txSize j = indexOfFurthestPoint(polygon2, d * (-1.0));

  // subtract (Minkowski sum) the two points to see if bodies 'overlap'
  return polygon1[i] - polygon2[j]; /* subtract(vertices1[i], vertices2[j]);*/
}

txBool IsPolygonClosed(const txPolygon2D& polygon) TX_NOEXCEPT { return (polygon.size() > 2); }

txBool HitPolygons(const txPolygon2D& polygon1, const txPolygon2D& polygon2) TX_NOEXCEPT {
  if (IsPolygonClosed(polygon1) && IsPolygonClosed(polygon2)) {
    txSize index = 0;  // index of current vertex of simplex
    cdVec2 a, b, c, d, ao, ab, ac, abperp, acperp, simplex[3];

    const cdVec2 position1 = averagePoint(polygon1);  // not a CoG but
    const cdVec2 position2 = averagePoint(polygon2);  // it's ok for GJK )

    // initial direction from the center of 1st body to the center of 2nd body
    d = position1 - position2; /*d = subtract(position1, position2);*/

    // if initial direction is zero C set it to any arbitrary axis (we choose X)
    if (Math::isZero(d.x()) && Math::isZero(d.y())) {
      d.x() = 1.0;
    }
    /*if ((d.x == 0) && (d.y == 0))
        d.x = 1.f;*/

    // set the first support as initial point of the new simplex
    a = simplex[0] = support(polygon1, polygon2, d);

    if (a.dot(d) /*dotProduct(a, d)*/ <= 0.0) {
      return false;  // no collision
    }

    d = (a) *
        (-1.0);  // The next search direction is always towards the origin, so the next search direction is negate(a)

    int max_iterations = 50;  // set max iterations
    int iter_count = 0;

    while (iter_count < max_iterations) {
      iter_count++;

      a = simplex[++index] = support(polygon1, polygon2, d);

      if (/*dotProduct(a, d)*/ a.dot(d) <= 0.0) {
        return false;  // no collision
      }

      ao = (a) * (-1.0);  // from point A to Origin is just negative A

      // simplex has 2 points (a line segment, not a triangle yet)
      if (index < 2) {
        b = simplex[0];
        ab = b - a; /* subtract(b, a);*/     // from point A to B
        d = tripleProduct(ab, ao, ab);       // normal to AB towards Origin
        if (Math::isZero(d.squaredNorm())) { /*(lengthSquared(d) == 0)*/
          d = perpendicular(ab);
        }
        continue;  // skip to next iteration
      }
      b = simplex[1];
      c = simplex[0];
      ab = b - a; /* subtract(b, a);*/  // from point A to B
      ac = c - a; /* subtract(c, a);*/  // from point A to C

      acperp = tripleProduct(ab, ac, ac);

      if (/*dotProduct(acperp, ao)*/ acperp.dot(ao) >= 0.0) {
        d = acperp;  // new direction is normal to AC towards Origin
      } else {
        abperp = tripleProduct(ac, ab, ab);

        if (/*dotProduct(abperp, ao)*/ abperp.dot(ao) < 0) {
          return true;  // collision
        }

        simplex[0] = simplex[1];  // swap first element (point C)

        d = abperp;  // new direction is normal to AB towards Origin
      }

      simplex[1] = simplex[2];  // swap element in the middle (point B)
      --index;
    }

    return false;
  } else {
    return false;
  }
}

TX_NAMESPACE_CLOSE(GJK)
TX_NAMESPACE_CLOSE(CD2D)
TX_NAMESPACE_CLOSE(Geometry)
