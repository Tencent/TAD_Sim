
// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "inc/car_common.h"

#include <algorithm>
#include <list>
#include <memory>
#include <queue>
#include <stack>
#include <unordered_map>

#include "car.pb.h"
#include "car_basic.pb.h"
#include "carsim_par.pb.h"

namespace tx_car {
// par keyword types
enum KeywordTypes {
  ROOT_BEGIN = 0,
  ROOT_END = 1,
  PARSFILE_BEGIN = 2,
  PARSFILE_END = 3,
  MAP_1D_BEGIN = 4,
  MAP_2D_BEGIN = 5,
  TABLE_END = 6,
  Not_Keyword = 7
};

enum ParDataType { KV = 1, MAP_1D = 2, MAP_2D = 3 };

enum Axle { No_Axle = 0, AXLE_1 = 1, AXLE_2 = 2 };

enum Side { No_Side = 0, SIDE_1 = 1, SIDE_2 = 2 };

struct LineStatus {
  std::string line;
  bool processed = false;
};

struct ParKeywordProperty {
  Axle axle{Axle::No_Axle};
  Side side{Side::No_Side};

  std::string iaxle, iside;  // iaxle, iside extracted from .par file
};

// key word in par file
struct ParKeyword {
  KeywordTypes type{KeywordTypes::Not_Keyword};
  std::string rawLine;
  std::string id;  // extracted id
  ParKeywordProperty mProperty;

  // calculate axle and side of keyword according to iaxle and iside
  void calIaxleIsideProperty();
};
// left key and right key
using KeywordMap = std::unordered_map<KeywordTypes, KeywordTypes, std::hash<int>>;

// parsed carsim .par data
struct ParData {
  ParKeyword keyword;
  ParDataType dataType;
  std::pair<std::string, std::string> keyValue;
  tx_car::Real1DMap map1D;
  tx_car::Real2DMap map2D;
};
using ParDataContainer = std::list<ParData>;
struct ParsedData {
  std::string version;
  ParDataContainer parDatas;
};

// constant
namespace Constants {
// keyword appendex
const char leftAppend[] = "_LEFT";
const char rightAppend[] = "_RIGHT";
const char frontAppend[] = "_FRONT";
const char rearAppend[] = "_REAR";

// keywork with raw string value
const char rootBegin[] = "PARSFILE";
const char rootEnd[] = "END";

const char parsfileBegin[] = "ENTER_PARSFILE";
const char parsfileEnd[] = "EXIT_PARSFILE";

const char map1dBegin[] = "_TABLE";
const char map2dBegin[] = "_CARPET";
const char tableEnd[] = "ENDTABLE";

// skip this keywords
const std::vector<std::string> skipKeyword{"ADD_ENTITY", "LOG_ENTRY", "END_ENTRY", "END_ENTITY"};

const char iaxle[] = "iaxle";  // as property of keyword or value in .par
const char iside[] = "iside";  // as property of keyword or value in .par

// keyword closeloop pair
const KeywordMap keywordMap = {{KeywordTypes::ROOT_BEGIN, KeywordTypes::ROOT_END},
                               {KeywordTypes::PARSFILE_BEGIN, KeywordTypes::PARSFILE_END},
                               {KeywordTypes::MAP_1D_BEGIN, KeywordTypes::TABLE_END},
                               {KeywordTypes::MAP_2D_BEGIN, KeywordTypes::TABLE_END}};

// left and right of operator
const std::vector<KeywordTypes> leftOperators{KeywordTypes::ROOT_BEGIN, KeywordTypes::PARSFILE_BEGIN,
                                              KeywordTypes::MAP_1D_BEGIN, KeywordTypes::MAP_2D_BEGIN};
const std::vector<KeywordTypes> rightOperators{KeywordTypes::ROOT_END, KeywordTypes::PARSFILE_END,
                                               KeywordTypes::TABLE_END};

namespace version {
const char carsim_2016_1[] = "CarSim 2016.1";
const char carsim_2019_1[] = "CarSim 2019.1";
}  // namespace version
}  // namespace Constants

// data in par file
struct ParDataNode;
using ParDataNodePtr = std::shared_ptr<ParDataNode>;

// .par data tree, keyword is PARSFILE, ENTER_PARSFILE, _TABLE, _CARPET
struct ParDataNode {
  ParKeyword keyword;                      // keyword of this node
  std::vector<ParKeyword> nodeRawContent;  // raw content of this keyword
  std::vector<ParDataNodePtr> children;    // children, tree, organized in layer order
  ParDataNodePtr parent;                   // parent

  ParDataNode();
  virtual ~ParDataNode();

  // calculate axle and side of keyword according to iaxle and iside
  void calIaxleIsideProperty();
  static ParDataNodePtr makeNode();
  static void printParDataTree(const ParDataNodePtr& parTree);
};

// fwd
class ParConverter;
using ParConverterPtr = std::shared_ptr<ParConverter>;
}  // namespace tx_car
