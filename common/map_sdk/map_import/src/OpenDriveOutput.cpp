// Copyright 2024 Tencent Inc. All rights reserved.
// Project:  SimMapsdk
// @author   longerding@tencent
// Modify history:
//

#include "OpenDriveOutput.h"
#include <time.h>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <tuple>

#include <eigen3/Eigen/Eigen>
#include "OpenDriveStruct.h"
#include "common/coord_trans.h"
#ifdef PROJ_NEW_API
#  include <proj.h>
#else
#  ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#    define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#  endif
#  include "proj_api.h"
#endif
#define LINK_ROAD_ID 30001
using namespace Eigen;
namespace hadmap {
const char* HadMapLaneType(LANE_TYPE type) {
  switch (type) {
    case LANE_TYPE_None:
      return "none";
    case LANE_TYPE_Driving:
      return "driving";
    case LANE_TYPE_Stop:
      return "stop";
    case LANE_TYPE_Shoulder:
      return "shoulder";
    case LANE_TYPE_Biking:
      return "biking";
    case LANE_TYPE_Sidewalk:
      return "sidewalk";
    case LANE_TYPE_Border:
      return "border";
    case LANE_TYPE_Restricted:
      return "restricted";
    case LANE_TYPE_Parking:
      return "parking";
    case LANE_TYPE_BIDIRECTIONAL:
      return "bidirectional";
    case LANE_TYPE_CONNECTINGRAMP:
      return "connectingRamp";
    case LANE_TYPE_CURB:
      return "curb";
    case LANE_TYPE_ENTRY:
      return "entry";
    case LANE_TYPE_EXIT:
      return "exit";
    case LANE_TYPE_MEDIAN:
      return "median";
    case LANE_TYPE_OFFRAMP:
      return "offRamp";
    case LANE_TYPE_ONRAMP:
      return "onRamp";
    case LANE_TYPE_RAIL:
      return "rail";
    case LANE_TYPE_ROADWORKS:
      return "roadworks";
    case LANE_TYPE_TRAM:
      return "tram";
    case LANE_TYPE_MwyEntry:
      return "MwyEntry";
    case LANE_TYPE_MwyExit:
      return "MwyExit";
    case LANE_TYPE_Emergency:
      return "Emergency";
    case LANE_TYPE_TurnWaiting:
      return "TurnWaiting";
  }
  return "none";
}

const char* HadMapLaneMark(LANE_MARK mark) {
  switch (mark) {
    case LANE_MARK_None:
      return "none";
    case LANE_MARK_Solid:
      return "solid";
    case LANE_MARK_Broken:
      return "broken";
    case LANE_MARK_Solid2:
      return "solid2";
    case LANE_MARK_Broken2:
      return "broken2";
    case LANE_MARK_SolidSolid:
      return "solid solid";
    case LANE_MARK_SolidBroken:
      return "solid broken";
    case LANE_MARK_BrokenSolid:
      return "broken solid";
    case LANE_MARK_BrokenBroken:
      return "broken broken";
    case LANE_MARK_Curb:
      return "curb";
    case LANE_MARK_Fence:
      return "fence";
    case LANE_MARK_Bold:
      return "bold";
    case LANE_MARK_Yellow:
      return "yellow";
    case LANE_MARK_Red:
      return "red";
    case LANE_MARK_Yellow2:
    case LANE_MARK_YellowYellow:
      return "yellow";
    case LANE_MARK_Blue:
      return "blue";
    case LANE_MARK_Green:
      return "green";
  }
}

const char* HadMapLaneColor(int mark) {
  switch (mark) {
    case LANE_MARK_Yellow:
      return "yellow";
    case LANE_MARK_Red:
      return "red";
    case LANE_MARK_Yellow2:
      return "yellow yellow";
    case LANE_MARK_Blue:
      return "blue";
    case LANE_MARK_Green:
      return "green";
  }
  return "white";
}

const char* HadMapObjectColor(uint32_t tColor) {
  switch (tColor) {
    case OBJECT_COLOR_Yellow:
      return "yellow";
    case OBJECT_COLOR_Red:
      return "red";
    case LANE_MARK_Yellow2:
      return "yellow yellow";
    case OBJECT_COLOR_Blue:
      return "blue";
    case OBJECT_COLOR_Green:
      return "green";
    case OBJECT_COLOR_Black:
      return "black";
  }
  return "white";
}
// 通过两点计算 直线方程（Ax+By+C = 0)
bool LineEquation(const hadmap::txPoint& pt1, const hadmap::txPoint& pt2, double& A, double& B, double& C) {
  // 求直线方程 算出A B C
  A = 1.0;
  B = 1.0;
  C = 1.0;
  hadmap::txPoint pt;
  double F = pt1.x * pt2.y - pt1.y * pt2.x;
  pt.x = pt2.x - pt1.x;
  pt.y = pt2.y - pt1.y;
  if (fabs(F) < 1e-6) {
    if ((fabs(pt.x) < 1e-6) && (fabs(pt.y) < 1e-6)) {
      return false;
    } else if (fabs(pt.x) < 1e-6) {
      A = 1.0;
      B = 0.0;
      C = 0.0 - pt1.x;
    } else if (fabs(pt.y) < 1e-6) {
      A = 0.0;
      B = 1.0;
      C = 0.0 - pt1.y;
    } else {
      A = 1.0;
      B = 0.0 - pt1.x / pt1.y;
      C = 0.0;
    }
  } else {
    A = 0.0 - pt.y / F;
    B = pt.x / F;
    C = 1.0;
  }
  return true;
}
// 通过一个点以及直线方程中的AB值求C（Ax+By+C=0）
bool LineEquation(const hadmap::txPoint& pt, double A, double B, double& C) {
  // 求经过pt点的平行线 A B同 求C
  C = 0.0 - A * pt.x - B * pt.y;
  return true;
}
// 通过两条直线交点求出圆心
bool LineLineCross(double A1, double B1, double C1, double A2, double B2, double C2, hadmap::txPoint& ptCross) {
  double F = A1 * B2 - A2 * B1;
  // 判断两条直线是否平行
  if (fabs(F) < 1e-8) {
    return false;
  }

  ptCross.x = (B1 * C2 - B2 * C1) / F;
  ptCross.y = 0.0 - (A1 * C2 - A2 * C1) / F;
  return true;
}
// 已知3点，求半径，圆心，是否顺时针(G代码)，弧长，开始角度(数学坐标系，弧度)，结束角度(数学坐标系，弧度)
bool Pt3CircleWithAngle(const hadmap::txPoint& pt1, const hadmap::txPoint& pt2, const hadmap::txPoint& pt3, double& R,
                        hadmap::txPoint& ptCen, double& radLength, double& startAngleG, double& endAngleG) {
  // pt1:圆弧点     pt2:上一个中间点         pt3:当前中间点
  double A[4];
  double B[4];
  double C[4];

  hadmap::txPoint pt[3];

  // (1) 整合出两条直线 Ax+By+C=0;
  LineEquation(pt1, pt2, A[0], B[0], C[0]);
  LineEquation(pt2, pt3, A[1], B[1], C[1]);

  if (fabs(B[0]) < 1e-6) {
    if (fabs(B[1]) < 1e-6) {
      // 三点同一条垂直线不能构成圆
      return false;
    }
  } else {
    if (fabs(B[1]) > 1e-6) {
      if (fabs(A[0] / B[0] - A[1] / B[1]) < 1e-6) {
        return false;
      }
    }
  }

  // (2)  找出pt1与点pt2的中间点
  pt[0] = pt1;
  pt[0].x *= 0.5;
  pt[0].y *= 0.5;
  pt[2] = pt2;
  pt[2].x *= 0.5;
  pt[2].y *= 0.5;
  pt[0].x += pt[2].x;
  pt[0].y += pt[2].y;

  // 找出pt2与点pt3的中间点  （准备找中垂线）
  pt[1] = pt2;
  pt[1].x *= 0.5;
  pt[1].y *= 0.5;
  pt[2] = pt3;
  pt[2].x *= 0.5;
  pt[2].y *= 0.5;
  pt[1].x += pt[2].x;
  pt[1].y += pt[2].y;

  // (3)   做出中垂线（L1的中垂线）
  A[2] = 0.0 - B[0];
  B[2] = A[0];
  LineEquation(pt[0], A[2], B[2], C[2]);

  // 做出L2的中垂线
  A[3] = 0.0 - B[1];
  B[3] = A[1];
  LineEquation(pt[1], A[3], B[3], C[3]);

  // 求两线的交点  ptCen 为两条中垂线的交点同时也是圆心
  LineLineCross(A[2], B[2], C[2], A[3], B[3], C[3], ptCen);

  // 求半经
  pt[2] = pt1;
  pt[2].x -= ptCen.x;
  pt[2].y -= ptCen.y;
  pt[2].z = 0.0;
  R = sqrt(pt[2].x * pt[2].x + pt[2].y * pt[2].y + pt[2].z * pt[2].z);

  pt[0] = pt2;
  pt[0].x -= pt1.x;
  pt[0].y -= pt1.y;
  pt[1] = pt3;
  pt[1].x -= pt2.x;
  pt[1].y -= pt2.y;

  A[0] = atan2(pt[0].y, pt[0].x);

  A[1] = atan2(pt[1].y, pt[1].x);

  A[2] = A[1] - A[0];
  // 这里需要确保 A[2]在 -M_PI 到 M_PI范围内
  if (A[2] < 0.0 - M_PI) {
    A[2] += 2 * M_PI;
  } else if (A[2] > M_PI) {
    A[2] -= 2 * M_PI;
  }

  radLength = 2 * fabs(A[2]);

  startAngleG = atan2(pt1.y - ptCen.y, pt1.x - ptCen.x);
  // 将范围从-pi 到 pi 转换为 0 到 2*pi
  if (startAngleG < 0 && startAngleG >= -M_PI) {
    startAngleG += 2 * M_PI;
  }
  endAngleG = startAngleG + 2 * A[2];

  return true;
}

bool OpenDriveOutput::initFilePath(const char* filePath, bool replace) {
  m_filePath = filePath;
  if (replace) {
    doc = new tinyxml2::XMLDocument();
    tinyxml2::XMLDeclaration* tinyXmlDelare = doc->NewDeclaration();
    tinyxml2::XMLElement* _opendrive_ele = doc->NewElement("OpenDRIVE");
    doc->InsertFirstChild(tinyXmlDelare);
    doc->InsertEndChild(_opendrive_ele);
  } else {
    doc = new tinyxml2::XMLDocument();
    doc->LoadFile(m_filePath);
  }
  m_isInit = true;
  return true;
}

bool OpenDriveOutput::insertHeader(const hadmap::txPoint refPoint) {
  tinyxml2::XMLElement* _header = doc->NewElement("header");
  _header->SetAttribute("revMajor", "1");
  _header->SetAttribute("revMinor", "4");
  _header->SetAttribute("name", "");
  _header->SetAttribute("version", "1.0");
  time_t now = time(NULL);
  tm* tm_t = localtime(&now);
  std::stringstream ss;
  ss << tm_t->tm_year + 1900 << "-" << tm_t->tm_mon + 1 << "-" << tm_t->tm_mday;
  _header->SetAttribute("date", std::string(ss.str()).c_str());
  _header->SetAttribute("north", std::to_string(refPoint.y).c_str());
  _header->SetAttribute("sourth", std::to_string(refPoint.y).c_str());
  _header->SetAttribute("west", std::to_string(refPoint.x).c_str());
  _header->SetAttribute("east", std::to_string(refPoint.x).c_str());
  _header->SetAttribute("vendor", "");
  // _header->InsertNewText("<![CDATA[+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0
  // +units=m +nadgrids=@null +wktext +no_defs]]>");
  doc->FirstChildElement("OpenDRIVE")->InsertFirstChild(_header);
  return true;
}

bool OpenDriveOutput::insertRoads(const hadmap::txRoads& roads) {
  for (auto it : roads) {
    tinyxml2::XMLElement* _road = insertOneRoad(it);
    tinyxml2::XMLElement* _lastRoadEle = doc->RootElement()->LastChildElement("road");
    if (_lastRoadEle) {
      doc->FirstChildElement("OpenDRIVE")->InsertAfterChild(_lastRoadEle, _road);
    } else {
      doc->FirstChildElement("OpenDRIVE")->InsertEndChild(_road);
    }
  }
  return true;
}

bool OpenDriveOutput::insertObjects(const hadmap::txObjects& objects, const hadmap::txRoads& roads) {
  std::map<roadpkid, hadmap::txObjects> roadid2mapObjects;
  for (auto it : objects) {
    int roadid = it->getOdData().roadid;
    if (roadid > 0) {
      if (roadid2mapObjects.find(roadid) == roadid2mapObjects.end()) {
        hadmap::txObjects _objects;
        _objects.push_back(it);
        roadid2mapObjects[roadid] = _objects;
      } else {
        roadid2mapObjects.find(roadid)->second.push_back(it);
      }
    } else {
      if (roadid2mapObjects.find(it->getOdData().lanelinkid + LINK_ROAD_ID) == roadid2mapObjects.end()) {
        hadmap::txObjects _objects;
        _objects.push_back(it);
        roadid2mapObjects[it->getOdData().lanelinkid + LINK_ROAD_ID] = _objects;
      } else {
        roadid2mapObjects.find(it->getOdData().lanelinkid + LINK_ROAD_ID)->second.push_back(it);
      }
    }
  }
  for (auto it : roadid2mapObjects) {
    roadpkid roadid = it.first;
    tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
    while (roadEle) {
      if (roadEle->Int64Attribute("id") == static_cast<uint64_t>(roadid)) {
        // roads contains multi lanelinks
        ODRoad roadtmp;
        bool isMultilane = false;
        if (roadEle->Int64Attribute("junction") > 0) {
          if (roadtmp.parse(roadEle)) {
            ODLaneSection section;
            roadtmp.getSection(0, section);
            if (section.size() > 2) {
              isMultilane = true;
            }
          }
        }
        tinyxml2::XMLElement* _objects;
        if (roadEle->FirstChildElement("objects")) {
          _objects = roadEle->FirstChildElement("objects");
        } else {
          _objects = doc->NewElement("objects");
          roadEle->InsertEndChild(_objects);
        }
        tinyxml2::XMLElement* _signals;
        if (roadEle->FirstChildElement("signals")) {
          _signals = roadEle->FirstChildElement("signals");
        } else {
          _signals = doc->NewElement("signals");
          roadEle->InsertEndChild(_signals);
        }
        for (auto itObject : it.second) {
          //
          if (isMultilane) {
            tx_od_object_t od_data = itObject->getOdData();
            double _s = od_data.s;
            txPoint pt1 = itObject->getPos();
            double _tmpx = pt1.x;
            double _tmpy = pt1.y;
            // std::cout << "_tmpx : " << _tmpx << "_tmpy: " << _tmpy << std::endl;
            lonlat2xy(_tmpx, _tmpy);
            double _t = 0;
            roadtmp.getT(_tmpx, _tmpy, _s, _t);
            od_data.t = _t;
            itObject->setTxOdData(od_data);
          }
          if (itObject->getObjectType() == OBJECT_TYPE_TrafficLights) {
            tinyxml2::XMLElement* signal = doc->NewElement("signal");
            tx_od_object_t od_data = itObject->getOdData();
            signal->SetAttribute("s", od_data.s);
            signal->SetAttribute("t", od_data.t);
            signal->SetAttribute("name", od_data.name);
            signal->SetAttribute("id", static_cast<int64_t>(itObject->getId()));
            signal->SetAttribute("dynamic", "yes");
            signal->SetAttribute("orientation", ((od_data.orientation == 1) ? "+" : "-"));
            signal->SetAttribute("zOffset", od_data.zoffset);
            signal->SetAttribute("country", "OpenDRIVE");
            signal->SetAttribute("hOffset", od_data.hdg);
            std::string name;
            std::string subtype;
            std::string type;
            itObject->getObjectFromatType(name, type, subtype);
            signal->SetAttribute("type", type.c_str());
            signal->SetAttribute("subtype", subtype.c_str());
            signal->SetAttribute("width", od_data.width);
            signal->SetAttribute("height", od_data.height);
            std::map<std::string, std::string> userData;
            itObject->getUserData(userData);
            if (userData.size() > 0) {
              for (auto itUser : userData) {
                tinyxml2::XMLElement* userdata = doc->NewElement("userData");
                userdata->SetAttribute("code", itUser.first.c_str());
                userdata->SetAttribute("value", itUser.second.c_str());
                signal->InsertEndChild(userdata);
              }
            }
            _signals->InsertEndChild(signal);
          } else if (itObject->getObjectType() == OBJECT_TYPE_TrafficSign) {
            tinyxml2::XMLElement* object = doc->NewElement("signal");
            tx_od_object_t od_data = itObject->getOdData();
            object->SetAttribute("s", od_data.s);
            object->SetAttribute("t", od_data.t);
            object->SetAttribute("name", od_data.name);

            object->SetAttribute("id", static_cast<int64_t>(itObject->getId()));
            object->SetAttribute("dynamic", "no");
            object->SetAttribute("orientation", ((od_data.orientation == 1) ? "+" : "-"));
            object->SetAttribute("zOffset", od_data.zoffset);
            object->SetAttribute("country", "OpenDRIVE");
            object->SetAttribute("hOffset", od_data.hdg);
            std::string name;
            std::string subtype;
            std::string type;
            itObject->getObjectFromatType(name, type, subtype);
            object->SetAttribute("type", type.c_str());
            object->SetAttribute("subtype", subtype.c_str());
            object->SetAttribute("width", od_data.width);
            object->SetAttribute("height", od_data.height);

            std::map<std::string, std::string> userData;
            itObject->getUserData(userData);
            if (userData.size() > 0) {
              for (auto itUser : userData) {
                tinyxml2::XMLElement* userdata = doc->NewElement("userData");
                userdata->SetAttribute("code", itUser.first.c_str());
                userdata->SetAttribute("value", itUser.second.c_str());
                object->InsertEndChild(userdata);
              }
            }
            _signals->InsertEndChild(object);
          } else if (itObject->getObjectType() == OBJECT_TYPE_Tunnel) {
            tinyxml2::XMLElement* _tunnel = doc->NewElement("tunnel");
            tx_od_object_t od_data = itObject->getOdData();
            _tunnel->SetAttribute("s", od_data.s);
            _tunnel->SetAttribute("length", od_data.length);
            _tunnel->SetAttribute("name", od_data.name);
            _tunnel->SetAttribute("id", static_cast<int64_t>(od_data.pkid));
            _tunnel->SetAttribute("type", "standard");
            _tunnel->SetAttribute("lighting", "0.1");
            _tunnel->SetAttribute("daylight", "0.1");

            std::map<std::string, std::string> userData;
            itObject->getUserData(userData);
            if (userData.size() > 0) {
              for (auto itUser : userData) {
                tinyxml2::XMLElement* userdata = doc->NewElement("userData");
                userdata->SetAttribute("code", itUser.first.c_str());
                userdata->SetAttribute("value", itUser.second.c_str());
                _tunnel->InsertEndChild(userdata);
              }
            }
            _objects->LinkEndChild(_tunnel);
          } else {
            tinyxml2::XMLElement* object = doc->NewElement("object");
            std::string name;
            std::string subtype;
            std::string type;
            itObject->getObjectFromatType(name, type, subtype);
            object->SetAttribute("type", type.c_str());
            object->SetAttribute("id", static_cast<int64_t>(itObject->getId()));
            double s = 0, t = 0;
            tx_od_object_t od_data = itObject->getOdData();
            object->SetAttribute("s", od_data.s);
            object->SetAttribute("t", od_data.t);
            object->SetAttribute("name", od_data.name);
            object->SetAttribute("zOffset", od_data.zoffset);
            object->SetAttribute("orientation", ((od_data.orientation == 1) ? "+" : "-"));
            object->SetAttribute("length", od_data.length);
            object->SetAttribute("width", od_data.width);
            object->SetAttribute("height", od_data.height);
            object->SetAttribute("hdg", od_data.hdg);
            object->SetAttribute("pitch", od_data.pitch);
            object->SetAttribute("roll", od_data.roll);
            if (!std::string(od_data.repeatdata).empty()) {
              tinyxml2::XMLElement* repeats = doc->NewElement("repeat");
              std::string repeatdata = od_data.repeatdata;
              std::string::size_type pos;
              std::vector<std::string> result;
              int size = repeatdata.size();
              for (int i = 0; i < size; i++) {
                pos = repeatdata.find(";", i);
                if (pos < size) {
                  std::string s = repeatdata.substr(i, pos - i);
                  if (!s.empty()) result.push_back(s);
                  i = pos;
                }
              }
              std::map<std::string, std::string> repeatdataVec;
              for (auto it : result) {
                int pos = it.find(":");
                std::string key = it.substr(0, pos);
                std::string value = it.substr(pos + 1, it.size() - pos - 1);
                repeatdataVec[key] = value;
              }
              for (auto it : repeatdataVec) {
                repeats->SetAttribute(it.first.c_str(), it.second.c_str());
              }
              object->InsertEndChild(repeats);
            }
            if (itObject->getObjectType() == OBJECT_TYPE_ParkingSpace) {
              object->SetAttribute("type", "parkingSpace");
              tinyxml2::XMLElement* outlines = doc->NewElement("outlines");
              tinyxml2::XMLElement* parkingSpace = doc->NewElement("parkingSpace");
              tinyxml2::XMLElement* markings = doc->NewElement("markings");
              parkingSpace->SetAttribute("access", "all");
              int id = 0;
              int index = 0;
              int markingid = 0;
              std::string::size_type pos;
              std::string strData = (od_data.outlines);
              int size = strData.size();
              std::vector<std::string> mapData;
              // splite mapdata
              for (int i = 0; i < size; i++) {
                pos = strData.find(";", i);
                if (pos < size) {
                  std::string s = strData.substr(i, pos - i);
                  if (!s.empty()) mapData.push_back(s);
                  i = pos;
                }
              }
              int nindex = 0;

              for (int j = 0; j < mapData.size() / 4; j++) {
                tinyxml2::XMLElement* outline = doc->NewElement("outline");
                outline->SetAttribute("id", j);
                for (int i = 0; i < 4; i++) {
                  outlines->InsertEndChild(outline);
                  tinyxml2::XMLElement* cornerRoad = doc->NewElement("cornerRoad");
                  int nPosdata = mapData.at(4 * j + i).find(",");
                  std::string key = mapData.at(4 * j + i).substr(0, nPosdata);
                  std::string value =
                      mapData.at(4 * j + i).substr(nPosdata + 1, mapData.at(4 * j + i).size() - nPosdata - 1);
                  cornerRoad->SetAttribute("s", std::atof(key.c_str()));
                  cornerRoad->SetAttribute("t", std::atof(value.c_str()));
                  cornerRoad->SetAttribute("dz", 0.0);
                  cornerRoad->SetAttribute("height", 4.0);
                  cornerRoad->SetAttribute("id", nindex++);
                  outline->InsertEndChild(cornerRoad);
                }
                outlines->InsertEndChild(outline);
              }

              for (int i = 0; i < mapData.size() / 2; i++) {
                tinyxml2::XMLElement* marking = doc->NewElement("marking");
                marking->SetAttribute("width", itObject->getOdData().markWidth);
                std::string color = "white";
                if (itObject->getGeomSize() > 0) {
                  txObjGeomPtr geom = itObject->getGeom();
                  if (geom) {
                    color = HadMapObjectColor(geom->getColor());
                  }
                }
                marking->SetAttribute("color", color.c_str());
                marking->SetAttribute("zOffset", "0.0");
                marking->SetAttribute("spaceLength", "0.0");
                marking->SetAttribute("lineLength", "1.0");
                marking->SetAttribute("startOffset", "0.0");
                marking->SetAttribute("stopOffset", "0.0");
                if (i % 2) {
                  marking->SetAttribute("side", "right");
                } else {
                  marking->SetAttribute("side", "left");
                }
                for (int j = 0; j < 2; j++) {
                  tinyxml2::XMLElement* cornerReference = doc->NewElement("cornerReference");
                  cornerReference->SetAttribute("id", id++);
                  marking->InsertEndChild(cornerReference);
                }
                markings->InsertEndChild(marking);
              }
              object->InsertEndChild(outlines);
              object->InsertEndChild(parkingSpace);
              object->InsertEndChild(markings);
            }
            if (itObject->getObjectType() == OBJECT_TYPE_Pole) {
              object->SetAttribute("type", "pole");
            }
            std::map<std::string, std::string> userData;
            itObject->getUserData(userData);
            if (userData.size() > 0) {
              for (auto itUser : userData) {
                tinyxml2::XMLElement* userdata = doc->NewElement("userData");
                userdata->SetAttribute("code", itUser.first.c_str());
                userdata->SetAttribute("value", itUser.second.c_str());
                object->InsertEndChild(userdata);
              }
            }
            _objects->InsertEndChild(object);
          }
        }
        break;
      }
      roadEle = roadEle->NextSiblingElement("road");
    }
  }
  return true;
}

bool OpenDriveOutput::insertLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  std::map<junctionpkid, txLaneLinks> id2lanelinks;

