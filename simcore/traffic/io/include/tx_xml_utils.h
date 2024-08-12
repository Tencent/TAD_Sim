// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <fstream>
#include "tx_header.h"
#include "tx_logger.h"
#include "tx_string_utils.h"
TX_NAMESPACE_OPEN(Utils)

inline Base::txBool FileToStreamWithStandaloneYes(const Base::txString& _src,
                                                  std::stringstream& fileStream) TX_NOEXCEPT {
  std::ifstream infile(_src);
  if (infile) {
    fileStream.str("");
    fileStream << infile.rdbuf();
    infile.close();

    Base::txString s = fileStream.str();
    if (Utils::StringReplace(s, "standalone=\"true\"", "standalone=\"yes\"")) {
      LOG(INFO) << "scene file has header with [standalone=true]";
    }
    fileStream.str(s);
    return true;
  } else {
    LOG(WARNING) << "Load Xml File Failure. " << _src;
    return false;
  }
}

inline Base::txBool FileToStreamWithStandaloneYes(std::stringstream& fileStream) TX_NOEXCEPT {
  Base::txString s = fileStream.str();
  if (Utils::StringReplace(s, "standalone=\"true\"", "standalone=\"yes\"")) {
    LOG(INFO) << "scene file has header with [standalone=true]";
  }
  fileStream.str(s);
  return true;
}

TX_NAMESPACE_CLOSE(Utils)
