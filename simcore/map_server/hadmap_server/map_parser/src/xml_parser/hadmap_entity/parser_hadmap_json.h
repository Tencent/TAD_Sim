/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <structs/hadmap_curve.h>
#include <structs/hadmap_map.h>
#include <structs/hadmap_section.h>
#include <string>

class CMapScene;
class CCross;
class CParserHadmapJson {
 public:
  CMapScene* Parse(const char* strJson);
  int JsonToXml(const char* strPath, const char* strFilename, const char* strJson);
  int ParseRoad(const char* strJson, hadmap::txRoadPtr& road, hadmap::txLaneBoundaries& vecBoundaries, double dRefLon,
                double dRefLat, double dRefAlt);

  int ParseCross(CCross* pCross, const char* strJson, double dRefLon, double dRefLat, double dRefAlt);

  std::string ToJson(CMapScene* pScene);

 protected:
};