  std::map<hadmap::roadpkid, txLaneLinkPtr> fromRoad2LaneLink;
  std::map<hadmap::roadpkid, txLaneLinkPtr> toRoad2LaneLink;

  for (auto itLink : lanelinks) {
    fromRoad2LaneLink[itLink->fromRoadId()] = itLink;
    toRoad2LaneLink[itLink->toRoadId()] = itLink;
    if (id2lanelinks.find(itLink->getJunctionId()) != id2lanelinks.end()) {
      id2lanelinks.find(itLink->getJunctionId())->second.push_back(itLink);
    } else {
      txLaneLinks lanelinks;
      lanelinks.push_back(itLink);
      id2lanelinks[itLink->getJunctionId()] = lanelinks;
    }
    tinyxml2::XMLElement* _road = insertOneLaneLink(itLink);
    tinyxml2::XMLElement* _lastRoadEle = doc->RootElement()->LastChildElement("road");
    if (_lastRoadEle) {
      doc->FirstChildElement("OpenDRIVE")->InsertAfterChild(_lastRoadEle, _road);
    } else {
      doc->FirstChildElement("OpenDRIVE")->InsertEndChild(_road);
    }
  }

  for (auto itJuc : id2lanelinks) {
    tinyxml2::XMLElement* junction = doc->FirstChildElement("OpenDRIVE")->FirstChildElement("junction");
    while (junction) {
      if (junction->Int64Attribute("id") == itJuc.first) {
        break;
      }
      junction = junction->NextSiblingElement("junction");
    }
    if (!junction) {
      junction = doc->NewElement("junction");
      tinyxml2::XMLElement* _lastJunctionEle = doc->RootElement()->LastChildElement("junction");
      if (_lastJunctionEle) {
        doc->FirstChildElement("OpenDRIVE")->InsertAfterChild(_lastJunctionEle, junction);
      } else {
        doc->FirstChildElement("OpenDRIVE")->InsertEndChild(junction);
      }
    }

    junction->SetAttribute("id", static_cast<int64_t>(itJuc.first));
    for (auto itLink : itJuc.second) {
      tinyxml2::XMLElement* connection = doc->NewElement("connection");
      connection->SetAttribute("connectingRoad", static_cast<int64_t>(itLink->getOdrRoadId()));
      connection->SetAttribute("contactPoint", ((itLink->getSuccContact() == START) ? "start" : "end"));
      connection->SetAttribute("incomingRoad", static_cast<int64_t>(itLink->fromRoadId()));

      tinyxml2::XMLElement* lanelink = doc->NewElement("laneLink");
      lanelink->SetAttribute("from", itLink->fromLaneId());
      lanelink->SetAttribute("to", itLink->toLaneId());
      connection->InsertEndChild(lanelink);
      junction->InsertEndChild(connection);
    }
  }

