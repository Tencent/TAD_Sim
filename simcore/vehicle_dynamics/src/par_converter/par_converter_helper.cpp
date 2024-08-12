// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "par_converter_helper.h"

#include <cctype>
#include <fstream>
#include <string>

#include "inc/car_log.h"
#include "inc/proto_helper.h"

namespace tx_car {
ParHelper::ParHelper(const std::string& parPath) {
  // reset
  reset();

  // do init work
  bool parExist = tx_car::isFileExist(parPath);

  if (!parExist) {
    mState.m_state = tx_car::TxCarStateCode::parameter_error;
    mState.m_info = "par file does not exist.";
    LOG_ERROR << mState.m_info << "\n";
    return;
  }

  if (!tx_car::car_config::loadFromFile(mParContent, parPath)) {
    mState.m_state = tx_car::TxCarStateCode::parameter_error;
    mState.m_info = "par file load error.";
    LOG_ERROR << mState.m_info << "\n";
    return;
  }

  mParPath = parPath;
}

void ParHelper::reset() {
  mState.m_state = tx_car::TxCarStateCode::no_error;
  mState.m_info = "everything is fine";

  mParContent.clear();

  while (!mKeyword_Stack.empty()) {
    mKeyword_Stack.pop();
  }

  mParTree.reset();
  mParTree = ParDataNode::makeNode();
  mCurNode = mParTree;

  LOG_0 << "reset done.\n";
}

void ParHelper::process(ParsedData& parsedData) {
  if (!isParserValid()) {
    printParserStateInfo();
    return;
  }

  // pre-process
  for (auto iter = mParContent.begin(); iter != mParContent.end();) {
    auto& line = *iter;

    if (beginWithChar(line, '!')) {
      std::string versionCarsim = extractVersion(line);
      if (versionCarsim.size() > 0) {
        tx_car::trim(versionCarsim, '\n');
        tx_car::trim(versionCarsim, '\r');
        parsedData.version = versionCarsim;
      }
    }

    // remove comment
    tx_car::trim(line, '!');
    // tx_car::trim(line, '#'); // keep # comment, since suspension compliance missing iaxle, need to use information
    // from #
    tx_car::trim(line, '\n');
    tx_car::trim(line, '\r');

    // remove empty line
    if (line.size() == 0) {
      iter = mParContent.erase(iter);
    } else {
      ++iter;
    }

    // remove all space and tab at beginning
    removeAllCharAtBeginning(line, ' ');
    removeAllCharAtBeginning(line, '\t');
  }

  LOG_2 << "pre-process done.\n";
}

std::string ParHelper::extractVersion(const std::string& line) {
  if (line.find("File created:") != line.npos) {
    auto startIndex = line.find("CarSim");
    if (startIndex != line.npos) {
      mCarsimVersion = line.substr(startIndex);
      tx_car::trim(mCarsimVersion, '\n');
      tx_car::trim(mCarsimVersion, '\r');
      return mCarsimVersion;
    }
  }

  return std::string();
}

void ParHelper::iaxleIsidePrefixProcess(ParKeyword& keyword, LineStatus& prefix1, LineStatus& prefix2) {
  // check prefix2 with iaxle and iside
  if (prefix2.processed) return;

  if (this->beginWith(prefix2.line, Constants::iaxle)) {
    keyword.mProperty.iaxle = prefix2.line;
    prefix2.processed = true;
  } else if (this->beginWith(prefix2.line, Constants::iside)) {
    keyword.mProperty.iside = prefix2.line;
    prefix2.processed = true;
  }

  // check prefix1 with iaxle and iside
  if (prefix1.processed) return;

  if (this->beginWith(prefix1.line, Constants::iaxle)) {
    keyword.mProperty.iaxle = prefix1.line;
    prefix1.processed = true;
  } else if (this->beginWith(prefix1.line, Constants::iside)) {
    keyword.mProperty.iside = prefix1.line;
    prefix1.processed = true;
  }
}

void ParHelper::parse(ParsedData& parsedData) {
  // build par tree
  buildParTree();

  // print par tree
  // tx_car::ParDataNode::printParDataTree(getParDataTree());

  // parse .par data from tree
  convertParData(mParTree, parsedData.parDatas);

  LOG_2 << "parse done.\n";
}

void ParHelper::buildParTree() {
  // line prefix 2 is closer to current line
  LineStatus linePrefix1, linePrefix2;

  // build .par data tree
  for (auto iter = mParContent.begin(); iter != mParContent.end(); iter++) {
    const auto& line = *iter;
    ParKeyword&& keyword = extractKeywordFromLine(line);

    // skip empty line
    if (keyword.rawLine.size() <= 1) {
      VLOG(3) << "keyword line should contain at least 2 words ---> " << line << "\n";
      continue;
    }

    // par data content matched
    if (!isCarSimKeyword(keyword.type)) {
      // process keyword line except property line
      if (!isPropertyLine(keyword.rawLine)) {
        // calculate property of keyword if it has
        iaxleIsidePrefixProcess(keyword, linePrefix1, linePrefix2);
        // push keyword
        mCurNode->nodeRawContent.push_back(keyword);
      }
    } else {
      // build tree, keyword is PARSFILE, ENTER_PARSFILE, _TABLE, _CARPET
      if (keyword.type == KeywordTypes::MAP_2D_BEGIN) {
        // map2d keyword begin
        ParDataNodePtr node = ParDataNode::makeNode();
        node->parent = mCurNode;
        node->keyword = keyword;
        mCurNode->children.push_back(node);
        mCurNode = node;
        iaxleIsidePrefixProcess(node->keyword, linePrefix1, linePrefix2);
      } else if (keyword.type == KeywordTypes::MAP_1D_BEGIN) {
        // map1d keyword begin
        ParDataNodePtr node = ParDataNode::makeNode();
        node->parent = mCurNode;
        node->keyword = keyword;
        mCurNode->children.push_back(node);
        mCurNode = node;
        iaxleIsidePrefixProcess(node->keyword, linePrefix1, linePrefix2);
      } else if (keyword.type == KeywordTypes::TABLE_END) {
        // map1d keyword end
        mCurNode = mCurNode->parent;

        linePrefix2.line = "";
        linePrefix2.processed = false;
        linePrefix1 = linePrefix2;
      } else if (keyword.type == KeywordTypes::PARSFILE_BEGIN) {
        // parsfile keyword begin
        ParDataNodePtr node = ParDataNode::makeNode();
        node->parent = mCurNode;
        node->keyword = keyword;
        mCurNode->children.push_back(node);
        mCurNode = node;
        iaxleIsidePrefixProcess(node->keyword, linePrefix1, linePrefix2);
      } else if (keyword.type == KeywordTypes::PARSFILE_END) {
        // parsfile keyword end
        mCurNode = mCurNode->parent;
        linePrefix2.line = "";
        linePrefix2.processed = false;
        linePrefix1 = linePrefix2;
      } else if (keyword.type == KeywordTypes::ROOT_BEGIN) {
        // root keyword begin
        mCurNode = mParTree;
        mCurNode->keyword = keyword;
      } else if (keyword.type == KeywordTypes::ROOT_END) {
        // root keyword end
        mCurNode = mParTree;
      }
    }

    // update line prefix
    linePrefix1 = linePrefix2;

    linePrefix2.line = line;
    linePrefix2.processed = false;
  }
}

void ParHelper::convertParData(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  if (node.get() == nullptr) return;

  const auto& keyword = node->keyword;

  if (keyword.type == KeywordTypes::MAP_1D_BEGIN) {
    LOG_2 << "converting map1d:" << node->keyword.id << "\n";
    convertMap1d(node, parContainer);
    LOG_2 << "end of converting map1d:" << node->keyword.id << "\n";
  } else if (keyword.type == KeywordTypes::MAP_2D_BEGIN) {
    LOG_2 << "converting map2d:" << node->keyword.id << "\n";
    convertMap2d(node, parContainer);
    LOG_2 << "end of converting map2d:" << node->keyword.id << "\n";
  } else if (keyword.type == KeywordTypes::PARSFILE_BEGIN) {
    LOG_2 << "converting ENTER_PARSFILE:" << node->keyword.id << "\n";
    convertParsfile(node, parContainer);
    LOG_2 << "end of converting ENTER_PARSFILE:" << node->keyword.id << "\n";
  } else if (keyword.type == KeywordTypes::ROOT_BEGIN) {
    LOG_2 << "converting PARSFILE:" << node->keyword.id << "\n";
    convertRoot(node, parContainer);
    LOG_2 << "end of converting PARSFILE:" << node->keyword.id << "\n";
  } else {
    LOG_WARNING << "unknown keyword:" << keyword.rawLine << "\n";
  }

  for (auto child = node->children.begin(); child != node->children.end(); ++child) {
    convertParData(*child, parContainer);
  }
}

bool ParHelper::convertPreProcess(const ParDataNodePtr& node) {
  // get keyword in carsim
  {
    std::vector<std::string> splitted;
    splitString(node->keyword.rawLine, splitted, " ");

    node->keyword.id = splitted.front();
    node->calIaxleIsideProperty();

    VLOG(3) << "keyword:" << node->keyword.id << "\n";

    if (node->keyword.mProperty.iaxle.size() > 0) {
      LOG_2 << node->keyword.mProperty.iaxle << "\n";
    }

    if (node->keyword.mProperty.iside.size() > 0) {
      LOG_2 << node->keyword.mProperty.iside << "\n";
    }
  }

  if (node->keyword.id.size() <= 1) {
    LOG_ERROR << "error while parse " << node->keyword.rawLine << "\n";
    return false;
  }

  return true;
}

void ParHelper::convertMap1d(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  auto& nodeRawContent = node->nodeRawContent;
  const std::string removeChars(" \t");

  // get keyword in carsim
  LOG_2 << "keyword id in map1d:" << node->keyword.id << "\n";
  auto state = convertPreProcess(node);
  if (!state) return;

  // set name of map1d
  ParData parData;
  parData.map1D.set_disp_name(node->keyword.rawLine.c_str());

  // extract table
  for (auto& line : nodeRawContent) {
    // remove space and tab
    removeCharsFromString(line.rawLine, removeChars);

    if (line.rawLine.empty()) {
      LOG_ERROR << "\tmap 1d has an empty line, " << node->keyword.rawLine << "\n";
      continue;
    }

    if (beginWithChar(line.rawLine, '#')) {
      continue;
    }

    std::vector<std::string> splitted;
    splitString(line.rawLine, splitted, ",");

    if (splitted.size() != 2) {
      LOG_ERROR << "\tmap 1d data format error, " << node->keyword.id << ", " << line.rawLine << "\n";
      continue;
    }
    try {
      double u0 = std::atof(splitted.front().c_str());
      double y0 = std::atof(splitted.back().c_str());
      parData.map1D.mutable_u0_axis()->add_data(u0);
      parData.map1D.mutable_y0_axis()->add_data(y0);
      LOG_2 << "\ttable data:[" << u0 << "," << y0 << "]\n";
    } catch (const std::exception& e) {
      LOG_ERROR << "\terror " << node->keyword.id << ", " << line.rawLine << e.what() << ".\n";
      return;
    }
  }
  parData.map1D.set_data_source(tx_car::CarSim_PAR);
  parData.dataType = ParDataType::MAP_1D;
  parData.keyword = node->keyword;

  parContainer.push_back(parData);
}

void ParHelper::convertMap2d(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  auto& nodeRawContent = node->nodeRawContent;
  const std::string removeChars(" \t\n\r");

  // get keyword in carsim
  LOG_2 << "keyword in map2d:" << node->keyword.id << "\n";
  auto state = convertPreProcess(node);
  if (!state) return;

  // set name of map1d
  ParData parData;
  parData.map2D.set_disp_name(node->keyword.rawLine.c_str());

  // extract table
  const int maxSize = tx_car::kMap1dSize;
  double map2dMatrix[maxSize][maxSize];
  memset(map2dMatrix, 0x00, maxSize * maxSize);
  int row = 0, col = 0;

  for (auto& line : nodeRawContent) {
    // remove space and tab
    removeCharsFromString(line.rawLine, removeChars);
    VLOG(0) << "\t" << line.rawLine << "\n";

    if (line.rawLine.empty()) {
      LOG_ERROR << "\tmap 2d has an empty line, " << node->keyword.rawLine << "\n";
      continue;
    }

    if (beginWithChar(line.rawLine, '#')) {
      continue;
    }

    std::vector<std::string> tableRowStrs;
    splitString(line.rawLine, tableRowStrs, ",");

    if (tableRowStrs.size() < 3) {
      LOG_ERROR << "\tmap 2d data format error, " << node->keyword.id << ", " << line.rawLine << "\n";
      continue;
    }

    col = 0;
    for (auto& str : tableRowStrs) {
      double v = std::stof(str);
      map2dMatrix[row][col] = v;
      col++;
    }
    row++;
  }

  LOG_2 << "\tnumber of lines in map2d:" << nodeRawContent.size() << ", row:" << row << ", col:" << col << "\n";

  // copy data
  if (row < 3 || col < 3) {
    LOG_ERROR << "\terror " << node->keyword.id << ", row or column size. " << row << ", " << col << "\n";
    return;
  }

  // copy
  for (auto i = 1; i < row; ++i) {
    parData.map2D.mutable_u0_axis()->add_data(map2dMatrix[i][0]);
  }
  for (auto i = 1; i < col; ++i) {
    parData.map2D.mutable_u1_axis()->add_data(map2dMatrix[0][i]);
  }
  // coloumn order
  for (auto c = 1; c < col; ++c) {
    for (auto r = 1; r < row; ++r) {
      parData.map2D.mutable_y0_axis()->add_data(map2dMatrix[r][c]);
    }
  }

  printMap2d(parData.map2D);
  parData.map2D.set_data_source(tx_car::CarSim_PAR);
  parData.dataType = ParDataType::MAP_2D;
  parData.keyword = node->keyword;

  parContainer.push_back(parData);
}

void ParHelper::processCompliance(const ParDataNodePtr& node) {
  if (mCarsimVersion == Constants::version::carsim_2016_1 || mCarsimVersion == Constants::version::carsim_2019_1) {
    const auto& keyword = node->keyword;
    const std::string complianceConst = "ENTER_PARSFILE Suspensions\\Compliance";

    const std::string frontComp_1 = "front comp", frontComp_2 = "front";
    const std::string rearComp_1 = "rear comp", rearComp_2 = "rear";

    LOG_2 << "inside compliance process.\n";

    auto convertToLower = [](const std::string& strIn) -> std::string {
      std::string strOut = strIn;

      for (auto& ch : strOut) {
        ch = static_cast<char>(std::tolower(ch));
      }

      return strOut;
    };

    // no iaxle and suspension compliance
    if (node->keyword.mProperty.iaxle.empty() && beginWith(keyword.rawLine, complianceConst)) {
      if (node->nodeRawContent.size() > 0) {
        // get first line in ENTER_PARSFILE
        const auto& firstLine = node->nodeRawContent.front();
        std::string line = convertToLower(firstLine.rawLine);
        if (line.find(frontComp_1) != line.npos || line.find(frontComp_2) != line.npos) {
          LOG_2 << "front compliance process.\n";
          node->keyword.mProperty.iaxle = "iaxle 1";
        } else if (line.find(rearComp_1) != line.npos || line.find(rearComp_2) != line.npos) {
          LOG_2 << "rear compliance process.\n";
          node->keyword.mProperty.iaxle = "iaxle 2";
        }
      }
    }
  }
}

void ParHelper::convertParsfile(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  // special process for compliance since iaxle missing in .par file
  processCompliance(node);

  convertKVOfNode(node, parContainer);
}

void ParHelper::convertRoot(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  convertKVOfNode(node, parContainer);
}

void ParHelper::convertKVOfNode(const ParDataNodePtr& node, ParDataContainer& parContainer) {
  auto& nodeRawContent = node->nodeRawContent;

  // get keyword in carsim
  LOG_2 << "keyword in parsfile:" << node->keyword.id << " ---> " << node->keyword.rawLine << "\n";
  auto state = convertPreProcess(node);
  if (!state) return;

  // extract key and value
  for (auto& keyword_ : nodeRawContent) {
    if (keyword_.rawLine.empty()) {
      LOG_ERROR << "\tempty line, " << node->keyword.rawLine << "\n";
      continue;
    }

    // comment line, skip
    if (beginWithChar(keyword_.rawLine, '#')) {
      LOG_WARNING << "\tskpipped keyword rawline:" << keyword_.rawLine << "\n";
      continue;
    }

    std::vector<std::string> splitted;
    splitString(keyword_.rawLine, splitted, " ");

    if (splitted.size() != 2) {
      LOG_WARNING << "\tskipped " << keyword_.rawLine << " ---> splitted size:" << splitted.size()
                  << " ---> node:" << node->keyword.rawLine << "\n";
      continue;
    }

    ParData parData;

    keyword_.calIaxleIsideProperty();
    if (node->keyword.mProperty.axle > 0) {
      keyword_.mProperty.axle = node->keyword.mProperty.axle;
    }
    if (node->keyword.mProperty.side > 0) {
      keyword_.mProperty.side = node->keyword.mProperty.side;
    }
    parData.keyValue.first = splitted.front();
    keyword_.id = splitted.front();
    parData.keyValue.second = splitted.back();
    parData.dataType = ParDataType::KV;
    parData.keyword = keyword_;

    parContainer.push_back(parData);

    LOG_2 << "\t" << parData.keyValue.first << " <- " << parData.keyValue.second << "\n";
  }

  LOG_2 << "end process of " << node->keyword.rawLine << "\n";
}

bool ParHelper::pushStack(const ParKeyword& keyword) {
  if (!isCarSimKeyword(keyword.type)) {
    LOG_2 << "error, not .par keyword, " << keyword.rawLine;
    return false;
  }

  if (mKeyword_Stack.empty()) {
    LOG_2 << "error, stack should be empty here.\n";
    return false;
  }

  // push stack if stack top is left operator
  const auto& front = mKeyword_Stack.top();
  if (isLeftOperator(front.type)) {
    mKeyword_Stack.push(keyword);
    return true;
  } else {
    LOG_2 << "error, front of stack should be left operator, " << front.rawLine << "\n";
  }

  return false;
}

bool ParHelper::popStack(ParKeyword& keyword) {
  if (!isCarSimKeyword(keyword.type)) {
    LOG_2 << "error, not .par keyword, " << keyword.type;
    return false;
  }

  if (mKeyword_Stack.empty()) {
    LOG_2 << "error, stack should be empty here.\n";
    return false;
  }

  const auto& front = mKeyword_Stack.top();
  if (!isLeftOperator(front.type) || !isRightOperator(keyword.type)) {
    LOG_2 << "error, front of stack should be left operator, " << front.rawLine << keyword.rawLine << "\n";
    return false;
  }

  auto iter = Constants::keywordMap.find(front.type);
  if (iter == Constants::keywordMap.end()) {
    LOG_2 << "unknown par operator pair" << front.type << "\n";
    return false;
  }

  // if matched left and right operator with stack top operator
  if (iter->second == keyword.type) {
    mKeyword_Stack.pop();
    return true;
  } else {
    LOG_2 << "error, pop operator " << keyword.type << ", but front of stack:" << front.rawLine << "\n";
  }

  return false;
}

bool ParHelper::checkIfParSymmetry() {
  for (auto iter = mParContent.begin(); iter != mParContent.end(); iter++) {
    const auto& line = *iter;
    ParKeyword&& keyword = extractKeywordFromLine(line);

    VLOG(3) << "keyword type:" << keyword.type << ", id:" << keyword.id << ", raw line:" << keyword.rawLine << "\n";

    // skip empty line
    if (keyword.rawLine.size() <= 1) {
      VLOG(3) << "keyword line should contain at least 2 words. ---> " << line << "\n";
      continue;
    }

    // when new par keyword matched
    if (keyword.type == KeywordTypes::MAP_2D_BEGIN) {
      // map2d keyword begin
      if (!pushStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to push"));
      }
    } else if (keyword.type == KeywordTypes::MAP_1D_BEGIN) {
      // map1d keyword begin
      if (!pushStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to push"));
      }
    } else if (keyword.type == KeywordTypes::TABLE_END) {
      // map1d keyword end
      if (!popStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to pop"));
      }
    } else if (keyword.type == KeywordTypes::PARSFILE_BEGIN) {
      if (!pushStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to push"));
      }
    } else if (keyword.type == KeywordTypes::PARSFILE_END) {
      // parsfile keyword end
      if (!popStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to pop"));
      }
    } else if (keyword.type == KeywordTypes::ROOT_BEGIN) {
      // root keyword begin
      if (mKeyword_Stack.empty()) {
        mKeyword_Stack.push(keyword);
      } else {
        printKeyword(keyword);
        throw(std::runtime_error("fail to push"));
      }
    } else if (keyword.type == KeywordTypes::ROOT_END) {
      // root keyword end
      if (!popStack(keyword)) {
        printKeyword(keyword);
        throw(std::runtime_error("fail to pop"));
      }
    }
  }

  if (!mKeyword_Stack.empty()) {
    while (!mKeyword_Stack.empty()) {
      auto front = mKeyword_Stack.top();
      mKeyword_Stack.pop();
      LOG_2 << "unpoped front:" << front.type << ", content:" << front.rawLine << "\n";
    }
    throw(std::runtime_error("operator stack not closed"));
  }

  return true;
}

bool ParHelper::isCarSimKeyword(const KeywordTypes& keyword) { return (keyword != KeywordTypes::Not_Keyword); }

bool ParHelper::isLeftOperator(const KeywordTypes& keyword) {
  auto iter = std::find(Constants::leftOperators.begin(), Constants::leftOperators.end(), keyword);
  return iter != Constants::leftOperators.end();
}

bool ParHelper::isRightOperator(const KeywordTypes& keyword) {
  auto iter = std::find(Constants::rightOperators.begin(), Constants::rightOperators.end(), keyword);
  return iter != Constants::rightOperators.end();
}

bool ParHelper::isPropertyLine(const std::string& line) {
  return beginWith(line, Constants::iaxle) || beginWith(line, Constants::iside);
}

ParKeyword ParHelper::extractKeywordFromLine(const std::string& line) {
  ParKeyword keyword;

  std::vector<std::string> splitted;
  tx_car::splitString(line, splitted, " ");

  const auto& subString = splitted.front();

  // process skip keyword
  for (const auto& skippedKeyword : Constants::skipKeyword) {
    if (ParHelper::beginWith(subString, skippedKeyword)) {
      keyword.type = KeywordTypes::Not_Keyword;
      keyword.rawLine.clear();
      return keyword;
    }
  }

  VLOG(3) << "first string on extraction:" << subString << "\n";

  // process keyword
  if (endWith(subString, Constants::map1dBegin)) {
    keyword.type = KeywordTypes::MAP_1D_BEGIN;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (endWith(subString, Constants::map2dBegin)) {
    keyword.type = KeywordTypes::MAP_2D_BEGIN;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (subString == Constants::tableEnd) {
    keyword.type = KeywordTypes::TABLE_END;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (subString == Constants::parsfileBegin) {
    keyword.type = KeywordTypes::PARSFILE_BEGIN;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (subString == Constants::parsfileEnd) {
    keyword.type = KeywordTypes::PARSFILE_END;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (subString == Constants::rootBegin) {
    keyword.type = KeywordTypes::ROOT_BEGIN;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else if (subString == Constants::rootEnd) {
    keyword.type = KeywordTypes::ROOT_END;
    keyword.rawLine = line;
    keyword.id = subString;
    return keyword;
  } else {
  }

  keyword.type = KeywordTypes::Not_Keyword;
  keyword.rawLine = line;
  keyword.id = "";

  return keyword;
}

void ParHelper::removeAllCharAtBeginning(std::string& line, const char& ch) {
  if (line.size() == 0) return;

  size_t startIndex = 0;

  for (auto i = 0ul; i < line.size(); ++i) {
    if (line.at(i) != ch) {
      startIndex = i;
      break;
    }
  }
  if (startIndex == line.size() - 1) {
    line.clear();
    return;
  }

  line = line.substr(startIndex);
}

void ParHelper::removeCharsFromString(std::string& line, const std::string& chars) {
  char buffer[4096]{'\0'};
  bool shouldKeep = true;
  size_t index = 0;

  for (auto i = 0ul; i < line.size() && i < 2048; ++i) {
    shouldKeep = true;
    for (auto k = 0ul; k < chars.size(); ++k) {
      if (line.at(i) == chars.at(k)) {
        shouldKeep = false;
        break;
      }
    }
    if (shouldKeep) {
      buffer[index] = line.at(i);
      index++;
    }
  }

  buffer[index] = '\0';

  line = buffer;
}

void ParHelper::printKeyword(const ParKeyword& keyword) {
  VLOG(3) << "keyword:" << keyword.type << ", content:" << keyword.rawLine << "\n";
}

void ParHelper::dumpToFile(const std::list<std::string>& contents, const std::string& outPath, bool manualAddNewLine) {
  std::ofstream out;
  out.open(outPath, std::ios::out);
  if (out.is_open()) {
    for (auto iter = contents.begin(); iter != contents.end(); ++iter) {
      const auto& line = *iter;
      out.write(line.c_str(), line.size());
      if (manualAddNewLine) {
        out.write("\n", 1);
      }
    }
    out.close();
  }
}

void ParHelper::printParserStateInfo() { LOG_2 << "state info:" << mState.m_info << "\n"; }

bool ParHelper::isParserValid() { return mState.m_state == tx_car::TxCarStateCode::no_error; }

bool ParHelper::beginWith(const std::string& line, const std::string& targetString) {
  if (line.size() < targetString.size()) return false;
  if (targetString.size() == 0 || line.size() == 0) return false;

  for (auto i = 0ul; i < targetString.size(); ++i) {
    if (targetString[i] != line[i]) {
      return false;
    }
  }
  return true;
}

bool ParHelper::endWith(const std::string& line, const std::string& targetString) {
  if (line.size() < targetString.size()) return false;
  if (targetString.size() == 0 || line.size() == 0) return false;

  auto offset = line.size() - targetString.size();

  for (auto i = 0; i < targetString.size(); ++i) {
    if (targetString[i] != line.at(offset + i)) {
      return false;
    }
  }
  return true;
}

ParHelper::~ParHelper() {
  mState.m_state = tx_car::TxCarStateCode::no_error;
  mParContent.clear();
}

void ParHelper::debugShowContent() {
  for (auto iter = mParContent.begin(); iter != mParContent.end(); ++iter) {
    LOG_2 << *iter;  // << "\n";
  }
}
}  // namespace tx_car
