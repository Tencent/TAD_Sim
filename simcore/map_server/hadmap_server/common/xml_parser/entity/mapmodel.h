/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Modify history:
 ******************************************************************************/

#pragma once
#include "json/value.h"
//
class CMapModel {
 public:
  static int getModelListJson(const char* modelPath, const char* modelDir, Json::Value& json);
  static int addModelFromJson(const char* modelPath, const Json::Value& json, int flagupdate = 0);
  static int delModelFromName(const char* modelPath, const std::string variablename);
  static int getModelPathFromName(const char* modelPath, const std::string variablename);

 private:
  static constexpr const char* kDefaultModelTpl = R"(
<?xml version="1.0"?>
<Tadsim>
    <FileHeader revMajor="1" revMinor="0" date="1970-01-01T08:00:00" description="" author="TAD Sim" />
    <MapModels>
    </MapModels>
</Tadsim>
    )";
};