  tinyxml2::XMLElement* roadelement = doc->RootElement()->FirstChildElement("road");
  while (roadelement) {
    roadpkid roadid = roadelement->Int64Attribute("id");
    if (fromRoad2LaneLink.find(roadid) != fromRoad2LaneLink.end()) {
      tinyxml2::XMLElement* link;
      if (roadelement->FirstChildElement("link")) {
        link = roadelement->FirstChildElement("link");
      } else {
        link = doc->NewElement("link");
        roadelement->InsertFirstChild(link);
      }
      if (link->FirstChildElement("predecessor")) {
        link->FirstChildElement("predecessor")->SetAttribute("elementType", "junction");
        link->FirstChildElement("predecessor")
            ->SetAttribute("elementId", static_cast<int64_t>(fromRoad2LaneLink.find(roadid)->second->getJunctionId()));
      } else {
        tinyxml2::XMLElement* predecessor = doc->NewElement("predecessor");
        predecessor->SetAttribute("elementType", "junction");
        predecessor->SetAttribute("elementId",
                                  static_cast<int64_t>(fromRoad2LaneLink.find(roadid)->second->getJunctionId()));
        link->InsertFirstChild(predecessor);
      }
    }
    if (toRoad2LaneLink.find(roadid) != toRoad2LaneLink.end()) {
      tinyxml2::XMLElement* link;
      if (roadelement->FirstChildElement("link")) {
        link = roadelement->FirstChildElement("link");
      } else {
        link = doc->NewElement("link");
        roadelement->InsertFirstChild(link);
      }
      if (link->FirstChildElement("successor")) {
        link->FirstChildElement("successor")->SetAttribute("elementType", "junction");
        link->FirstChildElement("successor")
            ->SetAttribute("elementId", static_cast<int64_t>(toRoad2LaneLink.find(roadid)->second->getJunctionId()));
      } else {
        tinyxml2::XMLElement* predecessor = doc->NewElement("successor");
        predecessor->SetAttribute("elementType", "junction");
        predecessor->SetAttribute("elementId",
                                  static_cast<int64_t>(toRoad2LaneLink.find(roadid)->second->getJunctionId()));
        link->InsertEndChild(predecessor);
      }
    }
    roadelement = roadelement->NextSiblingElement("road");
  }
  return true;
}
bool OpenDriveOutput::updateRoads(const hadmap::txRoads& roads) {
  std::map<roadpkid, txRoadPtr> _roadpkidMap;
  for (auto it : roads) {
    _roadpkidMap[it->getId()] = it;
  }
  tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
  while (roadEle) {
    roadpkid roadid = roadEle->Int64Attribute("id");
    if (_roadpkidMap.find(roadid) != _roadpkidMap.end()) {
      tinyxml2::XMLElement* _InsertRoad = insertOneRoad(_roadpkidMap[roadid]);
      tinyxml2::XMLElement* objectsEle = NULL;
      if (roadEle->FirstChildElement("objects")) {
        _InsertRoad->InsertEndChild(roadEle->FirstChildElement("objects"));
      }
      if (roadEle->FirstChildElement("signals")) {
        _InsertRoad->InsertEndChild(roadEle->FirstChildElement("signals"));
      }
      doc->RootElement()->InsertAfterChild(roadEle, _InsertRoad);
      doc->RootElement()->DeleteChild(roadEle);
      roadEle = _InsertRoad;
    }
    roadEle = roadEle->NextSiblingElement("road");
  }
  return true;
}

bool OpenDriveOutput::updateLaneLinks(const hadmap::txLaneLinks& lanelinks) {
  this->deleteLanelinks(lanelinks);
  this->insertLaneLinks(lanelinks);
  return true;
}

