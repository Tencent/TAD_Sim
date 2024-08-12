/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#pragma once

#include <hadmap.h>
#include "entity_link2.h"
#include "image_params.h"
#include "model_params.h"
#include "parser_hadmap_json.h"

namespace Json {
class Value;
}

class CMapSceneV2;

class CParserHadmapJsonV2 : public CParserHadmapJson {
 public:
  CMapSceneV2* ParseV2(const char* strJson);

  int ParseCross(CCross* pCross, hadmap::txLaneLinks& laneLinks, const char* strJson, double dRefLon, double dRefLat,
                 double dRefAlt);

  int ParseLink(CLinkEntityV2* pLink, hadmap::txLaneLinkPtr& laneLinkPtr, const char* strJson, double dRefLon,
                double dRefLat, double dRefAlt);

  std::string ToJson(CMapSceneV2* pScene);

  int ParseGISImageRenameParams(const char* strJson, sTagGISImageRenameParams& param);
  int ParseGISImageDeleteParams(const char* strJson, sTagGISImageDeleteParams& param);
  int ParseGISImageUploadParams(const char* strJson, sTagGISImageUploadParams& param);

  int ParseGISModelRenameParams(const char* strJson, sTagGISModelRenameParams& param);
  int ParseGISModelDeleteParams(const char* strJson, sTagGISModelDeleteParams& param);
  int ParseGISModelUploadParams(const char* strJson, sTagGISModelUploadParams& param);

 protected:
  int ExtractOneLinkData(CLinkEntityV2* pLink, hadmap::txLaneLinkPtr& laneLinkPtr, Json::Value& root, double dRefLon,
                         double dRefLat, double dRefAlt);
};
