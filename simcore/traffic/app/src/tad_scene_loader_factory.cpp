// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "tad_scene_loader_factory.h"
#include "tad_simrec_loader.h"
TX_NAMESPACE_OPEN(Scene)

Base::ISceneLoaderPtr TAD_SceneLoaderFactory::CreateSceneLoader() const TX_NOEXCEPT {
  // 如果当前场景类型为eSimrec
  if (+Base::Enums::ESceneType::eSimrec == EvaluteSceneType()) {
    // 创建一个Simrec_SceneLoader对象并返回
    return std::make_shared<SceneLoader::Simrec_SceneLoader>();
  } else {
    // 如果当前场景类型不是eSimrec，调用父类的CreateSceneLoader方法并返回
    return ParentClass::CreateSceneLoader();
  }
}

TX_NAMESPACE_CLOSE(Scene)