bool OpenDriveOutput::updateObjects(const hadmap::txObjects& objects, const hadmap::txRoads& roads) {
  this->deleteObjects(objects);
  this->insertObjects(objects, roads);
  return true;
}

bool OpenDriveOutput::deleteRoads(const hadmap::txRoads& roads) {
  std::map<roadpkid, txRoadPtr> _roadpkidMap;
  for (auto it : roads) {
    _roadpkidMap[it->getId()] = it;
  }
  tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
  while (roadEle) {
    roadpkid roadid = roadEle->Int64Attribute("id");
    if (_roadpkidMap.find(roadid) != _roadpkidMap.end()) {
      doc->RootElement()->DeleteChild(roadEle);
    }
    roadEle = roadEle->NextSiblingElement("road");
  }
  return true;
}

bool OpenDriveOutput::deleteLanelinks(const hadmap::txLaneLinks& lanelinks) {
  std::map<hadmap::roadpkid, txLaneLinkPtr> fromRoad2LaneLink;
  std::map<hadmap::roadpkid, txLaneLinkPtr> toRoad2LaneLink;
  // delete lanelink road
  std::map<roadpkid, txLaneLinkPtr> _roadpkidMap;
  std::map<junctionpkid, txLaneLinks> id2lanelinks;
  for (auto it : lanelinks) {
    fromRoad2LaneLink[it->fromRoadId()] = it;
    toRoad2LaneLink[it->toRoadId()] = it;
    _roadpkidMap[it->getOdrRoadId()] = it;
    if (id2lanelinks.find(it->getJunctionId()) != id2lanelinks.end()) {
      id2lanelinks.find(it->getJunctionId())->second.push_back(it);
    } else {
      txLaneLinks lanelinks;
      lanelinks.push_back(it);
      id2lanelinks[it->getJunctionId()] = lanelinks;
    }
  }
  tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
  while (roadEle) {
    roadpkid roadid = roadEle->Int64Attribute("id");
    if (_roadpkidMap.find(roadid) != _roadpkidMap.end()) {
      doc->RootElement()->DeleteChild(roadEle);
    }
    roadEle = roadEle->NextSiblingElement("road");
  }
  // delete junctions
  tinyxml2::XMLElement* junction = doc->RootElement()->FirstChildElement("junction");
  while (junction) {
    junctionpkid junctionid = junction->Int64Attribute("id");
    if (id2lanelinks.find(junctionid) != id2lanelinks.end()) {
      tinyxml2::XMLElement* connection = junction->FirstChildElement("connection");
      while (connection) {
        if (_roadpkidMap.find(connection->Int64Attribute("connectingRoad")) != _roadpkidMap.end()) {
          junction->DeleteChild(connection);
        }
        connection = connection->NextSiblingElement("connection");
      }
      if (!junction->FirstChildElement()) {
        doc->RootElement()->DeleteChild(junction);
      }
    }
    junction = junction->NextSiblingElement("junction");
  }
  // delete fromroadid and toroads
  tinyxml2::XMLElement* roadelement = doc->RootElement()->FirstChildElement("road");
  while (roadelement) {
    roadpkid roadid = roadelement->Int64Attribute("id");
    if (fromRoad2LaneLink.find(roadid) != fromRoad2LaneLink.end()) {
      tinyxml2::XMLElement* link;
      if (roadelement->FirstChildElement("link")) {
        link = roadelement->FirstChildElement("link");
      } else {
        link = doc->NewElement("link");
        roadelement->InsertFirstChild(link);
      }
      if (link->FirstChildElement("predecessor")) {
        link->DeleteChild(link->FirstChildElement("predecessor"));
      }
    }
    if (toRoad2LaneLink.find(roadid) != toRoad2LaneLink.end()) {
      tinyxml2::XMLElement* link;
      if (roadelement->FirstChildElement("link")) {
        link = roadelement->FirstChildElement("link");
      } else {
        link = doc->NewElement("link");
        roadelement->InsertFirstChild(link);
      }
      if (link->FirstChildElement("successor")) {
        link->DeleteChild(link->FirstChildElement("successor"));
      }
    }
    roadelement = roadelement->NextSiblingElement("road");
  }
  return true;
}

bool OpenDriveOutput::deleteObjects(const hadmap::txObjects& objects) {
  std::map<roadpkid, hadmap::txObjects> roadid2mapObjects;
  std::map<hadmap::objectpkid, bool> objectIdVec;
  for (auto it : objects) {
    int roadid = it->getOdData().roadid;
    if (roadid > 0) {
      if (roadid2mapObjects.find(roadid) == roadid2mapObjects.end()) {
        hadmap::txObjects _objects;
        _objects.push_back(it);
        roadid2mapObjects[roadid] = _objects;
      } else {
        roadid2mapObjects.find(roadid)->second.push_back(it);
      }
    } else {
      objectIdVec.insert(std::make_pair(it->getId(), true));
      /*
      if (roadid2mapObjects.find(it->getOdData().lanelinkid + LINK_ROAD_ID) == roadid2mapObjects.end()) {
        hadmap::txObjects _objects;
        _objects.push_back(it);
        roadid2mapObjects[it->getOdData().lanelinkid + LINK_ROAD_ID] = _objects;
      } else {
        roadid2mapObjects.find(it->getOdData().lanelinkid + LINK_ROAD_ID)->second.push_back(it);
      }
      */
    }
  }
  for (auto it : roadid2mapObjects) {
    roadpkid roadid = it.first;
    tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
    while (roadEle) {
      if (roadEle->Int64Attribute("id") == static_cast<uint64_t>(roadid)) {
        tinyxml2::XMLElement* _objects;
        if (roadEle->FirstChildElement("objects")) {
          _objects = roadEle->FirstChildElement("objects");
        } else {
          _objects = doc->NewElement("objects");
          roadEle->InsertEndChild(_objects);
        }
        tinyxml2::XMLElement* _signals;
        if (roadEle->FirstChildElement("signals")) {
          _signals = roadEle->FirstChildElement("signals");
        } else {
          _signals = doc->NewElement("signals");
          roadEle->InsertEndChild(_signals);
        }

        for (auto itObject : it.second) {
          tinyxml2::XMLElement* _object = _objects->FirstChildElement("object");
          while (_object) {
            if (itObject->getId() == _object->Int64Attribute("id")) {
              _objects->DeleteChild(_object);
              break;
            }
            _object = _object->NextSiblingElement("object");
          }
          tinyxml2::XMLElement* _tunnel = _objects->FirstChildElement("tunnel");
          while (_tunnel) {
            if (itObject->getId() == _tunnel->Int64Attribute("id")) {
              _objects->DeleteChild(_tunnel);
              break;
            }
            _tunnel = _tunnel->NextSiblingElement("tunnel");
          }

          tinyxml2::XMLElement* _signal = _signals->FirstChildElement("signal");
          while (_signal) {
            if (itObject->getId() == _signal->Int64Attribute("id")) {
              _signals->DeleteChild(_signal);
              break;
            }
            _signal = _signal->NextSiblingElement("signal");
          }
        }
        break;
      }
      roadEle = roadEle->NextSiblingElement("road");
    }
  }

  tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
  while (roadEle) {
    if (roadEle->FirstChildElement("objects") && roadEle->FirstChildElement("objects")->FirstChildElement("object")) {
      objectpkid objectid = roadEle->FirstChildElement("objects")->FirstChildElement("object")->Int64Attribute("id");
      if (objectIdVec.find(objectid) != objectIdVec.end()) {
        roadEle->FirstChildElement("objects")->DeleteChild(
            roadEle->FirstChildElement("objects")->FirstChildElement("object"));
      }
    }
    roadEle = roadEle->NextSiblingElement("road");
  }
  return true;
}
bool OpenDriveOutput::specLaneLinks(const roadpkid roadid, hadmap::txLaneLinks& lanelinks) {
  if (lanelinks.size() < 1) return true;
  // if road contains serveral lanelinks
  tinyxml2::XMLElement* roadEle = doc->RootElement()->FirstChildElement("road");
  while (roadEle) {
    roadpkid _cur_roadid = roadEle->Int64Attribute("id");
    if (_cur_roadid == roadid) {
      doc->RootElement()->DeleteChild(roadEle);
      break;
    }
    roadEle = roadEle->NextSiblingElement("road");
  }
  int index = 0;
  for (auto& itLaneLink : lanelinks) {
    itLaneLink->setOdrRoadId((itLaneLink->getOdrRoadId()) * 10000 + index);
    tinyxml2::XMLElement* ele = insertOneLaneLink(itLaneLink);
    doc->FirstChildElement("OpenDRIVE")->InsertAfterChild(doc->RootElement()->LastChildElement("road"), ele);
    index++;
  }
  return true;
}

int OpenDriveOutput::saveFile(const char* filePath) {
  if (doc != NULL) return doc->SaveFile(filePath);
  return -1;
}

