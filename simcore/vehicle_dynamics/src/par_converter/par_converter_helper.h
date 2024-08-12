// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include "par_converter_core.h"

namespace tx_car {
class ParHelper {
 public:
  explicit ParHelper(const std::string& parPath);
  virtual ~ParHelper();

  // check if carsim left/right operator is symmetry
  bool checkIfParSymmetry();

  // pre-process
  void process(ParsedData& parsedData);

  // conversion work
  void parse(ParsedData& parsedData);

  // reset
  void reset();

  // get raw content
  const std::list<std::string>& getParContent() { return mParContent; }
  const ParDataNodePtr& getParDataTree() { return mParTree; }

 public:
  static void dumpToFile(const std::list<std::string>& contents, const std::string& outPath,
                         bool manualAddNewLine = false);
  static bool beginWith(const std::string& line, const std::string& targetString);
  static bool endWith(const std::string& line, const std::string& targetString);
  static void removeAllCharAtBeginning(std::string& line, const char& ch);
  static void removeCharsFromString(std::string& line, const std::string& chars);

 public:
  void debugShowContent();
  void printParserStateInfo();
  static void printKeyword(const ParKeyword& keyword);

 protected:
  // extract version from .par file
  std::string extractVersion(const std::string& line);

  // build .par tree
  void buildParTree();

  // extract carsim par keyword from line, return empty rawLine if skip keyword
  ParKeyword extractKeywordFromLine(const std::string& line);

  // check if is carsim .par keyword
  bool isCarSimKeyword(const KeywordTypes& keyword);

  // check if is carsim .par left operator
  bool isLeftOperator(const KeywordTypes& keyword);

  // check if is carsim .par right operator
  bool isRightOperator(const KeywordTypes& keyword);

  // is property line
  bool isPropertyLine(const std::string& line);

  // push into operator stack
  bool pushStack(const ParKeyword& keyword);

  // pop from operator stack
  bool popStack(ParKeyword& keyword);

  /*
          extract keyword from line of par file,
          return keyword and keyword appendix if line does has keyword,
          return data ky with its raw content and string value instead if line has on keyword
          ParKeyword extractKeywordFromLine(const std::string& line);
  */
  bool isParserValid();

  // line prefix 2 is closer to current line
  void iaxleIsidePrefixProcess(ParKeyword& keyword, LineStatus& prefix1, LineStatus& prefix2);

  // traverse .par tree and convert par data
  bool convertPreProcess(const ParDataNodePtr& node);
  void convertParData(const ParDataNodePtr& node, ParDataContainer& parContainer);
  void convertMap1d(const ParDataNodePtr& node, ParDataContainer& parContainer);
  void convertMap2d(const ParDataNodePtr& node, ParDataContainer& parContainer);
  void convertParsfile(const ParDataNodePtr& node, ParDataContainer& parContainer);
  void convertRoot(const ParDataNodePtr& node, ParDataContainer& parContainer);
  void convertKVOfNode(const ParDataNodePtr& node, ParDataContainer& parContainer);

  // special process for suspension compliance since iaxle missing in carsim 2016.1 .par file
  void processCompliance(const ParDataNodePtr& node);

 private:
  std::string mParPath;
  std::list<std::string> mParContent;
  TxCarInfo mState;
  ParDataNodePtr mParTree, mCurNode;
  std::stack<ParKeyword> mKeyword_Stack;
  std::string mCarsimVersion;
};
}  // namespace tx_car
