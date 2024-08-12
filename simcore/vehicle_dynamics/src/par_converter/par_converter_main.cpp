// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "inc/car_log.h"
#include "inc/proto_helper.h"
#include "par_converter.h"
#include "par_converter_factory.h"

int main(int argc, char **argv) {
  FLAGS_v = 2;

  std::string parPath, txcarJsonPath;
  tx_car::ParsedData parsedData;        // parsed par data
  tx_car::CarSimParData carsimParData;  // internal proto data format
  tx_car::car txcar;                    // txcar json

  // print parameters
  for (auto i = 0; i < argc; ++i) {
    LOG_0 << "par " << i << " is " << argv[i] << "\n";
  }

  if (argc != 3) {
    LOG_ERROR << "usage par_converter xxx.par xxx.json\n";
    return -1;
  }

  // get parameter from console
  parPath = argv[1];
  txcarJsonPath = argv[2];

  // parser helper
  tx_car::ParHelper parHelper(parPath);

  // load txcar json
  std::string content;
  tx_car::car_config::loadFromFile(content, txcarJsonPath);
  tx_car::jsonToProto(content, txcar);

  // pre-process
  parHelper.process(parsedData);

  // dump pre-processed .par
  tx_car::ParHelper::dumpToFile(parHelper.getParContent(), txcarJsonPath + ".par", true);

  // check if .par keyword format correct
  if (!parHelper.checkIfParSymmetry()) {
    LOG_ERROR << "par file keyword not symmetry. " << parPath << "\n";
    return -1;
  }

  // parse
  parHelper.parse(parsedData);

  // print parsed .par data tree
  // tx_car::ParDataNode::printParDataTree(parHelper.getParDataTree());

  // make converter
  auto converter = tx_car::ParConverterFactory::makeConverter(parsedData.version);

  if (converter.get() == nullptr) {
    LOG_ERROR << "fail to make convertor for " << parsedData.version << "\n";
    return -1;
  }

  // dump in-process convertion file
  converter->parsedParToProto(parsedData, carsimParData);
  converter->parsedJsonToFile(carsimParData, txcarJsonPath + ".par.json");

  // conversion and dump final result if success
  if (converter->convertToTxCarJson(carsimParData, txcar)) {
    converter->parsedJsonToFile(txcar, txcarJsonPath + ".convert.json");
  }

  return 0;
}