tinyxml2::XMLElement* OpenDriveOutput::insertOneRoad(const hadmap::txRoadPtr& it) {
  tinyxml2::XMLElement* _road = doc->NewElement("road");
  _road->SetAttribute("name", it->getName().c_str());
  _road->SetAttribute("id", static_cast<int64_t>(it->getId()));
  _road->SetAttribute("junction", -1);
  // link
  tinyxml2::XMLElement* _link = doc->NewElement("link");
  tinyxml2::XMLElement* _predecessor = doc->NewElement("predecessor");
  txRoadLink pre = it->getRoadPreLink();
  if (!pre.Id.empty()) {
    _predecessor->SetAttribute("elementType", pre.Type.c_str());
    _predecessor->SetAttribute("elementId", pre.Id.c_str());
    _link->InsertEndChild(_predecessor);
  }
  tinyxml2::XMLElement* _successor = doc->NewElement("successor");
  txRoadLink to = it->getRoadToLink();
  if (!to.Id.empty()) {
    _successor->SetAttribute("elementType", to.Type.c_str());
    _successor->SetAttribute("elementId", to.Id.c_str());
    _link->InsertEndChild(_successor);
  }
  _road->InsertEndChild(_link);
  // type
  tinyxml2::XMLElement* _type = doc->NewElement("type");
  _type->SetAttribute("type", "motorway");
  _type->SetAttribute("s", "0");
  /*
  tinyxml2::XMLElement* _speed = doc->NewElement("speed");
  _speed->SetAttribute("max", ((it->getSpeedLimit() != 0) ? 60 : it->getSpeedLimit()));
  _speed->SetAttribute("unit", "km/h");
  _type->InsertEndChild(_speed);
  */
  _road->InsertEndChild(_type);
  // planview
  const txCurve* curve = it->getGeometry();
  PointVec points;
  CurveToPoints(curve, points);
  std::vector<PlanView> planViewVec;
  // get control point and make planview
  txControlPoint point = it->getControlPoint();
  getPlanViewLink(point, points, planViewVec);
  tinyxml2::XMLElement* _planView = doc->NewElement("planView");
  double length = 0.0;
  for (auto it : planViewVec) {
    tinyxml2::XMLElement* _geometry = doc->NewElement("geometry");
    _geometry->SetAttribute("x", it.x);
    _geometry->SetAttribute("y", it.y);
    _geometry->SetAttribute("length", it.length);
    _geometry->SetAttribute("hdg", it.hdg);
    _geometry->SetAttribute("s", it.s);
    if (it.type == "line") {
      tinyxml2::XMLElement* _line = doc->NewElement("line");
      _geometry->InsertEndChild(_line);
    } else if (it.type == "paramPoly3") {
      tinyxml2::XMLElement* _paramPoly3 = doc->NewElement("paramPoly3");
      _paramPoly3->SetAttribute("aU", it.aU);
      _paramPoly3->SetAttribute("aV", it.aV);
      _paramPoly3->SetAttribute("bU", it.bU);
      _paramPoly3->SetAttribute("bV", it.bV);
      _paramPoly3->SetAttribute("cU", it.cU);
      _paramPoly3->SetAttribute("cV", it.cV);
      _paramPoly3->SetAttribute("dU", it.dU);
      _paramPoly3->SetAttribute("dV", it.dV);
      _paramPoly3->SetAttribute("pRange", "normalized");

      _geometry->InsertEndChild(_paramPoly3);
    } else {
      if (std::abs(it.arc) < 1e-5) {
        tinyxml2::XMLElement* _line = doc->NewElement("line");
        _geometry->InsertEndChild(_line);
      } else {
        tinyxml2::XMLElement* _arc = doc->NewElement("arc");
        _arc->SetAttribute("curvature", it.arc);
        _geometry->InsertEndChild(_arc);
      }
    }
    length = it.s + it.length;
    _planView->InsertEndChild(_geometry);
  }
  _road->SetAttribute("length", length);
  _road->InsertEndChild(_planView);
  // elevationProfile
  tinyxml2::XMLElement* _elevationProfile = doc->NewElement("elevationProfile");
  std::vector<ele> elevec;
  if (it->getElePoint().m_type != "none" && it->getElePoint().m_points.size() > 1) {
    getEleFromControl(it->getElePoint().m_points, elevec);
  } else {
    getEle(points, elevec);
  }
  for (auto itEle : elevec) {
    tinyxml2::XMLElement* _elevation = doc->NewElement("elevation");
    _elevation->SetAttribute("a", itEle.a);
    _elevation->SetAttribute("b", itEle.b);
    _elevation->SetAttribute("c", itEle.c);
    _elevation->SetAttribute("d", itEle.d);
    _elevation->SetAttribute("s", itEle.s);
    _elevationProfile->LinkEndChild(_elevation);
  }
  _road->InsertEndChild(_elevationProfile);
  // lanes
  tinyxml2::XMLElement* _lanes = doc->NewElement("lanes");
  double sectionLength = 0;
  for (auto itSection : it->getSections()) {
    tinyxml2::XMLElement* _laneSection = doc->NewElement("laneSection");
    _laneSection->SetAttribute("s", sectionLength);
    // left
    std::vector<hadmap::txLaneId> leftLanes = itSection->getLeftLanes();
    if (leftLanes.size() > 0) {
      tinyxml2::XMLElement* _left = doc->NewElement("left");
      for (auto& _itlaneid : leftLanes) {
        tinyxml2::XMLElement* _lane_ele = insertOneLane(itSection->getLane(_itlaneid.laneId));
        _left->InsertEndChild(_lane_ele);
      }

      _laneSection->InsertEndChild(_left);
    }
    // center
    tinyxml2::XMLElement* _center = doc->NewElement("center");
    tinyxml2::XMLElement* _lane = doc->NewElement("lane");
    _lane->SetAttribute("id", 0);
    _lane->SetAttribute("level", false);
    _lane->SetAttribute("type", "none");
    txLaneBoundaryPtr centerLine = NULL;
    if (itSection->getRightLanes().size() > 0) {
      centerLine = itSection->getLane(-1)->getLeftBoundary();
    } else if (itSection->getLeftLanes().size() > 0) {
      centerLine = itSection->getLane(1)->getRightBoundary();
    }
    tinyxml2::XMLElement* _roadMark = doc->NewElement("roadMark");
    _roadMark->SetAttribute("sOffset", 0);
    _roadMark->SetAttribute("weight", "standard");
    _roadMark->SetAttribute("height", "0");
    _roadMark->SetAttribute("material", "standard");
    _roadMark->SetAttribute("width", "0.15");
    _roadMark->SetAttribute("laneChange", "none");
    std::string strType = HadMapLaneMark(LANE_MARK(centerLine->getLaneMark() & 0xFF));
    _roadMark->SetAttribute("type", strType.c_str());
    int n = centerLine->getLaneMark() & 0x0F0000;
    std::string strColor = HadMapLaneColor(n);
    _roadMark->SetAttribute("color", strColor.c_str());
    _lane->InsertEndChild(_roadMark);
    _center->InsertEndChild(_lane);
    _laneSection->InsertEndChild(_center);
    // right
    std::vector<hadmap::txLaneId> _rightLanes = itSection->getRightLanes();
    if (_rightLanes.size() > 0) {
      tinyxml2::XMLElement* _right = doc->NewElement("right");
      for (auto& _itlaneid : _rightLanes) {
        tinyxml2::XMLElement* _lane_ele = insertOneLane(itSection->getLane(_itlaneid.laneId));
        _right->InsertEndChild(_lane_ele);
      }
      _laneSection->InsertEndChild(_right);
    }
    _lanes->InsertEndChild(_laneSection);
    sectionLength += itSection->getLength();
  }
  _road->InsertEndChild(_lanes);

  // surface
  if (it->getCrgVec().size() > 0) {
    tinyxml2::XMLElement* _surface = insertCrgs(it->getCrgVec(), it->getLength());
    _road->InsertEndChild(_surface);
  }

  return _road;
}

tinyxml2::XMLElement* OpenDriveOutput::insertOneLaneLink(const hadmap::txLaneLinkPtr& itLink) {
  tinyxml2::XMLElement* _road = doc->NewElement("road");
  _road->SetAttribute("name", "");
  _road->SetAttribute("id", static_cast<int64_t>(itLink->getOdrRoadId()));
  _road->SetAttribute("junction", static_cast<int64_t>(itLink->getJunctionId()));
  // link
  roadpkid fromRoadId = itLink->fromRoadId();
  roadpkid toRoadId = itLink->toRoadId();
  tinyxml2::XMLElement* _link = doc->NewElement("link");
  tinyxml2::XMLElement* _predecessor = doc->NewElement("predecessor");
  std::string startType = ((itLink->getPreContact() == START) ? "start" : "end");
  _predecessor->SetAttribute("contactPoint", startType.c_str());
  _predecessor->SetAttribute("elementId", static_cast<int64_t>(fromRoadId));
  _predecessor->SetAttribute("elementType", "road");
  _link->LinkEndChild(_predecessor);
  tinyxml2::XMLElement* _successor = doc->NewElement("successor");
  std::string startTypeSuc = ((itLink->getSuccContact() == START) ? "start" : "end");
  _successor->SetAttribute("contactPoint", startTypeSuc.c_str());
  _successor->SetAttribute("elementId", static_cast<int64_t>(toRoadId));
  _successor->SetAttribute("elementType", "road");
  _link->LinkEndChild(_successor);
  _road->InsertEndChild(_link);
  // planView
  hadmap::txLineCurve* curve = (hadmap::txLineCurve*)itLink->getGeometry();
  hadmap::PointVec points;

  CurveToPoints(curve, points);
  std::vector<PlanView> planViewVec;
  txControlPoint point = itLink->getControlPoint();
  getPlanViewLink(point, points, planViewVec);
  tinyxml2::XMLElement* _planView = doc->NewElement("planView");
  double length = 0.0;
  for (auto it : planViewVec) {
    tinyxml2::XMLElement* _geometry = doc->NewElement("geometry");
    _geometry->SetAttribute("x", it.x);
    _geometry->SetAttribute("y", it.y);
    _geometry->SetAttribute("length", it.length);
    _geometry->SetAttribute("hdg", it.hdg);
    _geometry->SetAttribute("s", it.s);
    if (it.type == "line") {
      tinyxml2::XMLElement* _line = doc->NewElement("line");
      _geometry->InsertEndChild(_line);
    } else if (it.type == "paramPoly3") {
      tinyxml2::XMLElement* _paramPoly3 = doc->NewElement("paramPoly3");
      _paramPoly3->SetAttribute("aU", it.aU);
      _paramPoly3->SetAttribute("aV", it.aV);
      _paramPoly3->SetAttribute("bU", it.bU);
      _paramPoly3->SetAttribute("bV", it.bV);
      _paramPoly3->SetAttribute("cU", it.cU);
      _paramPoly3->SetAttribute("cV", it.cV);
      _paramPoly3->SetAttribute("dU", it.dU);
      _paramPoly3->SetAttribute("dV", it.dV);
      _paramPoly3->SetAttribute("pRange", "normalized");
      _geometry->InsertEndChild(_paramPoly3);
    } else {
      if (std::abs(it.arc) < 1e-5) {
        tinyxml2::XMLElement* _line = doc->NewElement("line");
        _geometry->InsertEndChild(_line);
      } else {
        tinyxml2::XMLElement* _arc = doc->NewElement("arc");
        _arc->SetAttribute("curvature", it.arc);
        _geometry->InsertEndChild(_arc);
      }
    }
    length = it.s + it.length;
    _planView->InsertEndChild(_geometry);
  }
  _road->SetAttribute("length", length);
  _road->InsertEndChild(_planView);
  // elevationProfile
  tinyxml2::XMLElement* _elevationProfile = doc->NewElement("elevationProfile");
  std::vector<ele> elevec;

  getEle(points, elevec);
  for (auto itEle : elevec) {
    tinyxml2::XMLElement* _elevation = doc->NewElement("elevation");
    _elevation->SetAttribute("a", itEle.a);
    _elevation->SetAttribute("b", itEle.b);
    _elevation->SetAttribute("c", itEle.c);
    _elevation->SetAttribute("d", itEle.d);
    _elevation->SetAttribute("s", itEle.s);
    _elevationProfile->LinkEndChild(_elevation);
  }
  _road->InsertEndChild(_elevationProfile);
  // lanes
  tinyxml2::XMLElement* _lanes = doc->NewElement("lanes");
  double sectionLength = 0;
  tinyxml2::XMLElement* _laneSection = doc->NewElement("laneSection");
  _laneSection->SetAttribute("s", 0);
  _lanes->InsertEndChild(_laneSection);
  // center
  tinyxml2::XMLElement* _center = doc->NewElement("center");
  tinyxml2::XMLElement* _lane = doc->NewElement("lane");
  _lane->SetAttribute("id", 0);
  _lane->SetAttribute("level", false);
  _lane->SetAttribute("type", "none");
  _lane = doc->NewElement("lane");
  tinyxml2::XMLElement* _roadMark = doc->NewElement("roadMark");
  _roadMark->SetAttribute("sOffset", 0);
  _roadMark->SetAttribute("weight", "standard");
  _roadMark->SetAttribute("width", "1.0");
  _roadMark->SetAttribute("height", "0");
  _lane->InsertEndChild(_roadMark);
  _center->InsertEndChild(_lane);
  _laneSection->InsertEndChild(_center);
  // right
  tinyxml2::XMLElement* _right = doc->NewElement("right");
  tinyxml2::XMLElement* _laner = doc->NewElement("lane");
  _laner->SetAttribute("id", -1);
  _laner->SetAttribute("level", false);
  _laner->SetAttribute("type", "driving");
  // link
  int laneidF = itLink->fromTxLaneId().laneId;
  int laneidT = itLink->toTxLaneId().laneId;
  tinyxml2::XMLElement* _lane_link = doc->NewElement("link");
  tinyxml2::XMLElement* _lane_predecessor = doc->NewElement("predecessor");
  _lane_predecessor->SetAttribute("id", laneidF);
  tinyxml2::XMLElement* _lane_successor = doc->NewElement("successor");
  _lane_successor->SetAttribute("id", laneidT);
  _lane_link->InsertEndChild(_lane_predecessor);
  _lane_link->InsertEndChild(_lane_successor);
  _laner->InsertEndChild(_lane_link);
  // roadMark
  tinyxml2::XMLElement* _roadMarkr = doc->NewElement("roadMark");
  _roadMarkr->SetAttribute("sOffset", 0);
  _roadMarkr->SetAttribute("weight", "standard");
  _roadMarkr->SetAttribute("height", "0");
  _roadMarkr->SetAttribute("material", "standard");
  _roadMarkr->SetAttribute("width", "0.2");
  _roadMarkr->SetAttribute("laneChange", "none");
  // width
  tinyxml2::XMLElement* _width = doc->NewElement("width");
  _width->SetAttribute("a", "0.1");
  _width->SetAttribute("b", 0.0);
  _width->SetAttribute("c", 0.0);
  _width->SetAttribute("d", 0.0);
  _width->SetAttribute("sOffset", 0.0);
  _laner->InsertEndChild(_width);
  _laner->InsertEndChild(_roadMarkr);
  _right->InsertEndChild(_laner);
  _laneSection->InsertEndChild(_right);
  // to road
  _road->InsertEndChild(_lanes);
  return _road;
}

