// Copyright 2024 Tencent Inc. All rights reserved.
//
// Author: kekesong@tencent.com
//
#pragma once
#include <string>
#include "tinyxml2.h"

tinyxml2::XMLElement *GetXMLRoot(const std::string &path, tinyxml2::XMLDocument &doc);
const tinyxml2::XMLElement *TryGetChildNode(const tinyxml2::XMLElement *f, const char *name);
