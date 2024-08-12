// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tx_scene_loader_factory.h"
#include "SceneLoader/tx_tadloader.h"
#include "SceneLoader/tx_xoscloader.h"
#include "tx_path_utils.h"
#include "tx_string_utils.h" /*to_lower*/

TX_NAMESPACE_OPEN(Base)

Base::ISceneLoaderPtr SceneLoaderFactory::CreateSceneLoader() const TX_NOEXCEPT {
  using ESceneType = ISceneLoader::ESceneType;
  // 根据场景类型进行判断
  switch (EvaluteSceneType()) {
    case ESceneType::eOSC:
      return nullptr;
    // 如果场景类型为TAD
    case ESceneType::eTAD:
      // 创建TAD_SceneLoader对象并返回
      return std::make_shared<SceneLoader::TAD_SceneLoader>();
    case ESceneType::eRAW:
    default:
      return nullptr;
  }
}

ISceneLoader::ESceneType SceneLoaderFactory::EvaluteSceneType() const TX_NOEXCEPT {
  using ESceneType = ISceneLoader::ESceneType;
  // 根据场景类型字符串进行判断
  if (txString("OpenScenario") == _sceneType) {
    return ESceneType::eOSC;
  } else if (txString("TAD") == _sceneType) {
    // 如果场景类型字符串为"TAD"，则返回eTAD枚举值
    return ESceneType::eTAD;
  } else if (txString("RAW") == _sceneType) {
    return ESceneType::eRAW;
  } else if (txString("Simrec") == _sceneType) {
    // 如果场景类型字符串为"Simrec"，则返回eSimrec枚举值
    return ESceneType::eSimrec;
  } else {
    // 如果场景类型字符串不匹配任何已知类型，则输出错误日志并返回eUNDEF枚举值
    LOG(ERROR) << "UnSupport SceneType, Name = " << _sceneType;
    return ESceneType::eUNDEF;
  }
}

txString SceneLoaderFactory::ExternNameToSceneTypeByPath(txString const& strFilePath) TX_NOEXCEPT {
  using namespace Utils;
  using namespace boost::algorithm;
  // 创建FilePath对象
  FilePath path(strFilePath);
  if (path.has_extension()) {
    // 获取文件扩展名并转换为小写
    txString strExtern = path.extension().string();
    to_lower(strExtern);
    if (txString(".xosc") == strExtern) {
      // 如果扩展名为".xosc"，则返回"OpenScenario"
      return txString("OpenScenario");
    } else if (txString(".xml") == strExtern) {
      // 如果扩展名为".xml"，则返回"TAD"
      return txString("TAD");
    } else if (txString(".simrec") == strExtern) {
      // 如果扩展名为".simrec"，则返回"Simrec"
      return txString("Simrec");
    } else {
      return txString("UNDEF");
    }
  } else {
    return txString("UNDEF");
  }
}

TX_NAMESPACE_CLOSE(Base)