tinyxml2::XMLElement* OpenDriveOutput::insertCrgs(const hadmap::txOpenCrgVec& crgs, double roadlength) {
  tinyxml2::XMLElement* _surface = doc->NewElement("surface");
  for (auto it : crgs) {
    tinyxml2::XMLElement* CRG = doc->NewElement("CRG");
    CRG->SetAttribute("sStart", 0);
    CRG->SetAttribute("sEnd", roadlength);
    CRG->SetAttribute("file", it.m_file.c_str());
    CRG->SetAttribute("orientation", it.m_orientation.c_str());
    CRG->SetAttribute("mode", it.m_mode.c_str());
    CRG->SetAttribute("purpose", it.m_purpose.c_str());
    CRG->SetAttribute("sOffset", it.m_soffset.c_str());
    CRG->SetAttribute("tOffset", it.m_toffset.c_str());
    CRG->SetAttribute("zOffset", it.m_zoffset.c_str());
    CRG->SetAttribute("hOffset", it.m_hoffset.c_str());
    CRG->SetAttribute("zScale", it.m_zscale.c_str());
    _surface->InsertEndChild(CRG);
  }
  return _surface;
}

tinyxml2::XMLElement* OpenDriveOutput::insertOneLane(const hadmap::txLanePtr lanePtr) {
  tinyxml2::XMLElement* _lane = doc->NewElement("lane");
  _lane->SetAttribute("id", lanePtr->getTxLaneId().laneId);
  _lane->SetAttribute("level", false);
  _lane->SetAttribute("type", HadMapLaneType(lanePtr->getLaneType()));
  //
  txLaneBoundaryPtr boundaryPtr = NULL;
  if (lanePtr->getTxLaneId().laneId > 0) {
    boundaryPtr = lanePtr->getLeftBoundary();
  } else {
    boundaryPtr = lanePtr->getRightBoundary();
  }

  tinyxml2::XMLElement* _roadMark = doc->NewElement("roadMark");
  _roadMark->SetAttribute("sOffset", 0);
  _roadMark->SetAttribute("weight", "standard");
  _roadMark->SetAttribute("height", "0");
  _roadMark->SetAttribute("material", "standard");
  _roadMark->SetAttribute("width", "0.2");
  _roadMark->SetAttribute("laneChange", "none");

  int laneMark = boundaryPtr->getLaneMark();
  // lane type and color
  _roadMark->SetAttribute("type", HadMapLaneMark(LANE_MARK(boundaryPtr->getLaneMark() & 0xFF)));
  _roadMark->SetAttribute("color", HadMapLaneMark(LANE_MARK(boundaryPtr->getLaneMark() & 0xFFFF00)));
  _lane->InsertEndChild(_roadMark);
  // lane width
  tinyxml2::XMLElement* _width = doc->NewElement("width");
  double laneWidth = GetLaneWidth(lanePtr);

  _width->SetAttribute("a", laneWidth);
  _width->SetAttribute("b", 0.0);
  _width->SetAttribute("c", 0.0);
  _width->SetAttribute("d", 0.0);
  _width->SetAttribute("sOffset", 0.0);
  _lane->InsertEndChild(_width);

  // lane speed
  tinyxml2::XMLElement* _speed = doc->NewElement("speed");
  _speed->SetAttribute("sOffset", 0);
  _speed->SetAttribute("max", lanePtr->getSpeedLimit() / 3.6);
  _lane->InsertEndChild(_speed);

  // lane material
  tinyxml2::XMLElement* _material = doc->NewElement("material");
  _material->SetAttribute("surface", "");
  _material->SetAttribute("friction", lanePtr->getLaneFriction());
  _material->SetAttribute("sOffset", lanePtr->getMaterialOffset());
  _lane->InsertEndChild(_material);

  return _lane;
}

double OpenDriveOutput::GetLaneWidth(const hadmap::txLanePtr& lane) {
  double lanewidth = 0.0;
  if (lane->getLeftBoundary() && lane->getRightBoundary()) {
    const hadmap::txLineCurve* leftCurve =
        dynamic_cast<const hadmap::txLineCurve*>(lane->getLeftBoundary()->getGeometry());
    PointVec _leftPoints;
    leftCurve->getPoints(_leftPoints);
    const hadmap::txLineCurve* rightCurve =
        dynamic_cast<const hadmap::txLineCurve*>(lane->getRightBoundary()->getGeometry());
    PointVec _rightPoints;
    rightCurve->getPoints(_rightPoints);
    if (_leftPoints.size() != _rightPoints.size()) {
      std::cout << "left point size is not equal right point" << std::endl;
      hadmap::txPoint left = _leftPoints.at(0);
      lonlat2xy(left.x, left.y);
      hadmap::txPoint right = _rightPoints.at(0);
      lonlat2xy(right.x, right.y);
      return distanceTwoPoint(left, right);
    }
    for (int i = 0; i < _leftPoints.size(); i++) {
      hadmap::txPoint left = _leftPoints.at(i);
      lonlat2xy(left.x, left.y);
      hadmap::txPoint right = _rightPoints.at(i);
      lonlat2xy(right.x, right.y);
      lanewidth += distanceTwoPoint(left, right);
    }
    lanewidth /= _leftPoints.size();
    std::cout << "lanewidth = " << lanewidth << std::endl;
    return lanewidth;
  }
  return 3.5;
}

const char* OpenDriveOutput::fromatRoadType(const hadmap::ROAD_TYPE& type) {
  // std::string = strType;
  switch (type) {
    default:
      break;
  }
  return nullptr;
}

void OpenDriveOutput::CurveToPoints(const hadmap::txCurve* curve, hadmap::PointVec& points) {
  double fLen = curve->getLength();
  double fInterval = 10;
  if (fLen < 50) {
    fInterval = fLen / 10;
  } else if (fLen >= 50 && fLen < 100) {
    fInterval = fLen / 20;
  } else if (fLen >= 100 && fLen < 300) {
    fInterval = fLen / 30;
  }
  if (fLen < 0.1) {
    fInterval = 1;
  }
  hadmap::PointVec pts;
  curve->sample(fInterval, pts);
  hadmap::PointVec::iterator itr = pts.begin();
  for (; itr != pts.end(); itr++) {
    double x = itr->x;
    double y = itr->y;
    double z = itr->z;
    lonlat2xy(x, y);
    hadmap::txPoint tmp;
    tmp.x = x;
    tmp.y = y;
    tmp.z = z;
    points.push_back(tmp);
  }
}

double OpenDriveOutput::distanceTwoPoint(txPoint a, txPoint b) {
  return sqrt((b.y - a.y) * (b.y - a.y) + (b.x - a.x) * (b.x - a.x));
}

