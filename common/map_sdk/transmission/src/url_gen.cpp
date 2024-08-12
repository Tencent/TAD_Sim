// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "transmission/url_gen.h"

namespace hadmap {
UrlGen::UrlGen(const std::string& location) : curUrl(location) { curUrl += "?"; }
UrlGen::~UrlGen() {}

void UrlGen::urlConcat(const std::string& info) {
  if (curUrl.back() == '?') {
    curUrl += info;
  } else {
    curUrl += "&" + info;
  }
}

std::string UrlGen::url() { return curUrl; }

UrlGen& UrlGen::db(const std::string& dbName) {
  urlConcat("mapname=" + dbName);
  return *this;
}

UrlGen& UrlGen::getRoad() {
  urlConcat("cmd=getRoads");
  return *this;
}

UrlGen& UrlGen::getAllRoad() {
  urlConcat("cmd=getRoads");
  return *this;
}

UrlGen& UrlGen::getRoadMaxId() {
  urlConcat("cmd=getRoadMaxId");
  return *this;
}

UrlGen& UrlGen::getSection() {
  urlConcat("cmd=getSections");
  return *this;
}

UrlGen& UrlGen::getLane() {
  urlConcat("cmd=getLanes");
  return *this;
}

UrlGen& UrlGen::getLink() {
  urlConcat("cmd=getLaneLinks");
  return *this;
}

UrlGen& UrlGen::getLinkMaxId() {
  urlConcat("cmd=getLaneLinkMaxId");
  return *this;
}

UrlGen& UrlGen::getBoundary() {
  urlConcat("cmd=getLaneBoundaries");
  return *this;
}

UrlGen& UrlGen::getBoundaryMaxId() {
  urlConcat("cmd=getBoundaryMaxId");
  return *this;
}

UrlGen& UrlGen::getObject() {
  urlConcat("cmd=getObjects");
  return *this;
}

UrlGen& UrlGen::getRoute() {
  urlConcat("cmd=queryRoute");
  return *this;
}

UrlGen& UrlGen::roadId(const roadpkid& rid) {
  char info[128];
  sprintf(info, "roadId=%s", std::to_string(rid).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::sectionId(const sectionpkid& sid) {
  char info[128];
  sprintf(info, "sectionId=%s", std::to_string(sid).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::laneId(const lanepkid& lid) {
  char info[128];
  sprintf(info, "laneId=%s", std::to_string(lid).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::center(const double& lon, const double& lat) {
  char info[128];
  sprintf(info, "centerLon=%s&centerLat=%s", std::to_string(lon).c_str(), std::to_string(lat).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::radius(const double& radius) {
  char info[128];
  sprintf(info, "radius=%s", std::to_string(radius).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::topoId(const txLaneId& fromId, const txLaneId& toId) {
  char info[256];
  sprintf(info, "fromRoadId=%s&toRoadId=%s", std::to_string(fromId.roadId).c_str(),
          std::to_string(toId.roadId).c_str());

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::envelope(const double& lbX, const double& lbY, const double& rbX, const double& rbY) {
  char info[256];
  sprintf(info, "startLon=%lf&startLat=%lf&endLon=%lf&endLat=%lf", lbX, lbY, rbX, rbY);

  urlConcat(info);
  return *this;
}

UrlGen& UrlGen::wholeData() {
  urlConcat("wholeData=1");
  return *this;
}
}  // namespace hadmap
