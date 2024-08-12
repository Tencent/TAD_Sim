// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <string>

#include "common/macros.h"
#include "structs/hadmap_predef.h"
#include "types/map_defs.h"

namespace hadmap {
class TXSIMMAP_API UrlGen {
 public:
  explicit UrlGen(const std::string& location);

  ~UrlGen();

 public:
  // get url result
  std::string url();

 public:
  // search db
  UrlGen& db(const std::string& dbName);

 public:
  // search cmd
  UrlGen& getRoad();

  UrlGen& getAllRoad();

  UrlGen& getRoadMaxId();

  UrlGen& getSection();

  UrlGen& getLane();

  UrlGen& getLink();

  UrlGen& getLinkMaxId();

  UrlGen& getBoundary();

  UrlGen& getBoundaryMaxId();

  UrlGen& getObject();

  UrlGen& getRoute();

 public:
  // search condition
  UrlGen& envelope(const double& lbX, const double& lbY, const double& rbX, const double& rbY);

  UrlGen& roadId(const roadpkid& rid);

  UrlGen& sectionId(const sectionpkid& sid);

  UrlGen& laneId(const lanepkid& lid);

  UrlGen& center(const double& lon, const double& lat);

  UrlGen& radius(const double& r);

  UrlGen& topoId(const txLaneId& fromId, const txLaneId& toId);

  UrlGen& wholeData();

 private:
  void urlConcat(const std::string& info);

 private:
  std::string curUrl;
};
}  // namespace hadmap