void OpenDriveOutput::FitCenterByLeastSquares(std::vector<hadmap::txPoint> mapPoint, double& radius,
                                              std::vector<double>& radiusVec) const {
  if (mapPoint.size() < 3) {
    radius = 0.0;
    return;
  }
  double radiusVecSum = 0.0;

  for (int i = 0; i < mapPoint.size() - 2; i++) {
    struct point {
      double x;
      double y;
    };
    point P1{mapPoint.at(i).x, mapPoint.at(i).y};
    point P2{mapPoint.at(i + 1).x, mapPoint.at(i + 1).y};
    point P3{mapPoint.at(i + 2).x, mapPoint.at(i + 2).y};
    if (P1.x == P2.x == P3.x) {
      radius = 0;
    } else {
      double dis1, dis2, dis3;
      double cosA, sinA, dis;
      dis1 = sqrt((P1.x - P2.x) * (P1.x - P2.x) + (P1.y - P2.y) * (P1.y - P2.y));
      dis2 = sqrt((P1.x - P3.x) * (P1.x - P3.x) + (P1.y - P3.y) * (P1.y - P3.y));
      dis3 = sqrt((P2.x - P3.x) * (P2.x - P3.x) + (P2.y - P3.y) * (P2.y - P3.y));
      dis = dis1 * dis1 + dis3 * dis3 - dis2 * dis2;

      if (dis1 == 0 || dis2 == 0 || dis3 == 0 || dis == 0) {
        radius = 0;
      } else {
        cosA = dis / (2 * dis1 * dis3);
        if (std::abs(1 - cosA * cosA) < 1e-6 || std::abs(dis2) < 1e-6) {
          radius = 0;
        } else {
          sinA = sqrt(1 - cosA * cosA);
          radius = 0.5 * dis2 / sinA;
          radius = 1 / radius;
        }
        double z = (P1.x - P2.x) * (P3.y - P2.y) - (P1.y - P2.y) * ((P3.x - P2.x));
        if (z > 0) {
          radius = radius * (-1);
        }
      }
    }
    radiusVec.push_back(radius);
    radiusVecSum += radius;
  }
  radius = radiusVecSum / static_cast<double>(mapPoint.size() - 2);
}

double OpenDriveOutput::getEle(const PointVec vec, std::vector<ele>& elevec) {
  PointVec points;
  double s = 0;
  points.push_back(txPoint(0, vec.at(0).z, 0));
  for (int i = 1; i < vec.size(); i++) {
    s += distanceTwoPoint(vec.at(i), vec.at(i - 1));
    points.push_back(txPoint(s, vec.at(i).z, 0));
  }
  double fRadius = 0.0;
  std::vector<double> radiusVec;
  FitCenterByLeastSquares(points, fRadius, radiusVec);
  int index_start = 0;
  int index_end = 2;
  std::vector<std::tuple<int, int>> start2end;
  for (int i = 0; i < radiusVec.size(); i++) {
    if (index_end == radiusVec.size() - 1) {
      start2end.push_back(std::make_tuple(index_start, index_end));
    }
    if (std::fabs(radiusVec.at(i)) < 0.01) {
      index_end++;
    } else {
      start2end.push_back(std::make_tuple(index_start, index_end));
      index_start = index_end;
      index_end++;
    }
  }
  for (int i = 0; i < start2end.size(); i++) {
    int startIndex = std::get<0>(start2end.at(i));
    int endIndex = std::get<0>(start2end.at(i));
    if (startIndex + 2 >= points.size() || startIndex < 0) continue;
    std::cout << "startIndex = " << startIndex << std::endl;
    std::cout << "points = " << points.size() << std::endl;
    double starts = points.at(startIndex).x;
    double starth = points.at(startIndex).y;
    double ends = points.at(startIndex + 2).x;
    double endh = points.at(startIndex + 2).y;
    ele tmp;
    tmp.s = starts;
    tmp.a = starth;
    tmp.b = (endh - starth) / (ends - starts);
    tmp.c = 0.0;
    tmp.d = 0.0;
    elevec.push_back(tmp);
  }
  return true;
}

bool OpenDriveOutput::getPlanViewLink(const std::vector<hadmap::txPoint> points, std::vector<PlanView>& planViewVec) {
  double allLength = 0;
  planViewVec.clear();
  double fRadius = 0.0;
  std::vector<double> radiusVec;
  FitCenterByLeastSquares(points, fRadius, radiusVec);
  double sample_distance = 3.0;
  for (int i = 0; i < points.size() - 1; i++) {
    allLength += distanceTwoPoint(points.at(i + 1), points.at(i));
  }
  if (allLength < 10 || fabs(fRadius) > 0.1) {
    sample_distance = 1.0;
  }
  int index = 0;
  double length = 0;
  for (int i = 0; i < points.size(); i++) {
    double distance = distanceTwoPoint(points.at(index), points.at(i));
    if (i == points.size() - 1) {
      PlanView it;
      it.x = points.at(index).x;
      it.y = points.at(index).y;
      it.s = length;
      it.length = distance;
      it.arc = 0;
      double deg = std::atan2((points.at(i).y - points.at(index).y), (points.at(i).x - points.at(index).x));
      it.hdg = deg;
      planViewVec.push_back(it);
      index = i;
      length += distance;
    }
    if (distance > sample_distance) {
      if (i < radiusVec.size() - 1 && std::abs(radiusVec.at(i - 1)) > 1e-5) {
        PlanView it;
        it.x = points.at(index).x;
        it.y = points.at(index).y;
        double deg = std::atan2((points.at(i).y - points.at(index).y), (points.at(i).x - points.at(index).x));
        it.hdg = deg;
        it.length = distance;
        it.arc = 0.0;
        it.s = length;
        it.arc = 0;
        planViewVec.push_back(it);
        index = i;
        length += distance;
      }
    }
  }

  return true;
}

bool OpenDriveOutput::getPlanViewLink(const txControlPoint controlPoint, const std::vector<hadmap::txPoint> points,
                                      std::vector<PlanView>& planViewVec) {
  if (controlPoint.m_type == "none") {
    return getPlanViewLink(points, planViewVec);
  }
  if (controlPoint.m_type == "line") {
    PlanView tmp;
    tmp.type = "line";
    tmp.s = 0;
    tmp.x = controlPoint.m_points.front().x;
    tmp.y = controlPoint.m_points.front().y;
    tmp.hdg = controlPoint.m_points.front().hdg;
    tmp.length = distanceTwoPoint(controlPoint.m_points.front(), controlPoint.m_points.back());
    planViewVec.push_back(tmp);
  } else if (controlPoint.m_type == "arc") {
    PlanView tmp;
    tmp.type = "arc";
    tmp.s = 0;
    tmp.x = controlPoint.m_points.front().x;
    tmp.y = controlPoint.m_points.front().y;
    tmp.hdg = controlPoint.m_points.front().hdg;
    double arc = 0.0;
    double length = 0.0;
    hadmap::txPoint nextPoint = controlPoint.m_points.front() - controlPoint.m_points.back();

    double nextHdg = std::atan2(nextPoint.y, nextPoint.x);

    getArcAndLength(controlPoint.m_points.front().hdg, controlPoint.m_points.back().hdg, nextHdg,
                    distanceTwoPoint(controlPoint.m_points.front(), controlPoint.m_points.back()), arc, length);
    tmp.arc = arc;
    tmp.length = length;
    planViewVec.push_back(tmp);
  } else if (controlPoint.m_type == "catmullrom") {
    if (controlPoint.m_points.size() < 4) return getPlanViewLink(points, planViewVec);
    double s = 0;
    for (int i = 0; i < controlPoint.m_points.size() - 3; i++) {
      std::vector<hadmap::txPoint> pointVec;
      for (int j = i; j < i + 4; j++) {
        pointVec.push_back(controlPoint.m_points.at(j));
      }
      PlanView tmp;

      controlPointsToParamPloy(pointVec, tmp);
      tmp.s = s;
      s += tmp.length;
      planViewVec.push_back(tmp);
    }
    return true;
  } else {
    return getPlanViewLink(points, planViewVec);
  }
  return true;
}

bool OpenDriveOutput::getPlanView(const std::vector<hadmap::txPoint> points, std::vector<PlanView>& planViewVec) {
  // hadmap::txPoint pa = points.at(0);
  // hadmap::txPoint pb = points.at(1);
  // double dx1 = pb.x - pa.x;

  // hadmap::txPoint pc = points.at(points.size()-2);
  // hadmap::txPoint pd = points.at(points.size()-1);
  // double dx2 = pd.x - pc.x;
  // PlanView tmp;
  // tmp.aU = (2 * pa.x - 2 * pd.x + dx1 + dx2);
  // tmp.bU = (-3 * pa.x - 2 * pd.x + dx1 + dx2);
  // tmp.cU = (0 * pa.x - 0 * pd.x + dx1 + 0);
  // tmp.dU = (1 * pa.x - 0 * pd.x + 0 + 0);

  double fRadius = 0.0;
  std::vector<double> radiusVec;
  // cal radius
  FitCenterByLeastSquares(points, fRadius, radiusVec);
  double radius = 0.0;
  for (int i = 0; i < radiusVec.size(); i++) {
    for (int j = i + 1; j < radiusVec.size(); j++) {
      if (fabs(radiusVec.at(j) - radiusVec.at(i)) < 0.01) {
        if (j == radiusVec.size() - 1) {
          PlanView tmp;
          tmp.startIndex = i;
          tmp.endIndex = j;
          i = j;
          planViewVec.push_back(tmp);
          break;
        }
        continue;
      }
      PlanView tmp;
      tmp.startIndex = i;
      tmp.endIndex = j;
      i = j;
      planViewVec.push_back(tmp);
      break;
    }
  }

  double flength = 0;
  int index = 0;
  for (auto& it : planViewVec) {
    double _length = 0.0;
    int startIndex = 0;
    int nextIndex = 0;
    int endIndex = 0;
    startIndex = it.startIndex;
    nextIndex = it.startIndex + 1;
    endIndex = it.endIndex + 1;
    if (index == planViewVec.size() - 1) endIndex += 1;
    index++;
    double deg = std::atan2((points.at(endIndex).y - points.at(it.startIndex).y),
                            (points.at(it.endIndex).x - points.at(it.startIndex).x));
    _length = distanceTwoPoint(points.at(endIndex), points.at(startIndex));

    it.x = points.at(startIndex).x;
    it.y = points.at(startIndex).y;
    it.s = flength;
    it.arc = 0;
    it.hdg = deg;
    it.length = _length;
    flength += _length;
  }
  return true;
}

int OpenDriveOutput::getEleFromControl(const PointVec vec, std::vector<ele>& elevec) {
  if (vec.size() < 2) return -1;
  elevec.clear();
  // insert firstPoint and endPoint
  PointVec tmp = vec;
  hadmap::txPoint p1 = vec.at(0);
  hadmap::txPoint p2 = vec.at(1);
  hadmap::txPoint p0 = (p1 + p1 - p2);
  tmp.insert(tmp.begin(), p0);
  hadmap::txPoint p3 = vec.at(vec.size() - 2);
  hadmap::txPoint p4 = vec.at(vec.size() - 1);
  hadmap::txPoint p5 = (p4 + p4 - p3);
  tmp.push_back(p5);
  for (int i = 0; i < tmp.size() - 3; i++) {
    PointVec controlVec;
    controlVec.push_back(tmp.at(i));
    controlVec.push_back(tmp.at(i + 1));
    controlVec.push_back(tmp.at(i + 2));
    controlVec.push_back(tmp.at(i + 3));
    ele _ele;
    controlPointsToPloy3(controlVec, _ele);
    elevec.push_back(_ele);
  }
  return 0;
}

