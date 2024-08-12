// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "par_converter_core.h"
#include <fstream>
#include "inc/car_log.h"
#include "inc/proto_helper.h"
#include "par_converter_helper.h"

namespace tx_car {
///////////////////////////////////////
void ParKeyword::calIaxleIsideProperty() {
  const std::string one("1"), two("2");

  if (mProperty.iaxle.size() > 0) {
    std::vector<std::string> splitted;
    splitString(mProperty.iaxle, splitted, " ");
    if (splitted.size() == 2) {
      if (splitted.back() == one) {
        mProperty.axle = Axle::AXLE_1;
      } else if (splitted.back() == two) {
        mProperty.axle = Axle::AXLE_2;
      } else {
        LOG_ERROR << "iaxle error:" << mProperty.iaxle << ", " << rawLine << "\n";
      }
    } else {
      for (auto& str : splitted) {
        LOG_ERROR << str << "\n";
      }
      LOG_ERROR << "iaxle error:" << mProperty.iaxle << ", " << rawLine << "\n";
    }
  }

  if (mProperty.iside.size() > 0) {
    std::vector<std::string> splitted;
    splitString(mProperty.iside, splitted, " ");
    if (splitted.size() == 2) {
      if (splitted.back() == one) {
        mProperty.side = Side::SIDE_1;
      } else if (splitted.back() == two) {
        mProperty.side = Side::SIDE_2;
      } else {
        LOG_ERROR << "iside error:" << mProperty.iside << ", " << rawLine << "\n";
      }
    } else {
      for (auto& str : splitted) {
        LOG_ERROR << str << "\n";
      }
      LOG_ERROR << "iside error:" << mProperty.iside << ", " << rawLine << "\n";
    }
  }
}

ParDataNode::ParDataNode() {
  keyword.type = KeywordTypes::Not_Keyword;
  keyword.rawLine = "";
  parent.reset();

  nodeRawContent.reserve(256);
  children.reserve(32);
}

ParDataNode::~ParDataNode() {
  LOG_2 << "node released, keyword type:" << keyword.type << ", keyword:" << keyword.rawLine << "\n";
}

void ParDataNode::calIaxleIsideProperty() {
  keyword.calIaxleIsideProperty();

  if (parent.get() != nullptr) {
    parent->calIaxleIsideProperty();
    if (parent->keyword.mProperty.axle > 0) {
      keyword.mProperty.axle = parent->keyword.mProperty.axle;
    }
    if (parent->keyword.mProperty.side > 0) {
      keyword.mProperty.side = parent->keyword.mProperty.side;
    }
  }
}

ParDataNodePtr ParDataNode::makeNode() { return std::make_shared<ParDataNode>(); }

void ParDataNode::printParDataTree(const ParDataNodePtr& node) {
  if (node.get() == nullptr) return;

  LOG_2 << "keyword type:" << node->keyword.type << ", keyword:" << node->keyword.rawLine << "\n";

  if (node->keyword.mProperty.iaxle.size() > 0) {
    LOG_2 << node->keyword.mProperty.iaxle << "\n";
  }

  if (node->keyword.mProperty.iside.size() > 0) {
    LOG_2 << node->keyword.mProperty.iside << "\n";
  }

  // print par data content, key and value of this node
  for (auto iter = node->nodeRawContent.begin(); iter != node->nodeRawContent.end(); ++iter) {
    LOG_2 << "\t" << iter->rawLine << "\n";
  }

  // print children
  for (auto iter = node->children.begin(); iter != node->children.end(); ++iter) {
    ParDataNodePtr child = *iter;
    ParDataNode::printParDataTree(child);
  }

  return;
}
}  // namespace tx_car
