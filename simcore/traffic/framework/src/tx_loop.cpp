// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_loop.h"
#include <boost/algorithm/string.hpp>
#include "boost/optional.hpp"
#include "tx_path_utils.h"
TX_NAMESPACE_OPEN(Base)

txString txLoop::ComputeSceneType(const txString scene_path) TX_NOEXCEPT {
  boost::filesystem::path p = scene_path;
  std::string strExt = p.extension().string();
  // 如果扩展名为".xosc"
  if (boost::iequals(strExt, ".xosc")) {
    return "OpenScenario";
  } else {
    return "TAD";
  }
}

Base::txString txLoop::ConvertFromXOSC2Sim(const Base::txString xosc_scene_name) const TX_NOEXCEPT {
  if (xosc_scene_name.size() > 4 && Base::txString(".xosc") == xosc_scene_name.substr(xosc_scene_name.size() - 5)) {
    using namespace Utils;
    using namespace boost::filesystem;
    // 获取XOSC场景名称的父路径
    FilePath path_CurrentPath = FilePath(xosc_scene_name).parent_path();
    // 获取XOSC场景名称的文件名（不包括扩展名）
    FilePath file_name = FilePath(xosc_scene_name).filename().stem();
    // 构造SIM场景名称
    Base::txString simPath = path_CurrentPath.string() + "/" + file_name.string() + "_convert.sim";
    return simPath;
  } else {
    return xosc_scene_name;
  }
}
TX_NAMESPACE_CLOSE(Base)