void OpenDriveOutput::getArcAndLength(double hdg1, double hdg2, double nextHdg, double length, double& arc,
                                      double& arcLength) {
  double a_b = std::sin(hdg1) * std::cos(nextHdg) - std::cos(hdg1) * std::sin(nextHdg);
  if (a_b > 0) {
    double a = (hdg2 - hdg1);
    if (a < 0) {
      a += (2 * M_PI);
    }
    double half_a = 0.5 * a;
    double R = length * 0.5 / std::sin(half_a);
    arc = 1 / R;
    arcLength = std::abs(a * R);
    return;
  } else {
    double a = (hdg2 - hdg1);
    if (a < 0) {
      a += (2 * M_PI);
    }
    a = 2 * M_PI - a;
    double half_a = 0.5 * a;
    double R = length * 0.5 / std::sin(half_a);
    arc = -1 / R;
    arcLength = std::abs(a * R);
    return;
  }
}

void OpenDriveOutput::lonlat2xy(double& x, double& y) {
  tinyxml2::XMLElement* rootelemet = doc->FirstChildElement("OpenDRIVE");
  if (rootelemet->FirstChildElement("header")) {
    double south = rootelemet->FirstChildElement("header")->DoubleAttribute("south");
    double west = rootelemet->FirstChildElement("header")->DoubleAttribute("west");
    if (rootelemet->FirstChildElement("header")->FirstChildElement("geoReference")) {
      std::string geoReference = rootelemet->FirstChildElement("header")->FirstChildElement("geoReference")->GetText();
#ifdef PROJ_NEW_API
      PJ_CONTEXT* C = proj_context_create();
      PJ* lon2mer = proj_create_crs_to_crs(C, "+proj=longlat +datum=WGS84 +no_defs", /*  EPSG:4326 */
                                           geoReference.c_str(), NULL);
      if (lon2mer) {
        // txlog::printf("proj4 cannot init: %s \t", od_header.getGeoRef().c_str());

        PJ_COORD a = proj_coord(x, y, 0, 0);
        PJ_COORD b = proj_trans(lon2mer, PJ_FWD, a);
        x = b.xyz.x;
        y = b.xyz.y;
        x -= west;
        y -= south;
        proj_destroy(lon2mer);
        proj_context_destroy(C);
        return;
      }
#else
      projPJ pj_src = pj_init_plus("+proj=longlat +datum=WGS84 +no_defs");
      projPJ pj_dst = pj_init_plus(geoReference.c_str());
      if (pj_src && pj_dst) {
        x /= RAD_TO_DEG;
        y /= RAD_TO_DEG;
        int code = pj_transform(pj_src, pj_dst, 1, 0, &x, &y, 0);
        x -= west;
        y -= south;
        return;
      }
#endif
    }
    coord_trans_api::lonlat2mercator(x, y);
    x -= west;
    y -= south;
    return;
  }
  coord_trans_api::lonlat2mercator(x, y);
  return;
}

txPoint OpenDriveOutput::transformAngle(const txPoint centerPoint, const double rad, const txPoint point) {
  txPoint ret;
  ret.x = (point.x - centerPoint.x) * std::cos(rad) - (point.y - centerPoint.y) * std::sin(rad) + centerPoint.x;
  ret.y = (point.y - centerPoint.y) * std::cos(rad) + (point.x - centerPoint.x) * std::sin(rad) + centerPoint.y;
  return ret;
}

void OpenDriveOutput::rotate(double angle, double& x, double& y) {
  const double cos_a = std::cos(angle);
  const double sin_a = std::sin(angle);
  double xx = x * cos_a - y * sin_a;
  double yy = y * cos_a + x * sin_a;
  x = xx;
  y = yy;
}
void OpenDriveOutput::controlPointsToPloy3(const std::vector<txPoint> pts, ele& _ele) {
  if (pts.size() != 4) {
    return;
  }
  const double s = pts.at(1).x;
  // 构造系数矩阵和常数矩阵
  double x1 = pts.at(0).x - s;
  double x2 = pts.at(1).x - s;
  double x3 = pts.at(2).x - s;
  double x4 = pts.at(3).x - s;
  Matrix4d A;
  A << x1 * x1 * x1, x1 * x1, x1, 1, x2 * x2 * x2, x2 * x2, x2, 1, x3 * x3 * x3, x3 * x3, x3, 1, x4 * x4 * x4, x4 * x4,
      x4, 1;
  double y1 = pts.at(0).y;
  double y2 = pts.at(1).y;
  double y3 = pts.at(2).y;
  double y4 = pts.at(3).y;
  Vector4d b(y1, y2, y3, y4);

  // 求解线性方程组
  Vector4d x = A.colPivHouseholderQr().solve(b);

  // 输出结果
  // cout << "三次曲线方程为：y = " << x(0) << "x^3 + " << x(1) << "x^2 + " << x(2) << "x + " << x(3) << endl;
  _ele.d = x(0);
  _ele.c = x(1);
  _ele.b = x(2);
  _ele.a = x(3);
  _ele.s = pts.at(1).x;
  return;
}

void OpenDriveOutput::controlPointsToParamPloy(const std::vector<txPoint> pts, PlanView& plan) {
  if (pts.size() != 4) {
    return;
  }
  txPoint P0 = pts.at(0);
  txPoint P1 = pts.at(1);
  txPoint P2 = pts.at(2);
  txPoint P3 = pts.at(3);
  const txPoint P11 = P1;
  // cal t
  double t0 = 0;
  double t1 = std::pow(distanceTwoPoint(pts.at(0), pts.at(1)), 0.5) + t0;
  double t2 = std::pow(distanceTwoPoint(pts.at(1), pts.at(2)), 0.5) + t1;
  double t3 = std::pow(distanceTwoPoint(pts.at(2), pts.at(3)), 0.5) + t2;
  // cal heading
  double t = t1;
  txPoint A1, A2, A3, B1, B2;
  A1 = P0 * (t1 - t) / (t1 - t0) + P1 * (t - t0) / (t1 - t0);
  A2 = P1 * (t2 - t) / (t2 - t1) + P2 * (t - t1) / (t2 - t1);
  A3 = P2 * (t3 - t) / (t3 - t2) + P3 * (t - t2) / (t3 - t2);
  B1 = A1 * (t2 - t) / (t2 - t0) + A2 * (t - t0) / (t2 - t0);
  B2 = A2 * (t3 - t) / (t3 - t1) + A3 * (t - t1) / (t3 - t1);
  txPoint A1p = (P1 - P0) / (t1 - t0);
  txPoint A2p = (P2 - P1) / (t2 - t1);
  txPoint A3p = (P3 - P2) / (t3 - t2);
  txPoint B1p = (A2 - A1) / (t2 - t0) + A1p * (t2 - t) / (t2 - t0) + A2p * (t - t0) / (t2 - t0);
  txPoint B2p = (A3 - A2) / (t3 - t1) + A2p * (t3 - t) / (t3 - t1) + A3p * (t - t1) / (t3 - t1);
  txPoint Cp = (B2 - B1) / (t2 - t1) + B1p * (t2 - t) / (t2 - t1) + B2p * (t - t1) / (t2 - t1);

  double hdg = std::atan2(Cp.y, Cp.x);

  P0 = P0 - P11;
  rotate(-hdg, P0.x, P0.y);

  P1 = P1 - P11;
  rotate(-hdg, P1.x, P1.y);

  P2 = P2 - P11;

  rotate(-hdg, P2.x, P2.y);

  P3 = P3 - P11;

  rotate(-hdg, P3.x, P3.y);

  t0 = 0;
  t1 = std::pow(distanceTwoPoint(P0, P1), 0.5) + t0;
  t2 = std::pow(distanceTwoPoint(P1, P2), 0.5) + t1;
  t3 = std::pow(distanceTwoPoint(P2, P3), 0.5) + t2;

  // double C = (t2 - t0) *
  double t10 = t1 - t0;
  double t20 = t2 - t0;
  double t21 = t2 - t1;
  double t31 = t3 - t1;
  double t32 = t3 - t2;
  txPoint aa1 = (P1 - P0) * t21 / t10;
  txPoint ba1 = P1;

  txPoint aa2 = P2 - P1;
  txPoint ba2 = P1;

  txPoint aa3 = (P3 - P2) * t21 / t32;
  txPoint ba3 = (P2 * t31 - P3 * t21) / t32;

  txPoint ab1 = (aa2 - aa1) * t21 / t20;
  txPoint bb1 = (aa1 * t21 + aa2 * t10) / t20;
  txPoint cb1 = P1;

  txPoint ab2 = (aa3 - aa2) * t21 / t31;
  txPoint bb2 = (aa2 * t31 - (ba2 - ba3) * t21) / t31;
  txPoint cb2 = P1;

  txPoint a = ab2 - ab1;
  txPoint pp = ab1 - bb1;
  txPoint b = pp + bb2;
  txPoint c = bb1;
  txPoint d = cb1;

  plan.aU = d.x;
  plan.aV = d.y;
  plan.bU = c.x;
  plan.bV = c.y;
  plan.cU = b.x;
  plan.cV = b.y;
  plan.dU = a.x;
  plan.dV = a.y;
  plan.type = "paramPoly3";
  plan.hdg = hdg;
  plan.x = P11.x;
  plan.y = P11.y;

  txPoint lastPoint = pts.at(1);
  double length = 0.0;

  for (int i = 0; i < 101; i++) {
    double p = 0.01 * i;
    txPoint tmp;
    tmp.x = plan.aU + plan.bU * p + plan.cU * p * p + plan.dU * p * p * p;
    tmp.y = plan.aV + plan.bV * p + plan.cV * p * p + plan.dV * p * p * p;

    txPoint tmp111 = tmp;
    rotate(hdg, tmp111.x, tmp111.y);
    tmp111.x += pts.at(1).x;
    tmp111.y += pts.at(1).y;

    tmp.x += pts.at(1).x;
    tmp.y += pts.at(1).y;
    tmp = transformAngle(P11, -hdg, tmp);

    length += distanceTwoPoint(lastPoint, tmp111);
    lastPoint = tmp111;
  }
  plan.length = length;
}

double OpenDriveOutput::getHdg(const hadmap::txPoint frontPoint, const hadmap::txPoint nextPoint) {
  return std::atan2((frontPoint.y - frontPoint.y), (nextPoint.x - nextPoint.x));
}

}  // namespace hadmap
