// Copyright 2024 Tencent Inc. All rights reserved.
//

#include <stdio.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include "common/coord_trans.h"
#include "common/log.h"
#include "mapengine/hadmap_codes.h"
#include "mapengine/hadmap_engine.h"
#include "routingmap/routing_map.h"
#include "structs/hadmap_curve.h"
#include "structs/hadmap_lane.h"
#include "structs/hadmap_predef.h"
#include "types/map_defs.h"

using namespace hadmap;
using namespace std;
int main(int argn, char** argv) {
  std::string fname = argv[1];
  hadmap::txMapHandle* pHandle = nullptr;
  hadmap::MAP_DATA_TYPE mdp = hadmap::SQLITE;
  if (fname.back() == 'r' || fname.back() == 'R') {
    mdp = hadmap::OPENDRIVE;
  }
  std::cout << fname << std::endl;

  auto start_time = std::chrono::system_clock::now();
  if (hadmap::hadmapConnect(fname.c_str(), mdp, &pHandle) != TX_HADMAP_HANDLE_OK) {
    std::cout << "faild.\n";
  }
  auto end_time = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  std::cout << "duration = "
            << static_cast<double>(duration.count()) * std::chrono::microseconds::period::num /
                   std::chrono::microseconds::period::den
            << " s.\n";

  txRoads _roads;
  getRoads(pHandle, true, _roads);

  std::ofstream lanegps(string(argv[1]) + ".lane.gps.txt");
  std::ofstream llinegps(string(argv[1]) + ".lline.gps.txt");
  lanegps.setf(std::ios::fixed, std::ios::floatfield);
  lanegps.precision(12);
  llinegps.setf(std::ios::fixed, std::ios::floatfield);
  llinegps.precision(12);
  for (auto& road_ptr : _roads) {
    const txSections& sections = road_ptr->getSections();
    for (auto& section_ptr : sections) {
      const txLanes& lanes = section_ptr->getLanes();
      for (auto& lane_ptr : lanes) {
        PointVec points;
        if (!lane_ptr->getGeometry()) continue;
        dynamic_cast<const txLineCurve*>(lane_ptr->getGeometry())->getPoints(points);
        if (points.empty()) {
          std::cout << "lane[" << lane_ptr->getTxLaneId().roadId << "," << lane_ptr->getTxLaneId().sectionId << ","
                    << lane_ptr->getTxLaneId().laneId << "] points is empty\n";
        }
        auto coordtype = lane_ptr->getGeometry()->getCoordType();
        if (coordtype == COORD_WGS84) {
          for (auto& p : points) {
            lanegps << std::to_string(lane_ptr->getTxLaneId().roadId) + "_" +
                           std::to_string(lane_ptr->getTxLaneId().sectionId) + "_" +
                           std::to_string(lane_ptr->getTxLaneId().laneId)
                    << " " << p.x << " " << p.y << " " << p.z << std::endl;
          }
        }
      }
      const txLaneBoundaries& bdys = section_ptr->getBoundaries();
      for (auto& boundaryPtr : bdys) {
        PointVec points;
        dynamic_cast<const txLineCurve*>(boundaryPtr->getGeometry())->getPoints(points);

        auto coordtype = boundaryPtr->getGeometry()->getCoordType();
        if (coordtype == COORD_WGS84) {
          for (auto& p : points) {
            llinegps << std::to_string(section_ptr->getRoadId()) + "_" + std::to_string(section_ptr->getId()) << " "
                     << p.x << " " << p.y << " " << p.z << std::endl;
          }
        }
      }
    }
  }

  lanegps.close();
  llinegps.close();

  hadmap::hadmapClose(&pHandle);

  return 0;
}
