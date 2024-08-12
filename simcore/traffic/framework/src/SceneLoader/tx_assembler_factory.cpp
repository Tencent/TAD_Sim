// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "SceneLoader/tx_assembler_factory.h"

TX_NAMESPACE_OPEN(Scene)

Base::IAssemblerContextPtr AssemblerFactory::GetAssemblerContext(Base::ISceneLoader::ESceneType _sceneType)
    TX_NOEXCEPT {
#if 0
    using ESceneType = Base::ISceneLoader::ESceneType;
    switch (_sceneType) {
    case ESceneType::eTAD:
        return std::make_shared<TAD_AssemblerContext>();
    case ESceneType::eOSC:
        return std::make_shared<TAD_AssemblerContext>();
    default:
        LOG(FATAL) << "AssemblerFactory::GetAssemblerContext " << ", UnSupport Scene Type Context.";
        return Base::IAssemblerContextPtr();
    }
#else
  return nullptr;
#endif
}

TX_NAMESPACE_CLOSE(Scene)
